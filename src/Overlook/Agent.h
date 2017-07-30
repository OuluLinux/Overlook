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
	bool accum_signal;
	
	
	// Tmp vars
	Vector<double> input_array;
	Snapshot* cur_snap;
	double smooth_reward;
	double prev_equity;
	int accum_buf;
	int group_count;
	int tf_step;
	bool has_yeartime;
	
	virtual void Create(int width, int height);
	virtual void Forward(Snapshot& snap, SimBroker& broker);
	virtual void Backward(double reward);
	void RealAction();
	void Main();
	virtual void SetAskBid(SimBroker& sb, int pos);
	int GetRandomAction() const {return Random(ACTIONCOUNT);}
	int GetAction(const Volume& fwd, int sym) const;
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG, ACT_RESETSIG,     ACTIONCOUNT};
	
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
	
	Callback1<double> WhenRewardAverage;
	
};

}

#endif
