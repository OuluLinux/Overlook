#include "Overlook.h"

namespace Overlook {

GroupOverview::GroupOverview() {
	CtrlLayout(*this);
	
}

void GroupOverview::Data() {
	AgentGroup& ag = GetSystem().GetAgentGroup();
	
	String infostr;
	//infostr << "Reward period: " << group->agent_input_width << "x" << group->agent_input_height << "\n";
	infostr << "Created: " << Format("%", ag.created) << "\n";
	infostr << "\n";
	infostr << "Agent input size: 1 * " << AGENT_STATES << "\n";
	infostr << "Agent action count: " << AGENT_ACTIONCOUNT << "\n";
	infostr << "Average agent drawdown: " << ag.GetAverageAgentDrawdown() << "\n";
	infostr << "Average agent iterations: " << ag.GetAverageAgentIterations() << "\n";
	infostr << "Agent random action probability: " << ag.GetAgentEpsilon() << "\n";
	infostr << "\n";
	infostr << "Joiner input size: 1 * " << JOINER_STATES << "\n";
	infostr << "Joiner action count: " << JOINER_ACTIONCOUNT << "\n";
	infostr << "Average joiner drawdown: " << ag.GetAverageJoinerDrawdown() << "\n";
	infostr << "Average joiner iterations: " << ag.GetAverageJoinerIterations() << "\n";
	infostr << "Joiner random action probability: " << ag.GetJoinerEpsilon() << "\n";
	
	
	info.SetLabel(infostr);
}









GroupTabCtrl::GroupTabCtrl() {
	Add(overview);
	Add(overview, "Overview");
	Add(snapctrl);
	Add(snapctrl, "Snapshot list");
	
	WhenSet << THISBACK(Data);
}

void GroupTabCtrl::Data() {
	int tab = Get();
	if      (tab == 0)
		overview.Data();
	else if (tab == 1)
		snapctrl.Data();
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
		const Symbol& sym = GetMetaTrader().GetSymbol(a.sym);
		
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
		overview.spread_points.SetLabel(DblStr(a.spread_points));
		
		
		double minimum_margin = GetMetaTrader().GetMargin(a.sym, sym.volume_min);
		overview.minbasemargin.SetLabel(Format("%2!,n %s", minimum_margin, GetMetaTrader().AccountCurrency()));
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}
}

void AgentTabCtrl::SetAgent(Agent& agent) {
	this->agent = &agent;
	
	trainingctrl.SetAgent(agent);
}











JoinerTabCtrl::JoinerTabCtrl() {
	joiner = NULL;
	
	CtrlLayout(overview);
	
	Add(overview);
	Add(overview, "Overview");
	Add(trainingctrl);
	Add(trainingctrl, "Training");
	
	WhenSet << THISBACK(Data);
}

void JoinerTabCtrl::Data() {
	if (!joiner) return;
	
	Joiner& j = *joiner;
	int tab = Get();
	
	if (tab == 0) {
		overview.bestresult.SetLabel(DblStr(j.best_result));
		overview.iters.SetLabel(IntStr(j.iter));
		overview.epsilon.SetLabel(DblStr(j.dqn.GetEpsilon()));
		overview.expcount.SetLabel(IntStr(j.dqn.GetExperienceCount()));
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}
}

void JoinerTabCtrl::SetJoiner(Joiner& joiner) {
	this->joiner = &joiner;
	
	trainingctrl.SetJoiner(joiner);
}



















ManagerCtrl::ManagerCtrl() {
	view = -1;
	
	Add(hsplit.SizePos());
	
	hsplit.Horz();
	hsplit << listsplit << mainview;
	hsplit.SetPos(1500);
	
	mainview.Add(group_tabs.SizePos());
	mainview.Add(agent_tabs.SizePos());
	mainview.Add(joiner_tabs.SizePos());
	mainview.Add(datactrl.SizePos());
	
	listsplit.Vert();
	listsplit << glist << alist;
	listsplit.SetPos(2000);
	
	glist.AddColumn("View");
	glist.Add("Overview");
	glist.Add("Agents");
	glist.Add("Joiner");
	glist.Add("Realtime");
	glist <<= THISBACK(SetView);
	
	alist.AddColumn("Symbol");
	alist.AddColumn("Best result");
	alist.AddColumn("Drawdown");
	alist <<= THISBACK(SetView);
	alist.WhenLeftClick << THISBACK(SetView);
	
	SetView();
}

void ManagerCtrl::SetView() {
	System& sys = GetSystem();
	AgentGroup& group = sys.GetAgentGroup();
	
	agent_tabs.Hide();
	group_tabs.Hide();
	joiner_tabs.Hide();
	datactrl.Hide();
	
	Data();
	
	
	int view = glist.GetCursor();
	this->view = view;
	
	if (view == 0) {
		group_tabs.Show();
		alist.SetCount(0);
	}
	else if (view == 1) {
		int agent_id = alist.GetCursor();
		if (agent_id >= 0 && agent_id < group.agents.GetCount()) {
			agent_tabs.SetAgent(group.agents[agent_id]);
			agent_tabs.Show();
		}
	}
	else if (view == 2) {
		int joiner_id = alist.GetCursor();
		if (joiner_id >= 0 && joiner_id < group.joiners.GetCount()) {
			joiner_tabs.SetJoiner(group.joiners[joiner_id]);
			joiner_tabs.Show();
		}
	}
	else if (view == 3) {
		alist.SetCount(0);
		datactrl.Show();
	}
}

void ManagerCtrl::Data() {
	System& sys = GetSystem();
	AgentGroup& ag = sys.GetAgentGroup();
	
	int view = glist.GetCursor();
	
	if (view == 0) {
		group_tabs.Data();
	}
	else if (view == 1) {
		for(int i = 0; i < ag.agents.GetCount(); i++) {
			Agent& a = ag.agents[i];
			
			alist.Set(i, 0, IntStr(a.group_id) + " " + (a.sym != -1 ? sys.GetSymbol(a.sym) : ""));
			alist.Set(i, 1, a.best_result);
			alist.Set(i, 2, a.last_drawdown);
		}
		alist.SetCount(ag.agents.GetCount());
		
		if (this->view != view && alist.GetCount())
			alist.SetCursor(0);
		
		agent_tabs.Data();
	}
	else if (view == 2) {
		for(int i = 0; i < ag.joiners.GetCount(); i++) {
			Joiner& j = ag.joiners[i];
			
			alist.Set(i, 0, i);
			alist.Set(i, 1, j.best_result);
			alist.Set(i, 2, j.last_drawdown);
		}
		alist.SetCount(ag.joiners.GetCount());
		
		if (this->view != view && alist.GetCount())
			alist.SetCursor(0);
		
		joiner_tabs.Data();
	}
	else if (view == 3) {
		datactrl.Data();
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
