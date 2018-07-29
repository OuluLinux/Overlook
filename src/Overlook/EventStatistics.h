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
	
	
	enum {OPEN, HU4, HU8, HU16, MA3, MA9, MA27, BB, PSAR, PC, TB4, TB8, TB16, PEEKC5, PEEKC15, PEEKC30, SRC_COUNT};
	
	// Persistent
	Vector<Vector<StatSlot> > stats;
	
	
	// Temporary
	Index<String> symbols;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<Vector<ConstBuffer*> > bufs;
	Vector<ConstLabelSignal*> lbls;
	DataBridge* db = NULL;
	Index<int> sym_ids, tf_ids, fac_ids;
	int prev_bars = 0;
	bool running = false, stopped = true;
	
	
public:
	typedef EventStatistics CLASSNAME;
	EventStatistics();
	~EventStatistics();
	
	virtual void Init();
	virtual void Start();
	void UpdateEvents();
	
	String GetDescription(int i);
	int GetSignal(int i, int src);
	
	bool IsRunning() const {return running && !Thread::IsShutdownThreads();}
	
};

inline EventStatistics& GetEventStatistics() {return GetSystem().GetCommon<EventStatistics>();}


class EventStatisticsCtrl : public CommonCtrl {
	ArrayCtrl list;
	
	
public:
	EventStatisticsCtrl();
	
	virtual void Data();
	
};

}
#endif
