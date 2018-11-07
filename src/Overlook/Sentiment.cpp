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
	symbols.Add("AUDJPY");
	symbols.Add("EURCHF");
	symbols.Add("EURJPY");
	symbols.Add("EURGBP");
	symbols.Add("EURCAD");
	symbols.Add("EURAUD");
	symbols.Add("EURNZD");
	symbols.Add("GBPCHF");
	symbols.Add("GBPJPY");
	symbols.Add("CHFJPY");
	symbols.Add("NZDJPY");
	symbols.Add("CADJPY");
	symbols.Add("GBPCAD");
	symbols.Add("CADCHF");
	symbols.Add("NZDCAD");
	symbols.Add("AUDCAD");
	symbols.Add("GBPAUD");
	symbols.Add("AUDCHF");
	symbols.Add("AUDCHF");
	symbols.Add("AUDNZD");
	symbols.Add("GBPNZD");
	
	currencies.Add("USD");
	currencies.Add("EUR");
	currencies.Add("GBP");
	currencies.Add("JPY");
	currencies.Add("CAD");
	currencies.Add("AUD");
	currencies.Add("NZD");
	currencies.Add("CHF");
	
	LoadThis();
	
}

void Sentiment::Init() {
	
}

void Sentiment::Start() {
	
	SetSignals();
	
}

void Sentiment::SetSignals() {
	MetaTrader& mt = GetMetaTrader();
	System& sys = GetSystem();
	
	One<SentimentSnapshot> sent;
	sent.Create();
	int changes = 0;
	
	
	// Simple trailing stop
	double tplimit = 0.3;
	if (sents.GetCount())
		tplimit = sents.Top().tplimit;
	double balance = mt.AccountBalance();
	double equity = mt.AccountEquity();
	double profit = equity - balance;
	if (enable_takeprofit && profit >= balance * tplimit) {
		sent->pair_pres.SetCount(symbols.GetCount(), 0);
		sent->cur_pres.SetCount(sys.GetCurrencyCount(), 0);
		sent->comment = "Auto take-profit";
		sent->added = GetUtcTime();
		sent->equity = GetMetaTrader().AccountEquity();
		changes++;
	}
	
	if (changes && sents.IsEmpty()) {
		sents.Add(sent.Detach());
		StoreThis();
	}
	
	if (sents.GetCount()) {
		if (changes && !sents.Top().IsPairEqual(*sent)) {
			sents.Add(sent.Detach());
			StoreThis();
		}
		
		SentimentSnapshot& real = sents.Top();
		System& sys = GetSystem();
		sys.SetFreemarginLevel(real.fmlevel);
		System::NetSetting& actnet = sys.GetNet(sys.GetNetCount() - 1);
		for(int i = 0; i < GetSymbolCount() && i < real.pair_pres.GetCount(); i++) {
			String sym = GetSymbol(i);
			int j = sys.FindSymbol(sym);
			ASSERT(j != -1);
			int pres = real.pair_pres[i];
			int sig = pres == 0 ? 0 : (pres > 0 ? +1 : -1);
			sys.SetSignal(j, pres);
			actnet.Assign(sym, sig);
		}
	}
}












SentimentCtrl::SentimentCtrl() {
	Add(split.SizePos());
	split << historylist << curpreslist << pairpreslist << errlist << console;
	
	console.Add(comment.HSizePos().VSizePos(0, 60));
	console.Add(fmlevel.BottomPos(30, 30).HSizePos());
	console.Add(save.BottomPos(0, 30).HSizePos());
	
	historylist.AddColumn("Time");
	historylist.AddColumn("Comment");
	historylist.AddColumn("Profit");
	historylist.ColumnWidths("2 3 1");
	historylist <<= THISBACK(LoadHistory);
	curpreslist.AddColumn("Symbol");
	curpreslist.AddColumn("Pressure");
	curpreslist.ColumnWidths("1 2");
	curpreslist <<= THISBACK(SetCurProfile);
	pairpreslist.AddColumn("Symbol");
	pairpreslist.AddColumn("Pressure");
	pairpreslist.ColumnWidths("1 2");
	pairpreslist <<= THISBACK(SetPairProfile);
	errlist.AddColumn("Message");
	
	fmlevel.SetData(0);
	
	save.SetLabel("Save");
	save <<= THISBACK(Save);
}

void SentimentCtrl::Data() {
	Sentiment& sent = GetSentiment();
	
	if (curpreslist.GetCount() == 0) {
		curpreslist.SetLineCy(30);
		for(int i = 0; i < sent.GetCurrencyCount(); i++) {
			curpreslist.Set(i, 0, sent.GetCurrency(i));
			SentPresCtrl& ctrl = cur_pres_ctrl.Add();
			ctrl <<= THISBACK(SetCurPairPressures);
			curpreslist.SetCtrl(i, 1, ctrl);
		}
		
		
		pairpreslist.SetLineCy(30);
		for(int i = 0; i < sent.GetSymbolCount(); i++) {
			pairpreslist.Set(i, 0, sent.GetSymbol(i));
			pairpreslist.SetCtrl(i, 1, pair_pres_ctrl.Add());
		}
		
		LoadHistory();
	}
	
	
	int count = sent.GetSentimentCount();
	double prev_equity = 0;
	for(int i = 0; i < count; i++) {
		SentimentSnapshot& snap = sent.GetSentiment(i);
		
		historylist.Set(i, 0, snap.added);
		historylist.Set(i, 1, snap.comment);
		if (i) historylist.Set(i - 1, 2, snap.equity - prev_equity);
		prev_equity = snap.equity;
	}
	historylist.SetCount(count);
	
	int his = historylist.GetCursor();
	if (count && (his < 0 || his >= count))
		historylist.SetCursor(historylist.GetCount()-1);
	
	
	SentimentSnapshot snap;
	snap.cur_pres.SetCount(curpreslist.GetCount(), 0);
	snap.pair_pres.SetCount(pairpreslist.GetCount());
	for(int i = 0; i < pairpreslist.GetCount(); i++)
		snap.pair_pres[i] = pairpreslist.Get(i, 1);
	Index<EventError> errors;
	GetEventSystem().GetErrorList(snap, errors);
	for(int i = 0; i < errors.GetCount(); i++) {
		const EventError& e = errors[i];
		String msg;
		switch (e.level) {
			case 0: msg += "info: "; break;
			case 1: msg += "warning: "; break;
			case 2: msg += "error: "; break;
		}
		msg += e.msg;
		errlist.Set(i, 0, msg);
	}
	errlist.SetCount(errors.GetCount());
}


void SentimentCtrl::LoadHistory() {
	Sentiment& sent = GetSentiment();
	
	
	int his = historylist.GetCursor();
	if (his == -1) return;
	
	SentimentSnapshot& snap = sent.GetSentiment(his);
	
	
	for(int i = 0; i < curpreslist.GetCount() && i < snap.cur_pres.GetCount(); i++) {
		curpreslist.Set(i, 1, snap.cur_pres[i]);
	}
	
	for(int i = 0; i < pairpreslist.GetCount() && i < snap.pair_pres.GetCount(); i++) {
		pairpreslist.Set(i, 1, snap.pair_pres[i]);
	}
	
	comment.SetData(snap.comment);
	
	fmlevel.SetData(snap.fmlevel);
}

void SentimentCtrl::Save() {
	Sentiment& sent = GetSentiment();
	
	SentimentSnapshot& snap = sent.AddSentiment();
	
	snap.added = GetUtcTime();
	
	snap.cur_pres.SetCount(curpreslist.GetCount());
	for(int i = 0; i < curpreslist.GetCount(); i++) {
		snap.cur_pres[i] = curpreslist.Get(i, 1);
	}
	
	snap.pair_pres.SetCount(pairpreslist.GetCount());
	for(int i = 0; i < pairpreslist.GetCount(); i++) {
		snap.pair_pres[i] = pairpreslist.Get(i, 1);
	}
	
	snap.comment = comment.GetData();
	snap.fmlevel = fmlevel.GetData();
	snap.equity = GetMetaTrader().AccountEquity();
	
	sent.StoreThis();
	Data();
	historylist.SetCursor(historylist.GetCount()-1);
}

void SentimentCtrl::SetCurPairPressures() {
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

void SentimentCtrl::SetCurProfile() {
	System& sys = GetSystem();
	int cur = curpreslist.GetCursor();
	String str = curpreslist.Get(cur, 0);
	GetOverlook().LoadSymbolProfile(sys.FindSymbol(str), 0);
}

void SentimentCtrl::SetPairProfile() {
	System& sys = GetSystem();
	int cur = pairpreslist.GetCursor();
	String str = pairpreslist.Get(cur, 0);
	GetOverlook().LoadSymbolProfile(sys.FindSymbol(str), 0);
}









void SentPresCtrl::Paint(Draw& w) {
	Size sz(GetSize());
	
	w.DrawRect(sz, White());
	
	int value = GetData();
	
	if (value < 0.0) {
		int W = -value * (sz.cx / 2) / SIGNALSCALE;
		w.DrawRect(sz.cx / 2 - W, 0, W, sz.cy, Blue());
	} else {
		int W = value * (sz.cx / 2) / SIGNALSCALE;
		w.DrawRect(sz.cx / 2, 0, W, sz.cy, Green());
	}
	
	w.DrawRect(sz.cx/2, 0, 1, sz.cy, GrayColor());
}

void SentPresCtrl::LeftDown(Point p, dword keyflags) {
	Size sz(GetSize());
	int value = (p.x - sz.cx / 2) * SIGNALSCALE / (sz.cx / 2);
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
		int value = (p.x - sz.cx / 2) * SIGNALSCALE / (sz.cx / 2);
		SetData(value);
		Refresh();
		WhenAction();
	}
}

}
