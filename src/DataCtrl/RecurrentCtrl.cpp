#include "DataCtrl.h"

namespace DataCtrl {

RecurrentDraw::RecurrentDraw() {
	sym = 0;
	tf = 0;
	week = 0;
	
}

void RecurrentDraw::Paint(Draw& w) {
	TimeVector& tv = GetTimeVector();
	
	if (!src) {
		src = tv.FindLinkSlot("/open");
		rnn = tv.FindLinkSlot("/rnn");
		ASSERTEXC(src);
		ASSERTEXC(rnn);
		rec = dynamic_cast<DataCore::Recurrent*>(&*rnn);
		ASSERTEXC(rec);
	}
	
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	int fast_tf = tv.GetPeriod(0);
	int week_tf = tv.GetTfFromSeconds(7*24*60*60); // 1 week
	int begin_pos = tv.GetShift(week_tf, fast_tf, week);
	int end_pos   = tv.GetShift(week_tf, fast_tf, week+1);
	end_pos = min(tv.GetCount(1), end_pos);
	
	int count = end_pos - begin_pos;
	double xstep = (double)sz.cx / (count-1);
	
	
	
	// Reserve fast memory for visible values to avoid iterating slow memory twice
	int sym_count = 1;
	int tf_count = 1;
	int values = 1+4*4;
	int total = sym_count * tf_count * values * count;
	tmp.SetCount(total);
	
	// Lock currently cached pages for this usage (don't release these or other's pages)
	tv.EnterCache();
	
	double maxv = -DBL_MAX;
	double minv = +DBL_MAX;
	int t = 0;
	for(int i = 0; i < count; i++) {
		int pos = begin_pos + i;
		
		for(int j = 0; j < sym_count; j++) {
			for(int k = 0; k < tf_count; k++) {
				double src_value = *tv.GetSlotValue<double>(j, k, pos, *src, 0, true);
				tmp[t++] = src_value;
				if (src_value > maxv) maxv = src_value;
				if (src_value < minv) minv = src_value;
				
				for (int v = 0; v < 4*4; v++) {
					double rnn_value = *tv.GetSlotValue<double>(j, k, pos, *rnn, v, true);
					tmp[t++] = rnn_value;
					if (rnn_value > maxv) maxv = rnn_value;
					if (rnn_value < minv) minv = rnn_value;
				}
			}
		}
	}
	
	tv.LeaveCache();
	
	double diff = maxv - minv;
	
	// Re-use temp point memory for efficiency
	pts.SetCount(17);
	for(int i = 0; i < pts.GetCount(); i++)
		pts[i].SetCount(0);
	
	t = 0;
	for(int i = 0; i < count; i++) {
		int x = xstep * i;
		
		for(int j = 0; j < sym_count; j++) {
			for(int k = 0; k < tf_count; k++) {
				double val = tmp[t++];
				double y = (val - minv) / diff * sz.cy;
				pts[0] << Point(x, y);
				
				for (int v = 0; v < 4*4; v++) {
					double val = tmp[t++];
					double y = (val - minv) / diff * sz.cy;
					pts[1+v] << Point(x, y);
				}
			}
		}
	}
	
	for(int i = 0; i < pts.GetCount(); i++) {
		id.DrawPolyline(pts[i], 1, GrayColor(128*i/pts.GetCount()));
	}
	
	w.DrawImage(0, 0, id);
}
















RecurrentCtrl::RecurrentCtrl() {
	CtrlLayout(*this);
	
	//refresh.Set(true);
	
	learning_rate_slider.MinMax(1,1000);
	learning_rate_slider.SetData(1000);
	learning_rate_slider <<= THISBACK(SetLearningRate);
	
	lowtemp		.MinMax(10, 900);
	medtemp		.MinMax(10, 900);
	hightemp	.MinMax(10, 900);
	lowtemp		.SetData(100);
	medtemp		.SetData(100);
	hightemp	.SetData(100);
	lowtemp		<<= THISBACK1(SetSampleTemperature, 0);
	medtemp		<<= THISBACK1(SetSampleTemperature, 1);
	hightemp	<<= THISBACK1(SetSampleTemperature, 2);
	restart		<<= THISBACK(Reset);
	//save <<= THISBACK(Save);
	//load <<= THISBACK(Load);
	//load_pretrained <<= THISBACK(LoadPretrained);
	//pause <<= THISBACK(Pause);
	//resume <<= THISBACK(Resume);
	set_rnn		<<= THISBACK1(SetPreset, 0);
	set_lstm	<<= THISBACK1(SetPreset, 1);
	set_rhn		<<= THISBACK1(SetPreset, 2);
	
	PostCallback(THISBACK(Refresher));
}

void RecurrentCtrl::SetArguments(const VectorMap<String, Value>& args) {
	
}

void RecurrentCtrl::Init() {
	TimeVector& tv = GetTimeVector();
	
	int week_tf = tv.GetTfFromSeconds(7*24*60*60); // 1 week
	int week_count = tv.GetCount(week_tf);
	week_slider.MinMax(0, week_count-1);
	week_slider.SetData(0);
	week_slider <<= THISBACK(SetWeekFromSlider);
	
	SlotPtr rnn = tv.FindLinkSlot("/rnn");
	ASSERTEXC(rnn);
	rec = dynamic_cast<DataCore::Recurrent*>(&*rnn);
	ASSERTEXC(rec);
	network_view.SetRecurrentSession(rec->GetSession(0, 0));
	
	lowtemp.SetData(rec->GetSampleTemperature(0) / 0.01);
	medtemp.SetData(rec->GetSampleTemperature(1) / 0.01);
	hightemp.SetData(rec->GetSampleTemperature(2) / 0.01);
	SetSampleTemperature(0);
	SetSampleTemperature(1);
	SetSampleTemperature(2);
	
	model_edit.SetData(rec->GetModel());
	
}

void RecurrentCtrl::Refresher() {
	TimeVector& tv = GetTimeVector();
	
	draw.Refresh();
	network_view.Refresh();
	
	int tick_iter	= rec->GetIter();
	if (tick_iter > 0) {
		int epoch_size	= tv.GetCount(1);
		double ppl		= rec->GetPerplexity();
		int tick_time	= rec->GetTickTime();
		
		ppl_list.Add(ppl); // keep track of perplexity
		if (ppl_list.GetCount() >= 100) {
			double median_ppl = Median(ppl_list);
			ppl_list.SetCount(0);
			perp.AddValue(median_ppl);
		}
		
		SetStats((double)tick_iter/epoch_size, ppl, tick_time);
	}
	
	//if (refresh.Get())
		PostCallback(THISBACK(Refresher));
}

void RecurrentCtrl::Reset() {
	perp.Clear();
	String model_str = model_edit.GetData();
	rec->SetModel(model_str);
	rec->Reload();
}

void RecurrentCtrl::SetWeekFromSlider() {
	draw.week = week_slider.GetData();
}

void RecurrentCtrl::SetLearningRate() {
	double value = learning_rate_slider.GetData();
	value *= 0.00001;
	lbl_learning_rate.SetLabel(FormatDoubleFix(value, 5, FD_ZEROS));
	rec->SetLearningRate(value);
}

void RecurrentCtrl::SetPreset(int i) {
	String model_str;
	if (i == 0) {
		model_str = "{\n"
	
			// model parameters
			"\t\"generator\":\"rnn\",\n" // can be 'rnn' or 'lstm' or 'highway'
			"\t\"hidden_sizes\":[20,20],\n" // list of sizes of hidden layers
			"\t\"letter_size\":5,\n" // size of letter embeddings
			
			// optimization
			"\t\"regc\":0.000001,\n" // L2 regularization strength
			"\t\"learning_rate\":0.01,\n" // learning rate
			"\t\"clipval\":5.0\n" // clip gradients at this value
			"}";
	}
	else if (i == 1) {
		model_str = "{\n"
	
			// model parameters
			"\t\"generator\":\"lstm\",\n" // can be 'rnn' or 'lstm' or 'highway'
			"\t\"hidden_sizes\":[20,20],\n" // list of sizes of hidden layers
			"\t\"letter_size\":5,\n" // size of letter embeddings
			
			// optimization
			"\t\"regc\":0.000001,\n" // L2 regularization strength
			"\t\"learning_rate\":0.01,\n" // learning rate
			"\t\"clipval\":5.0\n" // clip gradients at this value
			"}";
	}
	else if (i == 2) {
		model_str = "{\n"
	
			// model parameters
			"\t\"generator\":\"highway\",\n" // can be 'rnn' or 'lstm' or 'highway'
			"\t\"hidden_sizes\":[20,20],\n" // list of sizes of hidden layers
			
			// optimization
			"\t\"regc\":0.000001,\n" // L2 regularization strength
			"\t\"learning_rate\":0.01,\n" // learning rate
			"\t\"clipval\":5.0\n" // clip gradients at this value
			"}";
	}
	model_edit.SetData(model_str);
}

void RecurrentCtrl::SetSampleTemperature(int i) {
	SliderCtrl& temp = i == 0 ? lowtemp : i == 1 ? medtemp : hightemp;
	Label& lbl_temp = i == 0 ? lbl_lowtemp : i == 1 ? lbl_medtemp : lbl_hightemp;
	double value = temp.GetData();
	value *= 0.01;
	lbl_temp.SetLabel(FormatDoubleFix(value, 2, FD_ZEROS));
	rec->SetSampleTemperature(i, value);
}

double RecurrentCtrl::Median(Vector<double>& values) {
	Sort(values, StdGreater<double>());
	int half = values.GetCount() / 2;
	if (values.GetCount() % 2)
		return values[half];
	else
		return (values[half-1] + values[half]) / 2.0;
}

void RecurrentCtrl::SetStats(double epoch, double ppl, int time) {
	lbl_epoch.SetLabel("epoch: " + FormatDoubleFix(epoch, 2, FD_ZEROS));
	lbl_perp.SetLabel("perplexity: " + FormatDoubleFix(ppl, 2, FD_ZEROS));
	lbl_time.SetLabel("forw/bwd time per example: " + FormatDoubleFix(time, 1) + "ms");
}


}
