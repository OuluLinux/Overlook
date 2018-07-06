#include "Overlook.h"

namespace Overlook {

ExpertAdvisor::ExpertAdvisor() {

}

void ExpertAdvisor::Init() {
	String factory_str = GetSystem().CoreFactories()[GetFactory()].a;
	String tf_str = factory_str + " " + GetSystem().GetPeriodString(GetTf()) + " ";
	
	SetJobCount(1);
	
	point = GetDataBridge()->GetPoint();
	
	SetJob(0, tf_str + " Training")
		.SetBegin(THISBACK(TrainingBegin))
		.SetIterator(THISBACK(TrainingIterator))
		.SetEnd(THISBACK(TrainingEnd))
		.SetInspect(THISBACK(TrainingInspect))
		.SetCtrl		<TrainingCtrl>();
}

void ExpertAdvisor::Start() {
	if (once) {
		if (prev_counted > 0)
			prev_counted--;
			
		once = false;
		
		//RefreshGrid(true);
		RefreshSourcesOnlyDeep();
	}
	
	if (IsJobsFinished()) {
		int bars = GetBars();
		
		if (prev_counted < bars) {
			LOG("ExpertAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

bool ExpertAdvisor::TrainingBegin() {
	SetAvoidRefresh();
	
	if (round == 0) {
		sb.Brokerage::operator=(GetMetaTrader());
		sb.Init();
		
		opt.Min().SetCount(args.GetCount());
		opt.Max().SetCount(args.GetCount());
		for(int i = 0; i < args.GetCount(); i++) {
			opt.Min()[i] = args[i].min;
			opt.Max()[i] = args[i].max;
		}
		
		opt.SetMaxGenerations(100);
		opt.Init(args.GetCount(), 33);
	}
	max_rounds = opt.GetMaxRounds();
	
	training_pts.SetCount(max_rounds, 0);
	
	// Allow iterating
	return true;
}

bool ExpertAdvisor::TrainingIterator() {

	// Show progress
	GetCurrentJob().SetProgress(round, max_rounds);
	
	
	// ---- Do your training work here ----
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	ConstBuffer& time_buf = GetInputBuffer(0, 4);
	
	
	opt.Start();
	const Vector<double>& trial = opt.GetTrialSolution();
	for(int i = 0; i < args.GetCount(); i++) {
		ArgPtr& ap = args[i];
		int value = min(ap.max, max(ap.min, (int)trial[i]));
		*ap.ptr = value;
	}
	
	try {
		ResetSubCores();
		sb.Clear();
		InitEA();
		InitSubCores();
		int count = open_buf.GetCount(); // take count before updating dependencies to match their size
		RefreshSubCores();
		
		Point = point;
		for (Digits = 1; ; Digits++) {
			double d = point * pow(10, Digits);
			if (d >= 1.0)
				break;
		}
		ASSERT(Digits >= 0);
		Bars = count;
		
		TimeStop ts;
		for(int i = 0; i < count; i++) {
			Ask = open_buf.Get(i);
			Bid = Ask - 3 * point;
			Now = Time(1970,1,1) + time_buf.Get(i);
			sb.SetPrice(GetSymbol(), Ask, Bid);
			sb.RefreshOrders();
			//sb.CycleChanges();
			StartEA(i);
			//sb.Cycle();
			if (ts.Elapsed() > 1000)
				throw ConfExc();
		}
		
		double eq = max(0.0, sb.AccountEquity());
		training_pts[round] = eq + Random(2);
		opt.Stop(eq);
	}
	catch (ConfExc e) {
		LOG(e);
		opt.Stop(-DBL_MAX);
	}
	
	
	
	// Keep count of iterations
	round++;
	
	// Stop eventually
	if (round >= max_rounds) {
		SetJobFinished();
	}
	
	if (save_elapsed.Elapsed() > 60*1000) {
		serialization_lock.Leave();
		save_elapsed.Reset();
		StoreCache();
		serialization_lock.Enter();
	}
	
	return true;
}

bool ExpertAdvisor::TrainingEnd() {
	double max_d = -DBL_MAX;
	int max_i = 0;
	
	
	RefreshAll();
	
	SetAvoidRefresh(false);
	
	serialization_lock.Leave();
	StoreCache();
	serialization_lock.Enter();
		
	return true;
}

bool ExpertAdvisor::TrainingInspect() {
	bool success = false;
	
	INSPECT(success, "ok: this is an example");
	INSPECT(success, "warning: this is an example");
	INSPECT(success, "error: this is an example");
	
	// You can fail the inspection too
	//if (!success) return false;
	
	return true;
}

void ExpertAdvisor::RefreshAll() {
	/*
	RefreshSourcesOnlyDeep();
	ConstBuffer& timebuf = GetInputBuffer(0, 4);
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	VectorBool& signal  = GetOutput(0).label;
	VectorBool& enabled = GetOutput(1).label;
	Buffer& out = GetBuffer(0);
	
	// ---- Do your final result work here ----
	//RefreshBits();
	//SortByValue(results, GridResult());
	
	double spread = GetDataBridge()->GetSpread();
	if (spread == 0)
		spread = GetDataBridge()->GetPoint() * 2.0;
	
	int bars = GetBars();
	signal.SetCount(bars);
	enabled.SetCount(bars);
	
	bool initial = prev_counted == 0;
	if (prev_counted < input_length) prev_counted = input_length;
	
	if (have_othersyms) {
		for(int i = 0; i < inputs[1].GetCount(); i++) {
			bars = min(bars, inputs[1][i].core->GetBuffer(0).GetCount());
		}
	}
	
	for(int i = prev_counted; i < bars; i++) {
		
		LoadInput(i);
		
		int sig, mult = 1;
		
		int action = dqn.Act(tmp_mat);
		sig = action ? -1 : +1;
		
		
		signal. Set(i, sig <  0);
		enabled.Set(i, sig != 0);
		
		
		if (i < bars-1) {
			if (sig > 0) {
				total += (+(open_buf.Get(i+1) - open_buf.Get(i)) - spread) * mult;
			}
			else if (sig < 0) {
				total += (-(open_buf.Get(i+1) - open_buf.Get(i)) - spread) * mult;
			}
		}
		out.Set(i, total);
	}
	
	#ifndef flagDEBUG
	if (do_test && initial) {
		DumpTest();
	}
	#endif
	
	//int sig = enabled.Get(bars-1) ? (signal.Get(bars-1) ? -1 : +1) : 0;
	//GetSystem().SetSignal(GetSymbol(), sig);
	//ReleaseLog("ExpertAdvisor::RefreshAll symbol " + IntStr(GetSymbol()) + " sig " + IntStr(sig));
	
	
	// Keep counted manually
	prev_counted = bars;
	
	
	//DUMP(main_interval);
	//DUMP(grid_interval);
	*/
}

void ExpertAdvisor::TrainingCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	ExpertAdvisor* ea = dynamic_cast<ExpertAdvisor*>(&*job->core);
	ASSERT(ea);
	
	Size graphsz(sz.cx, sz.cy);
	DrawVectorPolyline(id, graphsz, ea->training_pts, polyline, ea->round);
	
	w.DrawImage(0, 0, id);
}

}
