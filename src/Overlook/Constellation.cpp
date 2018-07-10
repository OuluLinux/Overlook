#include "Overlook.h"

#if 0

// turbomax

namespace Overlook {

Constellation::Constellation() {

}

void Constellation::InitEA() {
	gd_412 = MarketInfo(Symbol(), MODE_SPREAD) * Point;
	
	if (IsTesting() == true)
		f0_15();
		
	if (IsTesting() == false)
		f0_15();
	
}

void Constellation::StartEA(int pos) {
	double order_lots_0;
	double order_lots_8;
	double iclose_16;
	double iclose_24;
	
	if (((!IsOptimization()) && !IsTesting() && (!IsVisualMode())) || (ShowTableOnTesting && IsTesting() && (!IsOptimization()))) {
		f0_16();
		f0_17();
	}
	
	gd_136 = LotMultiplikator;
	
	gd_184 = TakeProfit;
	gd_232 = Step;
	gi_316 = Magic;
	string ls_32 = "false";
	string ls_40 = "false";
	
	if (gi_292 == false || (gi_292 && (gi_300 > gi_296 && (Hour() >= gi_296 && Hour() <= gi_300)) || (gi_296 > gi_300 && (!(Hour() >= gi_300 && Hour() <= gi_296)))))
		ls_32 = "true";
		
	if (gi_292 && (gi_300 > gi_296 && (!(Hour() >= gi_296 && Hour() <= gi_300))) || (gi_296 > gi_300 && (Hour() >= gi_300 && Hour() <= gi_296)))
		ls_40 = "true";
		
	if (gi_276)
		f0_11(gd_200, gd_208, g_price_372);
		
	if (gi_280) {
		if (TimeCurrent() >= gi_428) {
			f0_4();
			Print("Closed All due to TimeOut");
		}
	}
	
	if (g_time_424 == Time[0])
		return (0);
		
	g_time_424 = Time[0];
	
	double ld_48 = f0_10();
	
	if (UseEquityStop) {
		if (ld_48 < 0.0 && MathAbs(ld_48) > TotalEquityRisk / 100.0 * f0_12()) {
			f0_4();
			Print("Closed All due to Stop Out");
			gi_476 = false;
		}
	}
	
	gi_448 = f0_3();
	
	if (gi_448 == 0)
		gi_420 = false;
		
	for (g_pos_444 = OrdersTotal() - 1; g_pos_444 >= 0; g_pos_444--) {
		OrderSelect(g_pos_444, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != gi_316)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316) {
			if (OrderType() == OP_BUY) {
				gi_464 = true;
				gi_468 = false;
				order_lots_0 = OrderLots();
				break;
			}
		}
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316) {
			if (OrderType() == OP_SELL) {
				gi_464 = false;
				gi_468 = true;
				order_lots_8 = OrderLots();
				break;
			}
		}
	}
	
	if (gi_448 > 0 && gi_448 <= MaxTrades) {
		RefreshRates();
		gd_396 = f0_13();
		gd_404 = f0_14();
		
		if (gi_464 && gd_396 - Ask >= gd_232 * Point)
			gi_460 = true;
			
		if (gi_468 && Bid - gd_404 >= gd_232 * Point)
			gi_460 = true;
	}
	
	if (gi_448 < 1) {
		gi_468 = false;
		gi_464 = false;
		gi_460 = true;
		gd_348 = AccountEquity();
	}
	
	if (gi_460) {
		gd_396 = f0_13();
		gd_404 = f0_14();
		
		if (gi_468) {
			if (gi_272 || ls_40 == "true") {
				f0_1(0, 1);
				gd_436 = NormalizeDouble(gd_136 * order_lots_8, gd_88);
			}
			
			else
				gd_436 = f0_2(OP_SELL);
				
			if (gi_116 && ls_32 == "true") {
				gi_432 = gi_448;
				
				if (gd_436 > 0.0) {
					RefreshRates();
					gi_472 = f0_5(1, gd_436, Bid, g_slippage_144, Ask, 0, 0, gs_80 + "-" + gi_432, gi_316, 0, HotPink);
					
					if (gi_472 < 0) {
						Print("Error: ", GetLastError());
						return (0);
					}
					
					gd_404 = f0_14();
					
					gi_460 = false;
					gi_476 = true;
				}
			}
		}
		
		else {
			if (gi_464) {
				if (gi_272 || ls_40 == "true") {
					f0_1(1, 0);
					gd_436 = NormalizeDouble(gd_136 * order_lots_0, gd_88);
				}
				
				else
					gd_436 = f0_2(OP_BUY);
					
				if (gi_116 && ls_32 == "true") {
					gi_432 = gi_448;
					
					if (gd_436 > 0.0) {
						gi_472 = f0_5(0, gd_436, Ask, g_slippage_144, Bid, 0, 0, gs_80 + "-" + gi_432, gi_316, 0, Lime);
						
						if (gi_472 < 0) {
							Print("Error: ", GetLastError());
							return (0);
						}
						
						gd_396 = f0_13();
						
						gi_460 = false;
						gi_476 = true;
					}
				}
			}
		}
	}
	
	if (gi_460 && gi_448 < 1) {
		iclose_16 = iClose(Symbol(), 0, 2);
		iclose_24 = iClose(Symbol(), 0, 1);
		g_bid_380 = Bid;
		g_ask_388 = Ask;
		
		if ((!gi_468) && !gi_464 && ls_32 == "true") {
			gi_432 = gi_448;
			
			if (iclose_16 > iclose_24) {
				gd_436 = f0_2(OP_SELL);
				
				if (gd_436 > 0.0) {
					gi_472 = f0_5(1, gd_436, g_bid_380, g_slippage_144, g_bid_380, 0, 0, gs_80 + "-" + gi_432, gi_316, 0, HotPink);
					
					if (gi_472 < 0) {
						Print(gd_436, "Error: ", GetLastError());
						return (0);
					}
					
					gd_396 = f0_13();
					
					gi_476 = true;
				}
			}
			
			else {
				gd_436 = f0_2(OP_BUY);
				
				if (gd_436 > 0.0) {
					gi_472 = f0_5(0, gd_436, g_ask_388, g_slippage_144, g_ask_388, 0, 0, gs_80 + "-" + gi_432, gi_316, 0, Lime);
					
					if (gi_472 < 0) {
						Print(gd_436, "Error: ", GetLastError());
						return (0);
					}
					
					gd_404 = f0_14();
					
					gi_476 = true;
				}
			}
		}
		
		if (gi_472 > 0)
			gi_428 = TimeCurrent() + 60.0 * (60.0 * gd_284);
			
		gi_460 = false;
	}
	
	gi_448 = f0_3();
	
	g_price_372 = 0;
	double ld_56 = 0;
	
	for (g_pos_444 = OrdersTotal() - 1; g_pos_444 >= 0; g_pos_444--) {
		OrderSelect(g_pos_444, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != gi_316)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316) {
			if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
				g_price_372 += OrderOpenPrice() * OrderLots();
				ld_56 += OrderLots();
			}
		}
	}
	
	if (gi_448 > 0)
		g_price_372 = NormalizeDouble(g_price_372 / ld_56, Digits);
		
	if (gi_476) {
		for (g_pos_444 = OrdersTotal() - 1; g_pos_444 >= 0; g_pos_444--) {
			OrderSelect(g_pos_444, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != gi_316)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316) {
				if (OrderType() == OP_BUY) {
					g_price_340 = g_price_372 + gd_184 * Point;
					gd_unused_356 = g_price_340;
					gd_452 = g_price_372 - g_pips_192 * Point;
					gi_420 = true;
				}
			}
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316) {
				if (OrderType() == OP_SELL) {
					g_price_340 = g_price_372 - gd_184 * Point;
					gd_unused_364 = g_price_340;
					gd_452 = g_price_372 + g_pips_192 * Point;
					gi_420 = true;
				}
			}
		}
	}
	
	if (gi_476) {
		if (gi_420 == true) {
			for (g_pos_444 = OrdersTotal() - 1; g_pos_444 >= 0; g_pos_444--) {
				OrderSelect(g_pos_444, SELECT_BY_POS, MODE_TRADES);
				
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != gi_316)
					continue;
					
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316)
					OrderModify(OrderTicket(), g_price_372, OrderStopLoss(), g_price_340, 0, Yellow);
					
				gi_476 = false;
			}
		}
	}
	
	return (0);
}

double Constellation::f0_0(double ad_0) {
	return (NormalizeDouble(ad_0, Digits));
}

int Constellation::f0_1(bool ai_0 = true, bool ai_4 = true) {
	int li_ret_8 = 0;
	
	for (int pos_12 = OrdersTotal() - 1; pos_12 >= 0; pos_12--) {
		if (OrderSelect(pos_12, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316) {
				if (OrderType() == OP_BUY && ai_0) {
					RefreshRates();
					
					if (!IsTradeContextBusy()) {
						if (!OrderClose(OrderTicket(), OrderLots(), f0_0(Bid), 5, CLR_NONE)) {
							Print("Error close BUY " + OrderTicket());
							li_ret_8 = -1;
						}
					}
					
					else {
						if (g_datetime_480 != iTime(NULL, 0, 0)) {
							g_datetime_480 = iTime(NULL, 0, 0);
							Print("Need close BUY " + OrderTicket() + ". Trade Context Busy");
						}
						
						return (-2);
					}
				}
				
				if (OrderType() == OP_SELL && ai_4) {
					RefreshRates();
					
					if (!IsTradeContextBusy()) {
						if (!(!OrderClose(OrderTicket(), OrderLots(), f0_0(Ask), 5, CLR_NONE)))
							continue;
							
						Print("Error close SELL " + OrderTicket());
						
						li_ret_8 = -1;
						
						continue;
					}
					
					if (g_datetime_484 != iTime(NULL, 0, 0)) {
						g_datetime_484 = iTime(NULL, 0, 0);
						Print("Need close SELL " + OrderTicket() + ". Trade Context Busy");
					}
					
					return (-2);
				}
			}
		}
	}
	
	return (li_ret_8);
}

double Constellation::f0_2(int a_cmd_0) {
	double lots_4;
	int datetime_12;
	
	switch (MMType) {
	
	case 0:
		lots_4 = Lots;
		break;
		
	case 1:
		lots_4 = NormalizeDouble(Lots * MathPow(gd_136, gi_432), gd_88);
		break;
		
	case 2:
		datetime_12 = 0;
		lots_4 = Lots;
		
		for (int pos_20 = OrdersHistoryTotal() - 1; pos_20 >= 0; pos_20--) {
			if (OrderSelect(pos_20, SELECT_BY_POS, MODE_HISTORY)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316) {
					if (datetime_12 < OrderCloseTime()) {
						datetime_12 = OrderCloseTime();
						
						if (OrderProfit() < 0.0) {
							lots_4 = NormalizeDouble(OrderLots() * gd_136, gd_88);
							continue;
						}
						
						lots_4 = Lots;
					}
				}
			}
			
			else
				return (-3);
		}
	}
	
	if (AccountFreeMarginCheck(Symbol(), a_cmd_0, lots_4) <= 0.0)
		return (-1);
		
	if (GetLastError() == 134/* NOT_ENOUGH_MONEY */)
		return (-2);
		
	return (lots_4);
}

int Constellation::f0_3() {
	int count_0 = 0;
	
	for (int pos_4 = OrdersTotal() - 1; pos_4 >= 0; pos_4--) {
		OrderSelect(pos_4, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != gi_316)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316)
			if (OrderType() == OP_SELL || OrderType() == OP_BUY)
				count_0++;
	}
	
	return (count_0);
}

void Constellation::f0_4() {
	for (int pos_0 = OrdersTotal() - 1; pos_0 >= 0; pos_0--) {
		OrderSelect(pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol()) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316) {
				if (OrderType() == OP_BUY)
					OrderClose(OrderTicket(), OrderLots(), Bid, g_slippage_144, Blue);
					
				if (OrderType() == OP_SELL)
					OrderClose(OrderTicket(), OrderLots(), Ask, g_slippage_144, Red);
			}
			
			Sleep(1000);
		}
	}
}

int Constellation::f0_5(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_unused_32, int ai_36, string a_comment_40, int a_magic_48, int a_datetime_52, color a_color_56) {
	int ticket_60 = 0;
	int error_64 = 0;
	int count_68 = 0;
	int li_72 = 100;
	
	switch (ai_0) {
	
	case 2:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_BUYLIMIT, a_lots_4, a_price_12, a_slippage_20, f0_6(ad_24, g_pips_192), f0_8(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52,
						a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(1000);
		}
		
		break;
		
	case 4:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_4, a_price_12, a_slippage_20, f0_6(ad_24, g_pips_192), f0_8(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52,
						a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 0:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			RefreshRates();
			ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, f0_6(Bid, g_pips_192), f0_8(Ask, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 3:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_SELLLIMIT, a_lots_4, a_price_12, a_slippage_20, f0_7(ad_24, g_pips_192), f0_9(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52,
						a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 5:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_4, a_price_12, a_slippage_20, f0_7(ad_24, g_pips_192), f0_9(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52,
						a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 1:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, f0_7(Ask, g_pips_192), f0_9(Bid, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
	}
	
	return (ticket_60);
}

double Constellation::f0_6(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 - ai_8 * Point);
}

double Constellation::f0_7(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 + ai_8 * Point);
}

double Constellation::f0_8(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 + ai_8 * Point);
}

double Constellation::f0_9(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 - ai_8 * Point);
}

double Constellation::f0_10() {
	double ld_ret_0 = 0;
	
	for (g_pos_444 = OrdersTotal() - 1; g_pos_444 >= 0; g_pos_444--) {
		OrderSelect(g_pos_444, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != gi_316)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316)
			if (OrderType() == OP_BUY || OrderType() == OP_SELL)
				ld_ret_0 += OrderProfit();
	}
	
	return (ld_ret_0);
}

void Constellation::f0_11(int ai_0, int ai_4, double a_price_8) {
	int li_16;
	double order_stoploss_20;
	double price_28;
	
	if (ai_4 != 0) {
		for (int pos_36 = OrdersTotal() - 1; pos_36 >= 0; pos_36--) {
			if (OrderSelect(pos_36, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != gi_316)
					continue;
					
				if (OrderSymbol() == Symbol() || OrderMagicNumber() == gi_316) {
					if (OrderType() == OP_BUY) {
						li_16 = NormalizeDouble((Bid - a_price_8) / Point, 0);
						
						if (li_16 < ai_0)
							continue;
							
						order_stoploss_20 = OrderStopLoss();
						
						price_28 = Bid - ai_4 * Point;
						
						if (order_stoploss_20 == 0.0 || (order_stoploss_20 != 0.0 && price_28 > order_stoploss_20))
							OrderModify(OrderTicket(), a_price_8, price_28, OrderTakeProfit(), 0, Aqua);
					}
					
					if (OrderType() == OP_SELL) {
						li_16 = NormalizeDouble((a_price_8 - Ask) / Point, 0);
						
						if (li_16 < ai_0)
							continue;
							
						order_stoploss_20 = OrderStopLoss();
						
						price_28 = Ask + ai_4 * Point;
						
						if (order_stoploss_20 == 0.0 || (order_stoploss_20 != 0.0 && price_28 < order_stoploss_20))
							OrderModify(OrderTicket(), a_price_8, price_28, OrderTakeProfit(), 0, Red);
					}
				}
				
				Sleep(1000);
			}
		}
	}
}

double Constellation::f0_12() {
	if (f0_3() == 0)
		gd_488 = AccountEquity();
		
	if (gd_488 < gd_496)
		gd_488 = gd_496;
	else
		gd_488 = AccountEquity();
		
	gd_496 = AccountEquity();
	
	return (gd_488);
}

double Constellation::f0_13() {
	double order_open_price_0;
	int ticket_8;
	double ld_unused_12 = 0;
	int ticket_20 = 0;
	
	for (int pos_24 = OrdersTotal() - 1; pos_24 >= 0; pos_24--) {
		OrderSelect(pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != gi_316)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316 && OrderType() == OP_BUY) {
			ticket_8 = OrderTicket();
			
			if (ticket_8 > ticket_20) {
				order_open_price_0 = OrderOpenPrice();
				ld_unused_12 = order_open_price_0;
				ticket_20 = ticket_8;
			}
		}
	}
	
	return (order_open_price_0);
}

double Constellation::f0_14() {
	double order_open_price_0;
	int ticket_8;
	double ld_unused_12 = 0;
	int ticket_20 = 0;
	
	for (int pos_24 = OrdersTotal() - 1; pos_24 >= 0; pos_24--) {
		OrderSelect(pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != gi_316)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_316 && OrderType() == OP_SELL) {
			ticket_8 = OrderTicket();
			
			if (ticket_8 > ticket_20) {
				order_open_price_0 = OrderOpenPrice();
				ld_unused_12 = order_open_price_0;
				ticket_20 = ticket_8;
			}
		}
	}
	
	return (order_open_price_0);
}

void Constellation::f0_15() {
	Comment("            Stark-Profit " + Symbol() + "  " + Period(),
			"\n", "            Forex Account Server:", AccountServer(),
			"\n", "            Lots:  ", Lots,
			"\n", "            Symbol: ", Symbol(),
			"\n", "            Price:  ", NormalizeDouble(Bid, 4),
			"\n", "            Date: ", Month(), "-", Day(), "-", Year(), " Server Time: ", Hour(), ":", Minute(), ":", Seconds(),
			"\n");
}

void Constellation::f0_16() {
	double ld_0 = f0_18(0);
	string name_8 = gs_96 + "1";
	
	if (ObjectFind(name_8) == -1) {
		ObjectCreate(name_8, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_8, OBJPROP_CORNER, 1);
		ObjectSet(name_8, OBJPROP_XDISTANCE, 10);
		ObjectSet(name_8, OBJPROP_YDISTANCE, 15);
	}
	
	ObjectSetText(name_8, "Çàðàáîòîê ñåãîäíÿ: " + DoubleToStr(ld_0, 2), 10, "Courier New", Yellow);
	
	ld_0 = f0_18(1);
	name_8 = gs_96 + "2";
	
	if (ObjectFind(name_8) == -1) {
		ObjectCreate(name_8, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_8, OBJPROP_CORNER, 1);
		ObjectSet(name_8, OBJPROP_XDISTANCE, 10);
		ObjectSet(name_8, OBJPROP_YDISTANCE, 30);
	}
	
	ObjectSetText(name_8, "Çàðàáîòîê â÷åðà: " + DoubleToStr(ld_0, 2), 10, "Courier New", Yellow);
	
	ld_0 = f0_18(2);
	name_8 = gs_96 + "3";
	
	if (ObjectFind(name_8) == -1) {
		ObjectCreate(name_8, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_8, OBJPROP_CORNER, 1);
		ObjectSet(name_8, OBJPROP_XDISTANCE, 10);
		ObjectSet(name_8, OBJPROP_YDISTANCE, 45);
	}
	
	ObjectSetText(name_8, "Çàðàáîòîê ïîçàâ÷åðà: " + DoubleToStr(ld_0, 2), 10, "Courier New", Yellow);
	
	name_8 = gs_96 + "4";
	
	if (ObjectFind(name_8) == -1) {
		ObjectCreate(name_8, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_8, OBJPROP_CORNER, 1);
		ObjectSet(name_8, OBJPROP_XDISTANCE, 10);
		ObjectSet(name_8, OBJPROP_YDISTANCE, 75);
	}
	
	ObjectSetText(name_8, "Áàëàíñ: " + DoubleToStr(AccountBalance(), 2), 12, "Courier New", Yellow);
	
	name_8 = gs_96 + "5";
	
	if (ObjectFind(name_8) == -1) {
		ObjectCreate(name_8, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_8, OBJPROP_CORNER, 1);
		ObjectSet(name_8, OBJPROP_XDISTANCE, 10);
		ObjectSet(name_8, OBJPROP_YDISTANCE, 100);
	}
	
	ObjectSetText(name_8, "Ñâîá. ñð-âà ñ÷åòà: " + DoubleToStr(AccountFreeMargin(), 2), 10, "Courier New", Yellow);
}

void Constellation::f0_17() {
	string name_0 = gs_96 + "L_1";
	
	if (ObjectFind(name_0) == -1) {
		ObjectCreate(name_0, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_0, OBJPROP_CORNER, 0);
		ObjectSet(name_0, OBJPROP_XDISTANCE, 390);
		ObjectSet(name_0, OBJPROP_YDISTANCE, 10);
	}
	
	ObjectSetText(name_0, " T U R B O ", 28, "Arial", DarkTurquoise);
	
	name_0 = gs_96 + "L_2";
	
	if (ObjectFind(name_0) == -1) {
		ObjectCreate(name_0, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_0, OBJPROP_CORNER, 0);
		ObjectSet(name_0, OBJPROP_XDISTANCE, 382);
		ObjectSet(name_0, OBJPROP_YDISTANCE, 50);
	}
	
	ObjectSetText(name_0, "           M A X", 16, "Arial", Gold);
	
	name_0 = gs_96 + "L_3";
	
	if (ObjectFind(name_0) == -1) {
		ObjectCreate(name_0, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_0, OBJPROP_CORNER, 0);
		ObjectSet(name_0, OBJPROP_XDISTANCE, 397);
		ObjectSet(name_0, OBJPROP_YDISTANCE, 75);
	}
	
	ObjectSetText(name_0, "  *** forex-sovetnic.ru ***", 12, "Arial", DarkTurquoise);
	
	name_0 = gs_96 + "L_4";
	
	if (ObjectFind(name_0) == -1) {
		ObjectCreate(name_0, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_0, OBJPROP_CORNER, 0);
		ObjectSet(name_0, OBJPROP_XDISTANCE, 382);
		ObjectSet(name_0, OBJPROP_YDISTANCE, 57);
	}
	
	ObjectSetText(name_0, "  _____________________", 12, "Arial", DarkGray);
	
	name_0 = gs_96 + "L_5";
	
	if (ObjectFind(name_0) == -1) {
		ObjectCreate(name_0, OBJ_LABEL, 0, 0, 0);
		ObjectSet(name_0, OBJPROP_CORNER, 0);
		ObjectSet(name_0, OBJPROP_XDISTANCE, 382);
		ObjectSet(name_0, OBJPROP_YDISTANCE, 76);
	}
	
	ObjectSetText(name_0, "  _____________________", 12, "Arial", DarkGray);
}

double Constellation::f0_18(int ai_0) {
	double ld_ret_4 = 0;
	
	for (int pos_12 = 0; pos_12 < OrdersHistoryTotal(); pos_12++) {
		if (!(OrderSelect(pos_12, SELECT_BY_POS, MODE_HISTORY)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic)
			if (OrderCloseTime() >= iTime(Symbol(), PERIOD_D1, ai_0) && OrderCloseTime() < iTime(Symbol(), PERIOD_D1, ai_0) + 86400)
				ld_ret_4 = ld_ret_4 + OrderProfit() + OrderCommission() + OrderSwap();
	}
	
	return (ld_ret_4);
}

}

#endif
