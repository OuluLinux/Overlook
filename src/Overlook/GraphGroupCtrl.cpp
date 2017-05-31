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
	bardata = 0;
	tf = -1;
	
	SetGraph(src);
}

void GraphGroupCtrl::Start() {
	LOG("GraphGroupCtrl::Start");
	
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
	BaseSystem& bs = src->GetBaseSystem();
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

void GraphGroupCtrl::RefreshData() {
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

}
