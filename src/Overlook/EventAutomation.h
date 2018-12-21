#ifndef _Overlook_EventAutomation_h_
#define _Overlook_EventAutomation_h_


namespace Overlook {
using namespace libmt;

struct GroupSettings : Moveable<GroupSettings> {
	int period, group_step, average_period;
	
	void Serialize(Stream& s) {s % period % group_step % average_period;}
	bool operator==(const GroupSettings& gs) const {return gs.period == period && gs.group_step == group_step && gs.average_period == average_period;}
	unsigned GetHashValue() const {return CombineHash(period, group_step, average_period);}
};

struct UniqueGroup : Moveable<UniqueGroup> {
	Index<byte> symbols;
	Index<GroupSettings> settings;
	
	void Serialize(Stream& s) {s % symbols % settings;}
	bool operator()(const UniqueGroup& a, const UniqueGroup& b) const {return a.settings.GetCount() > b.settings.GetCount();}
};

struct EventOptResult : Moveable<EventOptResult> {
	Vector<int> cis, signals;
	double neg_first_prob, neg_first_confidence;
	int sym = 0;
};

class EventAutomation : public Common {
	
protected:
	friend class EventAutomationCtrl;
	
	static const int pips_first = 10;
	
	
	// Persistency
	VectorMap<GroupSettings, VectorMap<unsigned, VectorBool> > group_data;
	VectorMap<GroupSettings, Vector<unsigned> > current_groups;
	Vector<VectorMap<int, OnlineAverage1> > data;
	Vector<VectorMap<int, VectorBool> > simple_data;
	VectorMap<unsigned, UniqueGroup> unique_groups;
	VectorMap<GroupSettings, int> group_counted;
	Vector<VectorBool> sig_whichfirst;
	Vector<byte> current_signals;
	int counted = 100;
	
	
	// Temporary
	Vector<Vector<int> > symbol_simple_datas;
	Vector<Ptr<EventCoreItem> > ci_queue;
	Vector<EventOptResult> opt_results;
	CoreList cl_sym;
	Time prev_update;
	bool do_store = false;
	
	
public:
	typedef EventAutomation CLASSNAME;
	EventAutomation();
	
	virtual void Init();
	virtual void Start();
	void Process();
	void RefreshCachedSignals();
	void RefreshGroups();
	void RefreshSimpleEvents();
	void RefreshCurrentComplexEvents();
	
	int GetSignalWhichFirst(int sym, int pos, int pips);
	
	void LoadThis() {LoadFromFile(*this, GetOverlookFile("EventAutomation.bin"));}
	void StoreThis() {StoreToFile(*this, GetOverlookFile("EventAutomation.bin"));}
	void Serialize(Stream& s) {
		s % group_data % current_groups % data % simple_data % unique_groups % group_counted
		  % sig_whichfirst % current_signals % counted;
	}
	
};

inline EventAutomation& GetEventAutomation() {return GetSystem().GetCommon<EventAutomation>();}

class EventAutomationCtrl : public CommonCtrl {
	TabCtrl tabs;
	
	ArrayCtrl curopt;
	
	ParentCtrl curdata;
	SliderCtrl slider;
	Upp::Label date;
	ArrayCtrl list;
	
	Splitter uniquesplit;
	ArrayCtrl uniquegroups, uniquesettings;
	
public:
	typedef EventAutomationCtrl CLASSNAME;
	EventAutomationCtrl();
	
	virtual void Data();
	
};


}

#endif
