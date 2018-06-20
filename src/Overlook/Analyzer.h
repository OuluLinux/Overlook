#ifndef _Overlook_Analyzer_h_
#define _Overlook_Analyzer_h_

namespace Overlook {
	
enum {
	START_ENABLE,
	START_DISABLE,
	START_POS_LONG,
	START_NEG_LONG,
	START_POS_SHORT,
	START_NEG_SHORT,
	SUSTAIN_ENABLE,
	SUSTAIN_DISABLE,
	SUSTAIN_POS_LONG,
	SUSTAIN_NEG_LONG,
	SUSTAIN_POS_SHORT,
	SUSTAIN_NEG_SHORT,
	STOP_ENABLE,
	STOP_DISABLE,
	STOP_POS_LONG,
	STOP_NEG_LONG,
	STOP_POS_SHORT,
	STOP_NEG_SHORT,
	EVENT_COUNT
};

enum {EVT_START, EVT_SUST, EVT_STOP};

inline int GetEventType(int i) {return i < SUSTAIN_ENABLE ? EVT_START : (i < STOP_ENABLE ? EVT_SUST : EVT_STOP);}
String GetEventString(int i);

class AnalyzerOrder : Moveable<AnalyzerOrder> {
	
public:
	int symbol;
	int begin, end;
	bool action = 0;
	double lots = 0.0;
	VectorBool descriptor;
	
public:
	typedef AnalyzerOrder CLASSNAME;
	AnalyzerOrder();
	
	void Serialize(Stream& s) {s % symbol % begin % end % action % lots % descriptor;}
	
	bool operator() (const AnalyzerOrder& a, const AnalyzerOrder& b) const {
		return a.begin < b.begin;
	}
};

struct MatchTest : Moveable<MatchTest> {
	int false_match = 0;
	int false_total = 0;
	int true_match = 0;
	int true_total = 0;
	
	void Serialize(Stream& s) {s % false_match % false_total % true_match % true_total;}
	
	bool operator()(const MatchTest& a, const MatchTest& b) const {
		if (a.false_match == 0 && b.false_match == 0) return a.true_match > b.true_match;
		if (a.false_match == 0 && b.false_match > 0) return true;
		return a.false_match < b.false_match;
	}
};

struct MatcherItem : Moveable<MatcherItem> {
	int cache = 0, event = 0;
};

struct MatcherCache {
	VectorBool matcher_and, matcher_or;
};

class AnalyzerCluster : Moveable<AnalyzerCluster> {
	
public:
	BeamSearchOptimizer start_optimizer;
	VectorBool av_descriptor;
	Vector<int> orders;
	VectorMap<int, int> stats;
	Vector<MatchTest> test_results;
	int total = 0;
	
public:
	typedef AnalyzerCluster CLASSNAME;
	AnalyzerCluster() {}
	
	void Add() {total++;}
	void AddEvent(int id) {stats.GetAdd(id, 0)++;}
	void Sort() {SortByValue(stats, StdGreater<int>());}
	
	void Serialize(Stream& s) {s % start_optimizer % av_descriptor % orders % stats % test_results % total;}
	
};

struct LabelSource : Moveable<LabelSource> {
	int factory = -1, label = -1, buffer = -1;
	LabelSignal data;
	VectorBool eventdata[EVENT_COUNT];
	String title;
	
	void Serialize(Stream& s) {s % factory % label % buffer % data % title; for(int i = 0; i < EVENT_COUNT; i++) s % eventdata[i];}
};

void UpdateEventVectors(LabelSource& ls);

class Analyzer;

class AnalyzerSymbol : Moveable<AnalyzerSymbol> {
	
public:
	
	static const int CLUSTER_COUNT = 10;
	
	
	// Persistent
	Vector<AnalyzerOrder> orders;
	Vector<AnalyzerCluster> clusters;
	LabelSource scalper_signal;
	bool type = false;
	
	
public:
	typedef AnalyzerSymbol CLASSNAME;
	AnalyzerSymbol() {}
	
	void Serialize(Stream& s) {s % orders % clusters % scalper_signal % type;}
	void InitScalperSignal(ConstLabelSignal& scalper_sig, bool type, const Vector<AnalyzerOrder>& orders);
	void Init();
	void InitOrderDescriptor();
	void InitClusters();
	void InitStartMatchers();
	void InitStartMatchers(int cluster);
	void Analyze(AnalyzerCluster& am);
	void RunMatchTest(int type, const Vector<MatcherItem>& list, MatchTest& t, MatcherCache& mcache, AnalyzerCluster& c);
	
	Callback InitCb() {return THISBACK(Init);}
	
	
	Analyzer* a = NULL;
};

class Analyzer {
	
protected:
	friend class AnalyzerCtrl;
	friend class AnalyzerSymbol;
	
	
	// Persistent
	Vector<LabelSource> cache;
	VectorMap<int, AnalyzerSymbol> symbols;
	
	// Temporary
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Index<int> sym_ids, tf_ids;
	bool running = false, stopped = true;
	
public:
	typedef Analyzer CLASSNAME;
	Analyzer();
	~Analyzer();
	
	void Serialize(Stream& s) {s % cache % symbols;}
	void Process();
	void FillOrdersScalper();
	void FillOrdersMyfxbook();
	void FillInputBooleans();
	//void RefreshRealtimeClusters();
	void GetRealtimeStartDescriptor(bool label, int i, VectorBool& descriptor);
	int  GetEvent(const AnalyzerOrder& o, const LabelSource& ls);
	int  GetEventBegin(bool label, int begin, const LabelSource& ls);
	int  GetEventSustain(bool label, int begin, int end, const LabelSource& ls);
	int  GetEventEnd(bool label, int end, const LabelSource& ls);
	void RefreshSymbolPointer() {for(int i = 0; i < symbols.GetCount(); i++) symbols[i].a = this;}
	//int  FindClosestCluster(int type, const VectorBool& descriptor);
	void LoadThis() {LoadFromFile(*this, ConfigFile("Analyzer.bin")); RefreshSymbolPointer();}
	void StoreThis() {StoreToFile(*this, ConfigFile("Analyzer.bin"));}
	bool IsRunning() const {return running && !Thread::IsShutdownThreads();}
	
	template <class T> void Add(int arg0=INT_MIN, int arg1=INT_MIN, int arg2=INT_MIN, int arg3=INT_MIN) {
		auto& a = indi_ids.Add().Set(GetSystem().Find<T>());
		if (arg0 != INT_MIN) a.AddArg(arg0);
		if (arg1 != INT_MIN) a.AddArg(arg1);
		if (arg2 != INT_MIN) a.AddArg(arg2);
		if (arg3 != INT_MIN) a.AddArg(arg3);
	}
	
};

inline Analyzer& GetAnalyzer() {return Single<Analyzer>();}



class AnalyzerCtrl : public ParentCtrl {
	TabCtrl tabs;
	Splitter cluster, minmatchers;
	ArrayCtrl symbollist, clusterlist, eventlist;
	
public:
	typedef AnalyzerCtrl CLASSNAME;
	AnalyzerCtrl();
	
	void Data();
	
};


}

#endif
