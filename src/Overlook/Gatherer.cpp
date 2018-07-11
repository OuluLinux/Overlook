#include "Overlook.h"

// harvester

namespace Overlook {

Gatherer::Gatherer() {

}

void Gatherer::InitEA() {
	g_spread_144 = MarketInfo(Symbol(), MODE_SPREAD);
	
	
	AddSubCore<Laguerre>()
		.Set("gamma", lag_gamma);
	
	AddSubCore<CommodityChannelIndex>()
		.Set("period", cci_period);
	
	AddSubCore<MovingAverage>()
		.Set("period", ma_period).Set("method", 1);
	
	
}

void Gatherer::StartEA(int pos) {
	double l_icustom_0;
	double l_icci_8;
	double l_ima_16;
	double l_ima_24;
	int l_ticket_36;
	int l_ord_total_40;
	bool li_44;
	bool li_48;
	double l_price_52;
	
	if (pos < 1)
		return;
	
	if ((RazVSutki == true && Hour() == Hours && Minute() == Ìinutes) || RazVSutki == false) {
		l_icustom_0 = At(0).GetBuffer(0).Get(pos - 0);
		l_icci_8 = At(1).GetBuffer(0).Get(pos - 0);
		l_ima_16 = At(2).GetBuffer(0).Get(pos - 0);
		l_ima_24 = At(2).GetBuffer(0).Get(pos - 1);
		l_ord_total_40 = OrdersTotal();
		li_44 = true;
		li_48 = true;
		
		for (int l_pos_32 = 0; l_pos_32 < l_ord_total_40; l_pos_32++) {
			OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderType() == OP_SELL && OrderSymbol() == Symbol())
				li_44 = false;
				
			if (OrderType() == OP_BUY && OrderSymbol() == Symbol())
				li_48 = false;
		}
		
		if (AccountFreeMargin() < 1000.0 * gd_88) {
			return;
		}
		
		if (l_icustom_0 == 0.0 && l_icci_8 < -5.0 && li_48) {
			l_price_52 = Ask - g_pips_120 * Point;
			l_ticket_36 = OrderSend(Symbol(), OP_BUY, LotsOptimized(), Ask, 3, l_price_52, Ask + g_pips_136 * Point, "starter", 16384, 0, Green);
		}
		
		if (l_icustom_0 == 1.0 && l_icci_8 > 5.0 && li_44) {
			l_price_52 = Bid + g_pips_120 * Point;
			l_ticket_36 = OrderSend(Symbol(), OP_SELL, LotsOptimized(), Bid, 3, l_price_52, Bid - g_pips_136 * Point, "starter", 16384, 0, Red);
		}
		
		for (int l_pos_32 = 0; l_pos_32 < l_ord_total_40; l_pos_32++) {
			OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderType() <= OP_SELL && OrderSymbol() == Symbol()) {
				if (OrderType() == OP_BUY) {
					if (l_icustom_0 > 0.9) {
						OrderClose(OrderTicket(), OrderLots(), Bid, 3);
						return;
					}
					
					if (g_pips_112 <= 0.0)
						continue;
						
					if (Bid - OrderOpenPrice() <= Point * g_pips_112)
						continue;
						
					OrderClose(OrderTicket(), OrderLots(), Bid, 3);
					
					return;
				}
				
				if (l_icustom_0 < 0.1) {
					OrderClose(OrderTicket(), OrderLots(), Ask, 3);
					return;
				}
				
				if (g_pips_112 > 0.0) {
					if (OrderOpenPrice() - Ask > Point * g_pips_112) {
						OrderClose(OrderTicket(), OrderLots(), Ask, 3);
						return;
					}
				}
			}
		}
	}
}

double Gatherer::LotsOptimized() {
	double ld_ret_0 = gd_88;
	int l_hist_total_8 = OrdersHistoryTotal();
	int l_count_12 = 0;
	ld_ret_0 = NormalizeDouble(AccountFreeMargin() * gd_96 / 500.0, 1);
	
	if (gd_104 > 0.0) {
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
			ld_ret_0 = NormalizeDouble(ld_ret_0 - ld_ret_0 * l_count_12 / gd_104, 1);
	}
	
	if (ld_ret_0 < 0.1)
		ld_ret_0 = 0.1;
		
	if (ld_ret_0 > 1000.0)
		ld_ret_0 = 1000;
		
	return (ld_ret_0);
}

}
