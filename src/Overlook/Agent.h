#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_

namespace Overlook {
using namespace Upp;
using ConvNet::DQNAgent;
using ConvNet::Volume;
using ConvNet::VolumePtr;

typedef Tuple3<double, double, double> DoubleTrio;

struct Snapshot : Moveable<Snapshot> {
	Vector<int> pos, tfs, periods, period_in_slower, time_values;
	Vector<double> values;
	Time time, added;
	Time begin;
	int begin_ts;
	int bars;
	int shift;
	bool is_valid;
	
	Snapshot() : is_valid(false), shift(-1) {}
	
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
	friend struct RealtimeSession;
	
	
	// Persistent vars
	Vector<double>			seq_results;
	DQNAgent				dqn;
	String					param_str;
	double					global_free_margin_level;
	int						sequence_count;
	int						epochs;
	int						prev_symset_hash;
	
	
	
	// Tmp vars
	Vector<double> input_array;
	Vector<double> thrd_equity;
	Array<Snapshot> snaps;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Vector<ConstBuffer*> > value_buffers;
	Vector<VolumePtr> vec;
	Vector<double> tf_muls;
	Index<int> tf_ids, indi_ids;
	Vector<int> tf_periods;
	Vector<int> data_begins;
	Vector<int> train_pos;
	SimBroker broker, latest_broker;
	Snapshot latest_snap;
	TfSymAverage reward_average, signal_average;
	System* sys;
	AgentGroup* group;
	Core* databridge_core;
	Core* databridge_proxy_core;
	double epsilon_min, rand_epsilon, prev_reward;
	double smooth_reward;
	int epoch_actual, epoch_total;
	int iter;
	int group_id;
	int buf_count;
	int group_count;
	int learning_epochs_total, learning_epochs_burnin;
	int thrd_count;
	int max_sequences, max_tmp_sequences;
	int not_stopped;
	int test_interval;
	int input_width, input_height, output_width;
	int training_limit;
	int session_cur;
	int seq_cur;
	int sym, proxy_sym, tf;
	bool sig_freeze;
	bool train_single;
	bool prefer_high;
	bool running;
	bool paused;
	
	void Forward(Snapshot& snap, Brokerage& broker);
	void Backward(double reward);
	void Action();
	void RealAction();
	void Main();
	void SetAskBid(SimBroker& sb, int pos);
	void GenerateSnapshots();
	int GetRandomAction() const {return Random(ACTIONCOUNT);}
	int GetAction(const Volume& fwd, int sym) const;
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG,     ACTIONCOUNT};
	
	
public:
	typedef Agent CLASSNAME;
	
	Agent();
	~Agent();
	
	void Serialize(Stream& s) {
		s % seq_results % dqn % param_str % global_free_margin_level % sequence_count % epochs;
	}
	
	void Init();
	void InitThreads();
	void Start();
	void Stop();
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ProcessDataBridgeQueue();
	void ResetValueBuffers();
	void SetBrokerageSignals(Brokerage& broker, int pos);
	
	void RefreshSnapshots();
	void ResetSnapshot(Snapshot& snap);
	bool Seek(Snapshot& snap, int shift);
	bool SeekCur(Snapshot& snap, int shift);
	
	const Vector<double>& GetSequenceResults() const {return seq_results;}
	
	Callback1<double> WhenRewardAverage;
	
};

}

#endif
