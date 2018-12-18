#include "Overlook.h"


namespace Overlook {
using namespace libmt;

NNAutomation::NNAutomation() {
	
}

void NNAutomation::Init() {
	System& sys = GetSystem();
	
	int tf = 1;
	
	sys.GetNNCoreQueue(ci_queue, tf, sys.FindNN<MartNN>());
	
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
}

void NNAutomation::Start() {
	System& sys = GetSystem();
	
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		sys.ProcessNN(*ci_queue[i], true);
	}
	
	
	NNCore& c = *ci_queue.Top()->core;
	
	cl_net.Refresh();
	cl_sym.Refresh();
	int pos = cl_sym.GetBuffer(0, 0, 4).GetCount() - 1;
	
	Vector<double> output;
	c.Start(true, pos, output);
	
	double balance = GetMetaTrader().AccountBalance();
	double max_lots_sum = balance * 0.001;
	double lot_sum = 0;
	for(int j = 0; j < output.GetCount(); j++)
		lot_sum += fabs(output[j]);
	double factor = max_lots_sum / lot_sum;

	for(int i = 0; i < output.GetCount(); i++)
		SetRealSymbolLots(i, output[i] * factor);

}

void NNAutomation::SetRealSymbolLots(int sym_, double lots) {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	const auto& orders = mt.GetOpenOrders();
	
	System::NetSetting& net = sys.GetNet(0);
	int sym_id = net.symbol_ids.GetKey(sym_);
	
	if (lots > 10) lots = 10;
	if (lots < -10) lots -10;
	
	double buy_lots, sell_lots;
	if (lots > 0) {
		buy_lots = lots;
		sell_lots = 0;
	} else {
		sell_lots = -lots;
		buy_lots = 0;
	}
		
	double cur_buy_lots = 0;
	double cur_sell_lots = 0;
	
	
	for(int i = 0; i < orders.GetCount(); i++) {
		const auto& o = orders[i];
		if (o.symbol != sym_id) continue;
		if (o.type == OP_BUY) {
			cur_buy_lots += o.volume;
		}
		else if (o.type == OP_SELL) {
			cur_sell_lots += o.volume;
		}
	}
	
	double close_buy_lots = 0, close_sell_lots = 0;
	double open_buy_lots = 0, open_sell_lots = 0;
	
	if (buy_lots == 0 && sell_lots == 0) {
		close_buy_lots = cur_buy_lots;
		close_sell_lots = cur_sell_lots;
	}
	else if (buy_lots > 0) {
		close_sell_lots = cur_sell_lots;
		close_buy_lots = max(0.0, cur_buy_lots - buy_lots);
		open_buy_lots = max(0.0, buy_lots - cur_buy_lots);
	}
	else if (sell_lots > 0) {
		close_buy_lots = cur_buy_lots;
		close_sell_lots = max(0.0, cur_sell_lots - sell_lots);
		open_sell_lots = max(0.0, sell_lots - cur_sell_lots);
	}
	
	if (close_buy_lots > 0.0 || close_sell_lots > 0.0) {
		for(int i = orders.GetCount()-1; i >= 0; i--) {
			auto& o = orders[i];
			if (o.symbol != sym_id) continue;
			
			if (o.type == OP_BUY && close_buy_lots > 0.0) {
				if (o.volume <= close_buy_lots) {
					close_buy_lots -= o.volume;
					mt.CloseOrder(o, o.volume);
				}
				else {
					mt.CloseOrder(o, close_buy_lots);
					close_buy_lots = 0;
				}
			}
			else if (o.type == OP_SELL && close_sell_lots > 0.0) {
				if (o.volume <= close_sell_lots) {
					close_sell_lots -= o.volume;
					mt.CloseOrder(o, o.volume);
				}
				else {
					mt.CloseOrder(o, close_sell_lots);
					close_sell_lots = 0;
				}
			}
		}
	}
	
	if (open_buy_lots > 0)
		mt.OpenOrder(sym_id, OP_BUY, open_buy_lots);
	else if (open_sell_lots > 0)
		mt.OpenOrder(sym_id, OP_SELL, open_sell_lots);
	
}




NNAutomationCtrl::NNAutomationCtrl() {
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << queuelist << itemctrl;
	hsplit.SetPos(2000);
	queuelist.AddColumn("Name");
	queuelist.AddColumn("Progress");
	queuelist.ColumnWidths("2 1");
	queuelist <<= THISBACK(SetItem);
}

void NNAutomationCtrl::SetItem() {
	Data();
}

void NNAutomationCtrl::Data() {
	NNAutomation& a = GetNNAutomation();
	
	if (is_initing) return;
	
	if (init) {
		if (a.ci_queue.IsEmpty()) return;
		
		is_initing = true;
		
		for(int i = 0; i < a.ci_queue.GetCount(); i++) {
			NNCore& c = *a.ci_queue[i]->core;
			String fac = System::NNCoreFactories()[a.ci_queue[i]->factory].a;
			String tf = GetSystem().GetPeriodString(a.ci_queue[i]->tf);
			queuelist.Set(i * 2 + 0, 0, tf + " " + fac + " test");
			queuelist.Set(i * 2 + 1, 0, tf + " " + fac + " rt");
		}
		queuelist.SetCursor(0);
		
		int count = a.ci_queue.GetCount() * 2;
		
		tabslist.SetCount(count);
		ses_view.SetCount(count);
		train_view.SetCount(count);
		status.SetCount(count);
		draws.SetCount(count);
		
		for(int i = 0; i < count; i++) {
			TabCtrl& tabs = tabslist[i];
			
			int ci_id = i / 2;
			int is_rt = i % 2;
			NNCore& c = *a.ci_queue[ci_id]->core;
			ConvNet::Session& ses = c.data[is_rt].ses;
			ses_list.Add(&ses);
			
			draws[i].buf = &c.data[is_rt].buf;
			
			tabs.Add(draws[i].SizePos(), "Buffer drawer");
			tabs.Add(ses_view[i].SizePos(), "Session");
			tabs.Add(train_view[i].SizePos(), "Training");
			tabs.Add(status[i].SizePos(), "Status");
			
			ses_view[i].SetSession(ses);
			train_view[i].SetSession(ses);
			train_view[i].SetModeLoss();
			
			ses_view[i].RefreshLayers();
		}
		
		init = false;
		is_initing = false;
	}
	
	for(int i = 0; i < a.ci_queue.GetCount(); i++) {
		NNCore& c = *a.ci_queue[i]->core;
		
		queuelist.Set(i * 2 + 0, 1, c.data[0].ses.GetStepCount() * 1000 / NNCore::MAX_TRAIN_STEPS);
		queuelist.Set(i * 2 + 1, 1, c.data[1].ses.GetStepCount()   * 1000 / NNCore::MAX_TRAIN_STEPS);
		queuelist.SetDisplay(i * 2 + 0, 1, ProgressDisplay());
		queuelist.SetDisplay(i * 2 + 1, 1, ProgressDisplay());
	}
	
	int id = queuelist.GetCursor();
	TabCtrl& tabs = tabslist[id];
	if (&tabs != prev_tabs) {
		if (prev_tabs)
			itemctrl.RemoveChild(prev_tabs);
		prev_tabs = &tabs;
		itemctrl.Add(tabs.SizePos());
	}
	
	int tab_mode = tabs.Get();
	
	if (tab_mode == 0) draws[id].Refresh();
	if (tab_mode == 1) ses_view[id].Refresh();
	if (tab_mode == 2) train_view[id].RefreshData();
	if (tab_mode == 3) {
		ConvNet::Session& ses = *ses_list[id];
		String s;
		s << "   Forward time per example: " << ses.GetForwardTime() << "\n";
		s << "   Backprop time per example: " << ses.GetBackwardTime() << "\n";
		s << "   Regression loss: " << ses.GetLossAverage() << "\n";
		s << "   L2 Weight decay loss: " << ses.GetL2DecayLossAverage() << "\n";
		s << "   L1 Weight decay loss: " << ses.GetL1DecayLossAverage() << "\n";
		s << "   Examples seen: " << ses.GetStepCount();
		status[id].SetLabel(s);
	}
}


}
