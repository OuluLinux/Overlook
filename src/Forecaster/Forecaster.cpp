#include "Forecaster.h"
#include <plugin/bz2/bz2.h>

namespace Forecast {

ManagerCtrl::ManagerCtrl() {
	
	Title("ManagerCtrl");
	Sizeable().MaximizeBox().MinimizeBox();
	
	hsplit.Horz();
	vsplit.Vert();
	Add(hsplit.SizePos());
	hsplit << vsplit << main_task;
	hsplit.SetPos(2000);
	
	vsplit << seslist << tasklist;
	
	seslist.AddColumn("Name");
	seslist.AddColumn("Status");
	seslist.AddColumn("Progress");
	seslist <<= THISBACK(Data);
	tasklist.AddColumn("Task");
	tasklist.AddColumn("Progress");
	tasklist <<= THISBACK(SelectTask);
	
	
	Manager& m = GetManager();
	
	tc.Set(60, THISBACK(PeriodicalRefresh));
}

ManagerCtrl::~ManagerCtrl() {
	if (prev_ctrl) {
		main_task.RemoveChild(prev_ctrl);
		delete prev_ctrl;
	}
}

void ManagerCtrl::PeriodicalRefresh() {
	tc.Set(60, THISBACK(PeriodicalRefresh));
	Data();
	
	if (prev_ctrl) {
		prev_ctrl->Data();
		prev_ctrl->Refresh();
	}
	
}

void ManagerCtrl::SelectTask() {
	Manager& m = GetManager();
	int cursor = seslist.GetCursor();
	Session& ses = m.sessions[cursor];
	
	int task = tasklist.GetCursor();
	Task& t = ses.tasks[task];
	
	if (t.task.Left(9) == "Indicator") {
		One<IndicatorCtrl> c;
		c.Create();
		c->SetTask(t);
		SwitchCtrl(c.Detach());
	}
	else if (t.task == "Bitstream join") {
		SwitchCtrl(NULL);
	}
	else if (t.task == "Optimization") {
		One<OptimizationCtrl> c;
		c.Create();
		c->SetTask(t);
		SwitchCtrl(c.Detach());
	}
	else if (t.task == "DQN Sampling") {
		SwitchCtrl(NULL);
	}
	else if (t.task == "DQN Training") {
		One<DqnTrainingCtrl> c;
		c.Create();
		c->SetTask(t);
		SwitchCtrl(c.Detach());
	}
	else if (t.task == "Forecast") {
		One<ForecastCtrl> c;
		c.Create();
		c->SetTask(t);
		SwitchCtrl(c.Detach());
	}
	
	PeriodicalRefresh();
}

void ManagerCtrl::SwitchCtrl(DataCtrl* c) {
	if (prev_ctrl) {
		main_task.RemoveChild(prev_ctrl);
		delete prev_ctrl;
	}
	
	if (c)
		main_task.Add(c->SizePos());
	prev_ctrl = c;
}

void ManagerCtrl::Data() {
	Manager& m = GetManager();
	for(int i = 0; i < m.sessions.GetCount(); i++) {
		Session& ses = m.sessions[i];
		seslist.Set(i, 0, ses.symbol);
		
		String state;
		int actual, total;
		ses.GetProgress(actual, total, state);
		
		seslist.Set(i, 1, state);
		seslist.Set(i, 2, (int64)actual * 1000L / (int64)total);
		seslist.SetDisplay(i, 2, ProgressDisplay());
	}
	
	int ses_cursor = seslist.GetCursor();
	if (ses_cursor >= 0 && ses_cursor < seslist.GetCount()) {
		Session& ses = m.sessions[ses_cursor];
		
		for(int i = 0; i < ses.tasks.GetCount(); i++) {
			Task& t = ses.tasks[i];
			tasklist.Set(i, 0, t.task);
			tasklist.Set(i, 1, (int64)t.actual * 1000L / (int64)t.total);
			tasklist.SetDisplay(i, 1, ProgressDisplay());
		}
		tasklist.SetCount(ses.tasks.GetCount());
	}
}












OptimizationCtrl::OptimizationCtrl() {
	Add(tabs.SizePos());
	tabs.Add(status.SizePos(), "Status");
	
	status.Add(main_vsplit.HSizePos().VSizePos());
	main_vsplit << main_hsplit << opt_draw;
	main_vsplit.Vert();
	main_vsplit.SetPos(8000);
	main_hsplit << his_list << his_draw;
	main_hsplit.Horz();
	main_hsplit.SetPos(2000);
	
	opt_draw.type = opt_draw.OPTSTATS;
	his_draw.type = opt_draw.HISVIEW;
	
	his_list.AddColumn("#");
	his_list.AddColumn("Gen#");
	his_list.AddColumn("Score");
	his_list <<= THISBACK(SelectHistoryItem);
}

void OptimizationCtrl::SetTask(Task& t) {
	this->task = &t;
	opt_draw.t = &t;
	his_draw.t = &t;
	for(int i = 0; i < GetUsedCpuCores(); i++) {
		DrawLines& dl = gens.Add();
		dl.SetTask(t);
		dl.gen_id = i;
		dl.type = DrawLines::GENVIEW;
		tabs.Add(dl.SizePos(), "Gen " + IntStr(i));
	}
}

void OptimizationCtrl::SelectHistoryItem() {
	Manager& mgr = GetManager();
	
	int id = his_list.GetCursor();
	if (id < 0 || id == prev_id) return;
	prev_id = id;
	
	task->result_lock.Enter();
	if (id < 0 || id >= task->results.GetCount()) {
		task->result_lock.Leave();
		return;
	}
	const OptResult& rr = task->results[id];
	StringStream ss;
	ss << BZ2Decompress(rr.heatmap);
	task->result_lock.Leave();
	
	ss.Seek(0);
	ss.SetLoading();
	ss % his_draw.image;
	
	his_draw.Refresh();
}

void OptimizationCtrl::Data() {
	Manager& mgr = GetManager();
		
	task->result_lock.Enter();
	for(int i = 0; i < task->results.GetCount(); i++) {
		const OptResult& rr = task->results[i];
		his_list.Set(i, 0, rr.id);
		his_list.Set(i, 1, rr.gen_id);
		his_list.Set(i, 2, rr.err);
	}
	his_list.SetCount(task->results.GetCount());
	task->result_lock.Leave();
	
	his_list.SetSortColumn(2, false);
	
}










ForecastCtrl::ForecastCtrl() {
	Add(draw.HSizePos().VSizePos());
	
	draw.type = draw.FCASTVIEW;
}

void ForecastCtrl::Data() {
	draw.Refresh();
}









IndicatorCtrl::IndicatorCtrl() {
	Add(draw.HSizePos().VSizePos());
	
	draw.type = draw.INDIVIEW;
}

void IndicatorCtrl::Data() {
	draw.Refresh();
}














DqnTrainingCtrl::DqnTrainingCtrl() {
	Add(split.HSizePos().VSizePos());
	
	split.Horz();
	split << list << draw;
	split.SetPos(2000);
	
	list.AddColumn("#");
	list <<= THISBACK(SelectSample);
}

void DqnTrainingCtrl::Data() {
	if (!t) return;
	Task& t = *this->t;
	
	for(int i = 0; i < t.l.nnsamples.GetCount(); i++) {
		list.Set(i, 0, i);
	}
	list.SetCount(t.l.nnsamples.GetCount());
	
	draw.Refresh();
}

void DqnTrainingCtrl::SelectSample() {
	int cursor = list.GetCursor();
	
	NNSample& sample = t->l.nnsamples[cursor];
	draw.sample = &sample;
	draw.Refresh();
}

void NNSampleDraw::Paint(Draw& d) {
	Size sz(GetSize());
	d.DrawRect(sz, White());
	
	if (!sample) return;
	
	double xstep = (double)sz.cx / NNSample::single_count;
	double ystep = (double)sz.cy / NNSample::single_size;
	
	for(int i = 0; i < NNSample::single_count; i++) {
		int x = i * xstep;
		
		double absmax = 0;
		for(int j = 0; j < NNSample::single_size; j++) {
			double d = sample->input[i][j];
			if (fabs(d) > absmax) absmax = fabs(d);
		}
		
		for(int j = 0; j < NNSample::single_size; j++) {
			int y = j * ystep;
			double v = sample->input[i][j] / absmax;
			Color clr;
			clr = GrayColor(255 - v * 255);
			d.DrawRect(x, y, xstep+1, ystep+1, clr);
		}
	}
	
	double max = -DBL_MAX;
	double min = +DBL_MAX;
	for(int i = 0; i < NNSample::fwd_count; i++) {
		double d = sample->output[i];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	polyline.SetCount(NNSample::fwd_count);
	
	xstep = (double)sz.cx / (NNSample::fwd_count-1);
	for(int i = 0; i < NNSample::fwd_count; i++) {
		double v = sample->output[i];
		polyline[i].x = xstep * i;
		polyline[i].y = sz.cy - (v - min) / (max - min) * sz.cy;
	}
	d.DrawPolyline(polyline, 5, Color(0,0,255));
}

}










#ifdef flagMAIN
GUI_APP_MAIN
{
	RLOG(GetAmpDevices());
	try {
		Forecast::GetManager();
		Forecast::ManagerCtrl().Run();
		
	}
	catch (...) {
		RLOG("Unexpected exception");
	}
	RLOG("Shutting down");
	Thread::ShutdownThreads();
}
#endif
