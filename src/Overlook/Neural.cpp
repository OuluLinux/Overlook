#include "Overlook.h"


namespace Overlook {

AutoEncoder::AutoEncoder() {
	
}

String AutoEncoder::GetStyle() const {
	
}

void AutoEncoder::SetArguments(const VectorMap<String, Value>& args) {
	
}

void AutoEncoder::Init() {
	//AddDependency("/open", 0, 0);
	
}

bool AutoEncoder::Process(const CoreProcessAttributes& attr) {
	
	// Add autoencoder by looking MNIST digits autoencoder as example
	//  - but instead of backpropagating with input, use future values
	Panic("TODO");
	
	return false;
}









Recurrent::Recurrent() {
	shifts = 4;
	for(int i = 0; i < shifts; i++) {
		
		// Different softmax sample temperatures:
		//   lower setting will generate more likely predictions,
		//   but you'll see more of the same common values again and again. Higher
		//   setting will generate less frequent values but you might see more unrealistic errors.
		//AddValue<double>();  // "coldest"	greedy argmax prediction
		//AddValue<double>();  // "cold"		low  temperate prediction
		//AddValue<double>();  // "warm"		med  temperate prediction
		//AddValue<double>();  // "hot"		high temperate prediction
	}
	
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
	
	is_training_loop = false;
	
}


String Recurrent::GetStyle() const {
	return
		"{"
			"\"window_type\":\"SEPARATE\","
			"\"value0\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":4,"
			"},"
			"\"value1\":{"
				"\"color\":\"0,0,255\","
				"\"line_width\":4,"
			"},"
			"\"value2\":{"
				"\"color\":\"255,0,0\","
				"\"line_width\":4,"
			"},"
			"\"value3\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":4,"
			"},"
			"\"value4\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":3,"
			"},"
			"\"value5\":{"
				"\"color\":\"0,0,255\","
				"\"line_width\":3,"
			"},"
			"\"value6\":{"
				"\"color\":\"255,0,0\","
				"\"line_width\":3,"
			"},"
			"\"value7\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":3,"
			"},"
			"\"value8\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":2,"
			"},"
			"\"value9\":{"
				"\"color\":\"0,0,255\","
				"\"line_width\":2,"
			"},"
			"\"value10\":{"
				"\"color\":\"255,0,0\","
				"\"line_width\":2,"
			"},"
			"\"value11\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":2,"
			"},"
			"\"value12\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
			"\"value13\":{"
				"\"color\":\"0,0,255\","
				"\"line_width\":1,"
			"},"
			"\"value14\":{"
				"\"color\":\"255,0,0\","
				"\"line_width\":1,"
			"},"
			"\"value15\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":1,"
			"}"
		"}";
}

void Recurrent::StoreThis() {
	String dir = ConfigFile("TimeVector.bin.d");
	RealizeDirectory(dir);
	StoreToFile(*this, AppendFileName(dir, "Recurrent.bin"));
}

void Recurrent::LoadThis() {
	String dir = ConfigFile("TimeVector.bin.d");
	RealizeDirectory(dir);
	LoadFromFile(*this, AppendFileName(dir, "Recurrent.bin"));
}

void Recurrent::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("type");
	if (i == -1) throw DataExc("Recurrent type must be set");
	String type = args[i];
	ASSERTEXC(type == "lstm"); // only lstm is supported currently
}

void Recurrent::Init() {
	//TODO: anchor this to timevector id in case of multiple instances
	
	//AddDependency("/open", 0, 0);
	//AddDependency("/change", 0, 0);
	//AddDependency("/whstat_slow", 0, 0);
	
	/*
	SetProcessedOnce(false);
	
	temperatures.SetCount(3);
	temperatures[0] = 0.1;
	temperatures[1] = 1.0;
	temperatures[2] = 5.0;
	
	tick_time = 0;
	iter = 0;
	value_count = 20;
	batch = 30;
	is_training_loop = true;
	
	LoadThis();
	
	BaseSystem& ol = Get<BaseSystem>();
	sym_count = ol.GetSymbolCount();
	tf_count = ol.GetPeriodCount();
	data.SetCount(sym_count * tf_count);
	
	Reload();
	*/
}

double Recurrent::GetPerplexity() const {
	/*double av = 0.0;
	int count = 0;
	for(int i = 0; i < data.GetCount(); i++) {
		av += data[i].ses.GetPerplexity();
		count++;
	}
	return av / count;*/
}

void Recurrent::SetLearningRate(double rate) {
	/*for(int i = 0; i < data.GetCount(); i++) {
		data[i].ses.SetLearningRate(rate);
	}*/
}

bool Recurrent::Process(const CoreProcessAttributes& attr) {
	/*
	const Core& src = GetDependency(0);
	const Core& change = GetDependency(1);
	const Core& whstat = GetDependency(2);
	SymTf& s = GetData(attr);
	
	if (attr.tf_id == 0 && attr.sym_id == 0)
		iter++;
	
	// Write current values to the sentence
	int count = Upp::min(batch, attr.GetCounted()-1);
	if (count > 10) {
		// Run single training
		// Run only unique batches
		if (attr.GetCounted() % batch == 0) {
			s.sequence.SetCount(count);
			for(int i = 0; i < count; i++) {
				double* value = change.GetValue<double>(0, count-1-i, attr);
				double* min = whstat.GetValue<double>(10, count-1-i, attr);
				double* max = whstat.GetValue<double>(11, count-1-i, attr);
				s.sequence[i] = ToChar(*value, *min, *max);
			}
			Tick(attr);
		}
	}
	
	// Train this again until training loop is finished, after this, only new values are
	// trained.
	return !is_training_loop;
	*/
}

bool Recurrent::ProcessRelease(const CoreProcessAttributes& attr) {
	/*
	const Core& src = GetDependency(0);
	const Core& change = GetDependency(1);
	const Core& whstat = GetDependency(2);
	SymTf& s = GetData(attr);
	
	int count = Upp::min(batch, attr.GetCounted()-1);
	s.sequence.SetCount(count);
	for(int i = 0; i < count; i++) {
		double* value = change.GetValue<double>(0, count-1-i, attr);
		double* min = whstat.GetValue<double>(10, count-1-i, attr);
		double* max = whstat.GetValue<double>(11, count-1-i, attr);
		s.sequence[i] = ToChar(*value, *min, *max);
	}
	
	// Write predictions
	// For greedy and 3 temperatures
	for(int i = 0; i < 4; i++) {
		
		// Erase the previous prediction (when i > 0)
		s.sequence.SetCount(count);
		
		// First is greedy argmax and after that are temperatures
		PredictSentence(attr, i > 0, i == 0 ? 0 : temperatures[i-1]);
		int new_count = s.sequence.GetCount();
		ASSERT(new_count >= count);
		
		// For last 4 values and shifts of current temperature
		// Hints:
		//  - from current time-position (+0) to next time-position (+1)
		//		- sequence[count] is change
		//		- whstat ... (.., -1 ...  is min/max change
		//		- this->GetValue<double>(0+0*4, ... is greedy predicted change
		//		- this->GetValue<double>(1+0*4, ... is low temperature predicted change
		//  - from +1 to +2
		//		- sequence[count+1] is change
		//		- whstat ... (.., -2 ...  is min/max change
		//		- this->GetValue<double>(0+1*4, ... is greedy predicted change
		//		- this->GetValue<double>(1+1*4, ... is low temperature predicted change
		for(int j = count, k=0, shift=-1; j < count+4 && j < new_count; j++, k+=4, shift--) {
			double* val = GetValue<double>(i+k, attr);
			double* min = whstat.GetValue<double>(10, shift, attr);
			double* max = whstat.GetValue<double>(11, shift, attr);
			if (j < new_count)
				*val = FromChar(s.sequence[j], *min, *max);
			else
				*val = 0;
		}
	}
	
	return true;
	*/
}

void Recurrent::Reload() {
	// note: reinit writes global vars
	iter = 0;
	tick_time = 0;
	
	InitSessions();
}

void Recurrent::InitSessions() {
	// eval options to set some globals
	/*ValueMap js = ParseJSON(model_str);
	for(int i = 0; i < data.GetCount(); i++) {
		ConvNet::RecurrentSession& ses = data[i].ses;
		ses.Load(js);
		ses.SetInputSize(value_count+1); // possible values + START token
		ses.SetOutputSize(value_count+1);
		ses.Init();
	}*/
}

int Recurrent::ToChar(double diff, double min, double max) {
	double range = max - min;
	double step = range / (value_count - 1);
	int d = (diff - min) / step;
	if (d < 0) d = 0;
	else if (d >= value_count) d = value_count-1;
	return d + 1; // first is START token
}

double Recurrent::FromChar(int i, double min, double max) {
	double range = max - min;
	double step = range / (value_count - 1);
	int d = i - 1; // first is START token
	ASSERT(d >= 0 && d < value_count);
	return d * step + min;
}

void Recurrent::PredictSentence(const CoreProcessAttributes& attr, bool samplei, double temperature) {
	/*
	SymTf& s = GetData(attr);
	s.ses.Predict(s.sequence, samplei, temperature, true, 4);
	*/
}

void Recurrent::Tick(const CoreProcessAttributes& attr) {
	/*
	TimeStop ts;  // log start timestamp
	
	SymTf& s = GetData(attr);
	s.ses.Learn(s.sequence);
	
	tick_time = ts.Elapsed();
	*/
	
	//double ppl = ses.GetPerplexity();
	//ppl_list.Add(ppl); // keep track of perplexity
	
	// evaluate now and then
	/*tick_iter += 1;
	
	if (tick_iter % 50 == 0) {
		// draw samples
		WString samples;
		for (int q = 0; q < 5; q++) {
			if (q) samples += "\n\n";
			samples += PredictSentence(true, sample_softmax_temperature);
		}
		
		PostCallback(THISBACK1(SetSamples, samples));
	}
	if (tick_iter % 10 == 0) {
		// draw argmax prediction
		WString pred = PredictSentence(false);
		
		PostCallback(THISBACK1(SetArgMaxSample, pred));
		
		// keep track of perplexity
		PostCallback(THISBACK3(SetStats, (double)tick_iter/epoch_size, ppl, tick_time));
		
		if (tick_iter % 100 == 0) {
			double median_ppl = Median(ppl_list);
			ppl_list.SetCount(0);
			perp.AddValue(median_ppl);
		}
	}*/
}

















NARX::NARX() {
	H = 8;				// hidden unit count
	hact = 0;			// sigmoid function
	a = 10;				// like max_shift for input values
	b = 10;				// like max_shift for exogenous vlaues
	targets = 1;
	feedback = 1;
	a1 = a + 1;
	
	//AddValue<double>();
	
}

String NARX::GetStyle() const {
	
}

void NARX::SetArguments(const VectorMap<String, Value>& args) {
	
}

void NARX::Init() {
	/*
	BaseSystem& ol = Get<BaseSystem>();
	
	ASSERTEXC_(hact >= 0 && hact < 3, "Hidden unit activation can be in range [0,2]");
	
	//AddDependency("/open", 1, 0);
	//AddDependency("/change", 1, 1);
	SetProcessing(1, 0);
	SetProcessedOnce(false);
	
	sym_count = ol.GetSymbolCount();
	tf_count = ol.GetPeriodCount();
	
	epoch = 0;
	
	data.SetCount(tf_count);
	for(int i = 0; i < tf_count; i++) {
		Tf& s = data[i];
		
		int slower_tfs = tf_count - 1 - i;
		s.value_count = slower_tfs * sym_count;
		s.input_count = s.value_count + sym_count + 1;	// total exogenous values
		
		s.ee.SetCount(sym_count);
		s.rw.SetCount(sym_count);
		
		for (int i = 0; i < sym_count; i++) {
			s.ee[i].Init(1);
			s.rw[i].Init(1);
		}
	
		s.hunits.SetCount(H);
	
		for (int i = 0; i < H; i++) {
			
			if (hact == 2) {
				s.hunits[i].set_activation_func(ActivationFunctions::AsLog);
				s.hunits[i].set_activation_func_derv(ActivationFunctions::AsLogDerv);
			}
			else if (hact == 1) {
				s.hunits[i].set_activation_func(ActivationFunctions::BSigmoid);
				s.hunits[i].set_activation_func_derv(ActivationFunctions::BSigmoidDerv);
			}
			else if (hact == 0) {
				s.hunits[i].set_activation_func(ActivationFunctions::Sigmoid);
				s.hunits[i].set_activation_func_derv(ActivationFunctions::SigmoidDerv);
			}
		}
	
		if (targets) {
			s.inputs.SetCount(sym_count);
	
			for (int i = 0; i < sym_count; i++) {
				s.inputs[i].SetCount(b);
				for(int j = 0; j < b; j++) {
					s.inputs[i][j].SetInput(0);
				}
			}
	
			for (int i = 0; i < H; i++) {
				for (int j = 0; j < sym_count; j++) {
					for(int k = 0; k < b; k++) {
						s.hunits[i].AddInputUnit(s.inputs[j][k]);
					}
				}
			}
		}
		
	
		if (feedback) {
			s.feedbacks.SetCount(sym_count);
	
			for (int i = 0; i < sym_count; i++) {
				s.feedbacks[i].SetCount(b);
				for(int j = 0; j < b; j++) {
					s.feedbacks[i][j].SetInput(0);
				}
			}
	
			for (int i = 0; i < H; i++) {
				for (int j = 0; j < sym_count; j++) {
					for(int k = 0; k < b; k++) {
						s.hunits[i].AddInputUnit(s.feedbacks[j][k]);
					}
				}
			}
		}
	
		s.exogenous.SetCount(s.input_count);
		for (int i = 0; i < s.input_count; i++) {
			s.exogenous[i].SetCount(a1);
			for(int j = 0; j < a1; j++) {
				s.exogenous[i][j].SetInput(0);
			}
		}
		
		for (int i = 0; i < H; i++) {
			for (int j = 0; j < s.input_count; j++) {
				for(int k = 0; k < a1; k++) {
					s.hunits[i].AddInputUnit(s.exogenous[j][k]);
				}
			}
		}
	
		//printf("%d\n", index);
		s.output_units.SetCount(sym_count);
	
		for (int i = 0; i < sym_count; i++) {
			s.output_units[i].set_activation_func(ActivationFunctions::Identity);
			s.output_units[i].set_activation_func_derv(ActivationFunctions::IdentityDerv);
		}
	
		for (int i = 0; i < H; i++)
			for (int j = 0; j < sym_count; j++)
				s.output_units[j].AddInputUnit(s.hunits[i]);
	
		//WhenLog(Format("NARX: arch=%d, H=%d, hact =%d, a=%d, b=%d, M=%d, N=%d, feedback=%d, targets=%d",
		//	arch, H, hact, a, b, input_count, sym_count, feedback, targets));
		
	}*/
}

void NARX::FillExogenous(const Core& src, const Core& change, const CoreProcessAttributes& attr) {
	/*BaseSystem& ol = Get<BaseSystem>();
	
	for (int i = 0; i < s.input_count; i++) {
		Vector<InputUnit>& exo = s.exogenous[i];
		if (i < s.value_count) {
			int sym = i % sym_count;
			int tf = attr.tf_id + 1 + i / sym_count;
			for (int j = 0; j < a1; j++) {
				double* d = change.GetValue<double>(0, sym, tf, j, attr);
				exo[j].SetInput(d ? *d : 0);
			}
		}
		else {
			int sym = i - s.value_count;
			if (sym < sym_count) {
				for (int j = 0; j < a1; j++) {
					double* volume = src.GetValue<double>(3, sym, attr.tf_id, j, attr);
					exo[j].SetInput(volume ? *volume : 0);
				}
			} else {
				for (int j = 0; j < a1; j++) {
					Time t = ol.GetTime(attr.GetPeriod(), attr.GetCounted() - j);
					double h = t.hour;
					double t1 = ((double)t.minute + h * 60.0 ) / (24.0 * 60.0);
					double wday = (DayOfWeek(t) + t1) / 7.0;
					exo[j].SetInput(wday);
				}
			}
		}
	}*/
}

bool NARX::Process(const CoreProcessAttributes& attr) {
	/*const Core& src = GetDependency(0);
	const Core& change = GetDependency(1);
	
	Tf& s = GetData(attr);
	
	
	//if (attr.sym_id != 0) return true;
	ASSERT(attr.sym_id == 0);
	
	
	LOG(Format("narx sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	
	// Clear feedback loop in the beginning
	if (attr.GetCounted() == 0) {
		s.Y.Clear();
		s.Y.SetCount(sym_count, 0);
		s.pY.Clear();
		s.pY.SetCount(sym_count, 0);
	}
	
	int series_index = attr.GetCounted();
	
	for (int i = 0; i < sym_count; i++) {
		s.ee[i].Clear();
		s.rw[i].Clear();
	}

	if (!feedback) {
		int feedback_index = 0;
		
		if (targets) {
			for (int j = 0; j < sym_count; j++) {
				Vector<InputUnit>& in = s.inputs[j];
				for (int i = 0; i < b; i++) {
					double* d = change.GetValue<double>(0, j, attr.tf_id, i, attr);
					in[i].SetInput(d ? *d : 0);
				}
			}
		}
		
		FillExogenous(src, change, s, attr);
		
		for (int i = 0; i < sym_count; i++) {
			double* d = change.GetValue<double>(0, i, attr.tf_id, -1, attr);
			double* p = change.GetValue<double>(0, i, attr.tf_id, 0, attr);
			ASSERT(p);
			s.output_units[i].SetTarget(*d);
			s.ee[i].Insert(*d, s.output_units[i].GetOutput());
			s.rw[i].Insert(*d, p ? *p : 0);
		}

		//FWhenLog(String("input target:index %1 : %2, output narx: %3\n").arg(series_index).arg(series[series_index]).arg(output_unit->GetOutput()).toStdString().c_str());
		for (int i = 0; i < sym_count; i++) {
			s.output_units[i].FixWeights();
			s.output_units[i].ComputeDelta();
			s.output_units[i].AdjustWeights();
		}

		//FWhenLog(String("ok delta=%1\n").arg(delta).toStdString().c_str());
		for (int i = 0; i < H; i++) {
			double delta = 0;
			
			for (int j = 0; j < sym_count; j++)
				delta += s.output_units[j].GetDelta(s.hunits[i]);
			
			s.hunits[i].ComputeDelta(delta);
			s.hunits[i].AdjustWeights();
		}
	}
	else {
		s.Y.SetCount(sym_count, 0);
		
		FeedbackInfo fi;
		fi.Init(s.input_count, a1, sym_count, b, sym_count, b);
		
		FillExogenous(src, change, s, attr);
		
		for (int i = 0; i < s.input_count; i++) {
			Vector<InputUnit>& exo = s.exogenous[i];
			for (int j = a1-1; j >= 0 ; j--) {
				fi.X[i][j] = exo[j].GetInput();
			}
		}

		if (targets) {
			for (int i = 0; i < sym_count; i++) {
				Vector<InputUnit>& in = s.inputs[i];
				for (int j = b-1; j >= 0; j--) {
					double* d = change.GetValue<double>(0, i, attr.tf_id, j, attr);
					in[j].SetInput(d ? *d : 0);
					fi.D[i][j] = in[j].GetInput();
				}
			}
		}
		
		for (int i = 0; i < sym_count; i++) {
			Vector<InputUnit>& f = s.feedbacks[i];
			for (int j = b-1; j >= 0; j--) {
				//FWhenLog(String("testing%1\n").arg(fi.Y[i*N+j]).toStdString().c_str());
				f[j].SetInput(s.Y[i]); // Value should have been set in previous iteration or 0 at first

				//FWhenLog(String("testing%1\n").arg(fi.Y[i*N+j]).toStdString().c_str());
				fi.Y[i][j] = f[j].GetInput();
			}
		}

		for (int i = 0; i < sym_count; i++) {
			double* d = change.GetValue<double>(0, i, attr.tf_id, -1, attr);
			double* p = change.GetValue<double>(0, i, attr.tf_id, 0, attr);
			ASSERT(p);
			s.output_units[i].SetTarget(*d);
			s.Y[i] = s.output_units[i].GetOutput();
			//FWhenLog(String("testing%1\n").arg(Y[i][t]).toStdString().c_str());
			s.ee[i].Insert(*d, s.Y[i]);
			s.rw[i].Insert(*d, p ? *p : 0);
		}
		
		for (int i = 0; i < s.input_count; i++) {
			Vector<InputUnit>& exo = s.exogenous[i];
			for (int j = a; j >= 0 ; j--)
				exo[j].SetInput(fi.X[i][j]);
		}

		if (targets) {
			for (int i = 0; i < sym_count; i++) {
				Vector<InputUnit>& in = s.inputs[i];
				for (int j = b-1; j >= 0; j--)
					in[j].SetInput(fi.D[i][j]);
			}
		}

		
		for (int i = 0; i < sym_count; i++) {
			for (int j = b-1; j >= 0; j--)
				s.feedbacks[i][j].SetInput(fi.Y[i][j]);
		}
		
		for (int i = 0; i < sym_count; i++) {
			s.output_units[i].FixWeights();
			
			double* d = change.GetValue<double>(0, i, attr.tf_id, -1, attr);
			ASSERT(d);
			s.output_units[i].SetTarget(*d);

			s.output_units[i].ComputeDelta();
			s.output_units[i].AdjustWeights();
		}

		for (int i = 0; i < H; i++) {
			double delta = 0;

			for (int j = 0; j < sym_count; j++)
				delta += s.output_units[j].GetDelta(s.hunits[i]);

			s.hunits[i].ComputeDelta(delta);
			s.hunits[i].FixWeights();
			s.hunits[i].AdjustWeights();
		}
	}*/
	
	/*
	if (logging) WhenLog("Epoch finished :)");

	for (int i = 0; i < sym_count; i++) {
		WhenLog(Format("target %d:epoch %d: F1 = %n; F2 = %n; F3 = %n; F4 = %n; KS1= %n; KS2=%n; KS12=%n; DA = %n"
					 "; F1RW=%n; F2RW=%n; F3RW=%n; F4RW=%n; KS1=%n; KS2=%n; KS12=%n; DA=%n",
			  i,
			  epo,
			  ee[i].F1(), ee[i].F2(), ee[i].F3(), ee[i].F4(),
			  ee[i].KS1(),
			  ee[i].KS2(), ee[i].KS12(), ee[i].DA(),
			  rw[i].F1(), rw[i].F2(), rw[i].F3(), rw[i].F4(),
			  rw[i].KS1(), rw[i].KS2(), rw[i].KS12(), rw[i].DA()));
	}

	Test(epo);*/
	
	return true;
}

bool NARX::ProcessRelease(const CoreProcessAttributes& attr) {
	/*const Core& src = GetDependency(0);
	const Core& change = GetDependency(1);
	
	Tf& s = GetData(attr);
	
	// Predict
	{
		out.SetCount(sym_count);
		
		if (targets) {
			for (int j = 0; j < sym_count; j++) {
				Vector<InputUnit>& in = s.inputs[j];
				for (int i = 0; i < b; i++) {
					double* d = change.GetValue<double>(0, i, attr.tf_id, i, attr);
					in[i].SetInput(d ? *d : 0);
				}
			}
		}
		
		if (feedback) {
			for (int j = 0; j < sym_count; j++) {
				Vector<InputUnit>& f = s.feedbacks[j];
				for (int i = 0; i < b; i++) {
					f[i].SetInput(s.pY[j]);
				}
			}
		}
		
		FillExogenous(src, change, s, attr);
		
		for (int i = 0; i < sym_count; i++) {
			double out = s.output_units[i].GetOutput();
			//ASSERT(out < 0.1 && out > -0.1); // sane limits for current usage, not generic
			if      (out < -0.1) out = -0.1;
			else if (out > +0.1) out = +0.1;
			s.pY[i] = out;
			
			// Set value
			double* prv = src.GetValue<double>(0, i, attr.tf_id, 0, attr);
			double* dst = GetValue<double>(0, i, attr.tf_id, 0, attr);
			*dst = *prv * (1.0 + out);
			
			// Mark the slot of the symbol as processed
			SetReady(i, attr.tf_id, attr.GetCounted(), attr, true);
		}
	}
	
	return true;*/
}


















Forecaster::Forecaster() {
	//AddValue<double>("Next open");
	
	// Neural Network structure
	t =		"[\n"
			"\t{\"type\":\"input\", \"input_width\":1, \"input_height\":1, \"input_depth\":1},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"sigmoid\"},\n"
			"\t{\"type\":\"regression\", \"neuron_count\":1},\n"
			"\t{\"type\":\"adadelta\", \"learning_rate\":0.01, \"momentum\":0, \"batch_size\":1, \"l2_decay\":0.001}\n"
			"]\n";
	
	single = -1;
	pair = -1;
	/*SetStyle(
		"{"
			//"\"window_type\":\"SEPARATE\","
			//"\"minimum\":-1,"
			//"\"maximum\":1,"
			//"\"point\":0.01,"
			
			"\"value0\":{"
				"\"label\":\"Reward\","
				"\"color\":\"64,128,192\","
				"\"style\":\"HISTOGRAM\","
				"\"line_width\":2,"
				"\"chr\":95,"
				"\"begin\":10,"
				"\"shift\":0"
			"}"
		"}"
	);*/
}

String Forecaster::GetStyle() const {
	
}

void Forecaster::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("single");
	if (i != -1) single = args[i];
	i = args.Find("pair");
	if (i != -1) pair = args[i];
}

void Forecaster::Init() {
	/*BaseSystem& ol = Get<BaseSystem>();
	
	//AddDependency("/open", 0, 0);
	//AddDependency("/change", 0, 0);
	//AddDependency("/lstm", 0, 0);
	//AddDependency("/narx", 0, 0);
	//AddDependency("/whstat_slow", 0, 0);
	//AddDependency("/whdiff", 0, 0);
	//AddDependency("/chp", 0, 0);
	//AddDependency("/eosc", 0, 0);
	//AddDependency("/ma", 0, 0);
	//AddDependency("/aenc", 0, 0);
	SetProcessedOnce(false);
	
	tf_count = ol.GetPeriodCount();
	sym_count = ol.GetSymbolCount();
	
	// Total input values:
	//  - 1 previous value difference (change)
	//  - 16 predictions from LSTM
	//  - 1 prediction from NARX
	//  - 12 min/max from WdayHourStats
	//  - 12 values from WdayHourDiff
	//  - 7 min/max from ChannelPredicter
	//  - 4 from EventOsc
	//  - 2 from moving average
	//  = 55
	input.Init(55, 1, 1, 0);
	
	output.Init(1, 1, 1, 0);
	
	data.SetCount(sym_count * tf_count);
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		s.ses.SetTestPredict(true);
		s.ses.SetPredictInterval(10);
		ASSERTEXC(s.ses.MakeLayers(t));
	}
	*/
	do_training = true;
}

bool Forecaster::Process(const CoreProcessAttributes& attr) {
	/*
	const Core& src		= GetDependency(0);
	const Core& change	= GetDependency(1);
	const Core& lstm	= GetDependency(2);
	const Core& narx	= GetDependency(3);
	const Core& whstat	= GetDependency(4);
	const Core& whdiff	= GetDependency(5);
	const Core& chp		= GetDependency(6);
	const Core& eosc	= GetDependency(7);
	const Core& ma		= GetDependency(8);
	Panic("TODO: use also autoencoder");
	
	// Check if position is useless for training
	double* open = src.GetValue<double>(0, 0, attr);
	double* prev = src.GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	SymTf& s = GetData(attr);
	
	
	// Create input to ConvNet::Session
	int pos = 0;
	double* d;
	d = change.GetValue<double>(0, 0, attr);
	input.Set(pos++, *d);
	for(int i = 0; i < 16; i++) {
		d = lstm.GetValue<double>(i, attr);
		input.Set(pos++, *d);
	}
	d = narx.GetValue<double>(0, attr);
	input.Set(pos++, *d);
	for(int i = 0; i < 12; i++) {
		d = whstat.GetValue<double>(i, attr);
		input.Set(pos++, *d);
	}
	for(int i = 0; i < 12; i++) {
		d = whdiff.GetValue<double>(i, attr);
		input.Set(pos++, *d);
	}
	for(int i = 0; i < 7; i++) {
		d = chp.GetValue<double>(i, attr);
		input.Set(pos++, *d);
	}
	for(int i = 0; i < 4; i++) {
		d = eosc.GetValue<double>(i, attr);
		input.Set(pos++, *d);
	}
	{
		double* cur = ma.GetValue<double>(0, 0, attr);
		double* prev = ma.GetValue<double>(0, 1, attr);
		double* prev2 = ma.GetValue<double>(0, 2, attr);
		double diff = 0;
		double diffdiff = 0;
		if (prev) {
			diff = *cur - *prev;
			if (prev2) {
				double prevdiff = *prev - *prev2;
				diffdiff = diff - prevdiff;
			}
		}
		input.Set(pos++, diff);
		input.Set(pos++, diffdiff);
	}
	ASSERT(pos == input.GetLength());
	
	
	// Create output
	d = change.GetValue<double>(0, -1, attr); // peeks future: only for training
	output.Set(0, *d);
	
	
	// Train once
	s.ses.TrainOnce(input, output.GetWeights());
	
	
	// Write value. Network has been forward-propagated in the TrainOnce.
	double value = s.ses.GetNetwork().GetOutput().Get(0);
	double* out = GetValue<double>(0, attr);
	*out = value;
	
	
	return do_training;
	*/
}

















ClassifierAgent::ClassifierAgent() {
	ideal = false;
}

String ClassifierAgent::GetStyle() const {
	
}

void ClassifierAgent::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("ideal");
	if (i != -1) ideal = args[i];
}

void ClassifierAgent::Init() {
	//AddDependency("/open", 0, 0);
	//AddDependency("/ideal", 0, 0);
	
}

bool ClassifierAgent::Process(const CoreProcessAttributes& attr) {
	
	// Add classifier by looking Classify2D as example
	//  - but instead of 2D (x,y) input use previous values as input and ideal order signal as output
	Panic("TODO");
	
	return false;
}






















RLAgent::RLAgent() {
	//AddValue<char>("Signal");
	//AddValue<double>("Reward");
	
}

String RLAgent::GetStyle() const {
	return
		"{"
			"\"window_type\":\"SEPARATE\","
			//"\"minimum\":-1,"
			//"\"maximum\":1,"
			//"\"point\":0.01,"
			
			"\"value0\":{"
				"\"label\":\"Reward\","
				"\"color\":\"64,128,192\","
				"\"style\":\"HISTOGRAM\","
				"\"line_width\":2,"
				"\"chr\":95,"
				"\"begin\":10,"
				"\"shift\":0"
			"}"
		"}";
}

void RLAgent::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("ideal");
	if (i != -1) ideal = args[i];
}

void RLAgent::Init() {
	/*
	BaseSystem& ol = Get<BaseSystem>();
	
	//AddDependency("/open", 0, 0);
	//AddDependency("/change", 0, 0);
	//AddDependency("/forecaster", 0, 0);
	SetProcessedOnce(false);
	
	tf_count = ol.GetPeriodCount();
	max_shift = 4;
	sym_count = ol.GetSymbolCount();
	total = max_shift * 2;
	
	data.SetCount(sym_count * tf_count);
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		
		// setup virtual brokerage
		s.broker.InitLightweight();
		s.broker.SetFreeMarginLevel(0.7);
		
		// braaainzzz.....
		s.brain.Init(total, 3); // actions: short, idle, ong
		
		s.action = ACT_IDLE;
		s.prev_action = ACT_IDLE;
	}
	
	
	do_training = true;
	*/
}

bool RLAgent::Process(const CoreProcessAttributes& attr) {
	/*
	const Core& src = GetDependency(0);
	
	// Check if position is useless for training
	double* open = src.GetValue<double>(0, 0, attr);
	double* prev = src.GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	// Return reward value
	Backward(attr);
	
	// Get new action
	Forward(attr);
	
	return do_training;
	*/
}

void RLAgent::Forward(const CoreProcessAttributes& attr) {
	/*
	const Core& change = GetDependency(1);
	const Core& forecaster = GetDependency(2);
	SymTf& s = GetData(attr);
	
	// in forward pass the agent simply behaves in the environment
	// create input to brain
	input_array.SetCount(total);
	int pos = 0;
	double* value;
	for(int k = 0; k < max_shift; k++) {
		value = change.GetValue<double>(0, k, attr);
		input_array[pos++] = value ? *value : 0;
		value = forecaster.GetValue<double>(0, k, attr);
		input_array[pos++] = value ? *value : 0;
	}
	ASSERT(pos == total);
	
	// get action from brain
	s.prev_action = s.action;
	s.action = s.brain.Forward(input_array);
	s.broker.SetSignal(attr.sym_id, s.action - 1); // from 0:2 to -1:+1 --> signal
	s.broker.Cycle();
	*/
}

void RLAgent::Backward(const CoreProcessAttributes& attr) {
	/*
	const Core& change = GetDependency(1);
	SymTf& s = GetData(attr);
	
	double value = s.broker.GetPreviousCycleChange();
	
	// in backward pass agent learns.
	// compute reward
	s.reward = value;
	
	// pass to brain for learning
	s.brain.Backward(s.reward);
	
	// Write reward to oscillator
	char* sig = GetValue<char>(0, attr);
	*sig = s.action == ACT_LONG ? 1 : s.action == ACT_SHORT ? -1 : 0;
	double* out = GetValue<double>(1, attr);
	*out = s.reward;
	*/
}




















DQNAgent::DQNAgent() {
	//AddValue<char>("Signal");
	//AddValue<double>("Reward");
	
}

String DQNAgent::GetStyle() const {
	return
		"{"
			"\"window_type\":\"SEPARATE\","
			//"\"minimum\":-1,"
			//"\"maximum\":1,"
			//"\"point\":0.01,"
			
			"\"value0\":{"
				"\"label\":\"Reward\","
				"\"color\":\"64,128,192\","
				"\"style\":\"HISTOGRAM\","
				"\"line_width\":2,"
				"\"chr\":95,"
				"\"begin\":10,"
				"\"shift\":0"
			"}"
		"}";
}

void DQNAgent::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("ideal");
	if (i != -1) ideal = args[i];
}

void DQNAgent::Init() {
	/*
	BaseSystem& ol = Get<BaseSystem>();
	
	//AddDependency("/open", 0, 0);
	//AddDependency("/change", 0, 0);
	//AddDependency("/forecaster", 0, 0);
	SetProcessedOnce(false);
	
	tf_count = ol.GetPeriodCount();
	max_shift = 8;
	sym_count = ol.GetSymbolCount();
	total = max_shift * 2;
	
	data.SetCount(sym_count * tf_count);
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		
		// setup virtual brokerage
		s.broker.InitLightweight();
		s.broker.SetFreeMarginLevel(0.7);
		
		// My name is Bond, James Bond.
		s.agent.Init(1, total+1, 11); // actions: acceleration range -5 +5
		s.agent.Reset();
		
		s.action = 0;
		s.prev_action = 0;
		s.velocity = 0;
	}
	
	max_velocity = 5;
	do_training = true;
	*/
}

bool DQNAgent::Process(const CoreProcessAttributes& attr) {
	/*const Core& src = GetDependency(0);
	
	// Check if position is useless for training
	double* open = src.GetValue<double>(0, 0, attr);
	double* prev = src.GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	// Return reward value
	Backward(attr);
	
	// Get new action
	Forward(attr);
	
	return do_training;*/
}


void DQNAgent::Forward(const CoreProcessAttributes& attr) {
	/*const Core& change = GetDependency(1);
	const Core& forecaster = GetDependency(2);
	SymTf& s = GetData(attr);
	
	// in forward pass the agent simply behaves in the environment
	// create input to brain
	input_array.SetCount(total + 1);
	int pos = 0;
	for(int k = 0; k < max_shift; k++) {
		double* value = change.GetValue<double>(0, k, attr);
		input_array[pos++] = value ? *value : 0;
		value = forecaster.GetValue<double>(0, k, attr);
		input_array[pos++] = value ? *value : 0;
	}
	ASSERT(pos == total);
	input_array[pos++] = (double)s.velocity / (double)max_velocity;
	
	// get action from agent
	for(int i = 0; i < 10; i++) {
		s.prev_action = s.action;
		s.action = s.agent.Act(input_array);
		int acc = s.action - max_velocity;
		if (acc == 0) break;
		s.velocity = Upp::max(Upp::min(s.velocity + acc, +max_velocity), -max_velocity);
		input_array[total] = (double)s.velocity / (double)max_velocity;
	}
	
	// Only the last action is exported
	s.broker.SetSignal(attr.sym_id, s.action);
	s.broker.Cycle();*/
}

void DQNAgent::Backward(const CoreProcessAttributes& attr) {
	/*const Core& change = GetDependency(1);
	SymTf& s = GetData(attr);
	
	double value = s.broker.GetPreviousCycleChange();
	
	// in backward pass agent learns.
	// compute reward
	s.reward = value;
	
	// pass to brain for learning
	s.agent.Learn(s.reward);
	
	// Write reward to oscillator
	char* sig = GetValue<char>(0, attr);
	*sig = s.velocity;
	double* out = GetValue<double>(1, attr);
	*out = s.reward;*/
}























MonaAgent::MonaAgent() {
	//AddValue<char>("Signal");
	//AddValue<double>("Reward");
	
	// Cheese need and goal.
	CHEESE_NEED = 1.0;
	CHEESE_GOAL = 0.5;
	
}

String MonaAgent::GetStyle() const {
	return
		"{"
			"\"window_type\":\"SEPARATE\","
			//"\"minimum\":-1,"
			//"\"maximum\":1,"
			//"\"point\":0.01,"
			
			"\"value0\":{"
				"\"label\":\"Reward\","
				"\"color\":\"64,128,192\","
				"\"style\":\"HISTOGRAM\","
				"\"line_width\":2,"
				"\"chr\":95,"
				"\"begin\":10,"
				"\"shift\":0"
			"}"
		"}";
}

void MonaAgent::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("ideal");
	if (i != -1) ideal = args[i];
}

void MonaAgent::Init() {
	/*BaseSystem& ol = Get<BaseSystem>();
	
	//AddDependency("/open", 0, 0);
	//AddDependency("/change", 0, 0);
	//AddDependency("/forecaster", 0, 0);
	SetProcessedOnce(false);
	
	tf_count = ol.GetPeriodCount();
	max_shift = 10;
	sym_count = ol.GetSymbolCount();
	total = max_shift * 2 + 2;
	
	data.SetCount(sym_count * tf_count);
	
	// Goal vector 0 .... 3, 0.01
	input_array.SetCount(0);
	input_array.SetCount(max_shift + 2, 0.0);
	input_array[max_shift+0] = CLOSE;
	input_array[max_shift+1] = 0.01;
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		
		// setup virtual brokerage
		s.broker.InitLightweight();
		s.broker.SetFreeMarginLevel(0.7);
		
		// My, my, my, aye-aye, whoa!
		// M-m-m-my Mona
		// M-m-m-my Mona
		s.mona.MAX_MEDIATOR_LEVEL = 1;
		s.mona.Init(max_shift+2, 4, 1);
		
		// Set a long second effect interval
		// for a higher level mediator.
		s.mona.SetEffectEventIntervals(1, 2);
		s.mona.SetEffectEventInterval(1, 0, 2, 0.5);
		s.mona.SetEffectEventInterval(1, 1, 10, 0.5);
		
		// Set need and goal for cheese.
		s.mona.SetNeed(0, CHEESE_NEED);
		s.mona.AddGoal(0, input_array, 0, CHEESE_GOAL);
		
		// Reset need.
	    s.mona.SetNeed(0, CHEESE_NEED);
		s.mona.ClearResponseOverride();
		
		s.action = IDLE;
		s.prev_action = IDLE;
		s.reward = 0;
		s.prev_open = 0;
	}
	
	do_training = true;*/
}

bool MonaAgent::Process(const CoreProcessAttributes& attr) {
	/*const Core& src = GetDependency(0);
	const Core& change = GetDependency(1);
	const Core& forecaster = GetDependency(2);
	
	// Check if position is useless for training
	double* open = src.GetValue<double>(0, 0, attr);
	double* prev = src.GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	SymTf& s = GetData(attr);
	
	// in backward pass agent learns.
	// compute reward
	s.reward = s.broker.GetPreviousCycleChange();
	
	
	// in forward pass the agent simply behaves in the environment
	// create input to brain
	input_array.SetCount(total);
	int pos = 0;
	for(int k = 0; k < max_shift; k++) {
		double* value = change.GetValue<double>(0, k, attr);
		input_array[pos++] = value ? *value : 0;
		value = forecaster.GetValue<double>(0, k, attr);
		input_array[pos++] = value ? *value : 0;
	}
	input_array[pos++] = s.action;
	input_array[pos++] = s.reward;
	ASSERT(pos == total);
	
	
	// Calculate action
	s.prev_action = s.action;
	s.action = s.mona.Cycle(input_array);
	ASSERT(s.action >= IDLE && s.action <= CLOSE);
	
	
	// Write default oscillator value
	double* out = GetValue<double>(1, attr);
	*out = 0.0;
	
	
	// When action changes
	if (s.action == IDLE) {
		// If previous state wasn't IDLE, then override state to that
		if (s.prev_action != s.action) {
			ASSERT(s.prev_action != CLOSE); // this would be weird error
			s.mona.OverrideResponse(s.prev_action);
			s.action = s.mona.Cycle(input_array);
			s.mona.ClearResponseOverride();
		}
	}
	else if (s.action == CLOSE && s.prev_action == IDLE) {
		s.mona.OverrideResponse(IDLE);
		s.action = s.mona.Cycle(input_array);
		s.mona.ClearResponseOverride();
	}
	else if (s.action != s.prev_action) {
		
		// Close if currently open
		if (s.prev_action != IDLE) {
			
			// Write reward to oscillator
			*out = s.reward;
		
			// Set close state
			for(int k = 0; k < max_shift; k++)
				input_array[k] = 0.0;
			input_array[max_shift] = CLOSE;
			input_array[max_shift+1] = s.reward; // useless, should be same already
			
			// Override action (response) to what Mona decided already previously
			// but if the action was CLOSE then change it to the IDLE
			if (s.action == CLOSE) s.action = IDLE;
			s.mona.OverrideResponse(s.action);
			int act = s.mona.Cycle(input_array); // this was similar to the backpropagation or the learning experience
			ASSERT(act == s.action);
			s.mona.ClearResponseOverride();
			
		    // Reset need.
		    s.mona.SetNeed(0, CHEESE_NEED);
		
		    // Clear working memory.
		    s.mona.ClearWorkingMemory();
		}
		
		double* open = src.GetValue<double>(0, 0, attr);
		s.prev_open = *open;
	}
	
	
	// Write signal
	char* sig = GetValue<char>(0, attr);
	*sig = s.action == LONG ? 1 : s.action == SHORT ? -1 : 0;
	
	// Only the last action is exported
	s.broker.SetSignal(attr.sym_id, *sig);
	s.broker.Cycle();
	
	return do_training;*/
}

























MonaMetaAgent::MonaMetaAgent() {
	//AddValue<char>("Signal");
	//AddValue<double>("Reward");
	
	// Cheese need and goal.
	CHEESE_NEED = 1.0;
	CHEESE_GOAL = 0.5;
	
}

String MonaMetaAgent::GetStyle() const {
	return
		"{"
			"\"window_type\":\"SEPARATE\","
			//"\"minimum\":-1,"
			//"\"maximum\":1,"
			//"\"point\":0.01,"
			
			"\"value0\":{"
				"\"label\":\"Reward\","
				"\"color\":\"64,128,192\","
				"\"style\":\"HISTOGRAM\","
				"\"line_width\":2,"
				"\"chr\":95,"
				"\"begin\":10,"
				"\"shift\":0"
			"}"
		"}";
}

void MonaMetaAgent::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("ideal");
	if (i != -1) ideal = args[i];
}

void MonaMetaAgent::Init() {
	/*BaseSystem& ol = Get<BaseSystem>();
	
	//AddDependency("/open", 0, 0);
	//AddDependency("/rl", 0, 1);
	//AddDependency("/dqn", 0, 1);
	//AddDependency("/mona", 0, 1);
	//AddDependency("/change", 0, 0);
	//AddDependency("/forecaster", 0, 1);
	SetProcessedOnce(false);
	
	tf_count = ol.GetPeriodCount();
	sym_count = ol.GetSymbolCount();
	total = 3 * tf_count + 2;
	
	data.SetCount(sym_count * tf_count);
	
	// Goal vector 0 .... 3, 0.01
	input_array.SetCount(0);
	input_array.SetCount(total, 0.0);
	input_array[total-2] = CLOSE;
	input_array[total-1] = 0.01;
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		
		// setup virtual brokerage
		s.broker.InitLightweight();
		s.broker.SetFreeMarginLevel(0.7);
		
		// My, my, my, aye-aye, whoa!
		// M-m-m-my Mona
		// M-m-m-my Mona
		s.mona.MAX_MEDIATOR_LEVEL = 1;
		s.mona.Init(total, 4, 1);
		
		// Set a long second effect interval
		// for a higher level mediator.
		s.mona.SetEffectEventIntervals(1, 2);
		s.mona.SetEffectEventInterval(1, 0, 2, 0.5);
		s.mona.SetEffectEventInterval(1, 1, 10, 0.5);
		
		// Set need and goal for cheese.
		s.mona.SetNeed(0, CHEESE_NEED);
		s.mona.AddGoal(0, input_array, 0, CHEESE_GOAL);
		
		// Reset need.
	    s.mona.SetNeed(0, CHEESE_NEED);
		s.mona.ClearResponseOverride();
		
		s.action = IDLE;
		s.prev_action = IDLE;
		s.reward = 0;
		s.prev_open = 0;
	}
	
	do_training = true;*/
}

bool MonaMetaAgent::Process(const CoreProcessAttributes& attr) {
	/*const Core& src = GetDependency(0);
	const Core& rl = GetDependency(1);
	const Core& dqn = GetDependency(2);
	const Core& mona = GetDependency(3);
	const Core& change = GetDependency(4);
	
	// Check if position is useless for training
	double* open = src.GetValue<double>(0, 0, attr);
	double* prev = src.GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	SymTf& s = GetData(attr);
	
	// in backward pass agent learns.
	// compute reward
	s.reward = s.broker.GetPreviousCycleChange();
	
	
	// in forward pass the agent simply behaves in the environment
	// create input to brain
	input_array.SetCount(total);
	int pos = 0;
	for(int i = 0; i < 3; i++) {
		const Core& sigsrc = i == 0 ? rl : i == 1 ? dqn : mona;
		for(int j = 0; j < tf_count; j++) {
			char* sig = sigsrc.GetValue<char>(0, j, 0, attr);
			input_array[pos++] = *sig;
		}
	}
	input_array[pos++] = s.action;
	input_array[pos++] = s.reward;
	ASSERT(pos == total);
	
	
	// Calculate action
	s.prev_action = s.action;
	s.action = s.mona.Cycle(input_array);
	ASSERT(s.action >= IDLE && s.action <= CLOSE);
	
	
	// Write default oscillator value
	double* out = GetValue<double>(1, attr);
	*out = 0.0;
	
	
	// When action changes
	if (s.action == IDLE) {
		// If previous state wasn't IDLE, then override state to that
		if (s.prev_action != s.action) {
			ASSERT(s.prev_action != CLOSE); // this would be weird error
			s.mona.OverrideResponse(s.prev_action);
			s.action = s.mona.Cycle(input_array);
			s.mona.ClearResponseOverride();
		}
	}
	else if (s.action == CLOSE && s.prev_action == IDLE) {
		s.mona.OverrideResponse(IDLE);
		s.action = s.mona.Cycle(input_array);
		s.mona.ClearResponseOverride();
	}
	else if (s.action != s.prev_action) {
		
		// Close if currently open
		if (s.prev_action != IDLE) {
			
			// Write reward to oscillator
			*out = s.reward;
		
			// Set close state
			for(int k = 0; k < total; k++)
				input_array[k] = 0.0;
			input_array[total-2] = CLOSE;
			input_array[total-1] = s.reward; // useless, should be same already
			
			// Override action (response) to what Mona decided already previously
			// but if the action was CLOSE then change it to the IDLE
			if (s.action == CLOSE) s.action = IDLE;
			s.mona.OverrideResponse(s.action);
			int act = s.mona.Cycle(input_array); // this was similar to the backpropagation or the learning experience
			ASSERT(act == s.action);
			s.mona.ClearResponseOverride();
			
		    // Reset need.
		    s.mona.SetNeed(0, CHEESE_NEED);
		
		    // Clear working memory.
		    s.mona.ClearWorkingMemory();
		}
		double* open = src.GetValue<double>(1, attr);
		s.prev_open = *open;
	}
	
	
	// Write signal
	char* sig = GetValue<char>(0, attr);
	*sig = s.action == LONG ? 1 : s.action == SHORT ? -1 : 0;
	
	
	// Only the last action is exported
	s.broker.SetSignal(attr.sym_id, *sig);
	s.broker.Cycle();
	
	
	return do_training;*/
}
























MonaDoubleAgent::MonaDoubleAgent() {
	//AddValue<char>("Signal");
	//AddValue<double>("Reward");
	
	// Cheese need and goal.
	CHEESE_NEED = 1.0;
	CHEESE_GOAL = 0.5;
	
}

String MonaDoubleAgent::GetStyle() const {
	return
		"{"
			"\"window_type\":\"SEPARATE\","
			//"\"minimum\":-1,"
			//"\"maximum\":1,"
			//"\"point\":0.01,"
			
			"\"value0\":{"
				"\"label\":\"Reward\","
				"\"color\":\"64,128,192\","
				"\"style\":\"HISTOGRAM\","
				"\"line_width\":2,"
				"\"chr\":95,"
				"\"begin\":10,"
				"\"shift\":0"
			"}"
		"}";
}

void MonaDoubleAgent::SetArguments(const VectorMap<String, Value>& args) {
	
}

void MonaDoubleAgent::Init() {
	/*BaseSystem& ol = Get<BaseSystem>();
	
	AddDependency("/open", 1, 0);
	AddDependency("/metamona", 1, 0);
	SetProcessedOnce(false);
	
	tf_count = ol.GetPeriodCount();
	sym_count = ol.GetSymbolCount();
	
	// Sensors total:
	// - data sensors = sym_count
	// - current states per symbol = sym_count
	// - how many states are active (open orders)
	// - reward since last all-closed state
	// - max fraction of equity to use to open orders
	// - change in previous cycle (usually backward reward)
	total = sym_count + sym_count + 1 + 1 + 1 + 1;
	
	
	// Actions total:
	// - WAIT + INC_ACCOUNT_FACTOR + DEC_ACCOUNT_FACTOR + sym_count * [SHORT, LONG]
	actions_total = 3 + sym_count * 2;
	
	
	// Goal vector 0 .... 0.01, 0
	input_array.SetCount(0);
	input_array.SetCount(total, 0.0);
	input_array[total-3] = 0.01;
	
	// setup virtual brokerage
	broker.InitLightweight();
	broker.SetFreeMarginLevel(0.7);
	
	// My, my, my, aye-aye, whoa!
	// M-m-m-my Mona
	// M-m-m-my Mona
	agent.MAX_MEDIATOR_LEVEL = 1;
	agent.Init(total, actions_total, 1);
	
	// Set a long second effect interval
	// for a higher level mediator.
	agent.SetEffectEventIntervals(1, 2);
	agent.SetEffectEventInterval(1, 0, 2, 0.5);
	agent.SetEffectEventInterval(1, 1, 10, 0.5);
	
	// Set need and goal for cheese.
	agent.SetNeed(0, CHEESE_NEED);
	agent.AddGoal(0, input_array, 0, CHEESE_GOAL);
	
	// Reset need.
    agent.SetNeed(0, CHEESE_NEED);
	agent.ClearResponseOverride();
	
	action = 0;
	prev_action = 0;
	velocity.SetCount(sym_count, 0);
	
	max_velocity = 5;
	do_training = true;*/
}

bool MonaDoubleAgent::Process(const CoreProcessAttributes& attr) {
	/*const Core& src = GetDependency(0);
	const Core& metamona = GetDependency(1);
	
	if (attr.tf_id == 0 && attr.sym_id == 0) {
		*/
		// Check if position is useless for training
		/*double* open = src.GetValue<double>(0, 0, attr);
		double* prev = src.GetValue<double>(0, 1, attr);
		if (!prev || *prev == *open)
			return true;*/
		/*
		LOG(Format("MonaDoubleAgent::Process sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
		
		// Get new action
		Forward(attr);
	}
	
	// Write signal
	char* sig = GetValue<char>(0, attr);
	*sig = velocity[attr.sym_id];
	
	// Write signal to symbol's data-row
	double* out = GetValue<double>(0, attr);
	*out = broker.GetSignal(attr.sym_id);
	
	return do_training;*/
}


void MonaDoubleAgent::Forward(const CoreProcessAttributes& attr) {
	/*const Core& metamona = GetDependency(1);
	
	// Reserve memory
	input_array.SetCount(total);
	int pos = 0;
	
	// Write sensor values;
	for(int i = 0; i < sym_count; i++) {
		char* sig = metamona.GetValue<char>(0, i, attr.tf_id, 0, attr);
		input_array[pos++] = *sig;
	}
	
	
	// Write state values
	int state_begin = pos;
	for(int i = 0; i < sym_count; i++) {
		input_array[pos++] = (double)velocity[i] / (double)max_velocity;
	}
	
	
	// Write open orders and moving average of reward
	input_array[pos++] = broker.GetOpenOrderCount();
	input_array[pos++] = broker.GetWorkingMemoryChange();
	input_array[pos++] = broker.GetFreeMarginLevel();
	input_array[pos++] = broker.GetPreviousCycleChange(); // reward
	ASSERT(pos == total);
	
	
	// Loop actions until WAIT is received. In this way the velocity vector can be fine tuned
	// as long as it is needed and single errorneous action does not have too much weight.
	int max_actions = sym_count * 3;
	for (int i = 0; i < max_actions; i++) {
		
		// Get action from agent
		prev_action = action;
		action = agent.Cycle(input_array);
		
		// WAIT action breaks loop
		if (action == 0) break;
		
		// Increase free margin level
		else if (action == 1) {
			broker.SetFreeMarginLevel(broker.GetFreeMarginLevel() + 0.05);
			input_array[total-2] = broker.GetFreeMarginLevel();
		}
		
		// Decrease free margin level
		else if (action == 2) {
			broker.SetFreeMarginLevel(broker.GetFreeMarginLevel() - 0.05);
			input_array[total-2] = broker.GetFreeMarginLevel();
		}
		
		// Make change to signal
		else {
			action -= 3;
			int sym = action / 2;
			int act = action % 2;
			int& vel = velocity[sym];
			int acc = act == 0 ? +1 : -1;
			vel = Upp::max(Upp::min(vel + acc, +max_velocity), -max_velocity);
			double sig = (double)vel / (double)max_velocity;
			broker.SetSignal(sym, sig);
			
			// Write new value to the input_array for next agent.Act
			input_array[state_begin + sym] = sig;
		}
		
		
		// Check for goal state
		if (broker.GetOpenOrderCount() == 0 && broker.IsZeroSignal()) {
			
			// Set goal state, or at least as close as possible
			input_array.SetCount(0);
			input_array.SetCount(total, 0);
			input_array[total-3] = broker.GetWorkingMemoryChange();
			action = agent.Cycle(input_array);
			
			// Clear working memory.
		    agent.ClearWorkingMemory();
		    broker.ClearWorkingMemory();
		}
	}
	
	broker.Cycle();*/
}

}
