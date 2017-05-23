#include "Overlook.h"

namespace Overlook {


Overlook::Overlook()
{
	Title("Overlook");
	Icon(OverlookImg::icon());
	MinimizeBox().MaximizeBox().Sizeable();
	
	Add(droplist_split.TopPos(2, 26).HSizePos(2, 2));
	droplist_split << ctrllist << symlist << tflist << config;
	droplist_split.Horz();
	
	prev_core = NULL;
	prev_view = NULL;
	
	ctrllist <<= THISBACK(SetView);
	symlist <<= THISBACK(SetView);
	tflist <<= THISBACK(SetView);
	config.SetLabel("Configure");
	config <<= THISBACK(Configure);
	
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

void Overlook::Configure() {
	if (!prev_core) return;
	
	static One<TopWindow> tw;
	static bool save;
	save = false;
	tw.Create();
	
	tw->Title("Configure arguments");
	Button ok;
	ParentCtrl ctrl;
	Array<Label> labels;
	Array<EditDoubleSpin> edits;
	ok.SetLabel("OK");
	tw->Add(ctrl.HSizePos().VSizePos(0,30));
	tw->Add(ok.BottomPos(3,24).RightPos(3, 100));
	ok.WhenAction << [=] () {
		save = true;
		tw->Close();
	};
	int c = ctrllist.GetIndex();
	int s = symlist.GetIndex();
	int t = tflist.GetIndex();
	ArgChanger reg;
	reg.SetLoading();
	prev_core->IO(reg);
	
	Size sz(320, reg.args.GetCount()*30 + 50);
	tw->SetRect(sz);
	int xoff = sz.cx * 0.5;
	
	for(int i = 0; i < reg.args.GetCount(); i++) {
		Label& lbl = labels.Add();
		lbl.SetLabel(reg.keys[i]);
		lbl.SetAlign(ALIGN_RIGHT);
		EditDoubleSpin& edit = edits.Add();
		lbl.SetRect(4, i*30, xoff-8, 30);
		edit.SetRect(xoff, i*30, sz.cx-xoff, 30);
		edit.SetData(reg.args[i]);
		edit.WhenEnter = ok.WhenAction;
		ctrl.Add(lbl);
		ctrl.Add(edit);
	}
	
	tw->Run();
	tw.Clear();
	
	if (save) {
		// These settings does not affect the combination, so these can be applied to the current combination.
		for(int i = 0; i < reg.args.GetCount(); i++) {
			reg.args[i] = edits[i].GetData();
		}
		reg.SetStoring();
		prev_core->IO(reg);
		prev_core->ClearContent();
		prev_core->Init();
		prev_core->Refresh();
		Refresh();
	}
}

}
