#ifndef _Overlook_Trainer_h_
#define _Overlook_Trainer_h_

namespace Overlook {
using namespace Upp;

struct Iterator : Moveable<Iterator> {
	Vector<int> pos, periods, period_in_slower;
	
};

class Trainer {
	Vector<Iterator> iters;
	Vector<Ptr<CoreItem> > work_queue;
	Index<int> tf_ids, sym_ids, indi_ids;
	System* sys;
	
public:
	Trainer(System& sys);
	
	void Init();
	void RefreshWorkQueue();
	void ProcessWorkQueue();
};

}

#endif
