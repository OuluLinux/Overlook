#ifndef _Overlook_EventAutomation_h_
#define _Overlook_EventAutomation_h_

#if 0

namespace Overlook {
using namespace libmt;

struct GroupSettings : Moveable<GroupSettings> {
	int period, group_step, average_period;
	
	void Serialize(Stream& s) {s % period % group_step % average_period;}
	bool operator==(const GroupSettings& gs) const {return gs.period == period && gs.group_step == group_step && gs.average_period == average_period;}
	unsigned GetHashValue() const {return CombineHash(period, group_step, average_period);}
};

struct UniqueGroup : Moveable<UniqueGroup> {
	Index<char> symbols;
	Index<GroupSettings> settings;
	
	void Serialize(Stream& s) {s % symbols % settings;}
	bool operator()(const UniqueGroup& a, const UniqueGroup& b) const {return a.settings.GetCount() > b.settings.GetCount();}
};

struct EventOptResult : Moveable<EventOptResult> {
	Vector<int> cis, signals;
	double neg_first_prob, neg_first_confidence;
	int popcount = 0;
	int sym = 0;
	
	bool operator()(const EventOptResult& a, const EventOptResult& b) const {return a.neg_first_confidence > b.neg_first_confidence;}
};

struct BestEventOptSettings {
	int min_popcount = 100, whichfirst_pips = -1;
	double min_confidence = 0.0;
	
	void Serialize(Stream& s) {s % min_popcount % whichfirst_pips % min_confidence;}
};

class EventAutomation : public Common {
	
protected:
	friend class EventAutomationCtrl;
	
	
	
	// Persistency
	VectorMap<GroupSettings, VectorMap<unsigned, VectorBool> > group_data;
	Vector<VectorMap<int, OnlineAverage1> > data;
	Vector<VectorMap<int, VectorBool> > simple_data;
	VectorMap<unsigned, UniqueGroup> unique_groups;
	VectorMap<GroupSettings, int> group_counted;
	Vector<VectorBool> sig_whichfirst;
	BestEventOptSettings opt_settings;
	int counted = 100;
	bool is_optimized = false;
	
	
	// Temporary
	Vector<Vector<int> > symbol_simple_datas;
	Vector<Ptr<ScriptCoreItem> > ci_queue;
	Vector<EventOptResult> results;
	Vector<double> test_vector;
	CoreList cl_sym;
	Time prev_update;
	double best_opt_result;
	double opt_test_result = 0;
	int opt_total = 0, opt_actual = 0;
	bool do_store = false;
	
	
public:
	typedef EventAutomation CLASSNAME;
	EventAutomation();
	
	virtual void Init();
	virtual void Start();
	void Process();
	void ClearCachedSignals();
	void RefreshCachedSignals(int pips);
	void RefreshGroups();
	void RefreshSimpleEvents();
	void GetCurrentComplexEvents(int pos, int min_samplecount, Vector<EventOptResult>& results);
	
	void GetCurrentGroups(int pos, VectorMap<GroupSettings, Vector<unsigned> >& current_groups);
	void GetCurrentSignals(int pos, Vector<byte>& current_signals);
	int GetSignalWhichFirst(int sym, int pos, int pips);
	double GetTestResult(int test_size, int min_popcount, double min_confidence, int whichfirst_pips);
	
	void LoadThis() {LoadFromFile(*this, GetOverlookFile("EventAutomation.bin"));}
	void StoreThis() {StoreToFile(*this, GetOverlookFile("EventAutomation.bin"));}
	void Serialize(Stream& s) {
		s % group_data % data % simple_data % unique_groups % group_counted
		  % sig_whichfirst % opt_settings % counted % is_optimized;
	}
	
};

inline EventAutomation& GetEventAutomation() {return GetSystem().GetCommon<EventAutomation>();}

class EventAutomationCtrl : public CommonCtrl {
	
	struct TestCtrl : public Ctrl {
		Vector<Point> cache;
		virtual void Paint(Draw& d) {
			d.DrawRect(GetSize(), White());
			DrawVectorPolyline(d, GetSize(), GetEventAutomation().test_vector, cache);
		}
	};
	
	TabCtrl tabs;
	
	ArrayCtrl curopt;
	
	ParentCtrl curdata;
	SliderCtrl slider;
	Upp::Label date;
	ArrayCtrl list;
	
	Splitter uniquesplit;
	ArrayCtrl uniquegroups, uniquesettings;
	
	ParentCtrl optctrl;
	ProgressIndicator optprog;
	ArrayCtrl optlist;
	
	TestCtrl test;
	
public:
	typedef EventAutomationCtrl CLASSNAME;
	EventAutomationCtrl();
	
	virtual void Data();
	
};


}

#endif
#endif
