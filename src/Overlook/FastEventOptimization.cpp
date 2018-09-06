#if 0

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
	for(int i = 0; i < sys.GetNetCount(); i++)
		sym_ids.Add(sys.GetNetSymbol(i));
		
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
		
		int total = 24;
		opt.Min().SetCount(total);
		opt.Max().SetCount(total);
		int row = 0;
		for(int i = 0; i < 24; i++) {
			opt.Min()[row] = 0.0;
			opt.Max()[row] = 0.5;
			row++;
		}
		
		opt.SetMaxGenerations(50);
		opt.Init(total, 33);
		
		
		training_pts.SetCount(opt.GetMaxRounds(), 0.0);
	}
	
	
	
	(Brokerage&)sb = (Brokerage&)GetMetaTrader();
	sb.Init();
	sb.SetInitialBalance(1000);
	
	
	VectorMap<int, ConstBuffer*> open_bufs;
	ConstBuffer& time_buf = db[0]->GetBuffer(4);
	int end = INT_MAX;
	for(int i = 0; i < sys.GetNetCount(); i++) {
		ConstBuffer& buf = db[db.GetCount() - sys.GetNetCount() + i]->GetBuffer(0);
		int sym = sys.GetNetSymbol(i);
		open_bufs.Add(sym, &buf);
		end = min(end, buf.GetCount());
	}
	int begin = max(100, end - FASTOPT_LEN);
	last_round.SetCount(end - begin, 0);
	
	
	struct Temp : Moveable<Temp> {
		int net, src, sig, grade, abs_grade, len, neg_count, neg_count_max;
		double cdf, mean, tp_limit;
		bool inv, is_finished;
		
		bool operator() (const Temp& a, const Temp& b) const {return a.grade < b.grade;}
	};
	Temp temp;
	
	
	TimeStop ts;
	
	
	while (opt.GetRound() < opt.GetMaxRounds() && IsRunning()) {
		
		opt.Start();
		
		const Vector<double>& trial = opt.GetTrialSolution();
		
		sb.Clear();
		temp.is_finished = true;
		
		for (int cursor = begin; cursor < end; cursor++) {
			Time t = Time(1970,1,1) + time_buf.Get(cursor);
			int wday = DayOfWeek(t);
			int wb = wday * 24 + t.hour;
			
			
			// Set prices
			for(int i = 0; i < db.GetCount() - sys.GetNetCount(); i++) {
				int sym = sym_ids[i];
				double point = this->point[i];
				ConstBuffer& open_buf = db[i]->GetBuffer(0);
				double price = open_buf.Get(cursor);
				sb.SetPrice(sym, price, price - point * 2.6);
			}
			
			sb.RefreshOrders();
			
			// For all FastEventStatistics nets and sources
			// get all active signals to a temporary list
			// and pick random item, which gives net and source ids
			bool new_signal = temp.is_finished;
			if (t.minute == 0) {
				if (temp.is_finished) {
					temp.cdf = 0;
					temp.mean = 0;
					for(int i = 0; i < sys.GetNetCount(); i++) {
						for(int j = 0; j < es.SRC_COUNT; j++) {
							int sig = es.GetOpenSignal(i, cursor, j);
							
							if (!sig) continue;
							
							const FastStatSlot& ss = es.GetSlot(i, j, wb);
							
							for (int neg_count = 0; neg_count < NEGCOUNT_MAX; neg_count++) {
								if (ss.av[neg_count].GetEventCount() == 0)
									break;
								int event_count = ss.av[neg_count].GetEventCount();
								double mean = ss.av[neg_count].GetMean();
								double cdf = ss.av[neg_count].GetCDF(0.0, true);
								int grade = (1.0 - cdf) / GRADE_DIV;
								double abs_cdf = ss.abs_av[neg_count].GetCDF(SPREAD_FACTOR, true);
								int abs_grade = (1.0 - abs_cdf) / GRADE_DIV;
								
								if (event_count >= MIN_EVENTCOUNT && grade < grade_count && abs_grade < grade_count && mean > temp.mean) {
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
									temp.tp_limit = trial[t.hour];
									temp.is_finished = false;
									break;
								}
							}
							
							for (int neg_count = 0; neg_count < NEGCOUNT_MAX; neg_count++) {
								if (ss.inv_av[neg_count].GetEventCount() == 0)
									break;
								int event_count = ss.inv_av[neg_count].GetEventCount();
								double mean = ss.inv_av[neg_count].GetMean();
								double cdf = ss.inv_av[neg_count].GetCDF(0.0, true);
								int grade = (1.0 - cdf) / GRADE_DIV;
								double abs_cdf = ss.inv_abs_av[neg_count].GetCDF(SPREAD_FACTOR, true);
								int abs_grade = (1.0 - abs_cdf) / GRADE_DIV;
								
								if (event_count >= MIN_EVENTCOUNT && grade < grade_count && abs_grade < grade_count && mean > temp.mean) {
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
									temp.tp_limit = trial[t.hour];
									temp.is_finished = false;
									break;
								}
							}
						}
					}
				}
				else if (temp.len >= MAX_FAST_OPEN_LEN) {
					temp.is_finished = true;
				}
				else {
					ConstBuffer& open_buf = *open_bufs[temp.net];
					const FastStatSlot& ss = es.GetSlot(temp.net, temp.src, wb);
					
					double o0 = open_buf.Get(cursor);
					double o1 = open_buf.Get(cursor-1);
					double diff = o0 - o1;
					if (temp.sig < 0) diff *= -1;
					if (diff < 0) {
						double mean, cdf;
						if (!temp.inv) {
							mean = ss.av[temp.neg_count+1].GetMean();
							cdf = ss.av[temp.neg_count+1].GetCDF(0.0, true);
						}
						else {
							mean = ss.inv_av[temp.neg_count+1].GetMean();
							cdf = ss.inv_av[temp.neg_count+1].GetCDF(0.0, true);
						}
						int grade = (1.0 - cdf) / GRADE_DIV;
						if (grade >= grade_count /*|| mean < temp.mean*/ || mean <= 0) {
							temp.is_finished = true;
						}
						temp.neg_count++;
					}
					
					double balance = sb.AccountBalance();
					double equity = sb.AccountEquity();
					if (equity >= balance * (1.0 + temp.tp_limit)) {
						temp.is_finished = true;
					}
				}
			} else {
				double balance = sb.AccountBalance();
				double equity = sb.AccountEquity();
				if (equity >= balance * (1.0 + temp.tp_limit)) {
					temp.is_finished = true;
				}
			}
			
			if (temp.is_finished == false) {
				const System::NetSetting& ns = sys.GetNet(temp.net);
				
				
				
				sb.SetFreeMarginLevel(FMLIMIT);
				sb.SetFreeMarginScale(ns.symbols.GetCount());
				
				// Set signal using net and source ids.
				for(int i = 0; i < ns.symbols.GetCount(); i++) {
					int sym = ns.symbol_ids.GetKey(i);
					int sig = ns.symbol_ids[i] * temp.sig;
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
#endif
