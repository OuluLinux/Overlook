#include "Overlook.h"

namespace Overlook {

SystemOverview::SystemOverview() {
	CtrlLayout(*this);
	
}

void SystemOverview::Data() {
	AgentSystem& ag = GetSystem().GetAgentSystem();
	
	String infostr;
	
	infostr << "Created: " << Format("%", ag.created) << "\n";
	infostr << "\n";
	infostr << "Signal input size: 1 * " << SIGNAL_STATES << "\n";
	infostr << "Signal action count: " << SIGNAL_ACTIONCOUNT << "\n";
	infostr << "Average signal drawdown: " << ag.GetAverageSignalDrawdown() << "\n";
	infostr << "Average signal iterations: " << (int)ag.GetAverageSignalIterations() << "\n";
	infostr << "Signal random action probability: " << ag.GetSignalEpsilon() << "\n";
	infostr << "\n";
	
	infostr << "Amp input size: 1 * " << AMP_STATES << "\n";
	infostr << "Amp action count: " << AMP_ACTIONCOUNT << "\n";
	infostr << "Average amp drawdown: " << ag.GetAverageAmpDrawdown() << "\n";
	infostr << "Average amp iterations: " << (int)ag.GetAverageAmpIterations() << "\n";
	infostr << "Amp random action probability: " << ag.GetAmpEpsilon() << "\n";
	infostr << "\n";
	
	infostr << "Fuse input size: 1 * " << FUSE_STATES << "\n";
	infostr << "Fuse action count: " << FUSE_ACTIONCOUNT << "\n";
	infostr << "Average fuse drawdown: " << ag.GetAverageFuseDrawdown() << "\n";
	infostr << "Average fuse iterations: " << (int)ag.GetAverageFuseIterations() << "\n";
	infostr << "Fuse random action probability: " << ag.GetFuseEpsilon() << "\n";
	infostr << "\n";
	
	info.SetLabel(infostr);
}









SystemTabCtrl::SystemTabCtrl() {
	Add(overview);
	Add(overview, "Overview");
	Add(snapctrl);
	Add(snapctrl, "Snapshot list");
	
	WhenSet << THISBACK(Data);
}

void SystemTabCtrl::Data() {
	int tab = Get();
	if      (tab == 0)
		overview.Data();
	else if (tab == 1)
		snapctrl.Data();
}












SignalTabCtrl::SignalTabCtrl() {
	agent = NULL;
	
	CtrlLayout(overview);
	
	Add(overview);
	Add(overview, "Overview");
	Add(trainingctrl);
	Add(trainingctrl, "Training");
	
	WhenSet << THISBACK(Data);
}

void SignalTabCtrl::Data() {
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
		overview.bestresult.SetLabel(DblStr(!a.sig.result_equity.IsEmpty() ? a.sig.result_equity.Top() : 0.0));
		overview.iters.SetLabel(IntStr(a.sig.iter));
		overview.epsilon.SetLabel(DblStr(a.sig.dqn.GetEpsilon()));
		overview.expcount.SetLabel(IntStr(a.sig.dqn.GetExperienceCount()));
		
		const Vector<Price>& askbids = GetMetaTrader().GetAskBid();
		const Price& ab = askbids[a.sym];
		double factor = (ab.ask / ab.bid - 1.0) * 1000.0;
		overview.spread_factor.SetLabel(Format("%2!,n", factor));
		
		double minimum_margin = GetMetaTrader().GetMargin(a.sym, sym.volume_min);
		overview.minbasemargin.SetLabel(Format("%2!,n %s", minimum_margin, GetMetaTrader().AccountCurrency()));
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}
}

void SignalTabCtrl::SetAgent(Agent& agent) {
	this->agent = &agent;
	
	trainingctrl.SetAgent(agent, 0);
}











AmpTabCtrl::AmpTabCtrl() {
	agent = NULL;
	
	CtrlLayout(overview);
	
	Add(overview);
	Add(overview, "Overview");
	Add(trainingctrl);
	Add(trainingctrl, "Training");
	
	WhenSet << THISBACK(Data);
}

void AmpTabCtrl::Data() {
	if (!agent) return;
	
	Agent& a = *agent;
	int tab = Get();
	
	if (tab == 0) {
		overview.bestresult.SetLabel(DblStr(!a.amp.result_equity.IsEmpty() ? a.amp.result_equity.Top() : 0.0));
		overview.iters.SetLabel(IntStr(a.amp.iter));
		overview.epsilon.SetLabel(DblStr(a.amp.dqn.GetEpsilon()));
		overview.expcount.SetLabel(IntStr(a.amp.dqn.GetExperienceCount()));
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}
}

void AmpTabCtrl::SetAgent(Agent& agent) {
	this->agent = &agent;
	
	trainingctrl.SetAgent(agent, 1);
}



















FuseTabCtrl::FuseTabCtrl() {
	agent = NULL;
	
	CtrlLayout(overview);
	
	Add(overview);
	Add(overview, "Overview");
	Add(trainingctrl);
	Add(trainingctrl, "Training");
	
	WhenSet << THISBACK(Data);
}

void FuseTabCtrl::Data() {
	if (!agent) return;
	
	Agent& a = *agent;
	int tab = Get();
	
	if (tab == 0) {
		overview.bestresult.SetLabel(DblStr(!a.fuse.result_equity.IsEmpty() ? a.fuse.result_equity.Top() : 0.0));
		overview.iters.SetLabel(IntStr(a.fuse.iter));
		overview.epsilon.SetLabel(DblStr(a.fuse.dqn.GetEpsilon()));
		overview.expcount.SetLabel(IntStr(a.fuse.dqn.GetExperienceCount()));
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}
}

void FuseTabCtrl::SetAgent(Agent& agent) {
	this->agent = &agent;
	
	trainingctrl.SetAgent(agent, 2);
}









ManagerCtrl::ManagerCtrl() {
	view = -1;
	
	Add(hsplit.SizePos());
	
	hsplit.Horz();
	hsplit << listsplit << mainview;
	hsplit.SetPos(1500);
	
	mainview.Add(system_tabs.SizePos());
	mainview.Add(signal_tabs.SizePos());
	mainview.Add(amp_tabs.SizePos());
	mainview.Add(fuse_tabs.SizePos());
	mainview.Add(export_ctrl.SizePos());
	
	listsplit.Vert();
	listsplit << glist << alist;
	listsplit.SetPos(2000);
	
	glist.AddColumn("View");
	glist.Add("Overview");
	glist.Add("Signal");
	glist.Add("Amp");
	glist.Add("Fuse");
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
	AgentSystem& asys = sys.GetAgentSystem();
	
	signal_tabs.Hide();
	system_tabs.Hide();
	amp_tabs.Hide();
	fuse_tabs.Hide();
	export_ctrl.Hide();
	
	Data();
	
	
	int view = glist.GetCursor();
	this->view = view;
	
	if (view == 0) {
		system_tabs.Show();
		alist.SetCount(0);
	}
	else if (view == 1) {
		int agent_id = alist.GetCursor();
		int group_id = agent_id / SYM_COUNT;
		agent_id = agent_id % SYM_COUNT;
		signal_tabs.SetAgent(asys.groups[group_id].agents[agent_id]);
		signal_tabs.Show();
	}
	else if (view == 2) {
		int agent_id = alist.GetCursor();
		int group_id = agent_id / SYM_COUNT;
		agent_id = agent_id % SYM_COUNT;
		amp_tabs.SetAgent(asys.groups[group_id].agents[agent_id]);
		amp_tabs.Show();
	}
	else if (view == 3) {
		int agent_id = alist.GetCursor();
		int group_id = agent_id / SYM_COUNT;
		agent_id = agent_id % SYM_COUNT;
		fuse_tabs.SetAgent(asys.groups[group_id].agents[agent_id]);
		fuse_tabs.Show();
	}
	else if (view == 4) {
		alist.SetCount(0);
		export_ctrl.Show();
	}
}

void ManagerCtrl::Data() {
	System& sys = GetSystem();
	AgentSystem& ag = sys.GetAgentSystem();
	
	int view = glist.GetCursor();
	
	if (view == 0) {
		system_tabs.Data();
	}
	else if (view >= 1 && view <= 3) {
		int acount = 0;
		for(int i = 0; i < ag.groups.GetCount(); i++)
		for(int j = 0; j < ag.groups[i].agents.GetCount(); j++) {
			Agent& a = ag.groups[i].agents[j];
			
			alist.Set(acount, 0, IntStr(i) + " " + (a.sym != -1 ? sys.GetSymbol(a.sym) : ""));
			alist.Set(acount, 1, a.GetLastResult(view - 1));
			alist.Set(acount, 2, a.GetLastDrawdown(view - 1));
			acount++;
		}
		alist.SetCount(acount);
		
		if (this->view != view && alist.GetCount())
			alist.SetCursor(0);
		
		if      (view == 1) signal_tabs.Data();
		else if (view == 2) amp_tabs.Data();
		else if (view == 3) fuse_tabs.Data();
	}
	else if (view == 3) {
		export_ctrl.Data();
	}
	
}
















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
