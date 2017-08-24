#if 0

#include "Overlook.h"

namespace Overlook {

SymGroup::SymGroup() {
	group = NULL;
	sys = NULL;
	
}

void SymGroup::SetEpsilon(double d) {
	for(int i = 0; i < agents.GetCount(); i++)
		agents[i].SetEpsilon(d);
}

void SymGroup::RefreshWorkQueue() {
	Index<int> sym_ids;
	sym_ids <<= group->sym_ids;
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		const Symbol& sym = GetMetaTrader().GetSymbol(sym_ids[i]);
		if (sym.proxy_id == -1) continue;
		sym_ids.FindAdd(sym.proxy_id);
	}
	sys->GetCoreQueue(work_queue, sym_ids, group->tf_ids, group->indi_ids);
	
	// Get DataBridge work queue
	Index<int> db_indi_ids;
	db_indi_ids.Add(sys->Find<DataBridge>());
	sys->GetCoreQueue(db_queue, sym_ids, group->tf_ids, db_indi_ids);
}

void SymGroup::ProcessWorkQueue() {
	work_lock.Enter();
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		//LOG(i << "/" << work_queue.GetCount());
		group->SubProgress(i, work_queue.GetCount());
		sys->Process(*work_queue[i]);
	}
	
	work_lock.Leave();
}

void SymGroup::ProcessDataBridgeQueue() {
	work_lock.Enter();
	
	for(int i = 0; i < db_queue.GetCount(); i++) {
		//LOG(i << "/" << db_queue.GetCount());
		group->SubProgress(i, db_queue.GetCount());
		sys->Process(*db_queue[i]);
	}
	
	work_lock.Leave();
}


}
#endif
