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
	BaseSystem& bs = GetBaseSystem();
	
	// Init gui
	for(int i = 0; i < bs.GetPeriodCount(); i++)
		tflist.Add(bs.GetPeriodString(i));
	for(int i = 0; i < bs.GetSymbolCount(); i++)
		symlist.Add(bs.GetSymbol(i));
	for(int i = 0; i < Factory::GetCtrlFactories().GetCount(); i++)
		ctrllist.Add(Factory::GetCtrlFactories()[i].a);
	for(int i = 0; i < Factory::GetPipeFactories().GetCount(); i++)
		ctrllist.Add(Factory::GetPipeFactories()[i].a);
	
	tflist.SetIndex(0);
	symlist.SetIndex(0);
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
		prev_view = NULL;
		prev_core = NULL;
	}
	
	Core* core;
	CustomCtrl* view;
	if (c < Factory::GetCtrlFactories().GetCount()) {
		view = Factory::GetCtrlFactories()[c].c();
		core = NULL;
	} else {
		int fac_id = c - Factory::GetCtrlFactories().GetCount();
		view = Factory::GetPipeFactories()[fac_id].c();
		Prioritizer& prio = Factory::GetCore<Prioritizer>();
		core = &prio.GetPipe(fac_id, s, t);
	}
	
	if (!view->IsInited()) {
		view->SetSymbol(s);
		view->SetTf(t);
		
		
		view->Init(core);
		
		
		view->SetInited();
	}
	
	view->RefreshData();
	
	Add(view->VSizePos(30).HSizePos());
	prev_view = view;
	prev_core = core;
}

}
