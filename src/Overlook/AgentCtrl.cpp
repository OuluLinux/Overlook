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
	
	int tf_count		= snap.pos.GetCount();
	int value_count		= agent.buf_count * 2;
	
	int rows = tf_count;
	int cols = value_count;
	int grid_w = sz.cx;
	double xstep = (double)grid_w / (double)cols;
	double ystep = (double)sz.cy / (double)rows;
	
	
	int row = 0;
	for(int i = 0; i < tf_count; i++) {
		int y = row * ystep;
		int y2 = (row + 1) * ystep;
		int h = y2-y;
		
		for(int k = 0; k < value_count; k++) {
			int x = k * xstep;
			int x2 = (k + 1) * xstep;
			int w = x2 - x;
			double d = snap.values[i * value_count + k];
			double value = 255.0 *  Upp::min(1.0, Upp::max(0.0, d));
			int clr = Upp::min(255.0, value);
			Color c(255 - clr, clr, 0);
			id.DrawRect(x, y, w, h, c);
		}
		
		row++;
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
	/*SequencerThread& thrd = agent->thrds[thrd_id];
	Snapshot& snap = agent->snaps[thrd.snap_id];
	
	time_slider.MinMax(0, snap.bars-1);
	time_slider.SetData(snap.pos.Top());
	
	draw.SetSnap(thrd.snap_id);
	draw.Refresh();
	
	brokerctrl.SetBroker(thrd.broker);
	brokerctrl.Data();*/
}











AgentCtrl::AgentCtrl(Agent& agent) :
	agent(&agent),
	reward(agent)
{
	Add(thrdlist.TopPos(3, 24).LeftPos(2, 96));
	Add(update_brokerctrl.TopPos(3, 24).LeftPos(102, 96));
	Add(reward.BottomPos(0,200).HSizePos());
	init = true;
	update_brokerctrl.Set(false);
	update_brokerctrl.SetLabel("Update visible");
}

void AgentCtrl::Data() {
	/*if (init) {
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
	}*/
	
	
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
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	double last = 0.0;
	
	int max_steps = 0;
	const Vector<double>& data = agent->thrd_equity;
	int count = data.GetCount();
	for(int j = 0; j < count; j++) {
		double d = data[j];
		if (d == 0.0) break;
		if (d > max) max = d;
		if (d < min) min = d;
	}
	if (count > max_steps)
		max_steps = count;
	
	
	if (max_steps > 1 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		const Vector<double>& data = agent->thrd_equity;
		int count = data.GetCount();
		if (count >= 2) {
			polyline.SetCount(count);
			for(int j = 0; j < count; j++) {
				double v = data[j];
				double y = sz.cy - (v - min) / diff * sz.cy;
				polyline[j] = Point(j * xstep, y);
			}
			last = data[count-1];
			if (polyline.GetCount() >= 2)
				id.DrawPolyline(polyline, 1, RainbowColor(Randomf()));
		}
		
		int y = 0;
		String str = DblStr(last);
		Size str_sz = GetTextSize(str, fnt);
		id.DrawRect(3, y, 13, 13, RainbowColor(Randomf()));
		id.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
		id.DrawText(16, y, str, fnt, Black());
	}
	
	
	w.DrawImage(0, 0, id);
}












AgentTraining::AgentTraining(Agent& agent) :
	agent(&agent),
	draw(agent),
	reward(agent)
{
	Add(paused.TopPos(3, 24).LeftPos(2, 196));
	Add(prefer_highresults.TopPos(3, 24).LeftPos(202, 196));
	Add(lbl_fmlevel.TopPos(3, 24).LeftPos(402, 96));
	Add(fmlevel.TopPos(3, 24).LeftPos(502, 96));
	
	Add(hsplit.VSizePos(0, 200).HSizePos());
	Add(reward.BottomPos(0, 200).HSizePos());
	
	lbl_fmlevel.SetLabel("Free-margin level:");
	fmlevel.SetData(agent.global_free_margin_level);
	fmlevel.MinMax(0.60, 0.99);
	fmlevel.SetInc(0.01);
	fmlevel <<= THISBACK(SetFreeMarginLevel);
	
	hsplit << leftctrl << draw << conv << timescroll;
	hsplit.Horz();
	hsplit.SetPos(1500, 0);
	hsplit.SetPos(2000, 1);
	hsplit.SetPos(8000, 2);
	
	
	seslist.AddColumn("#");
	seslist.AddColumn("Profit");
	seslist.AddColumn("Orders");
	seslist.ColumnWidths("1 3 2");
	seslist <<= THISBACK(Data);
	
	
	paused.SetLabel("Paused");
	paused.Set(agent.paused);
	paused <<= THISBACK(SetPaused);
	
	
	prefer_highresults.SetLabel("Prefer high values");
	prefer_highresults.Set(agent.prefer_high);
	prefer_highresults <<= THISBACK(SetPreferHigh);
	
	init = true;
	
	leftctrl.Vert();
	leftctrl << seslist << graph << settings;
	lrate.SetLabel("Learning rate:");
	lmom.SetLabel("Momentum:");
	lbatch.SetLabel("Batch size:");
	ldecay.SetLabel("Weight decay:");
	apply.SetLabel("Apply");
	apply <<= THISBACK(ApplySettings);
	int row = 20;
	settings.Add(lrate.HSizePos(4,4).TopPos(0,row));
	settings.Add(rate.HSizePos(4,4).TopPos(1*row,row));
	settings.Add(lmom.HSizePos(4,4).TopPos(2*row,row));
	settings.Add(mom.HSizePos(4,4).TopPos(3*row,row));
	settings.Add(lbatch.HSizePos(4,4).TopPos(4*row,row));
	settings.Add(batch.HSizePos(4,4).TopPos(5*row,row));
	settings.Add(ldecay.HSizePos(4,4).TopPos(6*row,row));
	settings.Add(decay.HSizePos(4,4).TopPos(7*row,row));
	settings.Add(apply.HSizePos(4,4).TopPos(8*row,row));
	rate.SetData(0.01);
	mom.SetData(0.9);
	batch.SetData(20);
	decay.SetData(0.001);
}
	
void AgentTraining::Data() {
	
	
	if (init) {
		init = false;
		conv.Clear();
		/*conv.SetSession(agent->ses);
		timescroll.SetSession(agent->ses);
		conv.RefreshLayers();
		
		prefer_highresults.Set(agent->prefer_high);
		fmlevel.SetData(agent->global_free_margin_level);
		
		TrainerBase* t = agent->ses.GetTrainer();
		if (t) {
			TrainerBase& trainer = *t;
			rate.SetData(trainer.GetLearningRate());
			mom.SetData(trainer.GetMomentum());
			batch.SetData(trainer.GetBatchSize());
			decay.SetData(trainer.GetL2Decay());
		}*/
	}
	/*
	for(int i = 0; i < agent->sequences.GetCount(); i++) {
		const Sequence& seq = agent->sequences[i];
		seslist.Set(i, 0, seq.id);
		seslist.Set(i, 1, seq.equity);
		seslist.Set(i, 2, seq.orders);
	}
	agent->session_cur = seslist.GetCursor();
	if (agent->session_cur == -1) agent->session_cur = 0;
	*/
	
	draw.SetSnap(0);
	draw.Refresh();
	conv.Refresh();
	timescroll.Refresh();
	reward.Refresh();
}

void AgentTraining::ApplySettings() {
	/*TrainerBase* t = agent->ses.GetTrainer();
	if (!t) return;
	TrainerBase& trainer = *t;
	trainer.SetLearningRate(rate.GetData());
	trainer.SetMomentum(mom.GetData());
	trainer.SetBatchSize(batch.GetData());
	trainer.SetL2Decay(decay.GetData());*/
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
	//rtses->PostEvent(RealtimeSession::EVENT_REFRESH);
}

void RealtimeNetworkCtrl::KillSignals() {
	//rtses->PostEvent(RealtimeSession::EVENT_KILL);
}














SnapshotCtrl::SnapshotCtrl(Agent& agent) :
	agent(&agent),
	draw(agent)
{
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << draw << list;
	hsplit.SetPos(2000);
	
	list.AddColumn("#");
	list.AddColumn("Time");
	list.AddColumn("Added");
	list.AddColumn("Is valid");
	
	list <<= THISBACK(Data);
}

void SnapshotCtrl::Data() {
	int cursor = list.GetCursor();
	
	for(int i = 0; i < agent->snaps.GetCount(); i++) {
		const Snapshot& snap = agent->snaps[i];
		list.Set(i, 0, i);
		list.Set(i, 1, snap.time);
		list.Set(i, 2, snap.added);
		list.Set(i, 3, snap.is_valid ? "Valid" : "Invalid");
	}
	
	if (cursor >= 0 && cursor < agent->snaps.GetCount())
		draw.SetSnap(cursor);
	
	draw.Refresh();
}

}
