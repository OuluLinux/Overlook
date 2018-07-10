#include "Overlook.h"

#if 0

// topgun

namespace Overlook {

Hornet::Hornet() {

}

void Hornet::InitEA() {
	gi_unused_248 = 1;
	
	if (IsTesting() == true)
		gi_unused_248 = 1;
		
	string ls_0 = "EA Name:" + Program
				  + "\n";
	              
	string ls_8 = "Rev:" + Rev
				  + "\n";
	              
	Comment("Local Time:" + TimeToStr(TimeLocal(), TIME_DATE) + " " + TimeToStr(TimeLocal(), TIME_SECONDS), " System Run",
			"\n", "--Copyright----------------------------------------------------------------------\n", ls_0, ls_8, "--------------------------------------------------------------------------------------");
	        
	if (Aggressive) {
		gi_252 = Nanpin1;
		gi_256 = Nanpin2;
	}
	
	return (0);
}

void Hornet::StartEA(int pos) {
	HideTestIndicators(true);
	
	if (UseAutoLot)
		Lots = LotsOptimized();
		
	double ld_0 = 0;
	
	double ld_8 = NormalizeDouble(Ask, Digits);
	
	double ld_16 = NormalizeDouble(Bid, Digits);
	
	double maxlot_24 = MarketInfo(Symbol(), MODE_MAXLOT);
	
	double ibands_32 = iBands(NULL, 0, 16, 2, 0, PRICE_MEDIAN, MODE_UPPER, 3);
	
	double ibands_40 = iBands(NULL, 0, 16, 2, 0, PRICE_MEDIAN, MODE_LOWER, 3);
	
	double ld_48 = MathAbs(ibands_32 - ibands_40);
	
	if (FTLCurrentOrders(0, MAGIC1) > 0.0 && FTLCurrentOrders(0, MAGIC2) == 0.0) {
		if (ld_16 > FTLOrderOpenPrice(MAGIC1) + 10.0 * Point) {
			while (ld_16 < FTLOrderClose(Slippage, MAGIC1)) {
			}
		}
	} else {
	
		if (FTLCurrentOrders(1, MAGIC1) < 0.0 && FTLCurrentOrders(1, MAGIC2) == 0.0) {
			if (ld_8 < FTLOrderOpenPrice(MAGIC1) - 10.0 * Point) {
				while (ld_8 < FTLOrderClose(Slippage, MAGIC1)) {
				}
			}
		}
	}
	
	if (FTLCurrentOrders(0, MAGIC2) > 0.0 && FTLCurrentOrders(0, MAGIC3) == 0.0) {
		if (ld_16 > FTLOrderOpenPrice(MAGIC2) + 15.0 * Point) {
			while (ld_16 < FTLOrderClose(Slippage, MAGIC2)) {
			}
			
			while (ld_16 < FTLOrderClose(Slippage, MAGIC1)) {
			}
		}
	} else {
	
		if (FTLCurrentOrders(1, MAGIC2) < 0.0 && FTLCurrentOrders(1, MAGIC3) == 0.0) {
			if (ld_8 < FTLOrderOpenPrice(MAGIC2) - 15.0 * Point) {
				while (ld_8 < FTLOrderClose(Slippage, MAGIC2)) {
				}
				
				while (ld_8 < FTLOrderClose(Slippage, MAGIC1)) {
				}
			}
		}
	}
	
	if (FTLCurrentOrders(0, MAGIC3) > 0.0) {
		if (ld_16 > FTLOrderOpenPrice(MAGIC3) + 15.0 * Point || ld_16 < FTLOrderOpenPrice(MAGIC3) - BuyLossPoint * Point) {
			while (ld_16 < FTLOrderClose(Slippage, MAGIC3)) {
			}
			
			while (ld_16 < FTLOrderClose(Slippage, MAGIC2)) {
			}
			
			while (ld_16 < FTLOrderClose(Slippage, MAGIC1)) {
			}
		}
	} else {
	
		if (FTLCurrentOrders(1, MAGIC3) < 0.0) {
			if (ld_8 < FTLOrderOpenPrice(MAGIC3) - 15.0 * Point || ld_8 > FTLOrderOpenPrice(MAGIC3) + SellLossPoint * Point) {
				while (ld_8 < FTLOrderClose(Slippage, MAGIC3)) {
				}
				
				while (ld_8 < FTLOrderClose(Slippage, MAGIC2)) {
				}
				
				while (ld_8 < FTLOrderClose(Slippage, MAGIC1)) {
				}
			}
		}
	}
	
	if (TimeHour(TimeCurrent()) >= 22 || TimeHour(TimeCurrent()) <= 8) {
		if (FTLCurrentOrders(6, MAGIC1) == 0.0) {
			if (ld_48 >= 15.0 * Point) {
				for (ld_0 = Lots; ld_0 > maxlot_24; ld_0 -= maxlot_24) {
					if (ld_8 < ibands_40)
						FTLOrderSendSL(0, maxlot_24, ld_8, Slippage, 0, 0, gs_232, MAGIC1);
						
					if (ld_16 > ibands_32)
						FTLOrderSendSL(1, maxlot_24, ld_16, Slippage, 0, 0, gs_232, MAGIC1);
				}
				
				if (ld_8 < ibands_40)
					FTLOrderSendSL(0, ld_0, ld_8, Slippage, 0, 0, gs_232, MAGIC1);
					
				if (ld_16 > ibands_32)
					FTLOrderSendSL(1, ld_0, ld_16, Slippage, 0, 0, gs_232, MAGIC1);
			}
		}
		
		if (FTLCurrentOrders(0, MAGIC2) == 0.0 && FTLCurrentOrders(0, MAGIC1) > 0.0) {
			if (ld_8 < FTLOrderOpenPrice(MAGIC1) - 50.0 * Point) {
				for (ld_0 = Lots * gi_252; ld_0 > maxlot_24; ld_0 -= maxlot_24)
					FTLOrderSendSL(0, maxlot_24, ld_8, Slippage, 0, 0, gs_232, MAGIC2);
					
				FTLOrderSendSL(0, ld_0, ld_8, Slippage, 0, 0, gs_232, MAGIC2);
			}
		}
		
		else {
			if (FTLCurrentOrders(1, MAGIC2) == 0.0 && FTLCurrentOrders(1, MAGIC1) < 0.0) {
				if (ld_16 > FTLOrderOpenPrice(MAGIC1) + 50.0 * Point) {
					for (ld_0 = Lots * gi_252; ld_0 > maxlot_24; ld_0 -= maxlot_24)
						FTLOrderSendSL(1, maxlot_24, ld_16, Slippage, 0, 0, gs_232, MAGIC2);
						
					FTLOrderSendSL(1, ld_0, ld_16, Slippage, 0, 0, gs_232, MAGIC2);
				}
			}
		}
		
		if (FTLCurrentOrders(0, MAGIC3) == 0.0 && FTLCurrentOrders(0, MAGIC2) > 0.0) {
			if (ld_8 < FTLOrderOpenPrice(MAGIC2) - 50.0 * Point) {
				for (ld_0 = Lots * gi_256; ld_0 > maxlot_24; ld_0 -= maxlot_24)
					FTLOrderSendSL(0, maxlot_24, ld_8, Slippage, 0, 0, gs_232, MAGIC3);
					
				FTLOrderSendSL(0, ld_0, ld_8, Slippage, 0, 0, gs_232, MAGIC3);
			}
		}
		
		else {
			if (FTLCurrentOrders(1, MAGIC3) == 0.0 && FTLCurrentOrders(1, MAGIC2) < 0.0) {
				if (ld_16 > FTLOrderOpenPrice(MAGIC2) + 50.0 * Point) {
					for (ld_0 = Lots * gi_256; ld_0 > maxlot_24; ld_0 -= maxlot_24)
						FTLOrderSendSL(1, maxlot_24, ld_16, Slippage, 0, 0, gs_232, MAGIC3);
						
					FTLOrderSendSL(1, ld_0, ld_16, Slippage, 0, 0, gs_232, MAGIC3);
				}
			}
		}
	}
	
	return (0);
}

double Hornet::LotsOptimized() {
	double ld_ret_0 = MaxLots;
	int l_hist_total_8 = OrdersHistoryTotal();
	int l_count_12 = 0;
	ld_ret_0 = NormalizeDouble(AccountFreeMargin() * MaximumRisk / 1250000.0, 1);
	
	if (ld_ret_0 < 0.1)
		ld_ret_0 = 0.1;
		
	if (MinLots < 0.1 || ld_ret_0 < 0.1) {
		ld_ret_0 = NormalizeDouble(AccountFreeMargin() * MaximumRisk / 125000.0, 2);
		
		if (ld_ret_0 < 0.01)
			ld_ret_0 = 0.01;
	}
	
	{
		for (int l_pos_16 = l_hist_total_8 - 1; l_pos_16 >= 0; l_pos_16--) {
			if (OrderSelect(l_pos_16, SELECT_BY_POS, MODE_HISTORY) == false) {
				Print("Error in history!");
				break;
			}
			
			if (OrderSymbol() != Symbol() || OrderType() > OP_SELL)
				continue;
				
			if (OrderProfit() > 0.0)
				break;
				
			if (OrderProfit() < 0.0)
				l_count_12++;
		}
		
		if (l_count_12 > 1)
			ld_ret_0 = NormalizeDouble(ld_ret_0 - ld_ret_0 * l_count_12 , 1);
	}
	
	if (ld_ret_0 < MinLots)
		ld_ret_0 = MinLots;
		
	if (ld_ret_0 > MaxLots)
		ld_ret_0 = MaxLots;
		
	if (!UseAutoLot)
		ld_ret_0 = MaxLots;
		
	return (ld_ret_0);
}

double Hornet::FTLOrderOpenPrice(int a_magic_0) {
	for (int pos_4 = 0; pos_4 < OrdersTotal(); pos_4++) {
		if (OrderSelect(pos_4, SELECT_BY_POS) == false)
			break;
			
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != a_magic_0)
			continue;
			
		if (OrderType() != OP_BUY && OrderType() != OP_SELL)
			continue;
			
		return (OrderOpenPrice());
	}
	
	return (0);
}

double Hornet::FTLCurrentOrders(int ai_0, int a_magic_4) {
	double ld_ret_8 = 0.0;
	
	for (int pos_16 = 0; pos_16 < OrdersTotal(); pos_16++) {
		if (OrderSelect(pos_16, SELECT_BY_POS) == false)
			break;
			
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != a_magic_4)
			continue;
			
		switch (ai_0) {
		
		case 0:
		
			if (OrderType() == OP_BUY)
				ld_ret_8 += OrderLots();
				
			break;
			
		case 1:
			if (OrderType() == OP_SELL)
				ld_ret_8 -= OrderLots();
				
			break;
			
		case 2:
			if (OrderType() == OP_BUYLIMIT)
				ld_ret_8 += OrderLots();
				
			break;
			
		case 3:
			if (OrderType() == OP_SELLLIMIT)
				ld_ret_8 -= OrderLots();
				
			break;
			
		case 4:
			if (OrderType() == OP_BUYSTOP)
				ld_ret_8 += OrderLots();
				
			break;
			
		case 5:
			if (OrderType() == OP_SELLSTOP)
				ld_ret_8 -= OrderLots();
				
			break;
			
		case 6:
			if (OrderType() == OP_BUY)
				ld_ret_8 += OrderLots();
				
			if (OrderType() == OP_SELL)
				ld_ret_8 -= OrderLots();
				
			break;
			
		case 7:
			if (OrderType() == OP_BUYLIMIT)
				ld_ret_8 += OrderLots();
				
			if (OrderType() == OP_SELLLIMIT)
				ld_ret_8 -= OrderLots();
				
			break;
			
		case 8:
			if (OrderType() == OP_BUYSTOP)
				ld_ret_8 += OrderLots();
				
			if (OrderType() == OP_SELLSTOP)
				ld_ret_8 -= OrderLots();
				
			break;
			
		case 9:
			if (OrderType() == OP_BUYLIMIT || OrderType() == OP_BUYSTOP)
				ld_ret_8 += OrderLots();
				
			if (OrderType() == OP_SELLLIMIT || OrderType() == OP_SELLSTOP)
				ld_ret_8 -= OrderLots();
				
			break;
			
		case 10:
			if (OrderType() == OP_BUY || OrderType() == OP_BUYLIMIT || OrderType() == OP_BUYSTOP)
				ld_ret_8 += OrderLots();
				
			break;
			
		case 11:
			if (OrderType() == OP_SELL || OrderType() == OP_SELLLIMIT || OrderType() == OP_SELLSTOP)
				ld_ret_8 -= OrderLots();
				
			break;
			
		case 12:
			if (OrderType() == OP_BUY || OrderType() == OP_BUYLIMIT || OrderType() == OP_BUYSTOP)
				ld_ret_8 += OrderLots();
				
			if (OrderType() == OP_SELL || OrderType() == OP_SELLLIMIT || OrderType() == OP_SELLSTOP)
				ld_ret_8 -= OrderLots();
				
			break;
			
		default:
			Print("[CurrentOrdersError] : Illegal order type(" + ai_0 + ")");
		}
	}
	
	return (ld_ret_8);
}

int Hornet::FTLOrderSend(int a_cmd_0, double a_lots_4, double a_price_12, int a_slippage_20, double a_price_24, double a_price_32, string a_comment_40, int a_magic_48) {
	int error_52;
	a_price_12 = NormalizeDouble(a_price_12, Digits);
	a_price_24 = NormalizeDouble(a_price_24, Digits);
	a_price_32 = NormalizeDouble(a_price_32, Digits);
	int li_56 = GetTickCount();
	
	while (true) {
		if (GetTickCount() - li_56 > 1000 * FTLOrderWaitingTime) {
			Alert("OrderSend timeout. Check the experts log.");
			return (0);
		}
		
		if (IsTradeAllowed() == true) {
			RefreshRates();
			
			if (OrderSend(Symbol(), a_cmd_0, a_lots_4, a_price_12, a_slippage_20, a_price_24, a_price_32, a_comment_40, a_magic_48, 0, gia_260[a_cmd_0]) != -1)
				return (1);
				
			error_52 = GetLastError();
			
			Print("[OrderSendError] : ", error_52, " ", ErrorDescription(error_52));
			
			if (error_52 != 129/* INVALID_PRICE */) {
				if (error_52 != 130/* INVALID_STOPS */) {
				}
			}
		}
		
		Sleep(100);
	}
	
	return (0);
}

int Hornet::FTLOrderClose(int a_slippage_0, int a_magic_4) {
	int cmd_8;
	int error_12;
	int ticket_16 = 0;
	
	for (int pos_20 = 0; pos_20 < OrdersTotal(); pos_20++) {
		if (OrderSelect(pos_20, SELECT_BY_POS) == false)
			break;
			
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != a_magic_4)
			continue;
			
		cmd_8 = OrderType();
		
		if (cmd_8 == OP_BUY || cmd_8 == OP_SELL) {
			ticket_16 = OrderTicket();
			break;
		}
	}
	
	if (ticket_16 == 0)
		return (0);
		
	int li_24 = GetTickCount();
	
	while (true) {
		if (GetTickCount() - li_24 > 1000 * FTLOrderWaitingTime) {
			Alert("OrderClose timeout. Check the experts log.");
			return (0);
		}
		
		if (IsTradeAllowed() == true) {
			RefreshRates();
			
			if (OrderClose(ticket_16, OrderLots(), NormalizeDouble(OrderClosePrice(), Digits), a_slippage_0, gia_260[cmd_8]) == 1)
				return (1);
				
			error_12 = GetLastError();
			
			Print("[OrderCloseError] : ", error_12, " ", ErrorDescription(error_12));
			
			if (error_12 != 129/* INVALID_PRICE */) {
			}
		}
		
		Sleep(100);
	}
	
	return (0);
}

void Hornet::FTLOrderSendSL(int ai_0, double ad_4, double ad_12, int ai_20, int ai_24, int ai_28, string as_32, int ai_40) {
	int li_44 = 1;
	
	if (Digits == 3 || Digits == 5)
		li_44 = 10;
		
	ai_20 *= li_44;
	
	if (ai_0 == 1 || ai_0 == 3 || ai_0 == 5)
		li_44 = -1 * li_44;
		
	double ld_48 = 0;
	
	double ld_56 = 0;
	
	if (ai_24 > 0)
		ld_48 = ad_12 - ai_24 * Point * li_44;
		
	if (ai_28 > 0)
		ld_56 = ad_12 + ai_28 * Point * li_44;
		
	FTLOrderSend(ai_0, ad_4, ad_12, ai_20, ld_48, ld_56, as_32, ai_40);
}

}

#endif
