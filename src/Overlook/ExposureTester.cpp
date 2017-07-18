#include "Overlook.h"


namespace Overlook {

ExposureTester::ExposureTester() {
	Add(hsplit.SizePos());
	hsplit << expctrl << brokerctrl;
	hsplit.SetPos(2000);
	hsplit.Horz();
	
	expctrl.Add(siglist.HSizePos().VSizePos());
	
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

}
