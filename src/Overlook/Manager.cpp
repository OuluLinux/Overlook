#include "Overlook.h"

namespace Overlook {

Manager::Manager(System* sys) :
	sys(sys)
{
	
	
	
}

void Manager::Init() {
	FindFile ff(ConfigFile("*.agrp"));
	while (ff) {
		AgentGroup& ag = groups.Add();
		ag.sys = sys;
		ag.name = GetFileTitle(ff.GetName());
		ag.LoadThis();
		ag.Init();
		if (ag.enable_training)
			ag.Start();
		ff.Next();
	}
}

void Manager::Start() {
	
	
	
}

void Manager::Stop() {
	
	
	
}

}
