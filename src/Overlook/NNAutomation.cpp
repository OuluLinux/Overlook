#include "Overlook.h"


namespace Overlook {
using namespace libmt;

NNAutomation::NNAutomation() {
	
}

void NNAutomation::Init() {
	System& sys = GetSystem();
	
	sys.GetNNCoreQueue(ci_queue, 4, sys.FindNN<IntPerfNN>());
	
}

void NNAutomation::Start() {
	System& sys = GetSystem();
	
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		sys.ProcessNN(*ci_queue[i], true);
	}
	
	//for(int i = 0; i < used_sym_lots.GetCount(); i++)
	//	SetRealSymbolLots(i, used_sym_lots[i] * factor);

	
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
	NNAutomation& a = GetNNAutomation();
	
	ses_view.SetCount(NNAutomation::tf_count);
	train_view.SetCount(NNAutomation::tf_count);
	status.SetCount(NNAutomation::tf_count);
	
	Add(tabs.SizePos());
	for(int i = 0; i < NNAutomation::tf_count; i++) {
		tabs.Add(ses_view[i].SizePos(), "Session " + IntStr(i));
		tabs.Add(train_view[i].SizePos(), "Training " + IntStr(i));
		tabs.Add(status[i].SizePos(), "Status " + IntStr(i));
		
		ses_view[i].SetSession(a.ses[i]);
		train_view[i].SetSession(a.ses[i]);
		train_view[i].SetModeLoss();
	}
	
}

void NNAutomationCtrl::Data() {
	if (init) {
		for(int i = 0; i < NNAutomation::tf_count; i++)
			ses_view[i].RefreshLayers();
		init = false;
	}
	
	int tab = tabs.Get();
	int tab_mode = tab % 3;
	int tfi = tab / 3;
	
	if (tab_mode == 0) ses_view[tfi].Refresh();
	if (tab_mode == 1) train_view[tfi].RefreshData();
	if (tab_mode == 2) {
		NNAutomation& a = GetNNAutomation();
		ConvNet::Session& ses = a.ses[tfi];
		String s;
		s << "   Forward time per example: " << ses.GetForwardTime() << "\n";
		s << "   Backprop time per example: " << ses.GetBackwardTime() << "\n";
		s << "   Regression loss: " << ses.GetLossAverage() << "\n";
		s << "   L2 Weight decay loss: " << ses.GetL2DecayLossAverage() << "\n";
		s << "   L1 Weight decay loss: " << ses.GetL1DecayLossAverage() << "\n";
		s << "   Examples seen: " << ses.GetStepCount();
		status[tfi].SetLabel(s);
	}
}


}
