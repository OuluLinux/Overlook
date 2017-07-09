#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_

namespace Overlook {
using namespace Upp;
using ConvNet::SDQNAgent;

typedef Tuple3<double, double, double> DoubleTrio;
typedef ConvNet::VolumeData<double> VolumeDouble;

struct Snapshot : Moveable<Snapshot> {
	Vector<Vector<Vector<DoubleTrio> > > value;
	Vector<Vector<double> > min_value, max_value;
	Vector<int> pos, tfs, periods, period_in_slower, time_values;
	//ConvNet::VolumeData<double> volume_out;
	Volume volume_in;
	Time begin;
	int begin_ts;
	int value_count;
	int bars;
};


struct Sequence {
	
	
	int id, orders;
	double equity;
	Vector<Volume> outputs;
	
	
	Sequence() {
		id = -1;
		orders = 0;
	}
	void Serialize(Stream& s) {
		s % id % orders % equity % outputs;
	}
	
};


struct SequenceSorter {
	bool operator()(const Sequence& a, const Sequence& b) const {
		return a.equity > b.equity;
	}
};


struct SequencerThread {
	typedef SequencerThread CLASSNAME;
	SequencerThread() {
		snap_id = 0;
		id = -1;
	}
	One<Sequence>		seq;
	ConvNet::Session	ses;
	SimBroker			broker;
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
	Vector<int> data_begins;
	Vector<int> train_pos;
	SimBroker latest_broker;
	Snapshot latest_snap;
	TimeStop last_store;
	ConvNet::Session ro_ses;
	System* sys;
	int						thrd_count;
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
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG, ACT_RESETSIG, ACT_INCBET, ACT_DECBET,     ACTIONCOUNT};
	
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
