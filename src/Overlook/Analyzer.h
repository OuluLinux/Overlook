#ifndef _Overlook_Analyzer_h_
#define _Overlook_Analyzer_h_

namespace Overlook {

class AnalyzerOrder : Moveable<AnalyzerOrder> {
	
public:
	int symbol;
	int begin, end;
	bool action = 0;
	double lots = 0.0;
	VectorBool descriptor, start_descriptor, sust_descriptor, stop_descriptor;
	
public:
	typedef AnalyzerOrder CLASSNAME;
	AnalyzerOrder();
	
	void Serialize(Stream& s) {s % symbol % begin % end % action % lots % descriptor % start_descriptor % sust_descriptor % stop_descriptor;}
	
	bool operator() (const AnalyzerOrder& a, const AnalyzerOrder& b) const {
		return a.begin < b.begin;
	}
};

class AnalyzerCluster : Moveable<AnalyzerCluster> {
	
public:
	VectorBool av_start_descriptor;
	Vector<int> orders;
	VectorMap<int, int> stats;
	int total = 0;
	
public:
	typedef AnalyzerCluster CLASSNAME;
	AnalyzerCluster() {}
	
	void Add() {total++;}
	void AddEvent(int id) {stats.GetAdd(id, 0)++;}
	void Sort() {SortByValue(stats, StdGreater<int>());}
	
	void Serialize(Stream& s) {s % av_start_descriptor % orders % stats % total;}
	
};

class RtData : Moveable<RtData> {
	
public:
	int cluster_long = -1, cluster_short = -1;
	
public:
	typedef RtData CLASSNAME;
	RtData() {}
	
	void Serialize(Stream& s) {s % cluster_long % cluster_short;}
	
	
};

class Analyzer {
	
protected:
	friend class AnalyzerCtrl;
	
	struct LabelSource : Moveable<LabelSource> {
		int factory = -1, label = -1, buffer = -1;
		LabelSignal data;
		String title;
		
		void Serialize(Stream& s) {s % factory % label % buffer % data % title;}
	};
	
	enum {
		START_ENABLE,
		START_DISABLE,
		START_POS,
		START_NEG,
		SUSTAIN_ENABLE,
		SUSTAIN_DISABLE,
		SUSTAIN_POS,
		SUSTAIN_NEG,
		STOP_ENABLE,
		STOP_DISABLE,
		STOP_POS,
		STOP_NEG,
		EVENT_COUNT
	};
	
	static const int CLUSTER_COUNT = 100;
	
	
	// Persistent
	Vector<LabelSource> cache;
	Vector<AnalyzerOrder> orders;
	Vector<AnalyzerCluster> clusters;
	Vector<RtData> rtdata;
	VectorMap<int, LabelSignal> scalper_signal;
	int rtcluster_counted = 0;
	
	// Temporary
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Index<int> sym_ids, tf_ids;
	bool running = false, stopped = true;
	
public:
	typedef Analyzer CLASSNAME;
	Analyzer();
	~Analyzer();
	
	void Serialize(Stream& s) {s % cache % orders % clusters % rtdata % scalper_signal % rtcluster_counted;}
	void Process();
	void FillOrdersScalper();
	void FillOrdersMyfxbook();
	void FillInputBooleans();
	void InitOrderDescriptor();
	void InitClusters();
	void RefreshRealtimeClusters();
	void Analyze(AnalyzerCluster& am);
	//void Optimize(int cluster);
	//void Test(AnalyzerSolution& am);
	String GetEventString(int i);
	void GetRealtimeStartDescriptor(bool label, int i, VectorBool& descriptor);
	//bool Match(const AnalyzerMatcher& am, const AnalyzerOrder& o);
	int  GetEvent(const AnalyzerOrder& o, const LabelSource& ls);
	int  GetEventBegin(bool label, int begin, const LabelSource& ls);
	int  GetEventSustain(bool label, int begin, int end, const LabelSource& ls);
	int  GetEventEnd(bool label, int end, const LabelSource& ls);
	int  FindClosestCluster(int type, const VectorBool& descriptor);
	void LoadThis() {LoadFromFile(*this, ConfigFile("Analyzer.bin"));}
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
	Splitter cluster, realtime;
	ArrayCtrl clusterlist, eventlist, rtlist, rtdata;
	
public:
	typedef AnalyzerCtrl CLASSNAME;
	AnalyzerCtrl();
	
	void Data();
	
};


}

#endif
