#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_

namespace Overlook {
using namespace Upp;
using ConvNet::DQNAgent;
using ConvNet::Volume;
using ConvNet::VolumePtr;

typedef Tuple3<double, double, double> DoubleTrio;

class Agent : public TraineeBase {
	
protected:
	friend class AgentCtrl;
	friend class SnapshotDraw;
	friend class EquityGraph;
	friend class TrainingCtrl;
	friend class ResultGraph;
	friend class SnapshotCtrl;
	friend class ManagerCtrl;
	friend class AgentGroup;
	friend class AgentTabCtrl;
	friend class GroupOverview;
	
	
	// Persistent vars
	ConvNet::DQNAgent dqn;
	int agent_id, sym_id, sym, proxy_sym;
	int agent_input_width, agent_input_height;
	bool has_timesteps;
	
	
	// Tmp vars
	Vector<double> input_array;
	double posreward, negreward;
	double smooth_reward;
	double prev_equity;
	int group_count;
	int tf_step;
	int timestep;
	bool has_yeartime;
	bool is_training_iteration;
	
	virtual void Create(int width, int height);
	virtual void Forward(Snapshot& snap, SimBroker& broker);
	virtual void Backward(double reward);
	void RealAction();
	void Main();
	virtual void SetAskBid(SimBroker& sb, int pos);
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG, ACTIONCOUNT};
	
public:
	typedef Agent CLASSNAME;
	
	Agent();
	~Agent();
	
	void Serialize(Stream& s);
	void Init();
	void Start();
	void Stop();
	void SetBrokerageSignals(Brokerage& broker, int pos);
	void RefreshTotalEpochs();
	Callback MainCallback() {return Callback(THISBACK(Main));}
	
	Callback1<double> WhenRewardAverage;
	
};

}

#endif
