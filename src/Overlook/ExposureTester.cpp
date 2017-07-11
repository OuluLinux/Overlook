#include "Overlook.h"


namespace Overlook {

ExposureTester::ExposureTester() {
	Add(hsplit.SizePos());
	hsplit << expctrl << brokerctrl;
	hsplit.SetPos(2000);
	hsplit.Horz();
	
	expctrl.Add(siglist.HSizePos().VSizePos());
	//expctrl.Add(next.BottomPos(30,30).HSizePos());
	//expctrl.Add(reset.BottomPos(0,30).HSizePos());
	next.SetLabel("Next");
	reset.SetLabel("Reset");
	next <<= THISBACK(Next);
	reset <<= THISBACK(Reset);
	
	siglist.AddColumn("Symbol");
	siglist.AddColumn("Signal");
	
	
}

void ExposureTester::Init() {
	broker.Init();
	broker.Brokerage::operator=(GetMetaTrader());
	
	brokerctrl.SetBroker(broker);
	brokerctrl.Init();
	
	for(int i = 0; i < broker.GetSymbolCount(); i++) {
		siglist.Add(broker.GetSymbol(i).name, 0.0);
		EditIntSpin& spin = edits.Add();
		siglist.SetCtrl(i, 1, spin);
		spin <<= THISBACK(Signal);
	}
	
	Signal();
}

void ExposureTester::Signal() {
	for(int i = 0; i < broker.GetSymbolCount(); i++) {
		broker.SetSignal(i, edits[i].GetData());
	}
	broker.SignalOrders();
	broker.ForwardExposure();
	brokerctrl.Data();
}

void ExposureTester::Next() {
	
	
	
}

void ExposureTester::Reset() {
	
	
	
}

void ExposureTester::Data() {
	
	
	
}

}
