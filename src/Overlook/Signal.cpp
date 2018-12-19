#include "Overlook.h"

#if 0
namespace Overlook {


Signal::Signal() {
	
}

void Signal::Init() {
	
}

void Signal::Start() {
	
	RefreshDays();
	RefreshCalendar();
	RefreshCalendarSignals();
	//RefreshSignals();
	RefreshPips();
	
}

void Signal::RefreshDays() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	
	const Index<Time>& ti = dbc.GetTimeIndex(0);
	
	Date cur = ti[0];
	Date end = GetUtcTime();
	end++;
	
	if (days.GetCount())
		cur = days.TopKey() + 1;
	
	for (; cur < end; cur++) {
		int wday = DayOfWeek(cur);
		
		if (wday == 0 || wday == 6) continue;
		
		Day& d = days.Add(cur);
		d.date = cur;
		
		
	}
	
}

void Signal::RefreshCalendar() {
	CalendarCommon& cal = GetCalendar();
	
	Date begin = days.GetKey(0);
	
	for(;cal_counter < cal.GetCount(); cal_counter++) {
		const CalEvent& ev = cal.GetEvent(cal_counter);
		
		if (ev.timestamp < begin) continue;
		
		int i = days.Find(ev.timestamp);
		if (i != -1) {
			Day& d = days[i];
			d.cal.Add(cal_counter);
		}
	}
	
}

void Signal::RefreshCalendarSignals() {
	System& sys = GetSystem();
	CalendarCommon& cal = GetCalendar();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	
	const Index<Time>& ti = dbc.GetTimeIndex(0);
	
	if (calsig_counter > 0) calsig_counter--;
	
	bool debug = 0;
	int active_days = 0;
	int high_active_days = 0;
	int high_upcoming_active_days = 0;
	
	for(;calsig_counter < days.GetCount(); calsig_counter++) {
		Day& d = days[calsig_counter];
		
		bool is_today = calsig_counter == days.GetCount()-1;
		if (is_today) {
			d.calsig_counter = 0;
			d.cursig.Clear();
		}
		
		Time prev_time(1970,1,1);
		String prev_currency;
		for(; d.calsig_counter < d.cal.GetCount(); d.calsig_counter++) {
			const CalEvent& first = cal.GetEvent(d.cal[d.calsig_counter]);
			
			if (first.timestamp == prev_time && first.currency == prev_currency)
				continue;
			prev_time = first.timestamp;
			prev_currency = first.currency;
			
			int max_impact = first.impact;
			Vector<int> signals, prev_signals;
			
			for(int i = d.calsig_counter+1; i < d.cal.GetCount(); i++) {
				const CalEvent& ev = cal.GetEvent(d.cal[i]);
				
				if (first.timestamp == ev.timestamp && first.currency == ev.currency) {
					if (ev.impact > max_impact)
						max_impact = ev.impact;
				}
				else if (first.timestamp < ev.timestamp) break;
			}
			
			
			for(int i = d.calsig_counter; i < d.cal.GetCount(); i++) {
				const CalEvent& ev = cal.GetEvent(d.cal[i]);
				
				if (first.timestamp == ev.timestamp && first.currency == ev.currency) {
					if (ev.impact == max_impact) {
						int sig = GetSignal(ev);
						signals.Add(sig);
						int prev_sig = GetPrevSignal(ev);
						prev_signals.Add(prev_sig);
					}
				}
				else if (first.timestamp < ev.timestamp) break;
			}
			
			if (signals.GetCount()) {
				int first_sig = signals[0];
				int first_prev_sig = prev_signals[0];
				bool is_same_signal = true;
				for(int i = 1; i < signals.GetCount(); i++) {
					if (signals[i] != first_sig)
						is_same_signal = false;
				}
				
				if (is_same_signal) {
					CurrencySignal& cursig = d.cursig.Add();
					String second_currency;
					if (first.currency == "USD") second_currency = "EUR";
					else second_currency = "USD";
					
					String pair0 = first.currency + second_currency;
					String pair1 = second_currency + first.currency;
					cursig.id = sys.FindSymbol(pair0);
					cursig.sig = first_sig;
					if (cursig.id == -1) {
						cursig.id = sys.FindSymbol(pair1);
						cursig.sig = -1 * first_sig;
					}
					if (max_impact < 3)
						cursig.sig = 0;
					
					cursig.time = ti.Find(first.timestamp);
					cursig.has_prev_sig = first_prev_sig != 0;
					cursig.impact = max_impact;
					cursig.sig_count = signals.GetCount();
					cursig.currency = first.currency;
				}
			}
		}
		
		
		if (d.cursig.GetCount()) {
			active_days++;
			if (debug) {LOG(d.date);}
			d.has_high_impacts = false;
			d.has_high_impacts_upcoming = false;
			for(int j = 0; j < d.cursig.GetCount(); j++) {
				CurrencySignal& cs = d.cursig[j];
				if (debug) {LOG("    " << cs.sig << " " << cs.sig_count << " " << (int)cs.impact << " " << cs.currency);}
				if (cs.impact == 3 && cs.sig != 0)
					d.has_high_impacts = true;
				if (is_today && cs.impact == 3 && cs.has_prev_sig && cs.sig == 0)
					d.has_high_impacts_upcoming = true;
			}
			if (d.has_high_impacts)
				high_active_days++;
			if (d.has_high_impacts_upcoming)
				high_upcoming_active_days++;
		}
	}
	
	if (debug) {
		double perc = active_days * 100.0 / days.GetCount();
		double high_perc = high_active_days * 100.0 / days.GetCount();
		double high_upcoming_perc = high_upcoming_active_days * 100.0 / days.GetCount();
		LOG("Active days " << active_days << "/" << days.GetCount() << " " << perc << "%");
		LOG("High active days " << high_active_days << "/" << days.GetCount() << " " << high_perc << "%");
		LOG("High upcoming active days " << high_upcoming_active_days << "/" << days.GetCount() << " " << high_upcoming_perc << "%");
	}
	
}

int Signal::GetSignal(const CalEvent& ev) {
	if (ev.forecast.GetCount() && ev.actual.GetCount()) {
		double fc = ScanDouble(ev.forecast);
		double ac = ScanDouble(ev.actual);
		
		int sig = 0;
		if (ac > fc) sig = +1;
		if (ac < fc) sig = -1;
		
		if (ev.title.Find("Unemployment") != -1) sig *= -1;
		
		return sig;
	}
	else return 0;
}

int Signal::GetPrevSignal(const CalEvent& ev) {
	if (ev.forecast.GetCount() && ev.previous.GetCount()) {
		double fc = ScanDouble(ev.forecast);
		double pr = ScanDouble(ev.previous);
		
		int sig = 0;
		if (fc > pr) sig = +1;
		if (fc < pr) sig = -1;
		
		if (ev.title.Find("Unemployment") != -1) sig *= -1;
		
		return sig;
	}
	else return 0;
}

void Signal::RefreshSignals() {
	System& sys = GetSystem();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	
	const Index<Time>& ti = dbc.GetTimeIndex(0);
	
	CoreList cl;
	cl.AddTf(0);
	cl.AddSymbol("USD1");
	cl.AddSymbol("EUR1");
	cl.AddSymbol("GBP1");
	cl.AddSymbol("JPY1");
	cl.AddSymbol("CAD1");
	cl.AddIndi(sys.Find<MovingAverage>()).AddArg(60);
	cl.AddIndi(sys.Find<MovingAverage>()).AddArg(240);
	cl.AddIndi(sys.Find<ParabolicSAR>());
	cl.AddIndi(sys.Find<StochasticOscillator>()).AddArg(8);
	cl.AddIndi(sys.Find<StochasticOscillator>()).AddArg(15);
	cl.Init();
	cl.Refresh();
	
	Vector<int> signals;
	signals.SetCount(cl.GetSymbolCount(), 0);
	
	int debug = 0;
	
	Day* d = NULL;
	Date prev_date(1970,1,1);
	for(; sig_counter < ti.GetCount(); sig_counter++) {
		
		Date date = ti[sig_counter];
		if (date != prev_date || d == NULL) {
			int i = days.Find(date);
			if (i >= 0)
				d = &days[i];
			else
				d = NULL;
		}
		prev_date = date;
		
		if (d == NULL) continue;
		
		for(int i = 0; i < cl.GetSymbolCount(); i++) {
			
			ConstLabelSignal& ma0lbl = cl.GetLabelSignal(i, 0);
			ConstLabelSignal& ma1lbl = cl.GetLabelSignal(i, 1);
			ConstLabelSignal& psarlbl = cl.GetLabelSignal(i, 2);
			ConstBuffer& stoch0 = cl.GetBuffer(i, 3, 0);
			ConstBuffer& stoch1 = cl.GetBuffer(i, 4, 0);
			
			int sig = 0;
			
			bool ma0sig = ma0lbl.signal.Get(sig_counter);
			bool ma1sig = ma1lbl.signal.Get(sig_counter);
			bool psarsig = psarlbl.signal.Get(sig_counter);
			
			if (ma0sig == psarsig && ma1sig == ma0sig) {
				if (ma0sig == 0) {
					double s = stoch0.Get(sig_counter);
					if (s < 20)
						sig = +1;
				}
				else {
					double s = stoch0.Get(sig_counter);
					if (s > 80)
						sig = -1;
				}
			}
			
			signals[i] = sig;
		}
		
		for(int i = 0; i < signals.GetCount(); i++) {
			int sig_a = signals[i];
			for(int j = i+1; j < signals.GetCount(); j++) {
				int sig_b = signals[j];
				if (sig_a * sig_b < 0) {
					String pair0 = cl.GetSymbol(i).Left(3) + cl.GetSymbol(j).Left(3);
					String pair1 = cl.GetSymbol(j).Left(3) + cl.GetSymbol(i).Left(3);
					String pair;
					int id = sys.FindSymbol(pair0);
					int sig = 0;
					if (id >= 0) {
						sig = sig_a;
						pair = pair0;
					} else {
						id = sys.FindSymbol(pair1);
						sig = sig_b;
						pair = pair1;
					}
					
					if (d->pairsig.GetCount()) {
						PairSignal& prev_pairsig = d->pairsig.Top();
						if (prev_pairsig.id == id && prev_pairsig.sig == sig)
							continue;
					}
					PairSignal& pairsig = d->pairsig.Add();
					pairsig.time = sig_counter;
					pairsig.id = id;
					pairsig.sig = sig;
					
					if (debug) {
						LOG(ti[sig_counter] << " " << pair << " " << sig);
						if (id == -1) {
							LOG(pair);
						}
					}
				}
			}
		}
	}
	
	if (debug) {
		int days_have_signals = 0;
		for(int i = 0; i < days.GetCount(); i++) {
			if (days[i].pairsig.GetCount())
				days_have_signals++;
		}
		double perc = days_have_signals * 100.0 / days.GetCount();
		LOG("Active signal days " << days_have_signals << "/" << days.GetCount() << " " << perc << "%");
		LOG("");
	}
	
}

void Signal::RefreshPips() {
	
	if (pip_counter > 0) pip_counter--;
	
	int debug = 1;
	int days_count = 0;
	int cur_pos_pips = 0, cur_neg_pips = 0;
	int pair_pos_pips = 0, pair_neg_pips = 0;
	
	for (; pip_counter < days.GetCount(); pip_counter++) {
		Day& d = days[pip_counter];
		
		for(int i = 0; i < d.cursig.GetCount(); i++) {
			PairSignal& pairsig = d.cursig[i];
			GetPips(pairsig, 30, 30);
			if (pairsig.pips > 0)	cur_pos_pips += pairsig.pips;
			else					cur_neg_pips -= pairsig.pips;
			if (debug) {LOG(days.GetKey(pip_counter) << " " << pairsig.pips);}
		}
		
		if (!d.has_high_impacts)
		{
			for(int i = 0; i < d.pairsig.GetCount(); i++) {
				PairSignal& pairsig = d.pairsig[i];
				GetPips(pairsig, 30, 30);
				if (pairsig.pips > 0)	pair_pos_pips += pairsig.pips;
				else					pair_neg_pips -= pairsig.pips;
				if (debug) {LOG(days.GetKey(pip_counter) << " " << pairsig.pips);}
			}
		}
		
		days_count++;
	}
	
	if (debug) {
		double cur_dd = (double)cur_neg_pips / (cur_neg_pips + cur_pos_pips);
		double pair_dd = (double)pair_neg_pips / (pair_neg_pips + pair_pos_pips);
		LOG("Currency positive pips " << cur_pos_pips);
		LOG("Currency negative pips " << cur_neg_pips);
		LOG("Pair positive pips " << pair_pos_pips);
		LOG("Pair negative pips " << pair_neg_pips);
		LOG("Currency DD " << (cur_dd * 100));
		LOG("Pair DD " << (pair_dd * 100));
		LOG("");
	}
}

void Signal::GetPips(PairSignal& pairsig, int tp_pips, int sl_pips) {
	if (pairsig.sig == 0) return;
	if (pairsig.id == -1) return;
	bool is_pos = pairsig.sig > 0;
	
	System& sys = GetSystem();
	CoreList cl;
	cl.AddTf(0);
	cl.AddIndi(0);
	cl.AddSymbol(sys.GetSymbol(pairsig.id));
	cl.Init();
	cl.Refresh();
	
	int spread_pips = 3;
	
	
	ConstBuffer& open = cl.GetBuffer(0, 0, 0);
	DataBridge& db = *cl.GetDataBridge(0);
	double point = db.GetPoint();
	
	int end = min(open.GetCount(), pairsig.time + 240);
	
	double o = open.Get(pairsig.time);
	double tp, sl;
	if (is_pos) {
		o += spread_pips * point;
		tp = o + tp_pips * point;
		sl = o - sl_pips * point;
	} else {
		o -= spread_pips * point;
		tp = o - tp_pips * point;
		sl = o + sl_pips * point;
	}
	
	double price;
	for (int i = pairsig.time+1; i < end; i++) {
		
		price = open.Get(i);
		
		if (is_pos) {
			if (price > tp || price < sl) {
				break;
			}
		} else {
			if (price < tp || price > sl) {
				break;
			}
		}
	}
	
	if (is_pos)
		pairsig.pips = (price - o) / point;
	else
		pairsig.pips = -(price - o) / point;
}

SignalCtrl::SignalCtrl() {
	
}

void SignalCtrl::Data() {
	
}
	
	
}
#endif
