#include "Overlook.h"

using namespace Overlook;


#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>


GUI_APP_MAIN {
	Factory::Init();
	
	::Overlook::Overlook ol;
	ol.Init();
	ol.Run();
	ol.Deinit();
	
	Factory::Deinit();
	Thread::ShutdownThreads();
}






#if 0



GUI_APP_MAIN {
	bool autostart = false;
	
	const Vector<String>& args = CommandLine();
	for(int i = 0; i < args.GetCount(); i++) {
		const String& a = args[i];
		if (a == "-autostart")
			autostart = true;
	}
	
	{
		Loader loader;
		loader.autostart = autostart;
		loader.Run();
		
		if (loader.exit)
			return;
	}
	
	
	{
		Overlook ie;
		ie.Run();
		GetSession().Stop();
		GetRefContext().Clear();
	}
	
	Thread::ShutdownThreads();
}
#endif
