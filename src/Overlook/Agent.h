#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_

namespace Overlook {
using namespace Upp;
using ConvNet::SDQNAgent;

typedef Tuple3<double, double, double> DoubleTrio;
typedef ConvNet::VolumeData<double> VolumeDouble;

struct Snapshot : Moveable<Snapshot> {
	//Vector<Vector<Vector<DoubleTrio> > > value;
	Vector<Vector<double> > min_value, max_value;
	Vector<int> pos, tfs, periods, period_in_slower, time_values;
	Volume volume_in;
	Time begin;
	int begin_ts;
	//int value_count;
	int bars;
};


struct Experience : Moveable<Experience> {
	Volume input;
	Vector<double> reward;
	Vector<int> action;
	
	void Serialize(Stream& s) {
		s % input % reward % action;
	}
};

struct Sequence {
	int id, orders;
	double equity;
	Vector<Experience> exps;
	
	
	Sequence() {
		id = -1;
		orders = 0;
	}
	
	void Serialize(Stream& s) {
		s % id % orders % equity % exps;
	}
};


struct SequenceSorter {
	bool operator()(const Sequence& a, const Sequence& b) const {
		return a.equity > b.equity;
	}
};


struct TfSymAverage {
	Vector<Vector<double> > data;
	Vector<Vector<double> > tf_sums;
	Vector<int> tf_periods;
	int sym_count;
	int pos;
	int bars;
	
	TfSymAverage() : sym_count(-1), pos(-1), bars(-1) {}
	
	void Reset(int bars) {
		this->bars = bars;
		pos = 0;
		data.SetCount(0);
		tf_sums.SetCount(0);
		data.SetCount(sym_count);
		tf_sums.SetCount(sym_count);
		for(int i = 0; i < data.GetCount(); i++) data[i].SetCount(bars, 0.0);
		for(int i = 0; i < tf_sums.GetCount(); i++) tf_sums[i].SetCount(tf_periods.GetCount(), 0.0);
	}
	void SeekNext() {
		if (pos >= bars) return;
		for(int i = 0; i < sym_count; i++) {
			double value = data[i][pos];
			for(int j = 0; j < tf_periods.GetCount(); j++) {
				int prev = pos - tf_periods[j];
				if (prev >= 0)
					tf_sums[i][j] -= data[i][prev];
				tf_sums[i][j] += value;
			}
		}
		pos++;
	}
	void Set(int sym, double value) {
		data[sym][pos] = value;
	}
	double Get(int sym, int tf) {
		return tf_sums[sym][tf] / tf_periods[tf];
	}
};


struct SequencerThread {
	typedef SequencerThread CLASSNAME;
	SequencerThread() {
		snap_id = 0;
		id = -1;
		rand_epsilon = 1.0;
	}
	One<Sequence>		seq;
	ConvNet::Session	ses;
	SimBroker			broker;
	TfSymAverage		average;
	double				rand_epsilon;
	int					id;
	int					snap_id;
};


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
	friend struct RealtimeSession;
	
	
	// Persistent vars
	Array<Sequence>			sequences;
	Vector<double>			seq_results;
	ConvNet::Session		ses;
	int						sequence_count;
	int						epochs, epoch_actual, epoch_total;
	int						prev_symset_hash;
	
	
	// Tmp vars
	// Array<ConvNet::Window> thrd_priorities, thrd_performances;
	Vector<Vector<double> > thrd_equities;
	Array<Sequence> tmp_sequences;
	Array<Snapshot> snaps;
	Array<SequencerThread> thrds;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Vector<Vector<ConstBuffer*> > > value_buffers;
	Vector<Core*> databridge_cores;
	Vector<double> tf_muls;
	Index<int> tf_ids, sym_ids, indi_ids;
	Vector<int> tf_periods;
	Vector<int> data_begins;
	Vector<int> train_pos;
	SimBroker latest_broker;
	Snapshot latest_snap;
	TimeStop last_store;
	ConvNet::Session ro_ses;
	System* sys;
	double epsilon_min;
	int buf_count;
	int learning_epochs_total, learning_epochs_burnin;
	int thrd_count;
	int max_sequences, max_tmp_sequences;
	int not_stopped;
	int test_interval;
	int input_width, input_height, input_depth, output_width;
	int training_limit;
	int session_cur;
	int tf_limit;
	int symset_hash;
	int seq_cur;
	bool running;
	Mutex sequencer_lock, trainer_lock;
	
	
	void LoadThis();
	void SequencerHandler(int i);
	void TrainerHandler();
	void Runner(int thrd_id);
	void SetAskBid(SimBroker& sb, int pos);
	void GenerateSnapshots();
	void ForwardSignals(ConvNet::Net& net, Brokerage& broker, const Volume& fwd, Experience& exp, double epsilon=0.0, bool sig_freeze=false);
	int GetRandomAction() const {return Random(ACTIONCOUNT);}
	int GetAction(const Volume& fwd, int sym) const;
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG,     ACTIONCOUNT};
	
public:
	typedef Agent CLASSNAME;
	Agent(System& sys);
	~Agent();
	
	void Serialize(Stream& s) {
		s % sequences % seq_results % ses % sequence_count % epochs % epoch_actual % epoch_total
		  % prev_symset_hash;
	}
	void StoreThis();
	
	void Init();
	void InitThreads();
	void Start();
	void Stop();
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ProcessDataBridgeQueue();
	void ResetValueBuffers();
	void SetBrokerageSignals(Brokerage& broker, int pos);
	
	void ResetSnapshot(Snapshot& snap);
	bool Seek(Snapshot& snap, int shift);
	bool SeekCur(Snapshot& snap, int shift);
	
	const Vector<double>& GetSequenceResults() const {return seq_results;}
};

struct RealtimeSession {
	typedef RealtimeSession CLASSNAME;
	RealtimeSession(Agent& agent);
	
	void Init();
	void Run();
	void Stop();
	void PostEvent(int event);
	void Start() {running = true; stopped = false; Thread::Start(THISBACK(Run));}
	
	enum {EVENT_REFRESH, EVENT_KILL};
	
protected:
	Brokerage* broker;
	Agent* agent;
	Vector<int> event_queue;
	SpinLock lock;
	bool running, stopped;
};

}

#endif
