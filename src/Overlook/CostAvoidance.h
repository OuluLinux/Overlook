#ifndef _Overlook_CostAvoidance_h_
#define _Overlook_CostAvoidance_h_

namespace Overlook {

struct Chance : Moveable<Chance> {
	Vector<int> caches;
	int symbol = -1, type = -1, pips = 0, len = 0;
	bool action = 0;
	String desc;
	
	void Serialize(Stream& s) {s % caches % symbol % type % pips % len % action % desc;}
};

class CostAvoidance {
	
	
protected:
	friend class CostAvoidanceCtrl;
	
	// Persistent
	Vector<LabelSource> cache;
	VectorMap<int, AnalyzerSymbol> symbols;
	Array<Chance> chances;
	
	// Temporary
	Vector<FactoryDeclaration> indi_ids;
	Vector<DataBridge*> db_cores;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<String> sym_id_str;
	Index<int> sym_ids, tf_ids;
	int sel_sym = -1, sel_cluster = -1;
	bool running = false, stopped = true;
	
public:
	typedef CostAvoidance CLASSNAME;
	CostAvoidance();
	~CostAvoidance();
	
	void Process();
	void FillInputBooleans();
	void FillChances();
	void LoadThis() {LoadFromFile(*this, ConfigFile("CostAvoidance.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("CostAvoidance.bin"));}
	bool IsRunning() const {return running && !Thread::IsShutdownThreads();}
	void Serialize(Stream& s) {s % cache % symbols % chances;}
	
	template <class T> void Add(int arg0=INT_MIN, int arg1=INT_MIN, int arg2=INT_MIN, int arg3=INT_MIN) {
		auto& a = indi_ids.Add().Set(GetSystem().Find<T>());
		if (arg0 != INT_MIN) a.AddArg(arg0);
		if (arg1 != INT_MIN) a.AddArg(arg1);
		if (arg2 != INT_MIN) a.AddArg(arg2);
		if (arg3 != INT_MIN) a.AddArg(arg3);
	}
};

inline CostAvoidance& GetCostAvoidance() {return Single<CostAvoidance>();}

class CostAvoidanceCtrl : public ParentCtrl {
	ArrayCtrl currentlist;
	
public:
	typedef CostAvoidanceCtrl CLASSNAME;
	CostAvoidanceCtrl();
	
	void Data();
	
};

}

#endif
