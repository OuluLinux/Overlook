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
		SetBufferColor(i, Blend(GrayColor(), RainbowColor((double)i / SYM_COUNT)));
		SetBufferLineWidth(i, 2);
	}
	
	tfmins		= GetMinutePeriod();
	slotmins	= tfmins < 60 ? 60 : tfmins;
	weekslots	= 5 * 24 * 60 / slotmins;
	cols		= weekslots * SYM_COUNT;
	
	
	SetJobCount(1);
	SetJob(0, "Main optimization")
		.SetBegin		(THISBACK(MainOptimizationBegin))
		.SetIterator	(THISBACK(MainOptimizationIterator))
		.SetEnd			(THISBACK(MainOptimizationEnd))
		.SetInspect		(THISBACK(MainOptimizationInspect))
		.SetCtrl		<MainOptimizationCtrl>();
}

void WeekSlotAdvisor::Start() {
	if (once) {
		once = false;
		if (prev_counted) prev_counted--;
		//Reset(); // For developing
	}
	
	if (IsJobsFinished()) {
		int bars = GetBars();
		if (prev_counted < bars) {
			LOG("WeekSlotAdvisor::Start Refresh");
			RefreshMain();
			prev_counted = bars;
		}
		else {
			// TODO: dynamic sl/tp stuff
		}
	}
}

void WeekSlotAdvisor::Reset() {
	forced_optimizer_reset = true;
	Job& optjob = GetJob(0);
	optjob.actual = 0;
	optjob.total  = 1;
	optjob.state  = 0;
}

bool WeekSlotAdvisor::MainOptimizationBegin() {
	System& sys = GetSystem();
	
	// Begin fails until sources are processed
	bool all_ready = true;
	for(int i = 0; i < SYM_COUNT; i++) {
		int symbol = sys.GetPrioritySymbol(i);
		CoreIO* core = GetInputCore(1, symbol, GetTf());
		ASSERT(core);
		DqnAdvisor* rfa = dynamic_cast<DqnAdvisor*>(core);
		ASSERT(rfa);
		all_ready &= rfa->IsJobsFinished();
	}
	if (!all_ready)
		return false;


	SetRealArea();
	
	// Init genetic optimizer
	if (optimizer.GetRound() == 0 || forced_optimizer_reset) {
		forced_optimizer_reset = false;
		
		optimizer.SetArrayCount(1);
		optimizer.SetCount(cols);
		optimizer.SetPopulation(100);
		optimizer.SetMaxGenerations(100);
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
		optimizer.Init(StrategyBest1Exp);
	}
	
	optimization_pts.SetCount(10000, 0.0);
	
	return true;
}

bool WeekSlotAdvisor::MainOptimizationIterator() {
	GetCurrentJob().SetProgress(optimizer.GetRound(), optimizer.GetMaxRounds());
	
	
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
	
	
	if (optimizer.IsEnd())
		SetJobFinished();
	
	optimization_pts[optimizer.GetRound() % optimization_pts.GetCount()] = area_change_total[0];
	
	return true;
}

bool WeekSlotAdvisor::MainOptimizationEnd() {
	ASSERT(optimizer.IsEnd());
	LOG("WeekSlotAdvisor::MainOptimization finished");
	optimizer.GetLimitedBestSolution(trial);
	NormalizeTrial();
	
	ForceSetCounted(0);
	RefreshMainBuffer(false);
	RunMain();
	#ifdef ACCURACY
	RunSimBroker();
	#endif
	
	return true;
}

bool WeekSlotAdvisor::MainOptimizationInspect() {
	bool succ = area_change_total[1] > 0.0;
	
	INSPECT( succ, "warning: negative result (" + DblStr(succ) + ")");
	//INSPECT(!succ, "ok: nice result (" + DblStr(succ) + ")");
	
	return true;
}

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
		int weekslot			= wdaymins / slotmins;
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
		enabled			.SetCount(SYM_COUNT, NULL);
		weights			.SetCount(SYM_COUNT, NULL);
		spread_point	.SetCount(SYM_COUNT, 0);
		for(int i = 0; i < SYM_COUNT; i++) {
			int symbol					= sys.GetPrioritySymbol(i);
			ConstBuffer& open_buf		= GetInputBuffer(0, symbol, tf, 0);
			ConstBuffer& weight_buf		= GetBuffer(i);
			CoreIO* core				= CoreIO::GetInputCore(1, symbol, tf);
			ConstVectorBool& sig_buf	= core->GetOutput(0).label;
			ConstVectorBool& ena_buf	= core->GetOutput(1).label;
			inputs[i]					= &open_buf;
			signals[i]					= &sig_buf;
			enabled[i]					= &ena_buf;
			weights[i]					= &weight_buf;
			spread_point[i]				= dynamic_cast<DqnAdvisor*>(core)->GetSpreadPoint();
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
	
	#ifndef ACCURACY
	if (!sb.init)
		sys.SetFixedBroker(sb, -1);
	#endif
	
	for (int a = 0; a < 3; a++) {
		int begin = a == 0 ? area.train_begin : (a == 1 ? area.test0_begin : area.test1_begin);
		int end   = a == 0 ? area.train_end   : (a == 1 ? area.test0_end   : area.test1_end);
		
		
		#ifdef ACCURACY
		end--;
		double change_total	= 1.0;
		#else
		sb.Reset();
		#endif
		
		if (!begin) begin++;
		
		for(int i = begin; i < end; i++) {
			
			#ifdef ACCURACY
			double sym_change = 0;
			for(int j = 0; j < SYM_COUNT; j++) {
				ConstBuffer& open_buf	= *inputs[j];
				bool signal				= signals[j]->Get(i);
				bool is_enabled			= enabled[j]->Get(i);
				bool prev_signal		= signals[j]->Get(i - 1);
				bool prev_is_enabled	= enabled[j]->Get(i - 1);
				
				if (is_enabled) {
					double curr		= open_buf.GetUnsafe(i);
					double next		= open_buf.GetUnsafe(i + 1);
					double spread_point = this->spread_point[j];
					ASSERT(curr > 0.0);
					double change;
					
					if (!prev_is_enabled || prev_signal != signal) {
						if (!signal)	change = next / (curr + spread_point) - 1.0;
						else			change = 1.0 - next / (curr - spread_point);
					} else {
						if (!signal)	change = next / curr - 1.0;
						else			change = 1.0 - next / curr;
					}
					
					double weight	= weights[j]->Get(i);
					
					if (signal) change *= -1.0;
					
					change			*= weight;
					
					ASSERT(IsFin(change));
					sym_change		+= change;
				}
			}
			change_total		*= 1.0 + sym_change;
			if (change_total < 0.0) change_total = 0.0;
			
			open_buf.Set(i, change_total);
			low_buf.Set(i, change_total);
			high_buf.Set(i, change_total);
			#else
			for(int j = 0; j < SYM_COUNT; j++) {
				ConstBuffer& open_buf = *inputs[j];
				double curr = open_buf.GetUnsafe(i);
				ASSERT(curr > 0);
				sb.SetPrice(j, curr);
			}
			
			sb.RefreshOrders();
			
			
			for(int j = 0; j < SYM_COUNT; j++) {
				int sig = 0;
				if (enabled[j]->Get(i)) {
					int dir		= signals[j]->Get(i) ? -1 : +1;
					int mult	= weights[j]->Get(i) * MULT_MAX;
					sig			= dir * mult;
					ASSERT(sig >= -MULT_MAX && sig <= MULT_MAX);
				}
				
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
			#endif
		}
		
		#ifdef ACCURACY
		
		#else
		double change_total = sb.AccountEquity() / sb.begin_equity - 1.0;
		#endif
		
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
	int week				= 1*5*24*60 / MAIN_PERIOD_MINUTES;
	ConstBuffer& open_buf	= GetInputBuffer(0, 0);
	int data_count			= open_buf.GetCount();
	area.train_begin		= week;
	area.train_end			= data_count - 2*week;
	area.test0_begin		= data_count - 2*week;
	area.test0_end			= data_count - 1*week;
	area.test1_begin		= data_count - 1*week;
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
	
	
	#ifdef ACCURACY
	RunSimBroker();
	#endif
	
	try {
		mt.Data();
		mt.RefreshLimits();
		for (int i = 0; i < SYM_COUNT; i++) {
			int sym = sys.GetPrioritySymbol(i);
			
			#ifdef ACCURACY
			int sig = sb.GetSignal(sym);
			#else
			int sig = sb.GetSignal(i);
			#endif
			
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

void WeekSlotAdvisor::MainOptimizationCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	WeekSlotAdvisor* wsa = dynamic_cast<WeekSlotAdvisor*>(&*job->core);
	ASSERT(wsa);
	DrawVectorPolyline(id, sz, wsa->optimization_pts, polyline);
	
	w.DrawImage(0, 0, id);
}


#ifdef ACCURACY
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
	
	optimizer.GetLimitedBestSolution(trial);
	NormalizeTrial();
	RefreshMainBuffer(true);
	
	
	sb.Brokerage::operator=(GetMetaTrader());
	sb.SetInitialBalance(10000);
	sb.Init();
	sb.SetFreeMarginLevel(FMLEVEL);
	sb.SetFreeMarginScale((MULT_MAXSCALES - 1) * MULT_MAXSCALE_MUL * SYM_COUNT);
	
	RefreshInputs();
	
	for(int i = 0; i < bars; i++) {
		
		for(int j = 0; j < SYM_COUNT; j++) {
			ConstBuffer& open_buf = *inputs[j];
			double curr = open_buf.GetUnsafe(i);
			int symbol = sys.GetPrioritySymbol(j);
			sb.SetPrice(symbol, curr);
		}
		sb.RefreshOrders();
		
		
		for(int j = 0; j < SYM_COUNT; j++) {
			int sig = 0;
			if (enabled[j]->Get(i)) {
				int dir		= signals[j]->Get(i) ? -1 : +1;
				int mult	= weights[j]->Get(i) * MULT_MAX;
				sig			= dir * mult;
			}
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
		
		LOG("SB " << i << " eq " << value);
	}
}
#endif


}


