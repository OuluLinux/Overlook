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

void ManagerCtrl::PeriodicalRefresh() {
	tc.Set(60, THISBACK(PeriodicalRefresh));
	Data();
	
	
}

void ManagerCtrl::SelectTask() {
	Manager& m = GetManager();
	int cursor = seslist.GetCursor();
	Session& ses = m.sessions[cursor];
	
	PeriodicalRefresh();
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
	Add(status.SizePos(), "Status");
	
	for(int i = 0; i < GetUsedCpuCores(); i++) {
		HeatmapLooperCtrl& gc = gens.Add();
		gc.SetId(i);
		Add(gc.SizePos(), "Gen " + IntStr(i));
	}
	
	
	status.Add(main_vsplit.HSizePos().VSizePos(0, 30));
	status.Add(optprog.BottomPos(0, 30).HSizePos());
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

void OptimizationCtrl::SelectHistoryItem() {
	/*Manager& mgr = GetManager();
	Session* ses = mgr.GetSelectedSession();
	if (!ses) return;
	
	int cursor = his_list.GetCursor();
	if (cursor < 0 ) return;
	
	int id = his_list.Get(cursor, 0);
	if (id == prev_id)
		return;
	prev_id = id;
	
	ses->regen.result_lock.Enter();
	if (id < 0 || id >= ses->regen.results.GetCount()) {
		ses->regen.result_lock.Leave();
		return;
	}
	const OptResult& rr = ses->regen.results[id];
	StringStream ss;
	ss << BZ2Decompress(rr.heatmap);
	ses->regen.result_lock.Leave();
	
	ss.Seek(0);
	ss.SetLoading();
	ss % his_draw.image;
	
	his_draw.Refresh();*/
}

void OptimizationCtrl::Data() {
	/*Manager& mgr = GetManager();
	Session* ses = mgr.GetSelectedSession();
	if (!ses) return;
	
	ses->regen.result_lock.Enter();
	for(int i = 0; i < ses->regen.results.GetCount(); i++) {
		const OptResult& rr = ses->regen.results[i];
		his_list.Set(i, 0, rr.id);
		his_list.Set(i, 1, rr.gen_id);
		his_list.Set(i, 2, rr.err);
	}
	his_list.SetCount(ses->regen.results.GetCount());
	ses->regen.result_lock.Leave();
	
	his_list.SetSortColumn(2, false);
	
	int actual, total;
	ses->regen.GetProgress(actual, total, String());
	optprog.Set(actual, total);
	*/
}










ForecastCtrl::ForecastCtrl() {
	Add(fcast_list.LeftPos(0, 200).VSizePos());
	Add(draw.HSizePos(200).VSizePos());
	
	draw.type = draw.FCASTVIEW;
	
	fcast_list.AddColumn("#");
	fcast_list << THISBACK(SelectForecastItem);
}

void ForecastCtrl::Data() {
	/*
	Manager& mgr = GetManager();
	Session* ses = mgr.GetSelectedSession();
	if (!ses) return;
	
	ses->regen.result_lock.Enter();
	for(int i = 0; i < ses->regen.forecasts.GetCount(); i++) {
		const ForecastResult& fr = ses->regen.forecasts[i];
		fcast_list.Set(i, 0, fr.id);
	}
	fcast_list.SetCount(ses->regen.forecasts.GetCount());
	ses->regen.result_lock.Leave();
	*/
}

void ForecastCtrl::SelectForecastItem() {
	/*Manager& mgr = GetManager();
	Session* ses = mgr.GetSelectedSession();
	if (!ses) return;
	
	int cursor = fcast_list.GetCursor();
	if (cursor < 0 ) return;
	
	int id = fcast_list.Get(cursor, 0);
	if (id == prev_id)
		return;
	prev_id = id;
	
	ses->regen.result_lock.Enter();
	if (id < 0 ||id >= ses->regen.forecasts.GetCount()) {
		ses->regen.result_lock.Leave();
		return;
	}
	const ForecastResult& fr = ses->regen.forecasts[id];
	StringStream ss;
	ss << BZ2Decompress(fr.heatmap);
	draw.data <<= fr.data;
	ses->regen.result_lock.Leave();
	
	ss.Seek(0);
	ss.SetLoading();
	ss % draw.image;
	
	draw.Refresh();*/
}











HeatmapLooperCtrl::HeatmapLooperCtrl() {
	Add(draw.SizePos());
}

}

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
