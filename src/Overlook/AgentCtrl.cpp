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
	
	
	AgentGroup& group = *this->group;
	System& sys = *group.sys;
	
	if (snap_id < 0 || snap_id >= group.snaps.GetCount()) {w.DrawRect(sz, White()); return;}
	const Snapshot& snap = group.snaps[snap_id];
	
	int sym_count		= group.sym_ids.GetCount();
	int tf_count		= group.tf_ids.GetCount();
	int value_count		= group.buf_count;
	
	int rows = sym_count * tf_count;
	int cols = value_count;
	int grid_w = sz.cx;
	double xstep = (double)grid_w / (double)cols;
	double ystep = (double)sz.cy / (double)rows;
	
	
	int row = 0;
	for(int i = 0; i < tf_count; i++) {
		for(int j = 0; j < sym_count; j++) {
			int y = row * ystep;
			int y2 = (row + 1) * ystep;
			int h = y2-y;
			
			for(int k = 0; k < value_count; k++) {
				int x = k * xstep;
				int x2 = (k + 1) * xstep;
				int w = x2 - x;
				double d = snap.values[(j * group.tf_ids.GetCount() + i) * value_count + k];
				double min = 0.0;
				double max = 1.0;
				double value = 255.0 * Upp::max(0.0, Upp::min(1.0, d));
				int clr = Upp::min(255.0, value);
				Color c(255 - clr, clr, 0);
				id.DrawRect(x, y, w, h, c);
			}
			
			row++;
		}
	}
	
	w.DrawImage(0,0,id);
}






TrainingGraph::TrainingGraph() {
	trainee = NULL;
	
}

void TrainingGraph::Paint(Draw& w) {
	if (!trainee) {w.DrawRect(GetSize(), White()); return;}
	
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









AgentThreadCtrl::AgentThreadCtrl()
{
	agent = NULL;
	Add(hsplit.SizePos());
	
	hsplit.Horz();
	hsplit << draw << brokerctrl;
	hsplit.SetPos(500, 0);
	
	brokerctrl.ReadOnly();
}

void AgentThreadCtrl::Data() {
	MetaTrader& mt = GetMetaTrader();
	AgentGroup& group = *agent->group;
	System& sys = *group.sys;
	/*SequencerThread& thrd = agent->thrds[thrd_id];
	Snapshot& snap = agent->snaps[thrd.snap_id];
	
	time_slider.MinMax(0, snap.bars-1);
	time_slider.SetData(snap.pos.Top());
	
	draw.SetSnap(thrd.snap_id);
	draw.Refresh();
	
	brokerctrl.SetBroker(thrd.broker);
	brokerctrl.Data();*/
}










/*
AgentCtrl::AgentCtrl()
{
	agent = NULL;
	Add(update_brokerctrl.TopPos(3, 24).LeftPos(2, 96));
	Add(reward.BottomPos(0,200).HSizePos());
	init = true;
	update_brokerctrl.Set(false);
	update_brokerctrl.SetLabel("Update visible");
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
*/







StatsGraph::StatsGraph() {
	trainee = NULL;
	clr = RainbowColor(Randomf());
}

void StatsGraph::Paint(Draw& w) {
	if (!trainee) {w.DrawRect(GetSize(), White()); return;}
	
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
		
		const Vector<double>& data = trainee->thrd_equity;
		int count = data.GetCount();
		if (count >= 2) {
			polyline.SetCount(count);
			for(int j = 0; j < count; j++) {
				double v = data[j];
				double y = sz.cy - (v - min) / diff * sz.cy;
				polyline[j] = Point(j * xstep, y);
				if (v > peak) peak = v;
			}
			last = data[count-1];
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
	
	broker.ReadOnly();
	
	hsplit << draw << broker << timescroll;
	hsplit.Horz();
	hsplit.SetPos(2000, 0);
	hsplit.SetPos(8000, 1);
	
	bsplit.Horz();
	bsplit << stats << reward;
	
	init = true;
	
}

void TrainingCtrl::SetTrainee(TraineeBase& trainee) {
	this->trainee = &trainee;
	
	broker.SetBroker(trainee.broker);
	stats.SetTrainee(trainee);
	reward.SetTrainee(trainee);
	draw.SetGroup(*trainee.group);
	timescroll.SetGraph(trainee.dqn.GetGraph());
}

void TrainingCtrl::Data() {
	if (!trainee) return;
	
	if (init) {
		init = false;
		//broker.Clear();
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
	*/
	
	
	broker.Data();
	draw.SetSnap(trainee->epoch_actual);
	draw.Refresh();
	timescroll.Refresh();
	reward.Refresh();
	stats.Refresh();
	reward.Refresh();
}







RealtimeNetworkCtrl::RealtimeNetworkCtrl()
{
	agent = NULL;
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
	
	//brokerctrl.SetBroker(agent.latest_broker);
	
}

void RealtimeNetworkCtrl::Data() {
	/*if (agent->latest_broker.AccountEquity() > agent->latest_broker.GetInitialBalance()) {
		tfcmplbl.SetLabel("Network can be used in trading.");
		tfcmplbl.SetInk(Color(28, 85, 0));
	} else {
		tfcmplbl.SetLabel("Network can NOT be used in trading.");
		tfcmplbl.SetInk(Color(109, 0, 26));
	}
	brokerctrl.Data();
	draw.Refresh();*/
}

void RealtimeNetworkCtrl::RefreshSignals() {
	//rtses->PostEvent(RealtimeSession::EVENT_REFRESH);
}

void RealtimeNetworkCtrl::KillSignals() {
	//rtses->PostEvent(RealtimeSession::EVENT_KILL);
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
	
	list <<= THISBACK(Data);
}

void SnapshotCtrl::Data() {
	if (!group) return;
	
	AgentGroup& group = *this->group;
	int cursor = list.GetCursor();
	
	for(int i = 0; i < group.snaps.GetCount(); i++) {
		const Snapshot& snap = group.snaps[i];
		list.Set(i, 0, i);
		list.Set(i, 1, snap.time);
		list.Set(i, 2, snap.added);
	}
	
	if (cursor >= 0 && cursor < group.snaps.GetCount())
		draw.SetSnap(cursor);
	
	draw.Refresh();
}

}
