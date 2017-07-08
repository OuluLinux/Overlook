#include "Overlook.h"

namespace Overlook {
using namespace Upp;

AgentDraw::AgentDraw(Agent& agent) : agent(&agent) {
	
}

void AgentDraw::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	
	/*Agent& agent = *this->agent;
	System& sys = *agent.sys;
	
	
	const Snapshot& snap = agent.snaps[0];
	
	int sym_count		= agent.sym_ids.GetCount();
	int tf_count		= snap.pos.GetCount();
	int value_count		= snap.value_count;
	
	int rows = sym_count * tf_count;
	int cols = value_count;
	int grid_w = sz.cx;
	double xstep = (double)grid_w / (double)cols;
	double ystep = (double)sz.cy / (double)rows;
	
	
	int row = 0;
	for(int i = 0; i < tf_count; i++) {
		const Vector<Vector<DoubleTrio> >& sym_values = snap.value[i];
		const Vector<double>& min_values = snap.min_value[i];
		const Vector<double>& max_values = snap.max_value[i];
		
		for(int j = 0; j < sym_count; j++) {
			const Vector<DoubleTrio>& values = sym_values[j];
			
			int y = row * ystep;
			int y2 = (row + 1) * ystep;
			int h = y2-y;
			
			for(int k = 0; k < value_count; k++) {
				int x = k * xstep;
				int x2 = (k + 1) * xstep;
				int w = x2 - x;
				double min = min_values[k];
				double max = max_values[k];
				double value = 255.0 * (values[k].a - min) / (max - min);
				int clr = Upp::min(255.0, value);
				Color c(255 - clr, clr, 0);
				id.DrawRect(x, y, w, h, c);
			}
			
			row++;
		}
	}*/
	
	w.DrawImage(0,0,id);
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
	
	prev_thrd = NULL;
}

void AgentThreadCtrl::Data() {
	/*
	if (agent->thrds[thrd_id].IsEmpty()) return;
	
	MetaTrader& mt = GetMetaTrader();
	System& sys = *agent->sys;
	Snapshot& snap = agent->snaps[thrd_id];
	SequencerThread& thrd = *agent->thrds[thrd_id];
	
	if (prev_thrd != &thrd) {
		prev_thrd = &thrd;
		conv.Clear();
		conv.SetSession(thrd.ses);
		timescroll.SetSession(thrd.ses);
		conv.RefreshLayers();
	}
	
	time_slider.MinMax(0, snap.bars-1);
	time_slider.SetData(snap.pos.Top());
	
	prog.Set(thrd.epoch_actual, thrd.epoch_total);
	epoch.SetLabel("Epoch: " + IntStr(thrd.epochs));
	
	draw.Refresh();
	conv.Refresh();
	timescroll.Refresh();
	reward.Refresh();*/
}











AgentCtrl::AgentCtrl(Agent& agent) :
	agent(&agent),
	reward(3, agent)
{
	Add(thrdlist.TopPos(3, 24).LeftPos(4, 100));
	Add(reward.BottomPos(0,200).HSizePos());
	init = true;
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
	int i = thrdlist.GetIndex();
	if (i == -1) return;
	thrds[i].Data();
}

void AgentCtrl::SetView() {
	for(int i = 0; i < thrds.GetCount(); i++)
		thrds[i].Hide();
	int i = thrdlist.GetIndex();
	thrds[i].Show();
}







StatsGraph::StatsGraph(int mode, Agent& agent) : agent(&agent), mode(mode) {
	
	
	
}

const ConvNet::Window& StatsGraph::GetData(int thrd) {
	return null_win;
	/*
	switch (mode) {
		case 0:		if (agent->thrds[thrd].IsEmpty()) return null_win;
					return agent->thrds[thrd]->test_window1;
		
		case 1:		if (agent->thrds[thrd].IsEmpty()) return null_win;
					return agent->thrds[thrd]->test_window0;
		
		case 2:		if (agent->thrds[thrd].IsEmpty()) return null_win;
					return agent->thrds[thrd]->loss_window;
		
		case 3:		if (agent->thrds[thrd].IsEmpty()) return null_win;
					return agent->thrds[thrd]->reward_window;
		
		case 4:		return agent->thrd_priorities[thrd];
		
		case 5:		return agent->thrd_performances[thrd];
		
		case 6:		return agent->seq_results;
		
		case 7:		if (agent->sessions.IsEmpty()) return null_win;
					return agent->sessions[agent->session_cur].loss_window;
		
		case 8:		if (agent->sessions.IsEmpty()) return null_win;
					return agent->sessions[agent->session_cur].reward_window;
		
		case 9:		if (agent->sessions.IsEmpty()) return null_win;
					return agent->sessions[agent->session_cur].test_reward_window;
		
		case 10:	if (agent->sessions.IsEmpty()) return null_win;
					return agent->sessions[agent->session_cur].test_window0;
		
		case 11:	if (agent->sessions.IsEmpty()) return null_win;
					return agent->sessions[agent->session_cur].train_broker;
	}
	Panic("Invalid mode");*/
}

void StatsGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	int thrd_count = mode < 6 ? agent->thrds.GetCount() : 1;
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int max_steps = 0;
	for(int i = 0; i < thrd_count; i++) {
		const ConvNet::Window& data = GetData(i);
		int count = data.GetBufferCount();
		for(int j = 0; j < count; j++) {
			double d = data.Get(j);
			if (d > max) max = d;
			if (d < min) min = d;
		}
		if (count > max_steps)
			max_steps = count;
	}
	
	
	if (max_steps > 1) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		for(int i = 0; i < thrd_count; i++) {
			const ConvNet::Window& data = GetData(i);
			int count = data.GetBufferCount();
			if (count >= 2) {
				polyline.SetCount(count);
				for(int j = 0; j < count; j++) {
					double y = sz.cy - (data.Get(j) - min) / diff * sz.cy;
					polyline[j] = Point(j * xstep, y);
				}
				id.DrawPolyline(polyline, 1, RainbowColor((double)i / thrd_count));
			}
		}
		
		for(int i = 0; i < thrd_count; i++) {
			int y = i * 13;
			String str = DblStr(GetData(i).GetAverage());
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
	draw(agent)
{
	Add(epoch.TopPos(3, 24).LeftPos(350, 100));
	Add(prog.TopPos(3, 24).HSizePos(450));
	Add(hsplit.VSizePos(30).HSizePos());
	
	hsplit << seslist << draw << conv << timescroll;
	hsplit.Horz();
	hsplit.SetPos(1500, 0);
	hsplit.SetPos(2000, 1);
	hsplit.SetPos(8000, 2);
	
	
	seslist.AddColumn("#");
	seslist.AddColumn("Tf");
	seslist.AddColumn("Profit");
	seslist.AddColumn("Orders");
	seslist.AddColumn("Test-Signal");
	seslist <<= THISBACK(Data);
}
	
void AgentTraining::Data() {
	/*
	for(int i = 0; i < agent->sessions.GetCount(); i++) {
		const SequencerThread& st = agent->sessions[i];
		seslist.Set(i, 0, st.id);
		//seslist.Set(i, 1, st.train_brokerprofit);
		//seslist.Set(i, 2, st.train_brokerorders);
		//seslist.Set(i, 3, st.total_sigchange);
	}
	agent->session_cur = seslist.GetCursor();
	if (agent->session_cur == -1) agent->session_cur = 0;
	*/
	
	
}





TfCompDraw::TfCompDraw(Agent& agent) :
	agent(&agent)
{
	
}

void TfCompDraw::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	
	
	w.DrawImage(0, 0, id);
}





RealtimeNetworkCtrl::RealtimeNetworkCtrl(Agent& agent, RealtimeSession& rtses) :
	agent(&agent), rtses(&rtses),
	draw(agent),
	tfcmp(agent)
{
	Add(hsplit.VSizePos(150).HSizePos());
	Add(tfcmplbl.TopPos(0,150).LeftPos(100, 200));
	Add(tfcmp.TopPos(0,150).HSizePos(300));
	
	hsplit << draw << brokerctrl;
	hsplit.Horz();
	hsplit.SetPos(500);
	
	Add(refresh_signals.TopPos(2, 26).LeftPos(2, 96));
	refresh_signals.SetLabel("Refresh Signals");
	refresh_signals <<= THISBACK(RefreshSignals);
	
	Add(killall_signals.TopPos(32, 26).LeftPos(2, 96));
	killall_signals.SetLabel("Kill All Signals");
	killall_signals <<= THISBACK(KillSignals);
	
	tfcmplbl.SetLabel("<No-brainer assertions>");
}

void RealtimeNetworkCtrl::Data() {
	
}

void RealtimeNetworkCtrl::RefreshSignals() {
	rtses->PostEvent(RealtimeSession::EVENT_REFRESH);
}

void RealtimeNetworkCtrl::KillSignals() {
	rtses->PostEvent(RealtimeSession::EVENT_KILL);
}


}
