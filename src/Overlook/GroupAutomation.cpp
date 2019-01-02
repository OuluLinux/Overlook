#include "Overlook.h"

#if 0
namespace Overlook {

GroupAutomation::GroupAutomation() {
	prev_time = Time(1970,1,1);
	
}

void GroupAutomation::Init() {
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++)
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	cl_sym.AddTf(EventCore::fast_tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
	
	
	RefreshData();
}

void GroupAutomation::RefreshData() {
	lock.Enter();
	
	data.Clear();
	
	InitData(24, 40, 24*5);
	#ifndef flagDEBUG
	InitData(24, 20, 24*5);
	InitData(12, 40, 24*5);
	InitData(12, 20, 24*5);
	InitData(24, 40, 24*5*2);
	InitData(24, 20, 24*5*2);
	InitData(12, 40, 24*5*2);
	InitData(12, 20, 24*5*2);
	#endif
	
	lock.Leave();
}

void GroupAutomation::InitData(int pattern_period, int group_step, int average_period) {
	PatternMatcher& pm = GetPatternMatcher();
	int sym_count = pm.GetSymbolCount();
	
	PatternMatcherData& pmd = pm.RefreshData(group_step, pattern_period, average_period);
	int data_size = pmd.data.GetCount() / sym_count;
	
	GroupSettings gs;
	gs.period = pattern_period;
	gs.group_step = group_step;
	gs.average_period = average_period;
	
	PatternMatcherGroupData& d = data.GetAdd(gs);
	
	
	for(int i = 0; i < data_size; i++) {
		CombineHash ch;
		for(int j = 0; j < sym_count; j++) {
			int group = pmd.data[i * sym_count + j];
			ch.Put(group);
		}
		unsigned hash = ch;
		
		GroupData& gd = d.group_data.GetAdd(hash);
		
		gd.data.SetCount(sym_count*sym_count);
		
		int row = 0;
		for(int j0 = 0; j0 < sym_count; j0++) {
			int pre_code1 = GetPreCode(j0, 1, i);
			int pre_code2 = GetPreCode(j0, 2, i);
			int pre_code3 = GetPreCode(j0, 3, i);
			
			for(int j1 = 0; j1 < sym_count; j1++) {
				VectorMap<int, OnlineAverage1>& pairdata = gd.data[row++];
				
				double post_code1 = GetPostCode(j1, i);
				
				pairdata.GetAdd(pre_code1).Add(post_code1);
				pairdata.GetAdd(pre_code2).Add(post_code1);
				pairdata.GetAdd(pre_code3).Add(post_code1);
			}
		}
	}
}

GroupData& GroupAutomation::GetGroup(const GroupSettings& gs, int pos) {
	PatternMatcher& pm = GetPatternMatcher();
	int sym_count = pm.GetSymbolCount();
	
	PatternMatcherData& pmd = pm.RefreshData(gs.group_step, gs.period, gs.average_period);
	int data_size = pmd.data.GetCount() / sym_count;
	if (pos == -1) pos = data_size - 1;
	
	PatternMatcherGroupData& d = data.GetAdd(gs);
	
	CombineHash ch;
	for(int j = 0; j < sym_count; j++) {
		int group = pmd.data[pos * sym_count + j];
		ch.Put(group);
	}
	unsigned hash = ch;
	
	GroupData& gd = d.group_data.GetAdd(hash);
	
	return gd;
}

int GroupAutomation::GetPreCode(int sym, int codesize, int pos) {
	int code = 1 << codesize;
	
	ConstBuffer& buffer = cl_sym.GetBuffer(sym, 0, 0);
	ConstBuffer& lobuffer = cl_sym.GetBuffer(sym, 0, 1);
	ConstBuffer& hibuffer = cl_sym.GetBuffer(sym, 0, 2);
	double point = cl_sym.GetDataBridge(sym)->GetPoint();
	
	int begin = min(pos, buffer.GetCount()-1);
	double open = buffer.Get(begin);
	for(int i = 0; i < codesize; i++) {
		double histop = open + close_pips * point;
		double lostop = open - close_pips * point;
		for(int j = begin; j >= 1; j--) {
			double hi = hibuffer.Get(j-1);
			double lo = lobuffer.Get(j-1);
			if (hi >= histop) {
				begin = j;
				code |= 1 << i;
				open = histop;
				break;
			}
			else if (lo <= lostop) {
				begin = j;
				//code |= 0 << i;
				open = lostop;
				break;
			}
		}
	}
	
	return code;
}

double GroupAutomation::GetPostCode(int sym, int pos) {
	ConstBuffer& buffer = cl_sym.GetBuffer(sym, 0, 0);
	ConstBuffer& lobuffer = cl_sym.GetBuffer(sym, 0, 1);
	ConstBuffer& hibuffer = cl_sym.GetBuffer(sym, 0, 2);
	double point = cl_sym.GetDataBridge(sym)->GetPoint();
	
	double open = buffer.Get(pos);
	double histop = open + close_pips * point;
	double lostop = open - close_pips * point;
	for(int j = pos+1; j < buffer.GetCount(); j++) {
		double hi = hibuffer.Get(j-1);
		double lo = lobuffer.Get(j-1);
		if (hi >= histop) {
			return 0.0;
		}
		else if (lo <= lostop) {
			return 1.0;
		}
	}
	return 0.5;
}

void GroupAutomation::Start() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(EventCore::fast_tf);
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	int sym_count = cl_sym.GetSymbolCount();
	MetaTrader& mt = GetMetaTrader();
	const auto& orders = mt.GetOpenOrders();
	
	Vector<int> sym_buy_sig, sym_sell_sig;
	sym_buy_sig.SetCount(sym_count, 0);
	sym_sell_sig.SetCount(sym_count, 0);
	
	Time last_time = idx.Top();
	if (last_time != prev_time) {
		prev_time = last_time;
		
		
		RefreshData();
		
		mt.Data();
		
		int pos = cl_sym.GetBuffer(0,0,0).GetCount() - 1;
		for(int i = 0; i < data.GetCount(); i++) {
			const GroupSettings& gs = data.GetKey(i);
			GroupData& gd = GetGroup(gs, -1);
			
			double max_conf = 0.0;
			int max_conf_sym = -1;
			bool max_conf_sig;
			
			for(int j = 0; j < gd.data.GetCount(); j++) {
				int j0 = j / sym_count;
				int j1 = j % sym_count;
				VectorMap<int, OnlineAverage1>& data = gd.data[j];
				int pre1 = GetPreCode(j0, 1, pos);
				int pre2 = GetPreCode(j0, 2, pos);
				int pre3 = GetPreCode(j0, 3, pos);
				
				OnlineAverage1& d1 = data.GetAdd(pre1);
				OnlineAverage1& d2 = data.GetAdd(pre2);
				OnlineAverage1& d3 = data.GetAdd(pre3);
				double conf1 = fabs(d1.GetMean() - 0.5) * 2.0;
				double conf2 = fabs(d2.GetMean() - 0.5) * 2.0;
				double conf3 = fabs(d3.GetMean() - 0.5) * 2.0;
				if (conf1 > max_conf && d1.count >= 4) {
					max_conf = conf1;
					max_conf_sym = j1;
					max_conf_sig = d1.GetMean() >= 0.5;
				}
				if (conf2 > max_conf && d2.count >= 4) {
					max_conf = conf2;
					max_conf_sym = j1;
					max_conf_sig = d2.GetMean() >= 0.5;
				}
				if (conf3 > max_conf && d3.count >= 4) {
					max_conf = conf3;
					max_conf_sym = j1;
					max_conf_sig = d3.GetMean() >= 0.5;
				}
			}
			
			if (max_conf_sym != -1) {
				if (!max_conf_sig)
					sym_buy_sig[max_conf_sym]++;
				else
					sym_sell_sig[max_conf_sym]++;
			}
		}
		
		Index<int> open_symbols;
		for(int j = 0; j < orders.GetCount(); j++) {
			auto& o = orders[j];
			open_symbols.FindAdd(o.symbol);
		}
		int open_sym = open_symbols.GetCount();
		
		for(int i = 0; i < sym_count; i++) {
			int buy  = sym_buy_sig[i];
			int sell = sym_sell_sig[i];
			if (buy && sell) continue;
			
			int sym_id = net.symbol_ids.GetKey(i);
			double open_lots = GetOpenLots(sym_id);
			
			if (open_lots == 0.0 && open_sym < max_open_sym) {
				double balance = mt.AccountBalance();
				double lots = balance * 0.001 / max_open_sym;
				if (sell) lots *= -1;
				SetRealSymbolLots(i, lots);
				open_sym++;
			}
		}
	} else {
		for(int i = 0; i < sym_count; i++) {
			int sym_id = net.symbol_ids.GetKey(i);
			double open_lots = GetOpenLots(sym_id);
			if (open_lots == 0.0) continue;
			
			double point = cl_sym.GetDataBridge(i)->GetPoint();
			
			for(int j = 0; j < orders.GetCount(); j++) {
				auto& o = orders[j];
				if (o.symbol != sym_id) continue;
				
				if (o.type == OP_BUY) {
					double close = mt.RealtimeBid(sym_id);
					int profit_pips = (close - o.open) / point;
					if (profit_pips >= close_pips || profit_pips <= -close_pips) {
						SetRealSymbolLots(i, 0);
						break;
					}
				}
				if (o.type == OP_SELL) {
					double close = mt.RealtimeAsk(sym_id);
					int profit_pips = (o.open - close) / point;
					if (profit_pips >= close_pips || profit_pips <= -close_pips) {
						SetRealSymbolLots(i, 0);
						break;
					}
				}
			}
		}
	}
}

void GroupAutomation::SetRealSymbolLots(int sym_, double lots) {
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

double GroupAutomation::GetOpenLots(int sym) {
	MetaTrader& mt = GetMetaTrader();
	const auto& orders = mt.GetOpenOrders();
	double lots = 0;
	for(int i = 0; i < orders.GetCount(); i++) {
		if (orders[i].symbol == sym)
			lots += orders[i].volume;
	}
	return lots;
}



GroupAutomationCtrl::GroupAutomationCtrl() {
	Add(slider.TopPos(0, 30).HSizePos(0, 200));
	Add(date.TopPos(0, 30).RightPos(0, 200));
	Add(split.HSizePos().VSizePos(30));
	
	slider.MinMax(0,1);
	slider.SetData(0);
	slider <<= THISBACK(Data);
	
	split.Horz();
	split << group_list << sym_list << data_list;
	
	group_list.AddColumn("Group step");
	group_list.AddColumn("Pattern period");
	group_list.AddColumn("Average period");
	group_list.AddColumn("Best sig");
	group_list <<= THISBACK(Data);
	sym_list.AddColumn("Pre-symbol");
	sym_list.AddColumn("Post-symbol");
	sym_list <<= THISBACK(Data);
	data_list.AddColumn("Code");
	data_list.AddColumn("Probability");
	data_list.AddColumn("Event count");
	data_list.AddColumn("Is active");
}

void GroupAutomationCtrl::Data() {
	GroupAutomation& ga = GetGroupAutomation();
	System& sys = GetSystem();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(EventCore::fast_tf);
	PatternMatcher& pm = GetPatternMatcher();
	int sym_count = pm.GetSymbolCount();
	System::NetSetting& net = sys.GetNet(0);
	
	if (slider.GetMax() != idx.GetCount()-1 && idx.GetCount() > 0)
		slider.MinMax(0, idx.GetCount()-1);
	
	int pos = slider.GetData();
	
	date.SetLabel(Format("%", idx[pos]));
	
	ga.lock.Enter();
	for(int i = 0; i < ga.data.GetCount(); i++) {
		const GroupSettings& gs = ga.data.GetKey(i);
		group_list.Set(i, 0, gs.group_step);
		group_list.Set(i, 1, gs.period);
		group_list.Set(i, 2, gs.average_period);
		GroupData& gd = ga.GetGroup(gs, pos);
		
		double max_conf = 0.0;
		int max_conf_sym = -1;
		bool max_conf_sig;
		
		for(int j = 0; j < gd.data.GetCount(); j++) {
			int j0 = j / sym_count;
			int j1 = j % sym_count;
			VectorMap<int, OnlineAverage1>& data = gd.data[j];
			int pre1 = ga.GetPreCode(j0, 1, pos);
			int pre2 = ga.GetPreCode(j0, 2, pos);
			int pre3 = ga.GetPreCode(j0, 3, pos);
			
			OnlineAverage1& d1 = data.GetAdd(pre1);
			OnlineAverage1& d2 = data.GetAdd(pre2);
			OnlineAverage1& d3 = data.GetAdd(pre3);
			double conf1 = fabs(d1.GetMean() - 0.5) * 2.0;
			double conf2 = fabs(d2.GetMean() - 0.5) * 2.0;
			double conf3 = fabs(d3.GetMean() - 0.5) * 2.0;
			if (conf1 > max_conf && d1.count >= 4) {
				max_conf = conf1;
				max_conf_sym = j1;
				max_conf_sig = d1.GetMean() >= 0.5;
			}
			if (conf2 > max_conf && d2.count >= 4) {
				max_conf = conf2;
				max_conf_sym = j1;
				max_conf_sig = d2.GetMean() >= 0.5;
			}
			if (conf3 > max_conf && d3.count >= 4) {
				max_conf = conf3;
				max_conf_sym = j1;
				max_conf_sig = d3.GetMean() >= 0.5;
			}
		}
		
		String s;
		if (max_conf_sym != -1) {
			s << (max_conf_sig ? "Sell " : "Buy ");
			s << net.symbols.GetKey(max_conf_sym);
			s << " confidence " << DblStr(max_conf);
		}
		group_list.Set(i, 3, s);
	}
	
	if (group_list.IsCursor()) {
		int gs_id = group_list.GetCursor();
		if (gs_id < ga.data.GetCount()) {
			GroupData& gd = ga.GetGroup(ga.data.GetKey(gs_id), pos);
			
			for(int i = 0; i < gd.data.GetCount(); i++) {
				int j0 = i / sym_count;
				int j1 = i % sym_count;
				if (j0 >= net.symbols.GetCount()) break;
				String presym = net.symbols.GetKey(j0);
				String postsym = net.symbols.GetKey(j1);
				sym_list.Set(i, 0, presym);
				sym_list.Set(i, 1, postsym);
			}
			sym_list.SetCount(gd.data.GetCount());
			
			if (sym_list.IsCursor()) {
				int sym_id = sym_list.GetCursor();
				int j0 = sym_id / sym_count;
				int j1 = sym_id % sym_count;
				
				if (sym_id < gd.data.GetCount()) {
					VectorMap<int, OnlineAverage1>& data = gd.data[sym_id];
					
					int pre1 = ga.GetPreCode(j0, 1, pos);
					int pre2 = ga.GetPreCode(j0, 2, pos);
					int pre3 = ga.GetPreCode(j0, 3, pos);
					//int post1 = ga.GetPostCode(j0, pos);
					
					for(int i = 0; i < data.GetCount(); i++) {
						int code = data.GetKey(i);
						double prob = data[i].GetMean();
						int events = data[i].count;
						data_list.Set(i, 0, code);
						data_list.Set(i, 1, prob);
						data_list.Set(i, 2, events);
						data_list.Set(i, 3, (code == pre1 || code == pre2 || code == pre3) ? "X" : "");
					}
					data_list.SetCount(data.GetCount());
				}
			}
		}
	}
	
	ga.lock.Leave();
	
}

}
#endif
