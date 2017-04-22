#include "DataCore.h"

namespace DataCore {

Recurrent::Recurrent() {
	shifts = 4;
	for(int i = 0; i < shifts; i++) {
		
		// Different softmax sample temperatures:
		//   lower setting will generate more likely predictions,
		//   but you'll see more of the same common values again and again. Higher
		//   setting will generate less frequent values but you might see more unrealistic errors.
		AddValue<double>();  // "coldest"	greedy argmax prediction
		AddValue<double>();  // "cold"		low  temperate prediction
		AddValue<double>();  // "warm"		med  temperate prediction
		AddValue<double>();  // "hot"		high temperate prediction
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
	
	src = FindLinkSlot("/open");
	ASSERTEXC(src);
	
	temperatures.SetCount(3);
	temperatures[0] = 0.1;
	temperatures[1] = 1.0;
	temperatures[2] = 5.0;
	
	tick_time = 0;
	iter = 0;
	value_count = 20;
	batch = 30;
	is_training_loop = true;
	phase = PHASE_GATHERDATA;
	
	TimeVector& tv = GetTimeVector();
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	ses.SetCount(sym_count);
	for(int i = 0; i < ses.GetCount(); i++)
		ses[i].SetCount(tf_count);
	
	LoadThis();
}

double Recurrent::GetPerplexity() const {
	double av = 0.0;
	int count = 0;
	for(int i = 0; i < ses.GetCount(); i++) {
		for(int j = 0; j < ses[i].GetCount(); j++) {
			av += ses[i][j].GetPerplexity();
			count++;
		}
	}
	return av / count;
}

void Recurrent::SetLearningRate(double rate) {
	for(int i = 0; i < ses.GetCount(); i++) {
		for(int j = 0; j < ses[i].GetCount(); j++) {
			ses[i][j].SetLearningRate(rate);
		}
	}
}

bool Recurrent::Process(const SlotProcessAttributes& attr) {
	
	// In first phase, end of data is required to find all possible changes
	if (phase == PHASE_GATHERDATA) {
		
		// Gather statistics about the source values
		if (attr.GetCounted() > 0) {
			double* prev_value = src->GetValue<double>(0, 1, attr);
			double* src_value  = src->GetValue<double>(0, 0, attr);
			var.AddResult(*src_value - *prev_value);
		}
		
		// In the end
		if (attr.GetCounted() == attr.GetBars()-1) {
			Reload();
			phase = PHASE_TRAINING;
		}
		
		// there are other phases after this, so return "unprocessed"
		return false;
	}
	
	// In the second phase, training and prediction writing is processed
	else if (phase == PHASE_TRAINING) {
		
		if (attr.tf_id == 0 && attr.sym_id == 0)
			iter++;
		
		// Write current values to the sentence
		int count = Upp::min(batch, attr.GetCounted()-1);
		if (count > 10) {
			lock.Enter();
			
			// Run single training
			// Run only unique batches
			if (attr.GetCounted() % batch == 0) {
				sequence.SetCount(count);
				double prev_value = *src->GetValue<double>(0, count, attr);
				for(int i = 0; i < count; i++) {
					double* src_value = src->GetValue<double>(0, count-1-i, attr);
					double value = *src_value;
					sequence[i] = ToChar(value - prev_value);
					prev_value = value;
				}
				Tick(attr);
			}
			/*
			sequence.SetCount(count);
			double prev_value = *src->GetValue<double>(0, count, attr);
			for(int i = 0; i < count; i++) {
				double* src_value = src->GetValue<double>(0, count-1-i, attr);
				double value = *src_value;
				sequence[i] = ToChar(value - prev_value);
				prev_value = value;
			}
			
			// Run single training
			// Run only unique batches
			if (attr.GetCounted() % batch == 0) {
				Tick(attr);
			}
			
			
			// Write predictions
			// For 4 temperatures
			double* src_value = src->GetValue<double>(0, attr);
			for(int i = 0; i < 4; i++) {
				
				// Erase the previous prediction (when i > 0)
				sequence.SetCount(count);
				
				// First is greedy argmax and after that are temperatures
				PredictSentence(i > 0, i == 0 ? 0 : temperatures[i-1]);
				int new_count = sequence.GetCount();
				ASSERT(new_count >= count);
				
				// For last 4 values and shifts of current temperature
				double value = *src_value;
				for(int j = count, k=0; j < count+4; j++, k+=4) {
					double* val = GetValue<double>(i+k, attr);
					if (j < new_count) value += FromChar(sequence[j]);
					*val = value;
				}
			}*/
			
			
			
			
			lock.Leave();
		}
		else {
			double value = *src->GetValue<double>(0, attr);
			for(int i = 0; i < 4*4; i++)
				*GetValue<double>(i, attr) = value;
		}
		
		
		// At last position, check if training should be finished
		if (attr.GetCounted() == attr.GetBars()-1) {
			
			
			//is_training_loop = false;
			
			StoreThis();
		}
		
		// Train this again until training loop is finished, after this, only new values are
		// trained.
		return !is_training_loop;
	}
	else return true;
}



void Recurrent::Reload() {
	// note: reinit writes global vars
	lock.Enter();
	
	iter = 0;
	tick_time = 0;
	
	//tick_iter = 0;
	
	InitVocab(); // takes count threshold for characters
	
	InitSessions();
	
	lock.Leave();
	//learning_rate_slider.SetData(ses.GetLearningRate() / 0.00001);
	//SetLearningRate();
}

void Recurrent::InitSessions() {
	// eval options to set some globals
	ValueMap js = ParseJSON(model_str);
	for(int i = 0; i < ses.GetCount(); i++) {
		for(int j = 0; j < ses[i].GetCount(); j++) {
			ConvNet::RecurrentSession& ses = this->ses[i][j];
			ses.Load(js);
			ses.SetInputSize(value_count+1); // possible values + START token
			ses.SetOutputSize(value_count+1);
			ses.Init();
		}
	}
}

int Recurrent::ToChar(double diff) {
	int d = (diff - min) / step;
	if (d < 0) d = 0;
	else if (d >= value_count) d = value_count-1;
	return d + 1; // first is START token
}

double Recurrent::FromChar(int i) {
	int d = i - 1;
	ASSERT(d >= 0 && d < value_count);
	return d * step + min;
}

void Recurrent::InitVocab() {
	
	// filter by count threshold and create pointers
	//letterToIndex.Clear();
	//indexToLetter.Clear();
	vocab.Clear();
	
	// Create "vocabulary", or acceptable rounded values
	min = var.GetMean() - var.GetVariance();
	max = var.GetMean() + var.GetVariance();
	diff = max - min;
	step = diff / (value_count-1);
	/*
	double v = min;
	
	// NOTE: start at one because we will have START and END tokens!
	// that is, START token will be index 0 in model letter vectors
	// and END token will be index 0 in the next character softmax
	int q = 1;
	for (int i = 0; i < count; i++) {
		
		// add character to vocab
		letterToIndex.Add(i, q);
		indexToLetter.Add(q, v);
		vocab.Add(v);
		q++;
		
		v += step;
	}
	*/
	// globals written: indexToLetter, letterToIndex, vocab (list), and:
	/*input_size	= vocab.GetCount() + 1;
	output_size	= vocab.GetCount() + 1;
	epoch_size	= sents.GetCount();
	
	input_stats.SetLabel(Format("found %d distinct characters: %s", vocab.GetCount(), Join(vocab, WString(""))));*/
}

void Recurrent::PredictSentence(const SlotProcessAttributes& attr, bool samplei, double temperature) {
	ses[attr.sym_id][attr.tf_id].Predict(sequence, samplei, temperature, true, 4);
	/*
	WString s;
	for(int i = 0; i < sequence.GetCount(); i++) {
		int chr = indexToLetter.Get(sequence[i]);
		s.Cat(chr);
	}
	return s;*/
}

void Recurrent::Tick(const SlotProcessAttributes& attr) {
	TimeStop ts;  // log start timestamp
	
	// sample sentence fromd data
	/*int sentix = Random(data_sents.GetCount());
	const WString& sent = data_sents[sentix];
	
	sequence.SetCount(sent.GetCount());
	for(int i = 0; i < sent.GetCount(); i++) {
		sequence[i] = letterToIndex.Get(sent[i]);
	}*/
	
	ses[attr.sym_id][attr.tf_id].Learn(sequence);
	
	tick_time = ts.Elapsed();
	
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
	
	AddValue<double>();
	
}

void NARX::SetArguments(const VectorMap<String, Value>& args) {
	
}

void NARX::Init() {
	
	TimeVector& tv = GetTimeVector();
	
	ASSERTEXC_(hact >= 0 && hact < 3, "Hidden unit activation can be in range [0,2]");
	
	open = FindLinkSlot("/open");
	ASSERTEXC(open);
	change = FindLinkSlot("/change");
	ASSERTEXC(change);
	Panic("Change must be from previous to current, not current to next");
	
	sym_count = tv.GetSymbolCount();
	tf_count = tv.GetPeriodCount();
	
	epoch = 0;
	
	data.SetCount(tf_count);
	for(int i = 0; i < tf_count; i++) {
		Tf& s = data[i];
		
		int slower_tfs = tf_count - 1 - i;
		s.input_count = slower_tfs * sym_count;	// total exogenous values
		
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
		
	}
}

bool NARX::Process(const SlotProcessAttributes& attr) {
	
	Panic("TODO: DateTime Exogen series");
	Panic("TODO: Total Volume");
	
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
					double* d = change->GetValue<double>(0, j, attr.tf_id, i, attr);
					in[i].SetInput(d ? *d : 0);
				}
			}
		}
		
		for (int i = 0; i < s.input_count; i++) {
			int sym = i % sym_count;
			int tf = attr.tf_id + 1 + i / sym_count;
			
			Vector<InputUnit>& exo = s.exogenous[i];
			for (int j = 0; j < a1; j++) {
				double* d = change->GetValue<double>(0, sym, tf, j, attr);
				exo[j].SetInput(d ? *d : 0);
			}
		}

		for (int i = 0; i < sym_count; i++) {
			double* d = change->GetValue<double>(0, i, attr.tf_id, -1, attr);
			double* p = change->GetValue<double>(0, i, attr.tf_id, 0, attr);
			ASSERT(d && *d != 0.0);
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
		
		for (int i = 0; i < s.input_count; i++) {
			int sym = i % sym_count;
			int tf = attr.tf_id + 1 + i / sym_count;
			
			Vector<InputUnit>& exo = s.exogenous[i];
			for (int j = a1-1; j >= 0 ; j--) {
				double* d = change->GetValue<double>(0, sym, tf, j, attr);
				exo[j].SetInput(d ? *d : 0);
				fi.X[i][j] = exo[j].GetInput();
			}
		}

		if (targets) {
			for (int i = 0; i < sym_count; i++) {
				Vector<InputUnit>& in = s.inputs[i];
				for (int j = b-1; j >= 0; j--) {
					double* d = change->GetValue<double>(0, i, attr.tf_id, j, attr);
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
			double* d = change->GetValue<double>(0, i, attr.tf_id, -1, attr);
			double* p = change->GetValue<double>(0, i, attr.tf_id, 0, attr);
			ASSERT(d && *d != 0.0);
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
			
			double* d = change->GetValue<double>(0, i, attr.tf_id, -1, attr);
			ASSERT(d && *d != 0.0);
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
	}
	
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
	
	
	
	
	
	
	// Predict
	{
		out.SetCount(sym_count);
		
		if (targets) {
			for (int j = 0; j < sym_count; j++) {
				Vector<InputUnit>& in = s.inputs[j];
				for (int i = 0; i < b; i++) {
					double* d = change->GetValue<double>(0, i, attr.tf_id, i, attr);
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
		
		for (int i = 0; i < s.input_count; i++) {
			Vector<InputUnit>& exo = s.exogenous[i];
			int sym = i % sym_count;
			int tf = attr.tf_id + 1 + i / sym_count;
			
			for (int j = 0; j < a1; j++) {
				double* d = change->GetValue<double>(0, sym, tf, j, attr);
				exo[j].SetInput(d ? *d : 0);
			}
			//_log(String("ok %1").arg(exogenous_series[i][series_index]));
			//FWhenLog(String("ok exo=%1\n").arg(exogenous_series[i][series_index]).toStdString().c_str());
		}
		
		for (int i = 0; i < sym_count; i++) {
			double out = s.output_units[i].GetOutput();
			ASSERT(out < 0.1 && out > -0.1); // sane limits for current usage, not generic
			s.pY[i] = out;
			
			// Set value
			double* prv = open->GetValue<double>(0, i, attr.tf_id, 0, attr);
			double* dst = GetValue<double>(0, i, attr.tf_id, 0, attr);
			*dst = *prv * (1.0 + out);
			
			// Mark the slot of the symbol as processed
			SetReady(i, attr.tf_id, attr.GetCounted(), attr, true);
		}
	}
	
	
	return true;
}




















Forecaster::Forecaster() {
	AddValue<double>("Next open");
	
	// Neural Network structure
	t =		"[\n"
			"\t{\"type\":\"input\", \"input_width\":1, \"input_height\":1, \"input_depth\":1},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"sigmoid\"},\n"
			"\t{\"type\":\"regression\", \"neuron_count\":1},\n"
			"\t{\"type\":\"adadelta\", \"learning_rate\":0.01, \"momentum\":0, \"batch_size\":1, \"l2_decay\":0.001}\n"
			"]\n";
		
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

void Forecaster::SetArguments(const VectorMap<String, Value>& args) {
	
}

void Forecaster::Init() {
	TimeVector& tv = GetTimeVector();
	
	src = tv.FindLinkSlot("/open");
	change = tv.FindLinkSlot("/change");
	rnn = tv.FindLinkSlot("/lstm");
	narx = tv.FindLinkSlot("/narx");
	chstat = tv.FindLinkSlot("/chstat");
	chp = tv.FindLinkSlot("/chp");
	ASSERTEXC(src);
	ASSERTEXC(change);
	ASSERTEXC(rnn);
	ASSERTEXC(chstat);
	ASSERTEXC(chp);
	
	tf_count = tv.GetPeriodCount();
	sym_count = tv.GetSymbolCount();
	
	// Total input values:
	//  - 1 previous value difference (change)
	//  - 16 predictions from LSTM
	//  - 1 prediction from NARX
	//  - 6 min/max from ChannelStats
	//  - 6 min/max from ChannelPredicter
	//  = 30
	Panic("TODO: check to read range -1 +1 values only");
	input.Init(30, 1, 1, 0);
	
	output.Init(1, 1, 1, 0);
	
	data.SetCount(sym_count * tf_count);
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		s.ses.SetTestPredict(true);
		s.ses.SetPredictInterval(10);
		ASSERTEXC(s.ses.MakeLayers(t));
	}
	
	do_training = true;
}

bool Forecaster::Process(const SlotProcessAttributes& attr) {
	
	// Check if position is useless for training
	double* open = src->GetValue<double>(0, 0, attr);
	double* prev = src->GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	SymTf& s = GetData(attr);
	
	// Create input to ConvNet::Session
	int pos = 0;
	double* d;
	d = change->GetValue<double>(0, 1, attr); // taking current peeks future
	input.Set(pos++, *d);
	for(int i = 0; i < 16; i++) {
		d = rnn->GetValue<double>(i, attr);
		input.Set(pos++, *d);
	}
	d = narx->GetValue<double>(0, attr);
	input.Set(pos++, *d);
	for(int i = 0; i < 6; i++) {
		d = chstat->GetValue<double>(i, attr);
		input.Set(pos++, *d);
	}
	for(int i = 0; i < 6; i++) {
		d = chp->GetValue<double>(i, attr);
		input.Set(pos++, *d);
	}
	ASSERT(pos == input.GetLength());
	
	// Train once
	s.ses.TrainOnce(input, output.GetWeights());
	
	// Write value. Network has been forward-propagated in the TrainOnce.
	double value = s.ses.GetNetwork().GetOutput().Get(0);
	double* out = GetValue<double>(0, attr);
	*out = value;
	
	return do_training;
}





















RLAgent::RLAgent() {
	AddValue<char>("Signal");
	AddValue<double>("Reward");
	
	SetStyle(
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
		"}"
	);
}

void RLAgent::SetArguments(const VectorMap<String, Value>& args) {
	
}

void RLAgent::Init() {
	TimeVector& tv = GetTimeVector();
	
	src = tv.FindLinkSlot("/open");
	//rnn = tv.FindLinkSlot("/rnn");
	ASSERTEXC(src);
	//ASSERTEXC(rnn);
	
	tf_count = tv.GetPeriodCount();
	max_shift = 4;
	sym_count = tv.GetSymbolCount();
	total = max_shift;
	
	data.SetCount(sym_count * tf_count);
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		
		// braaainzzz.....
		s.brain.Init(total, 3); // actions: idle, long, short
		
		s.action = ACT_IDLE;
		s.prev_action = ACT_IDLE;
	}
	
	
	do_training = true;
}

bool RLAgent::Process(const SlotProcessAttributes& attr) {
	Panic("TODO: add spread costs");
	
	
	// Check if position is useless for training
	double* open = src->GetValue<double>(0, 0, attr);
	double* prev = src->GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	// Return reward value
	Backward(attr);
	
	// Get new action
	Forward(attr);
	
	return do_training;
}

void RLAgent::Forward(const SlotProcessAttributes& attr) {
	SymTf& s = GetData(attr);
	
	// in forward pass the agent simply behaves in the environment
	// create input to brain
	input_array.SetCount(total);
	int pos = 0;
	double* prev = src->GetValue<double>(0, max_shift, attr);
	for(int k = 0; k < max_shift; k++) {
		double* open = src->GetValue<double>(0, max_shift-1-k, attr);
		if (prev) {
			double d = *open / *prev - 1.0;
			input_array[pos++] = d;
		} else {
			input_array[pos++] = 0;
		}
		prev = open;
	}
	ASSERT(pos == total);
	
	// get action from brain
	s.prev_action = s.action;
	s.action = s.brain.Forward(input_array);
}

void RLAgent::Backward(const SlotProcessAttributes& attr) {
	SymTf& s = GetData(attr);
	
	double* open = src->GetValue<double>(0, 0, attr);
	double* prev_open = src->GetValue<double>(0, 1, attr);
	double change = prev_open ? *open / *prev_open - 1.0: 0;
	if (s.action == ACT_SHORT) change *= -1;
	else if (s.action == ACT_IDLE) change = -0.00001;
	
	// in backward pass agent learns.
	// compute reward
	s.reward = change;
	
	// pass to brain for learning
	s.brain.Backward(s.reward);
	
	// Write reward to oscillator
	char* sig = GetValue<char>(0, attr);
	*sig = s.action == ACT_LONG ? 1 : s.action == ACT_SHORT ? -1 : 0;
	double* out = GetValue<double>(1, attr);
	*out = s.reward;
}




















DQNAgent::DQNAgent() {
	AddValue<char>("Signal");
	AddValue<double>("Reward");
	
	SetStyle(
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
		"}"
	);
}

void DQNAgent::SetArguments(const VectorMap<String, Value>& args) {
	
}

void DQNAgent::Init() {
	TimeVector& tv = GetTimeVector();
	
	src = tv.FindLinkSlot("/open");
	//rnn = tv.FindLinkSlot("/rnn");
	ASSERTEXC(src);
	//ASSERTEXC(rnn);
	
	tf_count = tv.GetPeriodCount();
	max_shift = 8;
	sym_count = tv.GetSymbolCount();
	total = max_shift;
	
	data.SetCount(sym_count * tf_count);
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		
		// My name is Bond, James Bond.
		s.agent.Init(1, total+1, 11); // actions: acceleration range -5 +5
		s.agent.Reset();
		
		s.action = 0;
		s.prev_action = 0;
		s.velocity = 0;
	}
	
	max_velocity = 5;
	do_training = true;
}

bool DQNAgent::Process(const SlotProcessAttributes& attr) {
	Panic("TODO: add spread costs");
	
	// Check if position is useless for training
	double* open = src->GetValue<double>(0, 0, attr);
	double* prev = src->GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	// Return reward value
	Backward(attr);
	
	// Get new action
	Forward(attr);
	
	return do_training;
}


void DQNAgent::Forward(const SlotProcessAttributes& attr) {
	SymTf& s = GetData(attr);
	
	// in forward pass the agent simply behaves in the environment
	// create input to brain
	input_array.SetCount(total + 1);
	int pos = 0;
	double* prev = src->GetValue<double>(0, max_shift, attr);
	for(int k = 0; k < max_shift; k++) {
		double* open = src->GetValue<double>(0, max_shift-1-k, attr);
		if (prev) {
			double d = *open / *prev - 1.0;
			input_array[pos++] = d;
		} else {
			input_array[pos++] = 0;
		}
		prev = open;
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
}

void DQNAgent::Backward(const SlotProcessAttributes& attr) {
	SymTf& s = GetData(attr);
	
	double* open = src->GetValue<double>(0, 0, attr);
	double* prev_open = src->GetValue<double>(0, 1, attr);
	double change = prev_open ? *open / *prev_open - 1.0: 0;
	double mul = (double)s.velocity / (double)max_velocity * 10.0;
	change *= mul;
	
	// in backward pass agent learns.
	// compute reward
	s.reward = change;
	
	// pass to brain for learning
	s.agent.Learn(s.reward);
	
	// Write reward to oscillator
	char* sig = GetValue<char>(0, attr);
	*sig = s.velocity;
	double* out = GetValue<double>(1, attr);
	*out = s.reward;
}























MonaAgent::MonaAgent() {
	AddValue<char>("Signal");
	AddValue<double>("Reward");
	
	// Cheese need and goal.
	CHEESE_NEED = 1.0;
	CHEESE_GOAL = 0.5;
	
	SetStyle(
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
		"}"
	);
}

void MonaAgent::SetArguments(const VectorMap<String, Value>& args) {
	
}
	
void MonaAgent::Init() {
	TimeVector& tv = GetTimeVector();
	
	src = tv.FindLinkSlot("/open");
	//rnn = tv.FindLinkSlot("/rnn");
	ASSERTEXC(src);
	//ASSERTEXC(rnn);
	
	tf_count = tv.GetPeriodCount();
	max_shift = 10;
	sym_count = tv.GetSymbolCount();
	total = max_shift;
	
	data.SetCount(sym_count * tf_count);
	
	// Goal vector 0 .... 3, 0.01
	input_array.SetCount(0);
	input_array.SetCount(max_shift + 2, 0.0);
	input_array[max_shift+0] = CLOSE;
	input_array[max_shift+1] = 0.01;
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		
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
	
	do_training = true;
}

bool MonaAgent::Process(const SlotProcessAttributes& attr) {
	Panic("TODO: add spread costs");
	
	// Check if position is useless for training
	double* open = src->GetValue<double>(0, 0, attr);
	double* prev = src->GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	SymTf& s = GetData(attr);
	
	// in backward pass agent learns.
	// compute reward
	if (s.action != IDLE) {
		ASSERT(s.action == LONG || s.action == SHORT);
		double* open = src->GetValue<double>(0, 0, attr);
		double change = *open / s.prev_open - 1.0;
		s.reward = s.action == SHORT ? change * -1.0 : change;
		s.reward -= 0.0001; // add some constant expenses
	} else {
		s.reward = 0.0;
	}
	
	
	// in forward pass the agent simply behaves in the environment
	// create input to brain
	input_array.SetCount(total + 2);
	int pos = 0;
	prev = src->GetValue<double>(0, max_shift, attr);
	for(int k = 0; k < max_shift; k++) {
		double* open = src->GetValue<double>(0, max_shift-1-k, attr);
		if (prev) {
			double d = *open / *prev - 1.0;
			input_array[pos++] = d;
		} else {
			input_array[pos++] = 0;
		}
		prev = open;
	}
	ASSERT(pos == total);
	input_array[pos++] = s.action;
	input_array[pos++] = s.reward;
	
	
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
		
		double* open = src->GetValue<double>(0, 0, attr);
		s.prev_open = *open;
	}
	
	
	// Write signal
	char* sig = GetValue<char>(0, attr);
	*sig = s.action == LONG ? 1 : s.action == SHORT ? -1 : 0;
	
	
	return do_training;
}

























MonaMetaAgent::MonaMetaAgent() {
	AddValue<char>("Signal");
	AddValue<double>("Reward");
	
	// Cheese need and goal.
	CHEESE_NEED = 1.0;
	CHEESE_GOAL = 0.5;
	
	SetStyle(
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
		"}"
	);
}

void MonaMetaAgent::SetArguments(const VectorMap<String, Value>& args) {
	
}
	
void MonaMetaAgent::Init() {
	TimeVector& tv = GetTimeVector();
	
	src = tv.FindLinkSlot("/open");
	ASSERTEXC(src);
	
	rl = FindLinkSlot("/rl");
	ASSERTEXC(rl);
	dqn = FindLinkSlot("/dqn");
	ASSERTEXC(dqn);
	mona = FindLinkSlot("/mona");
	ASSERTEXC(mona);
	
	
	tf_count = tv.GetPeriodCount();
	sym_count = tv.GetSymbolCount();
	total = 3 * tf_count + 2;
	
	data.SetCount(sym_count * tf_count);
	
	// Goal vector 0 .... 3, 0.01
	input_array.SetCount(0);
	input_array.SetCount(total, 0.0);
	input_array[total-2] = CLOSE;
	input_array[total-1] = 0.01;
	
	for(int i = 0; i < data.GetCount(); i++) {
		SymTf& s = data[i];
		
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
	
	do_training = true;
}

bool MonaMetaAgent::Process(const SlotProcessAttributes& attr) {
	Panic("TODO: add spread costs");
	
	// Check if position is useless for training
	double* open = src->GetValue<double>(0, 0, attr);
	double* prev = src->GetValue<double>(0, 1, attr);
	if (!prev || *prev == *open)
		return true;
	
	//LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	SymTf& s = GetData(attr);
	
	// in backward pass agent learns.
	// compute reward
	if (s.action != IDLE) {
		ASSERT(s.action == LONG || s.action == SHORT);
		double* open = src->GetValue<double>(0, 0, attr);
		double change = *open / s.prev_open - 1.0;
		s.reward = s.action == SHORT ? change * -1.0 : change;
		s.reward -= 0.0001; // add some constant expenses. TODO: real spread
	} else {
		s.reward = 0.0;
	}
	
	
	// in forward pass the agent simply behaves in the environment
	// create input to brain
	input_array.SetCount(total);
	int pos = 0;
	for(int i = 0; i < 3; i++) {
		SlotPtr src = i == 0 ? rl : i == 1 ? dqn : mona;
		for(int j = 0; j < tf_count; j++) {
			char* sig = src->GetValue<char>(0, j, 0, attr);
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
		double* open = src->GetValue<double>(1, attr);
		s.prev_open = *open;
	}
	
	
	// Write signal
	char* sig = GetValue<char>(0, attr);
	*sig = s.action == LONG ? 1 : s.action == SHORT ? -1 : 0;
	
	
	return do_training;
}

}
