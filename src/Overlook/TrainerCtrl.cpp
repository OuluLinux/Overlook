#include "Overlook.h"

namespace Overlook {
using namespace Upp;


TrainerConfiguration::TrainerConfiguration(Trainer& trainer) :
	trainer(&trainer)
{
	CtrlLayout(*this);
	thrdcount.SetData(1);
	
}

void TrainerConfiguration::Data()
{
	
	
	
	
	
}







TrainerStatistics::TrainerStatistics(Trainer& trainer) :
	trainer(&trainer)
{
	CtrlLayout(*this);
	
}

void TrainerStatistics::Data()
{
	
	
	
	
	
}








TrainerDraw::TrainerDraw(TrainerCtrl& ctrl) : ctrl(&ctrl) {
	
}

void TrainerDraw::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	
	TrainerCtrl& ctrl = *this->ctrl;
	Trainer& trainer = *ctrl.trainer;
	System& sys = *trainer.sys;
	
	
	const Iterator& iter = trainer.iter;
	
	int sym_count		= trainer.sym_ids.GetCount();
	int tf_count		= iter.pos.GetCount();
	int value_count		= iter.value_count;
	
	int rows = sym_count * tf_count;
	int cols = value_count;
	int grid_w = sz.cx - 300;
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





TrainerCtrl::TrainerCtrl(Trainer& trainer) :
	trainer(&trainer),
	draw(*this)
{
	Add(time_lbl.TopPos(3, 24).LeftPos(0, 46));
	Add(time_slider.TopPos(3, 24).LeftPos(50, 300));
	Add(step_bwd.TopPos(3, 24).LeftPos(600, 100));
	Add(step_fwd.TopPos(3, 24).LeftPos(700, 100));
	Add(draw.VSizePos(30).HSizePos());
	
	time_lbl.SetLabel("Time:");
	time_lbl.AlignRight();
	time_slider.MinMax(0, 100).SetData(50);
	step_bwd.SetLabel("<--");
	step_fwd.SetLabel("-->");
	
	step_bwd <<= THISBACK1(SeekCur, -1);
	step_fwd <<= THISBACK1(SeekCur, +1);
	time_slider <<= THISBACK(Data);
}

void TrainerCtrl::Data() {
	MetaTrader& mt = GetMetaTrader();
	System& sys = *trainer->sys;
	
	time_slider.MinMax(0, trainer->iter.bars-1);
	time_slider.SetData(trainer->iter.bars-1);
	
	Iterator& iter = trainer->iter;
	int pos = time_slider.GetData();
	if (iter.pos.Top() != pos) {
		trainer->Seek(pos);
	}
	
	draw.Refresh();
}

void TrainerCtrl::SeekCur(int step) {
	if (!trainer->SeekCur(step))
		return;
	time_slider.MinMax(0, trainer->iter.bars-1);
	time_slider.SetData(trainer->iter.pos.Top());
	draw.Refresh();
}

}
