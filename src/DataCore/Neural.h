#ifndef _DataCore_Neural_h_
#define _DataCore_Neural_h_

#include <Mona/Mona.h>
#include <NARX/NARX.h>
#include "Slot.h"
#include "SimBroker.h"

namespace DataCore {
using namespace Narx;




class Recurrent : public Slot {
	enum {PHASE_GATHERDATA, PHASE_TRAINING};
	
	Array<Array<ConvNet::RecurrentSession> > ses;
	Vector<double> vocab;
	Vector<double> temperatures;
	Vector<int> sequence;
	OnlineVariance var;
	SlotPtr src;
	String model_str;
	double min, max, step, diff;
	int batch;
	int shifts;
	int phase;
	int value_count;
	int iter;
	int tick_time;
	bool is_training_loop;
	SpinLock lock;
	
	int ToChar(double diff);
	double FromChar(int i);
	
	void StoreThis();
	void LoadThis();
	void InitSessions();
public:
	Recurrent();
	virtual String GetKey() const {return "rnn";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
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
	void InitVocab();
	void PredictSentence(const SlotProcessAttributes& attr, bool samplei=false, double temperature=1.0);
	void Tick(const SlotProcessAttributes& attr);
	
	ConvNet::RecurrentSession& GetSession(int sym, int tf) {return ses[sym][tf];}
	
	
	
	void Serialize(Stream& s) {
		s % vocab % temperatures % var % model_str %
			min % max % step % diff % batch % shifts % phase % value_count % iter %
			tick_time % is_training_loop;
		if (s.IsLoading()) {
			InitSessions();
			for(int i = 0; i < ses.GetCount(); i++) {
				for(int j = 0; j < ses[i].GetCount(); j++) {
					ValueMap map;
					s % map;
					ses[i][j].Load(map);
				}
			}
		} else {
			for(int i = 0; i < ses.GetCount(); i++) {
				for(int j = 0; j < ses[i].GetCount(); j++) {
					ValueMap map;
					ses[i][j].Store(map);
					s % map;
				}
			}
		}
	}
};





class NARX : public Slot {
	
	struct Tf : Moveable<Tf> {
		Vector<double> Y, pY;
		int input_count;
		
		//Vector<Vector<double> > Y;
		Vector<Unit> hunits;
		Vector<OutputUnit> output_units;
		Vector<Vector<InputUnit> > inputs;
		Vector<Vector<InputUnit> > feedbacks;
		Vector<Vector<InputUnit> > exogenous;
		Vector<EvaluationEngine> ee;
		Vector<EvaluationEngine> rw;
	};
	Tf& GetData(const SlotProcessAttributes& attr) {return data[attr.tf_id];}
	
	Vector<double> out;
	Vector<Tf> data;
	SlotPtr change, open;
	int H;
	int a, a1;
	int b;
	int tf_count;
	int sym_count;
	int feedback, targets;
	int hact;
	int epoch;

	
public:
	NARX();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "narx";}
	virtual String GetName() {return "NARX";}
};




// RLAgent and DQNAgent are almost identical.

class RLAgent : public Slot {
	
	enum {ACT_IDLE, ACT_LONG, ACT_SHORT};
	
	struct SymTf : Moveable<SymTf> {
		ConvNet::Brain brain;
		int action, prev_action;
		double reward;
	};
	Vector<SymTf> data;
	SymTf& GetData(const SlotProcessAttributes& attr) {return data[attr.sym_id * tf_count + attr.tf_id];}
	
	SlotPtr src;
	Vector<double> input_array;
	int sym_count, tf_count;
	int max_shift, total;
	bool do_training;
	
	void Forward(const SlotProcessAttributes& attr);
	void Backward(const SlotProcessAttributes& attr);
public:
	RLAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "rl";}
	virtual String GetName() {return "RL-Agent";}
};



class DQNAgent : public Slot {
	
	struct SymTf : Moveable<SymTf> {
		ConvNet::DQNAgent agent;
		int action, prev_action, velocity;
		double reward;
	};
	Vector<SymTf> data;
	SymTf& GetData(const SlotProcessAttributes& attr) {return data[attr.sym_id * tf_count + attr.tf_id];}
	
	int max_velocity;
	
	SlotPtr src;
	Vector<double> input_array;
	int sym_count, tf_count;
	int max_shift, total;
	bool do_training;
	
	void Forward(const SlotProcessAttributes& attr);
	void Backward(const SlotProcessAttributes& attr);
public:
	DQNAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "dqn";}
	virtual String GetName() {return "DQN-Agent";}
};



class MonaAgent : public Slot {
	
	enum {IDLE, LONG, SHORT, CLOSE};
	
	struct SymTf : Moveable<SymTf> {
		Mona mona;
		int action, prev_action;
		double reward, prev_open;
	};
	Vector<SymTf> data;
	SymTf& GetData(const SlotProcessAttributes& attr) {return data[attr.sym_id * tf_count + attr.tf_id];}
	
	SlotPtr src;
	Vector<double> input_array;
	double CHEESE_NEED, CHEESE_GOAL;
	int sym_count, tf_count;
	int max_shift, total;
	bool do_training;
	
public:
	MonaAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "mona";}
	virtual String GetName() {return "Mona";}
};






}

#endif
