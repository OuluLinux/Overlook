#include "Overlook.h"

namespace Overlook {

FastEventOptimization::FastEventOptimization() {
	LoadThis();
}

FastEventOptimization::~FastEventOptimization() {
	sig.Stop();
}

void FastEventOptimization::Init() {
	
	System& sys = GetSystem();
	
	int sym_count = sys.GetNormalSymbolCount();
	int cur_count = sys.GetCurrencyCount();
	int net_count = sys.GetNetCount();
	int width = FAST_WIDTH;
	
	indi_ids.Add().Set(sys.Find<DataBridge>());

	for(int i = 0; i < indi_ids.GetCount(); i++)
		fac_ids.Add(indi_ids[i].factory);
	
	const System::NetSetting& ns = sys.GetNet(0);
	for(int i = 0; i < ns.symbol_ids.GetCount(); i++)
		sym_ids.Add(ns.symbol_ids.GetKey(i));
	
	tf_ids.Add(FAST_TF);
	
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

void FastEventOptimization::Start() {
	
}

void FastEventOptimization::Process() {
	System& sys = GetSystem();
	FastEventStatistics& es = GetFastEventStatistics();
	
	int weekbars = FAST_WIDTH;
	
	
	
	bool once = false;
	if (opt.GetRound() == 0 || opt.GetRound() >= opt.GetMaxRounds() || once) {
		once = false;
		
		int total = /*FAST_WIDTH * */grade_count;
		opt.Min().SetCount(total);
		opt.Max().SetCount(total);
		int row = 0;
		//for(int i = 0; i < FAST_WIDTH; i++) {
			for(int j = 0; j < grade_count; j++) {
				opt.Min()[row] = 0.9;
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
	ConstBuffer& time_buf = es.GetBuffer(0, 0, 4);
	int begin = 100, end = INT_MAX;
	for(int i = 0; i < sys.GetNetCount(); i++) {
		ConstBuffer& buf = es.GetBuffer(i, 0, 0);
		int sym = sys.GetNetSymbol(i);
		open_bufs.Add(sym, &buf);
		end = min(end, buf.GetCount());
	}
	
	last_round.SetCount(end - begin, 0);
	
	
	struct Temp : Moveable<Temp> {
		int net, src, sig, grade, abs_grade, len, neg_count, neg_count_max;
		double cdf, mean;
		bool inv;
		
		bool operator() (const Temp& a, const Temp& b) const {return a.grade < b.grade;}
	};
	Temp temp;
	
	
	TimeStop ts;
	
	
	while (opt.GetRound() < opt.GetMaxRounds() && IsRunning()) {
		
		opt.Start();
		
		const Vector<double>& trial = opt.GetTrialSolution();
		
		sb.Clear();
		temp.mean = 0;
		
		for (int cursor = begin; cursor < end; cursor++) {
			Time t = Time(1970,1,1) + time_buf.Get(cursor);
			int wday = DayOfWeek(t);
			int wb = wday * 24 + t.hour;
			
			
			// For all FastEventStatistics nets and sources
			// get all active signals to a temporary list
			// and pick random item, which gives net and source ids
			bool new_signal = temp.mean == 0;
			if (new_signal) {
				temp.cdf = 0;
				for(int i = 0; i < sys.GetNetCount(); i++) {
					for(int j = 0; j < FastEventStatistics::SRC_COUNT; j++) {
						int sig = es.GetOpenSignal(i, cursor, j);
						if (!sig) continue;
						
						const FastStatSlot& ss = es.GetSlot(i, j, wb);
						
						for (int neg_count = 0; neg_count < NEGCOUNT_MAX; neg_count++) {
							if (ss.av[neg_count].GetEventCount() == 0)
								break;
							double mean = ss.av[neg_count].GetMean();
							double cdf = ss.av[neg_count].GetCDF(0.0, true);
							int grade = (1.0 - cdf) / 0.05;
							double abs_cdf = ss.abs_av[neg_count].GetCDF(0.0003, true);
							int abs_grade = (1.0 - abs_cdf) / 0.05;
							
							int solution_i = /*wb * grade_count +*/ grade;
							double fmlevel = grade < grade_count ? trial[solution_i] : 1.0;
							
							if (grade < grade_count && abs_grade < grade_count && mean > temp.mean && fmlevel < 1.0) {
								temp.net = i;
								temp.src = j;
								temp.sig = sig;
								temp.grade = grade;
								temp.abs_grade = abs_grade;
								temp.cdf = cdf;
								temp.mean = mean;
								temp.len = 0;
								temp.neg_count = 0;
								temp.neg_count_max = neg_count;
								temp.inv = 0;
								break;
							}
						}
						
						for (int neg_count = 0; neg_count < NEGCOUNT_MAX; neg_count++) {
							if (ss.inv_av[neg_count].GetEventCount() == 0)
								break;
							double mean = ss.inv_av[neg_count].GetMean();
							double cdf = ss.inv_av[neg_count].GetCDF(0.0, true);
							int grade = (1.0 - cdf) / 0.05;
							double abs_cdf = ss.inv_abs_av[neg_count].GetCDF(0.0003, true);
							int abs_grade = (1.0 - abs_cdf) / 0.05;
							
							int solution_i = /*wb * grade_count +*/ grade;
							double fmlevel = grade < grade_count ? trial[solution_i] : 1.0;
							
							if (grade < grade_count && abs_grade < grade_count && mean > temp.mean && fmlevel < 1.0) {
								temp.net = i;
								temp.src = j;
								temp.sig = sig * -1;
								temp.grade = grade;
								temp.abs_grade = abs_grade;
								temp.cdf = cdf;
								temp.mean = mean;
								temp.len = 0;
								temp.neg_count = 0;
								temp.neg_count_max = neg_count;
								temp.inv = 1;
									break;
							}
						}
					}
				}
			}
			else if (temp.len >= MAX_FAST_LEN) {
				temp.mean = 0;
			}
			else {
				ConstBuffer& open_buf = *open_bufs[temp.net];
				const FastStatSlot& ss = es.GetSlot(temp.net, temp.src, wb);
				
				#if NEGCOUNT_ENABLED
				double o0 = open_buf.Get(cursor);
				double o1 = open_buf.Get(cursor-1);
				double diff = o0 - o1;
				if (temp.sig < 0) diff *= -1;
				if (diff < 0) {
					double mean;
					if (!temp.inv)
						mean = ss.av[temp.neg_count+1].GetMean();
					else
						mean = ss.inv_av[temp.neg_count+1].GetMean();
					if (mean < temp.mean) {
						temp.mean = 0; // reset
					}
					temp.neg_count++;
					/*if (temp.neg_count >= temp.neg_count_max)
						temp.mean = 0; // reset*/
				}
				#else
				if (temp.len >= temp.neg_count_max)
					temp.mean = 0; // reset
				#endif
			}
			
			
			if (temp.mean > 0) {
				const System::NetSetting& ns = sys.GetNet(temp.net);
				
				
				// Get class from item and use it to set fm-level
				int solution_i = /*wb * grade_count +*/ temp.grade;
				double fmlevel = trial[solution_i];
				sb.SetFreeMarginLevel(fmlevel);
				sb.SetFreeMarginScale(ns.symbols.GetCount());
				bool main_switch = fmlevel < 1.0;
				
				// Set signal using net and source ids.
				for(int i = 0; i < ns.symbols.GetCount(); i++) {
					int sym = ns.symbol_ids.GetKey(i);
					int sig = ns.symbol_ids[i] * temp.sig * (int)main_switch;
					sb.SetSignal(sym, sig);
					sb.SetSignalFreeze(sym, !new_signal);
				}
				
				
				temp.len++;
			} else {
				const System::NetSetting& ns = sys.GetNet(0);
				
				for(int i = 0; i < ns.symbols.GetCount(); i++) {
					int sym = ns.symbol_ids.GetKey(i);
					sb.SetSignal(sym, 0);
					sb.SetSignalFreeze(sym, false);
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




FastEventOptimizationCtrl::FastEventOptimizationCtrl() {
	Add(draw.HSizePos().VSizePos(0, 10));
	Add(prog.BottomPos(0, 10).HSizePos());
	
	prog.Set(0, 1);
	
	
}

void FastEventOptimizationCtrl::Data() {
	FastEventOptimization& eo = GetFastEventOptimization();
	
	prog.Set(eo.opt.GetRound(), eo.opt.GetMaxRounds());
	
	draw.Refresh();
}


void FastEventOptimizationDrawer::Paint(Draw& d) {
	FastEventOptimization& eo = GetFastEventOptimization();
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
