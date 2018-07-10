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
		sb.SetInitialBalance(1000);
		
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
		
		besteq_pts.SetCount(count, 0);
		cureq_pts.SetCount(count, 0);
		
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
			Bars = i + 1;
			sb.SetPrice(GetSymbol(), Ask, Bid);
			sb.RefreshOrders();
			//sb.CycleChanges();
			StartEA(i);
			//sb.Cycle();
			if (ts.Elapsed() > 1000)
				throw ConfExc();
			if (sb.AccountEquity() < 0.0) {
				sb.CloseAll();
				break;
			}
			cureq_pts[i] = sb.AccountEquity();
		}
		
		
		double eq = max(0.0, sb.AccountEquity());
		training_pts[round] = eq + Random(2);
		bool is_best = eq >= opt.GetBestEnergy();
		opt.Stop(eq);
		
		if (is_best) {
			for(int i = 0; i < count; i++)
				besteq_pts[i] = cureq_pts[i];
		}
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
	RefreshSourcesOnlyDeep();
	ConstBuffer& time_buf = GetInputBuffer(0, 4);
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	LabelSignal& signal = GetLabelBuffer(0, 0);
	
	const Vector<double>& trial = opt.GetBestSolution();
	for(int i = 0; i < args.GetCount(); i++) {
		ArgPtr& ap = args[i];
		int value = min(ap.max, max(ap.min, (int)trial[i]));
		*ap.ptr = value;
	}
	
	int bars = open_buf.GetCount(); // take count before updating dependencies to match their size
		
	if (prev_counted == 0) {
		sb.Clear();
	}
	
	if (subcores.IsEmpty() || prev_counted == 0) {
		ResetSubCores();
		InitEA();
		InitSubCores();
	}
	RefreshSubCores();


	// ---- Do your final result work here ----
	//RefreshBits();
	//SortByValue(results, GridResult());
	
	double spread = GetDataBridge()->GetSpread();
	if (spread == 0)
		spread = GetDataBridge()->GetPoint() * 2.0;
	
	
	/*if (have_othersyms) {
		for(int i = 0; i < inputs[1].GetCount(); i++) {
			bars = min(bars, inputs[1][i].core->GetBuffer(0).GetCount());
		}
	}*/
	
	cureq_pts.SetCount(bars, 0);
	lots_pts.SetCount(bars, 0);
	
	for(int i = prev_counted; i < bars; i++) {
		Ask = open_buf.Get(i);
		Bid = Ask - 3 * point;
		Now = Time(1970,1,1) + time_buf.Get(i);
		Bars = i + 1;
		sb.SetPrice(GetSymbol(), Ask, Bid);
		sb.RefreshOrders();
		//sb.CycleChanges();
		StartEA(i);
		//sb.Cycle();
		if (sb.AccountEquity() < 0.0) {
			sb.CloseAll();
			break;
		}
		cureq_pts[i] = sb.AccountEquity();
		
		double vol = 0.0;
		for(int i = 0; i < sb.OrdersTotal(); i++) {
			sb.OrderSelect(i, libmt::SELECT_BY_POS);
			int type = sb.OrderType();
			if (type == OP_BUY)
				vol += sb.OrderLots();
			else if (type == OP_SELL)
				vol -= sb.OrderLots();
		}
		lots_pts[i] = vol;
		signal.signal.Set(i, vol < 0.0);
		signal.enabled.Set(i, vol != 0.0);
	}
	
	// Keep counted manually
	prev_counted = bars;
	
	
	//DUMP(main_interval);
	//DUMP(grid_interval);
}

ExpertAdvisor::TrainingCtrl::TrainingCtrl() {
	Add(tabs.SizePos());
	tabs.Add(opt0.SizePos(), "Optimization");
	tabs.Add(opt1.SizePos(), "Best combination");
	tabs.Add(opt2.SizePos(), "Current combination");
	tabs.Add(opt3.SizePos(), "Current lots");
	opt0.type = 0;
	opt1.type = 1;
	opt2.type = 2;
	opt3.type = 3;
	opt0.job = this;
	opt1.job = this;
	opt2.job = this;
	opt3.job = this;
}

void ExpertAdvisor::OptimizationCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	ExpertAdvisor* ea = dynamic_cast<ExpertAdvisor*>(&*job->job->core);
	ASSERT(ea);
	
	Size graphsz(sz.cx, sz.cy);
	
	if (type == 0) DrawVectorPolyline(id, graphsz, ea->training_pts, polyline, ea->round);
	if (type == 1) DrawVectorPolyline(id, graphsz, ea->besteq_pts, polyline);
	if (type == 2) DrawVectorPolyline(id, graphsz, ea->cureq_pts, polyline);
	if (type == 3) DrawVectorPolyline(id, graphsz, ea->lots_pts, polyline);
	
	w.DrawImage(0, 0, id);
}

}
