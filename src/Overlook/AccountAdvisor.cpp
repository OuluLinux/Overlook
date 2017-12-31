#include "Overlook.h"


namespace Overlook {
using namespace Upp;


WeekSlotAdvisor::WeekSlotAdvisor() {
	
}

void WeekSlotAdvisor::Init() {
	tfmins		= GetMinutePeriod();
	weekslots	= 5 * 24 * 60 / tfmins;
	cols		= weekslots * SYM_COUNT;
}

void WeekSlotAdvisor::Start() {
	#if ENABLE_TRAINING
	System& sys = GetSystem();
	
	if (once) {
		once = false;
		//if (prev_counted) prev_counted--;
		prev_counted = false;
		//Reset(); // For developing
	}
	
	
	if (tf_ids.GetCount() == 0) {
		ASSERT(tf_ids.IsEmpty());
		int this_tfmins = GetMinutePeriod();
		for(int i = sys.GetPeriodCount()-1; i >= GetTf(); i--) {
			int tf_mins = sys.GetPeriod(i);
			if (IsTfUsed(tf_mins)) {
				tf_ids.Add(i);
				int ratio = tf_mins / this_tfmins;
				ASSERT(ratio > 0);
				ratios.Add(ratio);
			}
		}
		ASSERT(tf_ids.GetCount() > 0);
	}
	
	
	bool sources_finished = true;
	int bars = GetBars();
	int chk_count = 0;
	for(int i = 0; i < SYM_COUNT; i++) {
		int symbol						= sys.GetPrioritySymbol(i);
		for(int j = 0; j < tf_ids.GetCount(); j++) {
			int tf = tf_ids[j];
			CoreIO* core				= CoreIO::GetInputCore(2, symbol, tf);
			DqnAdvisor* adv				= dynamic_cast<DqnAdvisor*>(core);
			ASSERT(adv);
			sources_finished			&= adv->IsJobsFinished()
										&& adv->prev_counted == sys.GetCountTf(tf)
										&& adv->GetOutput(0).label.GetCount() == sys.GetCountTf(tf)
										&& adv->GetOutput(1).label.GetCount() == sys.GetCountTf(tf);
			chk_count++;
		}
	}
	
	if (sources_finished && prev_counted < bars && chk_count > 0) {
		RefreshSourcesOnlyDeep();
		
		if (time_slots.IsEmpty()) {
			spread_point	.SetCount(SYM_COUNT, 0);
			inputs			.SetCount(SYM_COUNT, NULL);
			for(int i = 0; i < SYM_COUNT; i++) {
				int symbol					= sys.GetPrioritySymbol(i);
				int tf						= GetTf();
				ConstBuffer& open_buf		= GetInputBuffer(0, symbol, tf, 0);
				inputs[i]					= &open_buf;
			}
			
			
			signals			.SetCount(tf_ids.GetCount());
			enabled			.SetCount(tf_ids.GetCount());
			for(int j = 0; j < tf_ids.GetCount(); j++) {
				int tf = tf_ids[j];
				signals[j]	.SetCount(SYM_COUNT, NULL);
				enabled[j]	.SetCount(SYM_COUNT, NULL);
				for(int i = 0; i < SYM_COUNT; i++) {
					int symbol					= sys.GetPrioritySymbol(i);
					CoreIO* core				= CoreIO::GetInputCore(2, symbol, tf);
					ASSERT(core);
					ConstVectorBool& sig_buf	= core->GetOutput(0).label;
					ConstVectorBool& ena_buf	= core->GetOutput(1).label;
					signals[j][i]				= &sig_buf;
					enabled[j][i]				= &ena_buf;
					
					if (!j) spread_point[i]		= dynamic_cast<DqnAdvisor*>(core)->GetSpreadPoint();
				}
			}
			
			
			int best_i = -1, best_j = -1;
			double best_res = -DBL_MAX;
			for(int i = 1; i < 15; i++) {
				for(int j = 0; j < 10; j++) {
					OptimizeLimit(0.0001 * i, 0.0001 * j);
					RunSimBroker();
					double e = sb.AccountEquity();
					if (e > best_res) {
						best_i = i;
						best_j = j;
						best_res = e;
					}
				}
			}
			
			double limit1 = best_i != -1 ? 0.0001 * best_i : 0.001;
			double limit2 = best_j != -1 ? 0.0001 * best_j : 0.001;
			OptimizeLimit(limit1, limit2);
		}
		
		LOG("WeekSlotAdvisor::Start Refresh");
		RefreshMain();
		prev_counted = bars;
	}
	#endif
}

void WeekSlotAdvisor::OptimizeLimit(double chg_limit, double slow_limit) {
	System& sys = GetSystem();
	time_slots.SetCount(SYM_COUNT);
	for(int i = 0; i < time_slots.GetCount(); i++) {
		Vector<int>& sym_slots = time_slots[i];
		sym_slots.SetCount(weekslots, -1);
		
		int sym = sys.GetPrioritySymbol(i);
		Vector<Vector<OnlineAverage1>*> stats;
		
		for(int j = 0; j < tf_ids.GetCount(); j++) {
			int tf = tf_ids[j];
			VolatilitySlots& vs = *dynamic_cast<VolatilitySlots*>(GetInputCore(1, sym, tf));
			
			int exp_vs_stats_count = weekslots / ratios[j];
			ASSERT(exp_vs_stats_count == vs.stats.GetCount());
			
			stats.Add(&vs.stats);
		}
		
		for(int j = 0; j < sym_slots.GetCount(); j++) {
			
			// try to zoom
			int level = -1;
			for(int k = 0; k < tf_ids.GetCount(); k++) {
				int ratio = ratios[k];
				int slow_slot = j / ratio;
				double mean_chg = (*stats[k])[slow_slot].mean;
				if (mean_chg < chg_limit || (k == 0 && mean_chg < slow_limit))
					break;
				level = k;
			}
			
			sym_slots[j] = level;
		}
		
		
		// Fill holes
		for(int i = 4; i < sym_slots.GetCount(); i++) {
			int buf[5];
			for(int j = 0; j < 5; j++)
				buf[j] = sym_slots[i-j];
			bool changed = false;
			// Fill 1 hole
			if (buf[0] == buf[2] && buf[1] == buf[0]-1) {
				buf[1] = buf[0];
				changed = true;
			}
			// Fill 2 hole
			else if (buf[0] == buf[3] && buf[1] == buf[0]-1 && buf[2] == buf[0]-1) {
				buf[1] = buf[0];
				buf[2] = buf[0];
				changed = true;
			}
			// Fill 3 hole
			else if (buf[0] == buf[4] && buf[1] == buf[0]-1 && buf[2] == buf[0]-1 && buf[3] == buf[0]-1) {
				buf[1] = buf[0];
				buf[2] = buf[0];
				buf[3] = buf[0];
				changed = true;
			}
			if (changed)
				for(int j = 0; j < 5; j++)
					sym_slots[i-j] = buf[j];
		}
	}
}

void WeekSlotAdvisor::RefreshMain() {
	RunSimBroker();
	
	MainReal();
}


void WeekSlotAdvisor::MainReal() {
	System&	sys				= GetSystem();
	Time now				= GetSysTime();
	int wday				= DayOfWeek(now);
	Time after_hour			= now + 60 * 60;
	int wday_after_hour		= DayOfWeek(after_hour);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
	// Skip weekends and first hours of monday
	if (wday == 0 || wday == 6 || (wday == 1 && now.hour < 1)) {
		LOG("Skipping weekend...");
		return;
	}
	
	
	// Inspect for market closing (weekend and holidays)
	else if (wday == 5 && wday_after_hour == 6) {
		sys.WhenInfo("Closing all orders before market break");

		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			mt.SetSignal(i, 0);
			mt.SetSignalFreeze(i, false);
		}

		mt.SignalOrders(true);
		return;
	}
	
	
	
	sys.WhenInfo("Updating MetaTrader");
	sys.WhenPushTask("Putting latest signals");
	
	// Reset signals
	if (realtime_count == 0) {
		for (int i = 0; i < mt.GetSymbolCount(); i++)
			mt.SetSignal(i, 0);
	}
	realtime_count++;
	
	
	try {
		mt.Data();
		mt.RefreshLimits();
		for (int i = 0; i < SYM_COUNT; i++) {
			int sym = sys.GetPrioritySymbol(i);
			
			int sig = sb.GetSignal(sym);
			
			if (sig == mt.GetSignal(sym) && sig != 0)
				mt.SetSignalFreeze(sym, true);
			else {
				mt.SetSignal(sym, sig);
				mt.SetSignalFreeze(sym, false);
			}
			LOG("Real symbol " << sym << " signal " << sig);
		}
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale((MULT_MAXSCALES - 1)*MULT_MAXSCALE_MUL * SYM_COUNT);
		mt.SignalOrders(true);
	}
	catch (...) {
		
	}
	
	
	sys.WhenRealtimeUpdate();
	sys.WhenPopTask();
}


void WeekSlotAdvisor::RunSimBroker() {
	System& sys = GetSystem();
	DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()));
	
	Buffer& open_buf	= db->GetBuffer(0);
	Buffer& low_buf		= db->GetBuffer(1);
	Buffer& high_buf	= db->GetBuffer(2);
	Buffer& volume_buf	= db->GetBuffer(3);
	
	int tf = GetTf();
	int bars = GetBars();
	db->ForceCount(bars);
	
	
	sb.Brokerage::operator=(GetMetaTrader());
	sb.SetInitialBalance(10000);
	sb.Init();
	sb.SetFreeMarginLevel(FMLEVEL);
	sb.SetFreeMarginScale(SYM_COUNT);
	
	
	for(int i = 0; i < bars; i++) {
		int weekslot = i % weekslots;
		
		for(int j = 0; j < SYM_COUNT; j++) {
			ConstBuffer& open_buf = *inputs[j];
			double curr = open_buf.GetUnsafe(i);
			int symbol = sys.GetPrioritySymbol(j);
			sb.SetPrice(symbol, curr);
		}
		sb.RefreshOrders();
		
		
		for(int j = 0; j < SYM_COUNT; j++) {
			int sig = 0;
			int read_tf = time_slots[j][weekslot];
			if (read_tf != -1) {
				int read_pos = sys.GetShiftTf(tf, tf_ids[read_tf], i);
				if (enabled[read_tf][j]->Get(read_pos)) {
					sig		= signals[read_tf][j]->Get(read_pos) ? -1 : +1;
				}
			}
			int sym		= sys.GetPrioritySymbol(j);
			
			if (sig == sb.GetSignal(sym) && sig != 0)
				sb.SetSignalFreeze(sym, true);
			else {
				sb.SetSignal(sym, sig);
				sb.SetSignalFreeze(sym, false);
			}
		}
		
		
		sb.SignalOrders(false);
		
		
		double value = sb.AccountEquity();
		if (value < 0.0) {
			sb.ZeroEquity();
			value = 0.0;
		}
		open_buf.Set(i, value);
		low_buf.Set(i, value);
		high_buf.Set(i, value);
		
		//LOG("SB " << i << " eq " << value);
	}
}

}


