#include "Overlook.h"


// fx maven

namespace Overlook {

Maverick::Maverick() {
	
}

void Maverick::InitEA() {
	g_count_112 = Interval;
	
	AddSubCore<CommodityChannelIndex>()
		.Set("period", cci_period0);
	
	AddSubCore<CommodityChannelIndex>()
		.Set("period", cci_period1);
	
	AddSubCore<CommodityChannelIndex>()
		.Set("period", cci_period2);
	
}

void Maverick::StartEA(int pos) {
	int li_unused_0 = 0;
	bool li_4 = false;
	bool li_8 = false;
	bool li_12 = false;
	double l_price_16 = 0;
	double l_price_24 = 0;
	double l_price_32 = 0;
	double l_price_40 = 0;
	double l_point_48 = Point;
	int l_ord_total_80 = 0;
	
	if (pos < 1)
		return;
	
	CompatBuffer Open(GetInputBuffer(0, 0), pos);
	
	g_open_96 = Open[0];
	
	g_open_104 = Open[1];
	
	g_count_112++;
	
	l_price_32 = Ask + l_point_48 * TakeProfit;
	
	l_price_16 = Ask - l_point_48 * StopLoss;
	
	l_price_40 = Bid - l_point_48 * TakeProfit;
	
	l_price_24 = Bid + l_point_48 * StopLoss;
	
	if (TakeProfit == 0) {
		l_price_32 = 0;
		l_price_40 = 0;
	}
	
	if (StopLoss == 0) {
		l_price_16 = 0;
		l_price_24 = 0;
	}
	
	double l_icci_56 = At(0).GetBuffer(0).Get(pos);
	double l_icci_64 = At(1).GetBuffer(0).Get(pos);
	double l_icci_72 = At(2).GetBuffer(0).Get(pos);
	
	if (l_icci_64 <= 0.0 && l_icci_56 >= 0.0 && l_icci_72 > 0.0) {
		li_4 = true;
		li_12 = true;
		Print("Rising  Cross");
	}
	
	if (l_icci_64 >= 0.0 && l_icci_56 <= 0.0 && l_icci_72 < 0.0) {
		li_8 = true;
		li_12 = true;
		Print("Falling Cross");
	}
	
	if (li_12) {
		for (l_ord_total_80 = OrdersTotal()-1; l_ord_total_80 >= 0; l_ord_total_80--) {
			OrderSelect(l_ord_total_80, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol()) {
				if (OrderType() == OP_BUY)
					OrderClose(OrderTicket(), Lots, Bid, 3, White);
					
				else if (OrderType() == OP_SELL)
					OrderClose(OrderTicket(), Lots, Ask, 3, Red);
					
				g_count_112 = 0;
			}
		}
		
		if (li_4)
			OrderSend(Symbol(), OP_BUY, Lots, Ask, 3, l_price_16, l_price_32, "ZZZ100", 11123, 0, White);
			
		if (li_8)
			OrderSend(Symbol(), OP_SELL, Lots, Bid, 3, l_price_24, l_price_40, "ZZZ100", 11321, 0, Red);
			
		g_count_112 = 0;
	}
	
	li_unused_0 = 0;
	
	for (l_ord_total_80 = OrdersTotal()-1; l_ord_total_80 >= 0; l_ord_total_80--) {
		OrderSelect(l_ord_total_80, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol()) {
			if (OrderType() == OP_BUY) {
				if (g_count_112 >= Interval) {
					OrderSend(Symbol(), OP_BUY, Lots, Ask, 3, l_price_16, l_price_32, "ZZZ100", 11123, 0, White);
					g_count_112 = 0;
				}
			}
			
			else if (OrderType() == OP_SELL) {
				if (g_count_112 >= Interval) {
					OrderSend(Symbol(), OP_SELL, Lots, Bid, 3, l_price_24, l_price_40, "ZZZ100", 11321, 0, Red);
					g_count_112 = 0;
				}
			}
			
			li_unused_0 = 1;
			
			break;
		}
	}
	
}

}

