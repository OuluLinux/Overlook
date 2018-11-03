#include "Upkeeper.h"

Upkeeper::Upkeeper()
{
	CtrlLayout(*this, "Upkeeper");
	
	exepath.SetData("Overlook.exe");
	dnspath.SetData("https://freedns.afraid.org/dynamic/update.php?M2xZU09nd1l5OHdQVG82T3lLZG46MTc4NjQ4MTM=");
	
	list.AddColumn("Time");
	list.AddColumn("What");
	list.ColumnWidths("1 2");
	
	Thread::Start(THISBACK(Process));
}

void Upkeeper::Process() {
	
	One<LocalProcess> lp;
	TimeStop urlts;
	
	while (!Thread::IsShutdownThreads()) {
		FileIn fin(ConfigFile("keepalive.bin"));
		Time t;
		fin % t;
		
		if (lp.IsEmpty() || !lp->IsRunning() || t < GetSysTime() - 1*60) {
			lp.Create();
			lp->Start((String)exepath.GetData());
			PostCallback(THISBACK1(Add, "App reboot"));
		}
		
		if (urlts.Elapsed() > 10*60*1000) {
			HttpRequest().Url((String)dnspath.GetData()).Execute();
			urlts.Reset();
			PostCallback(THISBACK1(Add, "Dyndns up"));
		}
		
		Sleep(1000);
	}
}

GUI_APP_MAIN
{
	Upkeeper().Run();
}
