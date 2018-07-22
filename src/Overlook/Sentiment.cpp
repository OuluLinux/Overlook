#include "Overlook.h"


namespace Overlook {
	


Sentiment::Sentiment() {
	
	
	symbols.Add("EURUSD");
	symbols.Add("GBPUSD");
	symbols.Add("USDCHF");
	symbols.Add("USDJPY");
	symbols.Add("USDCAD");
	symbols.Add("AUDUSD");
	symbols.Add("NZDUSD");
	symbols.Add("EURCHF");
	symbols.Add("EURJPY");
	symbols.Add("EURGBP");
	
	
	
	LoadThis();
	
}

SentimentSnapshot* Sentiment::FindReal() {
	SentimentSnapshot* s = NULL;
	
	if (sents.GetCount())
		s = &sents.Top();
	
	return s;
}











SentimentCtrl::SentimentCtrl() {
	Add(split.SizePos());
	split << historylist << tflist << eventlist << netlist << pairpreslist << console;
	
	console.Add(comment.HSizePos().VSizePos(0, 30));
	console.Add(save.BottomPos(0, 30).HSizePos());
	
	historylist.AddColumn("Time");
	historylist.AddColumn("Tf");
	historylist.AddColumn("Comment");
	historylist.ColumnWidths("1 1 3");
	historylist <<= THISBACK(LoadHistory);
	tflist.AddColumn("Tf");
	tflist.AddColumn("Events");
	tflist.ColumnWidths("1 3");
	tflist <<= THISBACK(LoadTf);
	eventlist.AddColumn("Event");
	eventlist.AddColumn("Symbol");
	eventlist.AddColumn("Signal");
	eventlist.AddIndex();
	eventlist.AddIndex();
	eventlist.AddIndex();
	eventlist.ColumnWidths("4 2 1");
	eventlist <<= THISBACK(SetEventProfile);
	netlist.AddColumn("Net");
	netlist.AddColumn("Pressure");
	netlist.ColumnWidths("1 2");
	netlist <<= THISBACK(SetNetProfile);
	pairpreslist.AddColumn("Symbol");
	pairpreslist.AddColumn("Pressure");
	pairpreslist.ColumnWidths("1 2");
	pairpreslist <<= THISBACK(SetPairProfile);
	
	save.SetLabel("Save");
	save <<= THISBACK(Save);
}

void SentimentCtrl::Data() {
	System& sys = GetSystem();
	Sentiment& sent = GetSentiment();
	EventSystem& es = GetEventSystem();
	
	if (tflist.GetCount() == 0) {
		for(int i = 0; i < sys.GetPeriodCount(); i++) {
			tflist.Set(i, 0, sys.GetPeriodString(i));
		}
		tflist.SetCursor(0);
		
		netlist.SetLineCy(30);
		for(int i = 0; i < sys.GetNetCount(); i++) {
			netlist.Set(i, 0, "Net" + IntStr(i));
			SentPresCtrl& ctrl = net_pres_ctrl.Add();
			ctrl <<= THISBACK(SetPairPressures);
			netlist.SetCtrl(i, 1, ctrl);
		}
		
		pairpreslist.SetLineCy(30);
		for(int i = 0; i < sent.GetSymbolCount(); i++) {
			pairpreslist.Set(i, 0, sent.GetSymbol(i));
			pairpreslist.SetCtrl(i, 1, pair_pres_ctrl.Add());
		}
		
		LoadTf();
		LoadHistory();
	}
	
	
	int count = sent.GetSentimentCount();
	for(int i = 0; i < count; i++) {
		SentimentSnapshot& snap = sent.GetSentiment(i);
		
		historylist.Set(i, 0, snap.added);
		historylist.Set(i, 1, sys.GetPeriodString(snap.realtf));
		historylist.Set(i, 2, snap.comment);
	}
	historylist.SetCount(count);
	
	int his = historylist.GetCursor();
	if (count && (his < 0 || his >= count))
		historylist.SetCursor(historylist.GetCount()-1);
	
	
	LoadTf();
}

void SentimentCtrl::LoadTf() {
	Sentiment& sent = GetSentiment();
	System& sys = GetSystem();
	
	int tf = tflist.GetCursor();
	
	
	
	VectorMap<int, int> tf_evcount;
	EventSystem& es = GetEventSystem();
	int row = 0;
	for(int i = 0; i < es.GetCount(); i++) {
		Event& ev = es.Get(i);
		tf_evcount.GetAdd(ev.tf, 0)++;
		if (ev.tf != tf) continue;
		eventlist.Set(row, 0, es.GetEventString(ev.ev));
		eventlist.Set(row, 1, sys.GetSymbol(ev.sym));
		eventlist.Set(row, 2, ev.sig);
		eventlist.Set(row, 3, ev.sym);
		eventlist.Set(row, 4, ev.tf);
		eventlist.Set(row, 5, ev.ev);
		row++;
	}
	eventlist.SetCount(row);
	eventlist.SetSortColumn(1);
	
	
	int max_ev = 1;
	for(int i = 0; i < tf_evcount.GetCount(); i++)
		max_ev = max(max_ev, tf_evcount[i]);
	
	for(int i = 0; i < tflist.GetCount(); i++) {
		int count = tf_evcount.GetAdd(i, 0);
		int perc = count * 100 / max_ev;
		tflist.Set(i, 1, perc);
		tflist.SetDisplay(i, 1, Single<PercentDisplay>());
	}
	
	SetSignals();
}

void SentimentCtrl::LoadHistory() {
	Sentiment& sent = GetSentiment();
	
	
	int his = historylist.GetCursor();
	if (his == -1) return;
	
	SentimentSnapshot& snap = sent.GetSentiment(his);
	
	tflist.SetCursor(snap.realtf);
	
	
	for(int i = 0; i < netlist.GetCount() && i < snap.net_pres.GetCount(); i++) {
		netlist.Set(i, 1, snap.net_pres[i]);
	}
	
	for(int i = 0; i < pairpreslist.GetCount() && i < snap.pair_pres.GetCount(); i++) {
		pairpreslist.Set(i, 1, snap.pair_pres[i]);
	}
	
	comment.SetData(snap.comment);
}

void SentimentCtrl::Save() {
	Sentiment& sent = GetSentiment();
	
	int tf = tflist.GetCursor();
	
	SentimentSnapshot& snap = sent.AddSentiment();
	
	snap.added = GetUtcTime();
	
	snap.net_pres.SetCount(netlist.GetCount());
	for(int i = 0; i < netlist.GetCount(); i++) {
		snap.net_pres[i] = netlist.Get(i, 1);
	}
	
	snap.pair_pres.SetCount(pairpreslist.GetCount());
	for(int i = 0; i < pairpreslist.GetCount(); i++) {
		snap.pair_pres[i] = pairpreslist.Get(i, 1);
	}
	
	snap.comment = comment.GetData();
	snap.realtf = tf;
	
	sent.StoreThis();
	Data();
	historylist.SetCursor(historylist.GetCount()-1);
}

void SentimentCtrl::SetPairPressures() {
	System& sys = GetSystem();
	VectorMap<String, int> pres;
	
	for(int i = 0; i < netlist.GetCount(); i++) {
		String a = netlist.Get(i, 0);
		int p = netlist.Get(i, 1);
		
		if (p == 0) continue;
		if (p < -100 || p > 100) {
			double dp = netlist.Get(i, 1);
			if (dp < -100 || dp > 100)
				continue;
			p = dp;
		}
		
		int sym = sys.FindSymbol(a);
		const System::NetSetting& net = sys.GetSymbolNet(sym);
		for(int j = 0; j < net.symbol_ids.GetCount(); j++) {
			int sym = net.symbol_ids.GetKey(j);
			int sig = net.symbol_ids[j];
			String str = sys.GetSymbol(sym);
			pres.GetAdd(str, 0) += p * sig;
		}
	}
	
	for(int i = 0; i < pairpreslist.GetCount(); i++) {
		String s = pairpreslist.Get(i, 0);
		int p = pres.GetAdd(s, 0);
		pairpreslist.Set(i, 1, p);
	}
	
	
}

void SentimentCtrl::SetSignals() {
	Sentiment& sent = GetSentiment();
	
	SentimentSnapshot* real = GetSentiment().FindReal();
	if (real) {
		System& sys = GetSystem();
		for(int i = 0; i < sent.GetSymbolCount() && i < real->pair_pres.GetCount(); i++) {
			int j = sys.FindSymbol(sent.GetSymbol(i));
			ASSERT(j != -1);
			int pres = real->pair_pres[i];
			int sig = pres == 0 ? 0 : (pres > 0 ? +1 : -1);
			sys.SetSignal(j, sig);
		}
	}
	
}

void SentimentCtrl::SetEventProfile() {
	System& sys = GetSystem();
	int cur = eventlist.GetCursor();
	int sym = eventlist.Get(cur, 3);
	int tf = eventlist.Get(cur, 4);
	int ev = eventlist.Get(cur, 5);
	int factory = -1;
	switch (ev) {
		case EventSystem::HIHURST:		factory = sys.Find<SimpleHurstWindow>(); break;
		case EventSystem::HIMA:			factory = sys.Find<MovingAverage>(); break;
		case EventSystem::HIBB:			factory = sys.Find<BollingerBands>(); break;
		case EventSystem::NEWSAR:		factory = sys.Find<ParabolicSAR>(); break;
		case EventSystem::HIVOLAV:		factory = sys.Find<VolatilityAverage>(); break;
		case EventSystem::ANOMALY:		factory = sys.Find<Anomaly>(); break;
		case EventSystem::HICHANGE:		factory = sys.Find<DataBridge>(); break;
		case EventSystem::HIPERCH:		factory = sys.Find<PeriodicalChange>(); break;
		case EventSystem::HIVOLSLOT:	factory = sys.Find<VolatilitySlots>(); break;
		case EventSystem::HINEWS:		factory = sys.Find<Calendar>(); break;
	}
	GetOverlook().LoadSymbolProfile(sym, tf, factory);
}

void SentimentCtrl::SetNetProfile() {
	System& sys = GetSystem();
	int cur = netlist.GetCursor();
	String str = netlist.Get(cur, 0);
	GetOverlook().LoadSymbolProfile(sys.FindSymbol(str), tflist.GetCursor());
}

void SentimentCtrl::SetPairProfile() {
	System& sys = GetSystem();
	int cur = pairpreslist.GetCursor();
	String str = pairpreslist.Get(cur, 0);
	GetOverlook().LoadSymbolProfile(sys.FindSymbol(str), tflist.GetCursor());
}









void SentPresCtrl::Paint(Draw& w) {
	Size sz(GetSize());
	
	w.DrawRect(sz, White());
	
	int value = GetData();
	
	if (value < 0.0) {
		int W = -value * (sz.cx / 2) / 10;
		w.DrawRect(sz.cx / 2 - W, 0, W, sz.cy, Blue());
	} else {
		int W = value * (sz.cx / 2) / 10;
		w.DrawRect(sz.cx / 2, 0, W, sz.cy, Green());
	}
	
	w.DrawRect(sz.cx/2, 0, 1, sz.cy, GrayColor());
}

void SentPresCtrl::LeftDown(Point p, dword keyflags) {
	Size sz(GetSize());
	int value = (p.x - sz.cx / 2) * 10 / (sz.cx / 2);
	SetData(value);
	Refresh();
	WhenAction();
	SetCapture();
}

void SentPresCtrl::LeftUp(Point p, dword keyflags) {
	ReleaseCapture();
}

void SentPresCtrl::MouseMove(Point p, dword keyflags) {
	if (HasCapture()) {
		Size sz(GetSize());
		int value = (p.x - sz.cx / 2) * 10 / (sz.cx / 2);
		SetData(value);
		Refresh();
		WhenAction();
	}
}

}
