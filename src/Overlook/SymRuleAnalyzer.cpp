#include "Overlook.h"
#if 0
namespace Overlook {

SymRuleAnalyzer::SymRuleAnalyzer() {
	
}

SymRuleAnalyzer::~SymRuleAnalyzer() {
	
}

bool SymRuleAnalyzer::RefreshReal() {
	
}






String SymRuleAnalyzer::GetSignalString(int i) {
	switch (i) {
		case OPEN_LONG: return "Open long";
		case OPEN_SHORT: return "Open short";
		case CLOSE_LONG: return "Close long";
		case CLOSE_SHORT: return "Close short";
		case SUSTAIN: return "Sustain";
		case BLOCK: return "Block";
		case ATTENTION1: return "1 min attention";
		case ATTENTION5: return "5 mins attention";
		case ATTENTION15: return "15 mins attention";
		default: return "Invalid";
	}
}

void SymRuleAnalyzer::StartProcess() {
	StopProcess();
	
	analyze.SetLabel("Stop");
	analyze.WhenAction = THISBACK(PostStopProcess);
	
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Process));
}

void SymRuleAnalyzer::StopProcess() {
	analyze.SetLabel("Analyze");
	analyze.WhenAction = THISBACK(PostStartProcess);
	
	running = false;
	while (!stopped) Sleep(100);
}

void SymRuleAnalyzer::Prepare() {
	System& sys = GetSystem();
	for(int i = 0; i < sys.GetSymbolCount(); i++) sym_ids.Add(i);
	tf_ids.Add(0);
	
	FactoryDeclaration tmp;
	tmp.arg_count = 0;
	tmp.factory = System::Find<DataBridge>();							indi_ids.Add(tmp);
    /*tmp.factory = System::Find<MovingAverage>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<MovingAverageConvergenceDivergence>();	indi_ids.Add(tmp);
    tmp.factory = System::Find<BollingerBands>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<ParabolicSAR>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<StandardDeviation>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<AverageTrueRange>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<BearsPower>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<BullsPower>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<CommodityChannelIndex>();				indi_ids.Add(tmp);
    tmp.factory = System::Find<DeMarker>();								indi_ids.Add(tmp);
    tmp.factory = System::Find<Momentum>();								indi_ids.Add(tmp);
    tmp.factory = System::Find<RelativeStrengthIndex>();				indi_ids.Add(tmp);
    tmp.factory = System::Find<RelativeVigorIndex>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<StochasticOscillator>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<AcceleratorOscillator>();				indi_ids.Add(tmp);
    tmp.factory = System::Find<AwesomeOscillator>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<PeriodicalChange>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<VolatilityAverage>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<VolatilitySlots>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<VolumeSlots>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<ChannelOscillator>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<ScissorChannelOscillator>();				indi_ids.Add(tmp);*/
    
    sys.System::GetCoreQueue(ci_queue, sym_ids, tf_ids, indi_ids);
    
    this->data_in.SetCount(sys.GetSymbolCount());
    this->data_out.SetCount(sys.GetSymbolCount());
	this->av_wins.SetCount(sys.GetSymbolCount());
	this->median_maps.SetCount(sys.GetSymbolCount());
	this->volat_divs.SetCount(sys.GetSymbolCount());
    
	opt.Init(signal_count, 1, opt_row_size, StrategyBest1Exp);
}

void SymRuleAnalyzer::ProcessData() {
	
}

void SymRuleAnalyzer::ProcessSignalPrediction() {
	RunAgent(false);
}

void SymRuleAnalyzer::ProcessSignalInitial() {
	
	
}

void SymRuleAnalyzer::Process() {
	System& sys = GetSystem();
	
	while (running && !is_prepared) {
		Prepare();
		is_prepared = true;
	}
	
	while (running && (ci_queue.IsEmpty() || processed_cursor < ci_queue.GetCount())) {
		sys.System::Process(*ci_queue[processed_cursor++], true);
	}
	
	while (running && data_cursor < sys.GetSymbolCount()) {
		ProcessData();
		ProcessSignalInitial();
		data_cursor++;
	}
	while (running) {
		ProcessIteration();
	}
	
	stopped = true;
}

void SymRuleAnalyzer::ProcessIteration() {
	System& sys = GetSystem();
	TimeStop ts;
	int phase_total = opt.GetMaxRounds();
	
	int current_in_loop = 0;
	while (ts.Elapsed() < 100 && phase < FINISHED) {
		if (current < phase_total && phase >= 0 && phase < FINISHED) {
			switch (phase) {
				case TRAIN:				IterateTrain(); break;
				case TEST:				IterateTest(); break;
			}
		}
		
		current++;
		
		
		if (current >= phase_total) {
			current = 0;
			phase++;
			
			if (phase >= FINISHED) {
				phase = 0;
				PostCallback(THISBACK(StopProcess));
			}
		}
	}
	
	actual = opt.GetRound();
	total = phase_total;
	prev_speed = ((double)current_in_loop / (double)ts.Elapsed()) * 1000.0;
	if (prev_speed < 1.0) prev_speed = 1.0;
	perc = actual * 100 / total;
	
	PostData();
	
	bool ready = phase == FINISHED;
	
}

void SymRuleAnalyzer::IterateTrain() {
	if (opt.IsEnd()) return;
	
	System& sys = GetSystem();
	
	
	opt.Start();
	
	double result = RunAgent(true);
	
	//if (opt.GetRound() % 100 == 0)
	{
		training_pts.Add(result);
	}
	
	opt.Stop(result);
}

void SymRuleAnalyzer::IterateTest() {
	
}

double SymRuleAnalyzer::RunAgent(bool test) {
	ASSERT(row_size <= 128);
	
	const Vector<bool>& trial = opt.GetTrialSolution();
	
	
	// signals open, close, sustain, attention, block
	bool cur_signals[signal_count];
	int bit = 0;
	Snap sig_poles[signal_count*2];
	double sig_dist_mult[signal_count];
	for(int i = 0; i < signal_count; i++) {
		
		// two per signal... on and off poles
		for(int j = 0; j < 2; j++) {
			Snap& s = sig_poles[i * 2 + j];
			double& d = sig_dist_mult[i * 2 + j];
			for(int k = 0; k < row_size; k++) {
				s.Set(k, trial[bit++]);
			}
			d = 1.0;
			for(int k = 0; k < trsh_bits; k++)
				if (trial[bit++]) d += 1.0;
			d /= trsh_bits + 2.0;
		}
	}
	ASSERT(bit == trial.GetCount());
	
	
	int sym_count = data_in.GetCount();
	double av_accuracy = 0.0;
	
	for(int sym_id = 0; sym_id < sym_count; sym_id++) {
		const VectorSnap& data_in = this->data_in[sym_id];
		VectorSnap& data_out = this->data_out[sym_id];
		ASSERT(data_in.GetCount() == data_out.GetCount());
		int data_count = data_in.GetCount();
		
		int correct_testbits = 0;
		int total_signaltestbits = 0;
		double total_change = 1.0;
		bool is_enabled = false, prev_signal;
		int step = 1;
		for(int i = 0; i < data_count; i += step) {
			
			const Snap& sym_in = data_in[i];
			
			Snap* pole = sig_poles;
			double* mult = sig_dist_mult;
			bool signal[signal_count];
			for(int j = 0; j < signal_count; j++) {
				
				// two per signal... on and off poles
				double dist[2];
				for(int k = 0; k < 2; k++) {
					dist[k] = sym_in.GetDistance(*pole) * *mult;
					pole++;
					mult++;
				}
				
				signal[j] = dist[1] > dist[0];
			}
			
			bool& is_openl			= signal[OPEN_LONG];
			bool& is_opens			= signal[OPEN_SHORT];
			bool& is_closel			= signal[CLOSE_LONG];
			bool& is_closes			= signal[CLOSE_SHORT];
			bool& is_sustain		= signal[SUSTAIN];
			bool& is_block			= signal[BLOCK];
			bool& is_attention1		= signal[ATTENTION1];
			bool& is_attention5		= signal[ATTENTION5];
			bool& is_attention15	= signal[ATTENTION15];
			
			
			
			
			if (is_enabled) {
				if (is_block) {
					is_openl = false;
					is_opens = false;
					is_closel = true;
					is_closes = true;
				}
				else if (is_sustain) {
					is_openl = false;
					is_opens = false;
					is_closel = false;
					is_closes = false;
				}

				if ((is_closel && prev_signal == 0) || (is_closes && prev_signal == 1))
					is_enabled = false;
			}
			
			if (is_openl || is_opens) {
				bool was_enable = is_enabled;
				is_enabled = true;
				
				if (was_enable) {
					if (is_openl && prev_signal == 0)
						; // keep going
					else if (is_opens && prev_signal == 1)
						; // keep going
					else if (is_openl && is_opens)
						is_enabled = false; // conflict, don't open
					else if (is_openl)
						prev_signal = 0;
					else if (is_opens)
						prev_signal = 1;
				}
			}
			
			if (is_attention1)
				step = 1;
			else if (is_attention5)
				step = 5;
			else if (is_attention15)
				step = 15;
			
			
			
			
			if (test) {
				const auto& out = data_out[i];
				bool test_enabled = out.Get(RT_ENABLED);
				bool test_signal = out.Get(RT_SIGNAL);
				
				if (test_enabled == is_enabled) {
					correct_testbits++;
					if (test_enabled) {
						total_signaltestbits++;
						if (test_signal == prev_signal)
							correct_testbits++;
					}
				}
			} else {
				
				for(int j = i; j < i+step; j++) {
					auto& out = data_out[j];
					
					out.Set(RT_ENABLED, is_enabled);
					out.Set(RT_SIGNAL, prev_signal);
				}
			}
		}
		
		av_accuracy += (double)correct_testbits / (double)(data_count + total_signaltestbits);
	}
	
	av_accuracy /= sym_count;
	
	return av_accuracy;
}






















void SymRuleAnalyzer::RADataCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	int sym_count = ra->data_in.GetCount();
	
	for(int sym_id = 0; sym_id < sym_count; sym_id++) {
		const VectorSnap& data_in = ra->data_in[sym_id];
		int sym_pos = min(data_in.GetCount() - 1, cursor);
		double xstep = (double)sz.cx / (ra->period_count * ra->data_in.GetCount());
		double ystep = (double)sz.cy / ra->INPUT_COUNT;
		int w = xstep + 0.5;
		int h = ystep + 0.5;
		if (sym_pos < 0 || sym_pos + 1 > data_in.GetCount()) continue;
		int bit_pos = 0;
		for(int i = 0; i < ra->period_count; i++) {
			int x = xstep * (i + sym_id * ra->period_count);
			for(int j = 0; j < ra->INPUT_COUNT; j++) {
				int y = ystep * j;
				
				bool bit = data_in[sym_pos].Get(bit_pos++);
				if (bit) id.DrawRect(x, y, w, h, Black());
			}
		}
	}
	
	
	d.DrawImage(0,0,id);
}

void SymRuleAnalyzer::OrganizationAgentCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	int data_count = ra->training_pts.GetCount();
	if (data_count >= 2) {
		
		pts.SetCount(0);
		
		double minv = DBL_MAX, maxv = -DBL_MAX;
		for(int i = 0; i < data_count; i++) {
			double d = ra->training_pts[i];
			if (d < minv) minv = d;
			if (d > maxv) maxv = d;
		}
		double diff = maxv - minv;
		
		double xstep = (double)sz.cx / (data_count - 1);
		for(int i = 0; i < data_count; i++) {
			double d = ra->training_pts[i];
			Point& pt = pts.Add();
			pt.x = i * xstep;
			pt.y = sz.cy * (1.0 - ((d - minv) / diff));
		}
		id.DrawPolyline(pts, 1, Color(56, 85, 150));
	}
	
	d.DrawImage(0,0,id);
}

}
#endif
