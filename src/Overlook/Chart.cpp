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
	
	RefreshImage();
	
	title = sys.GetSymbol(symbol) + ", " + sys.GetPeriodString(tf);
	Title(title);
	
	SetGraph();
	Data();
}

void Chart::SetShift(int i) {
	shift = i;
	
	int bars = GetSystem().GetSource(symbol, tf).db.open.GetCount();
	if (bars - shift < image.begin + screen_count/* || bars - shift > image.end*/)
		RefreshImage();
}

void Chart::RefreshImage() {
	System& sys = GetSystem();
	SourceImage& si = sys.GetSource(symbol, tf);
	si.db.Start();
	int bars = si.db.open.GetCount();
	
	int new_begin = max(0, bars - screen_count - shift);
	if (image.begin == 0) image.begin = new_begin;
	else                  image.begin = min(image.begin, new_begin);
	
	image.end = image.begin + screen_count;
	image.symbol = symbol;
	image.tf = tf;
	image.period = si.db.GetPeriod();
	image.cursor = 0;
	image.point = si.db.GetPoint();
	
	if (image.begin < 0)
		return;
	
	
	ImageCompiler comp;
	comp.SetMain(decl);
	comp.Compile(si, image);
}

void Chart::RefreshCoreData(bool store_cache) {
	RefreshImage();
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
	if (keep_at_end) {
		shift = 0;
		image.begin = 0; // undo seeking to beginning
		RefreshCoreData(false);
	}
	Refresh();
}

GraphCtrl& Chart::AddGraph(GraphImage& gi) {
	GraphCtrl& g = graphs.Add();
	g.WhenTimeValueTool = THISBACK(SetTimeValueTool);
	g.WhenMouseMove = THISBACK(GraphMouseMove);
	g.SetRightOffset(right_offset);
	g.chart = this;
	g.SetSource(image, gi);
	split << g;
	return g;
}

void Chart::SetGraph() {
	tf = image.GetTf();
	Clear();
	GraphImage& main = image.GetGraph(0);
	ASSERT(main.factory >= 0);
	if (main.factory == FACTORY_DataSource) {
		AddGraph(main);
	} else {
		GraphImage* src = NULL;
		for(int i = 0; i < image.GetCount(); i++) {
			int factory = image.GetGraph(i).factory;
			if (factory == FACTORY_DataSource) {
				src = &image.GetGraph(i);
				break;
			}
		}
		if (!src)
			return;
		
		GraphCtrl& mainctrl = AddGraph(*src);
		bool separate_window = main.IsCoreSeparateWindow();
		if (!separate_window) {
			mainctrl.AddSource(main);
		} else {
			AddGraph(main);
			split.SetPos(8000);
		}
	}
}

void Chart::Clear() {
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
	/*
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
	}*/
}

}
