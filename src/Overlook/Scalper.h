#ifndef _Overlook_Scalper_h_
#define _Overlook_Scalper_h_

namespace Overlook {




class ScalperConf : Moveable<ScalperConf> {
	
public:
	Vector<Vector<MatcherItem> > start_list, sust_list;
	
public:
	typedef ScalperConf CLASSNAME;
	ScalperConf() {}
	
	void Serialize(Stream& s) {s % start_list % sust_list;}
	
};

void UpdateEventVectors(LabelSource& ls);

class Scalper;

class ScalperSymbol : Moveable<ScalperSymbol> {
	
public:
	
	
	// Persistent
	Vector<ScalperConf> confs;
	VectorBool signal;
	bool type = false;
	
	
public:
	typedef ScalperSymbol CLASSNAME;
	ScalperSymbol() {}
	
	void Serialize(Stream& s) {s % confs % signal % type;}
	void Init();
	
	Callback InitCb() {return THISBACK(Init);}
	
	
	Scalper* a = NULL;
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
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Index<int> sym_ids, tf_ids;
	int sel_sym = -1, sel_cluster = -1;
	bool running = false, stopped = true;
	
public:
	typedef Scalper CLASSNAME;
	Scalper();
	~Scalper();
	
	void Serialize(Stream& s) {s % cache % symbols;}
	void Process();
	void FillInputBooleans();
	void RefreshSymbolPointer() {for(int i = 0; i < symbols.GetCount(); i++) symbols[i].a = this;}
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
