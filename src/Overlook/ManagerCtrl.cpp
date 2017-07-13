#include "Overlook.h"

namespace Overlook {

GroupOverview::GroupOverview()
{
	
	Add(lbl.HSizePos(4, 4).TopPos(3, 24));
	Add(prog.HSizePos(4, 4).TopPos(33, 12));
	Add(sub.HSizePos(4, 4).TopPos(33+15, 12));
	Add(subsub.HSizePos(4, 4).TopPos(33+30, 12));
	SetRect(0, 0, 400, 80);
	prog.Set(0, 1);
	sub.Set(0, 1);
	subsub.Set(0, 1);
	ret_value = 0;
}

void GroupOverview::SetGroup(AgentGroup& group) {
	if (this->group) {
		this->group->WhenProgress.Clear();
		this->group->WhenSubProgress.Clear();
	}
	this->group = &group;
	this->group->WhenProgress = THISBACK(PostProgress);
	this->group->WhenSubProgress = THISBACK(PostSubProgress);
}

void GroupOverview::Progress(int actual, int total, String label) {
	LOG("Progress " << actual << "/" << total);
	prog.Set(actual, total);
	sub.Set(0, 1);
	subsub.Set(0, 1);
	this->label = label;
	lbl.SetLabel(label);
}

void GroupOverview::SubProgress(int actual, int total) {
	sub.Set(actual, total);
	subsub.Set(0, 1);
	lbl.SetLabel(label + " " + IntStr(actual) + "/" + IntStr(total));
}

void GroupOverview::SubSubProgress(int actual, int total) {
	subsub.Set(actual, total);
}

void GroupOverview::Data() {
	if (group) {
		lbl.SetLabel(group->prog_desc);
		prog.Set(group->a0, group->t0);
		sub.Set(group->a1, group->t1);
	}
}









GroupTabCtrl::GroupTabCtrl() {
	
	Add(overview);
	Add(overview, "Overview");
	Add(agentctrl);
	Add(agentctrl, "Experiencer");
	Add(snapctrl);
	Add(snapctrl, "Snapshot list");
	//Add(rtnetctrl);
	//Add(rtnetctrl, "Real-Time Network");
	
	
	
}

void GroupTabCtrl::SetGroup(AgentGroup& group) {
	overview	.SetGroup(group);
	//agentctrl	.SetGroup(group);
	snapctrl	.SetGroup(group);
}

void GroupTabCtrl::Data() {
	int tab = Get();
	if      (tab == 0)
		overview.Data();
	else if (tab == 1)
		agentctrl.Data();
	else if (tab == 2)
		snapctrl.Data();
}









ManagerCtrl::ManagerCtrl(System& sys) : sys(&sys) {
	view = -1;
	
	CtrlLayout(newview);
	CtrlLayout(confview);
	
	Add(hsplit.SizePos());
	
	hsplit.Horz();
	hsplit << ctrl << mainview;
	hsplit.SetPos(1500);
	
	mainview.Add(newview.SizePos());
	mainview.Add(confview.SizePos());
	mainview.Add(group_tabs.SizePos());
	mainview.Add(agent_view.SizePos());
	
	
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
	
	newview.symlist.AddColumn("");
	newview.symlist.AddColumn("");
	newview.symlist.NoHeader();
	newview.symlist.ColumnWidths("3 1");
	newview.tflist.AddColumn("");
	newview.tflist.AddColumn("");
	newview.tflist.NoHeader();
	newview.tflist.ColumnWidths("3 1");
	newview.all  <<= THISBACK(SelectAll);
	newview.none <<= THISBACK(SelectNone);
	newview.reward.SetData(10);
	newview.fmlevel.SetData(0.97);
	newview.alldata.Set(true);
	newview.allsig.Set(true);
	
	
	MultiButton::SubButton& amonitor	= buttons.AddButton();
	MultiButton::SubButton& gmonitor	= buttons.AddButton();
	MultiButton::SubButton& configure	= buttons.AddButton();
	MultiButton::SubButton& add_new		= buttons.AddButton();
	add_new.SetLabel("Add ");
	configure.SetLabel("Settings");
	gmonitor.SetLabel("Group");
	amonitor.SetLabel("Agent");
	add_new   <<= THISBACK1(SetView, 0);
	configure <<= THISBACK1(SetView, 1);
	gmonitor  <<= THISBACK1(SetView, 2);
	amonitor  <<= THISBACK1(SetView, 3);
	
	
	ctrl.Add(buttons.TopPos(0, 30).HSizePos());
	ctrl.Add(listsplit.VSizePos(30).HSizePos());
	listsplit.Vert();
	listsplit << glist << alist;
	
	glist.AddColumn("Name");
	glist.AddColumn("Profit");
	glist <<= THISBACK1(SetView, 2);
	glist.WhenLeftClick << THISBACK1(SetView, 2);
	
	alist.AddColumn("Symbol");
	alist.AddColumn("Profit");
	alist <<= THISBACK1(SetView, 3);
	alist.WhenLeftClick << THISBACK1(SetView, 3);
	
	SetView(0);
}

void ManagerCtrl::SelectAll() {
	for(int i = 0; i < newview.symlist.GetCount(); i++) {
		newview.symlist.Set(i, 1, true);
	}
}

void ManagerCtrl::SelectNone() {
	for(int i = 0; i < newview.symlist.GetCount(); i++) {
		newview.symlist.Set(i, 1, false);
	}
}

void ManagerCtrl::SetView(int view) {
	Manager& mgr = sys->GetManager();
	
	newview.Hide();
	confview.Hide();
	agent_view.Hide();
	group_tabs.Hide();
	
	if (view == 0) {
		newview.Show();
		newview.SetFocus();
	}
	else if (view == 1) {
		confview.Show();
		confview.SetFocus();
	}
	else if (view == 2) {
		int group_id = glist.GetCursor();
		if (group_id >= 0 && group_id < mgr.groups.GetCount()) {
			group_tabs.SetGroup(mgr.groups[group_id]);
			group_tabs.Show();
			group_tabs.SetFocus();
		}
	}
	else if (view == 3) {
		int group_id = glist.GetCursor();
		if (group_id >= 0 && group_id < mgr.groups.GetCount()) {
			AgentGroup& group = mgr.groups[group_id];
			int agent_id = alist.GetCursor();
			if (agent_id >= 0 && agent_id < group.agents.GetCount()) {
				agent_view.SetAgent(group.agents[agent_id]);
				agent_view.Show();
				agent_view.SetFocus();
			}
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
		if (!g.tf_ids.IsEmpty())
			glist.Set(i, 2, sys->GetPeriodString(g.tf_ids.Top()));
	}
	glist.SetCount(mgr.groups.GetCount());
	
	int gcursor = glist.GetCursor();
	
	if (gcursor >= 0 && gcursor < mgr.groups.GetCount()) {
		AgentGroup& g = mgr.groups[gcursor];
		
		for(int i = 0; i < g.agents.GetCount(); i++) {
			Agent& a = g.agents[i];
			
			alist.Set(i, 0, sys->GetSymbol(a.sym));
			alist.Set(i, 1, 1234);
		}
		alist.SetCount(g.agents.GetCount());
	}
	
	if (view == 0) {
		// Set symbol and tf lists if empty
		if (newview.tflist.GetCount() == 0) {
			for(int i = 0; i < sys->GetPeriodCount(); i++) {
				newview.tflist.Set(i, 0, sys->GetPeriodString(i));
				newview.tflist.Set(i, 1, 0);
				newview.tflist.SetCtrl(i, 1, new_opts.Add());
			}
		}
		if (newview.symlist.GetCount() == 0) {
			for(int i = 0; i < sys->GetSymbolCount(); i++) {
				newview.symlist.Set(i, 0, sys->GetSymbol(i));
				newview.symlist.Set(i, 1, 0);
				newview.symlist.SetCtrl(i, 1, new_opts.Add());
			}
		}
		
		
		// Set opening times
		
		
		// Set base currency
		
		
		// Set minimum base margin
		// newview.minbasemargin
		
	}
	else if (view == 1) {
		
	}
	else if (view == 2) {
		
	}
	else if (view == 3) {
		
	}
}

void ManagerCtrl::NewAgent() {
	Manager& mgr = sys->GetManager();
	
	One<AgentGroup> group_;
	group_.Create();
	AgentGroup& group = *group_;
	
	group.name = newview.name.GetData();
	if (group.name == "") {
		PromptOK(DeQtf("Set name"));
		return;
	}
	
	// Timeframes
	for(int i = newview.tflist.GetCount()-1; i >= 0; i--) {
		if (newview.tflist.Get(i, 1))
			group.tf_ids.Add(i);
	}
	if (group.tf_ids.IsEmpty()) {
		PromptOK(DeQtf("At least one timeframe must be selected"));
		return;
	}
	
	
	// Symbols
	for(int i = 0; i < newview.symlist.GetCount(); i++) {
		if (newview.symlist.Get(i, 1))
			group.sym_ids.Add(i);
	}
	if (group.sym_ids.IsEmpty()) {
		PromptOK(DeQtf("At least one symbol must be selected"));
		return;
	}
		
	
	group.reward_period				= newview.reward.GetData();
	group.global_free_margin_level	= newview.fmlevel.GetData();
	group.single_data				= !newview.alldata.Get();
	group.single_signal				= !newview.allsig.Get();
	group.sig_freeze				= newview.freeze_sig.Get();
	group.param_str					= newview.params.GetData();
	
	group.sys = sys;
	group.Init();
	group.StoreThis();
	group.Start();
	
	mgr.groups.Add(group_.Detach());
	
	Data();
	glist.SetCursor(glist.GetCount()-1);
}

}
