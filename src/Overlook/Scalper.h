#ifndef _Overlook_Scalper_h_
#define _Overlook_Scalper_h_

namespace Overlook {


template <class T>
inline void CopyCC(T& dst, const T& src) {
	dst.SetCount(src.GetCount());
	for(int i = 0; i < dst.GetCount(); i++) {
		const auto& src_c = src[i];
		auto& dst_c = dst[i];
		dst_c.SetCount(src_c.GetCount());
		for(int j = 0; j < dst_c.GetCount(); j++) {
			dst_c[j] = src_c[j];
		}
	}
}

class ScalperConf : Moveable<ScalperConf> {
	
public:
	Vector<Vector<MatcherItem> > start_list, sust_list;
	double profit = -DBL_MAX, test_profit = -DBL_MAX;
	
public:
	typedef ScalperConf CLASSNAME;
	ScalperConf() {}
	ScalperConf(const ScalperConf& s) {*this = s;}
	void operator=(const ScalperConf& s) {
		CopyCC(start_list, s.start_list);
		CopyCC(sust_list, s.sust_list);
		profit = s.profit;
		test_profit = s.test_profit;
	}
	
	void Serialize(Stream& s) {s % start_list % sust_list % profit % test_profit;}
	
	bool operator()(const ScalperConf& a, const ScalperConf& b) const {
		return a.profit > b.profit;
	}
};

void UpdateEventVectors(LabelSource& ls);

class Scalper;

class ScalperSymbol : Moveable<ScalperSymbol> {
	
public:
	
	
	// Persistent
	SortedLimitedVectorMap<int, ScalperConf, ScalperConf> confs;
	VectorBool signal;
	int conf_counter = 0;
	int symbol = -1;
	bool type = false;
	
	
	// Temporary
	MatcherCache start_cache, sust_cache;
	
	
public:
	typedef ScalperSymbol CLASSNAME;
	ScalperSymbol() {}
	
	void Serialize(Stream& s) {s % confs % signal % conf_counter % symbol % type;}
	void Init();
	void Start();
	void Evolve();
	void Randomize(ScalperConf& sc);
	void Evolve(ScalperConf& sc);
	void Evaluate(ScalperConf& sc, bool write_signal);
	
	Callback InitCb() {return THISBACK(Init);}
	Callback StartCb() {return THISBACK(Start);}
	
	
	Scalper* s = NULL;
};

class Scalper {
	
protected:
	friend class ScalperCtrl;
	friend class ScalperSymbol;
	friend class ScalperViewer;
	
	
	// Persistent
	Vector<LabelSource> cache;
	VectorMap<int, ScalperSymbol> symbols;
	
	// Temporary
	VectorMap<int, DataBridge*> dbs;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Index<int> sym_ids, tf_ids;
	int sel_sym = -1, sel_conf = -1;
	bool running = false, stopped = true;
	
public:
	typedef Scalper CLASSNAME;
	Scalper();
	~Scalper();
	
	void Serialize(Stream& s) {s % cache % symbols;}
	void Process();
	void FillInputBooleans();
	void RefreshSymbolPointer() {for(int i = 0; i < symbols.GetCount(); i++) symbols[i].s = this;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("Scalper.bin")); RefreshSymbolPointer();}
	void StoreThis() {StoreToFile(*this, ConfigFile("Scalper.bin"));}
	bool IsRunning() const {return running && !Thread::IsShutdownThreads();}
	
	template <class T> void Add(int arg0=INT_MIN, int arg1=INT_MIN, int arg2=INT_MIN, int arg3=INT_MIN) {
		auto& a = indi_ids.Add().Set(GetSystem().Find<T>());
		if (arg0 != INT_MIN) a.AddArg(arg0);
		if (arg1 != INT_MIN) a.AddArg(arg1);
		if (arg2 != INT_MIN) a.AddArg(arg2);
		if (arg3 != INT_MIN) a.AddArg(arg3);
	}
	
};

inline Scalper& GetScalper() {return Single<Scalper>();}



class ScalperCtrl : public ParentCtrl {
	TabCtrl tabs;
	Splitter sustsplit;
	ArrayCtrl symbollist, conflist, startlist, sustlist;
	
public:
	typedef ScalperCtrl CLASSNAME;
	ScalperCtrl();
	
	void Data();
	
};

}

#endif
