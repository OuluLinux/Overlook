#include "Overlook.h"

namespace Overlook {

GraphGroupCtrl::GraphGroupCtrl() {
	right_offset = false;
	keep_at_end = true;
	tf = -1;
	
	Add(split.VSizePos().HSizePos());
	
	split.Vert();
	
	div = 8;
	shift = 0;
}

void GraphGroupCtrl::Init(Core* src) {
	if (!src) return;
	
	core = src;
	bardata = 0;
	tf = -1;
	
	System& sys = GetSystem();
	title = sys.GetSymbol(src->GetSymbol()) + ", " + sys.GetPeriodString(src->GetTf());
	Title(title);
	
	SetGraph(src);
}

void GraphGroupCtrl::ContextMenu(Bar& bar) {
	bar.Add("Settings", THISBACK(Settings));
}

String GraphGroupCtrl::GetTitle() {
	return title;
}

GraphGroupCtrl& GraphGroupCtrl::SetTimeframe(int tf_id) {
	this->tf = tf_id;
	
	Core* core = GetSystem().CreateSingle(indi, symbol, tf);
	if (core)
		Init(core);
	
	return *this;
}

GraphGroupCtrl& GraphGroupCtrl::SetIndicator(int indi) {
	this->indi = indi;
	
	Core* core = GetSystem().CreateSingle(indi, symbol, tf);
	if (core)
		Init(core);
	
	return *this;
}

void GraphGroupCtrl::Start() {
	
}

GraphCtrl& GraphGroupCtrl::AddGraph(Core* src) {
	GraphCtrl& g = graphs.Add();
	g.WhenTimeValueTool = THISBACK(SetTimeValueTool);
	g.WhenMouseMove = THISBACK(GraphMouseMove);
	g.SetRightOffset(right_offset);
	g.SetGraphGroupCtrl(*this);
	g.AddSource(src);
	split << g;
	return g;
}

void GraphGroupCtrl::SetGraph(Core* src) {
	ASSERT(src);
	tf = src->GetTf();
	System& bs = src->GetSystem();
	ClearCores();
	BarData* src_cast = dynamic_cast<BarData*>(src);
	if (src_cast) {
		bardata = src_cast;
		GraphCtrl& main = AddGraph(bardata);
	} else {
		bardata = src->GetBarData();
		ASSERT(bardata);
		GraphCtrl& main = AddGraph(bardata);
		bool separate_window = src->IsCoreSeparateWindow();
		if (!separate_window) {
			main.AddSource(src);
		} else {
			AddGraph(src);
		}
	}
}

void GraphGroupCtrl::ClearCores() {
	split.Clear();
	graphs.Clear();
}

void GraphGroupCtrl::Data() {
	if (keep_at_end)
		shift = 0;
	Refresh();
}

void GraphGroupCtrl::SetTimeValueTool(bool enable) {
	for(int i = 0; i < graphs.GetCount(); i++) {
		GraphCtrl& graph = graphs[i];
		graph.ShowTimeValueTool(enable);
	}
	Refresh();
}

void GraphGroupCtrl::GraphMouseMove(Point pt, GraphCtrl* g) {
	for(int i = 0; i < graphs.GetCount(); i++) {
		graphs[i].GotMouseMove(pt, g);
	}
	
}

void GraphGroupCtrl::SetRightOffset(bool enable) {
	right_offset = enable;
	for(int i = 0; i < graphs.GetCount(); i++) {
		GraphCtrl& graph = graphs[i];
		graph.SetRightOffset(right_offset);
		graph.Refresh();
	}
}

void GraphGroupCtrl::SetKeepAtEnd(bool enable) {
	keep_at_end = enable;
}

void GraphGroupCtrl::Settings() {
	if (!core) return;
	
	System& sys = GetSystem();
	
	One<TopWindow> tw;
	bool save = false;
	tw.Create();
	
	tw->Title("Configure arguments");
	Button ok;
	ParentCtrl ctrl;
	Array<Label> labels;
	Array<EditDoubleSpin> edits;
	ok.SetLabel("OK");
	tw->Add(ctrl.HSizePos().VSizePos(0,30));
	tw->Add(ok.BottomPos(3,24).RightPos(3, 100));
	ok.WhenAction << [&] () {
		save = true;
		tw->Close();
	};
	
	ArgChanger reg;
	reg.SetLoading();
	core->IO(reg);
	
	Size sz(320, reg.args.GetCount()*30 + 50);
	tw->SetRect(sz);
	int xoff = (int)(sz.cx * 0.5);
	
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
		Index<int> tf_ids, sym_ids;
		Vector<Ptr<CoreItem> > work_queue;
		Vector<FactoryDeclaration> indi_ids;
		
		FactoryDeclaration& decl = indi_ids.Add();
		decl.factory = indi;
		for(int i = 0; i < reg.args.GetCount(); i++)
			decl.AddArg(edits[i].GetData());
		tf_ids.Add(tf);
		sym_ids.Add(symbol);
		
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
		
		for (int i = 0; i < work_queue.GetCount(); i++)
			sys.Process(*work_queue[i]);
		
		if (!work_queue.IsEmpty())
			Init(&*work_queue.Top()->core);
	}
}

}
