#ifndef _Overlook_Extrapolation_h_
#define _Overlook_Extrapolation_h_

namespace Overlook {


class NeverTheSameExtrapolation : public ScriptCore {
	int symbol = 0;
	
public:
	virtual void Run();
};


}

#endif
