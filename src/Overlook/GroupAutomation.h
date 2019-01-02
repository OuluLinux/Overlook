#ifndef _Overlook_GroupAutomation_h_
#define _Overlook_GroupAutomation_h_

#if 0
namespace Overlook {
using namespace libmt;

struct GroupData : Moveable<GroupData> {
	Vector<VectorMap<int, OnlineAverage1> > data;
	
	
};

struct PatternMatcherGroupData : Moveable<PatternMatcherGroupData> {
	VectorMap<unsigned, GroupData> group_data;
	
	
};

class GroupAutomation : public Common {
	
protected:
	friend class GroupAutomationCtrl;
	
	static const int max_open_sym = 5;
	static const int close_pips = 10;
	
	
	// Temporary
	VectorMap<GroupSettings, PatternMatcherGroupData> data;
	CoreList cl_sym;
	Time prev_time;
	Mutex lock;
	
	void InitData(int pattern_period, int group_step, int average_period);
	int GetPreCode(int sym, int codesize, int pos);
	double GetPostCode(int sym, int pos);
	GroupData& GetGroup(const GroupSettings& gs, int pos);
	
	void RefreshData();
	double GetOpenLots(int sym);
	void SetRealSymbolLots(int sym_, double lots);
	
public:
	typedef GroupAutomation CLASSNAME;
	GroupAutomation();
	
	virtual void Init();
	virtual void Start();
	
};

inline GroupAutomation& GetGroupAutomation() {return GetSystem().GetCommon<GroupAutomation>();}

class GroupAutomationCtrl : public CommonCtrl {
	SliderCtrl slider;
	Upp::Label date;
	Splitter split;
	ArrayCtrl group_list, sym_list, data_list;
	
public:
	typedef GroupAutomationCtrl CLASSNAME;
	GroupAutomationCtrl();
	
	virtual void Data();
	
};


}

#endif
#endif
