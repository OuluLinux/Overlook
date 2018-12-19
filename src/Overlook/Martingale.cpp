#include "Overlook.h"

#if 0
namespace Overlook {
using namespace libmt;

double AbsMin(double absmax, double d) {
	if (absmax < 0.0)
		return 0.0;
	if (d > +absmax) return +absmax;
	if (d < -absmax) return -absmax;
	return d;
}


Martingale::Martingale() {
	
}
	
void Martingale::Init() {
	System& sys = GetSystem();
	
	LoadThis();
	
	int tf = 5;
	
	nets.SetCount(sys.GetNetCount());
	
	for(int i = 0; i < sys.GetNetCount(); i++) {
		NetSetting& n = nets[i];
		
		n.cl_net.AddSymbol("Net" + IntStr(i));
		n.cl_net.AddTf(tf);
		n.cl_net.AddIndi(0);
		n.cl_net.Init();
		n.cl_net.Refresh();
		
		
		n.cl_rsi.AddSymbol("Net" + IntStr(i));
		n.cl_rsi.AddTf(tf);
		n.cl_rsi.AddIndi(sys.Find<RelativeStrengthIndex>()).AddArg(10);
		n.cl_rsi.Init();
		n.cl_rsi.Refresh();
		
	}
	
	
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
	
	if (!opt.GetRound()) {
		int arg_count = ARG_COUNT * nets.GetCount() + 1;
		
		opt.Max().SetCount(arg_count, 1);
		opt.Min().SetCount(arg_count, 0);
		for(int i = 0; i < nets.GetCount(); i++) {
			int j = i * ARG_COUNT;
			opt.Min()[j + LOTS] = 0.01;
			opt.Max()[j + LOTS] = 100;
			opt.Min()[j + LOTEXP] = 1;
			opt.Max()[j + LOTEXP] = 100;
			opt.Min()[j + TP] = 1;
			opt.Max()[j + TP] = 200;
			opt.Min()[j + SL] = 1;
			opt.Max()[j + SL] = 100;
			opt.Min()[j + PIPSTEP] = 1;
			opt.Max()[j + PIPSTEP] = 100;
			opt.Min()[j + TRAILSTART] = 1;
			opt.Max()[j + TRAILSTART] = 100;
			opt.Min()[j + TRAILSTOP] = 1;
			opt.Max()[j + TRAILSTOP] = 100;
			opt.Min()[j + MAXTRADES] = 1;
			opt.Max()[j + MAXTRADES] = 100;
			opt.Min()[j + USETRAIL] = 0;
			opt.Max()[j + USETRAIL] = 2;
		}
		opt.SetMaxGenerations(100);
		opt.Init(arg_count, 100);
	}
	
	max_rounds = opt.GetMaxRounds();
	
	if (opt.GetRound() < max_rounds) {
		Thread::Start(THISBACK(Process));
	}
}

void Martingale::Start() {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	
	if (opt.GetRound() < max_rounds)
		return;
	
	mt.Data();
	
	for(int i = 0; i < nets.GetCount(); i++) {
		NetSetting& n = nets[i];
		n.cl_net.Refresh();
		n.cl_rsi.Refresh();
	}
	cl_sym.Refresh();
	
	int data_count = cl_sym.GetBuffer(0, 0, 0).GetCount();
	
	System::NetSetting& net = sys.GetNet(0);
	
	Vector<double> sym_lots;
	sym_lots.SetCount(net.symbols.GetCount());
	
	
	const Vector<double>& trial = opt.GetBestSolution();
	
	for(int i = 0; i < nets.GetCount(); i++) {
		int j = i * ARG_COUNT;
		NetSetting& n = nets[i];
		
		n.lots				= trial[j + LOTS];
		n.lot_exponent		= trial[j + LOTEXP];
		n.take_profit		= trial[j + TP];
		n.stop_loss			= trial[j + SL];
		n.pip_step			= trial[j + PIPSTEP];
		n.trail_start		= trial[j + TRAILSTART];
		n.trail_stop		= trial[j + TRAILSTOP];
		n.max_trades		= trial[j + MAXTRADES];
		n.use_trailing_stop	= trial[j + USETRAIL] >= 1.0;
		
		if (n.lots < 0.01) n.lots = 0.01;
		if (n.lots > 1000) n.lots = 1000;
	}
	double lot_multiplier = trial[trial.GetCount() - 1];
	
	balance = 10000;
	
	for(int i = 0; i < nets.GetCount(); i++) {
		NetSetting& n = nets[i];
		n.equity = 10000;
		n.balance = 10000;
		n.orders.Clear();
	}
	
	orders.Clear();
	
	double fixed_balance = mt.AccountBalance();
	
	for(int i = 0; i < data_count; i++) {
		pos = i;
		
		balance = fixed_balance;
		
		double max_lots = balance * 0.0006 / net.symbols.GetCount();
		
		for(int j = 0; j < sym_lots.GetCount(); j++)
			sym_lots[j] = 0;
		
		for(int j = 0; j < nets.GetCount(); j++) {
			NetSetting& n = nets[j];
			
			n.pos = i;
			
			n.balance = fixed_balance;
			
			n.CheckStops();
			n.CheckPendingOrders();
			n.RefreshEquity();
			n.ProcessNet();
			
			if (n.balance <= 0) {
				n.balance = 0;
				n.orders.Clear();
			}
			
			double lots = 0.0;
			for(int k = 0; k < n.orders.GetCount(); k++) {
				Order& o = n.orders[k];
				if (o.type == OP_BUY) {
					lots += o.lots;
				}
				else if (o.type == OP_SELL) {
					lots -= o.lots;
				}
			}
			
			if (lots != 0.0) {
				lots = lots / n.balance * balance;
				lots *= lot_multiplier;
				for(int k = 0; k < net.symbols.GetCount(); k++) {
					int sig = net.symbols[k];
					sym_lots[k] += sig * lots;
				}
			}
		}
		
		if (i < data_count-1) {
			for(int j = 0; j < sym_lots.GetCount(); j++)
				SetSymbolLots(j, sym_lots[j]);
		} else {
			for(int j = 0; j < sym_lots.GetCount(); j++)
				SetRealSymbolLots(j, sym_lots[j]);
		}
	}
	
}

void Martingale::Process() {
	System& sys = GetSystem();
	
	
	for(int i = 0; i < nets.GetCount(); i++) {
		NetSetting& n = nets[i];
		n.cl_net.Refresh();
		n.cl_rsi.Refresh();
	}
	cl_sym.Refresh();
	
	int data_count = cl_sym.GetBuffer(0, 0, 0).GetCount();
	
	System::NetSetting& net = sys.GetNet(0);
	
	Vector<double> sym_lots;
	sym_lots.SetCount(net.symbols.GetCount());
	
	while (!opt.IsEnd()) {
		
		opt.Start();
		
		const Vector<double>& trial = opt.GetTrialSolution();
		
		for(int i = 0; i < nets.GetCount(); i++) {
			int j = i * ARG_COUNT;
			NetSetting& n = nets[i];
			
			n.lots				= trial[j + LOTS];
			n.lot_exponent		= trial[j + LOTEXP];
			n.take_profit		= trial[j + TP];
			n.stop_loss			= trial[j + SL];
			n.pip_step			= trial[j + PIPSTEP];
			n.trail_start		= trial[j + TRAILSTART];
			n.trail_stop		= trial[j + TRAILSTOP];
			n.max_trades		= trial[j + MAXTRADES];
			n.use_trailing_stop	= trial[j + USETRAIL] >= 1.0;
			
			if (n.lots < 0.01) n.lots = 0.01;
			if (n.lots > 1000) n.lots = 1000;
		}
		double lot_multiplier = trial[trial.GetCount() - 1];
		
		balance = 10000;
		
		for(int i = 0; i < nets.GetCount(); i++) {
			NetSetting& n = nets[i];
			n.equity = 10000;
			n.balance = 10000;
			n.orders.Clear();
		}
		
		orders.Clear();
		
		for(int i = 0; i < data_count; i++) {
			pos = i;
			
			for(int j = 0; j < sym_lots.GetCount(); j++)
				sym_lots[j] = 0;
			
			for(int j = 0; j < nets.GetCount(); j++) {
				NetSetting& n = nets[j];
				
				n.pos = i;
				
				n.CheckStops();
				n.CheckPendingOrders();
				n.RefreshEquity();
				n.ProcessNet();
					
				if (n.balance <= 0) {
					n.balance = 0;
					n.orders.Clear();
				}
				
				double lots = 0.0;
				for(int k = 0; k < n.orders.GetCount(); k++) {
					Order& o = n.orders[k];
					if (o.type == OP_BUY) {
						lots += o.lots;
					}
					else if (o.type == OP_SELL) {
						lots -= o.lots;
					}
				}
				
				if (lots != 0.0) {
					lots = lots / n.balance * balance;
					lots *= lot_multiplier;
					for(int k = 0; k < net.symbols.GetCount(); k++) {
						int sig = net.symbols[k];
						sym_lots[k] += sig * lots;
					}
				}
			}
			
			if (balance <= 0)
				break;
			/*
			double max_lots = balance * 0.001;
			double lot_sum = 0;
			for(int j = 0; j < sym_lots.GetCount(); j++)
				lot_sum += sym_lots[j];
			double factor = max_lots / lot_sum;
			if (factor > 1) factor = 1;
			if (factor < 0) factor = 0;
			*/
			for(int j = 0; j < sym_lots.GetCount(); j++)
				SetSymbolLots(j, sym_lots[j]);
		}
		
		for(int i = orders.GetCount()-1; i >= 0; i--)
			CloseOrder(i, orders[i].lots);
		
		
		if (balance > 0)
			training_pts.Add(balance);
		opt.Stop(balance);
		
	}
	
	
	StoreThis();
}

void Martingale::SetSymbolLots(int sym, double lots) {
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
		Order& o = orders[i];
		if (o.symbol != sym) continue;
		if (o.type == OP_BUY) {
			cur_buy_lots += o.lots;
		}
		else if (o.type == OP_SELL) {
			cur_sell_lots += o.lots;
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
			Order& o = orders[i];
			if (o.symbol != sym) continue;
			
			if (o.type == OP_BUY && close_buy_lots > 0.0) {
				if (o.lots <= close_buy_lots) {
					close_buy_lots -= o.lots;
					CloseOrder(i, o.lots);
				}
				else {
					CloseOrder(i, close_buy_lots);
					close_buy_lots = 0;
				}
			}
			else if (o.type == OP_SELL && close_sell_lots > 0.0) {
				if (o.lots <= close_sell_lots) {
					close_sell_lots -= o.lots;
					CloseOrder(i, o.lots);
				}
				else {
					CloseOrder(i, close_sell_lots);
					close_sell_lots = 0;
				}
			}
		}
	}
	
	if (open_buy_lots > 0)
		OpenOrder(sym, OP_BUY, open_buy_lots);
	else if (open_sell_lots > 0)
		OpenOrder(sym, OP_SELL, open_sell_lots);
	
}

void Martingale::SetRealSymbolLots(int sym_, double lots) {
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

void Martingale::CloseOrder(int i, double lots) {
	Order& o = orders[i];
	if (o.type == OP_BUY || o.type == OP_SELL) {
		double close = cl_sym.GetBuffer(o.symbol, 0, 0).Get(pos);
		if (o.type == OP_SELL)
			close += 3 * points[o.symbol]; // spread
		double profit = (close / o.open - 1.0) * 100000 * lots;
		if (o.type == OP_SELL)
			profit *= -1;
		balance += profit;
		o.lots -= lots;
		ASSERT(o.lots >= 0);
		if (o.lots <= 0)
			orders.Remove(i);
	}
	else
		orders.Remove(i);
}

void Martingale::OpenOrder(int sym, int type, double lots) {
	Order& o = orders.Add();
	o.symbol = sym;
	o.type = type;
	o.lots = lots;
	ASSERT(lots > 0.0);
	o.open = cl_sym.GetBuffer(o.symbol, 0, 0).Get(pos);
	if (type == OP_BUY)
		o.open += 3 * points[o.symbol]; // spread
}

int Martingale::NetSetting::CountTrades() {
	int count = 0;
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.type == OP_BUY || o.type == OP_SELL)
			count++;
	}
	return count;
}

double Martingale::NetSetting::FindLastBuyPrice() {
	for(int i = orders.GetCount()-1; i >= 0; i--) {
		Order& o = orders[i];
		if (o.type == OP_BUY)
			return o.open;
	}
	return 0.0;
}

double Martingale::NetSetting::FindLastSellPrice() {
	for(int i = orders.GetCount()-1; i >= 0; i--) {
		Order& o = orders[i];
		if (o.type == OP_SELL)
			return o.open;
	}
	return 0.0;
}

int Martingale::NetSetting::OpenPendingOrder(int pType, double pLots, double pPrice, int pSlippage, double ad_24, int ai_32, int ai_36) {
	if (pLots < 0.01 || pLots > 1000)
		return -1;
	Order& o = orders.Add();
	o.ticket = tickets++;
	switch (pType) {
		case OP_BUY:
			o.type = pType;
			o.lots = pLots;
			o.open = Ask();
			o.stop_loss = StopLong(Bid(), ai_32);
			o.take_profit = TakeLong(Ask(), ai_36);
			break;
		
		case OP_SELL:
			o.type = pType;
			o.lots = pLots;
			o.open = Bid();
			o.stop_loss = StopShort(Ask(), ai_32);
			o.take_profit = TakeShort(Bid(), ai_36);
			break;
		
		case OP_BUYLIMIT:
		case OP_BUYSTOP:
			o.type = pType;
			o.lots = pLots;
			o.open = pPrice;
			o.stop_loss = StopLong(ad_24, ai_32);
			o.take_profit = TakeLong(pPrice, ai_36);
			break;
		
		case OP_SELLLIMIT:
		case OP_SELLSTOP:
			o.type = pType;
			o.lots = pLots;
			o.open = pPrice;
			o.stop_loss = StopShort(ad_24, ai_32);
			o.take_profit = TakeShort(pPrice, ai_36);
			break;
	}
	return o.ticket;
}

void Martingale::NetSetting::OrderModify(Order& o, double price, double stoploss, double takeprofit) {
	if (o.type >= OP_BUYLIMIT)
		o.open = price;
	o.stop_loss = stoploss;
	o.take_profit = takeprofit;
}

void Martingale::NetSetting::CheckPendingOrders() {
	double ask = Ask();
	double bid = Bid();
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.type >= OP_BUYLIMIT) {
			
			switch (o.type) {
				case OP_BUYLIMIT:
					if (o.open >= ask)
						o.type = OP_BUY;
					break;
				
				case OP_BUYSTOP:
					if (o.open <= ask)
						o.type = OP_BUY;
					break;
				
				case OP_SELLLIMIT:
					if (o.open <= bid)
						o.type = OP_SELL;
					break;
				
				case OP_SELLSTOP:
					if (o.open >= bid)
						o.type = OP_SELL;
					break;
			}
		}
	}
	
}

void Martingale::NetSetting::CheckStops() {
	double ask = Ask();
	double bid = Bid();
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.type == OP_BUY) {
			if (bid >= o.take_profit || bid <= o.stop_loss) {
				double profit = (bid / o.open - 1.0) * 100000 * o.lots;
				balance += profit;
				orders.Remove(i);
				i--;
			}
		}
		else if (o.type == OP_SELL) {
			if (ask <= o.take_profit || ask >= o.stop_loss) {
				double profit = (ask / o.open - 1.0) * 100000 * o.lots;
				profit *= -1;
				balance += profit;
				orders.Remove(i);
				i--;
			}
		}
	}
}

void Martingale::NetSetting::RefreshEquity() {
	double ask = Ask();
	double bid = Bid();
	equity = balance;
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.type == OP_BUY) {
			double profit = (bid / o.open - 1.0) * 100000 * o.lots;
			equity += profit;
		}
		else if (o.type == OP_SELL) {
			double profit = (ask / o.open - 1.0) * 100000 * o.lots;
			profit *= -1;
			equity += profit;
		}
	}
}

void Martingale::NetSetting::ProcessNet() {
	double prev_close;
	double cur_close;
	bool long_trade;
	bool short_trade;
	double last_buy_price;
	double last_sell_price;
	int trade_count;
	double price_target;
	double buy_target;
	double sell_target;
	double stopper;
	
	if (use_trailing_stop)
		TrailingAlls();
	
	int total = CountTrades();
	bool flag = false;
	bool trade_now = false;
	bool new_orders_placed = false;
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		
		if (o.type == OP_BUY) {
			long_trade = true;
			short_trade = false;
			break;
		}
		
		else if (o.type == OP_SELL) {
			long_trade = false;
			short_trade = true;
			break;
		}
	}
	
	if (total > 0 && total <= max_trades) {
		last_buy_price = FindLastBuyPrice();
		last_sell_price = FindLastSellPrice();
		
		if (long_trade && last_buy_price - Ask() >= pip_step * Point())
			trade_now = true;
			
		if (short_trade && Bid() - last_sell_price >= pip_step * Point())
			trade_now = true;
	}
	
	if (total < 1) {
		short_trade = false;
		long_trade = false;
		trade_now = true;
	}
	
	if (trade_now) {
		last_buy_price = FindLastBuyPrice();
		last_sell_price = FindLastSellPrice();
		
		if (short_trade) {
			trade_count = total;
			double iLots = NormalizeDouble(lots * pow(lot_exponent, trade_count), 2);
			int ticket = OpenPendingOrder(1, iLots, Bid(), slippage, Ask(), 0, 0);
			
			if (ticket < 0) {
				return;
			}
			
			last_sell_price = FindLastSellPrice();
			
			trade_now = false;
			new_orders_placed = true;
		}
		
		else if (long_trade) {
			trade_count = total;
			double iLots = NormalizeDouble(lots * pow(lot_exponent, trade_count), 2);
			int ticket = OpenPendingOrder(0, iLots, Ask(), slippage, Bid(), 0, 0);
			
			if (ticket < 0) {
				return;
			}
			
			last_buy_price = FindLastBuyPrice();
			
			trade_now = false;
			new_orders_placed = true;
		}
	}
	
	if (trade_now && total < 1) {
		prev_close = Open(1);
		cur_close = Open(0);
		double sell_limit = Bid();
		double buy_limit = Ask();
		
		if (!short_trade && !long_trade) {
			trade_count = total;
			double iLots = NormalizeDouble(lots * pow(lot_exponent, trade_count), 2);
			
			int ticket = 0;
			
			if (prev_close > cur_close) {
				if (RSI() > 30.0) {
					ticket = OpenPendingOrder(1, iLots, sell_limit, slippage, sell_limit, 0, 0);
					
					if (ticket < 0) {
						return;
					}
					
					last_buy_price = FindLastBuyPrice();
					
					new_orders_placed = true;
				}
			}
			
			else {
				if (RSI() < 70.0) {
					ticket = OpenPendingOrder(0, iLots, buy_limit, slippage, buy_limit, 0, 0);
					
					if (ticket < 0) {
						return;
					}
					
					last_sell_price = FindLastSellPrice();
					
					new_orders_placed = true;
				}
			}
			
			trade_now = false;
		}
	}
	
	total = CountTrades();
	
	average_price = 0;
	double count = 0;
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.type == OP_BUY || o.type == OP_SELL) {
			average_price += o.open * o.lots;
			count += o.lots;
		}
	}
	
	if (total > 0)
		average_price /= count;
	
	
	if (new_orders_placed) {
		for(int i = 0; i < orders.GetCount(); i++) {
			Order& o = orders[i];
			
			if (o.type == OP_BUY) {
				price_target = average_price + take_profit * Point();
				buy_target = price_target;
				stopper = average_price - stop_loss * Point();
				flag = true;
			}
			
			else if (o.type == OP_SELL) {
				price_target = average_price - take_profit * Point();
				sell_target = price_target;
				stopper = average_price + stop_loss * Point();
				flag = true;
			}
		}
		
		if (flag == true) {
			for(int i = 0; i < orders.GetCount(); i++) {
				Order& o = orders[i];
				
				OrderModify(o, average_price, o.stop_loss, price_target);
					
				new_orders_placed = false;
			}
		}
	}
}

void Martingale::NetSetting::TrailingAlls() {
	if (trail_stop != 0) {
		for(int i = 0; i < orders.GetCount(); i++) {
			Order& o = orders[i];
			
			if (o.type == OP_BUY) {
				int pips = NormalizeDouble((Bid() - average_price) / Point(), 0);
				
				if (pips < trail_start)
					continue;
					
				double stoploss = o.stop_loss;
				
				double price = Bid() - trail_stop * Point();
				
				if (stoploss == 0.0 || (stoploss != 0.0 && price > stoploss))
					OrderModify(o, average_price, price, o.take_profit);
			}
			
			else if (o.type == OP_SELL) {
				int pips = NormalizeDouble((average_price - Ask()) / Point(), 0);
				
				if (pips < trail_start)
					continue;
					
				double stoploss = o.stop_loss;
				
				double price = Ask() + trail_stop * Point();
				
				if (stoploss == 0.0 || (stoploss != 0.0 && price < stoploss))
					OrderModify(o, average_price, price, o.take_profit);
			}
		}
	}
}



MartingaleCtrl::MartingaleCtrl() {
	Add(ctrl.HSizePos().VSizePos(0, 30));
	Add(prog.BottomPos(0, 30).HSizePos());
}

void MartingaleCtrl::Data() {
	ctrl.Refresh();
	
	Martingale& m = GetMartingale();
	prog.Set(m.opt.GetRound(), m.opt.GetMaxRounds());
}


}

#endif
