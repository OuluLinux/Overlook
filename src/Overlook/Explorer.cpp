#include "Overlook.h"


namespace Overlook {

Explorer::Explorer() {
	
}

void Explorer::InitEA() {
	
	AddSubCore<ParabolicSAR>()
		.Set("step", stepslow)
		.Set("maximum", maximumslow);
	
	AddSubCore<ParabolicSAR>()
		.Set("step", stepfast)
		.Set("maximum", maximumfast);
	
	AddSubCore<Channel>()
		.Set("period", barsearch);
}

void Explorer::StartEA(int pos) {
	this->pos = pos;
	
	if (!pos)
		return;
	
	if (g_bars_184 != Bars && (GetTimeControl(starttime, stoptime) != 1 && timecontrol == 1) || timecontrol == 0) {
		g_bars_184 = Bars;
		ret0 = ScalpParabolicPattern(pos);
	}
	
	if (TrailingStop > 0)
		ret0 = GetTrailingStop();
		
	if (BBUSize > 0)
		ret0 = BBU();
	
}

int Explorer::ScalpParabolicPattern(int pos) {
	int l_ticket_0;
	double ld_4;
	double l_price_12;
	double l_price_20;
	double l_high_28;
	double ld_36;
	double ld_44;
	double ld_52;
	double l_isar_60 = At(0).GetBuffer(0).Get(pos);
	double l_isar_68 = At(1).GetBuffer(0).Get(pos);
	
	if (l_isar_68 > Bid && l_isar_60 < Bid)
		ret1 = true;
		
	if (l_isar_60 > Bid)
		ret1 = false;
		
	if (l_isar_68 < Bid && l_isar_60 > Bid)
		ret2 = true;
		
	if (l_isar_60 < Bid)
		ret2 = false;
		
	bool l_bool_84 = RiskManagement;
	
	if (l_bool_84) {
		if (RiskPercent < 0.1 || RiskPercent > 100.0) {
			Comment("Invalid Risk Value.");
			return 0;
		}
		
		Lots = MathFloor(100.0 * (AccountFreeMargin() * AccountLeverage() * RiskPercent * Point) / (Ask * MarketInfo(Symbol(), MODE_LOTSIZE) * MarketInfo(Symbol(), MODE_MINLOT))) * MarketInfo(Symbol(), MODE_MINLOT);
	}
	
	if (l_bool_84 == false) {
	}
	
	if (l_isar_60 < Bid && l_isar_68 < Bid && ret1 == true) {
		ret1 = false;
		ld_36 = MaximumMinimum(0, barsearch);
		l_high_28 = GetInputBuffer(0, 2).Get(pos - 1);
		ld_44 = GetFiboUr(l_high_28, ld_36, Ur1 / 100.0);
		ld_52 = GetFiboUr(l_high_28, ld_36, Ur2 / 100.0);
		ld_4 = ld_44;
		l_price_12 = ld_36 - otstup * Point;
		l_price_20 = ld_52;
		
		if (Ask - ld_4 < 5.0 * Point || Ask - l_price_12 < 5.0 * Point || l_price_20 - Ask < 5.0 * Point)
			return 0;
			
		l_ticket_0 = OrderSend(Symbol(), OP_BUYLIMIT, Lots, NormalizeDouble(ld_4, Digits), 3, l_price_12, l_price_20, "", 0);
		
		if (l_ticket_0 < 0) {
			return (-1);
		}
		
		gi_unused_176 = 0;
	}
	
	if (l_isar_60 > Bid && l_isar_68 > Bid && ret2 == true) {
		ret2 = false;
		l_high_28 = MaximumMinimum(1, barsearch);
		ld_36 = GetInputBuffer(0, 1).Get(pos-1);
		ld_44 = GetFiboUr(ld_36, l_high_28, Ur1 / 100.0);
		ld_52 = GetFiboUr(ld_36, l_high_28, Ur2 / 100.0);
		ld_4 = ld_44;
		l_price_12 = l_high_28 + otstup * Point;
		l_price_20 = ld_52;
		
		if (ld_4 - Ask < 5.0 * Point || l_price_12 - Ask < 5.0 * Point || Ask - l_price_20 < 5.0 * Point)
			return 0;
			
		l_ticket_0 = OrderSend(Symbol(), OP_SELLLIMIT, Lots, NormalizeDouble(ld_4, Digits), 3, l_price_12, l_price_20, "", 0);
		
		if (l_ticket_0 < 0) {
			return (-1);
		}
		
		gi_unused_180 = 0;
	}
	
	if (l_isar_68 > Bid && ChLimitOrder(1) > 0)
		l_ticket_0 = deletelimitorder(1);
		
	if (l_isar_68 < Bid && ChLimitOrder(0) > 0)
		l_ticket_0 = deletelimitorder(0);
		
	return 0;
}

int Explorer::deletelimitorder(int ai_0) {
	int l_ord_delete_8;
	
	for (int li_4 = 1; li_4 <= OrdersTotal(); li_4++) {
		if (OrderSelect(li_4 - 1, SELECT_BY_POS) == true) {
			if (OrderType() == OP_BUYLIMIT && OrderSymbol() == Symbol() && ai_0 == 1)
				l_ord_delete_8 = OrderDelete(OrderTicket());
				
			else if (OrderType() == OP_SELLLIMIT && OrderSymbol() == Symbol() && ai_0 == 0)
				l_ord_delete_8 = OrderDelete(OrderTicket());
		}
	}
	
	return (l_ord_delete_8);
}

int Explorer::ChLimitOrder(int ai_0) {
	for (int li_4 = 1; li_4 <= OrdersTotal(); li_4++) {
		if (OrderSelect(li_4 - 1, SELECT_BY_POS) == true) {
			if (OrderType() == OP_BUYLIMIT && OrderSymbol() == Symbol() && ai_0 == 1)
				return (1);
				
			else if (OrderType() == OP_SELLLIMIT && OrderSymbol() == Symbol() && ai_0 == 0)
				return (1);
		}
	}
	
	return 0;
}

double Explorer::MaximumMinimum(int ai_0, int ai_4) {
	double ld_ret_16;
	int li_8 = 0;
	bool li_12 = false;
	
	Core& c = At(2);
	
	if (ai_0 == 0) {
		while (li_12 == false) {
			int p = pos - ai_4 - li_8;
			if (p < 0) break;
			ld_ret_16 = c.GetBuffer(0).Get(pos - li_8);
			double d = c.GetBuffer(0).Get(p);
			
			if (ld_ret_16 > d) {
				ld_ret_16 = d;
				li_8 += ai_4;
			}
			
			else {
				li_12 = true;
				return (ld_ret_16);
			}
		}
	}
	
	if (ai_0 == 1) {
		while (li_12 == false) {
			int p = pos - ai_4 - li_8;
			if (p < 0) break;
			ld_ret_16 = c.GetBuffer(1).Get(pos - li_8);
			double d = c.GetBuffer(1).Get(p);
			
			if (ld_ret_16 < d) {
				ld_ret_16 = d;
				li_8 += ai_4;
			}
			
			else {
				li_12 = true;
				return (ld_ret_16);
			}
		}
	}
	
	return 0;
}

double Explorer::GetFiboUr(double ad_0, double ad_8, double ad_16) {
	double ld_28;
	int l_digits_24 = MarketInfo(Symbol(), MODE_DIGITS);
	ld_28 = NormalizeDouble(ad_8 + (ad_0 - ad_8) * ad_16, l_digits_24);
	return (ld_28);
}

int Explorer::GetTrailingStop() {
	bool l_bool_4;
	
	for (int li_0 = 1; li_0 <= OrdersTotal(); li_0++) {
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (TrailingStop > 0 && OrderType() == OP_BUY && OrderSymbol() == Symbol()) {
				if (Bid - OrderOpenPrice() >= TrailingStop * Point && TrailingStop > 0 && Bid - Point * TrailingStop > OrderStopLoss()) {
					if (Bid - Point * TrailingStop - OrderStopLoss() >= TrailingShag * Point) {
						Print("ÒÐÅÉËÈÌ");
						l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), Bid - Point * TrailingStop, OrderTakeProfit());
						
						if (l_bool_4 == false)
							return (-1);
					}
				}
			}
		}
		
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (OrderType() == OP_SELL && OrderSymbol() == Symbol()) {
				if (OrderOpenPrice() - Ask >= TrailingStop * Point && TrailingStop > 0 && OrderStopLoss() > Ask + TrailingStop * Point) {
					if (OrderStopLoss() - (Ask + TrailingStop * Point) > TrailingShag * Point) {
						Print("ÒÐÅÉËÈÌ");
						l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), Ask + TrailingStop * Point, OrderTakeProfit());
						
						if (l_bool_4 == false)
							return (-1);
					}
				}
			}
		}
	}
	
	return 0;
}

int Explorer::BBU() {
	bool l_bool_4;
	
	for (int li_0 = 1; li_0 <= OrdersTotal(); li_0++) {
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (BBUSize > 0 && OrderType() == OP_BUY && OrderSymbol() == Symbol() && OrderStopLoss() < OrderOpenPrice()) {
				if (Bid - OrderOpenPrice() >= BBUSize * Point && BBUSize > 0) {
					Print("ÏÎÐÀ Â ÁåçóÁûòîê");
					l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + 1.0 * Point, OrderTakeProfit());
					
					if (l_bool_4 == false)
						return (-1);
				}
			}
		}
		
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (BBUSize > 0 && OrderType() == OP_SELL && OrderSymbol() == Symbol() && OrderStopLoss() > OrderOpenPrice() || OrderStopLoss() == 0.0) {
				if (OrderOpenPrice() - Ask >= BBUSize * Point && BBUSize > 0) {
					Print("ÏÎÐÀ Â ÁåçóÁûòîê");
					l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - 1.0 * Point, OrderTakeProfit());
					
					if (l_bool_4 == false)
						return (-1);
				}
			}
		}
	}
	
	return 0;
}

int Explorer::GetTimeControl(int ai_0, int ai_4) {
	if (Hour() >= ai_0 && Hour() <= ai_4)
		return 0;
		
	return (1);
}


}

