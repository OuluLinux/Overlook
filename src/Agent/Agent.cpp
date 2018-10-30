#include "Agent.h"

namespace Agent {


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
	
	if (t.task == "DQN Training") {
		One<DqnCtrl> c;
		c.Create();
		c->SetTask(t);
		SwitchCtrl(c.Detach());
	}
	else {
		SwitchCtrl(NULL);
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

















DqnCtrl::DqnCtrl() {
	Add(draw.HSizePos().VSizePos());
	
	draw.type = draw.DQN;
	draw.zero_line = 0;
}

void DqnCtrl::Data() {
	draw.Refresh();
}

}



#ifdef flagMAIN
GUI_APP_MAIN
{
	::Agent::ManagerCtrl().Run();
}
#endif
