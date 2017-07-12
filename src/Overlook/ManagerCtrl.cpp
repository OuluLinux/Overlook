#include "Overlook.h"

namespace Overlook {

AgentTabCtrl::AgentTabCtrl(Agent& agent, RealtimeSession& rtses) :
	snapctrl(agent),
	agentctrl(agent),
	trainingctrl(agent),
	rtnetctrl(agent, rtses)
{
	
	Add(snapctrl);
	Add(snapctrl, "Snapshot list");
	Add(agentctrl);
	Add(agentctrl, "Experiencer");
	Add(trainingctrl);
	Add(trainingctrl, "Training");
	Add(rtnetctrl);
	Add(rtnetctrl, "Real-Time Network");
	
}









ManagerCtrl::ManagerCtrl(System& sys) : sys(&sys) {
	view = -1;
	
	CtrlLayout(newview);
	CtrlLayout(confview);
	
	Add(hsplit.SizePos());
	
	hsplit.Horz();
	hsplit << ctrl << mainview;
	hsplit.SetPos(2000);
	
	mainview.Add(newview.SizePos());
	mainview.Add(confview.SizePos());
	
	
	String t =
			"{\n"
			"\t\"update\":\"qlearn\",\n"
			"\t\"gamma\":0.9,\n"
			"\t\"epsilon\":0.2,\n"
			"\t\"alpha\":0.005,\n"
			"\t\"experience_add_every\":5,\n"
			"\t\"experience_size\":10000,\n"
			"\t\"learning_steps_per_iteration\":5,\n"
			"\t\"tderror_clamp\":1.0,\n"
			"\t\"num_hidden_units\":100,\n"
			"}\n";
	newview.params.SetData(t);
	newview.create <<= THISBACK(NewAgent);
	newview.symlist <<= THISBACK(Data);
	
	
	add_new.SetLabel("Add new");
	configure.SetLabel("Configure");
	add_new <<= THISBACK1(SetView, 0);
	configure <<= THISBACK1(SetView, 1);
	
	ctrl.Add(add_new.TopPos(2, 26).HSizePos(2,2));
	ctrl.Add(configure.TopPos(32, 26).HSizePos(2,2));
	ctrl.Add(listsplit.VSizePos(60).HSizePos());
	listsplit.Vert();
	listsplit << glist << alist;
	
	glist.AddColumn("Name");
	glist.AddColumn("Agents");
	glist.AddColumn("Fastest tf");
	
	alist.AddColumn("Symbol");
	alist.AddColumn("Fastest tf");
	alist.AddColumn("Grade");
	alist.AddColumn("In realtime");
	alist <<= THISBACK1(SetView, 2);
	
	SetView(0);
}

void ManagerCtrl::SetView(int view) {
	newview.Hide();
	confview.Hide();
	for(int i = 0; i < agent_tabs.GetCount(); i++)
		agent_tabs[i].Hide();
	
	if (view == 0) {
		newview.Show();
		newview.SetFocus();
	}
	else if (view == 1) {
		confview.Show();
		confview.SetFocus();
	}
	else if (view == 2) {
		int agent_id = alist.GetCursor();
		if (agent_id >= 0 && agent_id < agent_tabs.GetCount()) {
			agent_tabs[agent_id].Show();
			agent_tabs[agent_id].SetFocus();
		}
	}
	this->view = view;
	Data();
}

void ManagerCtrl::Data() {
	Manager& mgr = sys->GetManager();
	
	for(int i = 0; i < mgr.groups.GetCount(); i++) {
		AgentGroup& g = mgr.groups[i];
		
		glist.Set(i, 0, g.name);
		glist.Set(i, 1, g.agents.GetCount());
		glist.Set(i, 2, sys->GetPeriodString(g.tf_ids.Top()));
	}
	glist.SetCount(mgr.groups.GetCount());
	
	int gcursor = glist.GetCursor();
	
	if (gcursor >= 0 && gcursor < mgr.groups.GetCount()) {
		AgentGroup& g = mgr.groups[gcursor];
		
		for(int i = 0; i < g.agents.GetCount(); i++) {
			Agent& a = g.agents[i];
			
			alist.Set(i, 0, sys->GetSymbol(a.sym));
			alist.Set(i, 1, sys->GetPeriodString(a.tf));
		}
		alist.SetCount(g.agents.GetCount());
	}
	
	if (view == 0) {
		// Set symbol and tf lists if empty
		
		
		// Set opening times
		
		
		// Set base currency
		
		
		// Set minimum base margin
		// newview.minbasemargin
		
	}
	else if (view == 1) {
		
	}
	else if (view == 2) {
		
	}
}

void ManagerCtrl::NewAgent() {
	
	
	
	
	
	Data();
}

}
