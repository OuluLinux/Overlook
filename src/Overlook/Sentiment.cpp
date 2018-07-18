#include "Overlook.h"


namespace Overlook {

Sentiment::Sentiment() {
	
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
	cureventlist.AddColumn("Event");
	cureventlist.AddColumn("Probability");
	cureventlist.AddColumn("Enabled");
	cureventlist.ColumnWidths("4 2 1");
	paireventlist.AddColumn("Event");
	paireventlist.AddColumn("Probability");
	paireventlist.AddColumn("Enabled");
	paireventlist.ColumnWidths("4 2 1");
	curpreslist.AddColumn("Symbol");
	curpreslist.AddColumn("Pressure");
	curpreslist.ColumnWidths("1 2");
	pairpreslist.AddColumn("Symbol");
	pairpreslist.AddColumn("Pressure");
	pairpreslist.ColumnWidths("1 2");
	
	save.SetLabel("Save");
	
}

void SentimentCtrl::Data() {
	System& sys = GetSystem();
	
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
			curpreslist.SetCtrl(i, 1, cur_pres_ctrl.Add());
		}
		cureventsym.SetIndex(0);
		
		pairpreslist.SetLineCy(30);
		paireventsym.Add("Any");
		for(int i = 0; i < sys.GetNormalSymbolCount(); i++) {
			paireventsym.Add(sys.GetSymbol(i));
			pairpreslist.Set(i, 0, sys.GetSymbol(i));
			pairpreslist.SetCtrl(i, 1, pair_pres_ctrl.Add());
		}
		paireventsym.SetIndex(0);
		
		LoadTf();
	}
	
}

void SentimentCtrl::LoadTf() {
	Sentiment& sent = GetSentiment();
	
	int tf = tflist.GetCursor();
	
	
	
}








void SentPresCtrl::Paint(Draw& w) {
	Size sz(GetSize());
	Value value = GetData();
	w.DrawRect(sz, Red());
}

}
