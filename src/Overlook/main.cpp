#include "Overlook.h"

using namespace Overlook;

#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>

namespace Config {
INI_BOOL(use_internet_m1_data, false, "Download M1 data from Internet")
INI_BOOL(wait_mt4, false, "Wait for MT4 to respond")
INI_STRING(arg_addr, "127.0.0.1", "Host address");
INI_INT(arg_port, 42000, "Host port");
};

GUI_APP_MAIN {
	SetIniFile(ConfigFile("overlook.ini"));
	
	const Vector<String>& args = CommandLine();
	for(int i = 1; i < args.GetCount(); i+=2) {
		const String& s = args[i-1];
		if (s == "-addr") {
			Config::arg_addr = args[i];
		}
		else if (s == "-port") {
			Config::arg_port = ScanInt(args[i]);
		}
		else if (s == "-internetdata") {
			Config::use_internet_m1_data = ScanInt(args[i]);
		}
		else if (s == "-waitmt4") {
			Config::wait_mt4 = ScanInt(args[i]);
		}
		/*else if (s == "-foresttest") {
			RandomForestTester().Run();
			return;
		}*/
		else if (s == "-extremumtest") {
			TestExtremumCache();
			return;
		}
	}
	
	
	if (Config::wait_mt4) {
		MetaTrader& mt = GetMetaTrader();
		while (true) {
			try {
				if (!mt.Init(Config::arg_addr, Config::arg_port))
					break;
			}
			catch (...) {
				Sleep(1000);
			}
		}
	}
	
	try {
		System& sys = GetSystem();
		sys.Init();
		
		{
			::Overlook::Overlook ol;
			ol.Run();
		}
		
		Thread::ShutdownThreads();
	}
	catch (::Overlook::UserExc e) {
		PromptOK(e);
	}
	catch (Exc e) {
		PromptOK(e);
	}
	catch (...) {
		PromptOK("Unknown error");
	}
}

