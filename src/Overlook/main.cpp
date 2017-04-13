#include "Overlook.h"

#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>

String addr = "192.168.0.5";
int port = 42000;
//Time begin = Time(2016, 1, 24);
Time begin = Time(2017, 1, 1);

void InitTimeVector() {
	TimeVector& tv = GetTimeVector();
	PathResolver& res = *GetPathResolver();
	
	tv.EnableCache();
	tv.LimitMemory();
	//tv.LimitMemory(640000);
	
	// Init sym/tfs/time space
	{
		MetaTrader mt;
		mt.Init(addr, port);
		
		// Add symbols
		Vector<Symbol> symbols = mt.GetSymbols();
		ASSERTEXC(!symbols.IsEmpty());
		for(int i = 0; i < symbols.GetCount(); i++) {
			tv.AddSymbol(symbols[i].name);
		}
		
		// Add periods
		ASSERT(mt.GetTimeframe(0) == 1);
		int base = 15; // mins
		tv.SetBasePeriod(60*base);
		Vector<int> tfs;
		for(int i = 0; i < mt.GetTimeframeCount(); i++) {
			int tf = mt.GetTimeframe(i);
			if (tf >= base) {
				tfs.Add(tf / base);
				tv.AddPeriod(tf * 60);
			}
		}
		
		
		// Link symbols and timeframes
		
		// Add DataBridge
		tv.LinkPath("/databridge", "/db?addr=\"" + addr + "\"&port=" + IntStr(port));
		tv.LinkPath("/osc", "/dummyoscillator");
		
		// Add GUI objects
		for(int i = 0; i < symbols.GetCount(); i++) {
			for(int j = 0; j < tfs.GetCount(); j++) {
				
				// Add by number
				String dest = "/id/id" + IntStr(i) + "/tf" + IntStr(tfs[j]);
				String src = "/bardata?bar=\"/databridge\"&id=" + IntStr(i) + "&period=" + IntStr(tfs[j]);
				res.LinkPath(dest, src);
				
				// Add by name
				dest = "/name/" + symbols[i].name + "/tf" + IntStr(tfs[j]);
				dest.Replace("#", "");
				res.LinkPath(dest, src);
				
				
				res.LinkPath(dest + "osc", dest + "/cont?slot=\"/osc\"");
				
			}
		}
	}
	
	// Init time range
	Date now = GetSysTime();
	do {++now;}
	while (now.day != 15);
	tv.SetBegin(::begin); // Find begin of fastest #SPX data
	tv.SetEnd(Time(now.year, now.month, now.day));
	MetaTime& mt = res.GetTime();
	mt.SetBegin(tv.GetBegin());
	mt.SetEnd(tv.GetEnd());
	mt.SetBasePeriod(tv.GetBasePeriod());
	
	
	
	
	
	
	
	
	
	
	
	
	// Create or load cache base on linked slots (keep this 2. last)
	tv.RefreshData();
	
	// Link runner (also starts processing, so keep this last)
	res.LinkPath("/runner", "/runnerctrl");
}

void InitIndicators() {
	TimeVector& tv = GetTimeVector();
	//tv.LinkPath("/open", "/dummyvalue");
	//tv.LinkPath("/open", "/testvalue");
	
	//tv.LinkPath("/open_norm", "/normvalue?src=\"/open\"");
	//tv.LinkPath("/open_derv", "/derivedvalue?src=\"/open\"");
	
	//tv.LinkPath("/ma0", "/ma?method=0");
	
	//tv.LinkPath("/dummytrainer", "/dummytrainer");
	//tv.LinkPath("/rnn", "/rnn");
	
	//PathResolver& res = *GetPathResolver();
	//res.LinkPath("/derv_H1", "/dervctrl?bar=\"/open\"&id=0&period=1");
	//res.LinkPath("/rnn_H1", "/rnnctrl?bar=\"/open\"&id=0&period=1");
	
	/*res.LinkPath("/M15", "/bardata?bar=\"/open\"&id=0&period=1");
	res.LinkPath("/M30", "/bardata?bar=\"/open\"&id=0&period=2");
	res.LinkPath("/H1", "/bardata?bar=\"/open\"&id=0&period=4");
	res.LinkPath("/H1_ma0", "/H1/cont?slot=\"/ma0\"");
	res.LinkPath("/H4", "/bardata?bar=\"/open\"&id=0&period=16");
	res.LinkPath("/D1", "/bardata?bar=\"/open\"&id=0&period=96");
	res.LinkPath("/W1", "/bardata?bar=\"/open\"&id=0&period=672");
	
	res.LinkPath("/sym2_D1", "/bardata?bar=\"/open\"&id=1&period=4");*/
	
	
}

struct Runner {
	typedef Runner CLASSNAME;
	
	void Start() {
		Thread::Start(THISBACK(Run));
	}
	
	void Run() {
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
