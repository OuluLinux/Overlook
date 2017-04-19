#ifdef flagDATACORE_ONLY
#include "DataCore.h"

using namespace Upp;
using namespace DataCore;

CONSOLE_APP_MAIN {
	//MemoryBreakpoint(1009);
	TimeVector::Chk();
	
	TimeVector tv;
	
	tv.EnableCache();
	tv.LimitMemory(2*sizeof(double)*4*3*100);
	
	// Init sym/tfs/time space
	tv.AddSymbol("dummy 1");
	tv.AddSymbol("dummy 2");
	tv.AddSymbol("dummy 3");
	
	tv.SetBasePeriod(60*60);
	
	tv.AddPeriod(1		*60*60);
	tv.AddPeriod(4		*60*60);
	tv.AddPeriod(24		*60*60);
	tv.AddPeriod(7*24	*60*60);
	
	tv.SetBegin(Time(2014, 9, 1));
	Date now = GetSysTime();
	do {++now;}
	while (now.day != 15);
	tv.SetEnd(Time(now.year, now.month, now.day));
	
	tv.LinkPath("/open", "/dummyvalue");
	tv.LinkPath("/indi", "/dummyindicator");
	tv.LinkPath("/ma0", "/ma?method=0");
	tv.LinkPath("/ma1", "/ma?method=1");
	tv.LinkPath("/ma2", "/ma?method=2");
	tv.LinkPath("/ma3", "/ma?method=3");
	
	tv.RefreshData();
	
	while (1) {
		bool unchanged = true;
		for (TimeVector::Iterator it = tv.Begin(); !it.IsEnd(); it++) {
		
			if (it.Process()) {
				unchanged = false;
			}
			
		}
		if (unchanged) break;
	}
	
	
	LOG("Exiting successfully");
}

#endif
