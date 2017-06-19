#ifndef _Overlook_Trainer_h_
#define _Overlook_Trainer_h_

namespace Overlook {
using namespace Upp;
using ConvNet::SDQNAgent;

typedef Tuple2<double, double> DoublePair;

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
	Vector<Iterator> iters;
	Vector<Ptr<CoreItem> > work_queue, major_queue;
	Vector<Vector<Vector<ConstBuffer*> > > value_buffers;
	Index<int> tf_ids, sym_ids, indi_ids;
	Array<SDQNAgent> agents;
	Array<SimBroker> seqs;
	System* sys;
	bool running, stopped;
	
	void Runner();
	void AgentAct(int tf_iter, int seq, int sym);
	
public:
	typedef Trainer CLASSNAME;
	Trainer(System& sys);
	~Trainer() {Stop();}
	
	void Init();
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
