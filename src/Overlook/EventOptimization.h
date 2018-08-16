#ifndef _Overlook_EventOptimization_h_
#define _Overlook_EventOptimization_h_


namespace Overlook {


class EventOptimization : public Common {
	
	
protected:
	friend class EventOptimizationCtrl;
	
	// Persistent
	Optimizer opt;
	Vector<double> training_pts;
	
	
	// Temporary
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<DataBridge*> db;
	Index<int> sym_ids, tf_ids, fac_ids;
	ThreadSignal sig;
	SimBroker sb;
	
	
public:
	typedef EventOptimization CLASSNAME;
	EventOptimization();
	~EventOptimization();
	
	virtual void Init();
	virtual void Start();
	
	void Process();
	
	void Serialize(Stream& s) {s % opt % training_pts;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("EventOptimization.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("EventOptimization.bin"));}
	
	bool IsRunning() const {return sig.IsRunning() && !Thread::IsShutdownThreads();}
	
};

inline EventOptimization& GetEventOptimization() {return GetSystem().GetCommon<EventOptimization>();}


class EventOptimizationCtrl : public CommonCtrl {
	Splitter hsplit;
	
public:
	typedef EventOptimizationCtrl CLASSNAME;
	EventOptimizationCtrl();
	
	virtual void Data();
	
	
};


}

#endif
