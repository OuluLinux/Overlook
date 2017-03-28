#include "Overlook.h"

#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>

void InitTimeVector() {
	TimeVector& tv = GetTimeVector();
	
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
	
	tv.SetBegin(Time(2014, 9, 1)); // Find begin of slowest #SPX data
	Date now = GetSysTime();
	do {++now;}
	while (now.day != 15);
	tv.SetEnd(Time(now.year, now.month, now.day));
	
	
	PathResolver& res = *GetPathResolver();
	MetaTime& mt = res.GetTime();
	
	mt.SetBegin(tv.GetBegin());
	mt.SetEnd(tv.GetEnd());
	mt.SetBasePeriod(tv.GetBasePeriod());
	
}

void InitIndicators() {
	TimeVector& tv = GetTimeVector();
	tv.LinkPath("/open", "/dummyvalue");
	tv.LinkPath("/ma0", "/ma?method=0");
	
	PathResolver& res = *GetPathResolver();
	res.LinkPath("/H1", "/bardata?bar=\"/open\"&id=0&period=1");
	res.LinkPath("/H1_ma0", "/H1/cont?slot=\"/ma0\"");
	
	
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
}

GUI_APP_MAIN {
	InitTimeVector();
	InitIndicators();
	
	Overlook ie;
	
	ie.Run();
}
