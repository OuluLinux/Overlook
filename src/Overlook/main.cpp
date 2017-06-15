#include "Overlook.h"

using namespace Overlook;


#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>


GUI_APP_MAIN {
	const Vector<String>& args = CommandLine();
	bool autostart = false;
	for(int i = 1; i < args.GetCount(); i+=2) {
		const String& s = args[i-1];
		if (s == "-addr") {
			arg_addr = args[i];
		}
		else if (s == "-port") {
			arg_port = ScanInt(args[i]);
		}
		else if (s == "-autostart") {
			autostart = ScanInt(args[i]);
		}
	}
	
	::Overlook::Overlook ol;
	ol.Init();
	ol.Load();
	if (autostart)
		ol.Start();
	ol.Run();
	ol.Deinit();
	
	Thread::ShutdownThreads();
}

