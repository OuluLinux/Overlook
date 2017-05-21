#include "Overlook.h"

namespace Overlook {


Overlook::Overlook()
{
	Title("Overlook");
	Icon(OverlookImg::icon());
	MinimizeBox().MaximizeBox().Sizeable();
	
	Add(droplist_split.TopPos(2, 26).HSizePos(2, 2));
	droplist_split << ctrllist << symlist << tflist;
	droplist_split.Horz();
	
	prev_core = NULL;
	prev_view = NULL;
	
	ctrllist <<= THISBACK(SetView);
	symlist <<= THISBACK(SetView);
	tflist <<= THISBACK(SetView);
	
	PostCallback(THISBACK(Refresher));
}

Overlook::~Overlook() {
	
}

void Overlook::Refresher() {
	if (prev_view)
		prev_view->RefreshData();
	tc.Set(1000, THISBACK(PostRefresher));
}

void Overlook::Init() {
	user.Init();
	optimizer.Init();
	
	BaseSystem& bs = user.GetBaseSystem();
	
	// Init gui
	for(int i = 0; i < bs.GetPeriodCount(); i++)
		tflist.Add(bs.GetPeriodString(i));
	for(int i = 0; i < bs.GetSymbolCount(); i++)
		symlist.Add(bs.GetSymbol(i));
	for(int i = 0; i < Factory::GetCtrlFactories().GetCount(); i++)
		ctrllist.Add(Factory::GetCtrlFactories()[i].a);
	
	tflist.SetIndex(tflist.GetCount()-2); // TODO: clear these development values
	symlist.SetIndex(70);
	ctrllist.SetIndex(1);
	
	PostCallback(THISBACK(SetView));
}

void Overlook::Deinit() {
	
}

void Overlook::SetView() {
	int c = ctrllist.GetIndex();
	int s = symlist.GetIndex();
	int t = tflist.GetIndex();
	
	if (prev_view) {
		RemoveChild(prev_view);
		delete prev_view;
		prev_view = NULL;
		prev_core = NULL;
	}
	
	Core* core;
	CustomCtrl* view;
	view = Factory::GetCtrlFactories()[c].c();
	
	// Get core-object by creating job-queue for factory/symbol/timeframe combination
	user.CreateSingle(c, s, t);
	user.Process();
	ASSERT(user.GetJobCount());
	JobItem& ji = user.GetJob(user.GetJobCount()-1);
	ASSERT(ji.core);
	core = ji.core;
	
	view->core = core;
	view->SetSymbol(s);
	view->SetTf(t);
	
	
	view->Init(core);
	
	view->RefreshData();
	
	Add(view->VSizePos(30).HSizePos());
	prev_view = view;
	prev_core = core;
}

}
