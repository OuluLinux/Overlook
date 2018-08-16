#ifndef _Overlook_EventStatistics_h_
#define _Overlook_EventStatistics_h_

namespace Overlook {


class StatSlot : Moveable<StatSlot> {
	
	
public:
	
	void AddResult(double d) {av.Add(d);}
	
	
	OnlineVariance av;
};

class EventStatistics : public Common {
	
	
protected:
	friend class EventStatisticsCtrl;
	friend class EventConsole;
	friend class EventOptimization;
	
	
	enum {OPEN, /*HU4, HU8, HU16, MA3, MA9, MA27,*/ BB5, BB10, BB20, BB40, /*PSAR, PC, TB4, TB8, TB16, PEEKC5,*/ /*PEEKC15, PEEKC30,*/ NEWSNOW, SRC_COUNT};
	
	// Persistent
	Vector<Vector<Vector<StatSlot> > > stats;
	
	
	// Temporary
	Index<String> symbols;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<Vector<Vector<ConstBuffer*> > > bufs;
	Vector<Vector<ConstLabelSignal*> > lbls;
	Vector<DataBridge*> db, db_m1;
	Index<int> sym_ids, tf_ids, fac_ids;
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
