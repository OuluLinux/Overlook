#ifndef _Overlook_Advisors_h_
#define _Overlook_Advisors_h_


namespace Overlook {


class SimpleBB : public Advisor {
	int wait_cursor = 0;
	double init_balance = 0;
	
public:
	SimpleBB();
	virtual void Tick();
	
	
};


}

#endif
