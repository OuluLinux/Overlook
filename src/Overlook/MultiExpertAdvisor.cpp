#include "Overlook.h"

namespace Overlook {


MultiExpertAdvisor::MultiExpertAdvisor() {

}

void MultiExpertAdvisor::Init() {
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

void MultiExpertAdvisor::Start() {
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
			LOG("MultiExpertAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

bool MultiExpertAdvisor::TrainingBegin() {
	SetAvoidRefresh();
	
	if (!IsDependencyJobsFinished())
		return false;
	
	if (round == 0) {
		sb.Brokerage::operator=(GetMetaTrader());
		sb.Init();
		sb.SetInitialBalance(1000.0);
		
		int opt_size = (inputs.GetCount()-1) * args_per_ea + 1;
		
		opt.Min().SetCount(opt_size);
		opt.Max().SetCount(opt_size);
		
		int row = 0;
		for(int i = 0; i < (inputs.GetCount()-1); i++) {
			opt.Min()[row] = 0.0;
			opt.Max()[row++] = 2.0;
			opt.Min()[row] = 1.0;
			opt.Max()[row++] = 1000.0;
		}
		opt.Min()[row] = 1.0;
		opt.Max()[row++] = 1000.;
		
		opt.SetMaxGenerations(750);
		opt.Init(opt_size, 50);
	}
	max_rounds = opt.GetMaxRounds();
	
	training_pts.SetCount(max_rounds, 0);
	
	// Allow iterating
	return true;
}

bool MultiExpertAdvisor::TrainingIterator() {

	// Show progress
	GetCurrentJob().SetProgress(round, max_rounds);
	
	
	// ---- Do your training work here ----
	
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	ConstBuffer& time_buf = GetInputBuffer(0, 4);
	int count = open_buf.GetCount();
	
	
	opt.Start();
	const Vector<double>& trial = opt.GetTrialSolution();
	
	
	// Get EA eq and lot vectors
	Vector<ExpertAdvisor*> eas;
	for(int i = 1; i < inputs.GetCount(); i++) {
		const Input& input = inputs[i];
		ExpertAdvisor* ea = dynamic_cast<ExpertAdvisor*>(input[0].core);
		eas.Add(ea);
		count = min(count, ea->lots_pts.GetCount());
	}
	
	
	try {
		sb.Clear();
		
		besteq_pts.SetCount(count, 0);
		cureq_pts.SetCount(count, 0);
		
		
		for(int i = 1; i < count; i++) {
			double ask = open_buf.Get(i);
			double bid = ask - 3 * point;
			sb.SetPrice(GetSymbol(), ask, bid);
			sb.RefreshOrders();
			//sb.CycleChanges();
			
			double lot_sum = 0.0, div = 0.0;
			bool lots_changed = false;
			for(int j = 0; j < eas.GetCount(); j++) {
				ExpertAdvisor& ea = *eas[j];
				double mult1 = trial[j * args_per_ea + 0];
				double offset = trial[j * args_per_ea + 1];
				double lots = ea.lots_pts[i];
				double prev_lots = ea.lots_pts[i-1];
				double eq = ea.cureq_pts[i];
				double initeq = ea.cureq_pts[0];
				
				mult1 = max(0.0, min(2.0, mult1));
				
				double l = mult1 * lots * (initeq - offset) / (eq - offset);
				lot_sum += l;
				div += mult1;
				
				lots_changed |= lots != prev_lots;
			}
			double eq = sb.AccountEquity();
			double mult2 = trial[eas.GetCount() * args_per_ea + 0];
			
			double sb_lots = lot_sum / div * eq / mult2;
			sb_lots = ((int64)(sb_lots * 100)) * 0.01;
			
			if (lots_changed) {
				sb.RealizeVolume(GetSymbol(), fabs(sb_lots), sb_lots < 0.0);
			}
			
			//sb.Cycle();
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

bool MultiExpertAdvisor::TrainingEnd() {
	double max_d = -DBL_MAX;
	int max_i = 0;
	
	
	RefreshAll();
	
	SetAvoidRefresh(false);
	
	serialization_lock.Leave();
	StoreCache();
	serialization_lock.Enter();
		
	return true;
}

bool MultiExpertAdvisor::TrainingInspect() {
	bool success = false;
	
	INSPECT(success, "ok: this is an example");
	INSPECT(success, "warning: this is an example");
	INSPECT(success, "error: this is an example");
	
	// You can fail the inspection too
	//if (!success) return false;
	
	return true;
}

void MultiExpertAdvisor::RefreshAll() {
	RefreshSourcesOnlyDeep();
	ConstBuffer& time_buf = GetInputBuffer(0, 4);
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	LabelSignal& signal = GetLabelBuffer(0, 0);
	
	int bars = open_buf.GetCount(); // take count before updating dependencies to match their size
		
	Vector<ExpertAdvisor*> eas;
	for(int i = 1; i < inputs.GetCount(); i++) {
		const Input& input = inputs[i];
		ExpertAdvisor* ea = dynamic_cast<ExpertAdvisor*>(input[0].core);
		eas.Add(ea);
		bars = min(bars, ea->GetLabelBuffer(0,0).signal.GetCount());
	}
	
	const Vector<double>& trial = opt.GetBestSolution();
	
	
	if (prev_counted == 0) {
		prev_counted++;
		sb.Clear();
	}
	
	String symstr = GetSystem().GetSymbol(GetSymbol());

	// ---- Do your final result work here ----
	
	cureq_pts.SetCount(bars, 0);
	
	for(int i = prev_counted; i < bars; i++) {
		Time t = Time(1970,1,1) + time_buf.Get(i);
		double ask = open_buf.Get(i);
		double bid = ask - 3 * point;
		sb.SetPrice(GetSymbol(), ask, bid);
		sb.RefreshOrders();
		//sb.CycleChanges();
		
		double lot_sum = 0.0, div = 0.0;
		bool lots_changed = false;
		for(int j = 0; j < eas.GetCount(); j++) {
			ExpertAdvisor& ea = *eas[j];
			double mult1 = trial[j * args_per_ea + 0];
			double offset = trial[j * args_per_ea + 1];
			double lots = ea.lots_pts[i];
			double prev_lots = ea.lots_pts[i-1];
			double eq = ea.cureq_pts[i];
			double initeq = ea.cureq_pts[0];
			
			mult1 = max(0.0, min(2.0, mult1));
			
			double l = mult1 * lots * (initeq - offset) / (eq - offset);
			lot_sum += l;
			div += mult1;
			
			lots_changed |= lots != prev_lots;
		}
		double eq = sb.AccountEquity();
		double mult2 = trial[eas.GetCount() * args_per_ea + 0];
		
		double sb_lots = lot_sum / div * eq / mult2;
		sb_lots = ((int64)(sb_lots * 100)) * 0.01;
		
		if (lots_changed) {
			sb.RealizeVolume(GetSymbol(), fabs(sb_lots), sb_lots < 0.0);
		}
		
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
		
		signal.signal.Set(i, vol < 0.0);
		signal.enabled.Set(i, vol != 0.0);
		
		lots_changed |= signal.signal.Get(i) != signal.signal.Get(i-1);
		
		//if (i == bars-1 && (lots_changed || !GetMetaTrader().IsSymbolTrading(GetSymbol()))) {
		if (i == bars-1) {
			MetaTrader& mt = GetMetaTrader();
			double mt_vol = vol * mt.AccountEquity() / sb.AccountEquity();
			mt_vol += mt_vol > 0 ? 0.005 : -0.005;
			mt.RealizeVolume(GetSymbol(), fabs(mt_vol), mt_vol < 0.0);
			ReleaseLog("MultiExpertAdvisor " + GetSystem().GetSymbol(GetSymbol()) + " lots " + DblStr(mt_vol) + " " + IntStr(i) + "/" + IntStr(bars) + " " + Format("%", t));
		}
	}
	
	// Keep counted manually
	prev_counted = bars;
	
	
	
	//DUMP(main_interval);
	//DUMP(grid_interval);
}

MultiExpertAdvisor::TrainingCtrl::TrainingCtrl() {
	Add(tabs.SizePos());
	tabs.Add(opt0.SizePos(), "Optimization");
	tabs.Add(opt1.SizePos(), "Best combination");
	tabs.Add(opt2.SizePos(), "Current combination");
	opt0.type = 0;
	opt1.type = 1;
	opt2.type = 2;
	opt0.job = this;
	opt1.job = this;
	opt2.job = this;
}

void MultiExpertAdvisor::OptimizationCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	MultiExpertAdvisor* ea = dynamic_cast<MultiExpertAdvisor*>(&*job->job->core);
	ASSERT(ea);
	
	Size graphsz(sz.cx, sz.cy);
	
	if (type == 0) DrawVectorPolyline(id, graphsz, ea->training_pts, polyline, ea->round);
	if (type == 1) DrawVectorPolyline(id, graphsz, ea->besteq_pts, polyline);
	if (type == 2) DrawVectorPolyline(id, graphsz, ea->cureq_pts, polyline);
	
	w.DrawImage(0, 0, id);
}

}
