#include "Overlook.h"

namespace Overlook {

RealtimeCtrl::RealtimeCtrl() {
	System& sys = GetSystem();
	
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << brokerctrl << journal;
	hsplit.SetPos(8000);
	
	
	journal.AddColumn("Time");
	journal.AddColumn("Level");
	journal.AddColumn("Message");
	journal.ColumnWidths("3 1 3");
	
	
	brokerctrl.SetBroker(GetMetaTrader());
	
	sys.WhenRealtimeUpdate << THISBACK(PostData);
}

RealtimeCtrl::~RealtimeCtrl() {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	mt.WhenInfo.Clear();
	mt.WhenError.Clear();
	
	sys.WhenInfo.Clear();
	sys.WhenError.Clear();
	sys.WhenRealtimeUpdate.Clear();
}

void RealtimeCtrl::AddMessage(String time, String level, String msg) {
	lock.Enter();
	messages.Add(Msg(time, level, msg));
	lock.Leave();
}

void RealtimeCtrl::Info(String msg) {
	AddMessage(
		Format("%", GetSysTime()),
		"Info",
		msg);
}

void RealtimeCtrl::Error(String msg) {
	AddMessage(
		Format("%", GetSysTime()),
		"Error",
		msg);
}

void RealtimeCtrl::Data() {
	MetaTrader& mt = GetMetaTrader();
	mt.ForwardExposure();
	
	brokerctrl.Data();
	
	lock.Enter();
	int count = Upp::min(messages.GetCount(), 20);
	for(int i = 0; i < messages.GetCount(); i++) {
		int j = messages.GetCount() - 1 - i;
		const Msg& m = messages[j];
		journal.Set(i, 0, m.a);
		journal.Set(i, 1, m.b);
		journal.Set(i, 2, m.c);
	}
	lock.Leave();
}

void RealtimeCtrl::Init() {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	mt.WhenInfo  << THISBACK(Info);
	mt.WhenError << THISBACK(Error);
	
	sys.WhenInfo  << THISBACK(Info);
	sys.WhenError << THISBACK(Error);
}

}
