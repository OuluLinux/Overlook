#ifndef _Overlook_Analyzer_h_
#define _Overlook_Analyzer_h_

namespace Overlook {

class AnalyzerOrder : Moveable<AnalyzerOrder> {
	
public:
	int symbol;
	int begin, end;
	bool action = 0;
	double lots = 0.0;
	
public:
	typedef AnalyzerOrder CLASSNAME;
	AnalyzerOrder();
	
	void Serialize(Stream& s) {s % symbol % begin % end % action % lots;}
	
	bool operator() (const AnalyzerOrder& a, const AnalyzerOrder& b) const {
		return a.begin < b.begin;
	}
};

struct Match : Moveable<Match> {
	int cache = -1, event = -1, row = -1;
	
	void Serialize(Stream& s) {s % cache % event % row;}
};

class AnalyzerSolution : Moveable<AnalyzerSolution> {
	
public:
	Vector<Match> required;
	double result;
	int symbol = -1;
	
public:
	typedef AnalyzerSolution CLASSNAME;
	AnalyzerSolution() {}
	
	void Serialize(Stream& s) {s % required % result % symbol;}
	
	bool operator() (const AnalyzerSolution& a, const AnalyzerSolution& b) const {
		return a.result > b.result;
	}
};

class AnalyzerMatcher : Moveable<AnalyzerMatcher> {
	
public:
	VectorMap<int, int> stats;
	Array<Vector<AnalyzerSolution> > solutions;
	int total = 0;
	int symbol = -1;
	
public:
	typedef AnalyzerMatcher CLASSNAME;
	AnalyzerMatcher() {}
	
	void Serialize(Stream& s) {s % stats % solutions % total % symbol;}
	void Add() {total++;}
	void AddEvent(int id) {stats.GetAdd(id, 0)++;}
	void Sort() {SortByValue(stats, StdGreater<int>());}
	
	String GetRequiredString() const {return IntStr(solutions.GetCount());}
	
};

class Analyzer {
	
protected:
	friend class AnalyzerCtrl;
	
	struct LabelSource : Moveable<LabelSource> {
		int factory = -1, label = -1, buffer = -1;
		LabelSignal data;
		String title;
		
		void Serialize(Stream& s) {s % factory % label % buffer % data;}
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
	
	
	// Persistent
	Vector<LabelSource> cache;
	Vector<AnalyzerOrder> orders;
	Vector<AnalyzerMatcher> matchers;
	VectorMap<int, LabelSignal> scalper_signal;
	
	// Temporary
	Vector<FactoryDeclaration> indi_ids;
	Index<int> sym_ids, tf_ids;
	bool running = false, stopped = true;
	
public:
	typedef Analyzer CLASSNAME;
	Analyzer();
	~Analyzer();
	
	void Serialize(Stream& s) {s % cache % orders % matchers % scalper_signal;}
	void Process();
	void FillOrdersScalper();
	void FillOrdersMyfxbook();
	void FillInputBooleans();
	void InitMatchers();
	void Analyze(AnalyzerMatcher& am);
	void Optimize(int matcher);
	void Test(AnalyzerSolution& am);
	String GetEventString(int i);
	bool Match(const AnalyzerMatcher& am, const AnalyzerOrder& o);
	int  GetEvent(const AnalyzerOrder& o, const LabelSource& ls);
	void LoadThis() {LoadFromFile(*this, ConfigFile("Analyzer.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("Analyzer.bin"));}
	
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
	Splitter split;
	ArrayCtrl matcherlist, eventlist;
	
public:
	typedef AnalyzerCtrl CLASSNAME;
	AnalyzerCtrl();
	
	void Data();
	
};


}

#endif
