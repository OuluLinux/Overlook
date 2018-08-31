#include "Overlook.h"

namespace Overlook {

EventOptimization::EventOptimization() {
	LoadThis();
}

EventOptimization::~EventOptimization() {
	sig.Stop();
}

void EventOptimization::Init() {
	
	System& sys = GetSystem();
	
	int sym_count = sys.GetNormalSymbolCount();
	int cur_count = sys.GetCurrencyCount();
	int net_count = sys.GetNetCount();
	int width = sys.GetVtfWeekbars();
	
	indi_ids.Add().Set(sys.Find<DataBridge>());

	for(int i = 0; i < indi_ids.GetCount(); i++)
		fac_ids.Add(indi_ids[i].factory);
	
	const System::NetSetting& ns = sys.GetNet(0);
	for(int i = 0; i < ns.symbol_ids.GetCount(); i++)
		sym_ids.Add(ns.symbol_ids.GetKey(i));
	
	tf_ids.Add(VTF);
	
	point.SetCount(sym_ids.GetCount(), 0.0001);
	
	sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	db.SetCount(sym_ids.GetCount(), NULL);
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
		
		Core& c = *ci.core;
		
		int symi = sym_ids.Find(c.GetSymbol());
		int tfi = tf_ids.Find(c.GetTf());
		
		if (symi == -1) continue;
		if (tfi == -1) continue;
		
		
		if (c.GetFactory() == 0) {
			db[symi] = &dynamic_cast<DataBridge&>(c);
			point[symi] = db[symi]->GetPoint();
		}
		
		
		sys.Process(*work_queue[i], true);
	}
	
	
	if (opt.GetRound() < opt.GetMaxRounds()) {
		sig.Start();
		Thread::Start(THISBACK(Process));
	}
}

void EventOptimization::Start() {
	
}

void EventOptimization::Process() {
	System& sys = GetSystem();
	EventStatistics& es = GetEventStatistics();
	
	int weekbars = sys.GetVtfWeekbars();
	
	
	
	bool once = false;
	if (opt.GetRound() == 0 || opt.GetRound() >= opt.GetMaxRounds() || once) {
		once = false;
		
		int total = /*sys.GetVtfWeekbars() * */grade_count;
		opt.Min().SetCount(total);
		opt.Max().SetCount(total);
		int row = 0;
		//for(int i = 0; i < sys.GetVtfWeekbars(); i++) {
			for(int j = 0; j < grade_count; j++) {
				opt.Min()[row] = 0.6;
				opt.Max()[row] = 1.1;
				row++;
			}
		//}
		
		#ifdef flagDEBUG
		opt.SetMaxGenerations(5);
		#else
		opt.SetMaxGenerations(10);
		#endif
		opt.Init(total, 33);
		
		
		training_pts.SetCount(opt.GetMaxRounds(), 0.0);
	}
	
	
	
	(Brokerage&)sb = (Brokerage&)GetMetaTrader();
	sb.Init();
	sb.SetInitialBalance(1000);
	
	
	VectorMap<int, ConstBuffer*> open_bufs;
	int begin = 100, end = INT_MAX;
	for(int i = 0; i < sys.GetNetCount(); i++) {
		ConstBuffer& buf = es.GetBuffer(i, 0, 0);
		int sym = sys.GetNetSymbol(i);
		open_bufs.Add(sym, &buf);
		end = min(end, buf.GetCount());
	}
	
	last_round.SetCount(end - begin, 0);
	
	
	struct Temp : Moveable<Temp> {
		int net, src, sig, grade, abs_grade;
		double cdf, mean;
		
		bool operator() (const Temp& a, const Temp& b) const {return a.grade < b.grade;}
	};
	Temp temp;
	
	
	TimeStop ts;
	
	
	while (opt.GetRound() < opt.GetMaxRounds() && IsRunning()) {
		
		opt.Start();
		
		const Vector<double>& trial = opt.GetTrialSolution();
		
		sb.Clear();
		
		for (int cursor = begin; cursor < end; cursor++) {
			int wb = cursor % weekbars;
			
			
			// For all EventStatistics nets and sources
			// get all active signals to a temporary list
			// and pick random item, which gives net and source ids
			temp.mean = 0;
			temp.cdf = 0;
			for(int i = 0; i < sys.GetNetCount(); i++) {
				for(int j = 0; j < EventStatistics::SRC_COUNT; j++) {
					int sig = es.GetSignal(i, cursor, j);
					if (!sig) continue;
					
					const StatSlot& ss = es.GetSlot(i, j, wb);
					double mean = ss.av.GetMean();
					bool inverse = mean < 0.0;
					if (inverse) {
						mean = -mean;
						sig *= -1;
					}
					double cdf = ss.av.GetCDF(0.0, !inverse);
					int grade = (1.0 - cdf) / 0.05;
					double abs_cdf = ss.abs_av.GetCDF(SPREAD_FACTOR, true);
					int abs_grade = (1.0 - abs_cdf) / 0.05;
					
					int solution_i = /*wb * grade_count +*/ grade;
					double fmlevel = grade < grade_count ? trial[solution_i] : 1.0;
					
					if (grade < grade_count && abs_grade < grade_count && mean > temp.mean && fmlevel < 1.0) {
					//if (grade < grade_count && (cdf > temp.cdf || (cdf == temp.cdf && mean > temp.mean))) {
						temp.net = i;
						temp.src = j;
						temp.sig = sig;
						temp.grade = grade;
						temp.abs_grade = abs_grade;
						temp.cdf = cdf;
						temp.mean = mean;
					}
				}
			}
			
			
			if (temp.mean > 0) {
				const System::NetSetting& ns = sys.GetNet(temp.net);
				
				
				// Get class from item and use it to set fm-level
				int solution_i = /*wb * grade_count +*/ temp.grade;
				double fmlevel = trial[solution_i];
				sb.SetFreeMarginLevel(fmlevel);
				sb.SetFreeMarginScale(ns.symbols.GetCount());
				bool main_switch = fmlevel < 1.0;
				
				// Set signal using net and source ids. Don't freeze signal.
				
				for(int i = 0; i < ns.symbols.GetCount(); i++) {
					int sym = ns.symbol_ids.GetKey(i);
					int sig = ns.symbol_ids[i] * temp.sig * (int)main_switch;
					sb.SetSignal(sym, sig);
				}
				
			} else {
				const System::NetSetting& ns = sys.GetNet(0);
				
				for(int i = 0; i < ns.symbols.GetCount(); i++) {
					int sym = ns.symbol_ids.GetKey(i);
					sb.SetSignal(sym, 0);
				}
			}
			
			
			// Set prices
			for(int i = 0; i < db.GetCount(); i++) {
				int sym = sym_ids[i];
				double point = this->point[i];
				ConstBuffer& open_buf = db[i]->GetBuffer(0);
				double price = open_buf.Get(cursor);
				sb.SetPrice(sym, price, price - point * 3);
			}
			
			sb.RefreshOrders();
			
			last_round[cursor - begin] = sb.AccountEquity();
			
			sb.Cycle();
		}
		
		sb.CloseAll();
		
		
		double result = sb.AccountEquity();
		training_pts[opt.GetRound()] = result;
		LOG(result);
		
		
		opt.Stop(result);
		
		
		if (ts.Elapsed() >= 60 *1000) {
			StoreThis();
			ts.Reset();
		}
	}
	
	StoreThis();
	
	sig.SetStopped();
}




EventOptimizationCtrl::EventOptimizationCtrl() {
	Add(draw.HSizePos().VSizePos(0, 10));
	Add(prog.BottomPos(0, 10).HSizePos());
	
	prog.Set(0, 1);
	
	
}

void EventOptimizationCtrl::Data() {
	EventOptimization& eo = GetEventOptimization();
	
	prog.Set(eo.opt.GetRound(), eo.opt.GetMaxRounds());
	
	draw.Refresh();
}


void EventOptimizationDrawer::Paint(Draw& d) {
	EventOptimization& eo = GetEventOptimization();
	Size sz(GetSize());
	sz.cx /= 2;
	
	ImageDraw id_a(sz);
	ImageDraw id_b(sz);
	
	id_a.DrawRect(sz, White());
	id_b.DrawRect(sz, White());
	
	DrawVectorPolyline(id_a, sz, eo.training_pts, cache, eo.opt.GetRound());
	DrawVectorPolyline(id_b, sz, eo.last_round, cache);
	
	d.DrawImage(0, 0, id_a);
	d.DrawImage(sz.cx, 0, id_b);
}

}
