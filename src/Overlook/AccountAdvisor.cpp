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
			int sig = current.weight[j * 2 + 0] > current.weight[j * 2 + 1] ? -1 : +1;
			
			
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


