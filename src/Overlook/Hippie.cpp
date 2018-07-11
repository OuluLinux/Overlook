#include "Overlook.h"

// warlord

namespace Overlook {

Hippie::Hippie() {

}

void Hippie::InitEA() {
	g_point_148 = MarketInfo(Symbol(), MODE_POINT);
	gi_156 = true;
	
	AddSubCore<Momentum>()
		.Set("period", TrendLevel);
	
	AddSubCore<AverageDirectionalMovement>()
		.Set("period", TrendPower);
	
	AddSubCore<OsMA>()
		.Set("period", osma_period);
	
	AddSubCore<WilliamsPercentRange>()
		.Set("period", Sensitivity);
	
	AddSubCore<MovingAverageConvergenceDivergence>()
		.Set("fast_ema", macd_period)
		.Set("slow_ema", macd_period*2)
		;
	
}

void Hippie::StartEA(int pos) {
	double l_imomentum_16;
	double l_iadx_24;
	double l_iosma_40;
	double l_iwpr_48;
	double l_imacd_56;
	
	if (gi_156 == false)
		return;
		
	double l_lots_0 = 0;
	
	int l_pos_8 = 0;
	
	if (Bars < 10) {
		Print("No Trade !!");
	}
	
	if (OrdersTotal() < 1) {
		if (AccountFreeMargin() < 1.0 * Lots) {
			Print("Money is Not Enough !!");
			return;
		}
		
		l_lots_0 = MathCeil(AccountEquity() * Risk / 10000.0) / 10.0;
		
		l_imomentum_16 = At(0).GetBuffer(0).Get(pos);
		l_iadx_24 = At(1).GetBuffer(0).Get(pos);
		l_iosma_40 = At(2).GetBuffer(0).Get(pos);
		l_iwpr_48 = At(3).GetBuffer(0).Get(pos);
		l_imacd_56 = At(4).GetBuffer(0).Get(pos);
		
		if (l_imacd_56 > 0.0 && l_iosma_40 > 0.0 && l_imomentum_16 > 100.0 && l_iadx_24 > 21.0 && l_iwpr_48 < -80.0) {
			OrderSend(Symbol(), OP_BUY, l_lots_0, Bid, Slippage, Bid - Stoploss * g_point_148, Ask + TakeProfit * g_point_148, "", MagicNumber, 0, Blue);
			return;
		}
		
		if (l_imacd_56 < 0.0 && l_iosma_40 < 0.0 && l_imomentum_16 < 100.0 && l_iadx_24 < 21.0 && l_iwpr_48 > -20.0) {
			OrderSend(Symbol(), OP_SELL, l_lots_0, Ask, Slippage, Ask + Stoploss * g_point_148, Bid - TakeProfit * g_point_148, "", MagicNumber, 0, Red);
			return;
		}
	}
	
	int l_ord_total_12 = OrdersTotal();
	
	for (l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol()) {
			if (OrderType() == OP_BUY) {
				if (OrderOpenTime().Get() - TimeCurrent().Get() >= 300 || AccountProfit() > 2.0) {
					if (TimeCurrent().Get() - OrderOpenTime().Get() >= 300 || AccountProfit() > 2.0) {
						OrderClose(OrderTicket(), OrderLots(), Ask, 0);
						return;
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
				if (TimeCurrent().Get() - OrderOpenTime().Get() >= 300 || AccountProfit() > 2.0) {
					OrderClose(OrderTicket(), OrderLots(), Bid, 0);
					return;
				}
			}
		}
	}
	
}

}
