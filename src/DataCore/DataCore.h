#ifndef _DataCore_DataCore_h_
#define _DataCore_DataCore_h_

#include "Vector.h"
#include "DummySlot.h"
#include "DataBridge.h"
#include "Indicators.h"
#include "EventManager.h"
#include "Neural.h"
#include "NeuralHuge.h"
#include "Periodicals.h"
#include "CostUtils.h"
#include "NeuralDoubleAgents.h"

namespace DataCore {

struct BatchPartStatus : Moveable<BatchPartStatus> {
	BatchPartStatus() {slot = NULL; begin = Time(1970,1,1); end = begin; sym_id = -1; tf_id = -1; actual = 0; total = 1; complete = false;}
	Slot* slot;
	Time begin, end;
	int sym_id, tf_id, actual, total;
	bool complete;
};

struct Batch : Moveable<Batch> {
	Batch() {begin = Time(1970,1,1); end = begin; stored = begin; loaded = begin;}
	VectorMap<String, SlotPtr> slots;
	Vector<BatchPartStatus> status;
	Time begin, end, stored, loaded;
};

class Session {
	
protected:
	VectorMap<String, String> link_core, link_ctrl;
	Vector<Batch> batches;
	Vector<int> tfs;
	Time begin, end;
	String addr;
	String datadir;
	Atomic cursor;
	int port;
	bool running, stopped;
	
	// Data locking
	struct LockedArea : Moveable<LockedArea> {int slot, sym, tf;};
	VectorMap<int64, LockedArea> locked;
	Mutex thrd_lock;
	
	
	TimeVector tv;
	String name;
	
	void Run();
	void BatchProcessor(int thread_id, Batch* batch);
	void Processor(BatchPartStatus& stat);
	void RefreshBatches();
	
	
public:
	typedef Session CLASSNAME;
	Session();
	~Session();
	
	void Init();
	void StoreThis();
	void LoadThis();
	
	TimeVector& GetTimeVector() {return tv;}
	Batch& GetBatch(int i) {return batches[i];}
	int GetBatchCount() const {return batches.GetCount();}
	
	void SetName(String s) {name = s;}
	void SetBegin(Time t) {begin = t;}
	void SetEnd(Time t) {end = t;}
	void SetAddress(String s) {addr = s;}
	void SetPort(int i) {port = i;}
	
	void LinkCore(String dst, String src) {link_core.Add(dst, src);}
	void LinkCtrl(String dst, String src) {link_ctrl.Add(dst, src);}
	void AddTimeframe(int i) {tfs.Add(i);}
	
	void Enter(int slot, int sym_id, int tf_id);
	void Leave(int slot, int sym_id, int tf_id);
	
	
	Callback WhenBatchFinished, WhenPartFinished;
	Callback2<int, int> WhenProgress, WhenPartProgress;
	
};
	




inline Session& GetSession() {return Single<Session>();}
inline TimeVector& GetTimeVector() {return GetSession().GetTimeVector();}

}

#endif
