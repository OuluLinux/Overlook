#include "Overlook.h"

namespace Overlook {
using namespace Upp;

SnapshotDraw::SnapshotDraw() {
	group = NULL;
	snap_id = -1;
}

void SnapshotDraw::Paint(Draw& w) {
	if (!group) {w.DrawRect(GetSize(), White()); return;}
	
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	
	/*AgentGroup& group = *this->group;
	SymGroup& group = *this->symgroup;
	System& sys = *group.sys;
	
	if (snap_id < 0 || snap_id >= group.snaps.GetCount()) {w.DrawRect(sz, White()); return;}
	const Snapshot& snap = group.snaps[snap_id];
	
	int sym_count		= group.sym_ids.GetCount();
	int tf_count		= group.tf_ids.GetCount();
	int value_count		= group.buf_count;
	
	int rows = 2 + sym_count * tf_count * 3;
	int cols = value_count;
	int grid_w = sz.cx;
	double xstep = (double)grid_w / (double)cols;
	double ystep = (double)sz.cy / (double)rows;
	int row = 0;
	
	
	// Time value
	{
		double value;
		int clr;
		Color c;
		
		value = 255.0 * Upp::max(0.0, Upp::min(1.0, snap.year_timesensor));
		clr = (int)Upp::min(255.0, value);
		c = Color(255 - clr, clr, 0);
		id.DrawRect(0, 0, sz.cx, (int)(ystep+1), c);
		row++;
		
		value = 255.0 * Upp::max(0.0, Upp::min(1.0, snap.wday_timesensor));
		clr = (int)Upp::min(255.0, value);
		c = Color(255 - clr, clr, 0);
		id.DrawRect(0, (int)ystep, sz.cx, (int)(ystep+1), c);
		row++;
	}
	
	
	
	// Data sensors
	ASSERT(snap.sensors.GetCount() == (group.sym_ids.GetCount() * value_count * group.tf_ids.GetCount()));
	for(int i = 0; i < tf_count; i++) {
		for(int j = 0; j < sym_count; j++) {
			int y = (int)(row * ystep);
			int y2 = (int)((row + 1) * ystep);
			int h = y2-y;
			
			for(int k = 0; k < value_count; k++) {
				int x = (int)(k * xstep);
				int x2 = (int)((k + 1) * xstep);
				int w = x2 - x;
				double d = snap.sensors[(i * group.sym_ids.GetCount() + j) * value_count + k];
				double min = 0.0;
				double max = 1.0;
				double value = 255.0 * Upp::max(0.0, Upp::min(1.0, d));
				int clr = (int)Upp::min(255.0, value);
				Color c(255 - clr, clr, 0);
				id.DrawRect(x, y, w, h, c);
			}
			
			row++;
		}
	}
	
	
	// Signals
	cols = 2;
	xstep = (double)grid_w / (double)cols;
	ASSERT(snap.signals.GetCount() == (cols * sym_count * tf_count));
	for(int i = 0; i < tf_count; i++) {
		for(int j = 0; j < sym_count; j++) {
			int y = (int)(row * ystep);
			int y2 = (int)((row + 1) * ystep);
			int h = y2-y;
			
			for(int k = 0; k < cols; k++) {
				int x = (int)(k * xstep);
				int x2 = (int)((k + 1) * xstep);
				int w = x2 - x;
				double d = snap.signals[(i * group.sym_ids.GetCount() + j) * cols + k];
				double min = 0.0;
				double max = 1.0;
				double value = 255.0 * Upp::max(0.0, Upp::min(1.0, d));
				int clr = (int)Upp::min(255.0, value);
				Color c(255 - clr, clr, 0);
				id.DrawRect(x, y, w, h, c);
			}
			
			row++;
		}
	}
	*/
	
	w.DrawImage(0,0,id);
}















ResultGraph::ResultGraph() {
	//trainee = NULL;
	
}

void ResultGraph::Paint(Draw& w) {
	/*if (!trainee) {w.DrawRect(GetSize(), White()); return;}
	
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	TraineeBase& trainee = *this->trainee;
	const Vector<double>& data = trainee.GetSequenceResults();
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int count = data.GetCount();
	for(int j = 0; j < count; j++) {
		double d = data[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	if (count >= 2 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		Font fnt = Monospace(10);
		
		int count = data.GetCount();
		if (count >= 2) {
			polyline.SetCount(count);
			for(int j = 0; j < count; j++) {
				double v = data[j];
				int y = (int)(sz.cy - (v - min) / diff * sz.cy);
				int x = (int)(j * xstep);
				polyline[j] = Point(x, y);
			}
			id.DrawPolyline(polyline, 1, Color(193, 255, 255));
			for(int j = 0; j < polyline.GetCount(); j++) {
				const Point& p = polyline[j];
				id.DrawRect(p.x-1, p.y-1, 3, 3, Blue());
			}
		}
	}
	
	
	w.DrawImage(0, 0, id);*/
}



















HeatmapTimeView::HeatmapTimeView() {
	
}

void HeatmapTimeView::Paint(Draw& d) {
	if (fn) fn(&d);
	else d.DrawRect(GetSize(), White());
}
























EquityGraph::EquityGraph() {
	//trainee = NULL;
	clr = RainbowColor(Randomf());
}

void EquityGraph::Paint(Draw& w) {
	/*if (!trainee) {w.DrawRect(GetSize(), White()); return;}
	
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	double last = 0.0;
	double peak = 0.0;
	
	int max_steps = 0;
	const Vector<double>& data = trainee->thrd_equity;
	int count = data.GetCount();
	for(int j = 0; j < count; j++) {
		double d = data[j];
		if (d == 0.0) continue;
		if (d > max) max = d;
		if (d < min) min = d;
	}
	if (count > max_steps)
		max_steps = count;
	
	
	if (max_steps > 1 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		int count = data.GetCount();
		if (count >= 2) {
			polyline.SetCount(0);
			for(int j = 0; j < count; j++) {
				double v = data[j];
				if (v == 0.0) continue;
				last = v;
				int x = (int)(j * xstep);
				int y = (int)(sz.cy - (v - min) / diff * sz.cy);
				polyline.Add(Point(x, y));
				if (v > peak) peak = v;
			}
			if (polyline.GetCount() >= 2)
				id.DrawPolyline(polyline, 1, clr);
		}
		
		{
			int y = 0;
			String str = DblStr(peak);
			Size str_sz = GetTextSize(str, fnt);
			id.DrawRect(3, y, 13, 13, clr);
			id.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
			id.DrawText(16, y, str, fnt, Black());
		}
		{
			int y = 0;
			String str = DblStr(last);
			Size str_sz = GetTextSize(str, fnt);
			id.DrawRect(sz.cx - 3  - str_sz.cx, y, 13, 13, clr);
			id.DrawRect(sz.cx - 16 - str_sz.cx, y, str_sz.cx, str_sz.cy, White());
			id.DrawText(sz.cx - 16 - str_sz.cx, y, str, fnt, Black());
		}
	}
	
	
	w.DrawImage(0, 0, id);*/
}












TrainingCtrl::TrainingCtrl()
{
	//trainee = NULL;
	
	Add(vsplit.SizePos());
	
	vsplit << hsplit << bsplit;
	vsplit.Vert();
	vsplit.SetPos(8000);
	
	broker.ReadOnly();
	
	hsplit << draw << broker;
	hsplit.Horz();
	hsplit.SetPos(2000, 0);
	
	bsplit.Horz();
	bsplit << stats << reward;
	
}
/*
void TrainingCtrl::SetTrainee(TraineeBase& trainee) {
	this->trainee = &trainee;
	
	broker.SetBroker(trainee.broker);
	stats.SetTrainee(trainee);
	reward.SetTrainee(trainee);
	draw.SetGroup(*trainee.group);
	
	Agent* agent = dynamic_cast<Agent*>(&trainee);
	if (agent) {
		if (hsplit.GetCount() == 2) {
			hsplit << timescroll;
			hsplit.SetPos(2000, 0);
			hsplit.SetPos(8000, 1);
		}
		timescroll.SetAgent(agent->dqn);
	}
}
*/
void TrainingCtrl::Data() {
	/*
	if (!trainee) return;
	broker.Data();
	draw.SetSnap(trainee->epoch_actual);
	draw.Refresh();
	timescroll.Refresh();
	reward.Refresh();
	stats.Refresh();
	reward.Refresh();*/
}












SnapshotCtrl::SnapshotCtrl()
{
	group = NULL;
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << draw << list;
	hsplit.SetPos(2000);
	
	list.AddColumn("#");
	list.AddColumn("Time");
	list.AddColumn("Added");
	list.AddColumn("Tfs");
	
	list <<= THISBACK(Data);
}

void SnapshotCtrl::Data() {
	if (!group) return;
	/*
	AgentGroup& group = *this->group;
	int cursor = list.GetCursor();
	
	for(int i = 0; i < group.snaps.GetCount(); i++) {
		const Snapshot& snap = group.snaps[i];
		list.Set(i, 0, i);
		list.Set(i, 1, snap.time);
		list.Set(i, 2, snap.added);
		list.Set(i, 3, snap.tfs_used);
	}
	list.SetCount(group.snaps.GetCount());
	
	if (cursor >= 0 && cursor < group.snaps.GetCount())
		draw.SetSnap(cursor);
	*/
	draw.Refresh();
}
















DataGraph::DataGraph(DataCtrl* dc) : dc(dc) {
	clr = Blue();
}

void DataGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	double last = 0.0;
	double peak = 0.0;
	
	int max_steps = 0;
	const Vector<double>& data = dc->equity;
	int count = data.GetCount();
	for(int j = 0; j < count; j++) {
		double d = data[j];
		if (d == 0.0) continue;
		if (d > max) max = d;
		if (d < min) min = d;
	}
	if (count > max_steps)
		max_steps = count;
	
	
	if (max_steps > 1 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		int count = data.GetCount();
		if (count >= 2) {
			polyline.SetCount(0);
			for(int j = 0; j < count; j++) {
				double v = data[j];
				if (v == 0.0) continue;
				last = v;
				int x = (int)(j * xstep);
				int y = (int)(sz.cy - (v - min) / diff * sz.cy);
				polyline.Add(Point(x, y));
				if (v > peak) peak = v;
			}
			if (polyline.GetCount() >= 2)
				id.DrawPolyline(polyline, 1, clr);
		}
		
		{
			int y = 0;
			String str = DblStr(peak);
			Size str_sz = GetTextSize(str, fnt);
			id.DrawRect(3, y, 13, 13, clr);
			id.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
			id.DrawText(16, y, str, fnt, Black());
		}
		{
			int y = 0;
			String str = DblStr(last);
			Size str_sz = GetTextSize(str, fnt);
			id.DrawRect(sz.cx - 3  - str_sz.cx, y, 13, 13, clr);
			id.DrawRect(sz.cx - 16 - str_sz.cx, y, str_sz.cx, str_sz.cy, White());
			id.DrawText(sz.cx - 16 - str_sz.cx, y, str, fnt, Black());
		}
	}
	
	
	w.DrawImage(0, 0, id);
}


DataCtrl::DataCtrl() :
	graph(this)
{
	last_pos = 0;
	group = NULL;
	
	timeslider.MinMax(0,1);
	timeslider <<= THISBACK(GuiData);
	
	Add(graph.HSizePos().TopPos(0,120));
	Add(timeslider.TopPos(120,30).LeftPos(0, 200-2));
	Add(data.TopPos(120,30).HSizePos(200));
	Add(hsplit.VSizePos(150).HSizePos());
	
	hsplit.Horz();
	hsplit << siglist << trade;
	hsplit.SetPos(2000);
	
	siglist.AddColumn("Symbol");
	siglist.AddColumn("Signal");
	
	trade.AddColumn("Order");
	trade.AddColumn("Time");
	trade.AddColumn("Type");
	trade.AddColumn("Size");
	trade.AddColumn("Symbol");
	trade.AddColumn("Price");
	trade.AddColumn("S / L");
	trade.AddColumn("T / P");
	trade.AddColumn("Price");
	trade.AddColumn("Commission");
	trade.AddColumn("Swap");
	trade.AddColumn("Profit");
	trade.ColumnWidths("3 4 2 2 2 2 2 2 2 2 2 3");
}

void DataCtrl::GuiData() {
	if (!group) return;
	
	FileIn fin(ConfigFile(group->name + ".log"));
	if (!fin.IsOpen())
		return;
	
	int cursor = timeslider.GetData();
	if (cursor < 0 || cursor >= poslist.GetCount())
		return;
	
	MetaTrader& mt = GetMetaTrader();
	
	int file_version;
	double balance, equity;
	Time time;
	Array<Order> orders;
	Vector<int> signals;
	
	int pos = poslist[cursor];
	fin.Seek(pos);
	fin.SeekCur(sizeof(int));
	fin % file_version % balance % equity % time % signals % orders;
	
	data.SetLabel(Format("%", time) + Format(" Equity: %f Balance: %f", equity, balance));
	
	for(int i = 0; i < signals.GetCount() && i < mt.GetSymbolCount(); i++) {
		const Symbol& sym = mt.GetSymbol(i);
		siglist.Set(i, 0, sym.name);
		siglist.Set(i, 1, signals[i]);
	}
	
	for(int i = 0; i < orders.GetCount(); i++) {
		const Order& o = orders[i];
		String name =
			o.symbol >= 0 && o.symbol < mt.GetSymbolCount() ?
				mt.GetSymbol(o.symbol).name :
				IntStr(o.symbol);
		trade.Set(i, 0, o.ticket);
		trade.Set(i, 1, Format("%", o.begin));
		trade.Set(i, 2, o.type == 0 ? "Buy" : "Sell");
		trade.Set(i, 3, o.volume);
		trade.Set(i, 4, name);
		trade.Set(i, 5, o.open);
		trade.Set(i, 6, o.stoploss);
		trade.Set(i, 7, o.takeprofit);
		trade.Set(i, 8, o.close);
		trade.Set(i, 9, o.commission);
		trade.Set(i, 10, o.swap);
		trade.Set(i, 11, Format("%2n", o.profit));
	}
	trade.SetCount(orders.GetCount());
	
	graph.Refresh();
}

void DataCtrl::Data() {
	if (!group) return;
	
	FileIn fin(ConfigFile(group->name + ".log"));
	if (!fin.IsOpen())
		return;
	
	fin.Seek(last_pos);
	while (!fin.IsEof()) {
		int64 pos = fin.GetPos();
		
		poslist.Add(pos);
		
		int size;
		if (fin.Get(&size, sizeof(int)) != sizeof(int))
			break;
		
		pos += sizeof(int) + size;
		
		int file_version;
		double balance, equity;
		fin % file_version % balance % equity;
		fin.Seek(pos);
		
		this->equity.Add(equity);
	}
	
	timeslider.MinMax(0, Upp::max(1, poslist.GetCount()-1));
	
	last_pos = fin.GetSize();
	
	GuiData();
}

void DataCtrl::SetGroup(AgentGroup& group) {
	this->group = &group;
	equity.Clear();
	last_pos = 0;
	poslist.Clear();
	timeslider.MinMax(0,1);
	timeslider.SetData(0);
	
	
	Data();
}

}
