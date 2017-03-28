#include "GraphGroupCtrl.h"

namespace DataCtrl {

GraphGroupCtrl::GraphGroupCtrl() {
	right_offset = false;
	keep_at_end = true;
	period = 0;
	
	Add(split.VSizePos().HSizePos());
	
	split.Vert();
	
	div = 8;
	shift = 0;
}

void GraphGroupCtrl::Init() {
	LOG("GraphGroupCtrl::Init");
	
	bardata = 0;
	period = 0;

	MetaVar src = GetSource();
	if (src.Is()) {
		SetGraph(src);
	}
}

void GraphGroupCtrl::Start() {
	LOG("GraphGroupCtrl::Start");
	
}

void GraphGroupCtrl::Init(MetaVar src) {
	bardata = 0;
	period = 0;
	
	SetGraph(src);
}

GraphCtrl& GraphGroupCtrl::AddGraph(MetaVar src) {
	GraphCtrl& g = graphs.Add();
	g.WhenTimeValueTool = THISBACK(SetTimeValueTool);
	g.WhenMouseMove = THISBACK(GraphMouseMove);
	g.SetRightOffset(right_offset);
	g.SetGraphGroupCtrl(*this);
	g.AddSource(src);
	split << g;
	return g;
}

void GraphGroupCtrl::SetGraph(MetaVar src) {
	ClearContainers();
	if (!src.Is())
		return;
	bardata = src.Get<BarData>();
	if (!bardata) {
		MetaVar bdata = src->GetSource();
		bardata = bdata.Get<BarData>();
		GraphCtrl& main = AddGraph(bdata);
		ArrayNode* arr = src.Get<ArrayNode>();
		if (arr) {
			// Array of containers
			for(int i = 0; i < arr->sub_nodes.GetCount(); i++) {
				MetaVar& src = arr->sub_nodes[i];
				BarData* cont = src.Get<BarData>();
				if (cont && !cont->IsContainerSeparateWindow()) {
					main.AddSource(src);
				} else {
					AddGraph(src);
				}
			}
		} else {
			// Single container
			Container* cont = src.Get<Container>();
			if (cont && !cont->IsContainerSeparateWindow()) {
				main.AddSource(src);
			} else {
				AddGraph(src);
			}
		}
	} else {
		AddGraph(src);
	}
	period = src->GetPeriod();
}

void GraphGroupCtrl::ClearContainers() {
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
