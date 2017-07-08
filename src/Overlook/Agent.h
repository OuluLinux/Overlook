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
	
	
	double equity;
	Vector<Volume> outputs;
	
	
	void Serialize(Stream& s);
	
};


struct SequenceSorter {
	bool operator()(const Sequence& a, const Sequence& b) const {
		return a.equity > b.equity;
	}
};


struct SequencerThread {
	typedef SequencerThread CLASSNAME;
	
	One<Sequence>		seq;
	ConvNet::Session	ses;
	SimBroker			broker;
	int					id;
	
	
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
	friend struct RealtimeSession;
	
	
	// Persistent vars
	int						thrd_count;
	ConvNet::Window			seq_results;
	int						session_count;
	Array<Sequence>			sequences;
	ConvNet::Session		ses;
	
	//ConvNet::Window			loss_window, reward_window, l1_loss_window, l2_loss_window, train_window, accuracy_window;
	//ConvNet::Window			test_reward_window, test_window0, test_window1, train_broker;
	String					params;
	int						epochs, epoch_actual, epoch_total;
	int  prev_symset_hash;
	int  seq_cur;
	/*double				total_sigchange, train_brokerprofit;
	int						train_brokerorders;
	int						symset_hash;*/
	/*void Serialize(Stream& s) {
		s % settings % ses % params % epochs % epoch_actual % epoch_total % id % is_finished
		  % loss_window % reward_window % l1_loss_window % l2_loss_window % train_window % accuracy_window
		  % test_reward_window % test_window0 % test_window1 % train_broker % total_sigchange
		  % train_brokerprofit % train_brokerorders % symset_hash;
	}*/
	
	
	
	// Tmp vars
	// Array<ConvNet::Window> thrd_priorities, thrd_performances;
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
	TimeStop last_store;
	ConvNet::Session ro_ses;
	System* sys;
	int max_sequences, max_tmp_sequences;
	int not_stopped;
	int test_interval;
	int input_width, input_height, input_depth, output_width;
	int training_limit;
	int session_cur;
	int tf_limit;
	int symset_hash;
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
	
	void Serialize(Stream& s) {/*s % thrd_count % sequences % seq_results % session_count;*/}
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
