#if 0

#include "Overlook.h"

namespace Overlook {

Manager::Manager(System* sys) :
	sys(sys)
{
	prev_shift = -1;
	
	
}

void Manager::Init() {
	FindFile ff(ConfigFile("*.agrp"));
	Vector<String> paths;
	while (ff) {
		paths.Add(ff.GetPath());
		ff.Next();
	}
	
	if (!paths.IsEmpty()) {
		ManagerLoader loader;
		loader.OpenMain();
		for(int i = 0; i < paths.GetCount(); i++) {
			AgentGroup& ag = groups.Add();
			ag.sys = sys;
			ag.name = GetFileTitle(paths[i]);
			loader.PostProgress("Loading agent group: " + ag.name, i, paths.GetCount());
			ag.LoadThis();
			ag.Init();
			if (ag.enable_training)
				ag.Start();
		}
		loader.Close();
	}
	
	AgentGroup* best_group = GetBestGroup();
	prev_update = GetMetaTrader().GetTime();
	if (best_group)
		prev_shift = sys->GetShiftFromTimeTf(prev_update, best_group->main_tf);
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
	/*MetaTrader& mt = GetMetaTrader();
	Time time = mt.GetTime();
	sys->SetEnd(time);
	
	AgentGroup* best_group = GetBestGroup();
	if (best_group) {
		int wday = DayOfWeek(time);
		int shift = sys->GetShiftFromTimeTf(time, best_group->main_tf);
		if (prev_shift != shift) {
			if (wday == 0 || wday == 6) {
				// Do nothing
				prev_shift = shift;
			} else {
				sys->WhenInfo("Shift changed");
				
				// Forced askbid data download
				DataBridgeCommon& common = GetDataBridgeCommon();
				common.DownloadAskBid();
				common.RefreshAskBidData(true);
				
				// Refresh databridges
				best_group->ProcessDataBridgeQueue();
				
				// Use best group to set broker signals
				bool succ = best_group->PutLatest(mt);
				
				// Notify about successful signals
				if (succ) {
					prev_shift = shift;
					
					sys->WhenRealtimeUpdate();
				}
			}
		}
		
		// Check for market closing (weekend and holidays)
		else {
			Time after_hour = time + 60*60;
			int wday_after_hour = DayOfWeek(after_hour);
			if (wday == 5 && wday_after_hour == 6) {
				sys->WhenInfo("Closing all orders before market break");
				for(int i = 0; i < mt.GetSymbolCount(); i++) {
					mt.SetSignal(i, 0);
					mt.SetSignalFreeze(i, false);
				}
				mt.SignalOrders(true);
			}
			
			if (wday != 0 && wday != 6)
				best_group->Data();
		}
	} else {
		prev_shift = -1;
	}
	prev_update = time;*/
}

AgentGroup* Manager::GetBestGroup() {
	AgentGroup* group = NULL;
	
	/*for(int i = 0; i < groups.GetCount(); i++) {
		AgentGroup& g = groups[i];
		if (!g.is_realtime)
			continue;
		if (!group) {
			group = &g;
		} else {
			if (group->best_result < g.best_result)
				group = &g;
		}
	}*/
	
	return group;
}

}
#endif
