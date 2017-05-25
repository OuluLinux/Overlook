#include "Overlook.h"

using namespace Overlook;


#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>


GUI_APP_MAIN {
	//MemoryBreakpoint();
	
	::Overlook::Overlook ol;
	ol.Init();
	ol.Run();
	ol.Deinit();
	
	Thread::ShutdownThreads();
}

