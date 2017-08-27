#include "Overlook.h"

namespace Overlook {

GroupOverview::GroupOverview() {
	CtrlLayout(*this);
	
}

void GroupOverview::Data() {
	/*if (group) {
		lbl.SetLabel(Format("%s: %d/%d, %d/%d", group->prog_desc, group->a0, group->t0, group->a1, group->t1));
		prog.Set(group->a0, group->t0);
		sub.Set(group->a1, group->t1);
		
		String infostr;
		infostr << "Name: " << group->name << "\n";
		infostr << "Free-margin level: " << group->fmlevel << "\n";
		//infostr << "Reward period: " << group->agent_input_width << "x" << group->agent_input_height << "\n";
		infostr << "Reward period: " << group->group_input_width << "x" << group->group_input_height << "\n";
		infostr << "Enable training: " << (group->enable_training ? "True" : "False") << "\n";
		infostr << "Created: " << Format("%", group->created) << "\n";
		for(int i = 0; i < group->tf_ids.GetCount(); i++) {
			infostr
				<< "    tf" << i << ": " << group->tf_ids[i] << ", "
				<< group->sys->GetPeriodString(group->tf_ids[i]) << ", av_dd="
				<< group->GetTfDrawdown(i) << "\n";
		}
		
		infostr << "\nDQN-agent parameters:\n" << group->param_str << "\n\n";
		infostr << "\nRandomization loops: " << group->random_loops << "\n\n";
		
		if (!group->agents.IsEmpty()) {
			epsilon.SetData(group->agents[0].dqn.GetEpsilon());
			limit_factor.SetData(group->limit_factor);
		}
		info.SetLabel(infostr);
	}*/
}









GroupTabCtrl::GroupTabCtrl() {
	Add(overview);
	Add(overview, "Overview");
	Add(trainingctrl);
	Add(trainingctrl, "Training");
	Add(snapctrl);
	Add(snapctrl, "Snapshot list");
	Add(datactrl);
	Add(datactrl, "Recorded data");
	
	WhenSet << THISBACK(Data);
}

void GroupTabCtrl::Data() {
	int tab = Get();
	if      (tab == 0)
		overview.Data();
	else if (tab == 1)
		trainingctrl.Data();
	else if (tab == 2)
		snapctrl.Data();
	else if (tab == 3)
		datactrl.Data();
}












AgentTabCtrl::AgentTabCtrl() {
	agent = NULL;
	
	CtrlLayout(overview);
	
	Add(overview);
	Add(overview, "Overview");
	Add(trainingctrl);
	Add(trainingctrl, "Training");
	
	WhenSet << THISBACK(Data);
}

void AgentTabCtrl::Data() {
	if (!agent) return;
	
	Agent& a = *agent;
	int tab = Get();
	
	if (tab == 0) {
		const Symbol& sym = GetMetaTrader().GetSymbol(a.sym__);
		
		String trading_hours;
		for(int i = 0; i < 7; i++) {
			if (i)
				trading_hours << "\n";
			
			trading_hours << Format("%d:%02d - %d:%02d",
				sym.trades_begin_hours[i], sym.trades_begin_minutes[i],
				sym.trades_end_hours[i],   sym.trades_end_minutes[i]);
		}
		overview.opentimes.SetLabel(trading_hours);
		overview.margincur.SetLabel(sym.currency_margin);
		overview.minvolume.SetLabel(DblStr(sym.volume_min));
		overview.bestresult.SetLabel(DblStr(a.best_result));
		overview.iters.SetLabel(IntStr(a.iter));
		overview.epsilon.SetLabel(DblStr(a.dqn.GetEpsilon()));
		overview.expcount.SetLabel(IntStr(a.dqn.GetExperienceCount()));
		
		
		double minimum_margin = GetMetaTrader().GetMargin(a.sym__, sym.volume_min);
		overview.minbasemargin.SetLabel(Format("%2!,n %s", minimum_margin, GetMetaTrader().AccountCurrency()));
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}
}

void AgentTabCtrl::SetAgent(Agent& agent) {
	this->agent = &agent;
	
	trainingctrl.SetTrainee(agent);
}










ManagerCtrl::ManagerCtrl() {
	view = -1;
	
	Add(hsplit.SizePos());
	
	hsplit.Horz();
	hsplit << listsplit << mainview;
	hsplit.SetPos(1500);
	
	mainview.Add(group_tabs.SizePos());
	mainview.Add(agent_tabs.SizePos());
	
	listsplit.Vert();
	listsplit << glist << alist;
	listsplit.SetPos(2000);
	
	glist.AddColumn("Name");
	glist.Add("Overview");
	glist.Add("Agents");
	glist.Add("Joiners");
	glist.Add("Joiners of joiners");
	glist <<= THISBACK(SelectView);
	
	alist.AddColumn("Symbol");
	alist.AddColumn("Group");
	alist.AddColumn("Best result");
	alist.AddColumn("Drawdown");
	alist <<= THISBACK(SelectView);
	alist.WhenLeftClick << THISBACK(SelectView);
	
	SetView(0);
}

void ManagerCtrl::SelectView() {
	SetView(glist.GetCursor());
}

void ManagerCtrl::SetView(int view) {
	System& sys = GetSystem();
	AgentGroup& group = sys.GetAgentGroup();
	
	agent_tabs.Hide();
	group_tabs.Hide();
	
	if (view == 0) {
		group_tabs.Show();
		//group_tabs.SetFocus();
	}
	else if (view == 1) {
		int agent_id = alist.GetCursor();
		if (agent_id >= 0 && agent_id < group.agents.GetCount()) {
			agent_tabs.SetAgent(group.agents[agent_id]);
			agent_tabs.Show();
			//agent_tabs.SetFocus();
		}
	}
	else if (view == 2) {
		/**/
	}
	this->view = view;
	Data();
}

void ManagerCtrl::Data() {
	System& sys = GetSystem();
	AgentGroup& ag = sys.GetAgentGroup();
	
	int gcursor = glist.GetCursor();
	
	if (gcursor == 0) {
		for(int i = 0; i < ag.agents.GetCount(); i++) {
			Agent& a = ag.agents[i];
			
			alist.Set(i, 0, a.sym__ != -1 ? sys.GetSymbol(a.sym__) : "");
			alist.Set(i, 1, a.group_id);
			alist.Set(i, 2, a.best_result);
			alist.Set(i, 3, a.last_drawdown);
		}
		alist.SetCount(ag.agents.GetCount());
	}
	
	
	if (view == 0) {
		agent_tabs.Data();
	}
	else if (view == 1) {
		group_tabs.Data();
	}
	else if (view == 2) {
		
	}
}

/*
void ManagerCtrl::NewAgent() {
	System& sys = GetSystem();
	
	One<AgentGroup> group_;
	group_.Create();
	AgentGroup& group = *group_;
	
	group.name = newview.name.GetData();
	if (group.name == "") {
		PromptOK(DeQtf("Set name"));
		return;
	}
	
	
	for(int i = 0; i < mgr.groups.GetCount(); i++) {
		if (group.name == mgr.groups[i].name) {
			PromptOK(DeQtf("An agent with the name " + group.name + " exists already."));
			return;
		}
	}
	
	
	// Timeframes
	Vector<int> tf_ids;
	for(int i = newview.tflist.GetCount()-1; i >= 0; i--) {
		if (newview.tflist.Get(i, 1))
			tf_ids.Add(i);
	}
	if (tf_ids.IsEmpty()) {
		PromptOK(DeQtf("At least one timeframe must be selected"));
		return;
	}
	
	// Sort timeframes
	struct TfSorter {
		System* sys;
		bool operator()(int a, int b) const {
			return sys->GetPeriod(a) > sys->GetPeriod(b);
		}
	};
	TfSorter sorter;
	sorter.sys = &sys;
	Sort(tf_ids, sorter);
	for(int i = 0; i < tf_ids.GetCount(); i++)
		group.tf_ids.Add(tf_ids[i]);
	
	
	// Symbols
	for(int i = 0; i < newview.symlist.GetCount(); i++) {
		if (newview.symlist.Get(i, 1))
			group.sym_ids.Add(i);
	}
	if (group.sym_ids.IsEmpty()) {
		PromptOK(DeQtf("At least one symbol must be selected"));
		return;
	}
		
	
	// Indicators
	for(int i = 0; i < newview.indilist.GetCount(); i++) {
		if (newview.indilist.Get(i, 1))
			group.indi_ids.Add(i);
	}
	if (group.indi_ids.IsEmpty()) {
		PromptOK(DeQtf("At least one indicator must be selected. Try clicking 'Sensors'."));
		return;
	}
	
	
	group.sys = &sys;
	
	mgr.groups.Add(group_.Detach());
	
	PostCallback(THISBACK(Data));
	PostCallback(THISBACK(LastCursor));
	
	
	group.Init();
	group.StoreThis();
	group.Start();
}
*/















RealtimeCtrl::RealtimeCtrl() {
	System& sys = GetSystem();
	
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << brokerctrl << journal;
	hsplit.SetPos(8000);
	
	
	journal.AddColumn("Time");
	journal.AddColumn("Level");
	journal.AddColumn("Message");
	journal.ColumnWidths("3 1 3");
	
	
	brokerctrl.SetBroker(GetMetaTrader());
	
	sys.WhenRealtimeUpdate << THISBACK(PostData);
}

void RealtimeCtrl::AddMessage(String time, String level, String msg) {
	journal.Insert(0);
	journal.Set(0, 0, time);
	journal.Set(0, 1, level);
	journal.Set(0, 2, msg);
}

void RealtimeCtrl::Info(String msg) {
	PostCallback(THISBACK3(AddMessage,
		Format("%", GetSysTime()),
		"Info",
		msg));
}

void RealtimeCtrl::Error(String msg) {
	PostCallback(THISBACK3(AddMessage,
		Format("%", GetSysTime()),
		"Error",
		msg));
}

void RealtimeCtrl::Data() {
	MetaTrader& mt = GetMetaTrader();
	mt.ForwardExposure();
	
	brokerctrl.Data();
}

void RealtimeCtrl::Init() {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	mt.WhenInfo  << THISBACK(Info);
	mt.WhenError << THISBACK(Error);
	
	sys.WhenInfo  << THISBACK(Info);
	sys.WhenError << THISBACK(Error);
}

}
