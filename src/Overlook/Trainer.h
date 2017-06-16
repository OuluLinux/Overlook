#ifndef _Overlook_Trainer_h_
#define _Overlook_Trainer_h_

namespace Overlook {
using namespace Upp;

typedef Tuple2<double, double> DoublePair;

struct Iterator : Moveable<Iterator> {
	Vector<Vector<Vector<DoublePair> > > value;
	Vector<Vector<double> > min_value, max_value;
	Vector<int> pos, tfs, periods, period_in_slower;
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
	Vector<Ptr<CoreItem> > work_queue;
	Vector<Vector<Vector<ConstBuffer*> > > value_buffers;
	Index<int> tf_ids, sym_ids, indi_ids;
	System* sys;
	
public:
	Trainer(System& sys);
	
	void Init();
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ResetIterators();
	void ResetValueBuffers();
	
	bool Seek(int tf_iter, int shift);
	bool SeekCur(int tf_iter, int shift);
};

}

#endif
