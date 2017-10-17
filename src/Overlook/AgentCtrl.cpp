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
	AgentSystem& group = sys.GetAgentSystem();
	
	if (snap_id < 0 || snap_id >= group.snaps.GetCount()) {w.DrawRect(sz, White()); return;}
	const Snapshot& snap = group.snaps[snap_id];
	
	int agent_count		= TRAINEE_COUNT;
	int value_count		= group.buf_count;
	
	int rows = TIME_SENSORS + SYM_COUNT;
	
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
		
		value = 255.0 * Upp::max(0.0, Upp::min(1.0, (double)snap.GetTimeSensor(0)));
		clr = (int)Upp::min(255.0, value);
		c = Color(255 - clr, clr, 0);
		id.DrawRect(0, 0, sz.cx, (int)(ystep+1), c);
		row++;
		
		value = 255.0 * Upp::max(0.0, Upp::min(1.0, (double)snap.GetTimeSensor(1)));
		clr = (int)Upp::min(255.0, value);
		c = Color(255 - clr, clr, 0);
		id.DrawRect(0, (int)ystep, sz.cx, (int)(ystep+2), c);
		row++;
		
		value = 255.0 * Upp::max(0.0, Upp::min(1.0, (double)snap.GetTimeSensor(2)));
		clr = (int)Upp::min(255.0, value);
		c = Color(255 - clr, clr, 0);
		id.DrawRect(0, (int)(ystep*2), sz.cx, (int)(ystep+2), c);
		row++;
	}
	
	
	// Data sensors
	for(int j = 0; j < SYM_COUNT; j++) {
		int y = (int)(row * ystep);
		int y2 = (int)((row + 1) * ystep);
		int h = y2-y;
		
		for(int k = 0; k < INPUT_SENSORS; k++) {
			int x = (int)(k * xstep);
			int x2 = (int)((k + 1) * xstep);
			int w = x2 - x;
			double d = snap.GetSensor(j, k);
			double min = 0.0;
			double max = 1.0;
			double value = 255.0 * Upp::max(0.0, Upp::min(1.0, d));
			int clr = (int)Upp::min(255.0, value);
			Color c(255 - clr, clr, 0);
			id.DrawRect(x, y, w, h, c);
		}
		
		row++;
	}
	
	
	ASSERT(row == rows);
	
	
	w.DrawImage(0,0,id);
}















ResultGraph::ResultGraph() {
	agent = NULL;
	fuse = NULL;
	
}

const Vector<double>& ResultGraph::GetEquityData() {
	if (type == PHASE_SIGNAL_TRAINING)
		return agent->sig.result_equity;
	if (type == PHASE_AMP_TRAINING)
		return agent->amp.result_equity;
	return fuse->result_equity;
}

const Vector<double>& ResultGraph::GetDrawdownData() {
	if (type == PHASE_SIGNAL_TRAINING)
		return agent->sig.result_drawdown;
	if (type == PHASE_AMP_TRAINING)
		return agent->amp.result_drawdown;
	return fuse->result_drawdown;
}

void ResultGraph::Paint(Draw& w) {
	if (!agent && !fuse) {w.DrawRect(GetSize(), White()); return;}
	
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	const Vector<double>& equity_data = GetEquityData();
	const Vector<double>& drawdown_data = GetDrawdownData();
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int count = equity_data.GetCount();
	for(int j = 0; j < count; j++) {
		double d = equity_data[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	if (count >= 2 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = equity_data[j];
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
	
	
	min = +DBL_MAX;
	max = -DBL_MAX;
	
	count = drawdown_data.GetCount();
	for(int j = 0; j < count; j++) {
		double d = drawdown_data[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	if (count >= 2 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = drawdown_data[j];
			int y = (int)(sz.cy - (v - min) / diff * sz.cy);
			int x = (int)(j * xstep);
			polyline[j] = Point(x, y);
		}
		id.DrawPolyline(polyline, 1, Red());
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
	agent = NULL;
	fuse = NULL;
	clr = RainbowColor(Randomf());
	type = 0;
}

const Vector<double>& EquityGraph::GetEquityData() {
	if (type == PHASE_SIGNAL_TRAINING)
		return agent->sig.equity;
	if (type == PHASE_AMP_TRAINING)
		return agent->amp.equity;
	return fuse->equity;
}

void EquityGraph::Paint(Draw& w) {
	if (!agent && !fuse) {w.DrawRect(GetSize(), White()); return;}
	AgentSystem& ag = GetSystem().GetAgentSystem();
	
	
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	double last = 0.0;
	double peak = 0.0;
	
	const Vector<double>& data = GetEquityData();
	int count = data.GetCount();
	if (!count) return;
	
	
	for(int j = 0; j < count; j++) {
		double d = data[j];
		if (d == 0.0) continue;
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	
	if (count > 1 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		Font fnt = Monospace(10);
		
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










RewardGraph::RewardGraph() {
	clr = Color(255, 105, 0);
}
	
void RewardGraph::Paint(Draw& d) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	if (agent || fuse) {
		double min = +DBL_MAX;
		double max = -DBL_MAX;
		double last = 0.0;
		double peak = 0.0;
		
		#define SRC(x) type == PHASE_SIGNAL_TRAINING ? agent->sig.x : (type == PHASE_AMP_TRAINING ? agent->amp.x : fuse->x)
		int reward_count = SRC(reward_count);
		const Vector<double>& data = SRC(rewards);
		int experience_add_every = SRC(dqn.GetExperienceAddEvery());
		int count = data.GetCount();
		
		for(int j = 0; j < count; j++) {
			double d = data[j];
			if (d == 0.0) continue;
			if (d > max) max = d;
			if (d < min) min = d;
		}
		
		
		if (count > 1 && max > min) {
			double diff = max - min;
			double xstep = (double)sz.cx / (count - 1);
			Font fnt = Monospace(10);
			
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
				int y = 13;
				String str = "Exp add every: " + IntStr(experience_add_every);
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
		} else {
			id.DrawText(3, 3, "No data in agent (" + IntStr(count) + "/2): "
				+ IntStr(reward_count) + "/"
				+ IntStr(REWARD_AV_PERIOD), Monospace(10));
		}
	} else {
		id.DrawText(3, 3, "No agent set", Monospace(10));
	}
	
	d.DrawImage(0, 0, id);
}

void RewardGraph::SetAgent(Agent& agent, int type) {
	this->type = type;
	this->agent = &agent;
}

void RewardGraph::SetFuse(AgentFuse& fuse) {
	type = PHASE_FUSE_TRAINING;
	this->fuse = &fuse;
}








TrainingCtrl::TrainingCtrl()
{
	agent = NULL;
	fuse = NULL;
	type = 0;
	
	Add(vsplit.SizePos());
	
	vsplit << hsplit << bsplit;
	vsplit.Vert();
	vsplit.SetPos(8000);
	
	hsplit.Horz();
	
	bsplit << reward << stats << result;
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

void TrainingCtrl::SetAgent(Agent& agent, int type) {
	this->type = type;
	this->agent = &agent;
	
	reward.SetAgent(agent, type);
	stats.SetAgent(agent, type);
	result.SetAgent(agent, type);
	
	if (type == PHASE_SIGNAL_TRAINING) {
		timescroll.SetAgent(agent.sig.dqn);
	}
	else if (type == PHASE_AMP_TRAINING) {
		timescroll.SetAgent(agent.amp.dqn);
	}
	
	hsplit.Clear();
	if (type == PHASE_AMP_TRAINING) {
		hsplit << draw << trade << timescroll;
		hsplit.SetPos(2000, 0);
		hsplit.SetPos(8000, 1);
	}
	else {
		hsplit << draw << timescroll;
	}
}

void TrainingCtrl::SetFuse(AgentFuse& fuse) {
	this->type = PHASE_FUSE_TRAINING;
	this->fuse = &fuse;
	
	reward.SetFuse(fuse);
	stats.SetFuse(fuse);
	result.SetFuse(fuse);
	
	timescroll.SetAgent(fuse.dqn);
	
	hsplit.Clear();
	hsplit << draw << timescroll;
}

void TrainingCtrl::Data() {
	if (!agent && !fuse) return;
	
	AgentSystem& ag = GetSystem().GetAgentSystem();
	
	// list
	int cursor = type == PHASE_SIGNAL_TRAINING ? agent->sig.cursor : (type == PHASE_AMP_TRAINING ? agent->amp.cursor : fuse->cursor);
	draw.SetSnap(cursor);
	draw.Refresh();
	timescroll.Refresh();
	result.Refresh();
	reward.Refresh();
	stats.Refresh();
	result.Refresh();
	
	if (type == PHASE_AMP_TRAINING) {
		FixedSimBroker& b = agent->amp.broker;
		MetaTrader& mt = GetMetaTrader();
		
		int j = 0;
		for(int i = 0; i < MAX_ORDERS; i++) {
			int symbol = i / ORDERS_PER_SYMBOL;
			FixedOrder& o = b.order[i];
			if (!o.is_open) continue;
			
			const Symbol& sym = mt.GetSymbol(ag.sym_ids[symbol]);
			trade.Set(j, 0, i);
			trade.Set(j, 1, o.type == 0 ? "Buy" : "Sell");
			trade.Set(j, 2, Format("%2!n", o.volume));
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
	Add(slider.HSizePos().TopPos(0, 30));
	Add(hsplit.HSizePos().VSizePos(30));
	
	hsplit.Horz();
	hsplit << draw << symtf << symcl << sym << one;
	
	symtf.AddColumn("Symbol");
	symtf.AddColumn("Timeframe");
	symtf.AddColumn("Result volatility");
	symtf.AddColumn("Result change");
	symtf.AddColumn("Result cluster");
	
	symcl.AddColumn("Symbol");
	symcl.AddColumn("Result cluster");
	symcl.AddColumn("Is predicted");
	symcl.AddColumn("Target");
	symcl.AddColumn("Volatiled");
	symcl.AddColumn("Changed");
	symcl.AddColumn("Signal");
	symcl.AddColumn("Amp");
	
	sym.AddColumn("Symbol");
	sym.AddColumn("Open");
	sym.AddColumn("Change");
	//for(int i = 0; i < SENSOR_SIZE; i++)
	//	sym.AddColumn("S" + IntStr(i));
	
	one.AddColumn("Key");
	one.AddColumn("Value");
	
	slider.SetData(0);
	slider <<= THISBACK(Data);
}

void SnapshotCtrl::Data() {
	System& sys = GetSystem();
	AgentSystem& group = sys.GetAgentSystem();
	int cursor = slider.GetData();
	int min_tf = sys.FindPeriod(1);
	slider.MinMax(0, group.snaps.GetCount()-1);
	
	if (cursor >= 0 && cursor < group.snaps.GetCount()) {
		draw.SetSnap(cursor);
		
		const Snapshot& snap = group.snaps[cursor];
		
		int k = 0;
		for(int i = 0; i < SYM_COUNT; i++) {
			int sym_id = group.sym_ids[i];
			for(int j = 0; j < MEASURE_PERIODCOUNT; j++) {
				symtf.Set(k, 0, sys.GetSymbol(sym_id));
				symtf.Set(k, 1, MEASURE_PERIOD(j));
				symtf.Set(k, 2, snap.GetPeriodVolatility(i, j));
				symtf.Set(k, 3, snap.GetPeriodChange(i, j));
				symtf.Set(k, 4, snap.GetResultCluster(i, j));
				k++;
			}
		}
		
		k = 0;
		for(int i = 0; i < SYM_COUNT; i++) {
			int sym_id = group.sym_ids[i];
			for(int j = 0; j < OUTPUT_COUNT; j++) {
				symcl.Set(k, 0, sys.GetSymbol(sym_id));
				symcl.Set(k, 1, j);
				symcl.Set(k, 2, snap.IsResultClusterPredicted(i, j));
				
				String target_str;
				for(int k = 0; k < TARGET_COUNT; k++)
					target_str << (int)snap.IsResultClusterPredictedTarget(i, j, k) << " ";
				symcl.Set(k, 3, target_str);
				
				symcl.Set(k, 4, snap.GetResultClusterPredictedVolatiled(i, j));
				symcl.Set(k, 5, snap.GetResultClusterPredictedChanged(i, j));
				symcl.Set(k, 6, snap.GetSignalOutput(j, i));
				symcl.Set(k, 7, snap.GetAmpOutput(j, i));
				k++;
			}
		}
		
		k = 0;
		for(int i = 0; i < SYM_COUNT; i++) {
			int sym_id = group.sym_ids[i];
			sym.Set(k, 0, sys.GetSymbol(sym_id));
			sym.Set(k, 1, snap.GetOpen(i));
			sym.Set(k, 2, snap.GetChange(i));
			//for(int j = 0; j < SENSOR_SIZE; j++)
			//	sym.Set(k, 3+j, snap.GetSensor(i, j));
			k++;
		}
		
		one.Set(0, 0, "Year sensor");
		one.Set(0, 1, snap.GetTimeSensor(0));
		one.Set(1, 0, "Week sensor");
		one.Set(1, 1, snap.GetTimeSensor(1));
		one.Set(2, 0, "Day sensor");
		one.Set(2, 1, snap.GetTimeSensor(2));
		one.Set(3, 0, "Indicator cluster");
		one.Set(3, 1, snap.GetIndicatorCluster());
		one.Set(4, 0, "Shift");
		one.Set(4, 1, snap.GetShift());
		one.Set(5, 0, "Time");
		one.Set(5, 1, Format("%", sys.GetTimeTf(min_tf, snap.GetShift())));
		
		int nonzero_signals = 0;
		for(int i = 0; i < GROUP_COUNT; i++)
			for(int j = 0; j < SYM_COUNT; j++)
				nonzero_signals += snap.GetSignalOutput(i, j) != 0 ? 1 : 0;
		one.Set(6, 0, "Signals total");
		one.Set(6, 1, nonzero_signals);
		
		int useful_nonzero_signals = 0;
		for(int i = 0; i < GROUP_COUNT; i++)
			for(int j = 0; j < SYM_COUNT; j++)
				if (snap.GetSignalOutput(i, j) != 0 && group.groups[i].agents[j].amp.GetLastDrawdown() < DD_LIMIT)
					useful_nonzero_signals += 1;
		one.Set(7, 0, "Useful signals total");
		one.Set(7, 1, useful_nonzero_signals);
	}
	
	draw.Refresh();
}


















DataGraph::DataGraph(ExportCtrl* dc) : dc(dc) {
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
		if (d <= 100.0 || d >= 10000000.0) continue;
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
				if (v <= 100.0 || v >= 10000000.0) continue;
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














SignalGraph::SignalGraph(ExportCtrl* dc) : dc(dc) {
	
}

void SignalGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	AgentSystem& sys = GetSystem().GetAgentSystem();
	double ydiv = ((double)sz.cy) / ((double)(SYM_COUNT));
	
	Time now = GetSysTime();
	int wday = DayOfWeek(now) - 1;
	double now_sensor = ((wday * 24 + now.hour) * 60 + now.minute) / (5.0 * 24.0 * 60.0);
	int now_x = now_sensor * sz.cx;
	int now_switch_count = -1;
	int prev_now_switch_dist = 0;
	
	int snap_end = sys.snaps.GetCount();
	int begin_x1 = 0;
	bool right_side = false;
	bool is_begin = true;
	
	for (int k = snap_end-1; k >= 0; k--) {
		Snapshot& snap1 = sys.snaps[k];
		int x1 = (snap1.GetTimeSensor(1) * 7.0 - 1.0) / 5.0 * sz.cx;
		int x2;
		if (!is_begin) {
			Snapshot& snap2 = sys.snaps[k+1];
			x2 = (snap2.GetTimeSensor(1) * 7.0 - 1.0) / 5.0 * sz.cx;
			
			if (!right_side && x1 > begin_x1)
				right_side = true;
			if (right_side && x1 <= begin_x1)
				break;
		} else {
			x2 = x1 + sz.cx / 0x100;
			begin_x1 = x1;
			is_begin = false;
		}
		if (x2 < x1) x2 = sz.cx;
		int w = x2 - x1;
		
		for(int j = 0; j < SYM_COUNT; j++) {
			int y1 = (j + 0) * ydiv;
			int y2 = (j + 1) * ydiv;
			int h = y2 - y1;
			
			Color clr;
			int signal = snap1.GetFuseOutput(j);
			if (signal) {
				if (signal > 0)
					clr = Color(137, 209, 135);
				else if (signal < 0)
					clr = Color(134, 154, 213);
				id.DrawRect(x1, y1, w, h, clr);
			}
		}
	}
	
	
	Font fnt = Monospace(ydiv - 2).Bold();
	for(int j = 0; j < SYM_COUNT; j++) {
		int y = j * ydiv;
		String sym = GetSystem().GetSymbol(sys.sym_ids[j]);
		id.DrawText(3, y + 1, sym, fnt, Black());
		id.DrawText(4, y + 2, sym, fnt, GrayColor(128+64));
	}
	
	id.DrawLine(now_x,   0,   now_x, sz.cy, 1, White());
	id.DrawLine(now_x+1, 0, now_x+1, sz.cy, 1, Black());
	
	w.DrawImage(0, 0, id);
}


ExportCtrl::ExportCtrl() :
	graph(this),
	siggraph(this)
{
	last_pos = 0;
	
	timeslider.MinMax(0,1);
	timeslider <<= THISBACK(GuiData);
	
	Add(graph.HSizePos().TopPos(0,120));
	Add(timeslider.TopPos(120,30).LeftPos(0, 200-2));
	Add(data.TopPos(120,30).HSizePos(200));
	Add(hsplit.VSizePos(150, 120).HSizePos());
	Add(siggraph.HSizePos().BottomPos(0,120));
	
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

void ExportCtrl::GuiData() {
	
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

void ExportCtrl::Data() {
	
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
	int cursor = timeslider.GetData();
	if (cursor < 0 || cursor >= poslist.GetCount())
		timeslider.SetData(poslist.GetCount()-1);
	
	last_pos = fin.GetSize();
	
	GuiData();
}

}
