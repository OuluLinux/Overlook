#ifndef _DataCore_NeuralDoubleAgents_h_
#define _DataCore_NeuralDoubleAgents_h_

#include "Slot.h"
#include "SimBroker.h"

namespace DataCore {
using namespace Narx;

/*
	The term "double-agent" has little meaning in this context. It comes from using agents of single
	symbols, thus being agents-of-agents, which leads to "double agents" in a word play.
*/

class DQNDoubleAgent : public Slot {
	
	ConvNet::DQNAgent agent;
	SimBroker broker;
	double reward;
	Vector<int> velocity;
	int action, prev_action;
	int actions_total;
	int max_velocity;
	
	SlotPtr src;
	Vector<double> input_array;
	int sym_count, tf_count;
	int max_shift, total;
	bool do_training;
	
	void Forward(const SlotProcessAttributes& attr);
	void Backward(const SlotProcessAttributes& attr);
public:
	DQNDoubleAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "doubledqn";}
	virtual String GetName() {return "Double DQN-Agent";}
};



class MonaDoubleAgent : public Slot {
	
	Mona agent;
	SimBroker broker;
	Vector<int> velocity;
	double CHEESE_NEED, CHEESE_GOAL;
	double reward;
	int action, prev_action;
	int actions_total;
	int max_velocity;
	
	SlotPtr src;
	Vector<double> input_array;
	int sym_count, tf_count;
	int max_shift, total;
	bool do_training;
	
	void Forward(const SlotProcessAttributes& attr);
	void Backward(const SlotProcessAttributes& attr);
public:
	MonaDoubleAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "doublemona";}
	virtual String GetName() {return "Double Mona-Agent";}
};


}

#endif
