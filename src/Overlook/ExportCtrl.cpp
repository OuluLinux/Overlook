#include "Overlook.h"

namespace Overlook {
using namespace Upp;

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
	
	/*
	
	
	// TODO: this must be updated to latest expert system when stable
	
	
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
	id.DrawLine(now_x+1, 0, now_x+1, sz.cy, 1, Black());*/
	
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
	
	FileIn fin(ConfigFile("expertsystem.log"));
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
	
	FileIn fin(ConfigFile("expertsystem.log"));
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
