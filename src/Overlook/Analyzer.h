#ifndef _Overlook_Analyzer_h_
#define _Overlook_Analyzer_h_

namespace Overlook {
	
enum {
	SUSTAIN_ENABLE,
	SUSTAIN_DISABLE,
	SUSTAIN_POS_LONG,
	SUSTAIN_NEG_LONG,
	SUSTAIN_POS_SHORT,
	SUSTAIN_NEG_SHORT,
	EVENT_COUNT
};

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
	int cache = 0, event = 0, key = 0;
	
	String ToString() const {return Format("cache=%d, event=%d, key=%d", cache, event, key);}
	void Serialize(Stream& s) {s % cache % event % key;}
};

struct MatcherCache {
	VectorBool matcher_and, matcher_or;
};

struct LabelSource : Moveable<LabelSource> {
	int factory = -1, label = -1, buffer = -1;
	LabelSignal data;
	VectorBool eventdata[EVENT_COUNT];
	String title;
	
	void Serialize(Stream& s) {s % factory % label % buffer % data % title; for(int i = 0; i < EVENT_COUNT; i++) s % eventdata[i];}
};

class AnalyzerCluster : Moveable<AnalyzerCluster> {
	
public:
	Vector<Vector<MatcherItem> > full_list;
	VectorBool av_descriptor;
	Vector<int> orders;
	VectorMap<int, int> stats;
	Vector<MatchTest> test_results;
	VectorBool closest_mask, match_mask;
	LabelSource scalper_signal;
	int total = 0;
	
public:
	typedef AnalyzerCluster CLASSNAME;
	AnalyzerCluster() {}
	
	void Add() {total++;}
	void AddEvent(int id) {stats.GetAdd(id, 0)++;}
	void Sort() {SortByValue(stats, StdGreater<int>());}
	
	void Serialize(Stream& s) {s % full_list % av_descriptor % orders % stats % test_results % closest_mask % match_mask % total % scalper_signal;}
	
};

void UpdateEventVectors(LabelSource& ls);

class Analyzer;

class AnalyzerSymbol : Moveable<AnalyzerSymbol> {
	
public:
	
	static const int CLUSTER_COUNT = 10;
	
	
	// Persistent
	Vector<int> rtdata;
	Vector<AnalyzerOrder> orders;
	Vector<AnalyzerCluster> clusters;
	LabelSource scalper_signal;
	int rtcluster_counted = 0;
	bool type = false;
	
	
public:
	typedef AnalyzerSymbol CLASSNAME;
	AnalyzerSymbol() {}
	
	void Serialize(Stream& s) {s % rtdata % orders % clusters % scalper_signal % rtcluster_counted % type;}
	void InitScalperSignal(ConstLabelSignal& scalper_sig, bool type, const Vector<AnalyzerOrder>& orders);
	void Init();
	void InitOrderDescriptor();
	void InitClusters();
	void InitMatchers();
	void InitMatchers(int cluster);
	void Analyze(AnalyzerCluster& am);
	void RunMatchTest(const Vector<MatcherItem>& list, MatchTest& t, MatcherCache& mcache, AnalyzerCluster& c);
	void RunMatchTest(const Vector<Vector<MatcherItem> >& list, MatchTest& t, MatcherCache& mcache, AnalyzerCluster& c);
	void InitMatchersCluster(AnalyzerCluster& c);
	void RefreshRealtimeClusters();
	void GetRealtimeDescriptor(bool label, int i, VectorBool& descriptor);
	int  FindClosestCluster(int type, const VectorBool& descriptor);
	
	Callback InitCb() {return THISBACK(Init);}
	
	
	Analyzer* a = NULL;
};

class Analyzer {
	
protected:
	friend class AnalyzerCtrl;
	friend class AnalyzerSymbol;
	friend class AnalyzerViewer;
	
	
	// Persistent
	Vector<LabelSource> cache;
	VectorMap<int, AnalyzerSymbol> symbols;
	
	// Temporary
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Index<int> sym_ids, tf_ids;
	int sel_sym = -1, sel_cluster = -1;
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
	int  GetEvent(const AnalyzerOrder& o, const LabelSource& ls);
	//int  GetEventBegin(bool label, int begin, const LabelSource& ls);
	int  GetEventSustain(bool label, int begin, int end, const LabelSource& ls);
	//int  GetEventEnd(bool label, int end, const LabelSource& ls);
	void RefreshSymbolPointer() {for(int i = 0; i < symbols.GetCount(); i++) symbols[i].a = this;}
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
	Splitter sustsplit;
	ArrayCtrl symbollist, clusterlist, eventlist, sustandlist, sustlist;
	
public:
	typedef AnalyzerCtrl CLASSNAME;
	AnalyzerCtrl();
	
	void Data();
	
};


}

#endif
