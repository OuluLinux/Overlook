#include "Overlook.h"

using namespace Overlook;


#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>


GUI_APP_MAIN {
	const Vector<String>& args = CommandLine();
	for(int i = 1; i < args.GetCount(); i+=2) {
		const String& s = args[i-1];
		if (s == "-addr") {
			arg_addr = args[i];
		}
		else if (s == "-port") {
			arg_port = ScanInt(args[i]);
		}
		else if (s == "-resetsignals") {
			reset_signals = ScanInt(args[i]);
		}
		else if (s == "-resetamps") {
			reset_amps = ScanInt(args[i]);
		}
		else if (s == "-resetfuses") {
			reset_fuses = ScanInt(args[i]);
		}
		else if (s == "-internetdata") {
			use_internet_m1_data = ScanInt(args[i]);
		}
	}
	
	try {
		System& sys = GetSystem();
		sys.Init();
		sys.Start();
		{
			::Overlook::Overlook ol;
			ol.Init();
			ol.Run();
		}
		sys.Stop();
		
		{
			TopWindow tw;
			tw.Icon(OverlookImg::icon());
			tw.Title("Saving agent group");
			Label lbl;
			lbl.SetLabel("Saving... please wait.");
			lbl.SetAlign(ALIGN_CENTER);
			tw.SetRect(0,0, 320, 60);
			tw.Add(lbl.SizePos());
			Thread::Start([&]() {
				AgentSystem& ag = sys.GetAgentSystem();
				ag.StoreThis();
				PostCallback(callback(&tw, &TopWindow::Close));
				PostCallback(callback(&tw, &TopWindow::Close));
			});
			tw.Run();
		}
		
		Thread::ShutdownThreads();
	}
	catch (::Overlook::UserExc e) {
		PromptOK(e);
	}
	catch (Exc e) {
		PromptOK("Error: " + e);
	}
	catch (ConnectionError e) {
		PromptOK("Connection error: " + e);
	}
	catch (...) {
		PromptOK("Unknown error");
	}
}

