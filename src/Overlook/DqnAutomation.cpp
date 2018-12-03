#include "Overlook.h"


namespace Overlook {
using namespace libmt;

DqnAutomation::DqnAutomation() {
	
}

void DqnAutomation::Init() {
	System& sys = GetSystem();
	
	LoadThis();
	
	int tf = 5;
	
	for(int i = 0; i < sym_count; i++) {
		cl_net.AddSymbol("Net" + IntStr(i));
	}
	cl_net.AddTf(tf);
	cl_net.AddIndi(0);
	cl_net.Init();
	cl_net.Refresh();
	
	
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
	
	
	points.SetCount(cl_sym.GetSymbolCount(), 0);
	for(int i = 0; i < cl_sym.GetSymbolCount(); i++) {
		double point = cl_sym.GetDataBridge(i)->GetPoint();
		points[i] = point;
	}
	
	
	//#ifdef flagDEBUG
	max_rounds = 100000;
	//#else
	//max_rounds = 20000000;
	//#endif
	training_pts.SetCount(max_rounds, 0);
	
	if (!iter) {
		dqn.Init(1, input_count, output_count);
		dqn.Reset();
		dqn.SetGamma(0);
	}
	
	if (iter < max_rounds) {
		Thread::Start(THISBACK(Process));
	}
}

void DqnAutomation::Start() {
	
	if (iter >= max_rounds) {
		
	}
	
}

void DqnAutomation::Process() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	
	int data_count = cl_sym.GetBuffer(0, 0, 0).GetCount();
	
	cl_net.Refresh();
	cl_sym.Refresh();
	
	TimeStop ts;
	
	ResetOrders(iter % data_count);
	
	int this_process_iter = 0;
	
	while (iter < max_rounds) {
		
		int pos = iter % data_count;
		if (pos == 0) ResetOrders(pos);
		
		if (this_process_iter > 0) {
			double prev_equity = equity;
			RefreshEquity(pos);
			double reward = (equity / prev_equity - 1.0) * 10000;
			dqn.Learn(reward, true);
			LOG("equity " << equity << "\treward " << reward);
			if (equity < 2000) {
				balance = 10000;
				RefreshEquity(pos);
			}
		}
		
		
		RefreshInput(pos);
		
		int a = dqn.Act(slist);
		
		if (a < 10) {
			Signal(pos, +1, a);
		}
		else if (a < 20) {
			Signal(pos, -1, a-10);
		}
		else {
			// idle
		}
		
		
		training_pts[iter] = equity;
		
		if (ts.Elapsed() > 60*1000) {
			StoreThis();
			ts.Reset();
		}
		
		iter++;
		this_process_iter++;
	}
	
	
	StoreThis();
}

void DqnAutomation::ResetOrders(int pos) {
	equity = 10000;
	balance = 10000;
	
	symstats.SetCount(cl_sym.GetSymbolCount());
	for(int i = 0; i < symstats.GetCount(); i++) {
		SymStat& ss = symstats[i];
		ss.open = cl_sym.GetBuffer(i, 0, 0).Get(pos);
		ss.lots = 0.01;
	}
}

void DqnAutomation::RefreshEquity(int pos) {
	equity = balance;
	for(int i = 0; i < symstats.GetCount(); i++) {
		SymStat& ss = symstats[i];
		if (!ss.type) continue;
		double close = cl_sym.GetBuffer(i, 0, 0).Get(pos);
		double profit = ((close / ss.open) - 1.0) * 100000 * ss.lots;
		if (ss.type < 0) profit *= -1;
		equity += profit;
	}
}

void DqnAutomation::RefreshInput(int pos) {
	slist.SetCount(input_count, 0);
	
	int k = 0;
	double absmax = 0;
	for(int i = 0; i < cl_net.GetSymbolCount(); i++) {
		ConstBuffer& buf = cl_net.GetBuffer(i, 0, 0);
		double last = buf.Get(pos);
		for(int j = 0; j < input_length; j++) {
			int pos2 = pos - 1 - j;
			
			if (pos2 < 0) pos2 = 0;
			
			double o = buf.Get(pos2);
			
			double f = last / o - 1.0;
			if (fabs(f) > absmax) absmax = fabs(f);
			
			last = o;
			slist[k++] = absmax;
		}
	}
	
	/*if (absmax > 0.0) {
		for(int i = 0; i < input_count; i++) {
			slist[i] /= absmax;
		}
	}*/
}

void DqnAutomation::Signal(int pos, int op, int net) {
	System& sys = GetSystem();
	
	System::NetSetting& n = sys.GetNet(net);
	
	for(int i = 0; i < cl_sym.GetSymbolCount(); i++) {
		
		int type = n.symbols[i];
		if (op < 0) type *= -1;
		
		SymStat& ss = symstats[i];
		
		if (ss.type != 0) {
			double close = cl_sym.GetBuffer(i, 0, 0).Get(pos);
			double profit = ((close / ss.open) - 1.0) * 100000 * ss.lots;
			if (ss.type < 0) profit *= -1;
			balance += profit;
		}

		ss.open = cl_sym.GetBuffer(i, 0, 0).Get(pos);
		if (ss.type != type) {
			if (type < 0)	ss.open -= 3 * points[i];
			else			ss.open += 3 * points[i];
		}
		
		ss.type = type;
	}
}


DqnAutomationCtrl::DqnAutomationCtrl() {
	Add(ctrl.SizePos());
}

void DqnAutomationCtrl::Data() {
	ctrl.Refresh();
}


}
