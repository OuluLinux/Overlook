#include "Overlook.h"

namespace Overlook {
using namespace Upp;

TrainerDraw::TrainerDraw(TrainerThreadCtrl& ctrl) : ctrl(&ctrl) {
	
}

void TrainerDraw::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	
	TrainerThreadCtrl& ctrl = *this->ctrl;
	Trainer& trainer = *ctrl.trainer;
	System& sys = *trainer.sys;
	
	
	const Iterator& iter = trainer.iters[0];
	
	int sym_count		= trainer.sym_ids.GetCount();
	int tf_count		= iter.pos.GetCount();
	int value_count		= iter.value_count;
	
	int rows = sym_count * tf_count;
	int cols = value_count;
	int grid_w = sz.cx;
	double xstep = (double)grid_w / (double)cols;
	double ystep = (double)sz.cy / (double)rows;
	
	
	int row = 0;
	for(int i = 0; i < tf_count; i++) {
		const Vector<Vector<DoubleTrio> >& sym_values = iter.value[i];
		const Vector<double>& min_values = iter.min_value[i];
		const Vector<double>& max_values = iter.max_value[i];
		
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
	}
	
	w.DrawImage(0,0,id);
}





TrainerThreadCtrl::TrainerThreadCtrl(Trainer& trainer, int thrd_id) :
	trainer(&trainer),
	draw(*this),
	thrd_id(thrd_id),
	reward(3, trainer)
{
	Add(time_lbl.TopPos(3, 24).LeftPos(0, 46));
	Add(time_slider.TopPos(3, 24).LeftPos(50, 300));
	Add(epoch.TopPos(3, 24).LeftPos(350, 100));
	Add(prog.TopPos(3, 24).HSizePos(450));
	Add(hsplit.VSizePos(30,200).HSizePos());
	Add(reward.BottomPos(0,200).HSizePos());
	
	hsplit.Horz();
	hsplit << draw << conv << timescroll;
	hsplit.SetPos(2000, 0);
	hsplit.SetPos(8000, 1);
	
	time_lbl.SetLabel("Time:");
	time_lbl.AlignRight();
	time_slider.MinMax(0, 100).SetData(50);
	
	prev_thrd = NULL;
}

void TrainerThreadCtrl::Data() {
	if (trainer->thrds[thrd_id].IsEmpty()) return;
	
	MetaTrader& mt = GetMetaTrader();
	System& sys = *trainer->sys;
	Iterator& iter = trainer->iters[thrd_id];
	SessionThread& thrd = *trainer->thrds[thrd_id];
	
	if (prev_thrd != &thrd) {
		prev_thrd = &thrd;
		conv.SetSession(thrd.ses);
		timescroll.SetSession(thrd.ses);
		conv.RefreshLayers();
	}
	
	time_slider.MinMax(0, iter.bars-1);
	time_slider.SetData(iter.pos.Top());
	
	prog.Set(thrd.epoch_actual, thrd.epoch_total);
	epoch.SetLabel("Epoch: " + IntStr(thrd.epochs));
	
	draw.Refresh();
	conv.Refresh();
	timescroll.Refresh();
	reward.Refresh();
}











TrainerCtrl::TrainerCtrl(Trainer& trainer) :
	trainer(&trainer)
{
	Add(thrdlist.TopPos(3, 24).LeftPos(4, 100));
	init = true;
}

void TrainerCtrl::Data() {
	if (init) {
		init = false;
		for(int i = 0; i < trainer->thrds.GetCount(); i++) {
			Ctrl& ctrl = thrds.Add(new TrainerThreadCtrl(*trainer, i));
			thrdlist.Add("Thread " + IntStr(i) + ".");
			Add(ctrl.HSizePos().VSizePos(30));
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

void TrainerCtrl::SetView() {
	for(int i = 0; i < thrds.GetCount(); i++)
		thrds[i].Hide();
	int i = thrdlist.GetIndex();
	thrds[i].Show();
}







StatsGraph::StatsGraph(int mode, Trainer& t) : trainer(&t), mode(mode) {
	
	
	
}

const ConvNet::Window& StatsGraph::GetData(int thrd) {
	switch (mode) {
		case 0:		if (trainer->thrds[thrd].IsEmpty()) return null_win;
					return trainer->thrds[thrd]->test_window1;
		
		case 1:		if (trainer->thrds[thrd].IsEmpty()) return null_win;
					return trainer->thrds[thrd]->test_window0;
		
		case 2:		if (trainer->thrds[thrd].IsEmpty()) return null_win;
					return trainer->thrds[thrd]->loss_window;
		
		case 3:		if (trainer->thrds[thrd].IsEmpty()) return null_win;
					return trainer->thrds[thrd]->reward_window;
		
		case 4:		return trainer->thrd_priorities[thrd];
		
		case 5:		return trainer->thrd_performances[thrd];
		
		case 6:		return trainer->ses_sigchanges;
		
		case 7:		if (trainer->sessions.IsEmpty()) return null_win;
					return trainer->sessions[trainer->session_cur].loss_window;
		
		case 8:		if (trainer->sessions.IsEmpty()) return null_win;
					return trainer->sessions[trainer->session_cur].reward_window;
		
		case 9:		if (trainer->sessions.IsEmpty()) return null_win;
					return trainer->sessions[trainer->session_cur].test_reward_window;
		
		case 10:	if (trainer->sessions.IsEmpty()) return null_win;
					return trainer->sessions[trainer->session_cur].test_window0;
		
		case 11:	if (trainer->sessions.IsEmpty()) return null_win;
					return trainer->sessions[trainer->session_cur].train_broker;
	}
	Panic("Invalid mode");
}

void StatsGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	int thrd_count = mode < 6 ? trainer->thrds.GetCount() : 1;
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












TrainerResult::TrainerResult(Trainer& trainer) :
	trainer(&trainer),
	graph(6, trainer),
	reward(8, trainer),
	loss(7, trainer),
	avdepth(9, trainer),
	sigtotal(10, trainer),
	sigbroker(11, trainer)
{
	Add(hsplit.SizePos());
	
	hsplit << seslist << vsplit;
	hsplit.SetPos(3000);
	hsplit.Horz();
	
	vsplit.Vert();
	vsplit << graph << reward << loss << avdepth << sigtotal << sigbroker;
	
	
	seslist.AddColumn("#");
	seslist.AddColumn("Profit");
	seslist.AddColumn("Orders");
	seslist.AddColumn("Test-Signal");
	seslist <<= THISBACK(Data);
}
	
void TrainerResult::Data() {
	for(int i = 0; i < trainer->sessions.GetCount(); i++) {
		const SessionThread& st = trainer->sessions[i];
		seslist.Set(i, 0, st.id);
		seslist.Set(i, 1, st.train_brokerprofit);
		seslist.Set(i, 2, st.train_brokerorders);
		seslist.Set(i, 3, st.total_sigchange);
	}
	trainer->session_cur = seslist.GetCursor();
	if (trainer->session_cur == -1) trainer->session_cur = 0;
	
	graph.Refresh();
	reward.Refresh();
	loss.Refresh();
	avdepth.Refresh();
	sigtotal.Refresh();
}










RealtimeNetworkCtrl::RealtimeNetworkCtrl(Trainer& trainer, RealtimeSession& rtses) :
	trainer(&trainer), rtses(&rtses)
{
	Add(refresh_signals.TopPos(2, 26).LeftPos(2, 96));
	refresh_signals.SetLabel("Refresh Signals");
	refresh_signals <<= THISBACK(RefreshSignals);
}

void RealtimeNetworkCtrl::Data() {
	
}

void RealtimeNetworkCtrl::RefreshSignals() {
	rtses->PostEvent(RealtimeSession::EVENT_REFRESH);
}


}


