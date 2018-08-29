#ifndef _Overlook_FastEventOptimization_h_
#define _Overlook_FastEventOptimization_h_



namespace Overlook {


class FastEventOptimization : public Common {
	
	
protected:
	friend class FastEventOptimizationCtrl;
	friend class FastEventConsole;
	friend class FastEventOptimizationDrawer;
	
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
	typedef FastEventOptimization CLASSNAME;
	FastEventOptimization();
	~FastEventOptimization();
	
	static const int grade_count = 3;
	
	virtual void Init();
	virtual void Start();
	
	void Process();
	
	void Serialize(Stream& s) {s % opt % training_pts % last_round;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("FastEventOptimization.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("FastEventOptimization.bin"));}
	
	bool IsRunning() const {return sig.IsRunning() && !Thread::IsShutdownThreads();}
	
};

inline FastEventOptimization& GetFastEventOptimization() {return GetSystem().GetCommon<FastEventOptimization>();}

class FastEventOptimizationDrawer : public Ctrl {
	Vector<Point> cache;
	
public:
	virtual void Paint(Draw& d);
	
};

class FastEventOptimizationCtrl : public CommonCtrl {
	FastEventOptimizationDrawer draw;
	ProgressIndicator prog;
	
public:
	typedef FastEventOptimizationCtrl CLASSNAME;
	FastEventOptimizationCtrl();
	
	virtual void Data();
	
	
};


}
#endif
