#include "Overlook.h"


namespace Overlook {
using namespace Upp;


WeekSlotAdvisor::WeekSlotAdvisor() {
	
}

void WeekSlotAdvisor::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum( 0.0);  // normalized
	SetCoreMaximum(+1.0);   // normalized
	
	for(int i = 0; i < SYM_COUNT; i++) {
		SetBufferColor(i, RainbowColor((double)i / SYM_COUNT));
		SetBufferLineWidth(i, 2);
	}
}

void WeekSlotAdvisor::Start() {
	System& sys = GetSystem();
	int tf = GetTf();
	
	
	//optimizer.SetMaxRounds(2000);
	
	
	if (!running) {
		int bars = GetBars();
		Output& out = GetOutput(0);
		for(int i = 0; i < out.buffers.GetCount(); i++)
			out.buffers[i].SetCount(bars);
		
		// Wait until sources have RF_IDLEREAL
		if (phase == WS_IDLE) {
			bool all_ready = true;
			for(int i = 0; i < SYM_COUNT; i++) {
				int symbol = sys.GetPrioritySymbol(i);
				CoreIO* core = GetInputCore(1, symbol, tf);
				ASSERT(core);
				RandomForestAdvisor* rfa = dynamic_cast<RandomForestAdvisor*>(core);
				ASSERT(rfa);
				all_ready &= rfa->GetPhase() == RF_REAL;
			}
			if (all_ready)
				phase = WS_TRAINING;
		}
		
		if (phase == WS_TRAINING) {
			running = true;
			Thread::Start(THISBACK(MainTraining));
		}
	}
	
	
	if (!running && phase == WS_REAL) {
		int bars = GetBars();
		if (prev_counted < bars) {
			RefreshMain();
			prev_counted = bars;
		}
	}
}

void WeekSlotAdvisor::MainTraining() {
	running = true;
	serializer_lock.Enter();
	
	SetRealArea();
	MainOptimizer();
	
	serializer_lock.Leave(); // leave before StoreCache
	
	StoreCache();
	
	if (optimizer.IsEnd()) {
		LOG("WeekSlotAdvisor::MainTraining finished");
		optimizer.GetLimitedBestSolution(trial);
		NormalizeTrial();
		
		ForceSetCounted(0);
		RefreshMainBuffer(false);
		RunMain();

		phase = WS_REAL;
		StoreCache();
	}
	
	running = false;
}

void WeekSlotAdvisor::MainOptimizer() {
	
	// Init genetic optimizer
	int tfmins		= GetMinutePeriod();
	int slotmins	= tfmins < 60 ? 60 : tfmins;
		weekslots	= 5 * 24 * 60 / slotmins;
	int cols		= weekslots * SYM_COUNT;
	if (optimizer.GetRound() == 0 || forced_optimizer_reset) {
		forced_optimizer_reset = false;
		
		optimizer.SetArrayCount(1);
		optimizer.SetCount(cols);
		optimizer.SetPopulation(100);
		optimizer.SetMaxGenerations(1000);
		optimizer.UseLimits();


		// Set optimizer column value ranges
		int col = 0;
		int weekmins = 0;
		for(int i = 0; i < weekslots; i++) {
			int wday = weekmins / (24 * 60);
			int hour = (weekmins % (24 * 60)) / 60;
			
			String title;
			switch (wday) {
				case 0: title << "Monday ";		break;
				case 1: title << "Tuesday ";	break;
				case 2: title << "Wednesday ";	break;
				case 3: title << "Thursday ";	break;
				case 4: title << "Friday ";		break;
				default: Panic("Invalid weekday");
			}
			title << "hour " << hour;
			
			for(int j = 0; j < SYM_COUNT; j++)
				optimizer.Set(col++, 0.0, +1.0, 0.01, title);
			
			weekmins += slotmins;
		}
		ASSERT(col == cols);
		optimizer.Init(StrategyRandom2Bin);
	}
	
	
	// Optimize
	while (!optimizer.IsEnd() && !Thread::IsShutdownThreads()) {
		
		// Do some backup storing
		if (optimizer.GetRound() % 10000 == 9999) {
			serializer_lock.Leave();
			StoreCache();
			serializer_lock.Enter();
		}
		
		
		// Refresh databridge candlesticks sometimes
		/*if (optimizer.GetRound() % 1000 == 999)
			RunSimBroker();*/
		
		
		// Get weights
		optimizer.Start();
		optimizer.GetLimitedTrialSolution(trial);
		NormalizeTrial();

		RefreshMainBuffer(true);
		RunMain();

		// Return training value with less than 10% of testing value.
		// Testing value should slightly direct away from weird locality...
		double change_total = (area_change_total[0] + area_change_total[1] * 0.1) / 1.1;
		LOG(GetSymbol() << " round " << optimizer.GetRound()
			<< ": tr=" << area_change_total[0]
			<<  " t0=" << area_change_total[1]
			<<  " t1=" << area_change_total[2]);
		optimizer.Stop(change_total);
	}
}
/*
void WeekSlotAdvisor::RunSimBroker() {
	System& sys = GetSystem();
	DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()));
	
	Buffer& open_buf	= db->GetBuffer(0);
	Buffer& low_buf		= db->GetBuffer(1);
	Buffer& high_buf	= db->GetBuffer(2);
	Buffer& volume_buf	= db->GetBuffer(3);
	
	int tf = GetTf();
	int data_count = sys.GetCountTf(tf);
	db->ForceCount(data_count);
	
	optimizer.GetLimitedBestSolution(trial);
	NormalizeTrial();
	RefreshMainBuffer(true);
	
	SimBroker sb;
	sb.Brokerage::operator=(GetMetaTrader());
	sb.SetInitialBalance(10000);
	sb.Init();
	sb.SetFreeMarginLevel(FMLEVEL);
	sb.SetFreeMarginScale((MULT_MAXSCALES - 1) * MULT_MAXSCALE_MUL * SYM_COUNT);
	
	RefreshInputs();
	
	for(int i = 0; i < data_count; i++) {
		
		for(int j = 0; j < SYM_COUNT; j++) {
			ConstBuffer& open_buf = *inputs[j];
			double curr = open_buf.GetUnsafe(i);
			int symbol = sys.GetPrioritySymbol(j);
			sb.SetPrice(symbol, curr);
		}
		sb.RefreshOrders();
		
		
		for(int j = 0; j < SYM_COUNT; j++) {
			int dir		= signals[j]->Get(i) < 0.0 ? -1 : +1;
			int mult	= weights[j]->Get(i) * MULT_MAX;
			int sig		= dir * mult;
			int sym		= sys.GetPrioritySymbol(j);
			ASSERT(sig >= -MULT_MAX && sig <= MULT_MAX);
			
			if (sig == sb.GetSignal(sym) && sig != 0)
				sb.SetSignalFreeze(sym, true);
			else {
				sb.SetSignal(sym, sig);
				sb.SetSignalFreeze(sym, false);
			}
		}
		
		
		sb.SignalOrders(true);
		
		
		double value = sb.AccountEquity();
		open_buf.Set(i, value);
		low_buf.Set(i, value);
		high_buf.Set(i, value);
	}
}
*/
void WeekSlotAdvisor::NormalizeTrial() {
	int i = 0;
	while (i < trial.GetCount()) {
		
		double sum = 0.0;
		for(int j = 0; j < SYM_COUNT; j++)
			sum += trial[i + j];
		
		if (sum >= 1.0) {
			double mul = 1.0 / sum;
			ASSERT(IsFin(mul));
			for(int j = 0; j < SYM_COUNT; j++)
				trial[i + j] *= mul;
		}
		
		i += SYM_COUNT;
	}
}

void WeekSlotAdvisor::RefreshMainBuffer(bool forced) {
	if (!forced && trial.IsEmpty()) return;
	
	System& sys = GetSystem();
	int begin = !forced ? prev_counted : 0;
	int tf = GetTf();
	int data_count = sys.GetCountTf(tf);
	ASSERT(weekslots > 0);
	
	for(int j = 0; j < SYM_COUNT; j++)
		GetBuffer(j).SetCount(data_count);
	
	for(int i = begin; i < data_count; i++) {
		Time t					= sys.GetTimeTf(tf, i);
		int wday				= DayOfWeek(t);
		if (wday == 0 || wday == 6)	continue;
		int wdaymins			= ((wday - 1) * 24 + t.hour) * 60 + t.minute;
		int weekslot			= wdaymins / weekslots;
		int trial_read_begin	= weekslot * SYM_COUNT;
		
		for(int j = 0; j < SYM_COUNT; j++) {
			Buffer& buf			= GetBuffer(j);
			double weight		= trial[trial_read_begin + j];
			ASSERT(IsFin(weight));
			buf.Set(i, weight);
		}
	}
}

void WeekSlotAdvisor::RefreshInputs() {
	if (inputs.IsEmpty()) {
		System& sys		= GetSystem();
		int tf			= GetTf();
		inputs			.SetCount(SYM_COUNT, NULL);
		signals			.SetCount(SYM_COUNT, NULL);
		weights			.SetCount(SYM_COUNT, NULL);
		for(int i = 0; i < SYM_COUNT; i++) {
			int symbol				= sys.GetPrioritySymbol(i);
			ConstBuffer& open_buf	= GetInputBuffer(0, symbol, tf, 0);
			ConstBuffer& sig_buf	= GetInputBuffer(1, symbol, tf, 0);
			ConstBuffer& weight_buf	= GetBuffer(i);
			inputs[i]				= &open_buf;
			signals[i]				= &sig_buf;
			weights[i]				= &weight_buf;
		}
	}
}

void WeekSlotAdvisor::RunMain() {
	System& sys = GetSystem();
	DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()));
	
	Buffer& open_buf	= db->GetBuffer(0);
	Buffer& low_buf		= db->GetBuffer(1);
	Buffer& high_buf	= db->GetBuffer(2);
	Buffer& volume_buf	= db->GetBuffer(3);
	
	RefreshInputs();
	
	if (!sb.init)
		sys.SetFixedBroker(sb, -1);
	
	for (int a = 0; a < 3; a++) {
		int begin = a == 0 ? area.train_begin : (a == 1 ? area.test0_begin : area.test1_begin);
		int end   = a == 0 ? area.train_end   : (a == 1 ? area.test0_end   : area.test1_end);
		
		//end--;
		//double change_total	= 0.0;
		
		sb.Reset();
		
		for(int i = begin; i < end; i++) {
			
			/*for(int j = 0; j < SYM_COUNT; j++) {
				ConstBuffer& open_buf = *inputs[j];
				bool signal		= signals[j]->Get(i) < 0.0;
				double curr		= open_buf.GetUnsafe(i);
				double next		= open_buf.GetUnsafe(i + 1);
				double change	= next / curr - 1.0;
				double weight	= weights[j]->Get(i);
				
				if (signal) change *= -1.0;
				
				change			*= weight;
				
				ASSERT(IsFin(change));
				change_total	+= change;
			}*/
			
			for(int j = 0; j < SYM_COUNT; j++) {
				ConstBuffer& open_buf = *inputs[j];
				double curr = open_buf.GetUnsafe(i);
				ASSERT(curr > 0);
				sb.SetPrice(j, curr);
			}
			
			sb.RefreshOrders();
			
			
			for(int j = 0; j < SYM_COUNT; j++) {
				int dir		= signals[j]->Get(i) < 0.0 ? -1 : +1;
				int mult	= weights[j]->Get(i) * MULT_MAX;
				int sig		= dir * mult;
				ASSERT(sig >= -MULT_MAX && sig <= MULT_MAX);
				
				if (sig == sb.GetSignal(j) && sig != 0)
					sb.SetSignalFreeze(j, true);
				else {
					sb.SetSignal(j, sig);
					sb.SetSignalFreeze(j, false);
				}
			}
			
			
			sb.Cycle();
			
			
			double value = sb.AccountEquity();
			open_buf.Set(i, value);
			low_buf.Set(i, value);
			high_buf.Set(i, value);
		}
		
		double change_total = sb.AccountEquity() / sb.begin_equity - 1.0;
		
		area_change_total[a] = change_total;
		LOG("WeekSlotAdvisor::TestMain " << GetSymbol() << " a" << a << ": change_total=" << change_total);
	}
}

void WeekSlotAdvisor::SetTrainingArea() {
	int tf = GetTf();
	int data_count = GetSystem().GetCountTf(tf);
	area.FillArea(data_count);
}

void WeekSlotAdvisor::SetRealArea() {
	ConstBuffer& open_buf	= GetInputBuffer(0, 0);
	int data_count			= open_buf.GetCount();
	area.train_begin		= 1*5*24*60 / MAIN_PERIOD_MINUTES;
	area.train_end			= data_count;
	area.test0_begin		= data_count;
	area.test0_end			= data_count;
	area.test1_begin		= data_count;
	area.test1_end			= data_count;
}

void WeekSlotAdvisor::RefreshMain() {
	SetRealArea();
	optimizer.GetLimitedBestSolution(trial);
	NormalizeTrial();
	RefreshMainBuffer(true);
	RunMain();
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
	
	
	// Check for market closing (weekend and holidays)
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
			int sig = sb.GetSignal(i);
	
			if (sig == mt.GetSignal(sym) && sig != 0)
				mt.SetSignalFreeze(sym, true);
			else {
				mt.SetSignal(sym, sig);
				mt.SetSignalFreeze(sym, false);
			}
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

}
