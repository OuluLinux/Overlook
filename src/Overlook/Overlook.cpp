#include "Overlook.h"

namespace Overlook {

LoaderWindow::LoaderWindow(Overlook& ol) {
	NoCloseBox();
	this->ol = &ol;
	
	ImageDraw id(64, 64);
	id.DrawRect(0,0,64,64,Black());
	id.DrawText(0,0, "O", Arial(64), White());
	Icon(id);
	
	Title("Loading Overlook");
	Add(lbl.HSizePos(4, 4).TopPos(3, 24));
	Add(prog.HSizePos(4, 4).TopPos(33, 12));
	Add(sub.HSizePos(4, 4).TopPos(33+15, 12));
	Add(subsub.HSizePos(4, 4).TopPos(33+30, 12));
	SetRect(0, 0, 400, 80);
	prog.Set(0, 1);
	sub.Set(0, 1);
	subsub.Set(0, 1);
	ret_value = 0;
}

void LoaderWindow::Progress(int actual, int total, String label) {
	LOG("Progress " << actual << "/" << total);
	prog.Set(actual, total);
	sub.Set(0, 1);
	subsub.Set(0, 1);
	this->label = label;
	lbl.SetLabel(label);
}

void LoaderWindow::SubProgress(int actual, int total) {
	sub.Set(actual, total);
	subsub.Set(0, 1);
	lbl.SetLabel(label + " " + IntStr(actual) + "/" + IntStr(total));
}

void LoaderWindow::SubSubProgress(int actual, int total) {
	subsub.Set(actual, total);
}










Overlook::Overlook() :
	trainer(sys),
	trainerctrl(trainer),
	resultctrl(trainer)
{
	Title("Overlook");
	Icon(OverlookImg::icon());
	MinimizeBox().MaximizeBox().Sizeable();
	
	Add(tabs.SizePos());
	tabs.Add(visins);
	tabs.Add(visins, "Traditional");
	tabs.Add(exposurectrl);
	tabs.Add(exposurectrl, "Exposure Tester");
	tabs.Add(trainerctrl);
	tabs.Add(trainerctrl, "Trainer");
	tabs.Add(resultctrl);
	tabs.Add(resultctrl, "Results");
	tabs.Add(rt_ctrl);
	tabs.Add(rt_ctrl, "Real-Time");
	tabs.WhenSet << THISBACK1(Data, false);
	
	
	visins.Add(droplist_split.TopPos(2, 26).HSizePos(2, 2));
	droplist_split << ctrllist << symlist << tflist << config;
	droplist_split.Horz();
	prev_core = NULL;
	prev_view = NULL;
	ctrllist <<= THISBACK(SetView);
	symlist <<= THISBACK(SetView);
	tflist <<= THISBACK(SetView);
	config.SetLabel("Configure");
	config <<= THISBACK(Configure);
	
	rt_ctrl.SetBroker(GetMetaTrader());
	
	PostCallback(THISBACK(Refresher));
}

Overlook::~Overlook() {
	if (prev_view) {
		visins.RemoveChild(prev_view);
		delete prev_view;
		prev_view = NULL;
	}
}

void Overlook::Refresher() {
	Data(true);
	int tab = tabs.Get();
	if (tab == 2) {
		PostRefresher();
	}
	else {
		tc.Set(1000, THISBACK(PostRefresher));
	}
}

void Overlook::Data(bool periodic) {
	int tab = tabs.Get();
	if (tab == 0) {
		if (prev_view)
			prev_view->Data();
	}
	else if (tab == 1) {
		exposurectrl.Data();
	}
	else if (tab == 2) {
		trainerctrl.Data();
	}
	else if (tab == 3) {
		resultctrl.Data();
	}
	else if (tab == 4) {
		rt_ctrl.Data();
	}
}

void Overlook::Init() {
	sys.Init();
	rt_ctrl.Init();
	exposurectrl.Init();
	trainer.Init();
	
	
	// Init gui
	for(int i = 0; i < sys.GetPeriodCount(); i++)
		tflist.Add(sys.GetPeriodString(i));
	for(int i = 0; i < sys.GetTotalSymbolCount(); i++)
		symlist.Add(sys.GetSymbol(i));
	for(int i = 0; i < System::GetCtrlFactories().GetCount(); i++)
		ctrllist.Add(System::GetCtrlFactories()[i].a);
	
	tflist.SetIndex(tflist.GetCount()-2); // TODO: clear these development values
	symlist.SetIndex(70);
	ctrllist.SetIndex(0);
	
	PostCallback(THISBACK(SetView));
}

void Overlook::Load() {
	loader = new LoaderWindow(*this);
	Thread::Start(THISBACK(Loader));
	loader->Run();
	loader.Clear();
}

void Overlook::Loader() {
	loader->PostProgress(0, 4, "Creating work queue");
	sys.WhenProgress = callback(&*loader, &LoaderWindow::PostSubProgress);
	sys.WhenSubProgress = callback(&*loader, &LoaderWindow::PostSubSubProgress);
	trainer.RefreshWorkQueue();
	
	loader->PostProgress(1, 4, "Processing data");
	trainer.ProcessWorkQueue();
	
	loader->PostProgress(2, 4, "Finding value buffers");
	trainer.ResetValueBuffers();
	
	loader->PostProgress(3, 4, "Reseting iterators");
	trainer.InitThreads();
	trainer.ResetIterators();
	
	sys.WhenProgress.Clear();
	sys.WhenSubProgress.Clear();
	loader->PostClose();
}

void Overlook::Start() {
	sys.Start();
	trainer.Start();
}

void Overlook::Deinit() {
	trainer.Stop();
	sys.Stop();
}

void Overlook::SetView() {
	int c = ctrllist.GetIndex();
	int s = symlist.GetIndex();
	int t = tflist.GetIndex();
	
	if (prev_view) {
		visins.RemoveChild(prev_view);
		delete prev_view;
		prev_view = NULL;
		prev_core = NULL;
	}
	
	Core* core;
	CustomCtrl* view;
	view = System::GetCtrlFactories()[c].c();
	
	core = sys.CreateSingle(c, s, t);
	ASSERT(core);
	
	view->core = core;
	view->SetSymbol(s);
	view->SetTf(t);
	
	
	view->Init(core);
	
	view->Data();
	
	visins.Add(view->VSizePos(30).HSizePos());
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
		prev_core->InitAll();
		prev_core->Refresh();
		Refresh();
	}
}

}
