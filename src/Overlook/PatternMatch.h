#ifndef _Overlook_PatternMatch_h_
#define _Overlook_PatternMatch_h_

namespace Overlook {
using namespace libmt;



struct PatternMatcherData {
	Vector<OnlineAverageWindow1> dist_averages;
	Vector<OnlineAverageWindow1> absdist_averages;
	Vector<char> data;
	Vector<Point> pattern;
	int counted = 0;
	
	
};

struct PatternDistance : Moveable<PatternDistance> {
	int j0, j1, dist, absdist;
	
	bool operator()(const PatternDistance& a, const PatternDistance& b) const {
		return a.absdist < b.absdist;
	}
};

class PatternMatcher : public Common {
	
protected:
	friend class PatternMatcherCtrl;
	
	// Persistent
	ArrayMap<int, PatternMatcherData> data;
	
	
	// Temporary
	int count = 0;
	
	
public:
	typedef PatternMatcher CLASSNAME;
	PatternMatcher();
	
	virtual void Init();
	virtual void Start();
	
	PatternMatcherData& RefreshData(int tf, int group_step, int period, int average_period);
};

inline PatternMatcher& GetPatternMatcher() {return GetSystem().GetCommon<PatternMatcher>();}

class PatternMatcherCtrl : public CommonCtrl {
	Upp::Label period_lbl, group_step_lbl, average_period_lbl;
	EditIntSpin period, group_step, average_period;
	SliderCtrl slider;
	Upp::Label date;
	ArrayCtrl list;
	
public:
	typedef PatternMatcherCtrl CLASSNAME;
	PatternMatcherCtrl();
	
	virtual void Data();
	
};


}

#endif
