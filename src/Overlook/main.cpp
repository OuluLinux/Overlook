#include "Overlook.h"

#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>



GUI_APP_MAIN {
	{
		Loader loader;
		loader.Run();
		
		if (loader.exit)
			return;
	}
	
	
	{
		Overlook ie;
		ie.Run();
	}
	
	Thread::ShutdownThreads();
}
