#include "Overlook.h"

namespace Overlook {
using namespace Upp;

AgentDraw::AgentDraw(Agent& agent) : agent(&agent) {
	snap_id = -1;
}

void AgentDraw::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	
	Agent& agent = *this->agent;
	System& sys = *agent.sys;
	
	if (snap_id < -1 || snap_id >= agent.snaps.GetCount()) {w.DrawRect(sz, White()); return;}
	const Snapshot& snap = snap_id >= 0 ? agent.snaps[snap_id] : agent.latest_snap;
	
	int sym_count		= agent.sym_ids.GetCount();
	int tf_count		= snap.pos.GetCount();
	int value_count		= agent.buf_count;
	
	int rows = sym_count * tf_count;
	int cols = value_count;
	int grid_w = sz.cx;
	double xstep = (double)grid_w / (double)cols;
	double ystep = (double)sz.cy / (double)rows;
	
	
	int row = 0;
	for(int i = 0; i < tf_count; i++) {
		//const Vector<Vector<DoubleTrio> >& sym_values = snap.value[i];
		const Vector<double>& min_values = snap.min_value[i];
		const Vector<double>& max_values = snap.max_value[i];
		
		for(int j = 0; j < sym_count; j++) {
			//const Vector<DoubleTrio>& values = sym_values[j];
			
			int y = row * ystep;
			int y2 = (row + 1) * ystep;
			int h = y2-y;
			
			for(int k = 0; k < value_count; k++) {
				int x = k * xstep;
				int x2 = (k + 1) * xstep;
				int w = x2 - x;
				double d = snap.volume_in.Get(j, i, k);
				double min = min_values[k];
				double max = max_values[k];
				double value = 255.0 * (d - min) / (max - min);
				int clr = Upp::min(255.0, value);
				Color c(255 - clr, clr, 0);
				id.DrawRect(x, y, w, h, c);
			}
			
			row++;
		}
	}
	
	w.DrawImage(0,0,id);
}






TrainingGraph::TrainingGraph(Agent& agent) : agent(&agent) {
	
}

void TrainingGraph::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	Agent& agent = *this->agent;
	const Vector<double>& data = agent.GetSequenceResults();
	
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
				double y = sz.cy - (v - min) / diff * sz.cy;
				int x = j * xstep;
				polyline[j] = Point(x, y);
			}
			id.DrawPolyline(polyline, 1, Color(193, 255, 255));
			for(int j = 0; j < polyline.GetCount(); j++) {
				const Point& p = polyline[j];
				id.DrawRect(p.x-1, p.y-1, 3, 3, Blue());
			}
		}
	}
	
	
	w.DrawImage(0, 0, id);
}









AgentThreadCtrl::AgentThreadCtrl(Agent& agent, int thrd_id) :
	agent(&agent),
	thrd_id(thrd_id),
	draw(agent)
{
	Add(hsplit.SizePos());
	
	hsplit.Horz();
	hsplit << draw << brokerctrl;
	hsplit.SetPos(500, 0);
	
	brokerctrl.ReadOnly();
}

void AgentThreadCtrl::Data() {
	MetaTrader& mt = GetMetaTrader();
	System& sys = *agent->sys;
	SequencerThread& thrd = agent->thrds[thrd_id];
	Snapshot& snap = agent->snaps[thrd.snap_id];
	
	time_slider.MinMax(0, snap.bars-1);
	time_slider.SetData(snap.pos.Top());
	
	draw.SetSnap(thrd.snap_id);
	draw.Refresh();
	
	brokerctrl.SetBroker(thrd.broker);
	brokerctrl.Data();
}











AgentCtrl::AgentCtrl(Agent& agent) :
	agent(&agent),
	reward(agent)
{
	Add(thrdlist.TopPos(3, 24).LeftPos(5, 96));
	Add(update_brokerctrl.TopPos(3, 24).LeftPos(102, 96));
	Add(reward.BottomPos(0,200).HSizePos());
	init = true;
	update_brokerctrl.Set(false);
	update_brokerctrl.SetLabel("Update broker");
}

void AgentCtrl::Data() {
	if (init) {
		init = false;
		for(int i = 0; i < agent->thrds.GetCount(); i++) {
			Ctrl& ctrl = thrds.Add(new AgentThreadCtrl(*agent, i));
			thrdlist.Add("Thread " + IntStr(i) + ".");
			Add(ctrl.HSizePos().VSizePos(30,200));
			ctrl.Hide();
		}
		thrdlist.SetIndex(0);
		thrdlist <<= THISBACK(SetView);
		thrds[0].Show();
	}
	
	if (update_brokerctrl.Get()) {
		int i = thrdlist.GetIndex();
		if (i == -1) return;
		thrds[i].Data();
	}
	
	
	reward.Refresh();
}

void AgentCtrl::SetView() {
	for(int i = 0; i < thrds.GetCount(); i++)
		thrds[i].Hide();
	int i = thrdlist.GetIndex();
	thrds[i].Show();
}







StatsGraph::StatsGraph(Agent& agent) : agent(&agent) {
	
	
	
}

void StatsGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	int thrd_count = agent->thrd_equities.GetCount();
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	last.SetCount(thrd_count);
	
	int max_steps = 0;
	for(int i = 0; i < thrd_count; i++) {
		const Vector<double>& data = agent->thrd_equities[i];
		int count = data.GetCount();
		for(int j = 0; j < count; j++) {
			double d = data[j];
			if (d == 0.0) break;
			if (d > max) max = d;
			if (d < min) min = d;
		}
		if (count > max_steps)
			max_steps = count;
	}
	
	
	if (max_steps > 1 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		for(int i = 0; i < thrd_count; i++) {
			const Vector<double>& data = agent->thrd_equities[i];
			int count = data.GetCount();
			if (count >= 2) {
				polyline.SetCount(count);
				for(int j = 0; j < count; j++) {
					double v = data[j];
					double y = sz.cy - (v - min) / diff * sz.cy;
					polyline[j] = Point(j * xstep, y);
				}
				last[i] = data[count-1];
				if (polyline.GetCount() >= 2)
					id.DrawPolyline(polyline, 1, RainbowColor((double)i / thrd_count));
			}
		}
		
		for(int i = 0; i < thrd_count; i++) {
			int y = i * 13;
			String str = DblStr(last[i]);
			Size str_sz = GetTextSize(str, fnt);
			id.DrawRect(3, y, 13, 13, RainbowColor((double)i / thrd_count));
			id.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
			id.DrawText(16, y, str, fnt, Black());
		}
	}
	
	
	w.DrawImage(0, 0, id);
}












AgentTraining::AgentTraining(Agent& agent) :
	agent(&agent),
	draw(agent),
	reward(agent)
{
	Add(epoch.TopPos(3, 24).LeftPos(2, 96));
	Add(prog.TopPos(3, 24).HSizePos(100));
	Add(hsplit.VSizePos(30, 200).HSizePos());
	Add(reward.BottomPos(0,200).HSizePos());
	
	hsplit << seslist << draw << conv << timescroll;
	hsplit.Horz();
	hsplit.SetPos(1500, 0);
	hsplit.SetPos(2000, 1);
	hsplit.SetPos(8000, 2);
	
	
	seslist.AddColumn("#");
	seslist.AddColumn("Profit");
	seslist.AddColumn("Orders");
	seslist.ColumnWidths("1 3 2");
	seslist <<= THISBACK(Data);
	
	init = true;
}
	
void AgentTraining::Data() {
	prog.Set(agent->epoch_actual, agent->epoch_total);
	epoch.SetLabel("Epoch: " + IntStr(agent->epochs));
	
	
	if (init) {
		init = false;
		conv.Clear();
		conv.SetSession(agent->ses);
		timescroll.SetSession(agent->ses);
		conv.RefreshLayers();
	}
	
	for(int i = 0; i < agent->sequences.GetCount(); i++) {
		const Sequence& seq = agent->sequences[i];
		seslist.Set(i, 0, seq.id);
		seslist.Set(i, 1, seq.equity);
		seslist.Set(i, 2, seq.orders);
	}
	agent->session_cur = seslist.GetCursor();
	if (agent->session_cur == -1) agent->session_cur = 0;
	
	
	draw.SetSnap(agent->epoch_actual);
	draw.Refresh();
	conv.Refresh();
	timescroll.Refresh();
	reward.Refresh();
}








RealtimeNetworkCtrl::RealtimeNetworkCtrl(Agent& agent, RealtimeSession& rtses) :
	agent(&agent), rtses(&rtses),
	draw(agent)
{
	Add(hsplit.VSizePos(30).HSizePos());
	Add(tfcmplbl.TopPos(2,26).LeftPos(202, 196));
	
	brokerctrl.ReadOnly();
	
	hsplit << draw << brokerctrl;
	hsplit.Horz();
	hsplit.SetPos(500);
	
	Add(refresh_signals.TopPos(2, 26).LeftPos(2, 96));
	refresh_signals.SetLabel("Refresh Signals");
	refresh_signals <<= THISBACK(RefreshSignals);
	
	Add(killall_signals.TopPos(2, 26).LeftPos(102, 96));
	killall_signals.SetLabel("Kill All Signals");
	killall_signals <<= THISBACK(KillSignals);
	
	tfcmplbl.SetLabel("<No-brainer assertions>");
	
	draw.SetSnap(-1);
	brokerctrl.SetBroker(agent.latest_broker);
}

void RealtimeNetworkCtrl::Data() {
	if (agent->latest_broker.AccountEquity() > agent->latest_broker.GetInitialBalance()) {
		tfcmplbl.SetLabel("Network can be used in trading.");
		tfcmplbl.SetInk(Color(28, 85, 0));
	} else {
		tfcmplbl.SetLabel("Network can NOT be used in trading.");
		tfcmplbl.SetInk(Color(109, 0, 26));
	}
	brokerctrl.Data();
	draw.Refresh();
}

void RealtimeNetworkCtrl::RefreshSignals() {
	rtses->PostEvent(RealtimeSession::EVENT_REFRESH);
}

void RealtimeNetworkCtrl::KillSignals() {
	rtses->PostEvent(RealtimeSession::EVENT_KILL);
}


}
