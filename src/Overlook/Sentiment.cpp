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
	
	if (sents.IsEmpty()) {
		System& sys = GetSystem();
		
		sents.SetCount(sys.GetPeriodCount());
	}
	
}

SentimentSnapshot* Sentiment::FindReal() {
	SentimentSnapshot* s = NULL;
	
	Time last_added(1970,1,1);
	for(int i = 0; i < sents.GetCount(); i++) {
		Array<SentimentSnapshot>& tfsents = sents[i];
		if (!tfsents.IsEmpty()) {
			SentimentSnapshot& sent = tfsents.Top();
			if (sent.added > last_added) {
				last_added = sent.added;
				s = &sent;
			}
		}
	}
	
	if (s) {
		Array<SentimentSnapshot>& tfsents = sents[s->realtf];
		if (tfsents.IsEmpty())
			s = NULL;
		else
			s = &tfsents.Top();
	}
	
	return s;
}











SentimentCtrl::SentimentCtrl() {
	Add(split.SizePos());
	split << tflist << historylist << curevent << pairevent << curpreslist << pairpreslist << console;
	
	curevent.Add(cureventsym.TopPos(0, 30).HSizePos());
	curevent.Add(cureventlist.VSizePos(30, 0).HSizePos());
	pairevent.Add(paireventsym.TopPos(0, 30).HSizePos());
	pairevent.Add(paireventlist.VSizePos(30, 0).HSizePos());
	
	console.Add(comment.HSizePos().VSizePos(0, 60));
	console.Add(save.BottomPos(0, 30).HSizePos());
	console.Add(realtf.BottomPos(30, 30).HSizePos());
	
	tflist.AddColumn("Tf");
	tflist <<= THISBACK(LoadTf);
	historylist.AddColumn("Time");
	historylist.AddColumn("Correctness");
	historylist <<= THISBACK(LoadHistory);
	cureventlist.AddIndex();
	cureventlist.AddColumn("Event");
	cureventlist.AddColumn("Probability");
	cureventlist.AddColumn("Enabled");
	cureventlist.ColumnWidths("4 2 1");
	paireventlist.AddIndex();
	paireventlist.AddColumn("Event");
	paireventlist.AddColumn("Probability");
	paireventlist.AddColumn("Enabled");
	paireventlist.ColumnWidths("4 2 1");
	curpreslist.AddColumn("Symbol");
	curpreslist.AddColumn("Pressure");
	curpreslist.ColumnWidths("1 2");
	curpreslist <<= THISBACK(SetCurrencyProfile);
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
	
	if (tflist.GetCount() == 0) {
		for(int i = 0; i < sys.GetPeriodCount(); i++) {
			tflist.Set(i, 0, sys.GetPeriod(i));
			realtf.Add(sys.GetPeriod(i));
		}
		tflist.SetCursor(0);
		realtf.SetIndex(0);
		
		curpreslist.SetLineCy(30);
		cureventsym.Add("Any");
		for(int i = 0; i < sys.GetCurrencyCount(); i++) {
			cureventsym.Add(sys.GetCurrency(i));
			curpreslist.Set(i, 0, sys.GetCurrency(i));
			SentPresCtrl& ctrl = cur_pres_ctrl.Add();
			ctrl <<= THISBACK(SetPairPressures);
			curpreslist.SetCtrl(i, 1, ctrl);
		}
		cureventsym.SetIndex(0);
		
		pairpreslist.SetLineCy(30);
		paireventsym.Add("Any");
		for(int i = 0; i < sent.GetSymbolCount(); i++) {
			paireventsym.Add(sent.GetSymbol(i));
			pairpreslist.Set(i, 0, sent.GetSymbol(i));
			pairpreslist.SetCtrl(i, 1, pair_pres_ctrl.Add());
		}
		paireventsym.SetIndex(0);
		
		LoadTf();
		LoadHistory();
	}
	
}

void SentimentCtrl::LoadTf() {
	Sentiment& sent = GetSentiment();
	
	int tf = tflist.GetCursor();
	
	int count = sent.GetSentimentCount(tf);
	for(int i = 0; i < count; i++) {
		SentimentSnapshot& snap = sent.GetSentiment(tf, i);
		
		historylist.Set(i, 0, snap.added);
		historylist.Set(i, 1, snap.correctness);
	}
	historylist.SetCount(count);
	
	//int his = historylist.GetCursor();
	//if (his < 0 || his >= count)
	historylist.SetCursor(historylist.GetCount()-1);
	
	
	
	EventSystem& es = GetEventSystem();
	
	
	
	
	
	SetSignals();
}

void SentimentCtrl::LoadHistory() {
	Sentiment& sent = GetSentiment();
	
	int tf = tflist.GetCursor();
	int his = historylist.GetCursor();
	if (his == -1) return;
	
	SentimentSnapshot& snap = sent.GetSentiment(tf, his);
	
	
	for(int i = 0; i < cureventlist.GetCount(); i++) {
		int ev_id = cureventlist.Get(i, 0);
		bool ena = snap.cur_events.Find(ev_id) != -1;
		cureventlist.Set(i, 3, ena);
	}
	
	for(int i = 0; i < paireventlist.GetCount(); i++) {
		int ev_id = paireventlist.Get(i, 0);
		bool ena = snap.pair_events.Find(ev_id) != -1;
		paireventlist.Set(i, 3, ena);
	}
	
	for(int i = 0; i < curpreslist.GetCount() && i < snap.cur_pres.GetCount(); i++) {
		curpreslist.Set(i, 1, snap.cur_pres[i]);
	}
	
	for(int i = 0; i < pairpreslist.GetCount() && i < snap.pair_pres.GetCount(); i++) {
		pairpreslist.Set(i, 1, snap.pair_pres[i]);
	}
	
	comment.SetData(snap.comment);
	realtf.SetIndex(snap.realtf);
}

void SentimentCtrl::Save() {
	Sentiment& sent = GetSentiment();
	
	int tf = tflist.GetCursor();
	
	SentimentSnapshot& snap = sent.AddSentiment(tf);
	
	snap.added = GetUtcTime();
	
	for(int i = 0; i < cureventlist.GetCount(); i++) {
		bool ena = cureventlist.Get(i, 3);
		if (ena) {
			int ev_id = cureventlist.Get(i, 0);
			snap.cur_events.Add(ev_id);
		}
	}
	
	for(int i = 0; i < paireventlist.GetCount(); i++) {
		bool ena = paireventlist.Get(i, 3);
		if (ena) {
			int ev_id = paireventlist.Get(i, 0);
			snap.pair_events.Add(ev_id);
		}
	}
	
	snap.cur_pres.SetCount(curpreslist.GetCount());
	for(int i = 0; i < curpreslist.GetCount(); i++) {
		snap.cur_pres[i] = curpreslist.Get(i, 1);
	}
	
	snap.pair_pres.SetCount(pairpreslist.GetCount());
	for(int i = 0; i < pairpreslist.GetCount(); i++) {
		snap.pair_pres[i] = pairpreslist.Get(i, 1);
	}
	
	snap.comment = comment.GetData();
	snap.realtf = realtf.GetIndex();
	
	sent.StoreThis();
	LoadTf();
	
}

void SentimentCtrl::SetPairPressures() {
	VectorMap<String, int> pres;
	
	for(int i = 0; i < curpreslist.GetCount(); i++) {
		String a = curpreslist.Get(i, 0);
		int p = curpreslist.Get(i, 1);
		pres.Add(a, p);
	}
	
	for(int i = 0; i < pairpreslist.GetCount(); i++) {
		String s = pairpreslist.Get(i, 0);
		String a = s.Left(3);
		String b = s.Right(3);
		int p = pres.GetAdd(a, 0) - pres.GetAdd(b, 0);
		pairpreslist.Set(i, 1, p);
	}
	
	
}

void SentimentCtrl::SetSignals() {
	Sentiment& sent = GetSentiment();
	
	SentimentSnapshot* real = GetSentiment().FindReal();
	if (real) {
		System& sys = GetSystem();
		for(int i = 0; i < sent.GetSymbolCount(); i++) {
			int j = sys.FindSymbol(sent.GetSymbol(i));
			ASSERT(j != -1);
			int pres = real->pair_pres[i];
			int sig = pres == 0 ? 0 : (pres > 0 ? +1 : -1);
			sys.SetSignal(j, sig);
		}
	}
	
}

void SentimentCtrl::SetCurrencyProfile() {
	System& sys = GetSystem();
	int cur = curpreslist.GetCursor();
	String str = curpreslist.Get(cur, 0);
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
