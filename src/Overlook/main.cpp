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
	//tv.LinkPath("/open", "/testvalue");
	tv.LinkPath("/open_norm", "/normvalue?src=\"/open\"");
	tv.LinkPath("/open_derv", "/derivedvalue?src=\"/open\"");
	
	tv.LinkPath("/ma0", "/ma?method=0");
	
	//tv.LinkPath("/dummytrainer", "/dummytrainer");
	tv.LinkPath("/rnn", "/rnn");
	
	PathResolver& res = *GetPathResolver();
	res.LinkPath("/derv_H1", "/dervctrl?bar=\"/open\"&id=0&period=1");
	res.LinkPath("/rnn_H1", "/rnnctrl?bar=\"/open\"&id=0&period=1");
	
	res.LinkPath("/H1", "/bardata?bar=\"/open\"&id=0&period=1");
	res.LinkPath("/H1_ma0", "/H1/cont?slot=\"/ma0\"");
	res.LinkPath("/H4", "/bardata?bar=\"/open\"&id=0&period=4");
	res.LinkPath("/D1", "/bardata?bar=\"/open\"&id=0&period=24");
	res.LinkPath("/W1", "/bardata?bar=\"/open\"&id=0&period=168");
	
	res.LinkPath("/sym2_D1", "/bardata?bar=\"/open\"&id=1&period=24");
	
	
	tv.RefreshData();
}


struct Runner {
	typedef Runner CLASSNAME;
	
	void Start() {
		Thread::Start(THISBACK(Run));
	}
	
	void Run() {
		TimeVector& tv = GetTimeVector();
		
		while (!Thread::IsShutdownThreads()) {
			while (!Thread::IsShutdownThreads()) {
				bool all_processed = true;
				for (TimeVector::Iterator it = tv.Begin(); !it.IsEnd() && !Thread::IsShutdownThreads(); it++) {
				
					if (it.Process()) {
						all_processed = false;
					}
					
				}
				if (all_processed)
					break;
			}
			
			Sleep(1000);
		}
	}
};

GUI_APP_MAIN {
	InitTimeVector();
	InitIndicators();
	
	Runner run;
	run.Start();
	
	{
		Overlook ie;
		ie.Run();
	}
	
	Thread::ShutdownThreads();
}
