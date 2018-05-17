#include "Overlook.h"


namespace ConvNet {

// return Mat but filled with random numbers from gaussian
void RandMat(int n, int d, double mu, double std, Mat& m) {
	m.Init(d, n, 0);
	
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(mu, std);
	generator.seed(Random(INT_MAX));
	
	for (int i = 0; i < m.GetLength(); i++)
		m.Set(i, distribution(generator));
}

int SampleWeighted(Vector<double>& p) {
	ASSERT(!p.IsEmpty());
	double r = Randomf();
	double c = 0.0;
	for (int i = 0; i < p.GetCount(); i++) {
		c += p[i];
		if (c >= r)
			return i;
	}
	Panic("Invalid input vector");
	return 0;
}

void UpdateMat(Mat& m, double alpha) {
	// updates in place
	for (int i = 0; i < m.GetLength(); i++) {
		double d = m.GetGradient(i);
		if (d != 0.0) {
			m.Add(i, -alpha * d);
			m.SetGradient(i, 0);
		}
	}
}




RecurrentSession::RecurrentSession() {
	mode = MODE_RNN;
	learning_rate = 0.01;
	clipval = 5.0;
	regc = 0.000001;
	input_size = -1;
	output_size = -1;
	letter_size = -1;
	max_graphs = 100;
	initial_bias = -4;
	
	// Solver
	decay_rate = 0.999;
	smooth_eps = 1e-8;
	
	index_sequence.SetCount(max_graphs, 0);
	graphs.SetCount(max_graphs);
	hidden_prevs.SetCount(max_graphs+1);
	cell_prevs.SetCount(max_graphs+1);
}

RecurrentSession::~RecurrentSession() {
	
}

int RecurrentSession::GetMatCount() {
	int count = 0;
	if (mode == MODE_RNN)
		count = rnn_model.GetCount() * RNNModel::GetCount();
	else if (mode == MODE_LSTM)
		count = lstm_model.GetCount() * LSTMModel::GetCount();
	else if (mode == MODE_HIGHWAY)
		count = hw_model.GetCount() * HighwayModel::GetCount() + 2;
	else
		Panic("Invalid mode");
	count += 3;
	return count;
}

Mat& RecurrentSession::GetMat(int i) {
	int count = 0, cols = 0, rows = 0;
	if (mode == MODE_RNN) {
		rows = rnn_model.GetCount();
		cols = RNNModel::GetCount();
	}
	else if (mode == MODE_LSTM) {
		rows = lstm_model.GetCount();
		cols = LSTMModel::GetCount();
	}
	else if (mode == MODE_HIGHWAY) {
		rows = hw_model.GetCount();
		cols = HighwayModel::GetCount();
	}
	else Panic("Invalid mode");
	
	int row = i / cols;
	if (row >= rows) {
		i = i - rows * cols;
		switch (i) {
			case 0: return Whd;
			case 1: return bd;
			case 2: return Wil;
			case 3: return noise_i[0];
			case 4: return noise_i[1];
			default: Panic("Invalid id " + IntStr(i));
		}
	} else {
		int col = i % cols;
		if (mode == MODE_RNN) {
			return rnn_model[row].GetMat(col);
		}
		else if (mode == MODE_LSTM) {
			return lstm_model[row].GetMat(col);
		}
		else if (mode == MODE_HIGHWAY) {
			return hw_model[row].GetMat(col);
		}
		else Panic("Invalid mode");
	}
	
	return Wil;
}

void RecurrentSession::Init() {
	ASSERT_(input_size != -1, "Input size must be set");
	ASSERT_(output_size != -1, "Output size must be set");
	
	if (mode == MODE_HIGHWAY)
		RandMat(input_size, hidden_sizes[0], 0, 0.08, Wil);
	else
		RandMat(input_size, letter_size, 0, 0.08, Wil);
	     
	
	if (mode == MODE_RNN) {
		InitRNN();
	}
	else if (mode == MODE_LSTM) {
		InitLSTM();
	}
	else if (mode == MODE_HIGHWAY) {
		InitHighway();
	}
	else Panic("Invalid RecurrentSession mode");
	
	InitGraphs();
}

void RecurrentSession::InitGraphs() {
	ASSERT(mode == MODE_RNN || mode == MODE_LSTM || mode == MODE_HIGHWAY);
	
	step_cache.Clear();
	int hidden_count = hidden_sizes.GetCount();
	ASSERT_(hidden_count > 0, "Hidden sizes must be set");
	
	for (int i = 0; i < hidden_prevs.GetCount(); i++) {
		Vector<Mat*>& hidden_prevs = this->hidden_prevs[i];
		Vector<Mat*>& cell_prevs = this->cell_prevs[i];
		hidden_prevs.SetCount(hidden_count, NULL);
		cell_prevs.SetCount(hidden_count, NULL);
	}
	
	
	first_hidden.SetCount(hidden_count);
	first_cell.SetCount(hidden_count);
	for(int i = 0; i < hidden_count; i++) {
		hidden_prevs[0][i]	= &first_hidden[i];
		cell_prevs[0][i]	= &first_cell[i];
		first_hidden[i]	.Init(1, hidden_sizes[i], 0);
		first_cell[i]	.Init(1, hidden_sizes[i], 0);
	}
	
	
	for (int i = 0; i < graphs.GetCount(); i++) {
		Array<GraphTree>& hidden_graphs = graphs[i];
		hidden_graphs.SetCount(hidden_count);
		for (int j = 0; j < hidden_count; j++) {
			if (mode == MODE_RNN)
				InitRNN(i, j, hidden_graphs[j]);
			else if (mode == MODE_LSTM)
				InitLSTM(i, j, hidden_graphs[j]);
			else
				InitHighway(i, j, hidden_graphs[j]);
		}
	}
}

void RecurrentSession::InitRNN() {
	int hidden_size = 0;
	
	// loop over depths
	rnn_model.SetCount(hidden_sizes.GetCount());
	for (int d = 0; d < hidden_sizes.GetCount(); d++) {
		int prev_size = d == 0 ? letter_size : hidden_sizes[d - 1];
		hidden_size = hidden_sizes[d];
		RNNModel& m = rnn_model[d];
		RandMat(hidden_size, prev_size,		0, 0.08,	m.Wxh);
		RandMat(hidden_size, hidden_size,	0, 0.08,	m.Whh);
		m.bhh.Init(1, hidden_size, 0);
	}
	
	// decoder params
	RandMat(output_size, hidden_size, 0, 0.08,	Whd);
	bd.Init(1, output_size, 0);
}

void RecurrentSession::InitRNN(int i, int j, GraphTree& g) {
	RNNModel& m = rnn_model[j];
	
	g.Clear();
	
	Vector<Mat*>& hidden_prevs = this->hidden_prevs[i];
	Vector<Mat*>& hidden_nexts = this->hidden_prevs[i+1];
	
	if (j == 0) {
		input = &g.RowPluck(&index_sequence[i], Wil);
	}
	
	Mat& input_vector = j == 0 ? *input : *hidden_nexts[j-1];
	Mat& hidden_prev = *hidden_prevs[j];
	
	Mat& h0 = g.Mul(m.Wxh, input_vector);
	Mat& h1 = g.Mul(m.Whh, hidden_prev);
	Mat& hidden_d = g.Relu(g.Add(g.Add(h0, h1), m.bhh));
	
	hidden_nexts[j] = &hidden_d;
	
	// one decoder to outputs at end
	if (j == hidden_prevs.GetCount() - 1) {
		g.Add(g.Mul(Whd, hidden_d), bd);
	}
}

// hidden size should be a list
void RecurrentSession::InitLSTM() {
	int hidden_size = 0;
	
	// loop over depths
	lstm_model.SetCount(hidden_sizes.GetCount());
	for (int d = 0; d < hidden_sizes.GetCount(); d++) {
		// loop over depths
		LSTMModel& m = lstm_model[d];
		
		int prev_size = d == 0 ? letter_size : hidden_sizes[d - 1];
		hidden_size = hidden_sizes[d];
		
		// gates parameters
		RandMat(hidden_size, prev_size,		0, 0.08,	m.Wix);
		RandMat(hidden_size, hidden_size,	0, 0.08,	m.Wih);
		m.bi	.Init(1, hidden_size, 0);
		RandMat(hidden_size, prev_size,		0, 0.08,	m.Wfx);
		RandMat(hidden_size, hidden_size,	0, 0.08,	m.Wfh);
		m.bf	.Init(1, hidden_size, 0);
		RandMat(hidden_size, prev_size,		0, 0.08,	m.Wox);
		RandMat(hidden_size, hidden_size,	0, 0.08,	m.Woh);
		m.bo	.Init(1, hidden_size, 0);
		
		// cell write params
		RandMat(hidden_size, prev_size,		0, 0.08,	m.Wcx);
		RandMat(hidden_size, hidden_size,	0, 0.08,	m.Wch);
		m.bc	.Init(1, hidden_size, 0);
	}
	
	// decoder params
	RandMat(output_size, hidden_size,	0, 0.08,	Whd);
	bd.Init(1, output_size, 0);
}

void RecurrentSession::InitLSTM(int i, int j, GraphTree& g) {
	LSTMModel& m = lstm_model[j];
	
	g.Clear();
	
	Vector<Mat*>& hidden_prevs = this->hidden_prevs[i];
	Vector<Mat*>& hidden_nexts = this->hidden_prevs[i+1];
	Vector<Mat*>& cell_prevs = this->cell_prevs[i];
	Vector<Mat*>& cell_nexts = this->cell_prevs[i+1];
	
	if (j == 0) {
		input = &g.RowPluck(&index_sequence[i], Wil);
	}
	
	Mat& input_vector = j == 0 ? *input : *hidden_nexts[j-1];
	Mat& hidden_prev = *hidden_prevs[j];
	Mat& cell_prev = *cell_prevs[j];
	
	// input gate
	Mat& h0 = g.Mul(m.Wix, input_vector);
	Mat& h1 = g.Mul(m.Wih, hidden_prev);
	Mat& input_gate = g.Sigmoid(g.Add(g.Add(h0, h1), m.bi));
	
	// forget gate
	Mat& h2 = g.Mul(m.Wfx, input_vector);
	Mat& h3 = g.Mul(m.Wfh, hidden_prev);
	Mat& forget_gate = g.Sigmoid(g.Add(g.Add(h2, h3), m.bf));
	
	// output gate
	Mat& h4 = g.Mul(m.Wox, input_vector);
	Mat& h5 = g.Mul(m.Woh, hidden_prev);
	Mat& output_gate = g.Sigmoid(g.Add(g.Add(h4, h5), m.bo));
	
	// write operation on cells
	Mat& h6 = g.Mul(m.Wcx, input_vector);
	Mat& h7 = g.Mul(m.Wch, hidden_prev);
	Mat& cell_write = g.Tanh(g.Add(g.Add(h6, h7), m.bc));
	
	// compute new cell activation
	Mat& retain_cell = g.EltMul(forget_gate, cell_prev); // what do we keep from cell
	Mat& write_cell = g.EltMul(input_gate, cell_write); // what do we write to cell
	Mat& cell_d = g.Add(retain_cell, write_cell); // new cell contents
	
	// compute hidden state as gated, saturated cell activations
	Mat& hidden_d = g.EltMul(output_gate, g.Tanh(cell_d));
	
	hidden_nexts[j] = &hidden_d;
	cell_nexts[j] = &cell_d;
	
	
	// one decoder to outputs at end
	if (j == hidden_prevs.GetCount() - 1) {
		g.Add(g.Mul(Whd, hidden_d), bd);
	}
}


/*
	Recurrent Highway Networks (Zilly and Srivastava et al., 2016)
		- References:
		- Zilly, J, Srivastava, R, Koutnik, J, Schmidhuber, J., "Recurrent Highway Networks", 2016
		- Gal, Y, "A Theoretically Grounded Application of Dropout in Recurrent Neural Networks", 2015.
		- Zaremba, W, Sutskever, I, Vinyals, O, "Recurrent neural network regularization", 2014.
	
	Also helpful: https://github.com/julian121266/RecurrentHighwayNetworks/blob/master/torch_rhn_ptb.lua
*/

void RecurrentSession::InitHighway() {
	ASSERT(input_size == output_size);
	int hidden_size = 0;
	
	// loop over depths
	hw_model.SetCount(hidden_sizes.GetCount());
	for (int d = 0; d < hidden_sizes.GetCount(); d++) {
		// loop over depths
		HighwayModel& m = hw_model[d];
		
		hidden_size = hidden_sizes[d];
		
		if (d == 0) {
			RandMat(hidden_size, 1,			0, 0.08,	noise_i[0]);
			RandMat(hidden_size, 1,			0, 0.08,	noise_i[1]);
		}
		RandMat(hidden_size, 1,				0, 0.08,	m.noise_h[0]);
		RandMat(hidden_size, 1,				0, 0.08,	m.noise_h[1]);
	}
	
	// decoder params
	RandMat(output_size, hidden_size,	0, 0.08,	Whd);
	bd.Init(1, output_size, 0);
}

void RecurrentSession::InitHighway(int i, int j, GraphTree& g) {
	HighwayModel& m = hw_model[j];
	
	g.Clear();
	
	Vector<Mat*>& hidden_prevs = this->hidden_prevs[i];
	Vector<Mat*>& hidden_nexts = this->hidden_prevs[i+1];
	Vector<Mat*>& cell_prevs = this->cell_prevs[i];
	Vector<Mat*>& cell_nexts = this->cell_prevs[i+1];
	
	if (j == 0) {
		input = &g.RowPluck(&index_sequence[i], Wil);
	}
	
	Mat& input_vector = j == 0 ? *input : *hidden_nexts[j-1];
	Mat& hidden_prev = *hidden_prevs[j];
	
	if (j == 0) {
		Mat& dropped_x0			= g.EltMul(input_vector, noise_i[0]);
		Mat& dropped_h_tab0		= g.EltMul(hidden_prev, m.noise_h[0]);
		
		Mat& dropped_x1			= g.EltMul(input_vector, noise_i[1]);
		Mat& dropped_h_tab1		= g.EltMul(hidden_prev, m.noise_h[1]);
		
		Mat& t_gate_tab			= g.Sigmoid(g.AddConstant(initial_bias, g.Add(dropped_x0, dropped_h_tab0)));
		Mat& in_transform_tab	= g.Tanh(g.Add(dropped_x1, dropped_h_tab1));
		Mat& c_gate_tab			= g.AddConstant(1, g.MulConstant(-1, t_gate_tab));
		Mat& hidden_d			= g.Add(
										g.EltMul(hidden_prev, c_gate_tab),
										g.EltMul(in_transform_tab, t_gate_tab));
		
		hidden_nexts[j] = &hidden_d;
	}
	else
	{
		Mat& dropped_h_tab0		= g.EltMul(input_vector, m.noise_h[0]);
		Mat& dropped_h_tab1		= g.EltMul(input_vector, m.noise_h[1]);
		
		Mat& t_gate_tab			= g.Sigmoid(g.AddConstant(initial_bias, dropped_h_tab0));
		Mat& in_transform_tab	= g.Tanh(dropped_h_tab1);
		Mat& c_gate_tab			= g.AddConstant(1, g.MulConstant(-1, t_gate_tab));
		Mat& hidden_d			= g.Add(
										g.EltMul(input_vector, c_gate_tab),
										g.EltMul(in_transform_tab, t_gate_tab));
		
		hidden_nexts[j] = &hidden_d;
	}
	
	
	// one decoder to outputs at end
	if (j == hidden_prevs.GetCount() - 1) {
		g.Add(g.Mul(Whd, *hidden_nexts.Top()), bd);
	}
}
	
void RecurrentSession::Learn(const Vector<int>& input_sequence) {
	double log2ppl = 0.0;
	double cost = 0.0;
	
	ASSERT(input_sequence.GetCount() < graphs.GetCount());
	
	// Copy input sequence. Fixed index_sequence addresses are used in RowPluck.
	int n = input_sequence.GetCount();
	
	// start and end tokens are zeros
	index_sequence[0] = 0; // first step: start with START token
	for(int i = 0; i < n; i++)
		index_sequence[i+1] = input_sequence[i]; // this value is used in the RowPluck
	for(int i = n+1; i < index_sequence.GetCount(); i++)
		index_sequence[i] = -1; // for debugging
	
	ResetPrevs();
	
	for(int i = 0; i <= n; i++) {
		
		int ix_target = i == n ? 0 : index_sequence[i+1]; // last step: end with END token
		
		Array<GraphTree>& list = graphs[i];
		for(int j = 0; j < list.GetCount(); j++) {
			list[j].Forward();
		}
		
		Mat& logprobs = list.Top().Top().output;
		Softmax(logprobs, probs); // compute the softmax probabilities
		
		double p = probs.Get(ix_target);
		log2ppl += -log2(p); // accumulate base 2 log prob and do smoothing
		cost += -log(p);
		
		// write gradients into log probabilities
		int count = logprobs.GetLength();
		for(int j = 0; j < count; j++)
			logprobs.SetGradient(j, probs.Get(j));
		logprobs.AddGradient(ix_target, -1.0);
	}
	
	ppl = pow(2, log2ppl / (n - 1));
	cost = cost;
	
	Backward(n);
	
	SolverStep();
}

void RecurrentSession::Backward(int seq_end_cursor) {
	for (int i = seq_end_cursor; i >= 0; i--) {
		Array<GraphTree>& list = graphs[i];
		for (int j = list.GetCount()-1; j >= 0; j--) {
			list[j].Backward();
		}
	}
}

void RecurrentSession::SolverStep() {
	// perform parameter update
	int num_clipped = 0;
	int num_tot = 0;
	int n = GetMatCount();
	
	int step_cache_count = step_cache.GetCount();
	step_cache.SetCount(n);
	
	for (int k = 0; k < n; k++) {
		Mat& m = GetMat(k);
		Mat& s = step_cache[k];
		
		if (k >= step_cache_count) {
			s.Init(m.GetWidth(), m.GetHeight(), 0);
		}
		
		for (int i = 0; i < m.GetLength(); i++) {
			// rmsprop adaptive learning rate
			double mdwi = m.GetGradient(i);
			s.Set(i, s.Get(i) * decay_rate + (1.0 - decay_rate) * mdwi * mdwi);
			
			// gradient clip
			if (mdwi > +clipval) {
				mdwi = +clipval;
				num_clipped++;
			}
			else if (mdwi < -clipval) {
				mdwi = -clipval;
				num_clipped++;
			}
			
			num_tot++;
			
			// update (and regularize)
			m.Add(i, - learning_rate * mdwi / sqrt(s.Get(i) + smooth_eps) - regc * m.Get(i));
			m.SetGradient(i, 0); // reset gradients for next iteration
		}
	}
	ratio_clipped = num_clipped * 1.0 / num_tot;
}

void RecurrentSession::ResetPrevs() {
	int hidden_count = hidden_sizes.GetCount();
	
	first_hidden.SetCount(hidden_count);
	for (int d = 0; d < hidden_count; d++) {
		first_hidden[d].Init(1, hidden_sizes[d], 0);
	}
	
	first_cell.SetCount(hidden_count);
	for (int d = 0; d < hidden_count; d++) {
		first_cell[d].Init(1, hidden_sizes[d], 0);
	}
}

void RecurrentSession::Predict(Vector<int>& output_sequence, bool samplei, double temperature, bool continue_sentence, int max_predictions) {
	int begin_write = 0;
	if (continue_sentence) {
		begin_write = output_sequence.GetCount();
	}
	else {
		output_sequence.SetCount(0);
	}
	
	index_sequence[0] = 0;
	for(int i = 1; i < index_sequence.GetCount(); i++)
		index_sequence[i] = -1; // for debugging
	
	ResetPrevs();
	int predictions = 0;
	
	for (int i = 0; ; i++) {
		
		Array<GraphTree>& list = graphs[i];
		for(int j = 0; j < list.GetCount(); j++) {
			GraphTree& g = list[j];
			g.Forward();
		}
		
		// Use given beginning if set
		if (continue_sentence && i < begin_write) {
			
			// Set index to variable what was given
			int ix = output_sequence[i];
			index_sequence[i+1] = ix;
		}
		
		// By default, predict from START token and previous input value
		else {
			// sample predicted letter
			Mat& logprobs = list.Top().Top().output;
			
			if (temperature != 1.0 && samplei) {
				// scale log probabilities by temperature and renormalize
				// if temperature is high, logprobs will go towards zero
				// and the softmax outputs will be more diffuse. if temperature is
				// very low, the softmax outputs will be more peaky
				for (int q = 0; q < logprobs.GetLength(); q++) {
					logprobs.Set(q, logprobs.Get(q) / temperature);
				}
			}
			
			Softmax(logprobs, probs);
			
			int ix = 0;
			if (samplei) {
				ix = probs.GetSampledColumn();
			} else {
				ix = probs.GetMaxColumn();
			}
			
			if (ix == 0) break; // END token predicted, break out
			if (i+1 >= max_graphs) break; // something is wrong
			
			output_sequence.Add(ix);
			predictions++;
			if (predictions == max_predictions) break;
			
			// Set index to variable what RowPluck reads
			index_sequence[i+1] = ix;
		}
	}
}

void RecurrentSession::Load(const ValueMap& js) {
	#define LOAD(x) if (js.Find(#x) != -1) {x = js.GetValue(js.Find(#x));}
	
	
	String generator;
	LOAD(generator);
	mode = generator == "lstm" ? MODE_LSTM : generator == "rnn" ? MODE_RNN : MODE_HIGHWAY;
	
	if (js.Find("hidden_sizes") != -1) {
		hidden_sizes.Clear();
		ValueMap hs = js.GetValue(js.Find("hidden_sizes"));
		for(int i = 0; i < hs.GetCount(); i++)
			hidden_sizes.Add(hs[i]);
	}
	
	LOAD(letter_size);
	LOAD(regc);
	LOAD(learning_rate);
	LOAD(clipval);
	
	if (js.Find("model") != -1) {
		ValueMap model = js.GetValue(js.Find("model"));
		
		#define LOADVOL(x) {ValueMap map = model.GetValue(model.Find(#x)); this->x.Load(map);}
		LOADVOL(Wil);
		LOADVOL(Whd);
		LOADVOL(bd);
		#undef LOADVOL
		
		if      (mode == MODE_LSTM)		{lstm_model.SetCount(hidden_sizes.GetCount());}
		else if (mode == MODE_RNN)		{rnn_model.SetCount(hidden_sizes.GetCount());}
		else if (mode == MODE_HIGHWAY)  {hw_model.SetCount(hidden_sizes.GetCount());}
		else Panic("Invalid mode");
		
		for(int i = 0; i < hidden_sizes.GetCount(); i++) {
			if (mode == MODE_LSTM) {
				#define LOADMODVOL(x) {ValueMap map = model.GetValue(model.Find(#x + IntStr(i))); lstm_model[i].x.Load(map);}
				LOADMODVOL(Wix);
				LOADMODVOL(Wih);
				LOADMODVOL(bi);
				LOADMODVOL(Wfx);
				LOADMODVOL(Wfh);
				LOADMODVOL(bf);
				LOADMODVOL(Wox);
				LOADMODVOL(Woh);
				LOADMODVOL(bo);
				LOADMODVOL(Wcx);
				LOADMODVOL(Wch);
				LOADMODVOL(bc);
				#undef LOADMODVOL
			}
			else if (mode == MODE_RNN) {
				#define LOADMODVOL(x) {ValueMap map = model.GetValue(model.Find(#x + IntStr(i))); rnn_model[i].x.Load(map);}
				LOADMODVOL(Wxh);
				LOADMODVOL(Whh);
				LOADMODVOL(bhh);
				#undef LOADMODVOL
			}
			else if (mode == MODE_HIGHWAY) {
				#define LOADMODVOL(x) {ValueMap map = model.GetValue(model.Find(#x + IntStr(i))); hw_model[i].x.Load(map);}
				LOADMODVOL(noise_h[0]);
				LOADMODVOL(noise_h[1]);
				#undef LOADMODVOL
			}
		}
	}
	#undef LOAD
}

void RecurrentSession::Store(ValueMap& js) {
	#define SAVE(x) js.GetAdd(#x) = x;
	
	String generator = mode == MODE_LSTM ? "lstm" : mode == MODE_RNN ? "rnn" : "highway";
	SAVE(generator);
	
	ValueMap hs;
	for(int i = 0; i < hidden_sizes.GetCount(); i++)
		hs.Add(IntStr(i), hidden_sizes[i]);
	
	SAVE(letter_size);
	SAVE(regc);
	SAVE(learning_rate);
	SAVE(clipval);
	
	ValueMap model;
	
	#define SAVEVOL(x) {ValueMap map; this->x.Store(map); model.GetAdd(#x) = map;}
	SAVEVOL(Wil);
	SAVEVOL(Whd);
	SAVEVOL(bd);
	#undef SAVEVOL
	
	for(int i = 0; i < hidden_sizes.GetCount(); i++) {
		if (mode == MODE_LSTM) {
			#define SAVEMODVOL(x) {ValueMap map; lstm_model[i].x.Store(map); model.GetAdd(#x + IntStr(i)) = map;}
			SAVEMODVOL(Wix);
			SAVEMODVOL(Wih);
			SAVEMODVOL(bi);
			SAVEMODVOL(Wfx);
			SAVEMODVOL(Wfh);
			SAVEMODVOL(bf);
			SAVEMODVOL(Wox);
			SAVEMODVOL(Woh);
			SAVEMODVOL(bo);
			SAVEMODVOL(Wcx);
			SAVEMODVOL(Wch);
			SAVEMODVOL(bc);
			#undef SAVEMODVOL
		}
		else if (mode == MODE_RNN) {
			#define SAVEMODVOL(x) {ValueMap map; rnn_model[i].x.Store(map); model.GetAdd(#x + IntStr(i)) = map;}
			SAVEMODVOL(Wxh);
			SAVEMODVOL(Whh);
			SAVEMODVOL(bhh);
			#undef SAVEMODVOL
		}
		else if (mode == MODE_HIGHWAY) {
			#define SAVEMODVOL(x) {ValueMap map; hw_model[i].x.Store(map); model.GetAdd(#x + IntStr(i)) = map;}
			SAVEMODVOL(noise_h[0]);
			SAVEMODVOL(noise_h[1]);
			#undef SAVEMODVOL
		}
	}
	
	js.Add("model", model);
}

void RecurrentSession::Serialize(Stream& s) {
	if (s.IsLoading()) {
		ValueMap map;
		s % map;
		Load(map);
	}
	else if (s.IsStoring()) {
		ValueMap map;
		Store(map);
		s % map;
	}
}

}
