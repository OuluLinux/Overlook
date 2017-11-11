#include "Overlook.h"

namespace Overlook {

Navigator::Navigator() {
	Add(tree.VSizePos().HSizePos());
	
	FillTree(tree);
	
	tree <<= THISBACK(SelectTree);
}

void Navigator::FillTree(TreeCtrl &tree) 
{
	Vector<int> parent, parent2;
	parent.Add(0);
	tree.SetRoot(OverlookImg::icon(), "Overlook");
	
	accounts		= tree.Add(0, OverlookImg::accounts(), "Accounts");
	indicators		= tree.Add(0, OverlookImg::indicators(), "Indicators");
	expertadvisors	= tree.Add(0, OverlookImg::expertadvisors(), "Expert Advisors");
	
}

void Navigator::Init() {
	Data();
}

void Navigator::Data() {
	MetaTrader& mt = GetMetaTrader();
	
	
	tree.RemoveChildren(accounts);
	String server = mt.AccountServer();
	int server_node = tree.Add(accounts, OverlookImg::servers(), server);
	String id = IntStr(mt.AccountNumber()) + ": " + mt.AccountName();
	tree.Add(server_node, OverlookImg::account(), id);
	
	
	tree.RemoveChildren(indicators);
	int count = System::CtrlFactories().GetCount();
	for(int i = 0; i < count; i++) {
		tree.Add(indicators, OverlookImg::indicator(), System::CtrlFactories()[i].a);
	}
	
	
	tree.RemoveChildren(expertadvisors);
	count = System::ExpertAdvisorFactories().GetCount();
	for(int i = 0; i < count; i++) {
		int id = System::ExpertAdvisorFactories()[i];
		tree.Add(expertadvisors, OverlookImg::expertadvisors(), System::CtrlFactories()[id].a);
	}
	
	
	tree.OpenDeep(0);
}

void Navigator::SelectTree() {
	int i = tree.GetCursor();
	int parent = tree.GetParent(i);
	if (parent == indicators) {
		int pos = tree.GetChildIndex(indicators, i);
		WhenIndicator(pos);
	}
	else if (parent == expertadvisors) {
		int pos = tree.GetChildIndex(expertadvisors, i);
		WhenExpertAdvisor(pos);
	}
}

}
