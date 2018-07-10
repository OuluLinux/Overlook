#include "Overlook.h"

#if 0

// warlord

namespace Overlook {

Hippie::Hippie() {

}

void Hippie::InitEA() {
	g_point_148 = MarketInfo(Symbol(), MODE_POINT);
	gi_156 = true;
	return (0);
}

void Hippie::StartEA(int pos) {
	double l_imomentum_16;
	double l_iadx_24;
	double l_iosma_40;
	double l_iwpr_48;
	double l_imacd_56;
	
	if (TimeMonth(TimeCurrent() && TimeYear(TimeCurrent())) >= 12.2009 && gi_156 == true) {
		Alert("The EA has expired");
		gi_156 = false;
	}
	
	if (gi_156 == false)
		return (0);
		
	double l_lots_0 = 0;
	
	int l_pos_8 = 0;
	
	if (Bars < 10) {
		Print("No Trade !!");
		return (0);
	}
	
	if (OrdersTotal() < 1) {
		if (AccountFreeMargin() < 1.0 * Lots) {
			Print("Money is Not Enough !!");
			return (0);
		}
		
		l_lots_0 = MathCeil(AccountEquity() * Risk / 10000.0) / 10.0;
		
		HideTestIndicators(true);
		l_imomentum_16 = iMomentum(NULL, 0, TrendLevel, PRICE_OPEN, 0);
		l_iadx_24 = iADX(NULL, 0, TrendPower, PRICE_CLOSE, MODE_MAIN, 0);
		l_iosma_40 = iOsMA(NULL, 0, 90, 99, 88, PRICE_OPEN, 1);
		l_iwpr_48 = iWPR(NULL, 0, Sensitivity, 0);
		l_imacd_56 = iMACD(NULL, 0, 90, 99, 88, PRICE_TYPICAL, MODE_SIGNAL, 0);
		HideTestIndicators(false);
		
		if (l_imacd_56 > 0.0 && l_iosma_40 > 0.0 && l_imomentum_16 > 100.0 && l_iadx_24 > 21.0 && l_iwpr_48 < -80.0) {
			OrderSend(Symbol(), OP_BUY, l_lots_0, Bid, Slippage, Bid - Stoploss * g_point_148, Ask + TakeProfit * g_point_148, "", MagicNumber, 0, Blue);
			return (0);
		}
		
		if (l_imacd_56 < 0.0 && l_iosma_40 < 0.0 && l_imomentum_16 < 100.0 && l_iadx_24 < 21.0 && l_iwpr_48 > -20.0) {
			OrderSend(Symbol(), OP_SELL, l_lots_0, Ask, Slippage, Ask + Stoploss * g_point_148, Bid - TakeProfit * g_point_148, "", MagicNumber, 0, Red);
			return (0);
		}
	}
	
	int l_ord_total_12 = OrdersTotal();
	
	for (l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol()) {
			if (OrderType() == OP_BUY) {
				if (OrderOpenTime() - (TimeCurrent() >= 300) || AccountProfit() > 2.0) {
					if (TimeCurrent() - (OrderOpenTime() >= 300) || AccountProfit() > 2.0) {
						OrderClose(OrderTicket(), OrderLots(), Ask, 0);
						return (0);
					}
				}
			}
		}
	}
	
	l_ord_total_12 = OrdersTotal();
	
	for (l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_BUY && OrderSymbol() == Symbol()) {
			if (OrderType() == OP_SELL) {
				if (TimeCurrent() - (OrderOpenTime() >= 300) || AccountProfit() > 2.0) {
					OrderClose(OrderTicket(), OrderLots(), Bid, 0);
					return (0);
				}
			}
		}
	}
	
	return (0);
}

}

#endif
