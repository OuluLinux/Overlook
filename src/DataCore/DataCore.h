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

struct Batch : Moveable<Batch> {
	Batch() {begin = Time(1970,1,1); end = begin; stored = begin; loaded = begin; cursor = 0;}
	VectorMap<String, SlotPtr> slots;
	Index<String> slot_links;
	Vector<BatchPartStatus> status;
	Time begin, end, stored, loaded;
	Atomic cursor;
	
	void Serialize(Stream& s) {
		s % slot_links % status % begin % end % stored % loaded;
		if (s.IsLoading()) {
			int i;
			s % i;
			cursor = i;
		} else {
			int i = cursor;
			s % i;
		}
	}
};

class Session {
	
protected:
	VectorMap<String, String> link_core, link_ctrl;
	Vector<Batch> batches;
	Vector<int> tfs;
	Time begin, end;
	String addr;
	String datadir;
	int port;
	int timeslot;
	int loop;
	bool running, stopped;
	
	// Data locking
	struct LockedArea : Moveable<LockedArea> {int slot, sym, tf;};
	Vector<int> locked;
	SpinLock thrd_lock;
	
	
	TimeVector tv;
	String name;
	enum {PROC_ATTACK, PROC_SUSTAIN, PROC_RELEASE}; // from musical terms
	
	void Run();
	void BatchProcessor(int thread_id, Batch* batch);
	void Processor(BatchPartStatus& stat);
	void RefreshBatches();
	bool Loop(BatchPartStatus& stat, SlotProcessAttributes& attr, TimeStop* ts=NULL, int call=PROC_SUSTAIN);
	
public:
	typedef Session CLASSNAME;
	Session();
	~Session();
	
	void Init();
	void StoreThis();
	void LoadThis();
	void StoreProgress();
	void LoadProgress();
	void Stop();
	
	TimeVector& GetTimeVector() {return tv;}
	Batch& GetBatch(int i) {return batches[i];}
	int GetBatchCount() const {return batches.GetCount();}
	String GetName() const {return name;}
	
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
	bool IsLocked(int slot, int sym_id, int tf_id);
	
	Callback WhenBatchFinished, WhenPartFinished;
	Callback2<int, int> WhenProgress, WhenPartProgress;
	
};
	




inline Session& GetSession() {return Single<Session>();}
inline TimeVector& GetTimeVector() {return GetSession().GetTimeVector();}

}

#endif
