#ifndef _Overlook_PatternMatch_h_
#define _Overlook_PatternMatch_h_

namespace Overlook {
using namespace libmt;

struct PatternGroup : Moveable<PatternGroup> {
	Vector<int> symbols;
	
};

struct PatternMatcherData {
	Vector<Vector<PatternGroup> > data;
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
	CoreList cl_sym;
	int count = 0;
	
	
public:
	typedef PatternMatcher CLASSNAME;
	PatternMatcher();
	
	virtual void Init();
	virtual void Start();
	
	PatternMatcherData& RefreshData(int group_step, int period);
};

inline PatternMatcher& GetPatternMatcher() {return GetSystem().GetCommon<PatternMatcher>();}

class PatternMatcherCtrl : public CommonCtrl {
	EditIntSpin period, group_step;
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
