#include "Overlook.h"

namespace Overlook {

Foster::Foster() {
	
}

void Foster::InitEA() {
	g_bars_168 = Bars;
	
	if (Point == 0.00001)
		g_point_180 = 0.0001;
	else {
		if (Point == 0.001)
			g_point_180 = 0.01;
		else
			g_point_180 = Point;
	}
	
	
	AddSubCore<AverageDirectionalMovement>()
		.Set("period", g_period_104);
	
}

void Foster::StartEA(int pos) {
	
	if (pos < 2)
		return;
	
	int l_ticket_0;
	double l_price_4;
	double l_price_12;
	bool li_20 = false;
	bool li_24 = false;
	g_lots_172 = NormalizeDouble(AccountFreeMargin() * gi_144 / 50000.0, 1);
	
	if (g_lots_172 > MaxLots)
		g_lots_172 = MaxLots;
		
	int l_ord_total_28 = OrdersTotal();
	
	double l_iadx_32 = At(0).GetBuffer(1).Get(pos - 1);
	
	double l_iadx_40 = At(0).GetBuffer(1).Get(pos - 2);
	
	double l_iadx_48 = At(0).GetBuffer(2).Get(pos - 1);
	
	double l_iadx_56 = At(0).GetBuffer(2).Get(pos - 2);
	
	/*double l_imfi_64 = iMFI(NULL, PERIOD_M30, g_period_108, 0);
	
	double l_imfi_72 = iMFI(NULL, PERIOD_M30, g_period_108, 1);*/
	
	bool li_80 = false;
	
	for (int l_pos_84 = 0; l_pos_84 < l_ord_total_28; l_pos_84++) {
		OrderSelect(l_pos_84, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == Magic) {
			li_80 = true;
			
			if (OrderType() == OP_BUY) {
				if (OrderMagicNumber() == Magic && gi_92 > 0 && Bid - OrderOpenPrice() > g_point_180 * gi_92 && OrderStopLoss() < Bid - g_point_180 * gi_92) {
					OrderModify(OrderTicket(), OrderOpenPrice(), Bid - g_point_180 * gi_92, OrderTakeProfit());
					g_bars_168 = Bars;
				}
			}
			
			else {
				if (OrderMagicNumber() == Magic && gi_92 > 0 && OrderOpenPrice() - Ask > g_point_180 * gi_92 && OrderStopLoss() > Ask + g_point_180 * gi_92 || OrderStopLoss() == 0.0) {
					OrderModify(OrderTicket(), OrderOpenPrice(), Ask + g_point_180 * gi_92, OrderTakeProfit());
					g_bars_168 = Bars;
				}
			}
		}
	}
	
	if (li_80)
		return;
		
	if (l_iadx_32 > l_iadx_40 && l_iadx_32 > l_iadx_48 /*&& l_imfi_64 < gi_120 && l_imfi_64 > l_imfi_72*/)
		li_20 = true;
		
	if (l_iadx_48 > l_iadx_56 && l_iadx_48 > l_iadx_32 /*&& l_imfi_64 > gi_124 && l_imfi_64 < l_imfi_72*/)
		li_24 = true;
		
	if (li_20 && Bars != g_bars_168) {
		if (AccountFreeMargin() < 1000.0 * g_lots_172) {
			return;
		}
		
		l_price_4 = Ask - SL * g_point_180;
		
		l_price_12 = Ask + TP * g_point_180;
		l_ticket_0 = OrderSend(Symbol(), OP_BUY, g_lots_172, Ask, g_slippage_156, l_price_4, l_price_12, "", Magic);
		
		if (l_ticket_0 > 0) {
			if (OrderSelect(l_ticket_0, SELECT_BY_TICKET, MODE_TRADES)) {
				Print("buy order opened at");
			}
			
			else
				Print("error opening buy order : " + GetLastError());
		}
	}
	
	if (li_24 && Bars != g_bars_168) {
		if (AccountFreeMargin() < 1000.0 * g_lots_172) {
			return;
		}
		
		l_price_4 = Bid + SL * g_point_180;
		
		l_price_12 = Bid - TP * g_point_180;
		l_ticket_0 = OrderSend(Symbol(), OP_SELL, g_lots_172, Bid, g_slippage_156, l_price_4, l_price_12, "", Magic);
		
	}
	
	g_bars_168 = Bars;
	
}

}
