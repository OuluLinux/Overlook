#include "Overlook.h"

namespace Overlook {

Joiner::Joiner() {
	
}

void Joiner::ResetEpoch() {
	if (broker.order_count > 0) {
		last_drawdown = broker.GetDrawdown();
		if (broker.equity > best_result)
			best_result = broker.equity;
		result[result_cursor] = broker.equity;
		result_cursor = (result_cursor + 1) % AGENT_RESULT_COUNT;
		result_count++;
	}
	
	broker.Reset();
	
	prev_equity = broker.AccountEquity();
	signal = 0;
	timestep_actual = 0;
	timestep_total = 1;
	cursor = 1;
}

void Joiner::Main(Snapshot& cur_snap, Snapshot& prev_snap) {
	
}

/*
bool Joiner::PutLatest(Brokerage& broker) {
	
	Time time = GetMetaTrader().GetTime();
	int shift = sys->GetShiftFromTimeTf(time, main_tf);
	if (shift != train_pos_all.Top()) {
		WhenError(Format("Current shift doesn't match the lastest snapshot shift (%d != %d)", shift, train_pos_all.Top()));
		return false;
	}
	
	
	group->WhenInfo("Looping agents until latest snapshot");
	LoopAgentsToEnd(tf_ids.GetCount(), true);
	Snapshot& shift_snap = snaps[train_pos_all.GetCount()-1];
	
	
	// Reset signals
	if (is_realtime) {
		if (realtime_count == 0) {
			for(int i = 0; i < broker.GetSymbolCount(); i++)
				broker.SetSignal(i, 0);
		}
		realtime_count++;
	}
	
	
	// Set probability for random actions to 0
	Forward(shift_snap, broker);
	
	
	String sigstr = "Signals ";
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		if (i) sigstr << ",";
		sigstr << broker.GetSignal(sym_ids[i]);
	}
	group->WhenInfo(sigstr);
	
	
	group->WhenInfo("Refreshing broker data");
	MetaTrader* mt = dynamic_cast<MetaTrader*>(&broker);
	if (mt) {
		mt->Data();
	} else {
		SimBroker* sb = dynamic_cast<SimBroker*>(&broker);
		sb->RefreshOrders();
	}
	broker.SetLimitFactor(limit_factor);
	
	
	group->WhenInfo("Updating orders");
	broker.SignalOrders(true);
	broker.RefreshLimits();
	
	return true;
}
*/

}
