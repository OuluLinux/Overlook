#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_

namespace Overlook {

class Agent : public Core {
	QueryTable qt;
	
public:
	Agent();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InSlower()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

}

#endif
