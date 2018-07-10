#include "Overlook.h"

#if 0

// ilan

namespace Overlook {

Musk::Musk() {

}

void Musk::InitEA() {
	gd_260 = MarketInfo(Symbol(), MODE_SPREAD) * Point;
	return (0);
}

void Musk::StartEA(int pos) {
	double l_iclose_8;
	double l_iclose_16;
	
	if (UseTrailingStop)
		TrailingAlls(TrailStart, TrailStop, g_price_212);
		
	if (UseTimeOut) {
		if (TimeCurrent() >= gi_284) {
			CloseThisSymbolAll();
			Print("Closed All due to TimeOut");
		}
	}
	
	if (gi_280 == Now)
		return (0);
		
	gi_280 = Now;
	
	double ld_0 = CalculateProfit();
	
	if (UseEquityStop) {
		if (ld_0 < 0.0 && MathAbs(ld_0) > TotalEquityRisk / 100.0 * AccountEquityHigh()) {
			CloseThisSymbolAll();
			Print("Closed All due to Stop Out");
			gi_332 = false;
		}
	}
	
	gi_304 = CountTrades();
	
	if (gi_304 == 0)
		gi_268 = false;
		
	for (g_pos_300 = OrdersTotal() - 1; g_pos_300 >= 0; g_pos_300--) {
		OrderSelect(g_pos_300, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_176)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176) {
			if (OrderType() == OP_BUY) {
				gi_320 = true;
				gi_324 = false;
				break;
			}
		}
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176) {
			if (OrderType() == OP_SELL) {
				gi_320 = false;
				gi_324 = true;
				break;
			}
		}
	}
	
	if (gi_304 > 0 && gi_304 <= MaxTrades) {
		RefreshRates();
		gd_236 = FindLastBuyPrice();
		gd_244 = FindLastSellPrice();
		
		if (gi_320 && gd_236 - Ask >= PipStep * Point)
			gi_316 = true;
			
		if (gi_324 && Bid - gd_244 >= PipStep * Point)
			gi_316 = true;
	}
	
	if (gi_304 < 1) {
		gi_324 = false;
		gi_320 = false;
		gi_316 = true;
		gd_188 = AccountEquity();
	}
	
	if (gi_316) {
		gd_236 = FindLastBuyPrice();
		gd_244 = FindLastSellPrice();
		
		if (gi_324) {
			gi_288 = gi_304;
			gd_292 = NormalizeDouble(Lots * MathPow(LotExponent, gi_288), lotdecimal);
			RefreshRates();
			gi_328 = OpenPendingOrder(1, gd_292, Bid, slip, Ask, 0, 0, gs_ilan_272 + "-" + gi_288, g_magic_176, 0, HotPink);
			
			if (gi_328 < 0) {
				Print("Error: " + GetLastError());
				return (0);
			}
			
			gd_244 = FindLastSellPrice();
			
			gi_316 = false;
			gi_332 = true;
		}
		
		else {
			if (gi_320) {
				gi_288 = gi_304;
				gd_292 = NormalizeDouble(Lots * MathPow(LotExponent, gi_288), lotdecimal);
				gi_328 = OpenPendingOrder(0, gd_292, Ask, slip, Bid, 0, 0, gs_ilan_272 + "-" + gi_288, g_magic_176, 0, Lime);
				
				if (gi_328 < 0) {
					Print("Error: " + GetLastError());
					return (0);
				}
				
				gd_236 = FindLastBuyPrice();
				
				gi_316 = false;
				gi_332 = true;
			}
		}
	}
	
	if (gi_316 && gi_304 < 1) {
		l_iclose_8 = iClose(Symbol(), 0, 2);
		l_iclose_16 = iClose(Symbol(), 0, 1);
		g_bid_220 = Bid;
		g_ask_228 = Ask;
		
		if (!gi_324 && !gi_320) {
			gi_288 = gi_304;
			gd_292 = NormalizeDouble(Lots * MathPow(LotExponent, gi_288), lotdecimal);
			
			if (l_iclose_8 > l_iclose_16) {
				gi_328 = OpenPendingOrder(1, gd_292, g_bid_220, slip, g_bid_220, 0, 0, gs_ilan_272 + "-" + gi_288, g_magic_176, 0, HotPink);
				
				if (gi_328 < 0) {
					Print("Error: " + GetLastError());
					return (0);
				}
				
				gd_236 = FindLastBuyPrice();
				
				gi_332 = true;
			}
			
			else {
				gi_328 = OpenPendingOrder(0, gd_292, g_ask_228, slip, g_ask_228, 0, 0, gs_ilan_272 + "-" + gi_288, g_magic_176, 0, Lime);
				
				if (gi_328 < 0) {
					Print("Error: " + GetLastError());
					return (0);
				}
				
				gd_244 = FindLastSellPrice();
				
				gi_332 = true;
			}
			
			if (gi_328 > 0)
				gi_284 = TimeCurrent() + 60.0 * (60.0 * MaxTradeOpenHours);
				
			gi_316 = false;
		}
	}
	
	gi_304 = CountTrades();
	
	g_price_212 = 0;
	double ld_24 = 0;
	
	for (g_pos_300 = OrdersTotal() - 1; g_pos_300 >= 0; g_pos_300--) {
		OrderSelect(g_pos_300, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_176)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176) {
			if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
				g_price_212 += OrderOpenPrice() * OrderLots();
				ld_24 += OrderLots();
			}
		}
	}
	
	if (gi_304 > 0)
		g_price_212 = NormalizeDouble(g_price_212 / ld_24, Digits);
		
	if (gi_332) {
		for (g_pos_300 = OrdersTotal() - 1; g_pos_300 >= 0; g_pos_300--) {
			OrderSelect(g_pos_300, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_176)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176) {
				if (OrderType() == OP_BUY) {
					g_price_180 = g_price_212 + TakeProfit * Point;
					gd_unused_196 = g_price_180;
					gd_308 = g_price_212 - Stoploss * Point;
					gi_268 = true;
				}
			}
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176) {
				if (OrderType() == OP_SELL) {
					g_price_180 = g_price_212 - TakeProfit * Point;
					gd_unused_204 = g_price_180;
					gd_308 = g_price_212 + Stoploss * Point;
					gi_268 = true;
				}
			}
		}
	}
	
	if (gi_332) {
		if (gi_268 == true) {
			for (g_pos_300 = OrdersTotal() - 1; g_pos_300 >= 0; g_pos_300--) {
				OrderSelect(g_pos_300, SELECT_BY_POS, MODE_TRADES);
				
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_176)
					continue;
					
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176)
					OrderModify(OrderTicket(), g_price_212, OrderStopLoss(), g_price_180, 0, Yellow);
					
				gi_332 = false;
			}
		}
	}
	
	return (0);
}

int CountTrades() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = OrdersTotal() - 1; l_pos_4 >= 0; l_pos_4--) {
		OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_176)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176)
			if (OrderType() == OP_SELL || OrderType() == OP_BUY)
				l_count_0++;
	}
	
	return (l_count_0);
}

void CloseThisSymbolAll() {
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol()) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176) {
				if (OrderType() == OP_BUY)
					OrderClose(OrderTicket(), OrderLots(), Bid, slip, Blue);
					
				if (OrderType() == OP_SELL)
					OrderClose(OrderTicket(), OrderLots(), Ask, slip, Red);
			}
			
			Sleep(1000);
		}
	}
}

int OpenPendingOrder(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_32, int ai_36, String a_comment_40, int a_magic_48, int a_datetime_52, color a_color_56) {
	int l_ticket_60 = 0;
	int l_error_64 = 0;
	int l_count_68 = 0;
	int li_72 = 100;
	
	switch (ai_0) {
	
	case 2:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_BUYLIMIT, a_lots_4, a_price_12, a_slippage_20, StopLong(ad_24, ai_32), TakeLong(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!(l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */))
				break;
				
			Sleep(1000);
		}
		
		break;
		
	case 4:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_4, a_price_12, a_slippage_20, StopLong(ad_24, ai_32), TakeLong(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!(l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 0:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			RefreshRates();
			l_ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, StopLong(Bid, ai_32), TakeLong(Ask, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!(l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 3:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_SELLLIMIT, a_lots_4, a_price_12, a_slippage_20, StopShort(ad_24, ai_32), TakeShort(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!(l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 5:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_4, a_price_12, a_slippage_20, StopShort(ad_24, ai_32), TakeShort(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!(l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 1:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			l_ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, StopShort(Ask, ai_32), TakeShort(Bid, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			l_error_64 = GetLastError();
			
			if (l_error_64 == 0/* NO_ERROR */)
				break;
				
			if (!(l_error_64 == 4/* SERVER_BUSY */ || l_error_64 == 137/* BROKER_BUSY */ || l_error_64 == 146/* TRADE_CONTEXT_BUSY */ || l_error_64 == 136/* OFF_QUOTES */))
				break;
				
			Sleep(5000);
		}
	}
	
	return (l_ticket_60);
}

double Musk::StopLong(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
	else
		return (ad_0 - ai_8 * Point);
}

double Musk::StopShort(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
	else
		return (ad_0 + ai_8 * Point);
}

double Musk::TakeLong(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
	else
		return (ad_0 + ai_8 * Point);
}

double Musk::TakeShort(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
	else
		return (ad_0 - ai_8 * Point);
}

double Musk::CalculateProfit() {
	double ld_ret_0 = 0;
	
	for (g_pos_300 = OrdersTotal() - 1; g_pos_300 >= 0; g_pos_300--) {
		OrderSelect(g_pos_300, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_176)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176)
			if (OrderType() == OP_BUY || OrderType() == OP_SELL)
				ld_ret_0 += OrderProfit();
	}
	
	return (ld_ret_0);
}

void Musk::TrailingAlls(int ai_0, int ai_4, double a_price_8) {
	int l_ticket_16;
	double l_ord_stoploss_20;
	double l_price_28;
	
	if (ai_4 != 0) {
		for (int l_pos_36 = OrdersTotal() - 1; l_pos_36 >= 0; l_pos_36--) {
			if (OrderSelect(l_pos_36, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_176)
					continue;
					
				if (OrderSymbol() == Symbol() || OrderMagicNumber() == g_magic_176) {
					if (OrderType() == OP_BUY) {
						l_ticket_16 = NormalizeDouble((Bid - a_price_8) / Point, 0);
						
						if (l_ticket_16 < ai_0)
							continue;
							
						l_ord_stoploss_20 = OrderStopLoss();
						
						l_price_28 = Bid - ai_4 * Point;
						
						if (l_ord_stoploss_20 == 0.0 || (l_ord_stoploss_20 != 0.0 && l_price_28 > l_ord_stoploss_20))
							OrderModify(OrderTicket(), a_price_8, l_price_28, OrderTakeProfit(), 0, Aqua);
					}
					
					if (OrderType() == OP_SELL) {
						l_ticket_16 = NormalizeDouble((a_price_8 - Ask) / Point, 0);
						
						if (l_ticket_16 < ai_0)
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

double Musk::AccountEquityHigh() {
	if (CountTrades() == 0)
		gd_336 = AccountEquity();
		
	if (gd_336 < gd_344)
		gd_336 = gd_344;
	else
		gd_336 = AccountEquity();
		
	gd_344 = AccountEquity();
	
	return (gd_336);
}

double Musk::FindLastBuyPrice() {
	double l_ord_open_price_8;
	int l_ticket_24;
	double ld_unused_0 = 0;
	int l_ticket_20 = 0;
	
	for (int l_pos_16 = OrdersTotal() - 1; l_pos_16 >= 0; l_pos_16--) {
		OrderSelect(l_pos_16, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_176)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176 && OrderType() == OP_BUY) {
			l_ticket_24 = OrderTicket();
			
			if (l_ticket_24 > l_ticket_20) {
				l_ord_open_price_8 = OrderOpenPrice();
				ld_unused_0 = l_ord_open_price_8;
				l_ticket_20 = l_ticket_24;
			}
		}
	}
	
	return (l_ord_open_price_8);
}

double Musk::FindLastSellPrice() {
	double l_ord_open_price_8;
	int l_ticket_24;
	double ld_unused_0 = 0;
	int l_ticket_20 = 0;
	
	for (int l_pos_16 = OrdersTotal() - 1; l_pos_16 >= 0; l_pos_16--) {
		OrderSelect(l_pos_16, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_176)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_176 && OrderType() == OP_SELL) {
			l_ticket_24 = OrderTicket();
			
			if (l_ticket_24 > l_ticket_20) {
				l_ord_open_price_8 = OrderOpenPrice();
				ld_unused_0 = l_ord_open_price_8;
				l_ticket_20 = l_ticket_24;
			}
		}
	}
	
	return (l_ord_open_price_8);
}
#endif
