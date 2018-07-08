#include "Overlook.h"

namespace Overlook {

Satellite::Satellite() {

}

void Satellite::InitEA() {
	bool li_4;
	gs_316 = "";
	gia_416[0] = 0;
	gia_416[1] = Level;
	gia_416[2] = Level;
	gia_416[3] = Level;
	gia_416[4] = Level;
	gia_416[5] = Level;
	gia_416[6] = Level;
	gia_416[7] = Level;
	gia_416[8] = Level;
	gia_416[9] = Level;
	gda_420[0] = Ord_1_Lots;
	gda_420[1] = Ord_2_Lots;
	gda_420[2] = Ord_3_Lots;
	gda_420[3] = Ord_4_Lots;
	gda_420[4] = Ord_5_Lots;
	gda_420[5] = Ord_6_Lots;
	gda_420[6] = Ord_7_Lots;
	gda_420[7] = Ord_8_Lots;
	gda_420[8] = Ord_9_Lots;
	gda_420[9] = Ord_10_Lots;
	gia_424[0] = Level;
	gia_424[1] = Level;
	gia_424[2] = Level;
	gia_424[3] = Level;
	gia_424[4] = Level;
	gia_424[5] = Level;
	gia_424[6] = Level;
	gia_424[7] = Level;
	gia_424[8] = Level;
	gia_424[9] = Level;
	gia_428[0] = Level;
	gia_428[1] = Level;
	gia_428[2] = Level;
	gia_428[3] = Level;
	gia_428[4] = Level;
	gia_428[5] = Level;
	gia_428[6] = Level;
	gia_428[7] = Level;
	gia_428[8] = Level;
	gia_428[9] = Level;
	
	switch (Trend) {
	
	case 1:
		gi_104 = 100;
		break;
		
	case 2:
		gi_104 = 200;
		break;
		
	case 3:
		gi_104 = 300;
	}
	
	if (Digits == 5 || Digits == 3)
		gi_104 = 10 * gi_104;
		
	gs_472 = "";
	
	if (IsDemo())
		gs_472 = gs_472 + "d_";
		
	if (!IsTesting() && !IsDemo()) {
		li_4 = true;
		
	}
}

void Satellite::StartEA(int pos) {

	bool li_0 = false;
	bool li_8 = false;
	bool li_12 = false;
	bool li_16 = false;
	bool li_20 = false;
	bool li_60 = false;
	int l_count_64 = 0;
	double l_price_68 = 0;
	double l_price_76 = 0;
	int li_84 = 0;
	
	if (!IsTesting() && !IsDemo()) {
		li_0 = false;
		
		for (int l_pos_4 = 0; l_pos_4 < 4; l_pos_4++) {
			if (gia_300[l_pos_4] == gia_300[l_pos_4]) {
				li_0 = true;
				break;
			}
		}
		
		if (!li_0) {
			Alert("Ýêñïåðò Capital-5ru - íåâåðíûé íîìåð ñ÷åòà!");
			return;
		}
	}
		
	
	if (!fLock())
		return;
		
	if (MW_Mode)
		WH_SLTP();
		
	int l_shift_24 = max(0, pos - 3600 * gi_100 / GetMinutePeriod());
	
	/*int l_highest_28 = GetInputBuffer(0, 2).Get(l_shift_24);
	
	int l_lowest_32 = GetInputBuffer(0, 1).Get(l_shift_24);
	
	if (l_highest_28 == 1)
		if (ND(iHigh(NULL, TimeFrame, l_highest_28) - iHigh(NULL, TimeFrame, l_lowest_32)) >= ND(Point * gi_104))
			li_12 = true;
			
	if (l_lowest_32 == 1)
		if (ND(iHigh(NULL, TimeFrame, l_highest_28) - iHigh(NULL, TimeFrame, l_lowest_32)) >= ND(Point * gi_104))
			li_8 = true;
			
	l_shift_24 = iBarShift(NULL, TimeFrame, iTime(NULL, TimeFrame, 1) - 3600 * gi_100, false);
	
	l_highest_28 = iHighest(NULL, TimeFrame, MODE_HIGH, l_shift_24 - 1, 2);
	
	l_lowest_32 = iLowest(NULL, TimeFrame, MODE_LOW, l_shift_24 - 1, 2);
	
	if (l_highest_28 == 2)
		if (ND(iHigh(NULL, TimeFrame, l_highest_28) - iHigh(NULL, TimeFrame, l_lowest_32)) >= ND(Point * gi_104))
			li_20 = true;
			
	if (l_lowest_32 == 2)
		if (ND(iHigh(NULL, TimeFrame, l_highest_28) - iHigh(NULL, TimeFrame, l_lowest_32)) >= ND(Point * gi_104))
			li_16 = true;
			*/
	int l_bool_36 = true;
	
	int l_bool_40 = true;
	
	bool li_44 = false;
	
	bool li_48 = false;
	
	bool li_52 = false;
	
	bool li_56 = false;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
				if (OrderType() == OP_BUY)
					li_44 = true;
					
				if (OrderType() == OP_SELL)
					li_48 = true;
					
				if (OrderType() == OP_BUYLIMIT)
					li_52 = true;
					
				if (OrderType() == OP_SELLLIMIT)
					li_56 = true;
			}
		}
		
		else
			return;
	}
	
	if ((!li_44 && li_52) || (!li_48 && li_56)) {
		li_60 = false;
		
		for (int l_pos_4 = OrdersTotal() - 1; l_pos_4 >= 0; l_pos_4--) {
			if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
					if (OrderType() == OP_BUYLIMIT && !li_44) {
						bool succ = OrderDelete(OrderTicket());
						
						if (!succ) {
								
							li_60 = true;
						}
						
					}
					
					else if (OrderType() == OP_SELLLIMIT && !li_48) {
						bool succ = OrderDelete(OrderTicket());
						
						if (!succ) {
								
							li_60 = true;
						}
						
					}
				}
			}
			
			else
				return;
		}
		
		if (li_60)
			return;
	}
	
	if (l_bool_36 || l_bool_40 && CanOpenNew) {
		l_count_64 = 0;
		
		for (int l_pos_4 = OrdersTotal() - 1; l_pos_4 >= 0; l_pos_4--) {
			if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N)
					l_count_64++;
			}
			
			else
				return;
		}
		
		if (l_count_64 == 0) {
			if (l_bool_36 && Now > gi_372) {
				RefreshRates();
				l_price_68 = ND(Ask - Point * Level);
				l_price_76 = ND(Ask + Point * Level);
				
				if (MW_Mode) {
					l_price_68 = 0;
					l_price_76 = 0;
				}
				
				g_ticket_328 = OrderSend(Symbol(), OP_BUY, Ord_1_Lots, ND(Ask), Slippage, l_price_68, l_price_76, "Ord_1_", Magic_N);
				
				
				if (g_error_332 != 0/* NO_ERROR */) {
						
					return;
				}
				
				if (gi_324)
					Print(gs_316 + "Order BUY " + g_ticket_328 + " opened (1)");
					
				gi_372 = TimeCurrent();
								
				gd_464 = Ask;
				
				if (OrderSelect(g_ticket_328, SELECT_BY_TICKET, MODE_TRADES))
					gd_464 = OrderOpenPrice();
					
				Alarm(g_ticket_328, 0, 1, gd_464, Ord_1_Lots);
				
				if (MW_Mode)
					WH_SLTP();
			}
			
			if (l_bool_40 && Now > gi_372) {
				RefreshRates();
				l_price_68 = ND(Bid + Point * Level);
				l_price_76 = ND(Bid - Point * Level);
				
				if (MW_Mode) {
					l_price_68 = 0;
					l_price_76 = 0;
				}
				
				g_ticket_328 = OrderSend(Symbol(), OP_SELL, Ord_1_Lots, ND(Bid), Slippage, l_price_68, l_price_76, "Ord_1_", Magic_N);
				
				
				if (g_error_332 != 0/* NO_ERROR */) {
						
					return;
				}
				
				if (gi_324)
					Print(gs_316 + "Order SELL " + g_ticket_328 + " opened (1)");
					
				gi_372 = TimeCurrent();
								
				gd_464 = Bid;
				
				if (OrderSelect(g_ticket_328, SELECT_BY_TICKET, MODE_TRADES))
					gd_464 = OrderOpenPrice();
					
				Alarm(g_ticket_328, 1, 1, gd_464, Ord_1_Lots);
				
				if (MW_Mode)
					WH_SLTP();
			}
		}
	}
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersCount; l_pos_4++) {
		gba_392[l_pos_4] = 0;
		gba_400[l_pos_4] = 0;
		gba_396[l_pos_4] = 0;
		gba_404[l_pos_4] = 0;
	}
	
	gd_432 = 99999999;
	gd_440 = 99999999;
	gd_448 = 0;
	gd_456 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
				li_84 = OrderIndex(OrderComment());
				
				if (li_84 != -1) {
					if (OrderType() == OP_BUY || OrderType() == OP_BUYLIMIT) {
						if (OrderType() == OP_BUY) {
							gba_392[li_84] = 1;
							gd_432 = MathMin(ND(gd_432), ND(OrderTakeProfit()));
						}
						
						if (OrderType() == OP_BUYLIMIT)
							gba_396[li_84] = 1;
							
						gda_408[li_84] = OrderOpenPrice();
						
						gd_440 = MathMin(ND(gd_440), ND(OrderStopLoss()));
					}
					
					if (OrderType() == OP_SELL || OrderType() == OP_SELLLIMIT) {
						if (OrderType() == OP_SELL) {
							gba_400[li_84] = 1;
							gd_456 = MathMax(ND(gd_456), ND(OrderTakeProfit()));
						}
						
						if (OrderType() == OP_SELLLIMIT)
							gba_404[li_84] = 1;
							
						gda_412[li_84] = OrderOpenPrice();
						
						gd_448 = MathMax(ND(gd_448), ND(OrderStopLoss()));
					}
				}
			}
		}
		
		else
			return;
	}
	
	for (int l_pos_4 = 1; l_pos_4 < OrdersCount; l_pos_4++) {
		if (!gba_392[l_pos_4] && !gba_396[l_pos_4] && gba_392[l_pos_4 - 1]) {
			g_price_340 = ND(ND(gda_408[l_pos_4 - 1]) - ND(Point * gia_416[l_pos_4]));
			g_price_348 = ND(g_price_340 - ND(Point * gia_424[l_pos_4]));
			g_price_356 = ND(g_price_340 + ND(Point * gia_428[l_pos_4]));
			g_comment_364 = "Ord_" + IntStr((l_pos_4 + 1)) + "_";
			g_ticket_328 = OrderSend(Symbol(), OP_BUYLIMIT, gda_420[l_pos_4], g_price_340, Slippage, g_price_348, g_price_356, g_comment_364, Magic_N);
			
			if (g_ticket_328 == -1) {

			}
			
			else
				if (gi_324)
					Print(gs_316 + "Order BUYLIMIT " + g_ticket_328 + " set (¹ " + ((l_pos_4 + 1)) + ")");
					
			return;
		}
		
		if (!gba_400[l_pos_4] && !gba_404[l_pos_4] && gba_400[l_pos_4 - 1]) {
			g_price_340 = ND(ND(gda_412[l_pos_4 - 1]) + ND(Point * gia_416[l_pos_4]));
			g_price_348 = ND(g_price_340 + ND(Point * gia_424[l_pos_4]));
			g_price_356 = ND(g_price_340 - ND(Point * gia_428[l_pos_4]));
			g_comment_364 = "Ord_" + IntStr((l_pos_4 + 1)) + "_";
			g_ticket_328 = OrderSend(Symbol(), OP_SELLLIMIT, gda_420[l_pos_4], g_price_340, Slippage, g_price_348, g_price_356, g_comment_364, Magic_N);
			
			if (g_ticket_328 == -1) {
				
			}
			
			else
				if (gi_324)
					Print(gs_316 + "Order SELLLIMIT " + g_ticket_328 + " set (¹ " + ((l_pos_4 + 1)) + ")");
					
			return;
		}
	}
	
	for (int l_pos_4 = 2; l_pos_4 < OrdersCount; l_pos_4++) {
		if (!gba_392[l_pos_4] && !gba_396[l_pos_4] && gba_392[l_pos_4 - 2] && gba_396[l_pos_4 - 1]) {
			g_price_340 = ND(ND(gda_408[l_pos_4 - 1]) - ND(Point * gia_416[l_pos_4]));
			g_price_348 = ND(g_price_340 - ND(Point * gia_424[l_pos_4]));
			g_price_356 = ND(g_price_340 + ND(Point * gia_428[l_pos_4]));
			g_comment_364 = "Ord_" + IntStr((l_pos_4 + 1)) + "_";
			g_ticket_328 = OrderSend(Symbol(), OP_BUYLIMIT, gda_420[l_pos_4], g_price_340, Slippage, g_price_348, g_price_356, g_comment_364, Magic_N);
			
			if (g_ticket_328 == -1) {
				
				return;
			}
			
			if (gi_324)
				Print(gs_316 + "Order BUYLIMIT " + g_ticket_328 + " (" + ((l_pos_4 + 1)) + ") set (¹ " + ((l_pos_4 + 1)) + ")");
				
			return;
		}
		
		if (!gba_400[l_pos_4] && !gba_404[l_pos_4] && gba_400[l_pos_4 - 2] && gba_404[l_pos_4 - 1]) {
			g_price_340 = ND(ND(gda_412[l_pos_4 - 1]) + ND(Point * gia_416[l_pos_4]));
			g_price_348 = ND(g_price_340 + ND(Point * gia_424[l_pos_4]));
			g_price_356 = ND(g_price_340 - ND(Point * gia_428[l_pos_4]));
			g_comment_364 = "Ord_" + IntStr((l_pos_4 + 1)) + "_";
			g_ticket_328 = OrderSend(Symbol(), OP_SELLLIMIT, gda_420[l_pos_4], g_price_340, Slippage, g_price_348, g_price_356, g_comment_364, Magic_N);
			
			if (g_ticket_328 == -1) {
				
			}
			
			else
				if (gi_324)
					Print(gs_316 + "Order SELLLIMIT " + g_ticket_328 + " (" + ((l_pos_4 + 1)) + ") set (¹ " + ((l_pos_4 + 1)) + ")");
					
			return;
		}
	}
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
				if (OrderType() == OP_BUY || OrderType() == OP_BUYLIMIT) {
					if (gd_440 != 99999999.0 || gd_432 != 99999999.0) {
						g_price_348 = OrderStopLoss();
						
						if (gd_440 != 99999999.0)
							g_price_348 = gd_440;
							
						g_price_356 = OrderTakeProfit();
						
						if (OrderType() == OP_BUY)
							if (gd_432 != 99999999.0)
								g_price_356 = gd_432;
								
						if (ND(OrderStopLoss()) != ND(g_price_348) || ND(OrderTakeProfit()) != ND(g_price_356)) {
							bool succ = OrderModify(OrderTicket(), OrderOpenPrice(), ND(g_price_348), ND(g_price_356));
							
							if (!succ) {
								
							}
							
							else
								if (gi_324)
									Print(gs_316 + "Order modified");
						}
					}
				}
				
				if (OrderType() == OP_SELL || OrderType() == OP_SELLLIMIT) {
					if (gd_448 != 0.0 || gd_456 != 0.0) {
						g_price_348 = OrderStopLoss();
						
						if (gd_448 != 0.0)
							g_price_348 = gd_448;
							
						g_price_356 = OrderTakeProfit();
						
						if (OrderType() == OP_SELL)
							if (gd_456 != 0.0)
								g_price_356 = gd_456;
								
						if (ND(OrderStopLoss()) != ND(g_price_348) || ND(OrderTakeProfit()) != ND(g_price_356)) {
							bool succ = OrderModify(OrderTicket(), OrderOpenPrice(), ND(g_price_348), ND(g_price_356));
							
							if (!succ) {

							}
							
							else
								if (gi_324)
									Print(gs_316 + "Order modified");
						}
					}
				}
			}
		}
		
		else
			return;
	}
	
	for (int l_pos_4 = 0; l_pos_4 < 10; l_pos_4++)
		gba_384[l_pos_4] = 0;
		
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
				li_84 = OrderIndex(OrderComment());
				
				if (li_84 != -1) {
					if (OrderType() == OP_BUYLIMIT || OrderType() == OP_SELLLIMIT)
						gba_376[li_84] = 1;
						
					if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
						if (gba_376[li_84]) {
							Alarm(OrderTicket(), OrderType(), li_84 + 1, OrderOpenPrice(), OrderLots());
							gba_376[li_84] = 0;
						}
						
						gba_380[li_84] = 1;
						
						gba_388[li_84] = OrderTicket();
						gba_384[li_84] = 1;
					}
				}
			}
		}
		
		else
			return;
	}
	
	for (int l_pos_4 = 0; l_pos_4 < 10; l_pos_4++) {
		if (gba_380[l_pos_4] && !gba_384[l_pos_4]) {
			if (OrderSelect(gba_388[l_pos_4], SELECT_BY_TICKET, MODE_HISTORY))
				Alarm2(gba_388[l_pos_4], OrderType(), l_pos_4 + 1, OrderProfit() + OrderSwap() + OrderCommission(), OrderLots());
				
			gba_380[l_pos_4] = 0;
		}
	}
	
}

void Satellite::WH_SLTP() {
	double ld_12;
	double ld_20;
	double ld_28;
	double ld_36;
	bool l_bool_44;
	int l_error_48;
	int li_0 = Level;
	int li_4 = Level;
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
				RefreshRates();
				
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					if (StringFind(OrderComment(), "Ord_1_", 0) == 0) {
						if ((ND(OrderStopLoss()) == 0.0 && li_0 != 0) || (ND(OrderTakeProfit()) == 0.0 && li_4 != 0)) {
							if (OrderType() == OP_BUY) {
								ld_12 = ND(OrderStopLoss());
								
								if (ld_12 == 0.0) {
									if (li_0 == 0)
										ld_12 = 0;
									else {
										ld_12 = ND(OrderOpenPrice() - Point * li_0);
										ld_20 = ND(Bid - Point * MarketInfo(Symbol(), MODE_STOPLEVEL) - Point);
										ld_12 = MathMin(ld_12, ld_20);
									}
								}
								
								ld_28 = ND(OrderTakeProfit());
								
								if (ld_28 == 0.0) {
									if (li_4 == 0)
										ld_28 = 0;
									else {
										ld_28 = ND(OrderOpenPrice() + Point * li_4);
										ld_36 = ND(Bid + Point * MarketInfo(Symbol(), MODE_STOPLEVEL) + Point);
										ld_28 = MathMax(ld_28, ld_36);
									}
								}
							}
							
							if (OrderType() == OP_SELL) {
								ld_12 = ND(OrderStopLoss());
								
								if (ld_12 == 0.0) {
									if (li_0 == 0)
										ld_12 = 0;
									else {
										ld_12 = ND(OrderOpenPrice() + Point * li_0);
										ld_20 = ND(Ask + Point * MarketInfo(Symbol(), MODE_STOPLEVEL) + Point);
										ld_12 = MathMax(ld_12, ld_20);
									}
								}
								
								ld_28 = ND(OrderTakeProfit());
								
								if (ld_28 == 0.0) {
									if (li_4 == 0)
										ld_28 = 0;
									else {
										ld_28 = ND(OrderOpenPrice() - Point * li_4);
										ld_36 = ND(Ask - Point * MarketInfo(Symbol(), MODE_STOPLEVEL) - Point);
										ld_28 = MathMin(ld_28, ld_36);
									}
								}
							}
							
							if (OrderStopLoss() != ld_12 || OrderTakeProfit() != ld_28) {
								l_bool_44 = OrderModify(OrderTicket(), OrderOpenPrice(), ld_12, ld_28);
							}
						}
					}
				}
			}
		}
	}
}

void Satellite::Alarm(int ai_0, int ai_4, int ai_8, double ad_12, double ad_20) {
	
}

void Satellite::Alarm2(int ai_0, int ai_4, int ai_8, double ad_12, double ad_20) {
	
}

double Satellite::ND(double ad_0) {
	return (NormalizeDouble(ad_0, Digits));
}

String Satellite::TimeFrameName(int ai_0) {
	return "";
}

int Satellite::OrderIndex(String as_0) {
	if (StringFind(as_0, "Ord_1_", 0) == 0)
		return (0);
		
	if (StringFind(as_0, "Ord_2_", 0) == 0)
		return (1);
		
	if (StringFind(as_0, "Ord_3_", 0) == 0)
		return (2);
		
	if (StringFind(as_0, "Ord_4_", 0) == 0)
		return (3);
		
	if (StringFind(as_0, "Ord_5_", 0) == 0)
		return (4);
		
	if (StringFind(as_0, "Ord_6_", 0) == 0)
		return (5);
		
	if (StringFind(as_0, "Ord_7_", 0) == 0)
		return (6);
		
	if (StringFind(as_0, "Ord_8_", 0) == 0)
		return (7);
		
	if (StringFind(as_0, "Ord_9_", 0) == 0)
		return (8);
		
	if (StringFind(as_0, "Ord_10_", 0) == 0)
		return (9);
		
	return (-1);
}

void Satellite::AccountAndSymbolLbls() {
	
}

String Satellite::MarketType(String as_0) {
	String ls_12;
	int l_str_len_8 = StringLen(as_0);
	
	if (StringSubstr(as_0, 0, 1) == "_")
		return ("Indexes");
		
	if (StringSubstr(as_0, 0, 1) == "#") {
		ls_12 = StringSubstr(as_0, l_str_len_8 - 1, 1);
		
		if (ls_12 == "0")
			return ("Futures");
			
		if (ls_12 == "1")
			return ("Futures");
			
		if (ls_12 == "2")
			return ("Futures");
			
		if (ls_12 == "3")
			return ("Futures");
			
		if (ls_12 == "4")
			return ("Futures");
			
		if (ls_12 == "5")
			return ("Futures");
			
		if (ls_12 == "6")
			return ("Futures");
			
		if (ls_12 == "7")
			return ("Futures");
			
		if (ls_12 == "8")
			return ("Futures");
			
		if (ls_12 == "9")
			return ("Futures");
			
		return ("CFD");
	}
	
	if (as_0 == "GOLD")
		return ("Metalls");
		
	if (as_0 == "SILVER")
		return ("Metalls");
		
	if (l_str_len_8 == 6) {
		ls_12 = StringSubstr(as_0, 0, 3);
		
		if (ls_12 == "AUD")
			return ("Forex");
			
		if (ls_12 == "CAD")
			return ("Forex");
			
		if (ls_12 == "CHF")
			return ("Forex");
			
		if (ls_12 == "EUR")
			return ("Forex");
			
		if (ls_12 == "GBP")
			return ("Forex");
			
		if (ls_12 == "LFX")
			return ("Forex");
			
		if (ls_12 == "NZD")
			return ("Forex");
			
		if (ls_12 == "SGD")
			return ("Forex");
			
		if (ls_12 == "USD")
			return ("Forex");
	}
	
	return ("");
}

double Satellite::FuturesLotMargin(String as_0) {
	double ld_ret_8 = 0;
	String ls_16 = AccountCurrency();
	double l_bid_24 = 1;
	
	if (ls_16 != "USD")
		l_bid_24 = MarketInfo(ls_16 + "USD", MODE_BID);
		
	int l_str_len_32 = StringLen(as_0);
	
	String ls_36 = StringSubstr(as_0, 0, l_str_len_32 - 2);
	
	if (ls_36 == "#ENQ")
		ld_ret_8 = 3750.0 * l_bid_24;
		
	if (ls_36 == "#EP")
		ld_ret_8 = 3938.0 * l_bid_24;
		
	if (ls_36 == "#SLV")
		ld_ret_8 = 5063.0 * l_bid_24;
		
	if (ls_36 == "#GOLD")
		ld_ret_8 = 2363.0 * l_bid_24;
		
	if (ls_36 == "#CL")
		ld_ret_8 = 4725.0 * l_bid_24;
		
	if (ls_36 == "#NG")
		ld_ret_8 = 8100.0 * l_bid_24;
		
	if (ls_36 == "#W")
		ld_ret_8 = 608.0 * l_bid_24;
		
	if (ls_36 == "#S")
		ld_ret_8 = 1148.0 * l_bid_24;
		
	if (ls_36 == "#C")
		ld_ret_8 = 473.0 * l_bid_24;
		
	return (ld_ret_8);
}

void Satellite::fObjDeleteByPrefix(String as_0) {
	
}

void Satellite::fObjLabel(String a_name_0, int a_x_8, int a_y_12, String a_text_16, int a_corner_24, int a_fontsize_32, int a_window_36, String a_fontname_40, bool a_bool_48) {
	
}

int Satellite::fOrderOpenBuy(double a_lots_0) {
	int l_ticket_8;
	RefreshRates();
	
	if (!IsTradeContextBusy()) {
		l_ticket_8 = OrderSend(Symbol(), OP_BUY, a_lots_0, ND(Ask), Slippage, 0, 0, 0, Magic_N + 1);
		
		if (l_ticket_8 > 0)
			return (l_ticket_8);
			
		
		return (-1);
	}
	
	if (TimeCurrent() > g_datetime_488 + 20) {
		g_datetime_488 = TimeCurrent();
		Print("Need open buy. Trade Context Busy");
	}
	
	return (-2);
}

int Satellite::fOrderOpenSell(double a_lots_0) {
	int l_ticket_8;
	RefreshRates();
	
	if (!IsTradeContextBusy()) {
		l_ticket_8 = OrderSend(Symbol(), OP_SELL, a_lots_0, ND(Bid), Slippage, 0, 0, 0, Magic_N + 1);
		
		if (l_ticket_8 > 0)
			return (l_ticket_8);
			
		
		return (-1);
	}
	
	if (TimeCurrent() > g_datetime_492 + 20) {
		g_datetime_492 = TimeCurrent();
		Print("Need open buy. Trade Context Busy");
	}
	
	return (-2);
}

String Satellite::fMyErDesc(int ai_0) {
	String ls_4 = "Err Num: " + IntStr(ai_0) + " - ";
	
	switch (ai_0) {
	
	case 0:
		return (ls_4 + "NO ERROR");
		
	case 1:
		return (ls_4 + "NO RESULT");
		
	case 2:
		return (ls_4 + "COMMON ERROR");
		
	case 3:
		return (ls_4 + "INVALID TRADE PARAMETERS");
		
	case 4:
		return (ls_4 + "SERVER BUSY");
		
	case 5:
		return (ls_4 + "OLD VERSION");
		
	case 6:
		return (ls_4 + "NO CONNECTION");
		
	case 7:
		return (ls_4 + "NOT ENOUGH RIGHTS");
		
	case 8:
		return (ls_4 + "TOO FREQUENT REQUESTS");
		
	case 9:
		return (ls_4 + "MALFUNCTIONAL TRADE");
		
	case 64:
		return (ls_4 + "ACCOUNT DISABLED");
		
	case 65:
		return (ls_4 + "INVALID ACCOUNT");
		
	case 128:
		return (ls_4 + "TRADE TIMEOUT");
		
	case 129:
		return (ls_4 + "INVALID PRICE");
		
	case 130:
		return (ls_4 + "INVALID STOPS");
		
	case 131:
		return (ls_4 + "INVALID TRADE VOLUME");
		
	case 132:
		return (ls_4 + "MARKET CLOSED");
		
	case 133:
		return (ls_4 + "TRADE DISABLED");
		
	case 134:
		return (ls_4 + "NOT ENOUGH MONEY");
		
	case 135:
		return (ls_4 + "PRICE CHANGED");
		
	case 136:
		return (ls_4 + "OFF QUOTES");
		
	case 137:
		return (ls_4 + "BROKER BUSY");
		
	case 138:
		return (ls_4 + "REQUOTE");
		
	case 139:
		return (ls_4 + "ORDER LOCKED");
		
	case 140:
		return (ls_4 + "LONG POSITIONS ONLY ALLOWED");
		
	case 141:
		return (ls_4 + "TOO MANY REQUESTS");
		
	case 145:
		return (ls_4 + "TRADE MODIFY DENIED");
		
	case 146:
		return (ls_4 + "TRADE CONTEXT BUSY");
		
	case 147:
		return (ls_4 + "TRADE EXPIRATION DENIED");
		
	case 148:
		return (ls_4 + "TRADE TOO MANY ORDERS");
		
	case 4000:
		return (ls_4 + "NO MQLERROR");
		
	case 4001:
		return (ls_4 + "WRONG FUNCTION POINTER");
		
	case 4002:
		return (ls_4 + "ARRAY INDEX OUT OF RANGE");
		
	case 4003:
		return (ls_4 + "NO MEMORY FOR FUNCTION CALL STACK");
		
	case 4004:
		return (ls_4 + "RECURSIVE STACK OVERFLOW");
		
	case 4005:
		return (ls_4 + "NOT ENOUGH STACK FOR PARAMETER");
		
	case 4006:
		return (ls_4 + "NO MEMORY FOR PARAMETER STRING");
		
	case 4007:
		return (ls_4 + "NO MEMORY FOR TEMP STRING");
		
	case 4008:
		return (ls_4 + "NOT INITIALIZED STRING");
		
	case 4009:
		return (ls_4 + "NOT INITIALIZED ARRAYSTRING");
		
	case 4010:
		return (ls_4 + "NO MEMORY FOR ARRAYSTRING");
		
	case 4011:
		return (ls_4 + "TOO LONG STRING");
		
	case 4012:
		return (ls_4 + "REMAINDER FROM ZERO DIVIDE");
		
	case 4013:
		return (ls_4 + "ZERO DIVIDE");
		
	case 4014:
		return (ls_4 + "UNKNOWN COMMAND");
		
	case 4015:
		return (ls_4 + "WRONG JUMP");
		
	case 4016:
		return (ls_4 + "NOT INITIALIZED ARRAY");
		
	case 4017:
		return (ls_4 + "DLL CALLS NOT ALLOWED");
		
	case 4018:
		return (ls_4 + "CANNOT LOAD LIBRARY");
		
	case 4019:
		return (ls_4 + "CANNOT CALL FUNCTION");
		
	case 4020:
		return (ls_4 + "EXTERNAL EXPERT CALLS NOT ALLOWED");
		
	case 4021:
		return (ls_4 + "NOT ENOUGH MEMORY FOR RETURNED STRING");
		
	case 4022:
		return (ls_4 + "SYSTEM BUSY");
		
	case 4050:
		return (ls_4 + "INVALID FUNCTION PARAMETERS COUNT");
		
	case 4051:
		return (ls_4 + "INVALID FUNCTION PARAMETER VALUE");
		
	case 4052:
		return (ls_4 + "STRING FUNCTION INTERNAL ERROR");
		
	case 4053:
		return (ls_4 + "SOME ARRAY ERROR");
		
	case 4054:
		return (ls_4 + "INCORRECT SERIES ARRAY USING");
		
	case 4055:
		return (ls_4 + "CUSTOM INDICATOR ERROR");
		
	case 4056:
		return (ls_4 + "INCOMPATIBLE ARRAYS");
		
	case 4057:
		return (ls_4 + "GLOBAL VARIABLES PROCESSING ERROR");
		
	case 4058:
		return (ls_4 + "GLOBAL VARIABLE NOT FOUND");
		
	case 4059:
		return (ls_4 + "FUNCTION NOT ALLOWED IN TESTING MODE");
		
	case 4060:
		return (ls_4 + "FUNCTION NOT CONFIRMED");
		
	case 4061:
		return (ls_4 + "SEND MAIL ERROR");
		
	case 4062:
		return (ls_4 + "STRING PARAMETER EXPECTED");
		
	case 4063:
		return (ls_4 + "INTEGER PARAMETER EXPECTED");
		
	case 4064:
		return (ls_4 + "DOUBLE PARAMETER EXPECTED");
		
	case 4065:
		return (ls_4 + "ARRAY AS PARAMETER EXPECTED");
		
	case 4066:
		return (ls_4 + "HISTORY WILL UPDATED");
		
	case 4067:
		return (ls_4 + "TRADE ERROR");
		
	case 4099:
		return (ls_4 + "END OF FILE");
		
	case 4100:
		return (ls_4 + "SOME FILE ERROR");
		
	case 4101:
		return (ls_4 + "WRONG FILE NAME");
		
	case 4102:
		return (ls_4 + "TOO MANY OPENED FILES");
		
	case 4103:
		return (ls_4 + "CANNOT OPEN FILE");
		
	case 4104:
		return (ls_4 + "INCOMPATIBLE ACCESS TO FILE");
		
	case 4105:
		return (ls_4 + "NO ORDER SELECTED");
		
	case 4106:
		return (ls_4 + "UNKNOWN SYMBOL");
		
	case 4107:
		return (ls_4 + "INVALID PRICE PARAM");
		
	case 4108:
		return (ls_4 + "INVALID TICKET");
		
	case 4109:
		return (ls_4 + "TRADE NOT ALLOWED");
		
	case 4110:
		return (ls_4 + "LONGS  NOT ALLOWED");
		
	case 4111:
		return (ls_4 + "SHORTS NOT ALLOWED");
		
	case 4200:
		return (ls_4 + "OBJECT ALREADY EXISTS");
		
	case 4201:
		return (ls_4 + "UNKNOWN OBJECT PROPERTY");
		
	case 4202:
		return (ls_4 + "OBJECT DOES NOT EXIST");
		
	case 4203:
		return (ls_4 + "UNKNOWN OBJECT TYPE");
		
	case 4204:
		return (ls_4 + "NO OBJECT NAME");
		
	case 4205:
		return (ls_4 + "OBJECT COORDINATES ERROR");
		
	case 4206:
		return (ls_4 + "NO SPECIFIED SUBWINDOW");
		
	case 4207:
		return (ls_4 + "SOME OBJECT ERROR");
	}
	
	return (ls_4 + "WRONG ERR NUM");
}

int Satellite::fOrderDeleteLimitModSLTP(bool ai_0, bool ai_4) {
	int l_error_16;
	int li_ret_8 = 0;
	
	for (int l_pos_12 = OrdersTotal() - 1; l_pos_12 >= 0; l_pos_12--) {
		if (OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderMagicNumber() == Magic_N) {
					if (ai_0) {
						if (OrderType() == OP_BUYLIMIT) {
							RefreshRates();
							
							if (!IsTradeContextBusy()) {
								if (!OrderDelete(OrderTicket())) {
									li_ret_8 = -1;
								}
							}
							
							else {
								if (TimeCurrent() > g_datetime_496 + 20) {
									g_datetime_496 = TimeCurrent();
								}
								
								return (-2);
							}
						}
						
						else if (OrderType() == OP_BUY)
							if (ND(OrderStopLoss()) != 0.0 || ND(OrderTakeProfit()) != 0.0)
								fModifyOrder(OrderTicket(), OrderOpenPrice(), 0, 0, 0, "Lock");
					}
					
					if (ai_4) {
						if (OrderType() == OP_SELLLIMIT) {
							RefreshRates();
							
							if (!IsTradeContextBusy()) {
								if (!OrderDelete(OrderTicket())) {
									li_ret_8 = -1;
								}
							}
							
							else {
								if (TimeCurrent() > g_datetime_500 + 20) {
									g_datetime_500 = TimeCurrent();
								}
								
								return (-2);
							}
						}
						
						else if (OrderType() == OP_SELL)
							if (ND(OrderStopLoss()) != 0.0 || ND(OrderTakeProfit()) != 0.0)
								fModifyOrder(OrderTicket(), OrderOpenPrice(), 0, 0, 0, "Lock");
					}
				}
			}
		}
	}
	
	return (li_ret_8);
}

int Satellite::fModifyOrder(int a_ticket_0, double ad_4, double ad_12, double ad_20, int a_datetime_28, String as_32) {
	int l_error_40;
	
	if (!IsTradeContextBusy()) {
		if (!OrderModify(a_ticket_0, ND(ad_4), ND(ad_12), ND(ad_20))) {
			return (-1);
		}
		
		return (0);
	}
	
	if (g_datetime_504 != Now) {
		g_datetime_504 = Now;
	}
	
	return (-2);
}

int Satellite::fLock() {
	int l_count_0;
	int l_count_4;
	double ld_8;
	double ld_16;
	double ld_24;
	double ld_32;
	double ld_52;
	double l_maxlot_60;
	double l_minlot_68;
	
	if (Lock > 0) {
		l_count_0 = 0;
		l_count_4 = 0;
		ld_8 = 0;
		ld_16 = 0;
		ld_24 = 0;
		ld_32 = 0;
		
		for (int l_pos_40 = 0; l_pos_40 < OrdersTotal(); l_pos_40++) {
			if (OrderSelect(l_pos_40, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() == Symbol()) {
					if (OrderMagicNumber() == Magic_N) {
						switch (OrderType()) {
						
						case OP_BUY:
							l_count_0++;
							ld_8 += OrderLots();
							break;
							
						case OP_SELL:
							l_count_4++;
							ld_16 += OrderLots();
						}
					}
					
					if (OrderMagicNumber() == Magic_N + 1) {
						switch (OrderType()) {
						
						case OP_BUY:
							ld_24 += OrderLots();
							break;
							
						case OP_SELL:
							ld_32 += OrderLots();
						}
					}
				}
			}
			
			else
				return (0);
		}
		
		if (l_count_0 >= Lock) {
			fOrderDeleteLimitModSLTP(1, 0);
			
			if (ld_8 > ld_32) {
				ld_52 = ND2(ld_8 - ld_32);
				l_maxlot_60 = ld_52;
				
				if (l_maxlot_60 > MarketInfo(Symbol(), MODE_MAXLOT)) {
					l_maxlot_60 = MarketInfo(Symbol(), MODE_MAXLOT);
					l_minlot_68 = ND2(ld_52 - l_maxlot_60);
					
					if (l_minlot_68 != 0.0) {
						if (l_minlot_68 < MarketInfo(Symbol(), MODE_MINLOT)) {
							l_minlot_68 = MarketInfo(Symbol(), MODE_MINLOT);
							l_maxlot_60 = ND2(ld_52 - l_minlot_68);
						}
					}
				}
				
				if (l_maxlot_60 >= MarketInfo(Symbol(), MODE_MINLOT))
					fOrderOpenSell(l_maxlot_60);
			}
			
			return (0);
		}
		
		if (l_count_4 >= Lock) {
			fOrderDeleteLimitModSLTP(0, 1);
			
			if (ld_16 > ld_24) {
				ld_52 = ND2(ld_16 - ld_24);
				l_maxlot_60 = ld_52;
				
				if (l_maxlot_60 > MarketInfo(Symbol(), MODE_MAXLOT)) {
					l_maxlot_60 = MarketInfo(Symbol(), MODE_MAXLOT);
					l_minlot_68 = ND2(ld_52 - l_maxlot_60);
					
					if (l_minlot_68 != 0.0) {
						if (l_minlot_68 < MarketInfo(Symbol(), MODE_MINLOT)) {
							l_minlot_68 = MarketInfo(Symbol(), MODE_MINLOT);
							l_maxlot_60 = ND2(ld_52 - l_minlot_68);
						}
					}
				}
				
				if (l_maxlot_60 >= MarketInfo(Symbol(), MODE_MINLOT))
					fOrderOpenBuy(l_maxlot_60);
			}
			
			return (0);
		}
	}
	
	return (1);
}

double Satellite::ND2(double ad_0) {
	return (NormalizeDouble(ad_0, 2));
}

}
