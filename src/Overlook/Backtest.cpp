#include "Overlook.h"

namespace Overlook {

/*
void SourceProcessingGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	
	const Vector<AccuracyConf>& acc_list = esys.acc_list;
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int count = acc_list.GetCount();
	for(int j = 0; j < count; j++) {
		double d = acc_list[j].test_valuehourfactor;
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	if (count >= 2 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = acc_list[j].test_valuehourfactor;
			int y = (int)(sz.cy - (v - min) / diff * sz.cy);
			int x = (int)(j * xstep);
			polyline[j] = Point(x, y);
		}
		id.DrawPolyline(polyline, 1, Color(193, 255, 255));
		for(int j = 0; j < polyline.GetCount(); j++) {
			const Point& p = polyline[j];
			id.DrawRect(p.x-1, p.y-1, 3, 3, Blue());
		}
	}
	
	id.DrawText(3, 3, IntStr(count), Monospace(15), Black());
	
	w.DrawImage(0, 0, id);
}

*/


BacktestCtrl::BacktestCtrl(Overlook* olook) : olook(olook) {
	win = 0;
	running = 0;
	stopped = true;
	id = -1;
	tf = -1;
	begin_shift = 0;
	//advisor = 0;
	//script = 0;
	
	CtrlLayout(*this);
	
	indi_props.WhenAction = THISBACK(IndicatorProperties);
	sym_props.WhenAction = THISBACK(SymbolProperties);
	open_chart.WhenAction = THISBACK(OpenChart);
	start.WhenAction = THISBACK(Start);
	list.WhenAction = THISBACK(RefreshTarget);
	
	type.Add("Indicator");
	type.Add("Expert Advisor");
	type.Add("Script");
	type.SetIndex(0);
	type.WhenAction = THISBACK(RefreshType);
	
	prog.Set(0,1);
}

void BacktestCtrl::Init() {
	System& sys = GetSystem();
	
	RefreshType();
	
	int count = sys.GetSymbolCount();
	for(int i = 0; i < count; i++) {
		sym_list.Add(sys.GetSymbol(i));
	}
	sym_list.SetIndex(0);
	
	model_list.Add("Control points");
	model_list.SetIndex(0);
	
	count = sys.GetPeriodCount();
	for(int i = 0; i < count; i++) {
		period.Add(sys.GetPeriodString(i));
	}
	period.SetIndex(0);
	
	spread.Add("Current");
	spread.Add("Average");
	spread.SetIndex(0);
}

void BacktestCtrl::RefreshType() {
	int count;
	target_type = type.GetIndex();
	
	list.Clear();
	
	if (target_type == 0) {
		count = System::Indicators().GetCount();
		for(int i = 0; i < count; i++) {
			int id = System::Indicators()[i];
			list.Add(System::CtrlFactories()[id].a);
		}
		indi_props.SetLabel("Indicator properties");
		sym_list.Enable();
	}
	if (target_type == 1) {
		
	}
	else Panic("WTF type");
	
	list.SetIndex(0);
	RefreshTarget();
}

void BacktestCtrl::RefreshTarget() {
	target_id = list.GetIndex();
	if (target_type == 0) {
		
	}
	else if (target_type == 1) {
		
	}
}

void BacktestCtrl::IndicatorProperties() {
	BacktestSettingsDialog dlg(this);
	//dlg.SetSettings(target_type, target_id, indi_settings);
	
	/*
	dlg.Run();
	if (dlg.IsOK() && dlg.IsChanged()) {
		indi_settings = dlg.GetSettings();
	}
	*/
}

void BacktestCtrl::SymbolProperties() {
	/*int id = sym_list.GetIndex();
	Specification spec;
	spec.Init(core, id);
	spec.OpenMain();*/
}

void BacktestCtrl::OpenChart() {
	/*int id = sym_list.GetIndex();
	int tf = period.GetIndex();
	win = &cman->AddChart(id, tf);*/
}

void BacktestCtrl::Start() {
	#if 0
	if (!running) {
		sleep_ms = 0;
		while (!stopped) Sleep(100);
		running = true;
		
		start.SetLabel("Stop");
		
		id = sym_list.GetIndex();
		tf = period.GetIndex();
		if (id == -1 || tf == -1) return;
		
		backtester = &dynamic_cast<BrokerBacktest&>(btb);
		
		Time now = GetSysTime();
		Time begin, end;
		if (use_date.Get()) {
			begin = from.GetData();
			end = to.GetData();
			if (begin > end) begin = end - 60*60;
		} else {
			begin = now - 60*60*24*30;//*3*4;
			end = now;
		}
		global_ordertracker.Clear();
		backtester->DeleteCache();
		backtester->SetBegin(begin);
		backtester->SetEnd(end);
		begin_shift = backtester->GetPriceCount(id, tf);
		cman->UseBacktestBroker(true);
		cman->CloseBacktestWindows();
		
		int type = this->type.GetIndex();
		if (type == 0) {
			win = &cman->AddChart(btb, id, tf);
			sleep_ms = 500;
			// TODO: add indicator
		}
		else Panic("TODO");
		
		/*else if (type == 1) {
			sleep_ms = 0;
			int adv = list.GetIndex();
			const Settings& as = btb.GetAdvisorSettings(adv);
			Advisor& ad = btb.GetAdvisor(adv, id, tf, as.GetValues());
			AdvisorCtrl& ac = cman->AddSubWindow<AdvisorCtrl>();
			ac.Init(ad);
			win = &ac;
			advisor = &ad;
			advisorctrl = &ac;
			backtester->SetSkippingSymbol(id);
		}
		else if (type == 2) {
			sleep_ms = 0;
			int sc_id = list.GetIndex();
			const Settings& ss = btb.GetScriptSettings(sc_id);
			const Vector<double>& values = ss.GetValues();
			Script& sc = btb.GetScript(sc_id, values);
			ScriptCtrl& scc = cman->AddSubWindow<ScriptCtrl>();
			scc.Init(sc);
			win = &scc;
			script = &sc;
			scriptctrl = &scc;
		}*/
		
		if (!thrd.IsEmpty()) {
			// Reset if old thread is stopped
			ASSERT(stopped); // see 20 lines higher
			thrd.Clear();
			thrd.Create();
		} else {
			thrd.Create();
		}
		ASSERT(!thrd->IsOpen());
		thrd->Run(THISBACK(Loop));
	} else {
		Stop();
	}
	#endif
}

void BacktestCtrl::Loop() {
	stopped = false;
	
	TimeStop ts;
	while (!Thread::IsShutdownThreads() && running) {
		
		/*if (!backtester->Next()) {
			running = false;
			PostCallback(THISBACK(Stop));
			break;
		}*/
		
#if 0
		if (advisor)
			advisor->Refresh();
		
		if (script)
			script->Refresh();
#endif
		
		if (sleep_ms) Sleep(sleep_ms);
		
		if (ts.Elapsed() > 500) {
			olook->cman.PostRefreshBacktestWindows();
			PostProgressRefresh();
			ts.Reset();
		}
		
	}
	stopped = true;
}

void BacktestCtrl::PostProgressRefresh() {
	PostCallback(THISBACK(RefreshProgress));
}

void BacktestCtrl::RefreshProgress() {
	/*if (win) {
		int actual = btb.GetPriceCount(id, tf) - begin_shift;
		int total = GetPriceCount(id, tf) - begin_shift;
		prog.Set(actual, total);
	}*/
}

void BacktestCtrl::Stop() {
	olook->cman.UseBacktestBroker(false);
	start.SetLabel("Start");
	running = false;
	win = NULL;
	prog.Set(0,1);
}












BacktestSettingsDialog::BacktestSettingsDialog(BacktestCtrl* bc) : bc(bc) {
	CtrlLayout(*this);
	is_ok = false;
	is_changed = false;
	
	CtrlLayout(testing);
	
	
	inputs.Add(input_values.HSizePos().VSizePos(0,30));
	inputs.Add(input_load.BottomPos(0,30).RightPos(100,100));
	inputs.Add(input_save.BottomPos(0,30).RightPos(0,100));
	
	input_values.AddColumn("Variable");
	input_values.AddColumn("Value").Edit(edit_value);
	input_values.AddColumn("Start").Edit(edit_start);
	input_values.AddColumn("Stop").Edit(edit_stop);
	input_values.AddColumn("Step").Edit(edit_step);
	input_values.ColumnWidths("2 1 1 1 1");
	edit_value.WhenAction = THISBACK(EditsValue);
	edit_start.WhenAction = THISBACK(EditsValue);
	edit_stop.WhenAction = THISBACK(EditsValue);
	edit_step.WhenAction = THISBACK(EditsValue);
	
	input_load.SetLabel("Load");
	input_save.SetLabel("Save");
	
	input_load.WhenAction = THISBACK(LoadInputs);
	input_save.WhenAction = THISBACK(SaveInputs);
	
	
	
	
	optimization.AddColumn("Limitation");
	optimization.AddColumn("Value");
	optimization.ColumnWidths("2 1");
	
	
	ok.WhenAction = THISBACK(OK);
	cancel.WhenAction = THISBACK(Cancel);
	reset.WhenAction = THISBACK(Reset);
}

#if 0
void BacktestSettingsDialog::SetSettings(int type, int id, Settings& is) {
	
	if (type != 0) {
		tabs.Add(testing);
		tabs.Add(testing, "Testing");
	}
	
	int tab_index = tabs.GetCount();
	
	tabs.Add(inputs);
	tabs.Add(inputs, "Inputs");
	const Vector<String>& names = is.GetNames();
	for(int i = 0; i < names.GetCount(); i++) {
		Option& o = input_opts.Add();
		o.WhenAction = THISBACK1(SelectInputLine, i);
		o.SetLabel(names[i]);
		input_values.SetCtrl(i, 0, o);
		input_values.Set(i, 1, is.GetValues()[i]);
		input_values.Set(i, 2, is.GetMins()[i]);
		input_values.Set(i, 3, is.GetMaxs()[i]);
		input_values.Set(i, 4, 0.01);
	}
	
	
	if (type != 0) {
		tabs.Add(optimization);
		tabs.Add(optimization, "Optimization");
	}
	
	/*
	if (type == 0)
		Title(bc->core->GetBacktestBroker().GetIndiName(id));
	else if (type == 1)
		Title(bc->core->GetBacktestBroker().GetAdvisorName(id));
	else if (type == 2)
		Title(bc->core->GetBacktestBroker().GetScriptName(id));*/
	
	tabs.Set(tab_index);
	
}
#endif


void BacktestSettingsDialog::LoadInputs() {
	
}

void BacktestSettingsDialog::SaveInputs() {
	
}

void BacktestSettingsDialog::OK() {
	is_ok = true;
	Close();
}

void BacktestSettingsDialog::Cancel() {
	Close();
}

void BacktestSettingsDialog::Reset() {
	is_changed = false;
	
}
/*
Settings BacktestSettingsDialog::GetSettings() {
	Settings out;
	for(int i = 0; i < input_values.GetCount(); i++) {
		double value = input_values.Get(i, 1);
		double start = input_values.Get(i, 2);
		double stop  = input_values.Get(i, 3);
		double step  = input_values.Get(i, 4);
		String name = input_opts[i].GetLabel();
		out.Add(value, name, start, stop);
	}
	return out;
}
*/
}
