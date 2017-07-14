#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_

namespace Overlook {
using namespace Upp;
using ConvNet::DQNAgent;
using ConvNet::Volume;
using ConvNet::VolumePtr;

typedef Tuple3<double, double, double> DoubleTrio;

struct Snapshot : Moveable<Snapshot> {
	Vector<double> values;
	Vector<int> time_values;
	Time time, added;
	int shift;
	
	Snapshot() : shift(-1) {}
	
};

struct TfSymAverage {
	Vector<double> data;
	Vector<double> tf_sums;
	Vector<int> tf_periods;
	int pos;
	int bars;
	
	TfSymAverage() : pos(-1), bars(-1) {}
	
	void Reset(int bars) {
		this->bars = bars;
		pos = 0;
		data.SetCount(0);
		tf_sums.SetCount(0);
		data.SetCount(bars, 0.0);
		tf_sums.SetCount(tf_periods.GetCount(), 0.0);
	}
	void SeekNext() {
		if (pos >= bars) return;
		double value = data[pos];
		for(int j = 0; j < tf_periods.GetCount(); j++) {
			int prev = pos - tf_periods[j];
			if (prev >= 0)
				tf_sums[j] -= data[prev];
			tf_sums[j] += value;
		}
		pos++;
	}
	void Set(double value) {
		data[pos] = value;
	}
	double Get(int tf) {
		return tf_sums[tf] / tf_periods[tf];
	}
};

class AgentGroup;

class Agent {
	
protected:
	friend class AgentCtrl;
	friend class AgentDraw;
	friend class AgentConfiguration;
	friend class RealtimeStatistics;
	friend class AgentThreadCtrl;
	friend class StatsGraph;
	friend class AgentTraining;
	friend class TrainingGraph;
	friend class RealtimeNetworkCtrl;
	friend class SnapshotCtrl;
	friend class ManagerCtrl;
	friend class AgentGroup;
	friend class AgentTabCtrl;
	friend struct RealtimeSession;
	
	
	// Persistent vars
	Vector<double>			seq_results;
	DQNAgent				dqn;
	int						sym, proxy_sym, group_id;
	
	
	// Tmp vars
	Vector<double> input_array;
	Vector<double> thrd_equity;
	TfSymAverage reward_average, signal_average;
	SimBroker broker;
	AgentGroup* group;
	double prev_equity;
	double prev_reward;
	double smooth_reward;
	int epoch_actual, epoch_total;
	int iter;
	int group_count;
	int not_stopped;
	bool running;
	
	void Forward(Snapshot& snap, Brokerage& broker, Snapshot* next_snap);
	void Backward(double reward);
	void Action();
	void RealAction();
	void Main();
	void SetAskBid(SimBroker& sb, int pos);
	int GetRandomAction() const {return Random(ACTIONCOUNT);}
	int GetAction(const Volume& fwd, int sym) const;
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG,     ACTIONCOUNT};
	
	
public:
	typedef Agent CLASSNAME;
	
	Agent();
	~Agent();
	
	void Serialize(Stream& s);
	void Create();
	void Init();
	void InitThreads();
	void Start();
	void Stop();
	void SetBrokerageSignals(Brokerage& broker, int pos);
	
	const Vector<double>& GetSequenceResults() const {return seq_results;}
	
	Callback1<double> WhenRewardAverage;
	
};

}

#endif
