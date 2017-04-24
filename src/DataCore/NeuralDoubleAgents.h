#ifndef _DataCore_NeuralDoubleAgents_h_
#define _DataCore_NeuralDoubleAgents_h_

#include "Slot.h"
#include "SimBroker.h"

namespace DataCore {
using namespace Narx;

/*
	The term "double-agent" has little meaning in this context. It comes from using agents of single
	symbols, thus being agents-of-agents, which leads to "double agents" in a word play.
	
	These requires single-symbol agents to be entirely trained. Difference between results with
	training data and testing data must be taken into account, because these get stacked on the
	training data, and the combination might get too bad with the testing data. Basically: good
	stack with training data is worthless if it is bad with the testing data, and moderately
	well working stack is better if it works with equally good results in the testing data.
	
	Single-symbol agents must also take spread costs into account. Training without spread-costs
	leads to very different strategy than with them. Without spreads, the agent will do very
	high frequency trading, which leads to rapid loss of value with retail client accounts.
	(Also real HFT is a completely different topic and not related to Overlook at all.)
	With spread-costs, agent will limit orders to those which will overcome spread-costs at least.
	
	On the other hand, pre-agent slots have no reason to use spread data, because they are
	concerned only about the signal, which comes from the big pool, mostly from outside of the broker.
	
	These "double-agents" uses directly the spread-overcoming-signal from single-symbol-agents.
	Together with exogenous signals (time of day, weekday, event oscillator, etc.) they learn
	probably surviving orders from different combinations of single-agent-signals.
	
	My current view is that there is no point in other type of high-level agents than Mona.
	These have to be increasingly state-aware and cycle-aware, and Mona also kind of "grows in
	complexity" until it fits input, unlike DQN, which must have enough neurons in layers since
	beginning.
	
	For performance reasons, MonaDoubleAgent doesn't take slower timeframes as sensors. They
	are already used in single-symbol meta-agent. The web-effect of symbols should be more
	sensored, but while the processing is slow, it must be done in efficient spots, which might
	have weak web-effect sensoring. Currently only NARX combines different symbols and
	MetaAgent combines different timeframes, which is sub-optimal and minimal, and untested.
*/



class MonaDoubleAgent : public Slot {
	
	Mona agent;
	SimBroker broker;
	Vector<int> velocity;
	double CHEESE_NEED, CHEESE_GOAL;
	int action, prev_action;
	int actions_total;
	int max_velocity;
	
	SlotPtr src, metamona;
	Vector<double> input_array;
	int sym_count, tf_count;
	int total;
	bool do_training;
	
	void Forward(const SlotProcessAttributes& attr);
public:
	MonaDoubleAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "doublemona";}
	virtual String GetName() {return "Double Mona-Agent";}
	virtual String GetCtrl() const {return "agentctrl";}
	virtual int GetCtrlType() const {return SLOT_ONCE;}
	virtual int GetType() const {return SLOT_ONCE;}
};


}

#endif
