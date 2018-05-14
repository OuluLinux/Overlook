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

struct LoaderWindow : public TopWindow {
	typedef LoaderWindow CLASSNAME;
	ImageCtrl img;
	bool fail = false, finished = false;
	
	LoaderWindow() {
		FrameLess();
		Add(img.SizePos());
		SetRect(0,0,640,400);
		Title("Overlook Loader");
		Icon(OverlookImg::icon());
		img.SetImage(OverlookImg::overlook());
	}
	
	void Start() {
		Thread::Start([=] {
			
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
				GetSystem().Init();
				GetCalendar().Init();
			}
			catch (::Overlook::UserExc e) {
				PromptOK(e);
				fail = true;
			}
			catch (Exc e) {
				PromptOK(e);
				fail = true;
			}
			catch (...) {
				PromptOK("Unknown error");
				fail = true;
			}
			
			finished = true;
			PostCallback(THISBACK(Close0));
		});
	}
	
	void Close0() {Close();}
};

GUI_APP_MAIN {
	TestLockMacro();
	
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
		else if (s == "-extremumtest") {
			TestExtremumCache();
			return;
		}
	}
	
	
	{
		LoaderWindow loader;
		loader.Start();
		loader.Run();
		while (!loader.finished) Sleep(100);
		if (loader.fail) return;
	}
	
	
	try {
		::Overlook::Overlook ol;
		ol.OpenMain();
		Ctrl::EventLoop();
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
	
	GetSystem().Deinit();
}

