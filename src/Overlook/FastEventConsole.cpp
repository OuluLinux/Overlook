#include "Overlook.h"

namespace Overlook {

FastEventConsole::FastEventConsole() {
	
}

FastEventConsole::~FastEventConsole() {
	StoreThis();
}

void FastEventConsole::Init() {
	LoadThis();
}

void FastEventConsole::Start() {
	System& sys = GetSystem();
	
	FastEventStatistics& es = GetFastEventStatistics();
	FastEventOptimization& eo = GetFastEventOptimization();
	Sentiment& sent = GetSentiment();
	
	if (!eo.opt.IsEnd())
		return;
	
	int slot_id = es.GetLatestSlotId();
	
	if (!snaps.IsEmpty() && snaps.Top().is_finished == false && snaps.Top().comment != "") {
		FastEventSnap& snap = snaps.Top();
		
		if (sent.IsTakeProfit())
			snap.is_finished = true;
		
		int len = slot_id - snap.slot_id;
		if (len < 0) len += FAST_WIDTH;
		if (len >= MAX_FAST_OPEN_LEN) {
			snap.is_finished = true;
		}
		else if (snap.prev_check_slot_id != slot_id) {
			snap.prev_check_slot_id = slot_id;
			
			const FastEventSnap::Stat& s = snap.stats[0];
			const FastStatSlot& ss = es.GetSlot(s.net, s.src, snap.slot_id);
			
			ConstBuffer& open_buf = es.db[s.net]->GetBuffer(0);
			int cursor = open_buf.GetCount() - 1;
			double o0 = open_buf.Get(cursor);
			double o1 = open_buf.Get(cursor-1);
			double diff = o0 - o1;
			if (s.signal < 0) diff *= -1;
			if (diff < 0) {
				double mean, cdf;
				if (!s.inverse) {
					mean = ss.av[snap.neg_count+1].GetMean();
					cdf = ss.av[snap.neg_count+1].GetCDF(0.0, true);
				}
				else {
					mean = ss.inv_av[snap.neg_count+1].GetMean();
					cdf = ss.inv_av[snap.neg_count+1].GetCDF(0.0, true);
				}
				int grade = (1.0 - cdf) / GRADE_DIV;
				if (grade >= FastEventOptimization::grade_count /*|| mean < s.mean*/ || mean <= 0) {
					snap.is_finished = true;
				}
				else {
					snap.neg_count++;
				}
			}
		}
		
		if (!snap.is_finished)
			return;
	}
	
	if (snaps.IsEmpty() || snaps.Top().slot_id != slot_id) {
		ConstBuffer& time_buf = es.db[0]->GetBuffer(4);
		if (time_buf.IsEmpty()) return;
		
		FastEventSnap& snap = snaps.Add();
		snap.slot_id = slot_id;
		snap.prev_check_slot_id = slot_id;
		
		snap.time = Time(1970,1,1) + time_buf.Top();
		int last_pos = time_buf.GetCount() - 1;
		
		// Calendar FastEvents
		Time calbegin = snap.time - 2*60*60;
		Time calend = snap.time + 2*60*60;
		const CalendarCommon& cal = GetCalendar();
		cal.GetEvents(snap.cal, calbegin, calend, 3);
		
		
		// FastEvent statistics
		for(int i = 0; i < sys.GetNetCount(); i++) {
			for(int j = 0; j < es.SRC_COUNT; j++) {
				const FastStatSlot& ss = es.GetLatestSlot(i, j);
				
				{
					int event_count = ss.av[0].GetEventCount();
					double mean = ss.av[0].GetMean();
					int count = ss.av[0].GetEventCount();
					double cdf = ss.av[0].GetCDF(0.0, true);
					int grade = (1.0 - cdf) / GRADE_DIV;
					double abs_cdf = ss.abs_av[0].GetCDF(SPREAD_FACTOR, true);
					int abs_grade = (1.0 - abs_cdf) / GRADE_DIV;
					if (event_count >= MIN_EVENTCOUNT && grade < FastEventOptimization::grade_count && abs_grade < FastEventOptimization::grade_count) {
						int sig = es.GetOpenSignal(i, last_pos, j);
						if (sig) {
							FastEventSnap::Stat& s = snap.stats.Add();
							s.net = i;
							s.src = j;
							s.mean = mean;
							s.cdf = cdf;
							s.inverse = false;
							s.signal = sig;
							s.count = count;
							s.grade = grade;
						}
					}
				}
				
				{
					int event_count = ss.inv_av[0].GetEventCount();
					double mean = ss.inv_av[0].GetMean();
					int count = ss.inv_av[0].GetEventCount();
					double cdf = ss.inv_av[0].GetCDF(0.0, true);
					int grade = (1.0 - cdf) / GRADE_DIV;
					double abs_cdf = ss.inv_abs_av[0].GetCDF(SPREAD_FACTOR, true);
					int abs_grade = (1.0 - abs_cdf) / GRADE_DIV;
					if (event_count >= MIN_EVENTCOUNT && grade < FastEventOptimization::grade_count && abs_grade < FastEventOptimization::grade_count) {
						int sig = es.GetOpenSignal(i, last_pos, j);
						if (sig) {
							FastEventSnap::Stat& s = snap.stats.Add();
							s.net = i;
							s.src = j;
							s.mean = mean;
							s.cdf = cdf;
							s.inverse = true;
							s.signal = sig;
							s.signal *= -1;
							s.count = count;
							s.grade = grade;
						}
					}
				}
			}
		}
		Sort(snap.stats, FastEventSnap::Stat());
		
		
		// Automation
		Sentiment& sent = GetSentiment();
		SentimentSnapshot& ss = sent.AddSentiment();
		ss.added = GetUtcTime();
		ss.comment = "Clear";
		ss.cur_pres.SetCount(sys.GetCurrencyCount(), 0);
		ss.pair_pres.SetCount(sys.GetSymbolCount(), 0);
		ss.fmlevel = 1.0;
		for(int i = 0; i < snap.stats.GetCount(); i++) {
			FastEventSnap::Stat& s = snap.stats[i];
			if (s.signal) {
				snap.comment = Format("Active %d %d %d", s.net, s.src, (int)s.inverse);
				ss.comment = snap.comment;
				ss.fmlevel = FMLIMIT;
				ss.tplimit = Upp::max(0.01, eo.opt.GetBestSolution()[snap.time.hour]);
				
				if (Config::fixed_tplimit)	ss.tplimit = TPLIMIT;
				
				System::NetSetting& net = sys.GetNet(s.net);
				
				for(int j = 0; j < sent.GetSymbolCount(); j++) {
					String sym = sent.GetSymbol(j);
					int k = net.symbols.Find(sym);
					if (k == -1) continue;
					ss.pair_pres[j] = s.signal * net.symbols[k];
				}
				
				break;
			}
		}
		sent.StoreThis();
		
		StoreThis();
	}
}








FastEventConsoleCtrl::FastEventConsoleCtrl() {
	Add(hsplit.SizePos());
	hsplit.Horz();
	
	hsplit << history << calendar << FastEvents /*<< actions*/;
	
	history.AddColumn("Time");
	history.AddColumn("Comment");
	history.ColumnWidths("2 3");
	history <<= THISBACK(LoadHistory);
	calendar.AddColumn("What");
	calendar.AddColumn("When");
	FastEvents.AddColumn("Net");
	FastEvents.AddColumn("Description");
	FastEvents.AddColumn("Average");
	FastEvents.AddColumn("Cdf");
	FastEvents.AddColumn("Count");
	FastEvents.AddColumn("Signal");
	
	
	actions.Add(act.BottomPos(0,30).HSizePos());
	actions.Add(skip.BottomPos(30,30).HSizePos());
	actions.Add(automation.BottomPos(60,30).HSizePos());
	actions.Add(against.BottomPos(90,30).HSizePos());
	actions.Add(actlbl.VSizePos(0,120).HSizePos());
	
	against.SetLabel("Against");
	automation.SetLabel("Automation");
	skip.SetLabel("Skip");
	act.SetLabel("Act");
}

void FastEventConsoleCtrl::Data() {
	FastEventConsole& ec = GetFastEventConsole();
	
	bool load_history = ec.GetSnapCount() && history.GetCount() == 0;
	
	for(int i = 0; i < ec.GetSnapCount(); i++) {
		FastEventSnap& s = ec.GetSnap(i);
		
		history.Set(i, 0, s.time);
		history.Set(i, 1, s.comment);
		
	}
	
	if (load_history) {
		history.SetCursor(history.GetCount()-1);
		LoadHistory();
	}
	
	
}

void FastEventConsoleCtrl::LoadHistory() {
	FastEventConsole& ec = GetFastEventConsole();
	System& sys = GetSystem();
	FastEventStatistics& es = GetFastEventStatistics();
	
	int cursor = history.GetCursor();
	
	const FastEventSnap& snap = ec.snaps[cursor];
	
	for(int i = 0; i < snap.cal.GetCount(); i++) {
		const CalEvent& e = snap.cal[i];
		
		calendar.Set(i, 0, e.title);
		calendar.Set(i, 1, e.timestamp);
	}
	calendar.SetCount(snap.cal.GetCount());
	
	for(int i = 0; i < snap.stats.GetCount(); i++) {
		const FastEventSnap::Stat& s = snap.stats[i];
		
		FastEvents.Set(i, 0, sys.GetSymbol(sys.GetNetSymbol(s.net)));
		FastEvents.Set(i, 1, es.GetDescription(s.src) + (s.inverse ? " inverse" : ""));
		FastEvents.Set(i, 2, s.mean);
		FastEvents.Set(i, 3, s.cdf);
		FastEvents.Set(i, 4, s.count);
		FastEvents.Set(i, 5, s.signal == 0 ? "" : (s.signal > 0 ? "Long" : "Short"));
	}
	FastEvents.SetCount(snap.stats.GetCount());
	
}

}
