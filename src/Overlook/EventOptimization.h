#ifndef _Overlook_EventOptimization_h_
#define _Overlook_EventOptimization_h_


namespace Overlook {


class EventOptimization : public Common {
	
	
protected:
	friend class EventOptimizationCtrl;
	friend class EventConsole;
	friend class EventOptimizationDrawer;
	
	// Persistent
	Optimizer opt;
	Vector<double> training_pts, last_round;
	
	
	// Temporary
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<DataBridge*> db;
	Index<int> sym_ids, tf_ids, fac_ids;
	Vector<double> point;
	ThreadSignal sig;
	SimBroker sb;
	
	
public:
	typedef EventOptimization CLASSNAME;
	EventOptimization();
	~EventOptimization();
	
	static const int grade_count = 5;
	
	virtual void Init();
	virtual void Start();
	
	void Process();
	
	void Serialize(Stream& s) {s % opt % training_pts % last_round;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("EventOptimization.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("EventOptimization.bin"));}
	
	bool IsRunning() const {return sig.IsRunning() && !Thread::IsShutdownThreads();}
	
};

inline EventOptimization& GetEventOptimization() {return GetSystem().GetCommon<EventOptimization>();}

class EventOptimizationDrawer : public Ctrl {
	Vector<Point> cache;
	
public:
	virtual void Paint(Draw& d);
	
};

class EventOptimizationCtrl : public CommonCtrl {
	EventOptimizationDrawer draw;
	ProgressIndicator prog;
	
public:
	typedef EventOptimizationCtrl CLASSNAME;
	EventOptimizationCtrl();
	
	virtual void Data();
	
	
};


}

#endif
