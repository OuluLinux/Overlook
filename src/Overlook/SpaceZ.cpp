#include "Overlook.h"

namespace Overlook {



SpaceZ::SpaceZ() {

}

void SpaceZ::InitEA() {
	g_bars_124 = Bars;
	
	if (gi_80)
		gi_unused_128 = 0;
	else
		gi_unused_128 = 1;
	
	AddSubCore<Momentum>()
		.Set("period", mom_period);
		
	AddSubCore<MovingAverageConvergenceDivergence>()
		.Set("fast_ema", macd_fast)
		.Set("slow_ema", macd_slow)
		.Set("signal_sma_period", macd_signal)
		.Set("method", 1);
	
	AddSubCore<ParabolicSAR>()
		.Set("step", stepfast)
		.Set("maximum", maximumfast);
	
	AddSubCore<RelativeStrengthIndex>()
		.Set("period", rsi_period);
	
	AddSubCore<AverageDirectionalMovement>()
		.Set("period", adx_period);
	
}

void SpaceZ::StartEA(int pos) {
	int l_ticket_8;
	double l_price_12;
	double l_price_20;
	int li_0 = 0;
	
	if (pos < 1)
		return;
	
	if (gi_80 && Bars != g_bars_124)
		gi_132 = false;
		
	int l_ord_total_4 = OrdersTotal();
	
	li_0 = 0;
	
	
	double l_imomentum_116 = At(0).GetBuffer(0).Get(pos - 1);
	
	double l_imomentum_124 = At(0).GetBuffer(0).Get(pos - 0);
	
	double l_imacd_76 = At(1).GetBuffer(0).Get(pos - 0);
	
	double l_imacd_84 = At(1).GetBuffer(0).Get(pos - 1);
	
	double l_imacd_92 = At(1).GetBuffer(1).Get(pos - 0);
	
	double l_isar_100 = At(2).GetBuffer(0).Get(pos - 1);
	
	double l_isar_108 = At(2).GetBuffer(0).Get(pos - 0);
	
	double l_irsi_132 = At(3).GetBuffer(0).Get(pos - 0);
	
	double l_iadx_140 = At(4).GetBuffer(0).Get(pos - 0);
		
	bool li_148 = false;
	
	for (int l_pos_152 = 0; l_pos_152 < l_ord_total_4; l_pos_152++) {
		OrderSelect(l_pos_152, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol()) {
			li_148 = true;
			
			if (OrderType() == OP_BUY) {
				if (li_0 == 3 && (gi_80 && !gi_132) || (!gi_80 && Bars != g_bars_124)) {
					OrderClose(OrderTicket(), OrderLots(), Bid, Slippage);
											
					if (!gi_80)
						g_bars_124 = Bars;
						
					li_148 = false;
				}
				
				else {
					if (UseTrailingStop && TrailingStop > 0) {
						if (Bid - OrderOpenPrice() > Point * TrailingStop) {
							if (OrderStopLoss() < Bid - Point * TrailingStop) {
								OrderModify(OrderTicket(), OrderOpenPrice(), Bid - Point * TrailingStop, OrderTakeProfit());
								
								if (!gi_80)
									g_bars_124 = Bars;
							}
						}
					}
				}
			}
			
			else {
				if (li_0 == 4 && (gi_80 && !gi_132) || (!gi_80 && Bars != g_bars_124)) {
					OrderClose(OrderTicket(), OrderLots(), Ask, Slippage);
						
					if (!gi_80)
						g_bars_124 = Bars;
						
					li_148 = false;
				}
				
				else {
					if (UseTrailingStop && TrailingStop > 0) {
						if (OrderOpenPrice() - Ask > Point * TrailingStop) {
							if (OrderStopLoss() > Ask + Point * TrailingStop || OrderStopLoss() == 0.0) {
								OrderModify(OrderTicket(), OrderOpenPrice(), Ask + Point * TrailingStop, OrderTakeProfit());
								
								if (!gi_80)
									g_bars_124 = Bars;
							}
						}
					}
				}
			}
		}
	}
	
	if (l_isar_100 < l_isar_108 && l_imomentum_124 < 100.0 && l_imacd_76 > 0.0 && l_irsi_132 < 70.0 && l_iadx_140 > 25.0)
		li_0 = 1;
		
	if (l_isar_100 > l_isar_108 && l_imomentum_124 > 100.0 && l_imacd_76 < 0.0 && l_irsi_132 > 30.0 && l_iadx_140 < 25.0)
		li_0 = 2;
		
	if (li_0 == 1 && (gi_80 && !gi_132) || (!gi_80 && Bars != g_bars_124)) {
		if (!li_148) {
			if (AccountFreeMargin() < 1000.0 * Lots) {
				return;
			}
			
			if (gi_96)
				l_price_12 = Ask - StopLoss * Point;
			else
				l_price_12 = 0.0;
				
			if (gi_104)
				l_price_20 = Ask + TakeProfit * Point;
			else
				l_price_20 = 0.0;
				
			l_ticket_8 = OrderSend(Symbol(), OP_BUY, Lots, Ask, Slippage, l_price_12, l_price_20, "", MagicNumber);
			
			if (l_ticket_8 > 0) {
				if (OrderSelect(l_ticket_8, SELECT_BY_TICKET, MODE_TRADES)) {
					Print("BUY order opened");
					
				}
				
				else
					Print("Error opening BUY order : " + GetLastError());
			}
			
			if (gi_80)
				gi_132 = true;
				
			if (!gi_80)
				g_bars_124 = Bars;
				
			return;
		}
	}
	
	if (li_0 == 2 && (gi_80 && !gi_132) || (!gi_80 && Bars != g_bars_124)) {
		if (!li_148) {
			if (AccountFreeMargin() < 1000.0 * Lots) {
				return;
			}
			
			if (gi_96)
				l_price_12 = Bid + StopLoss * Point;
			else
				l_price_12 = 0.0;
				
			if (gi_104)
				l_price_20 = Bid - TakeProfit * Point;
			else
				l_price_20 = 0.0;
				
			l_ticket_8 = OrderSend(Symbol(), OP_SELL, Lots, Bid, Slippage, l_price_12, l_price_20, "", MagicNumber);
			
			if (l_ticket_8 > 0) {
				if (OrderSelect(l_ticket_8, SELECT_BY_TICKET, MODE_TRADES)) {
					
				}
				
				else
					Print("Error opening SELL order : " + GetLastError());
			}
			
			if (gi_80)
				gi_132 = true;
				
			if (!gi_80)
				g_bars_124 = Bars;
				
			return;
		}
	}
	
	if (!gi_80)
		g_bars_124 = Bars;
	
}

}
