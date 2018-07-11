#include "Overlook.h"

#if 0

// bunny

namespace Overlook {

Rabbit::Rabbit() {
	
}

void Rabbit::InitEA() {
	gd_332 = MarketInfo(Symbol(), MODE_SPREAD) * Point;
	int i = MarketInfo(Symbol(), MODE_MINLOT) * 1000;
	
	switch (i) {
	
	case 1:
		gd_240 = 3;
		break;
		
	case 10:
		gd_240 = 2;
		break;
		
	case 100:
		gd_240 = 1;
		break;
		
	case 1000:
		gd_240 = 0;
	}
	
	return (0);
}

void Rabbit::StartEA(int pos) {
	double l_ord_lots_0;
	double l_ord_lots_8;
	double l_iclose_16;
	double l_iclose_24;
	
	if (UseTrailing)
		TrailingAlls(TrailStart, TrailStop, g_price_292);
		
	if (gi_220) {
		if (TimeCurrent() >= gi_356) {
			CloseThisSymbolAll();
			Print("Closed All due to TimeOut");
		}
	}
	
	if (g_time_352 == Time[0])
		return (0);
		
	g_time_352 = Time[0];
	
	double ld_32 = CalculateProfit();
	
	if (SafeEquity) {
		if (ld_32 < 0.0 && MathAbs(ld_32) > SafeEquityRisk / 100.0 * AccountEquityHigh()) {
			CloseThisSymbolAll();
			Print("Closed All due to Stop Out");
			gi_404 = false;
		}
	}
	
	gi_376 = CountTrades();
	
	if (gi_376 == 0)
		gi_340 = false;
		
	for (g_pos_372 = OrdersTotal() - 1; g_pos_372 >= 0; g_pos_372--) {
		OrderSelect(g_pos_372, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			if (OrderType() == OP_BUY) {
				gi_392 = true;
				gi_396 = false;
				l_ord_lots_0 = OrderLots();
				break;
			}
		}
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			if (OrderType() == OP_SELL) {
				gi_392 = false;
				gi_396 = true;
				l_ord_lots_8 = OrderLots();
				break;
			}
		}
	}
	
	if (gi_376 > 0 && gi_376 <= MaxCountOrders) {
		RefreshRates();
		gd_316 = FindLastBuyPrice();
		gd_324 = FindLastSellPrice();
		
		if (gi_392 && gd_316 - Ask >= StepLots * Point)
			gi_388 = true;
			
		if (gi_396 && Bid - gd_324 >= StepLots * Point)
			gi_388 = true;
	}
	
	if (gi_376 < 1) {
		gi_396 = false;
		gi_392 = false;
		gi_388 = true;
		gd_268 = AccountEquity();
	}
	
	if (gi_388) {
		gd_316 = FindLastBuyPrice();
		gd_324 = FindLastSellPrice();
		
		if (gi_396) {
			if (gi_252) {
				fOrderCloseMarket(0, 1);
				gd_364 = NormalizeDouble(MultiLotsFactor * l_ord_lots_8, gd_240);
			}
			
			else
				gd_364 = fGetLots(OP_SELL);
				
			if (gi_248) {
				gi_360 = gi_376;
				
				if (gd_364 > 0.0) {
					RefreshRates();
					gi_400 = OpenPendingOrder(1, gd_364, Bid, slippage, Ask, 0, 0, gs_344 + "-" + gi_360, MagicNumber, 0, HotPink);
					
					if (gi_400 < 0) {
						Print("Error: ", GetLastError());
						return (0);
					}
					
					gd_324 = FindLastSellPrice();
					
					gi_388 = false;
					gi_404 = true;
				}
			}
		}
		
		else {
			if (gi_392) {
				if (gi_252) {
					fOrderCloseMarket(1, 0);
					gd_364 = NormalizeDouble(MultiLotsFactor * l_ord_lots_0, gd_240);
				}
				
				else
					gd_364 = fGetLots(OP_BUY);
					
				if (gi_248) {
					gi_360 = gi_376;
					
					if (gd_364 > 0.0) {
						gi_400 = OpenPendingOrder(0, gd_364, Ask, slippage, Bid, 0, 0, gs_344 + "-" + gi_360, MagicNumber, 0, Lime);
						
						if (gi_400 < 0) {
							Print("Error: ", GetLastError());
							return (0);
						}
						
						gd_316 = FindLastBuyPrice();
						
						gi_388 = false;
						gi_404 = true;
					}
				}
			}
		}
	}
	
	if (gi_388 && gi_376 < 1) {
		l_iclose_16 = iClose(Symbol(), 0, 2);
		l_iclose_24 = iClose(Symbol(), 0, 1);
		g_bid_300 = Bid;
		g_ask_308 = Ask;
		
		if (!gi_396 && !gi_392) {
			gi_360 = gi_376;
			
			if (l_iclose_16 > l_iclose_24) {
				gd_364 = fGetLots(OP_SELL);
				
				if (gd_364 > 0.0) {
					gi_400 = OpenPendingOrder(1, gd_364, g_bid_300, slippage, g_bid_300, 0, 0, "", MagicNumber, 0, HotPink);
					
					if (gi_400 < 0) {
						Print(gd_364, "Error: ", GetLastError());
						return (0);
					}
					
					gd_316 = FindLastBuyPrice();
					
					gi_404 = true;
				}
			}
			
			else {
				gd_364 = fGetLots(OP_BUY);
				
				if (gd_364 > 0.0) {
					gi_400 = OpenPendingOrder(0, gd_364, g_ask_308, slippage, g_ask_308, 0, 0, "", MagicNumber, 0, Lime);
					
					if (gi_400 < 0) {
						Print(gd_364, "Error: ", GetLastError());
						return (0);
					}
					
					gd_324 = FindLastSellPrice();
					
					gi_404 = true;
				}
			}
		}
		
		if (gi_400 > 0)
			gi_356 = TimeCurrent() + 60.0 * (60.0 * gd_224);
			
		gi_388 = false;
	}
	
	gi_376 = CountTrades();
	
	g_price_292 = 0;
	double ld_40 = 0;
	
	for (g_pos_372 = OrdersTotal() - 1; g_pos_372 >= 0; g_pos_372--) {
		OrderSelect(g_pos_372, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
				g_price_292 += OrderOpenPrice() * OrderLots();
				ld_40 += OrderLots();
			}
		}
	}
	
	if (gi_376 > 0)
		g_price_292 = NormalizeDouble(g_price_292 / ld_40, Digits);
		
	if (gi_404) {
		for (g_pos_372 = OrdersTotal() - 1; g_pos_372 >= 0; g_pos_372--) {
			OrderSelect(g_pos_372, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY) {
					g_price_260 = g_price_292 + TakeProfit * Point;
					gd_unused_276 = g_price_260;
					gd_380 = g_price_292 - g_pips_232 * Point;
					gi_340 = true;
				}
			}
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_SELL) {
					g_price_260 = g_price_292 - TakeProfit * Point;
					gd_unused_284 = g_price_260;
					gd_380 = g_price_292 + g_pips_232 * Point;
					gi_340 = true;
				}
			}
		}
	}
	
	if (gi_404) {
		if (gi_340 == true) {
			for (g_pos_372 = OrdersTotal() - 1; g_pos_372 >= 0; g_pos_372--) {
				OrderSelect(g_pos_372, SELECT_BY_POS, MODE_TRADES);
				
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
					continue;
					
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
					OrderModify(OrderTicket(), g_price_292, OrderStopLoss(), g_price_260, 0, Yellow);
					
				gi_404 = false;
			}
		}
	}
	
	return (0);
}

double Rabbit::ND(double ad_0) {
	return (NormalizeDouble(ad_0, Digits));
}

int Rabbit::fOrderCloseMarket(bool ai_0, bool ai_4) {
	int li_ret_8 = 0;
	
	for (int l_pos_12 = OrdersTotal() - 1; l_pos_12 >= 0; l_pos_12--) {
		if (OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY && ai_0) {
					RefreshRates();
					
					if (!IsTradeContextBusy()) {
						if (!OrderClose(OrderTicket(), OrderLots(), ND(Bid), 5, CLR_NONE)) {
							Print("Error close BUY " + OrderTicket());
							li_ret_8 = -1;
						}
					}
					
					else {
						if (g_datetime_408 != iTime(NULL, 0, 0)) {
							g_datetime_408 = iTime(NULL, 0, 0);
							Print("Need close BUY " + OrderTicket() + ". Trade Context Busy");
						}
						
						return (-2);
					}
				}
				
				if (OrderType() == OP_SELL && ai_4) {
					RefreshRates();
					
					if (!IsTradeContextBusy()) {
						if (!OrderClose(OrderTicket(), OrderLots(), ND(Ask), 5, CLR_NONE)) {
							Print("Error close SELL " + OrderTicket());
							li_ret_8 = -1;
						}
					}
					
					else {
						if (g_datetime_412 != iTime(NULL, 0, 0)) {
							g_datetime_412 = iTime(NULL, 0, 0);
							Print("Need close SELL " + OrderTicket() + ". Trade Context Busy");
						}
						
						return (-2);
					}
				}
			}
		}
	}
	
	return (li_ret_8);
}

double Rabbit::fGetLots(int a_cmd_0) {
	double l_lots_4;
	int l_datetime_12;
	
	switch (gi_256) {
	
	case 0:
		l_lots_4 = Lots;
		break;
		
	case 1:
		l_lots_4 = NormalizeDouble(Lots * MathPow(MultiLotsFactor, gi_360), gd_240);
		break;
		
	case 2:
		l_datetime_12 = 0;
		l_lots_4 = Lots;
		
		for (int l_pos_20 = OrdersHistoryTotal() - 1; l_pos_20 >= 0; l_pos_20--) {
			if (OrderSelect(l_pos_20, SELECT_BY_POS, MODE_HISTORY)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
					if (l_datetime_12 < OrderCloseTime()) {
						l_datetime_12 = OrderCloseTime();
						
						if (OrderProfit() < 0.0)
							l_lots_4 = NormalizeDouble(OrderLots() * MultiLotsFactor, gd_240);
						else
							l_lots_4 = Lots;
					}
				}
			}
			
			else
				return (-3);
		}
	}
	
	if (AccountFreeMarginCheck(Symbol(), a_cmd_0, l_lots_4) <= 0.0)
		return (-1);
		
	if (GetLastError() == 134/* NOT_ENOUGH_MONEY */)
		return (-2);
		
	return (l_lots_4);
}

int Rabbit::CountTrades() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = OrdersTotal() - 1; l_pos_4 >= 0; l_pos_4--) {
		OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
			if (OrderType() == OP_SELL || OrderType() == OP_BUY)
				l_count_0++;
	}
	
	return (l_count_0);
}

void Rabbit::CloseThisSymbolAll() {
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol()) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY)
					OrderClose(OrderTicket(), OrderLots(), Bid, slippage, Blue);
					
				if (OrderType() == OP_SELL)
					OrderClose(OrderTicket(), OrderLots(), Ask, slippage, Red);
			}
			
			Sleep(1000);
		}
	}
}

int Rabbit::OpenPendingOrder(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_32, int ai_36, string a_comment_40, int a_magic_48, int a_datetime_52, color a_color_56) {
	int l_ticket_60 = 0;
	int l_error_64 = 0;
	int l_count_68 = 0;
	int li_72 = 100;
	
	switch (ai_0) {
	
	case 2:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_BUYLIMIT, a_lots_4, a_price_12, a_slippage_20, StopLong(ad_24, ai_32), TakeLong(a_price_12, ai_36), a_comment_40, a_magic_48);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(1000);
		}
		
		break;
		
	case 4:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_4, a_price_12, a_slippage_20, StopLong(ad_24, ai_32), TakeLong(a_price_12, ai_36), a_comment_40, a_magic_48);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 0:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			RefreshRates();
			l_ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, StopLong(Bid, ai_32), TakeLong(Ask, ai_36), a_comment_40, a_magic_48);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 3:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_SELLLIMIT, a_lots_4, a_price_12, a_slippage_20, StopShort(ad_24, ai_32), TakeShort(a_price_12, ai_36), a_comment_40, a_magic_48);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 5:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_4, a_price_12, a_slippage_20, StopShort(ad_24, ai_32), TakeShort(a_price_12, ai_36), a_comment_40, a_magic_48);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 1:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, StopShort(Ask, ai_32), TakeShort(Bid, ai_36), a_comment_40, a_magic_48);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
	}
	
	return (l_ticket_60);
}

double Rabbit::StopLong(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 - ai_8 * Point);
}

double Rabbit::StopShort(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 + ai_8 * Point);
}

double Rabbit::TakeLong(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 + ai_8 * Point);
}

double Rabbit::TakeShort(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 - ai_8 * Point);
}

double Rabbit::CalculateProfit() {
	double ld_ret_0 = 0;
	
	for (g_pos_372 = OrdersTotal() - 1; g_pos_372 >= 0; g_pos_372--) {
		OrderSelect(g_pos_372, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
			if (OrderType() == OP_BUY || OrderType() == OP_SELL)
				ld_ret_0 += OrderProfit();
	}
	
	return (ld_ret_0);
}

void Rabbit::TrailingAlls(int ai_0, int ai_4, double a_price_8) {
	int li_16;
	double l_ord_stoploss_20;
	double l_price_28;
	
	if (ai_4 != 0) {
		for (int l_pos_36 = OrdersTotal() - 1; l_pos_36 >= 0; l_pos_36--) {
			if (OrderSelect(l_pos_36, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
					continue;
					
				if (OrderSymbol() == Symbol() || OrderMagicNumber() == MagicNumber) {
					if (OrderType() == OP_BUY) {
						li_16 = NormalizeDouble((Bid - a_price_8) / Point, 0);
						
						if (li_16 < ai_0)
							continue;
							
						l_ord_stoploss_20 = OrderStopLoss();
						
						l_price_28 = Bid - ai_4 * Point;
						
						if (l_ord_stoploss_20 == 0.0 || (l_ord_stoploss_20 != 0.0 && l_price_28 > l_ord_stoploss_20))
							OrderModify(OrderTicket(), a_price_8, l_price_28, OrderTakeProfit(), 0, Aqua);
					}
					
					if (OrderType() == OP_SELL) {
						li_16 = NormalizeDouble((a_price_8 - Ask) / Point, 0);
						
						if (li_16 < ai_0)
							continue;
							
						l_ord_stoploss_20 = OrderStopLoss();
						
						l_price_28 = Ask + ai_4 * Point;
						
						if (l_ord_stoploss_20 == 0.0 || (l_ord_stoploss_20 != 0.0 && l_price_28 < l_ord_stoploss_20))
							OrderModify(OrderTicket(), a_price_8, l_price_28, OrderTakeProfit(), 0, Red);
					}
				}
				
				Sleep(1000);
			}
		}
	}
}

double Rabbit::AccountEquityHigh() {
	if (CountTrades() == 0)
		gd_416 = AccountEquity();
		
	if (gd_416 < gd_424)
		gd_416 = gd_424;
	else
		gd_416 = AccountEquity();
		
	gd_424 = AccountEquity();
	
	return (gd_416);
}

double Rabbit::FindLastBuyPrice() {
	double l_ord_open_price_0;
	int l_ticket_8;
	double ld_unused_12 = 0;
	int l_ticket_20 = 0;
	
	for (int l_pos_24 = OrdersTotal() - 1; l_pos_24 >= 0; l_pos_24--) {
		OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber && OrderType() == OP_BUY) {
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

double Rabbit::FindLastSellPrice() {
	double l_ord_open_price_0;
	int l_ticket_8;
	double ld_unused_12 = 0;
	int l_ticket_20 = 0;
	
	for (int l_pos_24 = OrdersTotal() - 1; l_pos_24 >= 0; l_pos_24--) {
		OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber && OrderType() == OP_SELL) {
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

}

#endif
