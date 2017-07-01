#ifndef _Overlook_Trainer_h_
#define _Overlook_Trainer_h_

namespace Overlook {
using namespace Upp;
using ConvNet::SDQNAgent;

typedef Tuple2<double, double> DoublePair;

struct SessionThread {
	typedef SessionThread CLASSNAME;
	
	ConvNet::Session	ses;
	SimBroker			broker;
	
	void Serialize(Stream& s) {s % ses;}
};

struct Iterator : Moveable<Iterator> {
	Vector<Vector<Vector<DoublePair> > > value;
	Vector<Vector<double> > min_value, max_value;
	Vector<int> pos, tfs, periods, period_in_slower, time_values;
	Time begin;
	int begin_ts;
	int value_count;
	int bars;
};

class Trainer {
	
protected:
	friend class TrainerCtrl;
	friend class TrainerDraw;
	
	
	// Persistent vars
	int thrd_count;
	Array<SessionThread> thrds;
	
	
	// Tmp vars
	Vector<Iterator> iters;
	Vector<Ptr<CoreItem> > work_queue, major_queue;
	Vector<Vector<Vector<ConstBuffer*> > > value_buffers;
	Index<int> tf_ids, sym_ids, indi_ids;
	System* sys;
	int not_stopped;
	bool running;
	
	void LoadThis();
	void StoreThis();
	void Runner(int i);
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG, ACT_RESETSIG, ACT_INCBET, ACT_DECBET,     ACTIONCOUNT};
	
public:
	typedef Trainer CLASSNAME;
	Trainer(System& sys);
	~Trainer() {Stop();}
	
	void Serialize(Stream& s) {s % thrd_count % thrds;}
	
	void Init();
	void InitThreads();
	void Start();
	void Stop();
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ResetIterators();
	void ResetValueBuffers();
	
	bool Seek(int tf_iter, int shift);
	bool SeekCur(int tf_iter, int shift);
};

}

#endif
