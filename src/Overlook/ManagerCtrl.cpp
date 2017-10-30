#include "Overlook.h"

namespace Overlook {

const Vector<int64>& ActionCountGraph::GetStats() {
	/*if (phase == PHASE_SIGNAL_TRAINING)
		return a->sig.action_counts;
	if (phase == PHASE_AMP_TRAINING)
		return a->amp.action_counts;
	if (phase == PHASE_FUSE_TRAINING)
		return f->action_counts;
	Panic("NEVER");*/
	return Single<Vector<int64> >();
}

void ActionCountGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	const Vector<int64>& stats = GetStats();
	
	int64 max_count = 0;
	for(int i = 0; i < stats.GetCount(); i++)
		max_count = Upp::max(max_count, stats[i]);
	
	if (max_count > 0) {
		double xstep = (double)sz.cx / stats.GetCount();
		
		for(int i = 0; i < stats.GetCount(); i++) {
			int x1 = i * xstep;
			int x2 = (i + 1) * xstep;
			int w = x2 - x1;
			int h = sz.cy * stats[i] / max_count;
			int y = sz.cy - h;
			id.DrawRect(x1, y, w, h, Color(85, 127, 255));
		}
	}
	
	w.DrawImage(0, 0, id);
}








/*


SystemOverview::SystemOverview() {
	CtrlLayout(*this);
	
}

void SystemOverview::Data() {
	
	AgentSystem& ag = GetSystem().GetAgentSystem();
	
	String infostr;
	
	infostr << "Created: " << Format("%", ag.created) << "\n";
	infostr << "\n";
	
	infostr << "Signal\n";
	infostr << "\tInput size: 1 * " << SIGNAL_STATES << "\n";
	infostr << "\tAction count: " << SIGNAL_ACTIONCOUNT << "\n";
	infostr << "\tAverage drawdown: " << ag.GetAverageSignalDrawdown() << "\n";
	infostr << "\tAverage iterations: " << (int)ag.GetAverageSignalIterations() << "\n";
	infostr << "\tAverage deep iterations: " << (int)ag.GetAverageSignalDeepIterations() << "\n";
	infostr << "\tRandom action probability: " << ag.GetSignalEpsilon() << "\n";
	infostr << "\n";
	
	infostr << "Amp\n";
	infostr << "\tInput size: 1 * " << AMP_STATES << "\n";
	infostr << "\tAction count: " << AMP_ACTIONCOUNT << "\n";
	infostr << "\tAverage drawdown: " << ag.GetAverageAmpDrawdown() << "\n";
	infostr << "\tAverage iterations: " << (int)ag.GetAverageAmpIterations() << "\n";
	infostr << "\tAverage deep iterations: " << (int)ag.GetAverageAmpDeepIterations() << "\n";
	infostr << "\tRandom action probability: " << ag.GetAmpEpsilon() << "\n";
	infostr << "\n";
	
	infostr << "Fuse\n";
	infostr << "\tInput size: 1 * " << FUSE_STATES << "\n";
	infostr << "\tAction count: " << FUSE_ACTIONCOUNT << "\n";
	infostr << "\tAverage drawdown: " << ag.GetAverageFuseDrawdown() << "\n";
	infostr << "\tAverage iterations: " << (int)ag.GetAverageFuseIterations() << "\n";
	infostr << "\tAverage deep iterations: " << (int)ag.GetAverageFuseDeepIterations() << "\n";
	infostr << "\tRandom action probability: " << ag.GetFuseEpsilon() << "\n";
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
			else if (ag.phase == PHASE_FUSE_TRAINING) {
				av_iters = ag.GetAverageFuseIterations();
				iters_max = FUSE_PHASE_ITER_LIMIT;
				phase_str = "Fuse";
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
*/















/*
SectorOverview::SectorOverview() {
	Add(vsplit.SizePos());
	vsplit.Vert();
	vsplit.SetPos(7500);
	
	in.AddColumn("Values");
	in.AddColumn("Total");
	
	out.AddColumn("Volatility");
	out.AddColumn("Change");
	out.AddColumn("Total");
	
	in_conn.AddColumn("Out ID");
	in_conn.AddColumn("Count");
	in_conn.AddColumn("Total");
	
	out_conn.AddColumn("In ID");
	out_conn.AddColumn("Count");
	out_conn.AddColumn("Total");
	
	out_poles.AddColumn("? X");
	out_poles.AddColumn("? Y");
	out_poles.AddColumn("? Count");
	for(int i = 0; i < TARGET_COUNT; i++) {
		out_poles.AddColumn("+ X");
		out_poles.AddColumn("+ Y");
		out_poles.AddColumn("+ Count");
		out_poles.AddColumn("- X");
		out_poles.AddColumn("- Y");
		out_poles.AddColumn("- Count");
	}
	
	split << in << out << in_conn << out_conn;
	vsplit << split << out_poles;
}

void SectorOverview::Data() {
	AgentSystem& as = GetSystem().GetAgentSystem();
	
	int indi_cursor		= in.GetCursor();
	int result_cursor	= out.GetCursor();
	
	for(int i = 0; i < as.indi_centroids.GetCount(); i++) {
		const IndicatorTuple& it = as.indi_centroids[i];
		const IndicatorSector& is = as.indi_sectors[i];
		
		String s;
		for(int j = 0; j < SENSOR_SIZE; j++) {
			s << " " << (int)it.values[j];
		}
		
		in.Set(i, 0, s);
		in.Set(i, 1, is.conn_total);
	}
	
	for(int i = 0; i < as.result_sectors.GetCount(); i++) {
		const ResultTuple& rt = as.result_centroids[i];
		const ResultSector& rs = as.result_sectors[i];
		
		out.Set(i, 0, rt.volat * VOLAT_DIV);
		out.Set(i, 1, rt.change * CHANGE_DIV);
		out.Set(i, 2, rs.conn_total);
	}
	
	if (indi_cursor >= 0 && indi_cursor < as.indi_centroids.GetCount()) {
		const IndicatorTuple& it = as.indi_centroids[indi_cursor];
		const IndicatorSector& is = as.indi_sectors[indi_cursor];
		
		for(int i = 0; i < is.sector_conn_counts.GetCount(); i++) {
			int c_id = is.sector_conn_counts.GetKey(i);
			int count = is.sector_conn_counts[i];
			double total = (double)count / is.conn_total;
			
			in_conn.Set(i, 0, c_id);
			in_conn.Set(i, 1, count);
			in_conn.Set(i, 2, total);
		}
		
		in_conn.SetCount(is.sector_conn_counts.GetCount());
	}
	
	if (result_cursor >= 0 && result_cursor < as.result_centroids.GetCount()) {
		const ResultTuple& rt = as.result_centroids[result_cursor];
		const ResultSector& rs = as.result_sectors[result_cursor];
		
		for(int i = 0; i < rs.sector_conn_counts.GetCount(); i++) {
			int c_id = rs.sector_conn_counts.GetKey(i);
			int count = rs.sector_conn_counts[i];
			double total = (double)count / rs.conn_total;
			
			out_conn.Set(i, 0, c_id);
			out_conn.Set(i, 1, count);
			out_conn.Set(i, 2, total);
		}
		
		out_conn.SetCount(rs.sector_conn_counts.GetCount());
	}
	
	
	for(int i = 0; i < as.result_sectors.GetCount(); i++) {
		const ResultTuple& rt = as.result_centroids[i];
		const ResultSector& rs = as.result_sectors[i];
		
		out_poles.Set(i, 0, rs.pnd.x_mean_int * VOLAT_DIV);
		out_poles.Set(i, 1, rs.pnd.y_mean_int * CHANGE_DIV);
		out_poles.Set(i, 2, rs.pnd.x.count);
		
		int k = 3;
		for(int j = 0; j < TARGET_COUNT; j++) {
			out_poles.Set(i, k++, rs.pos[j].x_mean_int * VOLAT_DIV);
			out_poles.Set(i, k++, rs.pos[j].y_mean_int * CHANGE_DIV);
			out_poles.Set(i, k++, rs.pos[j].x.count);
			out_poles.Set(i, k++, rs.neg[j].x_mean_int * VOLAT_DIV);
			out_poles.Set(i, k++, rs.neg[j].y_mean_int * CHANGE_DIV);
			out_poles.Set(i, k++, rs.neg[j].x.count);
		}
	}
	
	
}
*/








/*SystemTabCtrl::SystemTabCtrl() {
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
	//else if (tab == 1)
	//	snapctrl.Data();
}*/













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
	/*if (!agent) return;
	
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
		overview.deep_iters.SetLabel(IntStr(a.sig.deep_iter));
		
		const Vector<Price>& askbids = GetMetaTrader().GetAskBid();
		const Price& ab = askbids[a.sym];
		double spread_point = a.spread_points;
		double factor = ((ab.bid + spread_point) / ab.bid - 1.0) * 1000.0;
		overview.spread_factor.SetLabel(Format("%2!,n", factor));
		overview.spread_points.SetLabel(DblStr(spread_point));
		
		double minimum_margin = GetMetaTrader().GetMargin(a.sym, sym.volume_min);
		overview.minbasemargin.SetLabel(Format("%2!,n %s", minimum_margin, GetMetaTrader().AccountCurrency()));
		
		overview.act_graph.Refresh();
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}*/
}

void SignalTabCtrl::SetAgent(Agent& agent) {
	/*this->agent = &agent;
	
	overview.act_graph.SetAgent(agent, PHASE_SIGNAL_TRAINING);
	trainingctrl.SetAgent(agent, PHASE_SIGNAL_TRAINING);*/
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
	/*if (!agent) return;
	
	Agent& a = *agent;
	int tab = Get();
	
	if (tab == 0) {
		overview.bestresult.SetLabel(DblStr(!a.amp.result_equity.IsEmpty() ? a.amp.result_equity.Top() : 0.0));
		overview.iters.SetLabel(IntStr(a.amp.iter));
		overview.epsilon.SetLabel(DblStr(a.amp.dqn.GetEpsilon()));
		overview.expcount.SetLabel(IntStr(a.amp.dqn.GetExperienceCount()));
		overview.deep_iters.SetLabel(IntStr(a.amp.deep_iter));
		overview.act_graph.Refresh();
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}*/
}

void AmpTabCtrl::SetAgent(Agent& agent) {
	/*this->agent = &agent;
	
	overview.act_graph.SetAgent(agent, PHASE_AMP_TRAINING);
	trainingctrl.SetAgent(agent, PHASE_AMP_TRAINING);*/
}
























FuseTabCtrl::FuseTabCtrl() {
	CtrlLayout(overview);
	
	Add(overview);
	Add(overview, "Overview");
	Add(trainingctrl);
	Add(trainingctrl, "Training");
	
	WhenSet << THISBACK(Data);
	
	/*AgentFuse& fuse = GetSystem().GetAgentSystem().fuse;
	overview.act_graph.SetFuse(fuse);
	trainingctrl.SetFuse(fuse);*/
}

void FuseTabCtrl::Data() {
	/*AgentFuse& fuse = GetSystem().GetAgentSystem().fuse;
	int tab = Get();
	
	if (tab == 0) {
		overview.bestresult.SetLabel(DblStr(!fuse.result_equity.IsEmpty() ? fuse.result_equity.Top() : 0.0));
		overview.iters.SetLabel(IntStr(fuse.iter));
		overview.epsilon.SetLabel(DblStr(fuse.dqn.GetEpsilon()));
		overview.expcount.SetLabel(IntStr(fuse.dqn.GetExperienceCount()));
		overview.deep_iters.SetLabel(IntStr(fuse.deep_iter));
		overview.av_trigger.SetLabel(IntStr(fuse.av_trigger.mean));
		overview.act_graph.Refresh();
	}
	else if (tab == 1) {
		trainingctrl.Data();
	}*/
}

























/*
ManagerCtrl::ManagerCtrl() {
	view = -1;
	
	Add(hsplit.SizePos());
	
	hsplit.Horz();
	hsplit << glist << mainview;
	hsplit.SetPos(1500);
	
	mainview.Add(system_tabs.SizePos());
	mainview.Add(signal_tabs.SizePos());
	mainview.Add(amp_tabs.SizePos());
	mainview.Add(fuse_tabs.SizePos());
	mainview.Add(export_ctrl.SizePos());
	
	glist.AddColumn("View");
	glist.Add("Overview");
	glist.Add("Optimizer");
	glist.Add("Realtime");
	glist <<= THISBACK(SetView);
	
	SetView();
}

void ManagerCtrl::SetView() {
	System& sys = GetSystem();
	//AgentSystem& asys = sys.GetAgentSystem();
	
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
		alist.SetCount(0);
		fuse_tabs.Show();
	}
	else if (view == 2) {
		export_ctrl.Show();
	}
}

void ManagerCtrl::Data() {
	System& sys = GetSystem();
	//AgentSystem& ag = sys.GetAgentSystem();
	
	int view = glist.GetCursor();
	
	if (view == 0) {
		system_tabs.Data();
	}
	else if (view >= 1 && view <= 2) {
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
		else if (view == 2)	amp_tabs.Data();
	}
	else if (view == 3) {
		fuse_tabs.Data();
	}
	else if (view == 2) {
		export_ctrl.Data();
	}
	
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
	lock.Enter();
	messages.Add(Msg(time, level, msg));
	lock.Leave();
}

void RealtimeCtrl::Info(String msg) {
	AddMessage(
		Format("%", GetSysTime()),
		"Info",
		msg);
}

void RealtimeCtrl::Error(String msg) {
	AddMessage(
		Format("%", GetSysTime()),
		"Error",
		msg);
}

void RealtimeCtrl::Data() {
	MetaTrader& mt = GetMetaTrader();
	mt.ForwardExposure();
	
	brokerctrl.Data();
	
	lock.Enter();
	int count = Upp::min(messages.GetCount(), 20);
	for(int i = 0; i < messages.GetCount(); i++) {
		int j = messages.GetCount() - 1 - i;
		const Msg& m = messages[j];
		journal.Set(i, 0, m.a);
		journal.Set(i, 1, m.b);
		journal.Set(i, 2, m.c);
	}
	lock.Leave();
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
