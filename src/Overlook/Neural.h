#ifndef _Overlook_Neural_h_
#define _Overlook_Neural_h_

/*
	Catalog of possible additions: http://www.asimovinstitute.org/neural-network-zoo/
	
*/


namespace Overlook {
using namespace Narx;


class AutoEncoder : public Core {
	ConvNet::Session ses;
	
public:
	AutoEncoder();
	
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddOut(ForecastPhase, ForecastChangeValue, SymTf);
	}
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
};


class Recurrent : public Core {
	Vector<int> sequence;
	ConvNet::RecurrentSession ses;
	
	Vector<double> temperatures;
	OnlineVariance var;
	String model_str;
	String type;
	int batch;
	int shifts;
	int value_count;
	int iter;
	int tick_time;
	int sym_count, tf_count;
	bool is_training_loop;
	
	int ToChar(double diff, double min, double max);
	double FromChar(int i, double min, double max);
	
	void StoreThis();
	void LoadThis();
	void InitSessions();
public:
	Recurrent();
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddIn(IndiPhase, ForecastChannelValue, SymTf);
		reg.AddOut(ForecastPhase, ForecastChangeValue, SymTf);
	}
	virtual int GetType() const {return SLOT_SYMTF;}
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual bool ProcessRelease(const CoreProcessAttributes& attr);
	
	
	void Reset();
	void Reload();
	
	int GetIter() const {return iter;}
	int GetTickTime() const {return tick_time;}
	int GetSampleTemperature(int i) const {return temperatures[i];}
	double GetPerplexity() const;
	String GetModel() const {return model_str;}
	
	void SetPreset(int i);
	void SetLearningRate(double rate);
	void SetSampleTemperature(int i, double t) {temperatures[i] = t;}
	void SetModel(String model_str) {this->model_str = model_str;}
	void Pause();
	void Resume();
	void Refresher();
	void PredictSentence(const CoreProcessAttributes& attr, bool samplei=false, double temperature=1.0);
	void Tick(const CoreProcessAttributes& attr);
	
	void Serialize(Stream& s) {
		s % temperatures % var % model_str % batch % shifts % value_count % iter % tick_time % is_training_loop;
		if (s.IsLoading()) {
			InitSessions();
			ValueMap map;
			s % map;
			ses.Load(map);
		}
		else if(s.IsStoring()) {
			ValueMap map;
			ses.Store(map);
			s % map;
		}
	}
};





class NARX : public Core {
	
	Vector<double> Y, pY;
	int value_count, input_count;
	
	Vector<Unit> hunits;
	Vector<OutputUnit> output_units;
	Vector<Vector<InputUnit> > inputs;
	Vector<Vector<InputUnit> > feedbacks;
	Vector<Vector<InputUnit> > exogenous;
	Vector<EvaluationEngine> ee;
	Vector<EvaluationEngine> rw;
		
	
	Vector<double> out;
	int H;
	int a, a1;
	int b;
	int feedback, targets;
	int hact;
	int epoch;
	
	int tf_count;
	int sym_count;
	
	void FillExogenous(const Core& src, const Core& change, const CoreProcessAttributes& attr);
	
public:
	NARX();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual bool ProcessRelease(const CoreProcessAttributes& attr);
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddIn(IndiPhase, RealVolumeValue, SymTf);
		reg.AddIn(IndiPhase, TimeOscillatorValue, SymTf);
		reg.AddOut(ForecastPhase, ForecastChangeValue, SymTf);
	}
	virtual int GetType() const {return SLOT_TF;}
	
	virtual void Serialize(Stream& s) {
		s % Y % pY % value_count % input_count %
			hunits % output_units % inputs % feedbacks % exogenous % ee % rw;
		s % H % a % a1 % b % feedback % targets % hact % epoch;
	}
};


// Forecaster is just a regression neural network for multiple inputs.
// It is like a complex version of ConvNet Regression1D example.
class Forecaster : public Core {
	String t;
	ConvNet::Session ses;
	
	void Serialize(Stream& s) {s % ses;}
	
	ConvNet::Volume input, output;
	int sym_count, tf_count;
	int single, pair;
	bool do_training;
	
	void Forward(const CoreProcessAttributes& attr);
	void Backward(const CoreProcessAttributes& attr);
public:
	Forecaster();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddIn(ForecastPhase, ForecastChangeValue, Sym);
		reg.AddIn(IndiPhase, ForecastChannelValue, Sym);
		reg.AddInOptional(ForecastPhase, ForecastChangeValue, Sym);
		reg.AddInOptional(ForecastPhase, ForecastChangeValue, Sym);
		reg.AddInOptional(ForecastPhase, ForecastChangeValue, Sym);
		reg.AddInOptional(ForecastPhase, ForecastChangeValue, Sym);
		reg.AddInOptional(ForecastPhase, ForecastChangeValue, Sym);
		reg.AddInOptional(IndiPhase, IndicatorValue, Sym);
		reg.AddInOptional(IndiPhase, IndicatorValue, Sym);
		reg.AddInOptional(IndiPhase, IndicatorValue, Sym);
		reg.AddInOptional(IndiPhase, IndicatorValue, Sym);
		reg.AddInOptional(IndiPhase, IndicatorValue, Sym);
		reg.AddOut(ForecastCombPhase, ForecastChangeValue, SymTf);
	}
	virtual int GetType() const {return SLOT_SYMTF;}
	
};


class ClassifierAgent : public Core {
	ConvNet::Session ses;
	bool ideal;
	
public:
	ClassifierAgent();
	
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddIn(ForecastCombPhase, ForecastChangeValue, Sym);
		reg.AddIn(IndiPhase, IdealOrderSignal, Sym);
		reg.AddOut(AgentPhase, ForecastOrderSignal, SymTf);
	}
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
};


// RLAgent and DQNAgent are almost identical.
class RLAgent : public Core {
	
	enum {ACT_IDLE, ACT_LONG, ACT_SHORT};
	
	ConvNet::Brain brain;
	SimBroker broker;
	int action, prev_action;
	double reward;
	void Serialize(Stream& s) {s % brain %action % prev_action % reward;}
	
	Vector<double> input_array;
	int sym_count, tf_count;
	int max_shift, total;
	bool do_training;
	bool ideal;
	
	void Forward(const CoreProcessAttributes& attr);
	void Backward(const CoreProcessAttributes& attr);
public:
	RLAgent();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddIn(ForecastCombPhase, ForecastChangeValue, Sym);
		reg.AddOut(AgentPhase, ForecastOrderSignal, SymTf);
	}
	virtual int GetType() const {return SLOT_SYMTF;}
	
};



class DQNAgent : public Core {
	
	ConvNet::DQNAgent agent;
	SimBroker broker;
	double reward;
	int action, prev_action, velocity;
	int max_velocity;
	
	void Serialize(Stream& s) {s % agent % action % prev_action % velocity % reward;}
	
	Vector<double> input_array;
	int sym_count, tf_count;
	int max_shift, total;
	bool do_training;
	bool ideal;
	
	void Forward(const CoreProcessAttributes& attr);
	void Backward(const CoreProcessAttributes& attr);
public:
	DQNAgent();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddIn(ForecastCombPhase, ForecastChangeValue, Sym);
		reg.AddOut(AgentPhase, ForecastOrderSignal, SymTf);
	}
	virtual int GetType() const {return SLOT_SYMTF;}
	
};



class MonaAgent : public Core {
	
	enum {IDLE, LONG, SHORT, CLOSE};
	
	Mona mona;
	SimBroker broker;
	int action, prev_action;
	double reward, prev_open;
	
	void Serialize(Stream& s) {s % mona % action % prev_action % reward % prev_open;}
	
	Vector<double> input_array;
	double CHEESE_NEED, CHEESE_GOAL;
	int sym_count, tf_count;
	int max_shift, total;
	bool do_training;
	bool ideal;
	
public:
	MonaAgent();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddIn(ForecastCombPhase, ForecastChangeValue, Sym);
		reg.AddOut(AgentPhase, ForecastOrderSignal, SymTf);
	}
	virtual int GetType() const {return SLOT_SYMTF;}
	
};


class MonaMetaAgent : public Core {
	
	enum {IDLE, LONG, SHORT, CLOSE};
	
	Mona mona;
	SimBroker broker;
	int action, prev_action;
	double reward, prev_open;
	void Serialize(Stream& s) {s % mona % action % prev_action % reward % prev_open;}
	
	Vector<double> input_array;
	double CHEESE_NEED, CHEESE_GOAL;
	int sym_count, tf_count;
	int total;
	bool do_training;
	bool ideal;
	
public:
	MonaMetaAgent();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, Sym);
		reg.AddIn(AgentPhase, ForecastOrderSignal, Sym);
		reg.AddInOptional(AgentPhase, ForecastOrderSignal, Sym);
		reg.AddOut(AgentCombPhase, ForecastOrderSignal, SymTf);
	}
	virtual int GetType() const {return SLOT_SYMTF;}
	
};

class MonaDoubleAgent : public Core {
	
	Mona agent;
	SimBroker broker;
	Vector<int> velocity;
	double CHEESE_NEED, CHEESE_GOAL;
	int action, prev_action;
	int actions_total;
	int max_velocity;
	
	Vector<double> input_array;
	int sym_count, tf_count;
	int total;
	bool do_training;
	
	void Forward(const CoreProcessAttributes& attr);
public:
	MonaDoubleAgent();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, All);
		reg.AddIn(AgentCombPhase, ForecastOrderSignal, All);
		reg.AddOut(AgentCombPhase, ForecastOrderSignal, All);
	}
	virtual int GetType() const {return SLOT_ONCE;}
	
	virtual void Serialize(Stream& s) {
		s % agent;
	}
};

}

#endif
