#include "Overlook.h"


namespace Overlook {

Mari::Mari() {
	
}

void Mari::InitEA() {
	bool li_0;
	gia_548[0] = 0;
	gia_548[1] = Ord_2_Level;
	gia_548[2] = Ord_3_Level;
	gia_548[3] = Ord_4_Level;
	gia_548[4] = Ord_5_Level;
	gia_548[5] = Ord_6_Level;
	gia_548[6] = Ord_7_Level;
	gia_548[7] = Ord_8_Level;
	gia_548[8] = Ord_9_Level;
	gia_548[9] = Ord_10_Level;
	gda_552[0] = Ord_1_Lots;
	gda_552[1] = Ord_2_Lots;
	gda_552[2] = Ord_3_Lots;
	gda_552[3] = Ord_4_Lots;
	gda_552[4] = Ord_5_Lots;
	gda_552[5] = Ord_6_Lots;
	gda_552[6] = Ord_7_Lots;
	gda_552[7] = Ord_8_Lots;
	gda_552[8] = Ord_9_Lots;
	gda_552[9] = Ord_10_Lots;
	gia_556[0] = Ord_1_StopLoss;
	gia_556[1] = Ord_2_StopLoss;
	gia_556[2] = Ord_3_StopLoss;
	gia_556[3] = Ord_4_StopLoss;
	gia_556[4] = Ord_5_StopLoss;
	gia_556[5] = Ord_6_StopLoss;
	gia_556[6] = Ord_7_StopLoss;
	gia_556[7] = Ord_8_StopLoss;
	gia_556[8] = Ord_9_StopLoss;
	gia_556[9] = Ord_10_StopLoss;
	gia_560[0] = Ord_1_TakeProfit;
	gia_560[1] = Ord_2_TakeProfit;
	gia_560[2] = Ord_3_TakeProfit;
	gia_560[3] = Ord_4_TakeProfit;
	gia_560[4] = Ord_5_TakeProfit;
	gia_560[5] = Ord_6_TakeProfit;
	gia_560[6] = Ord_7_TakeProfit;
	gia_560[7] = Ord_8_TakeProfit;
	gia_560[8] = Ord_9_TakeProfit;
	gia_560[9] = Ord_10_TakeProfit;
	
	AddSubCore<Channel>().
		Set("period", ch_period);
}

void Mari::StartEA(int pos) {
	bool li_0;
	bool li_52;
	int l_count_56;
	int li_60;
	int i = 0;
	
	if (InfoOn)
		AccountAndSymbolLbls();
	
	if (!pos)
		ltt = Time(1970,1,1);
	
	int li_8 = 60 * end_hour0 / Period();
	
	double high = At(0).GetBuffer(1).Get(pos);
	
	double low = At(0).GetBuffer(0).Get(pos);
	
	bool li_28 = false;
	
	bool li_32 = false;
	
	double o0 = GetInputBuffer(0,0).Get(pos);
	if (ND(o0) <= ND(high - Point * point_factor0))
		li_28 = true;
		
	if (ND(o0) >= ND(low + Point * point_factor0))
		li_32 = true;
		
	bool li_36 = false;
	
	bool li_40 = false;
	
	bool li_44 = false;
	
	bool li_48 = false;
	
	for (i = 0; i < OrdersTotal(); i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
				if (OrderType() == OP_BUY)
					li_36 = true;
					
				if (OrderType() == OP_SELL)
					li_40 = true;
					
				if (OrderType() == OP_BUYLIMIT)
					li_44 = true;
					
				if (OrderType() == OP_SELLLIMIT)
					li_48 = true;
			}
		}
		
		else
			return;
	}
	
	if ((!li_36 && li_44) || (!li_40 && li_48)) {
		li_52 = false;
		
		for (i = OrdersTotal() - 1; i >= 0; i--) {
			if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
					if (OrderType() == OP_BUYLIMIT && !li_36) {
						bool succ = OrderDelete(OrderTicket());
						
						if (!succ) {
							if (verbose)
								Print("Can not delete BUYLIMIT");
								
							li_52 = true;
						}
						
						else
							if (verbose)
								Print("Order BUYLIMIT");
					}
					
					else if (OrderType() == OP_SELLLIMIT && !li_40) {
						bool succ = OrderDelete(OrderTicket());
						
						if (!succ) {
							if (verbose)
								Print("Can not delete SELLLIMIT");
								
							li_52 = true;
						}
						
						else
							if (verbose)
								Print("Order SELLLIMIT");
					}
				}
			}
			
			else
				return;
		}
		
		if (li_52)
			return;
	}
	
	if (li_28 || li_32 && CanOpenNew) {
		l_count_56 = 0;
		
		for (i = OrdersTotal() - 1; i >= 0; i--) {
			if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N)
					l_count_56++;
			}
			
			else
				return;
		}
		
		if (l_count_56 == 0) {
			if (li_28 && Now > ltt) {
				RefreshRates();
				g_ticket_460 = OrderSend(Symbol(), OP_BUY, Ord_1_Lots, ND(Ask), Slippage, ND(ND(Ask) - ND(Point * Ord_1_StopLoss)), ND(ND(Ask) + ND(Point * Ord_1_TakeProfit)), "Ord_1_", Magic_N);
				
				if (g_ticket_460 == -1) {
					if (verbose)
						Print("Can not open BUY.");
						
					return;
				}
				
				if (verbose)
					Print("Order BUY");
					
				ltt = TimeCurrent();
				
				alarm_ask = Ask;
				
				if (OrderSelect(g_ticket_460, SELECT_BY_TICKET, MODE_TRADES))
					alarm_ask = OrderOpenPrice();
					
			}
			
			if (li_32 && Now > ltt) {
				RefreshRates();
				g_ticket_460 = OrderSend(Symbol(), OP_SELL, Ord_1_Lots, ND(Bid), Slippage, ND(ND(Bid) + ND(Point * Ord_1_StopLoss)), ND(ND(Bid) - ND(Point * Ord_1_TakeProfit)), "Ord_1_", Magic_N);
				
				if (g_ticket_460 == -1) {
					if (verbose)
						Print("Can not open SELL.");
						
					return;
				}
				
				if (verbose)
					Print("Order SELL " + IntStr(g_ticket_460) + " opened (1)");
					
				ltt = TimeCurrent();
				
				alarm_ask = Bid;
				
				if (OrderSelect(g_ticket_460, SELECT_BY_TICKET, MODE_TRADES))
					alarm_ask = OrderOpenPrice();
					
			}
		}
	}
	
	for (i = 0; i < OrdersCount; i++) {
		gba_524[i] = 0;
		gba_532[i] = 0;
		gba_528[i] = 0;
		gba_536[i] = 0;
	}
	
	min_tp = 99999999;
	
	min_sl = 99999999;
	max_sl = 0;
	max_tp = 0;
	
	for (i = 0; i < OrdersTotal(); i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
				li_60 = OrderIndex(OrderComment());
				
				if (li_60 != -1) {
					if (OrderType() == OP_BUY || OrderType() == OP_BUYLIMIT) {
						if (OrderType() == OP_BUY) {
							gba_524[li_60] = 1;
							min_tp = MathMin(ND(min_tp), ND(OrderTakeProfit()));
						}
						
						if (OrderType() == OP_BUYLIMIT)
							gba_528[li_60] = 1;
							
						gda_540[li_60] = OrderOpenPrice();
						
						min_sl = MathMin(ND(min_sl), ND(OrderStopLoss()));
					}
					
					if (OrderType() == OP_SELL || OrderType() == OP_SELLLIMIT) {
						if (OrderType() == OP_SELL) {
							gba_532[li_60] = 1;
							max_tp = MathMax(ND(max_tp), ND(OrderTakeProfit()));
						}
						
						if (OrderType() == OP_SELLLIMIT)
							gba_536[li_60] = 1;
							
						gda_544[li_60] = OrderOpenPrice();
						
						max_sl = MathMax(ND(max_sl), ND(OrderStopLoss()));
					}
				}
			}
		}
		
		else
			return;
	}
	
	for (i = 1; i < OrdersCount; i++) {
		if (!gba_524[i] && !gba_528[i] && gba_524[i - 1]) {
			g_price_472 = ND(ND(gda_540[i - 1]) - ND(Point * gia_548[i]));
			g_price_480 = ND(g_price_472 - ND(Point * gia_556[i]));
			g_price_488 = ND(g_price_472 + ND(Point * gia_560[i]));
			g_comment_496 = "Ord_" + IntStr((i + 1)) + "_";
			g_ticket_460 = OrderSend(Symbol(), OP_BUYLIMIT, gda_552[i], g_price_472, Slippage, g_price_480, g_price_488, g_comment_496, Magic_N);
			
			if (g_ticket_460 != -1) {
				if (verbose)
					Print("Can not set BUYLIMIT.");
			}
			
			else
				if (verbose)
					Print("Order BUYLIMIT");
					
			return;
		}
		
		if (!gba_532[i] && !gba_536[i] && gba_532[i - 1]) {
			g_price_472 = ND(ND(gda_544[i - 1]) + ND(Point * gia_548[i]));
			g_price_480 = ND(g_price_472 + ND(Point * gia_556[i]));
			g_price_488 = ND(g_price_472 - ND(Point * gia_560[i]));
			g_comment_496 = "Ord_" + IntStr((i + 1)) + "_";
			g_ticket_460 = OrderSend(Symbol(), OP_SELLLIMIT, gda_552[i], g_price_472, Slippage, g_price_480, g_price_488, g_comment_496, Magic_N);
			
			if (g_ticket_460 == -1) {
				if (verbose)
					Print("Can not set SELLLIMIT.");
			}
			
			else
				if (verbose)
					Print("Order SELLLIMIT");
					
			return;
		}
	}
	
	for (i = 2; i < OrdersCount; i++) {
		if (!gba_524[i] && !gba_528[i] && gba_524[i - 2] && gba_528[i - 1]) {
			g_price_472 = ND(ND(gda_540[i - 1]) - ND(Point * gia_548[i]));
			g_price_480 = ND(g_price_472 - ND(Point * gia_556[i]));
			g_price_488 = ND(g_price_472 + ND(Point * gia_560[i]));
			g_comment_496 = "Ord_" + IntStr((i + 1)) + "_";
			g_ticket_460 = OrderSend(Symbol(), OP_BUYLIMIT, gda_552[i], g_price_472, Slippage, g_price_480, g_price_488, g_comment_496, Magic_N);
			
			if (g_ticket_460 == -1) {
				if (verbose)
					Print("Can not set BUYLIMIT");
					
				return;
			}
			
			if (verbose)
				Print("Order BUYLIMIT");
				
			return;
		}
		
		if (!gba_532[i] && !gba_536[i] && gba_532[i - 2] && gba_536[i - 1]) {
			g_price_472 = ND(ND(gda_544[i - 1]) + ND(Point * gia_548[i]));
			g_price_480 = ND(g_price_472 + ND(Point * gia_556[i]));
			g_price_488 = ND(g_price_472 - ND(Point * gia_560[i]));
			g_comment_496 = "Ord_" + IntStr((i + 1)) + "_";
			g_ticket_460 = OrderSend(Symbol(), OP_SELLLIMIT, gda_552[i], g_price_472, Slippage, g_price_480, g_price_488, g_comment_496, Magic_N);
			
			if (g_ticket_460 == -1) {
				if (verbose)
					Print("Can not set SELLLIMIT");
			}
			
			else
				if (verbose)
					Print("Order SELLLIMIT");
					
			return;
		}
	}
	
	for (i = 0; i < OrdersTotal(); i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
				if (OrderType() == OP_BUY || OrderType() == OP_BUYLIMIT) {
					if (min_sl != 99999999.0 || min_tp != 99999999.0) {
						g_price_480 = OrderStopLoss();
						
						if (min_sl != 99999999.0)
							g_price_480 = min_sl;
							
						g_price_488 = OrderTakeProfit();
						
						if (OrderType() == OP_BUY)
							if (min_tp != 99999999.0)
								g_price_488 = min_tp;
								
						if (ND(OrderStopLoss()) != ND(g_price_480) || ND(OrderTakeProfit()) != ND(g_price_488)) {
							bool succ = OrderModify(OrderTicket(), OrderOpenPrice(), ND(g_price_480), ND(g_price_488));
							
							if (!succ) {
								if (verbose)
									Print("Can not modify the order");
							}
							
							else
								if (verbose)
									Print("Order modified");
						}
					}
				}
				
				else if (OrderType() == OP_SELL || OrderType() == OP_SELLLIMIT) {
					if (max_sl != 0.0 || max_tp != 0.0) {
						g_price_480 = OrderStopLoss();
						
						if (max_sl != 0.0)
							g_price_480 = max_sl;
							
						g_price_488 = OrderTakeProfit();
						
						if (OrderType() == OP_SELL)
							if (max_tp != 0.0)
								g_price_488 = max_tp;
								
						if (ND(OrderStopLoss()) != ND(g_price_480) || ND(OrderTakeProfit()) != ND(g_price_488)) {
							bool succ = OrderModify(OrderTicket(), OrderOpenPrice(), ND(g_price_480), ND(g_price_488));
							
							if (!succ) {
								if (verbose)
									Print("Can not modify the order");
							}
							
							else
								if (verbose)
									Print("Order modified");
						}
					}
				}
			}
		}
		
		else
			return;
	}
	
	for (i = 0; i < 10; i++)
		gba_516[i] = 0;
		
	for (i = 0; i < OrdersTotal(); i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_N) {
				li_60 = OrderIndex(OrderComment());
				
				if (li_60 != -1) {
					if (OrderType() == OP_BUYLIMIT || OrderType() == OP_SELLLIMIT)
						gba_508[li_60] = 1;
						
					if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
						if (gba_508[li_60]) {
							gba_508[li_60] = 0;
						}
						
						gba_512[li_60] = 1;
						
						gba_520[li_60] = OrderTicket();
						gba_516[li_60] = 1;
					}
				}
			}
		}
		
		else
			return;
	}
	
	for (i = 0; i < 10; i++) {
		if (gba_512[i] && !gba_516[i]) {
			if (OrderSelect(gba_520[i], SELECT_BY_TICKET, MODE_HISTORY))
				Alarm2(gba_520[i], OrderType(), i + 1, OrderProfit() + OrderSwap() + OrderCommission(), OrderLots());
				
			gba_512[i] = 0;
		}
	}
}


void Mari::Alarm2(int ai_0, int ai_4, int ai_8, double ad_12, double ad_20) {
	
}

double Mari::ND(double d) {
	return (NormalizeDouble(d, Digits));
}

int Mari::OrderIndex(String as_0) {
	int i = as_0.Find("Ord_");
	ASSERT(i != -1);
	i += 4;
	int j = as_0.Find("_", i);
	ASSERT(j != -1);
	String num_str = as_0.Mid(i, j-i);
	ASSERT(num_str.GetCount());
	return StrInt(num_str) - 1;
}

void Mari::AccountAndSymbolLbls() {
	
}

String Mari::MarketType(String as_0) {
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

double Mari::FuturesLotMargin(String as_0) {
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


}

