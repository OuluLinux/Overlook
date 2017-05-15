#include "Overlook.h"

namespace Overlook {

GraphGroupCtrl::GraphGroupCtrl() {
	right_offset = false;
	keep_at_end = true;
	period = 0;
	
	Add(split.VSizePos().HSizePos());
	
	split.Vert();
	
	div = 8;
	shift = 0;
}

void GraphGroupCtrl::Init(Core* src) {
	/*LOG("GraphGroupCtrl::Init");
	
	bardata = 0;
	period = 0;

	Core* src = GetSource();
	if (src.Is()) {
		SetGraph(src);
	}*/
	
	bardata = 0;
	period = 0;
	
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
	/*ClearPipes();
	if (!src.Is())
		return;
	bardata = src.Get<Pipe>();
	if (!bardata) {
		Core* bdata = src->GetSource();
		bardata = bdata.Get<Pipe>();
		GraphCtrl& main = AddGraph(bdata);
		ArrayNode* arr = src.Get<ArrayNode>();
		if (arr) {
			// Array of containers
			for(int i = 0; i < arr->sub_nodes.GetCount(); i++) {
				Core*& src = arr->sub_nodes[i];
				Pipe* cont = src.Get<Pipe>();
				if (cont && !cont->IsPipeSeparateWindow()) {
					main.AddSource(src);
				} else {
					AddGraph(src);
				}
			}
		} else {
			// Single container
			Pipe* cont = src.Get<Pipe>();
			if (cont && !cont->IsPipeSeparateWindow()) {
				main.AddSource(src);
			} else {
				AddGraph(src);
			}
		}
	} else {
		AddGraph(src);
	}
	period = src->GetPeriod();*/
}

void GraphGroupCtrl::ClearPipes() {
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
