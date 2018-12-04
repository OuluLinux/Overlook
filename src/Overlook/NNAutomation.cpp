#include "Overlook.h"


namespace Overlook {
using namespace libmt;

NNAutomation::NNAutomation() {
	
}

void NNAutomation::Init() {
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
	
	
	
	if (!ses.GetStepCount()) {
		ses.MakeLayers(
			"[\n"
			"\t{\"type\":\"input\", \"input_width\":" + IntStr(sym_count) + ", \"input_height\":" + IntStr(input_length) + ", \"input_depth\":1},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
			"\t{\"type\":\"regression\", \"neuron_count\":" + IntStr(sym_count) + "},\n"
			"\t{\"type\":\"adadelta\", \"learning_rate\":1, \"batch_size\":50, \"l1_decay\":0.001, \"l2_decay\":0.001}\n"
			"]\n"
		);
		ses.Reset();
	}
	
	if (1) {
		int data_count = cl_sym.GetBuffer(0, 0, 0).GetCount();
		
		cl_net.Refresh();
		cl_sym.Refresh();
		
		ConvNet::SessionData& d = ses.Data();
		d.BeginDataResult(sym_count, data_count-input_length-1, sym_count * input_length);
		
		for(int i = input_length, row = 0; i < data_count-1; i++, row++) {
			
			int col = 0;
			for(int j = 0; j < sym_count; j++) {
				ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
				
				double next = buf.Get(i);
				double next2 = buf.Get(i+1);
				for(int k = 0; k < input_length; k++) {
					double cur = buf.Get(i-k);
					double ch = next / cur - 1.0;
					ch *= 100;
					if (!IsFin(ch)) ch = 0;
					d.SetData(row, col++, ch);
					next = cur;
				}
				double ch = next2 / next - 1.0;
				ch *= 100;
				d.SetResult(row, j, ch);
			}
		}
		d.EndData();
		
		ses.StartTraining();
	}
}

void NNAutomation::Start() {
	System& sys = GetSystem();
	
	if (ses.GetStepCount() > 200000) {
		if (ses.IsTraining()) {
			ses.StopTraining();
			StoreThis();
		}
		
		ConvNet::Volume vol;
		vol.Init(sym_count, input_length, 1, 0);
		int col = 0;
		for(int j = 0; j < sym_count; j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			int pos = buf.GetCount()-1;
			
			double next = buf.Get(pos);
			for(int k = 0; k < input_length; k++) {
				double cur = buf.Get(pos-k);
				double ch = next / cur - 1.0;
				ch *= 100;
				if (!IsFin(ch)) ch = 0;
				vol.Set(col++, ch);
				next = cur;
			}
		}
		
		ConvNet::Net& net = ses.GetNetwork();
		ConvNet::Volume& result = net.Forward(vol);
		
		System::NetSetting& n = sys.GetNet(0);
		Vector<double> sym_lots;
		sym_lots.SetCount(n.symbols.GetCount(), 0);
		
		for(int i = 0; i < sym_count; i++) {
			System::NetSetting& n = sys.GetNet(i);
			
			double pred = result.Get(i);
			int sig = pred > 0 ? +1 : -1;
			
			for(int j = 0; j < n.symbols.GetCount(); j++) {
				sym_lots[j] += sig * n.symbols[j] * 0.01;
			}
		}
		
		double balance = GetMetaTrader().AccountBalance();
		double max_lots_sum = balance * 0.001;
		double lot_sum = 0;
		for(int j = 0; j < sym_lots.GetCount(); j++)
			lot_sum += fabs(sym_lots[j]);
		double factor = max_lots_sum / lot_sum;
		
		for(int i = 0; i < sym_lots.GetCount(); i++)
			SetRealSymbolLots(i, sym_lots[i] * factor);
	}
	
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
	Add(tabs.SizePos());
	tabs.Add(ses_view.SizePos(), "Session");
	tabs.Add(train_view.SizePos(), "Training");
	tabs.Add(status.SizePos(), "Status");
	
	NNAutomation& a = GetNNAutomation();
	ses_view.SetSession(a.ses);
	train_view.SetSession(a.ses);
	train_view.SetModeLoss();
}

void NNAutomationCtrl::Data() {
	if (init) {
		ses_view.RefreshLayers();
		init = false;
	}
	
	int tab = tabs.Get();
	if (tab == 0) ses_view.Refresh();
	if (tab == 1) train_view.RefreshData();
	if (tab == 2) {
		NNAutomation& a = GetNNAutomation();
		ConvNet::Session& ses = a.ses;
		String s;
		s << "   Forward time per example: " << ses.GetForwardTime() << "\n";
		s << "   Backprop time per example: " << ses.GetBackwardTime() << "\n";
		s << "   Regression loss: " << ses.GetLossAverage() << "\n";
		s << "   L2 Weight decay loss: " << ses.GetL2DecayLossAverage() << "\n";
		s << "   L1 Weight decay loss: " << ses.GetL1DecayLossAverage() << "\n";
		s << "   Examples seen: " << ses.GetStepCount();
		status.SetLabel(s);
	}
}


}
