#include "Overlook.h"

using namespace Overlook;

#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>

namespace Config {
INI_BOOL(use_internet_m1_data, false, "Download M1 data from Internet");
INI_BOOL(wait_mt4, false, "Wait for MT4 to respond");
INI_BOOL(have_sys_signal, false, "Use system signal money management");
INI_BOOL(fixed_tplimit, false, "Use fixed take-profit level");
INI_STRING(arg_addr, "127.0.0.1", "Host address");
INI_INT(arg_port, 42000, "Host port");
INI_INT(start_time, 0, "Starting time");
INI_INT(server_port, 17001, "Server listening port");
INI_INT(server_max_sessions, 100, "Server max simultaneous client sessions");
INI_STRING(server_title, "Unnamed server", "Server title");

INI_BOOL(email_enable, false, "Enable email autochartist events");
INI_STRING(email_user, "", "Email username");
INI_STRING(email_pass, "", "Email password");
INI_STRING(email_server, "", "Email server");
INI_INT(email_port, 0, "Email port");
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
	WriteKeepalive();
	
	TestLockMacro();
	SetIniFile(ConfigFile("overlook.ini"));
	
	// Set persistent starting time
	if (Config::start_time == 0) {
		Time t = GetUtcTime();
		#ifndef flagSECONDS
		t -= 2*365*24*60*60;
		#else
		t -= 14*24*60*60;
		#endif
		t.hour = 0;
		t.minute = 0;
		t.second = 0;
		while (DayOfWeek(t) == 0) t -= 24*60*60;
		Config::start_time = t.Get() - Time(1970,1,1).Get();
		FileAppend fapp("overlook.ini");
		fapp.PutEol();
		fapp << "start_time=" << Config::start_time;
		fapp.PutEol();
	}
	
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
		
		#ifdef flagHAVE_NOTIFICATIONS
		Notification& n = GetNotification();
		n.SetImage(OverlookImg::icon());
		n.SetApp("Overlook");
		#endif
	}
	
	
	try {
		::Overlook::Overlook& ol = GetOverlook();
		ol.OpenMain();
		Ctrl::EventLoop();
		ol.Deinit();
	}
	catch (::Overlook::UserExc e) {
		PromptOK(e);
	}
	catch (Exc e) {
		PromptOK(e);
	}
	catch (::Overlook::ConfExc e) {
		PromptOK(e);
	}
	catch (...) {
		PromptOK("Unknown error");
	}
	
	GetSystem().Deinit();
	
	Thread::ShutdownThreads();
}

