#include "Overlook.h"

#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_source.h>


struct Runner {
	typedef Runner CLASSNAME;
	
	void Start() {
		Thread::Start(THISBACK(Run));
	}
	
	void Run() {
	}
};

GUI_APP_MAIN {
	Loader loader;
	loader.Run();
	
	if (loader.exit)
		return;
	
	Runner run;
	run.Start();
	
	{
		Overlook ie;
		ie.Run();
	}
	
	Thread::ShutdownThreads();
}
