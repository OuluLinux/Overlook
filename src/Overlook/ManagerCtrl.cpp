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
	
	for(int i = 0; i < FILTER_COUNT; i++) {
		infostr << "Filter " << i << "\n";
		infostr << "\tInput size: 1 * " << FILTER_STATES << "\n";
		infostr << "\tAction count: " << FILTER_ACTIONCOUNT << "\n";
		infostr << "\tAverage drawdown: " << ag.GetAverageFilterDrawdown(i) << "\n";
		infostr << "\tAverage filter " << i << " iterations: " << (int)ag.GetAverageFilterIterations(i) << "\n";
		infostr << "\tRandom action probability: " << ag.GetFilterEpsilon(i) << "\n";
		infostr << "\n";
	}
	
	infostr << "Signal\n";
	infostr << "\tInput size: 1 * " << SIGNAL_STATES << "\n";
	infostr << "\tAction count: " << SIGNAL_ACTIONCOUNT << "\n";
	infostr << "\tAverage drawdown: " << ag.GetAverageSignalDrawdown() << "\n";
	infostr << "\tAverage iterations: " << (int)ag.GetAverageSignalIterations() << "\n";
	infostr << "\tRandom action probability: " << ag.GetSignalEpsilon() << "\n";
	infostr << "\n";
	
	infostr << "Amp\n";
	infostr << "\tInput size: 1 * " << AMP_STATES << "\n";
	infostr << "\tAction count: " << AMP_ACTIONCOUNT << "\n";
	infostr << "\tAverage drawdown: " << ag.GetAverageAmpDrawdown() << "\n";
	infostr << "\tAverage iterations: " << (int)ag.GetAverageAmpIterations() << "\n";
	infostr << "\tRandom action probability: " << ag.GetAmpEpsilon() << "\n";
	infostr << "\n";
	
	
	if (ag.phase < PHASE_REAL) {
		dword elapsed = ts.Elapsed();
		
		if (elapsed > 3000) {
			double av_iters, iters_max;
			if (ag.phase == PHASE_SIGNAL_TRAINING) {
				av_iters = ag.GetAverageSignalIterations();
				iters_max = SIGNAL_PHASE_ITER_LIMIT;
				phase_str = "Signal";
			}
			else if (ag.phase == PHASE_AMP_TRAINING) {
				av_iters = ag.GetAverageAmpIterations();
				iters_max = AMP_PHASE_ITER_LIMIT;
				phase_str = "Amp";
			}
			else if (ag.phase < PHASE_SIGNAL_TRAINING) {
				av_iters = ag.GetAverageFilterIterations(ag.phase);
				iters_max = FILTER_PHASE_ITER_LIMIT;
				phase_str = "Filter " + IntStr(ag.phase);
			}
			
			double iters_diff = av_iters - prev_av_iters;
			double iters_per_sec = iters_diff / (elapsed / 1000.0);
			double iters_left = iters_max - av_iters;
			double sec_remaining = iters_left / iters_per_sec;
			
			sec  = ((int)sec_remaining) % 60;
			min  = ((int)sec_remaining / 60) % 60;
			hour = ((int)sec_remaining / 60 / 60);
			
			prev_av_iters = av_iters;
			
			ts.Reset();
		}
		
		infostr << "Current phase: " << phase_str << "\n";
		if (hour > 0) infostr << hour << " hours ";
		if (min > 0) infostr << min << " minutes ";
		if (hour == 0 && min == 0) infostr << sec << " seconds ";
		infostr << "left in the current phase.\n";
	}
	
	
	
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
	
	trainingctrl.SetAgent(agent, PHASE_SIGNAL_TRAINING);
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
	
	trainingctrl.SetAgent(agent, PHASE_AMP_TRAINING);
}



















FilterTabCtrl::FilterTabCtrl() {
	agent = NULL;
	
	CtrlLayout(overview);
	
	Add(overview);
	Add(overview, "Overview");
	Add(trainingctrl);
	Add(trainingctrl, "Training");
	
	WhenSet << THISBACK(Data);
}

void FilterTabCtrl::Data() {
	if (!agent) return;
	
	Agent& a = *agent;
	int tab = Get();
	
	if (tab == 0) {
		overview.bestresult.SetLabel(DblStr(!a.filter[level].result_equity.IsEmpty() ? a.filter[level].result_equity.Top() : 0.0));
		overview.iters.SetLabel(IntStr(a.filter[level].iter));
		overview.epsilon.SetLabel(DblStr(a.filter[level].dqn.GetEpsilon()));
		overview.expcount.SetLabel(IntStr(a.filter[level].dqn.GetExperienceCount()));
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}
}

void FilterTabCtrl::SetAgent(Agent& agent, int level) {
	this->agent = &agent;
	this->level = level;
	ASSERT(level >= 0 && level < FILTER_COUNT);
	
	trainingctrl.SetAgent(agent, level);
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
	mainview.Add(filter_tabs.SizePos());
	mainview.Add(export_ctrl.SizePos());
	
	listsplit.Vert();
	listsplit << glist << alist;
	listsplit.SetPos(2000);
	
	glist.AddColumn("View");
	glist.Add("Overview");
	glist.Add("Filter 1");
	glist.Add("Filter 2");
	glist.Add("Filter 3");
	glist.Add("Signal");
	glist.Add("Amp");
	glist.Add("Realtime");
	glist <<= THISBACK(SetView);
	
	alist.AddColumn("Symbol");
	alist.AddColumn("Result");
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
	filter_tabs.Hide();
	export_ctrl.Hide();
	
	Data();
	
	
	int view = glist.GetCursor();
	this->view = view;
	
	if (view == 0) {
		system_tabs.Show();
		alist.SetCount(0);
	}
	else if (view >= 1 && view <= 3) {
		int agent_id = alist.GetCursor();
		int group_id = agent_id / SYM_COUNT;
		agent_id = agent_id % SYM_COUNT;
		filter_tabs.SetAgent(asys.groups[group_id].agents[agent_id], view - 1);
		filter_tabs.Show();
	}
	else if (view == 4) {
		int agent_id = alist.GetCursor();
		int group_id = agent_id / SYM_COUNT;
		agent_id = agent_id % SYM_COUNT;
		signal_tabs.SetAgent(asys.groups[group_id].agents[agent_id]);
		signal_tabs.Show();
	}
	else if (view == 5) {
		int agent_id = alist.GetCursor();
		int group_id = agent_id / SYM_COUNT;
		agent_id = agent_id % SYM_COUNT;
		amp_tabs.SetAgent(asys.groups[group_id].agents[agent_id]);
		amp_tabs.Show();
	}
	else if (view == 6) {
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
	else if (view >= 1 && view <= 5) {
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
		
		if      (view == 4) signal_tabs.Data();
		else if (view == 5)	amp_tabs.Data();
		else				filter_tabs.Data();
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
	if (Thread::IsShutdownThreads()) return;
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
