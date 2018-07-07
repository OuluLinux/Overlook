#include "Overlook.h"


namespace Overlook {

Salmon::Salmon() {
	
}

void Salmon::InitEA() {
	gs_eurusd_116 = Symbol();
	
	if (StringFind(gs_eurusd_116, "JPY") < 0) {
		g_point_256 = MarketInfo(gs_eurusd_116, MODE_POINT);
		
		if (g_point_256 - 0.0001 < 0.0) {
			point_mult = 10;
			point_mode = 5;
		}
		
		else {
			point_mult = 1;
			point_mode = 4;
		}
	}
	
	else {
		g_point_256 = MarketInfo(gs_eurusd_116, MODE_POINT);
		
		if (g_point_256 - 0.01 < 0.0) {
			point_mult = 10;
			point_mode = 3;
		}
		
		else {
			point_mult = 1;
			point_mode = 2;
		}
	}
	
	if (ny_hour2 + NewYorkShift > 23)
		ny_hour4 = ny_hour2 + (-24) + NewYorkShift;
	else
		ny_hour4 = ny_hour2 + NewYorkShift;
		
	if (ny_hour + NewYorkShift > 23)
		ny_hour3 = ny_hour + (-24) + NewYorkShift;
	else
		ny_hour3 = ny_hour + NewYorkShift;
	
}

void Salmon::StartEA(int pos) {
	double l_iopen_0;
	g_datetime_232 = TimeCurrent();
	
	if (TimeHour(g_datetime_232) == ny_hour4 && TimeMinute(g_datetime_232) < 45 && g_datetime_272 != Now) {
		g_datetime_272 = Now;
		time_bool1 = true;
		time_bool0 = false;
		
	}
	
	if (g_datetime_276 != Now) {
		g_datetime_276 = Now;
		
		if (time_bool1 == true) {
			vDeleteOrders(gs_eurusd_116, -1, -1);
			time_bool1 = false;
		}
	}
	
	if (bExistPositions(gs_eurusd_116, OP_BUY, Magic + 1) == 1) {
		order_type1 = true;
		order_type2 = false;
	}
	
	if (bExistPositions(gs_eurusd_116, OP_SELL, Magic - 1) == 1) {
		order_type1 = false;
		order_type2 = true;
	}
	
	if (1.0001 * Lot1Size - dGetLotLastClosePos(gs_eurusd_116, -1, -1) > 0.0 && bIsCloseLastPosByStop(gs_eurusd_116, -1, -1) == 1 && bExistPositions(gs_eurusd_116, OP_BUY, Magic +
			2) == 0 && bExistPositions(gs_eurusd_116, OP_SELL, Magic - 2) == 0 && order_fail_value == true && order_type1 == true) {
		use_orders_lotlimit = false;
		use_orders_lotlimit2 = true;
		order_type1 = false;
		RefreshRates();
		g_bid_240 = MarketInfo(gs_eurusd_116, MODE_BID);
		g_ask_248 = MarketInfo(gs_eurusd_116, MODE_ASK);
		g_point_256 = MarketInfo(gs_eurusd_116, MODE_POINT);
		
		if (IsTesting() == true) {
			if (use_orders_lotlimit2 == true) {
				vOpenPosition(gs_eurusd_116, OP_SELL, NormalizeDouble(lots0 * Lot1Size, 2), Slippage * point_mult, NormalizeDouble(g_ask_248 + g_point_256 * SL2SELL * point_mult, point_mode), NormalizeDouble(g_bid_240 - 15.0 * g_point_256 * point_mult, point_mode), Magic - 2);
				order_fail_value = false;
				use_orders_lotlimit = false;
				use_orders_lotlimit2 = false;
				gi_unused_344 = false;
				gi_unused_348 = false;
			}
		}
		
		if (IsTesting() == false) {
			if (use_orders_lotlimit2 == true) {
				iOpenPosition(gs_eurusd_116, OP_SELL, NormalizeDouble(lots0 * Lot1Size, 2), open_orders_arg0, Slippage * point_mult, order_tries, alert_fails, NormalizeDouble(g_ask_248 +
							  g_point_256 * SL2SELL * point_mult, point_mode), NormalizeDouble(g_bid_240 - 15.0 * g_point_256 * point_mult, point_mode), Magic - 2);
				order_fail_value = false;
				use_orders_lotlimit = false;
				use_orders_lotlimit2 = false;
				gi_unused_344 = false;
				gi_unused_348 = false;
			}
		}
	}
	
	if (1.0001 * Lot1Size - dGetLotLastClosePos(gs_eurusd_116, -1, -1) > 0.0 && bIsCloseLastPosByStop(gs_eurusd_116, -1, -1) == 1 && bExistPositions(gs_eurusd_116, OP_BUY, Magic +
			2) == 0 && bExistPositions(gs_eurusd_116, OP_SELL, Magic - 2) == 0 && order_fail_value == true && order_type2 == true) {
		use_orders_lotlimit = true;
		use_orders_lotlimit2 = false;
		order_type2 = false;
		RefreshRates();
		g_bid_240 = MarketInfo(gs_eurusd_116, MODE_BID);
		g_ask_248 = MarketInfo(gs_eurusd_116, MODE_ASK);
		g_point_256 = MarketInfo(gs_eurusd_116, MODE_POINT);
		
		if (IsTesting() == true) {
			if (use_orders_lotlimit == true) {
				vOpenPosition(gs_eurusd_116, OP_BUY, NormalizeDouble(lots0 * Lot1Size, 2), Slippage * point_mult, NormalizeDouble(g_bid_240 - g_point_256 * SL2BUY * point_mult, point_mode), NormalizeDouble(g_ask_248 +
							  15.0 * g_point_256 * point_mult, point_mode), Magic + 2);
				order_fail_value = false;
				use_orders_lotlimit = false;
				use_orders_lotlimit2 = false;
				gi_unused_344 = false;
				gi_unused_348 = false;
			}
		}
		
		if (IsTesting() == false) {
			if (use_orders_lotlimit == true) {
				iOpenPosition(gs_eurusd_116, OP_BUY, NormalizeDouble(lots0 * Lot1Size, 2), open_orders_arg0, Slippage * point_mult, order_tries, alert_fails, NormalizeDouble(g_bid_240 - g_point_256 * SL2BUY * point_mult, point_mode), NormalizeDouble(g_ask_248 +
							  15.0 * g_point_256 * point_mult, point_mode), Magic + 2);
				order_fail_value = false;
				use_orders_lotlimit = false;
				use_orders_lotlimit2 = false;
				gi_unused_344 = false;
				gi_unused_348 = false;
			}
		}
	}
	
	g_datetime_232 = TimeCurrent();
	
	if (TimeHour(g_datetime_232) == ny_hour3 && TimeMinute(g_datetime_232) >= 0 && g_datetime_268 != Now && time_bool0 == false && some_fail == false &&
		use_order_limit0 == true && TimeDayOfWeek(g_datetime_232) != 5) {
		ConstBuffer& open_buf = GetInputBuffer(0, 0);
		g_datetime_268 = Now;
		l_iopen_0 = open_buf.Get(pos);
		RefreshRates();
		g_bid_240 = MarketInfo(gs_eurusd_116, MODE_BID);
		g_ask_248 = MarketInfo(gs_eurusd_116, MODE_ASK);
		g_point_256 = MarketInfo(gs_eurusd_116, MODE_POINT);
		
		if (IsTesting() == true) {
			if (use_orders == true) {
				vSetOrderForTest(gs_eurusd_116, OP_BUYLIMIT, Lot1Size, NormalizeDouble(l_iopen_0 - tp_factor1 * g_point_256 * point_mult, point_mode), NormalizeDouble(l_iopen_0 - sl_factor1 * g_point_256 * point_mult, point_mode), NormalizeDouble(l_iopen_0, point_mode), Magic + 1);
				vSetOrderForTest(gs_eurusd_116, OP_SELLLIMIT, Lot1Size, NormalizeDouble(l_iopen_0 + tp_factor2 * g_point_256 * point_mult, point_mode), NormalizeDouble(l_iopen_0 + sl_factor2 * g_point_256 * point_mult, point_mode), NormalizeDouble(l_iopen_0, point_mode), Magic - 1);
				order_fail_value = true;
			}
		}
		
		if (IsTesting() == false) {
			if (use_orders == true) {
				vSetOrderForReal(gs_eurusd_116, OP_BUYLIMIT, Lot1Size, NormalizeDouble(l_iopen_0 - tp_factor1 * g_point_256 * point_mult, point_mode), NormalizeDouble(l_iopen_0 - sl_factor1 * g_point_256 * point_mult, point_mode), NormalizeDouble(l_iopen_0, point_mode), Magic + 1);
				vSetOrderForReal(gs_eurusd_116, OP_SELLLIMIT, Lot1Size, NormalizeDouble(l_iopen_0 + tp_factor2 * g_point_256 * point_mult, point_mode), NormalizeDouble(l_iopen_0 + sl_factor2 * g_point_256 * point_mult, point_mode), NormalizeDouble(l_iopen_0, point_mode), Magic - 1);
				order_fail_value = true;
			}
		}
		
		time_bool0 = true;
	}
	
	if (TimeHour(g_datetime_232) == ny_hour2 && some_fail == false && use_order_limit0 == false)
		gi_unused_332 = true;
	
}

void Salmon::vSetOrderForReal(String a_symbol_0, int a_cmd_8, double a_lots_12, double a_price_20, double a_price_28, double a_price_36, int a_magic_44) {
	Time l_datetime_64;
	double l_ask_68;
	double l_bid_76;
	double l_point_84;
	int l_ticket_100;
	
	if (a_symbol_0 == "" || a_symbol_0 == "0")
		a_symbol_0 = Symbol();
		
	int l_stoplevel_104 = MarketInfo(a_symbol_0, MODE_STOPLEVEL);
	
	for (int li_96 = 1; li_96 <= order_tries; li_96++) {
		
		l_datetime_64 = TimeCurrent();
		
		l_ticket_100 = OrderSend(a_symbol_0, a_cmd_8, a_lots_12, a_price_20, Slippage, a_price_28, a_price_36, "FishForexRobot", a_magic_44);
		
		if (l_ticket_100 > 0) {
			if (alert_fails != true)
				break;
				
			return;
		}
		
		
		if (l_ticket_100 == -1) {
			Sleep(66000);
			
			if (bExistPositions(a_symbol_0, a_cmd_8, a_magic_44, l_datetime_64)) {
				if (alert_fails != true)
					break;
					
				return;
			}
			
			Print("Error(" + GetLastError() + ")");
		}
		
	}
}

void Salmon::vSetOrderForTest(String a_symbol_0, int a_cmd_8, double a_lots_12, double a_price_20, double a_price_28, double a_price_36, int a_magic_44) {
	
	if (a_symbol_0 == "" || a_symbol_0 == "0")
		a_symbol_0 = Symbol();
		
	int ticket = OrderSend(a_symbol_0, a_cmd_8, a_lots_12, a_price_20, Slippage, a_price_28, a_price_36, "FishForexRobot", a_magic_44);
	
	if (ticket < 0) {
		Print("Error(" + GetLastError() + ")");
	}
}

int Salmon::iOpenPosition(String a_symbol_0, int a_cmd_8, double a_lots_12, int ai_unused_20, int ai_24, int ai_28, int a_slippage_32, int ai_36, bool ai_40, double a_price_52, double a_price_60, int a_magic_68) {
	Time l_datetime_76;
	double l_price_80;
	double l_ask_88;
	double l_bid_96;
	int l_digits_104;
	int l_error_108;
	int l_ticket_116 = 0;
	
	if (a_symbol_0 == "" || a_symbol_0 == "0")
		a_symbol_0 = Symbol();
	
	for (int i = 1; i <= ai_36; i++) {
		
		l_digits_104 = MarketInfo(a_symbol_0, MODE_DIGITS);
		
		l_ask_88 = MarketInfo(a_symbol_0, MODE_ASK);
		
		l_bid_96 = MarketInfo(a_symbol_0, MODE_BID);
		
		if (a_cmd_8 == OP_BUY)
			l_price_80 = l_ask_88;
		else
			l_price_80 = l_bid_96;
			
		l_price_80 = NormalizeDouble(l_price_80, l_digits_104);
		
		l_datetime_76 = TimeCurrent();
		
		l_ticket_116 = OrderSend(a_symbol_0, a_cmd_8, a_lots_12, l_price_80, a_slippage_32, a_price_52, a_price_60, "", a_magic_68, 0);
		
		if (l_ticket_116 >= 0) {
			break;
		}
		
		Print("Error(" + GetLastError() + ")");
	}
	
	return (l_ticket_116);
}

void Salmon::vOpenPosition(String a_symbol_0, int a_cmd_8, double a_lots_12, int a_slippage_28, double a_price_32, double a_price_40, int a_magic_48) {
	double l_price_56;
	int l_error_64;
	
	if (a_symbol_0 == "")
		a_symbol_0 = Symbol();
		
	if (a_cmd_8 == OP_BUY) {
		l_price_56 = MarketInfo(a_symbol_0, MODE_ASK);
	}
	else {
		l_price_56 = MarketInfo(a_symbol_0, MODE_BID);
	}
	
	int l_ticket_68 = OrderSend(a_symbol_0, a_cmd_8, a_lots_12, l_price_56, a_slippage_28, a_price_32, a_price_40, "FishForexRobot", a_magic_48);
	
	if (l_ticket_68 < 0) {
		Print("Error(" + GetLastError() + ")");
	}
}

bool Salmon::bExistPositions(String as_0, int a_cmd_8, int a_magic_12, Time ai_16) {
	int l_ord_total_24 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_20 = 0; l_pos_20 < l_ord_total_24; l_pos_20++) {
		if (OrderSelect(l_pos_20, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "") {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					if (a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
						if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
							if (ai_16 <= OrderOpenTime())
								return (true);
					}
				}
			}
		}
	}
	
	return (false);
}

String Salmon::sGetNameOP(int ai_0) {
	switch (ai_0) {
	
	case 0:
		return ("Buy");
		
	case 1:
		return ("Sell");
		
	case 2:
		return ("Buy Limit");
		
	case 3:
		return ("Sell Limit");
		
	case 4:
		return ("Buy Stop");
		
	case 5:
		return ("Sell Stop");
	}
	
	return ("Unknown Operation");
}

int Salmon::clFuncC(bool ai_0, int ai_4, int ai_8) {
	if (ai_0)
		return (ai_4);
		
	return (ai_8);
}

int Salmon::bIsCloseLastPosByStop(String a_symbol_0, int a_cmd_8, int a_magic_12) {
	Time l_datetime_16(1970,1,1);
	double ld_20;
	double ld_28;
	int l_digits_36;
	int i4 = -1;
	int l_hist_total_48 = OrdersHistoryTotal();
	
	if (a_symbol_0 == "0")
		a_symbol_0 = Symbol();
		
	for (int i0 = 0; i0 < l_hist_total_48; i0++) {
		if (OrderSelect(i0, SELECT_BY_POS, MODE_HISTORY)) {
			if (OrderSymbol() == a_symbol_0 || a_symbol_0 == "") {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					if (a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
						if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
							if (l_datetime_16 < OrderCloseTime()) {
								l_datetime_16 = OrderCloseTime();
								i4 = i0;
							}
						}
					}
				}
			}
		}
	}
	
	if (OrderSelect(i4, SELECT_BY_POS, MODE_HISTORY)) {
		l_digits_36 = MarketInfo(a_symbol_0, MODE_DIGITS);
		
		if (l_digits_36 == 0) {
			if (StringFind(OrderSymbol(), "JPY") < 0) {
				if (point_mult == 1)
					l_digits_36 = 4;
					
				if (point_mult == 10)
					l_digits_36 = 5;
			}
			
			else {
				if (point_mult == 1)
					l_digits_36 = 2;
					
				if (point_mult == 10)
					l_digits_36 = 3;
			}
		}
		
		ld_20 = NormalizeDouble(OrderClosePrice(), l_digits_36);
		
		ld_28 = NormalizeDouble(OrderStopLoss(), l_digits_36);
		
		if (ld_20 == ld_28)
			return (1);
	}
	
	return 0;
}

void Salmon::vDeleteOrders(String as_0, int a_cmd_8, int a_magic_12) {
	bool l_ord_delete_16;
	int l_error_20;
	int l_cmd_36;
	int l_ord_total_32 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int i = l_ord_total_32 - 1; i >= 0; i--) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			l_cmd_36 = OrderType();
			
			if (l_cmd_36 > OP_SELL && l_cmd_36 < 6) {
				if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || l_cmd_36 == a_cmd_8) {
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
						for (int li_28 = 1; li_28 <= order_tries; li_28++) {
								
							l_ord_delete_16 = OrderDelete(OrderTicket());
							
							if (l_ord_delete_16) {
								break;
							}
							
							Print("Error(" + GetLastError() + ")");
						}
					}
				}
			}
		}
	}
}

double Salmon::dGetLotLastClosePos(String as_0, int a_cmd_8, int a_magic_12) {
	Time l_datetime_16(1970,1,1);
	double l_ord_lots_20 = -1;
	int l_hist_total_32 = OrdersHistoryTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_28 = 0; l_pos_28 < l_hist_total_32; l_pos_28++) {
		if (OrderSelect(l_pos_28, SELECT_BY_POS, MODE_HISTORY)) {
			if (OrderSymbol() == as_0 || as_0 == "") {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					if (a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
						if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
							if (l_datetime_16 < OrderCloseTime()) {
								l_datetime_16 = OrderCloseTime();
								l_ord_lots_20 = OrderLots();
							}
						}
					}
				}
			}
		}
	}
	
	return (l_ord_lots_20);
}

}

