#ifndef _DataCore_Neural_h_
#define _DataCore_Neural_h_

#include <Mona/Mona.h>
#include <NARX/NARX.h>
#include "Slot.h"
#include "SimBroker.h"

namespace DataCore {
using namespace Narx;




class Recurrent : public Slot {
	Array<Array<ConvNet::RecurrentSession> > ses;
	Vector<double> temperatures;
	Vector<int> sequence;
	OnlineVariance var;
	String model_str;
	int batch;
	int shifts;
	int value_count;
	int iter;
	int tick_time;
	bool is_training_loop;
	SpinLock lock;
	
	int ToChar(double diff, double min, double max);
	double FromChar(int i, double min, double max);
	
	void StoreThis();
	void LoadThis();
	void InitSessions();
public:
	Recurrent();
	virtual String GetKey() const {return "rnn";}
	virtual String GetCtrl() const {return "rnnctrl";}
	virtual int GetCtrlType() const {return SLOT_SYMTF;}
	virtual int GetType() const {return SLOT_SYMTF;}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual void SerializeCache(Stream& s, int sym_id, int tf_id);
	
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
	void PredictSentence(const SlotProcessAttributes& attr, bool samplei=false, double temperature=1.0);
	void Tick(const SlotProcessAttributes& attr);
	
	ConvNet::RecurrentSession& GetSession(int sym, int tf) {return ses[sym][tf];}
	
	void Serialize(Stream& s) {
		s % temperatures % var % model_str % batch % shifts % value_count % iter %
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
		
		void Serialize(Stream& s) {
			s % Y % pY % input_count %
				hunits % output_units % inputs % feedbacks % exogenous % ee % rw;
		}
	};
	Tf& GetData(const SlotProcessAttributes& attr) {return data[attr.tf_id];}
	
	Vector<double> out;
	Vector<Tf> data;
	int H;
	int a, a1;
	int b;
	int feedback, targets;
	int hact;
	int epoch;
	
	int tf_count;
	int sym_count;
	
public:
	NARX();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "narx";}
	virtual String GetName() {return "NARX";}
	virtual String GetCtrl() const {return "narxctrl";}
	virtual int GetCtrlType() const {return SLOT_TF;}
	virtual int GetType() const {return SLOT_TF;}
	
	virtual void Serialize(Stream& s) {
		s % data % H % a % a1 % b % feedback % targets % hact % epoch;
	}
};



// Forecaster is just a regression neural network for multiple inputs.
// It is like a complex version of ConvNet Regression1D example.
class Forecaster : public Slot {
	String t;
	ConvNet::Session ses;
	
	struct SymTf : Moveable<SymTf> {
		ConvNet::Session ses;
		void Serialize(Stream& s) {s % ses;}
	};
	Vector<SymTf> data;
	SymTf& GetData(const SlotProcessAttributes& attr) {return data[attr.sym_id * tf_count + attr.tf_id];}
	
	ConvNet::Volume input, output;
	int sym_count, tf_count;
	bool do_training;
	
	void Forward(const SlotProcessAttributes& attr);
	void Backward(const SlotProcessAttributes& attr);
public:
	Forecaster();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "forecaster";}
	virtual String GetName() {return "Forecaster";}
	virtual String GetCtrl() const {return "forecasterctrl";}
	virtual int GetCtrlType() const {return SLOT_SYMTF;}
	virtual int GetType() const {return SLOT_SYMTF;}
	virtual void SerializeCache(Stream& s, int sym_id, int tf_id);
};


// RLAgent and DQNAgent are almost identical.

class RLAgent : public Slot {
	
	enum {ACT_IDLE, ACT_LONG, ACT_SHORT};
	
	struct SymTf : Moveable<SymTf> {
		ConvNet::Brain brain;
		int action, prev_action;
		double reward;
		void Serialize(Stream& s) {s % brain % action % prev_action % reward;}
	};
	Vector<SymTf> data;
	SymTf& GetData(const SlotProcessAttributes& attr) {return data[attr.sym_id * tf_count + attr.tf_id];}
	
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
	virtual String GetCtrl() const {return "agentctrl";}
	virtual int GetCtrlType() const {return SLOT_SYMTF;}
	virtual int GetType() const {return SLOT_SYMTF;}
	virtual void SerializeCache(Stream& s, int sym_id, int tf_id);
};



class DQNAgent : public Slot {
	
	struct SymTf : Moveable<SymTf> {
		ConvNet::DQNAgent agent;
		int action, prev_action, velocity;
		double reward;
		void Serialize(Stream& s) {s % agent % action % prev_action % velocity % reward;}
	};
	Vector<SymTf> data;
	SymTf& GetData(const SlotProcessAttributes& attr) {return data[attr.sym_id * tf_count + attr.tf_id];}
	
	int max_velocity;
	
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
	virtual String GetCtrl() const {return "agentctrl";}
	virtual int GetCtrlType() const {return SLOT_SYMTF;}
	virtual int GetType() const {return SLOT_SYMTF;}
	virtual void SerializeCache(Stream& s, int sym_id, int tf_id);
};



class MonaAgent : public Slot {
	
	enum {IDLE, LONG, SHORT, CLOSE};
	
	struct SymTf : Moveable<SymTf> {
		Mona mona;
		int action, prev_action;
		double reward, prev_open;
		void Serialize(Stream& s) {s % mona % action % prev_action % reward % prev_open;}
	};
	Vector<SymTf> data;
	SymTf& GetData(const SlotProcessAttributes& attr) {return data[attr.sym_id * tf_count + attr.tf_id];}
	
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
	virtual String GetCtrl() const {return "agentctrl";}
	virtual int GetCtrlType() const {return SLOT_SYMTF;}
	virtual int GetType() const {return SLOT_SYMTF;}
	virtual void SerializeCache(Stream& s, int sym_id, int tf_id);
};




/*
	Mona-meta-agent takes multiple agents (rl, dqn, mona) signals as inputs values from multiple
	timeframes, and combines them as one better trader. Assertions have to be made, that it
	really is performancing better than any single agent.
*/
class MonaMetaAgent : public Slot {
	
	enum {IDLE, LONG, SHORT, CLOSE};
	
	struct SymTf : Moveable<SymTf> {
		Mona mona;
		int action, prev_action;
		double reward, prev_open;
		void Serialize(Stream& s) {s % mona % action % prev_action % reward % prev_open;}
	};
	Vector<SymTf> data;
	SymTf& GetData(const SlotProcessAttributes& attr) {return data[attr.sym_id * tf_count + attr.tf_id];}
	
	Vector<double> input_array;
	double CHEESE_NEED, CHEESE_GOAL;
	int sym_count, tf_count;
	int total;
	bool do_training;
	
public:
	MonaMetaAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "metamona";}
	virtual String GetName() {return "MetaMona";}
	virtual String GetCtrl() const {return "agentctrl";}
	virtual int GetCtrlType() const {return SLOT_SYMTF;}
	virtual int GetType() const {return SLOT_SYMTF;}
	virtual void SerializeCache(Stream& s, int sym_id, int tf_id);
};


}

#endif
