#include "Overlook.h"

namespace Overlook {
using namespace Upp;

SnapshotDraw::SnapshotDraw() {
	snap_id = -1;
}

void SnapshotDraw::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	AgentGroup& group = sys.GetAgentGroup();
	
	if (snap_id < 0 || snap_id >= group.snaps.GetCount()) {w.DrawRect(sz, White()); return;}
	const Snapshot& snap = group.snaps[snap_id];
	
	int agent_count		= GROUP_COUNT * SYM_COUNT;
	int value_count		= group.buf_count;
	
	int rows = TIME_SENSORS + SYM_COUNT;
	#ifndef HAVE_SYSTEM_AMP
	rows += SYM_COUNT * GROUP_COUNT;
	#endif
	
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
		
		value = 255.0 * Upp::max(0.0, Upp::min(1.0, (double)snap.year_timesensor));
		clr = (int)Upp::min(255.0, value);
		c = Color(255 - clr, clr, 0);
		id.DrawRect(0, 0, sz.cx, (int)(ystep+1), c);
		row++;
		
		value = 255.0 * Upp::max(0.0, Upp::min(1.0, (double)snap.week_timesensor));
		clr = (int)Upp::min(255.0, value);
		c = Color(255 - clr, clr, 0);
		id.DrawRect(0, (int)ystep, sz.cx, (int)(ystep+2), c);
		row++;
		
		value = 255.0 * Upp::max(0.0, Upp::min(1.0, (double)snap.day_timesensor));
		clr = (int)Upp::min(255.0, value);
		c = Color(255 - clr, clr, 0);
		id.DrawRect(0, (int)(ystep*2), sz.cx, (int)(ystep+2), c);
		row++;
	}
	
	
	// Data sensors
	int s = 0;
	for(int j = 0; j < SYM_COUNT; j++) {
		int y = (int)(row * ystep);
		int y2 = (int)((row + 1) * ystep);
		int h = y2-y;
		
		for(int k = 0; k < INPUT_SENSORS; k++) {
			int x = (int)(k * xstep);
			int x2 = (int)((k + 1) * xstep);
			int w = x2 - x;
			double d = snap.sensor[s++];
			double min = 0.0;
			double max = 1.0;
			double value = 255.0 * Upp::max(0.0, Upp::min(1.0, d));
			int clr = (int)Upp::min(255.0, value);
			Color c(255 - clr, clr, 0);
			id.DrawRect(x, y, w, h, c);
		}
		
		row++;
	}
	ASSERT(s == SENSOR_SIZE);
	
	
	// Signals
	#ifndef HAVE_SYSTEM_AMP
	cols = SIGNAL_SENSORS;
	xstep = (double)grid_w / (double)cols;
	s = 0;
	int signal_rows = SYM_COUNT * GROUP_COUNT;
	for(int j = 0; j < signal_rows; j++) {
		int y = (int)(row * ystep);
		int y2 = (int)((row + 1) * ystep);
		int h = y2-y;
		
		for(int k = 0; k < cols; k++) {
			int x = (int)(k * xstep);
			int x2 = (int)((k + 1) * xstep);
			int w = x2 - x;
			double d = snap.signal[s++];
			double min = 0.0;
			double max = 1.0;
			double value = 255.0 * Upp::max(0.0, Upp::min(1.0, d));
			int clr = (int)Upp::min(255.0, value);
			Color c(255 - clr, clr, 0);
			id.DrawRect(x, y, w, h, c);
		}
		
		row++;
	}
	ASSERT(s == SIGNAL_SIZE);
	ASSERT(row == rows);
	#endif
	
	
	w.DrawImage(0,0,id);
}















ResultGraph::ResultGraph() {
	trainee = NULL;
	
}

void ResultGraph::Paint(Draw& w) {
	if (!trainee) {w.DrawRect(GetSize(), White()); return;}
	
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	TraineeBase& trainee = *this->trainee;
	float* data = trainee.result;
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int count = Upp::min(trainee.result_count, AGENT_RESULT_COUNT);
	for(int j = 0; j < count; j++) {
		double d = data[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	if (count >= 2 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		Font fnt = Monospace(10);
		
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
	
	
	w.DrawImage(0, 0, id);
}



















HeatmapTimeView::HeatmapTimeView() {
	
}

void HeatmapTimeView::Paint(Draw& d) {
	if (fn) fn(&d);
	else d.DrawRect(GetSize(), White());
}
























EquityGraph::EquityGraph() {
	trainee = NULL;
	clr = RainbowColor(Randomf());
}

void EquityGraph::Paint(Draw& w) {
	if (!trainee) {w.DrawRect(GetSize(), White()); return;}
	AgentGroup& ag = GetSystem().GetAgentGroup();
	
	
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	double last = 0.0;
	double peak = 0.0;
	
	int max_steps = 0;
	const Vector<double>& data =
		trainee->type == 0 ?
			ag.agent_equities :
			ag.joiner_equities;
	int count = ag.snaps.GetCount();
	int data_begin = trainee->id * count;
	ASSERT(data_begin >= 0 && data_begin + count <= data.GetCount());
	
	
	for(int j = 0; j < count; j++) {
		double d = data[data_begin + j];
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
		
		if (count >= 2) {
			polyline.SetCount(0);
			for(int j = 0; j < count; j++) {
				double v = data[data_begin + j];
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












TrainingCtrl::TrainingCtrl()
{
	trainee = NULL;
	
	Add(vsplit.SizePos());
	
	vsplit << hsplit << bsplit;
	vsplit.Vert();
	vsplit.SetPos(8000);
	
	hsplit.Horz();
	
	bsplit << stats << reward;
	bsplit.Horz();
	
	trade.AddColumn("Order");
	trade.AddColumn("Type");
	trade.AddColumn("Size");
	trade.AddColumn("Symbol");
	trade.AddColumn("Price");
	trade.AddColumn("Price");
	trade.AddColumn("Profit");
	trade.ColumnWidths("3 2 2 2 2 2 3");
}

void TrainingCtrl::SetAgent(Agent& agent) {
	this->trainee = &agent;
	
	stats.SetTrainee(agent);
	reward.SetTrainee(agent);
	timescroll.SetAgent(agent.dqn);
	
	hsplit << draw << timescroll;
}

void TrainingCtrl::SetJoiner(Joiner& joiner) {
	this->trainee = &joiner;
	
	stats.SetTrainee(joiner);
	reward.SetTrainee(joiner);
	timescroll.SetAgent(joiner.dqn);
	
	hsplit << draw << trade << timescroll;
	hsplit.SetPos(2000, 0);
	hsplit.SetPos(8000, 1);
}

void TrainingCtrl::Data() {
	if (!trainee) return;
	
	AgentGroup& ag = GetSystem().GetAgentGroup();
	
	// list
	
	draw.SetSnap(ag.snap_begin + trainee->cursor);
	draw.Refresh();
	timescroll.Refresh();
	reward.Refresh();
	stats.Refresh();
	reward.Refresh();
	
	if (trainee->type == 1) {
		Joiner& joiner = *static_cast<Joiner*>(trainee);
		FixedSimBroker& b = joiner.broker;
		MetaTrader& mt = GetMetaTrader();
		
		int j = 0;
		for(int i = 0; i < MAX_ORDERS; i++) {
			int symbol = i % ORDERS_PER_SYMBOL;
			FixedOrder& o = b.order[i];
			if (!o.is_open) continue;
			
			const Symbol& sym = mt.GetSymbol(ag.sym_ids[symbol]);
			trade.Set(j, 0, i);
			trade.Set(j, 1, o.type == 0 ? "Buy" : "Sell");
			trade.Set(j, 2, o.volume);
			trade.Set(j, 3, sym.name);
			trade.Set(j, 4, o.open);
			trade.Set(j, 5, o.close);
			trade.Set(j, 6, o.profit);
			
			j++;
		}
		trade.SetCount(j);
	}
}












SnapshotCtrl::SnapshotCtrl() {
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
	AgentGroup& group = GetSystem().GetAgentGroup();
	int cursor = list.GetCursor();
	
	for(int i = 0; i < group.snaps.GetCount(); i++) {
		const Snapshot& snap = group.snaps[i];
		list.Set(i, 0, i);/*
		list.Set(i, 1, snap.time);
		list.Set(i, 2, snap.added);
		list.Set(i, 3, snap.tfs_used);*/
	}
	list.SetCount(group.snaps.GetCount());
	
	if (cursor >= 0 && cursor < group.snaps.GetCount())
		draw.SetSnap(cursor);
	
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
	
	FileIn fin(ConfigFile("agentgroup.log"));
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
	
	FileIn fin(ConfigFile("agentgroup.log"));
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

/*void DataCtrl::SetGroup(AgentGroup& group) {
	equity.Clear();
	last_pos = 0;
	poslist.Clear();
	timeslider.MinMax(0,1);
	timeslider.SetData(0);
	
	
	Data();
}*/

}
