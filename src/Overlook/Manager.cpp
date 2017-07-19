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
	sys->SetEnd(time);
	
	AgentGroup* best_group = GetBestGroup();
	if (best_group) {
		int shift = sys->GetShiftFromTimeTf(time, best_group->tf_ids.Top());
		if (prev_shift != shift) {
			sys->WhenInfo("Shift changed");
			
			// Forced askbid data download
			DataBridgeCommon& common = GetDataBridgeCommon();
			common.DownloadAskBid();
			common.RefreshAskBidData(true);
			
			// Refresh databridges
			best_group->ProcessDataBridgeQueue();
			
			// Use best group to set broker signals
			bool succ = best_group->PutLatest(GetMetaTrader());
			
			// Notify about successful signals
			if (succ) {
				prev_shift = shift;
				
				sys->WhenRealtimeUpdate();
			}
		}
	} else {
		prev_shift = -1;
	}
	prev_update = time;
}

AgentGroup* Manager::GetBestGroup() {
	AgentGroup* group = NULL;
	
	for(int i = 0; i < groups.GetCount(); i++) {
		AgentGroup& g = groups[i];
		if (!g.allow_realtime)
			continue;
		if (!group) {
			group = &g;
		} else {
			if (group->best_result < g.best_result)
				group = &g;
		}
	}
	
	return group;
}

}
