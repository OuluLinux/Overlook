#include "Overlook.h"

using namespace Overlook;


#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>


GUI_APP_MAIN {
	LOG(GetAmpDevices());
	//TestCompatAMP(); return;
	
	const Vector<String>& args = CommandLine();
	for(int i = 1; i < args.GetCount(); i+=2) {
		const String& s = args[i-1];
		if (s == "-addr") {
			arg_addr = args[i];
		}
		else if (s == "-port") {
			arg_port = ScanInt(args[i]);
		}
	}
	
	System& sys = GetSystem();
	sys.Init();
	sys.Start();
	{
		::Overlook::Overlook ol;
		ol.Init();
		ol.Run();
	}
	sys.Stop();
	
	Thread::ShutdownThreads();
}

