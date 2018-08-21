#ifndef _Overlook_EventStatistics_h_
#define _Overlook_EventStatistics_h_

namespace Overlook {


class StatSlot : Moveable<StatSlot> {
	
	
public:
	
	void AddResult(double d, double d2) {av.Add(d); abs_av.Add(fabs(d2));}
	
	
	OnlineVariance av, abs_av;
};

class EventStatistics : public Common {
	
	
protected:
	friend class EventStatisticsCtrl;
	friend class EventConsole;
	friend class EventOptimization;
	
	
	enum {OPEN, BB5, BB10, BB15, BB20, BB30, BB40, BB50, BB60, NEWSNOW, SRC_COUNT};
	
	// Persistent
	Vector<Vector<Vector<StatSlot> > > stats;
	
	
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
	typedef EventStatistics CLASSNAME;
	EventStatistics();
	~EventStatistics();
	
	virtual void Init();
	virtual void Start();
	void UpdateEvents(int sym);
	void RefreshData();
	
	String GetDescription(int i);
	int GetSignal(int sym, int i, int src);
	int GetPreferredNet();
	int GetLatestSlotId();
	const StatSlot& GetLatestSlot(int net, int i);
	const StatSlot& GetSlot(int net, int i, int wb);
	
	ConstBuffer& GetBuffer(int sym, int src, int buf) {return *bufs[sym][src][buf];}
	
};

inline EventStatistics& GetEventStatistics() {return GetSystem().GetCommon<EventStatistics>();}


class EventStatisticsCtrl : public CommonCtrl {
	ArrayCtrl list;
	DropList symlist;
	Upp::Label activelbl;
	
public:
	typedef EventStatisticsCtrl CLASSNAME;
	EventStatisticsCtrl();
	
	virtual void Data();
	
};

}

#endif
