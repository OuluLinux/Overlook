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
	thrd_id(thrd_id)
{
	Add(time_lbl.TopPos(3, 24).LeftPos(0, 46));
	Add(time_slider.TopPos(3, 24).LeftPos(50, 300));
	Add(epoch.TopPos(3, 24).LeftPos(350, 100));
	Add(prog.TopPos(3, 24).HSizePos(450));
	Add(hsplit.VSizePos(30).HSizePos());
	
	hsplit.Horz();
	hsplit << draw << conv << timescroll;
	hsplit.SetPos(2000, 0);
	hsplit.SetPos(8000, 1);
	
	time_lbl.SetLabel("Time:");
	time_lbl.AlignRight();
	time_slider.MinMax(0, 100).SetData(50);
	
	init = true;
}

void TrainerThreadCtrl::Data() {
	MetaTrader& mt = GetMetaTrader();
	System& sys = *trainer->sys;
	Iterator& iter = trainer->iters[thrd_id];
	SessionThread& thrd = *trainer->thrds[thrd_id];
	
	if (init) {
		init = false;
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







TrainerStatistics::TrainerStatistics(Trainer& trainer) :
	trainer(&trainer),
	profit(0, trainer, this),		sigprofit(1, trainer, this),		loss(2, trainer, this),
	reward(3, trainer, this),		thrdprio(4, trainer, this),			thrdperf(5, trainer, this)
{
	CtrlLayout(*this);
	
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << vsplit0 << vsplit1;
	
	vsplit0.Vert();
	vsplit0 << reward << sigprofit << thrdperf;
	vsplit1.Vert();
	vsplit1 << loss << profit << thrdprio;
	
	init = true;
}

void TrainerStatistics::Data()
{
	if (init) {
		init = false;
		/*for(int i = 0; i < trainer->thrds.GetCount(); i++) {
			const SessionThread& st = trainer->thrds[i];
			
		}*/
		
		
		
	}
	
	profit.Refresh();
	sigprofit.Refresh();
	loss.Refresh();
	reward.Refresh();
	thrdprio.Refresh();
	thrdperf.Refresh();
}










StatsGraph::StatsGraph(int mode, Trainer& t, TrainerStatistics* s) : trainer(&t), mode(mode), stats(s) {
	
	
	
}

const ConvNet::Window& StatsGraph::GetData(int thrd) {
	if (stats->mode == 0) {
		switch (mode) {
			// Profit
			case 0:		return trainer->thrds[thrd]->test_window1;
			// Signal Profit
			case 1:		return trainer->thrds[thrd]->test_window0;
			// Loss
			case 2:		return trainer->thrds[thrd]->loss_window;
			// Reward
			case 3:		return trainer->thrds[thrd]->reward_window;
			// Thread Priority
			case 4:		return trainer->thrd_priorities[thrd];
			// Thread Performance
			case 5:		return trainer->thrd_performances[thrd];
		}
	}
	Panic("Invalid mode");
}

void StatsGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	int thrd_count = trainer->thrds.GetCount();
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

}


