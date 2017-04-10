#include "DataCore.h"

namespace DataCore {

Recurrent::Recurrent() {
	shifts = 4;
	for(int i = 0; i < shifts; i++) {
		
		// Different softmax sample temperatures:
		//   lower setting will generate more likely predictions,
		//   but you'll see more of the same common changes again and again. Higher
		//   setting will generate less frequent changes but you might see more unrealistic errors.
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
		int count = Upp::min(30, attr.GetCounted()-1);
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

}
