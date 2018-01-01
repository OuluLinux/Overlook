#include "Overlook.h"


namespace Overlook {
using namespace Upp;


AccountAdvisor::AccountAdvisor() {
	
}

void AccountAdvisor::Init() {
	
}

void AccountAdvisor::Start() {
	System& sys = GetSystem();
	
	if (once) {
		once = false;
		//if (prev_counted) prev_counted--;
		prev_counted = false;
		//open_limit = -1;
	}
	
	
	int bars					= GetBars();
	int tf						= GetTf();
	CoreIO* core				= CoreIO::GetInputCore(1, GetSymbol(), tf);
	MainAdvisor* ma				= dynamic_cast<MainAdvisor*>(core);
	ASSERT(ma);
	bool sources_finished		= ma->IsJobsFinished() && ma->prev_counted == sys.GetCountTf(tf);
	
	if (sources_finished && prev_counted < bars) {
		RefreshSourcesOnlyDeep();
		
		if (inputs.IsEmpty()) {
			CoreIO* core				= CoreIO::GetInputCore(1, GetSymbol(), tf);
			MainAdvisor* ma				= dynamic_cast<MainAdvisor*>(core);
			spread_point	.SetCount(SYM_COUNT, 0);
			inputs			.SetCount(SYM_COUNT, NULL);
			for(int i = 0; i < SYM_COUNT; i++) {
				int symbol					= sys.GetPrioritySymbol(i);
				int tf						= GetTf();
				ConstBuffer& open_buf		= GetInputBuffer(0, symbol, tf, 0);
				inputs[i]					= &open_buf;
				spread_point[i]				= ma->GetSpreadPoint(i);
			}
		}
		
		if (open_limit < 0) {
			double best_i = -1, best_j = -1;
			double best_res = -DBL_MAX;
			for(double i = 0.0; i < 0.20; i += 0.01) {
				for(double j = 0.0; j < 0.20; j += 0.01) {
					
					open_limit = i;
					keep_limit = j;
					
					RunSimBroker();
					double e = sb.AccountEquity();
					if (e > best_res) {
						best_i = i;
						best_j = j;
						best_res = e;
					}
				}
			}
			
			open_limit = best_i;
			keep_limit = best_j;
			RunSimBroker();
		}
		
		LOG("AccountAdvisor::Start Refresh");
		RefreshMain();
		prev_counted = bars;
	}
}

void AccountAdvisor::RefreshMain() {
	RunSimBroker();
	
	MainReal();
}


void AccountAdvisor::MainReal() {
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


void AccountAdvisor::RunSimBroker() {
	System& sys			= GetSystem();
	DataBridge* db		= dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()));
	CoreIO* core		= CoreIO::GetInputCore(1, GetSymbol(), GetTf());
	MainAdvisor* ma		= dynamic_cast<MainAdvisor*>(core);
	
	Buffer& open_buf	= db->GetBuffer(0);
	Buffer& low_buf		= db->GetBuffer(1);
	Buffer& high_buf	= db->GetBuffer(2);
	Buffer& volume_buf	= db->GetBuffer(3);
	
	int tf = GetTf();
	int bars = Upp::min(GetBars(), ma->data.GetCount());
	db->ForceCount(bars);
	
	
	sb.Brokerage::operator=(GetMetaTrader());
	sb.SetInitialBalance(10000);
	sb.Init();
	sb.SetFreeMarginLevel(FMLEVEL);
	sb.SetFreeMarginScale(SYM_COUNT);
	
	
	for(int i = 0; i < bars; i++) {
		const auto& current = ma->data[i];
		
		
		for(int j = 0; j < SYM_COUNT; j++) {
			ConstBuffer& open_buf = *inputs[j];
			double curr = open_buf.GetUnsafe(i);
			int symbol = sys.GetPrioritySymbol(j);
			sb.SetPrice(symbol, curr);
		}
		sb.RefreshOrders();
		
		
		for(int j = 0; j < SYM_COUNT; j++) {
			int sym	= sys.GetPrioritySymbol(j);
			int sig = 0;
			int prev_sig = sb.GetSignal(sym);
			
			int begin = j * ADVISOR_PERIOD;
			int end = begin + ADVISOR_PERIOD;
			double weight_sum = 0.0;
			double pos_peak = 0.0, neg_peak = 0.0;
			for(int k = begin; k < end; k++) {
				weight_sum += current.weight[k];
				if (weight_sum > pos_peak) pos_peak = weight_sum;
				if (weight_sum < neg_peak) neg_peak = weight_sum;
			}
			int strong_sig = +pos_peak > -neg_peak ? +1 : -1;
			int next_sig = current.weight[0] > 0 ? +1 : -1;
			
			bool try_open = false;
			bool try_keep = false;
			
			if (prev_sig == 0) {
				
				if (strong_sig == next_sig)
					try_open = true;
				
			}
			else {
				
				if (next_sig == prev_sig)
					try_keep = true;
				else {
					try_open = true;
					
					double peak_neg = 0.0;
					double neg_sum = 0.0;
					for(int k = begin; k < end; k++) {
						neg_sum += prev_sig * -1 * current.weight[k];
						if (neg_sum < 0.0) break;
						if (neg_sum > peak_neg) peak_neg = neg_sum;
					}
					if (neg_sum < 0.0 && peak_neg < keep_limit)
						try_keep = true;
				}
				
			}
			
			if (try_open) {
				double peak_pos = 0.0;
				double pos_sum = 0.0;
				for(int k = begin; k < end; k++) {
					pos_sum += next_sig * current.weight[k];
					if (+pos_sum > peak_pos)
						peak_pos = pos_sum;
					
					// Don't use peaks after keep limit exceeded
					if (peak_pos - pos_sum > keep_limit)
						break;
				}
				if (peak_pos >= open_limit)
					sig = next_sig;
			}
			
			if (try_keep && sig == 0) {
				sig = prev_sig;
			}
			
			
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


