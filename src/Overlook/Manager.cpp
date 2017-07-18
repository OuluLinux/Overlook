#include "Overlook.h"

namespace Overlook {

Manager::Manager(System* sys) :
	sys(sys)
{
	prev_shift = -1;
	
	
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
	
	AgentGroup* best_group = GetBestGroup();
	prev_update = GetMetaTrader().GetTime();
	if (best_group)
		prev_shift = sys->GetShiftFromTimeTf(prev_update, best_group->tf_ids.Top());
	else
		prev_shift = -1;
}

void Manager::Start() {
	for(int i = 0; i < groups.GetCount(); i++) {
		AgentGroup& ag = groups[i];
		if (ag.enable_training)
			ag.Start();
	}
}

void Manager::Stop() {
	for(int i = 0; i < groups.GetCount(); i++) {
		groups[i].Stop();
	}
}

void Manager::Main() {
	Time time = GetMetaTrader().GetTime();
	
	AgentGroup* best_group = GetBestGroup();
	if (best_group) {
		int shift = sys->GetShiftFromTimeTf(time, best_group->tf_ids.Top());
		if (prev_shift != shift) {
			best_group->ProcessDataBridgeQueue();
			
			best_group->PutLatest(GetMetaTrader());
			
			prev_shift = shift;
		}
	} else {
		prev_shift = -1;
	}
	prev_update = time;
}

AgentGroup* Manager::GetBestGroup() {
	AgentGroup* group = NULL;
	
	for(int i = 0; i < groups.GetCount(); i++) {
		if (!group) {
			group = &groups[i];
		} else {
			AgentGroup& g = groups[i];
			if (group->best_result < g.best_result)
				group = &g;
		}
	}
	
	return group;
}

}
