#include "Overlook.h"

namespace Overlook {
using namespace Upp;


TrainerConfiguration::TrainerConfiguration(Trainer& trainer) :
	trainer(&trainer)
{
	CtrlLayout(*this);
	thrdcount.SetData(1);
	thrdlist.AddColumn("Name");
	thrdlist.NoHeader();
	thrdlist <<= THISBACK(Data);
	apply <<= THISBACK(Apply);
	reset <<= THISBACK(Reset);
}

void TrainerConfiguration::Data()
{
	for(int i = 0; i < trainer->thrds.GetCount(); i++)
		thrdlist.Set(i, 0, trainer->thrds[i].name);
	thrdlist.SetCount(trainer->thrds.GetCount());
	
	int thrd = thrdlist.GetCursor();
	if (thrd == -1) return;
	const SessionThread& st = trainer->thrds[thrd];
	
	filename.SetData(st.name);
	settings.SetData(st.params);
}

void TrainerConfiguration::Apply() {
	int thrd = thrdlist.GetCursor();
	if (thrd == -1) return;
	SessionThread& st = trainer->thrds[thrd];
	
	st.name = filename.GetData();
	st.params = settings.GetData();
	
	trainer->StoreThis();
	Data();
}

void TrainerConfiguration::Reset() {
	int thrd = thrdlist.GetCursor();
	if (thrd == -1) return;
	SessionThread& st = trainer->thrds[thrd];
	
	st.name = IntStr(thrd+1) + ". thread";
	st.params =
		"[\n"
		"\t{\"type\":\"input\""
			", \"input_width\":"  + IntStr(trainer->input_width)  +
			", \"input_height\":" + IntStr(trainer->input_height) +
			", \"input_depth\":"  + IntStr(trainer->input_depth)  +
			"},\n" // 2 inputs: x, y
		"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
		"\t{\"type\":\"regression\", \"neuron_count\":" + IntStr(trainer->output_width) + "},\n"
		"\t{\"type\":\"sgd\", \"learning_rate\":0.01, \"momentum\":0.9, \"batch_size\":5, \"l2_decay\":0.0}\n"
		"]\n";
	
	Data();
}







TrainerStatistics::TrainerStatistics(Trainer& trainer) :
	trainer(&trainer)
{
	CtrlLayout(*this);
	
}

void TrainerStatistics::Data()
{
	
	
	
	
	
}








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
	SessionThread& thrd = trainer->thrds[thrd_id];
	
	if (init) {
		init = false;
		conv.SetSession(thrd.ses);
		timescroll.SetSession(thrd.ses);
		conv.RefreshLayers();
	}
	
	time_slider.MinMax(0, iter.bars-1);
	time_slider.SetData(iter.pos.Top());
	
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
			thrdlist.Add(trainer->thrds[i].name);
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

}


