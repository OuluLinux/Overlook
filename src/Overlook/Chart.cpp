#include "Overlook.h"

namespace Overlook {

Chart::Chart() {
	right_offset = false;
	keep_at_end = true;
	tf = -1;
	
	Add(split.VSizePos().HSizePos());
	
	split.Vert();
	
	div = 8;
	shift = 0;
}

void Chart::Init(int symbol, const FactoryDeclaration& decl, int tf) {
	this->decl = decl;
	this->symbol = symbol;
	this->tf = tf;
	
	RefreshCore();
}

void Chart::RefreshCore() {
	System& sys = GetSystem();
	
	Index<int> tf_ids, sym_ids;
	Vector<FactoryDeclaration> indi_ids;
	ASSERT(tf >= 0 && tf < sys.GetPeriodCount());
	ASSERT(symbol >= 0 && symbol < sys.GetSymbolCount());
	indi_ids.Add(decl);
	tf_ids.Add(tf);
	sym_ids.Add(symbol);
	work_queue.Clear();
	sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	RefreshCoreData(true);
	
	Core* src = &*work_queue.Top()->core;
	if (!src) return;
	
	core = src;
	bardata = 0;
	
	title = sys.GetSymbol(symbol) + ", " + sys.GetPeriodString(tf);
	Title(title);
	
	SetGraph(src);
	Data();
}

void Chart::RefreshCoreData(bool store_cache) {
	System& sys = GetSystem();
	for (int i = 0; i < work_queue.GetCount(); i++)
		sys.Process(*work_queue[i], store_cache);
}

void Chart::ContextMenu(Bar& bar) {
	bar.Add("Settings", THISBACK(Settings));
}

String Chart::GetTitle() {
	return title;
}

Chart& Chart::SetTimeframe(int tf_id) {
	this->tf = tf_id;
	
	RefreshCore();
	
	return *this;
}

Chart& Chart::SetFactory(int f) {
	this->decl.factory = f;
	this->decl.arg_count = 0;
	
	RefreshCore();
	
	return *this;
}

void Chart::Start() {
	if (keep_at_end) shift = 0;
	RefreshCoreData(false);
	Refresh();
}

GraphCtrl& Chart::AddGraph(Core* src) {
	GraphCtrl& g = graphs.Add();
	g.WhenTimeValueTool = THISBACK(SetTimeValueTool);
	g.WhenMouseMove = THISBACK(GraphMouseMove);
	g.SetRightOffset(right_offset);
	g.chart = this;
	g.AddSource(src);
	split << g;
	return g;
}

void Chart::SetGraph(Core* src) {
	ASSERT(src);
	tf = src->GetTf();
	ClearCores();
	DataBridge* src_cast = dynamic_cast<DataBridge*>(src);
	if (src_cast) {
		bardata = src_cast;
		GraphCtrl& main = AddGraph(bardata);
	} else {
		bardata = src->GetDataBridge();
		ASSERT(bardata);
		GraphCtrl& main = AddGraph(bardata);
		bool separate_window = src->IsCoreSeparateWindow();
		if (!separate_window) {
			main.AddSource(src);
		} else {
			AddGraph(src);
			split.SetPos(8000);
		}
	}
}

void Chart::ClearCores() {
	split.Clear();
	graphs.Clear();
}

void Chart::Data() {
	if (keep_at_end) shift = 0;
	Refresh();
}

void Chart::SetTimeValueTool(bool enable) {
	for(int i = 0; i < graphs.GetCount(); i++) {
		GraphCtrl& graph = graphs[i];
		graph.ShowTimeValueTool(enable);
	}
	Refresh();
}

void Chart::GraphMouseMove(Point pt, GraphCtrl* g) {
	for(int i = 0; i < graphs.GetCount(); i++) {
		graphs[i].GotMouseMove(pt, g);
	}
	
}

void Chart::SetRightOffset(bool enable) {
	right_offset = enable;
	for(int i = 0; i < graphs.GetCount(); i++) {
		GraphCtrl& graph = graphs[i];
		graph.SetRightOffset(right_offset);
		graph.Refresh();
	}
}

void Chart::SetKeepAtEnd(bool enable) {
	keep_at_end = enable;
}

void Chart::Settings() {
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
		decl.arg_count = 0;
		for(int i = 0; i < reg.args.GetCount(); i++)
			decl.AddArg(edits[i].GetData());
		Init(symbol, decl, tf);
	}
}

}
