#ifndef _Overlook_FastEventStatistics_h_
#define _Overlook_FastEventStatistics_h_

namespace Overlook {

#define FAST_TF 4
#define MAX_FAST_LEN 4
#define FAST_WIDTH (24*7)
#define FAST_ENABLED 1
#define NEGCOUNT_ENABLED 1
#define NEGCOUNT_MAX 1

class FastStatSlot : Moveable<FastStatSlot> {
	
	
public:
	
	void AddResult(int i, double d, double d2) {ASSERT(i >= 0 && i < MAX_FAST_LEN); av[i].Add(d); abs_av[i].Add(fabs(d2));}
	void AddInvResult(int i, double d, double d2) {ASSERT(i >= 0 && i < MAX_FAST_LEN); inv_av[i].Add(d); inv_abs_av[i].Add(fabs(d2));}
	
	
	OnlineVariance av[MAX_FAST_LEN], abs_av[MAX_FAST_LEN];
	OnlineVariance inv_av[MAX_FAST_LEN], inv_abs_av[MAX_FAST_LEN];
};

class FastEventStatistics : public Common {
	
	
protected:
	friend class FastEventStatisticsCtrl;
	friend class FastEventConsole;
	friend class FastEventOptimization;
	
	
	enum {OPEN, BB5, BB10, BB15, BB20, BB30, BB40, BB50, BB60, SRC_COUNT};
	
	
	
	// Persistent
	Vector<Vector<Vector<FastStatSlot> > > stats;
	
	
	// Temporary
	Index<String> symbols;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<Vector<Vector<ConstBuffer*> > > bufs;
	Vector<Vector<ConstLabelSignal*> > lbls;
	Vector<DataBridge*> db, db_m1;
	Index<int> sym_ids, tf_ids;
	int prev_bars = 0;
	int last_alarm1_bars = 0;
	bool running = false, stopped = true;
	
	
public:
	typedef FastEventStatistics CLASSNAME;
	FastEventStatistics();
	~FastEventStatistics();
	
	virtual void Init();
	virtual void Start();
	void UpdateEvents(int sym);
	void RefreshData();
	
	String GetDescription(int i);
	int GetSignal(int sym, int i, int src);
	int GetOpenSignal(int sym, int i, int src);
	int GetLatestSlotId();
	const FastStatSlot& GetLatestSlot(int net, int i);
	const FastStatSlot& GetSlot(int net, int i, int wb);
	
	ConstBuffer& GetBuffer(int sym, int src, int buf) {return *bufs[sym][src][buf];}
	
};

inline FastEventStatistics& GetFastEventStatistics() {return GetSystem().GetCommon<FastEventStatistics>();}


class FastEventStatisticsCtrl : public CommonCtrl {
	ArrayCtrl list;
	DropList symlist, steplist;
	
public:
	typedef FastEventStatisticsCtrl CLASSNAME;
	FastEventStatisticsCtrl();
	
	virtual void Data();
	
};

}

#endif
