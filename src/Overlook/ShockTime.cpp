#include "Overlook.h"

namespace Overlook {

ShockTime::ShockTime() {
	
}

void ShockTime::InitEA() {
	
}

void ShockTime::StartEA(int pos) {
	if (pos == 0)
		return;
	
	double lots2, lots3, o2, o1;
	
	if (max_lots == 0.0)
		max_lots = MarketInfo(Symbol(), MODE_MAXLOT);
		
	double min_lots = MarketInfo(Symbol(), MODE_MINLOT);
	
	if (LotConst_or_not)
		lots = Lot * 0.1;
	else
		lots = AccountBalance() * RiskPercent / 100.0 / 10000.0;
		
	if (lots < min_lots)
		Print("Trying to open  " + DblStr(lots) + ", but it's more than min lots " + DblStr(min_lots));
		
	if (lots > max_lots && max_lots > 0.0)
		Print("Whoa, trying to open  " + DblStr(lots) + ", but max lots is " + DblStr(max_lots));
		
	lots_mul = LotMultiplikator * 0.001;
	tp_factor = TakeProfit;
	pip_step = Step;
	magic = Magic;
	String use_pending_orders = "false";
	String use_orderclose = "false";
	
	if (no_pending_orders == false || (no_pending_orders && (end_hour > begin_hour && (Hour() >= begin_hour && Hour() <= end_hour)) || (begin_hour > end_hour && !(Hour() >= end_hour && Hour() <= begin_hour))))
		use_pending_orders = "true";
		
	if (no_pending_orders && (end_hour > begin_hour && !(Hour() >= begin_hour && Hour() <= end_hour)) || (begin_hour > end_hour && (Hour() >= end_hour && Hour() <= begin_hour)))
		use_orderclose = "true";
		
	if (use_trailing)
		TrailingAlls(pips_limit0, sl_factor0, price1);
		
	
	
	double profit = CalculateProfit();
	
	if (UseEquityStop) {
		if (profit < 0.0 && MathAbs(profit) > TotalEquityRisk / 100.0 * AccountEquityHigh()) {
			CloseThisSymbolAll();
			Print("Closed All due to Stop Out");
			orders_open = false;
		}
	}
	
	order_count = CountTrades();
	
	if (order_count == 0)
		modify_order = false;
		
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			if (OrderType() == OP_BUY) {
				check_orders1 = true;
				check_orders2 = false;
				lots2 = OrderLots();
				break;
			}
		}
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			if (OrderType() == OP_SELL) {
				check_orders1 = false;
				check_orders2 = true;
				lots3 = OrderLots();
				break;
			}
		}
	}
	
	if (order_count > 0 && order_count <= MaxTrades) {
		RefreshRates();
		last_buy_price = FindLastBuyPrice();
		last_sell_price = FindLastSellPrice();
		
		if (check_orders1 && last_buy_price - Ask >= pip_step * Point)
			check_orders0 = true;
			
		if (check_orders2 && Bid - last_sell_price >= pip_step * Point)
			check_orders0 = true;
	}
	
	if (order_count < 1) {
		check_orders2 = false;
		check_orders1 = false;
		check_orders0 = true;
	}
	
	if (check_orders0) {
		last_buy_price = FindLastBuyPrice();
		last_sell_price = FindLastSellPrice();
		
		if (check_orders2) {
			if (use_orderclose == "true") {
				ForceOrderCloseMarket(0, 1);
				lots1 = NormalizeDouble(lots_mul * lots3, norm_digits);
			}
			
			else
				lots1 = GetLots(OP_SELL);
				
			if (use_pending_orders == "true") {
				order_count_lots = order_count;
				
				if (lots1 > 0.0) {
					RefreshRates();
					ret = OpenPendingOrder(1, lots1, Bid, slippage, Ask, 0, 0, Symbol() + "-ShockTime-" + order_count_lots, magic, 0);
					
					if (ret < 0) {
						Print("Error: " + GetLastError());
						throw ConfExc();
					}
					
					last_sell_price = FindLastSellPrice();
					
					check_orders0 = false;
					orders_open = true;
				}
			}
		}
		
		else {
			if (check_orders1) {
				if (use_orderclose == "true") {
					ForceOrderCloseMarket(1, 0);
					lots1 = NormalizeDouble(lots_mul * lots2, norm_digits);
				}
				
				else
					lots1 = GetLots(OP_BUY);
					
				if (use_pending_orders == "true") {
					order_count_lots = order_count;
					
					if (lots1 > 0.0) {
						ret = OpenPendingOrder(0, lots1, Ask, slippage, Bid, 0, 0, Symbol() + "-ShockTime-" + order_count_lots, magic, 0);
						
						if (ret < 0) {
							Print("Error: " + GetLastError());
							throw ConfExc();
						}
						
						last_buy_price = FindLastBuyPrice();
						
						check_orders0 = false;
						orders_open = true;
					}
				}
			}
		}
	}
	
	if (check_orders0 && order_count < 1) {
		ConstBuffer& open_buf = GetInputBuffer(0, 0);
		o2 = open_buf.Get(pos-1);
		o1 = open_buf.Get(pos);
		bid_price0 = Bid;
		ask_price0 = Ask;
		
		if (!check_orders2 && !check_orders1 && use_pending_orders == "true") {
			order_count_lots = order_count;
			
			if (o2 > o1) {
				lots1 = GetLots(OP_SELL);
				
				if (lots1 > 0.0) {
					ret = OpenPendingOrder(1, lots1, bid_price0, slippage, bid_price0, 0, 0, Symbol() + "-ShockTime-" + order_count_lots, magic, 0);
					
					if (ret < 0) {
						Print(DblStr(lots1) + " Error: " + GetLastError());
						throw ConfExc();
					}
					
					last_buy_price = FindLastBuyPrice();
					
					orders_open = true;
				}
			}
			
			else {
				lots1 = GetLots(OP_BUY);
				
				if (lots1 > 0.0) {
					ret = OpenPendingOrder(0, lots1, ask_price0, slippage, ask_price0, 0, 0, Symbol() + "-ShockTime-" + order_count_lots, magic, 0);
					
					if (ret < 0) {
						Print(DblStr(lots1) + " Error: " + GetLastError());
						throw ConfExc();
					}
					
					last_sell_price = FindLastSellPrice();
					
					orders_open = true;
				}
			}
		}
		
		check_orders0 = false;
	}
	
	order_count = CountTrades();
	
	price1 = 0;
	double ld_108 = 0;
	
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		if (!OrderSelect(i, SELECT_BY_POS, MODE_TRADES))
			continue;
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
				price1 += OrderOpenPrice() * OrderLots();
				ld_108 += OrderLots();
			}
		}
	}
	
	if (order_count > 0)
		price1 = NormalizeDouble(price1 / ld_108, Digits);
		
	if (orders_open) {
		for (int i = OrdersTotal() - 1; i >= 0; i--) {
			if (!OrderSelect(i, SELECT_BY_POS, MODE_TRADES))
				continue;
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_BUY) {
					price0 = price1 + tp_factor * Point;
					price2 = price1 - pips_factor * Point;
					modify_order = true;
				}
			}
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_SELL) {
					price0 = price1 - tp_factor * Point;
					price2 = price1 + pips_factor * Point;
					modify_order = true;
				}
			}
		}
	}
	
	if (orders_open) {
		if (modify_order == true) {
			for (int i = OrdersTotal() - 1; i >= 0; i--) {
				OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
				
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
					continue;
					
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
					OrderModify(OrderTicket(), price1, OrderStopLoss(), price0, Time(3000,1,1));
					
				orders_open = false;
			}
		}
	}
	
}

double ShockTime::ND(double d) {
	return (NormalizeDouble(d, Digits));
}

int ShockTime::ForceOrderCloseMarket(bool close_buy, bool close_sell) {
	int li_ret_8 = 0;
	
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_BUY && close_buy) {
					RefreshRates();
					
					if (!OrderClose(OrderTicket(), OrderLots(), ND(Bid), 5)) {
						Print("Error close BUY " + OrderTicket());
						li_ret_8 = -1;
					}
				}
				
				if (OrderType() == OP_SELL && close_sell) {
					RefreshRates();
					
					if (!OrderClose(OrderTicket(), OrderLots(), ND(Ask), 5)) {
						Print("Error close SELL " + OrderTicket());
						li_ret_8 = -1;
					}
				}
			}
		}
	}
	
	return (li_ret_8);
}

double ShockTime::GetLots(int cmd) {
	double ret;
	Time max_order_close_time;
	
	switch (MMType) {
	
	case 0:
		ret = lots;
		break;
		
	case 1:
		ret = NormalizeDouble(lots * MathPow(lots_mul, order_count_lots), norm_digits);
		break;
		
	case 2:
		max_order_close_time = Time(1970,1,1);
		ret = lots;
		
		for (int l_pos_20 = OrdersHistoryTotal() - 1; l_pos_20 >= 0; l_pos_20--) {
			if (OrderSelect(l_pos_20, SELECT_BY_POS, MODE_HISTORY)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
					if (max_order_close_time < OrderCloseTime()) {
						max_order_close_time = OrderCloseTime();
						
						if (OrderProfit() < 0.0)
							ret = NormalizeDouble(OrderLots() * lots_mul, norm_digits);
						else
							ret = lots;
					}
				}
			}
			
			else
				return (-3);
		}
	}
	
	if (AccountFreeMarginCheck(Symbol(), cmd, ret) <= 0.0)
		return (-1);
		
	return (ret);
}

int ShockTime::CountTrades() {
	int l_count_0 = 0;
	
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
			if (OrderType() == OP_SELL || OrderType() == OP_BUY)
				l_count_0++;
	}
	
	return (l_count_0);
}

void ShockTime::CloseThisSymbolAll() {
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol()) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				int type = OrderType();
				
				if (type == OP_BUY)
					OrderClose(OrderTicket(), OrderLots(), Bid, slippage);
					
				if (type == OP_SELL)
					OrderClose(OrderTicket(), OrderLots(), Ask, slippage);
			}
			
			Sleep(1000);
		}
	}
}

int ShockTime::OpenPendingOrder(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_unused_32, int ai_36, String a_comment_40, int a_magic_48, int a_datetime_52) {
	int ticket = 0;
	int i = 0;
	int count = 100;
	
	switch (ai_0) {
	
	case 2:
	
		for (i = 0; i < count; i++) {
			ticket = OrderSend(Symbol(), OP_BUYLIMIT, a_lots_4, a_price_12, a_slippage_20, StopLong(ad_24, pips_factor), TakeLong(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			if (ticket >= 0)
				break;
		}
		
		break;
		
	case 4:
	
		for (i = 0; i < count; i++) {
			ticket = OrderSend(Symbol(), OP_BUYSTOP, a_lots_4, a_price_12, a_slippage_20, StopLong(ad_24, pips_factor), TakeLong(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			if (ticket >= 0)
				break;
		}
		
		break;
		
	case 0:
	
		for (i = 0; i < count; i++) {
			RefreshRates();
			ticket = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, StopLong(Bid, pips_factor), TakeLong(Ask, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			if (ticket >= 0)
				break;
		}
		
		break;
		
	case 3:
	
		for (i = 0; i < count; i++) {
			ticket = OrderSend(Symbol(), OP_SELLLIMIT, a_lots_4, a_price_12, a_slippage_20, StopShort(ad_24, pips_factor), TakeShort(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			if (ticket >= 0)
				break;
		}
		
		break;
		
	case 5:
	
		for (i = 0; i < count; i++) {
			ticket = OrderSend(Symbol(), OP_SELLSTOP, a_lots_4, a_price_12, a_slippage_20, StopShort(ad_24, pips_factor), TakeShort(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			if (ticket >= 0)
				break;
		}
		
		break;
		
	case 1:
	
		for (i = 0; i < count; i++) {
			ticket = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, StopShort(Ask, pips_factor), TakeShort(Bid, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			if (ticket >= 0)
				break;
		}
	}
	
	return (ticket);
}

double ShockTime::StopLong(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return 0;
		
	return (ad_0 - ai_8 * Point);
}

double ShockTime::StopShort(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return 0;
		
	return (ad_0 + ai_8 * Point);
}

double ShockTime::TakeLong(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return 0;
		
	return (ad_0 + ai_8 * Point);
}

double ShockTime::TakeShort(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return 0;
		
	return (ad_0 - ai_8 * Point);
}

double ShockTime::CalculateProfit() {
	double ld_ret_0 = 0;
	
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
			if (OrderType() == OP_BUY || OrderType() == OP_SELL)
				ld_ret_0 += OrderProfit();
	}
	
	return ld_ret_0;
}

void ShockTime::TrailingAlls(int ai_0, int ai_4, double a_price_8) {
	int li_16;
	double l_ord_stoploss_20;
	double l_price_28;
	
	if (ai_4 != 0) {
		for (int l_pos_36 = OrdersTotal() - 1; l_pos_36 >= 0; l_pos_36--) {
			if (OrderSelect(l_pos_36, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
					continue;
					
				if (OrderSymbol() == Symbol() || OrderMagicNumber() == magic) {
					if (OrderType() == OP_BUY) {
						li_16 = NormalizeDouble((Bid - a_price_8) / Point, 0);
						
						if (li_16 < ai_0)
							continue;
							
						l_ord_stoploss_20 = OrderStopLoss();
						
						l_price_28 = Bid - ai_4 * Point;
						
						if (l_ord_stoploss_20 == 0.0 || (l_ord_stoploss_20 != 0.0 && l_price_28 > l_ord_stoploss_20))
							OrderModify(OrderTicket(), a_price_8, l_price_28, OrderTakeProfit(), Time(3000,1,1));
					}
					
					if (OrderType() == OP_SELL) {
						li_16 = NormalizeDouble((a_price_8 - Ask) / Point, 0);
						
						if (li_16 < ai_0)
							continue;
							
						l_ord_stoploss_20 = OrderStopLoss();
						
						l_price_28 = Ask + ai_4 * Point;
						
						if (l_ord_stoploss_20 == 0.0 || (l_ord_stoploss_20 != 0.0 && l_price_28 < l_ord_stoploss_20))
							OrderModify(OrderTicket(), a_price_8, l_price_28, OrderTakeProfit(), Time(3000,1,1));
					}
				}
				
				Sleep(1000);
			}
		}
	}
}

double ShockTime::AccountEquityHigh() {
	if (CountTrades() == 0)
		equity1 = AccountEquity();
		
	if (equity1 < equity2)
		equity1 = equity2;
	else
		equity1 = AccountEquity();
		
	equity2 = AccountEquity();
	
	return (equity1);
}

double ShockTime::FindLastBuyPrice() {
	double l_ord_open_price_0;
	int l_ticket_8;
	double ld_unused_12 = 0;
	int l_ticket_20 = 0;
	
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY) {
			l_ticket_8 = OrderTicket();
			
			if (l_ticket_8 > l_ticket_20) {
				l_ord_open_price_0 = OrderOpenPrice();
				ld_unused_12 = l_ord_open_price_0;
				l_ticket_20 = l_ticket_8;
			}
		}
	}
	
	return (l_ord_open_price_0);
}

double ShockTime::FindLastSellPrice() {
	double l_ord_open_price_0;
	int l_ticket_8;
	double ld_unused_12 = 0;
	int l_ticket_20 = 0;
	
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL) {
			l_ticket_8 = OrderTicket();
			
			if (l_ticket_8 > l_ticket_20) {
				l_ord_open_price_0 = OrderOpenPrice();
				ld_unused_12 = l_ord_open_price_0;
				l_ticket_20 = l_ticket_8;
			}
		}
	}
	
	return (l_ord_open_price_0);
}


double ShockTime::GetProfitForDay(int ai_0) {
	double profit = 0;
	
	Time day_begin = TimeCurrent();
	day_begin.hour = 0;
	day_begin.minute = 0;
	day_begin.second = 0;
	
	for (int i = 0; i < OrdersHistoryTotal(); i++) {
		if (!(OrderSelect(i, SELECT_BY_POS, MODE_HISTORY)))
			break;
						
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic)
			if (OrderCloseTime() >= day_begin && OrderCloseTime() < day_begin + 86400)
				profit += OrderProfit() + OrderCommission() + OrderSwap();
	}
	
	return profit;
}

}
