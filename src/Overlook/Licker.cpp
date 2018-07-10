#include "Overlook.h"

namespace Overlook {

Licker::Licker() {
	
}

void Licker::InitEA() {
	
	AddSubCore<ParabolicSAR>()
		.Set("step", gd_136)
		.Set("maximum", gd_144);
	
	AddSubCore<ParabolicSAR>()
		.Set("step", gd_120)
		.Set("maximum", gd_128);
	
	AddSubCore<ParabolicSAR>()
		.Set("step", gd_120)
		.Set("maximum", gd_128);
	
	AddSubCore<Channel>()
		.Set("period", gi_152);
	
}

void Licker::StartEA(int pos) {
	g_point_176 = Point;
	
	if (pos < gi_152)
		return;
	
	if (Digits == 3 || Digits == 5)
		g_point_176 = NormalizeDouble(10.0 * Point, Digits);
		
	bool l_bool_4 = RiskManagement;
	
	if (l_bool_4) {
		if (RiskPercent < 0.1 || RiskPercent > 100.0) {
			Comment("Invalid Risk Value.");
			return;
		}
		
		lot = MathFloor(100.0 * (AccountFreeMargin() * AccountLeverage() * RiskPercent * Point) / (Ask * MarketInfo(Symbol(), MODE_LOTSIZE) * MarketInfo(Symbol(), MODE_MINLOT))) * MarketInfo(Symbol(), MODE_MINLOT);
	}
	
	if (l_bool_4 == false) {
	}
	
	//if (g_bars_192 != Bars && (gettimecontrol(starttime, stoptime) != 1 && timecontrol == 1) || timecontrol == 0) {
	g_bars_192 = Bars;
	gi_196 = ScalpingPattern();
	//}
	
	if (TrailingStop > 0)
		gi_196 = GetTrailingStop();
		
	if (gi_112 > 0)
		gi_196 = BBU();
	
}

int Licker::ScalpingPattern() {
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
	double l_isar_76 = At(2).GetBuffer(0).Get(pos);
	ConstBuffer& low_buf = GetInputBuffer(0, 1);
	ConstBuffer& high_buf = GetInputBuffer(0, 2);
	
	if (l_isar_68 > Bid && l_isar_60 < Bid)
		gi_200 = true;
		
	if (l_isar_60 > Bid)
		gi_200 = false;
		
	if (l_isar_68 < Bid && l_isar_60 > Bid)
		gi_204 = true;
		
	if (l_isar_60 < Bid)
		gi_204 = false;
		
	if (l_isar_60 < Bid && l_isar_68 < Bid && gi_200 == true) {
		gi_200 = false;
		ld_36 = MaximumMinimum(0, gi_152);
		l_high_28 = high_buf.Get(pos-1);
		ld_44 = GetFiboUr(l_high_28, ld_36, gd_160 / 100.0);
		ld_52 = GetFiboUr(l_high_28, ld_36, gd_168 / 100.0);
		ld_4 = ld_44;
		l_price_12 = ld_36 - gi_156 * g_point_176;
		l_price_20 = ld_52;
		
		if (Ask - ld_4 < 5.0 * g_point_176 || Ask - l_price_12 < 5.0 * g_point_176 || l_price_20 - Ask < 5.0 * g_point_176)
			return (0);
			
		l_ticket_0 = OrderSend(Symbol(), OP_BUYLIMIT, lot, NormalizeDouble(ld_4, Digits), 3, l_price_12, l_price_20, "Slasher", MG);
		
		if (l_ticket_0 < 0) {
			return (-1);
		}
		
		gi_unused_184 = 0;
	}
	
	if (l_isar_60 > Bid && l_isar_68 > Bid && gi_204 == true) {
		gi_204 = false;
		l_high_28 = MaximumMinimum(1, gi_152);
		ld_36 = low_buf.Get(pos - 1);
		ld_44 = GetFiboUr(ld_36, l_high_28, gd_160 / 100.0);
		ld_52 = GetFiboUr(ld_36, l_high_28, gd_168 / 100.0);
		ld_4 = ld_44;
		l_price_12 = l_high_28 + gi_156 * g_point_176;
		l_price_20 = ld_52;
		
		if (ld_4 - Ask < 5.0 * g_point_176 || l_price_12 - Ask < 5.0 * g_point_176 || Ask - l_price_20 < 5.0 * g_point_176)
			return (0);
			
		l_ticket_0 = OrderSend(Symbol(), OP_SELLLIMIT, lot, NormalizeDouble(ld_4, Digits), 3, l_price_12, l_price_20, "Slasher", MG);
		
		if (l_ticket_0 < 0) {
			return (-1);
		}
		
		gi_unused_188 = 0;
	}
	
	if (l_isar_68 > Bid && ChLimitOrder(1) > 0)
		l_ticket_0 = deletelimitorder(1);
		
	if (l_isar_68 < Bid && ChLimitOrder(0) > 0)
		l_ticket_0 = deletelimitorder(0);
		
	return (0);
}

int Licker::deletelimitorder(int ai_0) {
	int l_ord_delete_8;
	
	for (int li_4 = 1; li_4 <= OrdersTotal(); li_4++) {
		if (OrderSelect(li_4 - 1, SELECT_BY_POS) == true) {
			if (OrderType() == OP_BUYLIMIT && OrderSymbol() == Symbol() && ai_0 == 1 && OrderMagicNumber() == MG)
				l_ord_delete_8 = OrderDelete(OrderTicket());
				
			if (OrderType() == OP_SELLLIMIT && OrderSymbol() == Symbol() && ai_0 == 0 && OrderMagicNumber() == MG)
				l_ord_delete_8 = OrderDelete(OrderTicket());
		}
	}
	
	return (l_ord_delete_8);
}

int Licker::ChLimitOrder(int ai_0) {
	for (int li_4 = 1; li_4 <= OrdersTotal(); li_4++) {
		if (OrderSelect(li_4 - 1, SELECT_BY_POS) == true) {
			if (OrderType() == OP_BUYLIMIT && OrderSymbol() == Symbol() && ai_0 == 1 && OrderMagicNumber() == MG)
				return (1);
				
			if (OrderType() == OP_SELLLIMIT && OrderSymbol() == Symbol() && ai_0 == 0 && OrderMagicNumber() == MG)
				return (1);
		}
	}
	
	return (0);
}

double Licker::MaximumMinimum(int ai_0, int ai_4) {
	double ld_ret_16;
	int li_8 = 0;
	bool li_12 = false;
	
	ConstBuffer& ll_buf = At(3).GetBuffer(0);
	ConstBuffer& hh_buf = At(3).GetBuffer(1);
	
	if (ai_0 == 0) {
		while (li_12 == false) {
			ld_ret_16 = ll_buf.Get(pos - li_8);
			
			double d = ll_buf.Get(pos - li_8 - ai_4);
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
			ld_ret_16 = hh_buf.Get(pos - li_8);
			
			double d = hh_buf.Get(pos - li_8 - ai_4);
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
	
	return (0);
}

double Licker::GetFiboUr(double ad_0, double ad_8, double ad_16) {
	double ld_28;
	int l_digits_24 = MarketInfo(Symbol(), MODE_DIGITS);
	ld_28 = NormalizeDouble(ad_8 + (ad_0 - ad_8) * ad_16, l_digits_24);
	return (ld_28);
}

int Licker::GetTrailingStop() {
	bool l_bool_4;
	
	for (int li_0 = 1; li_0 <= OrdersTotal(); li_0++) {
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (TrailingStop > 0 && OrderType() == OP_BUY && OrderSymbol() == Symbol() && OrderMagicNumber() == MG) {
				if (Bid - OrderOpenPrice() >= TrailingStop * g_point_176 && TrailingStop > 0 && Bid - g_point_176 * TrailingStop > OrderStopLoss()) {
					if (Bid - g_point_176 * TrailingStop - OrderStopLoss() >= gi_116 * g_point_176) {
						Print("");
						l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), Bid - g_point_176 * TrailingStop, OrderTakeProfit());
						
						if (l_bool_4 == false)
							return (-1);
					}
				}
			}
		}
		
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (OrderType() == OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == MG) {
				if (OrderOpenPrice() - Ask >= TrailingStop * g_point_176 && TrailingStop > 0 && OrderStopLoss() > Ask + TrailingStop * g_point_176) {
					if (OrderStopLoss() - (Ask + TrailingStop * g_point_176) > gi_116 * g_point_176) {
						Print("");
						l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), Ask + TrailingStop * g_point_176, OrderTakeProfit());
						
						if (l_bool_4 == false)
							return (-1);
					}
				}
			}
		}
	}
	
	return (0);
}

int Licker::BBU() {
	bool l_bool_4;
	
	for (int li_0 = 1; li_0 <= OrdersTotal(); li_0++) {
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (gi_112 > 0 && OrderType() == OP_BUY && OrderSymbol() == Symbol() && OrderStopLoss() < OrderOpenPrice() && OrderMagicNumber() == MG) {
				if (Bid - OrderOpenPrice() >= gi_112 * g_point_176 && gi_112 > 0) {
					Print("");
					l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + 1.0 * g_point_176, OrderTakeProfit());
					
					if (l_bool_4 == false)
						return (-1);
				}
			}
		}
		
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (gi_112 > 0 && OrderMagicNumber() == MG && OrderType() == OP_SELL && OrderSymbol() == Symbol() && OrderStopLoss() > OrderOpenPrice() || OrderStopLoss() == 0.0) {
				if (OrderOpenPrice() - Ask >= gi_112 * g_point_176 && gi_112 > 0) {
					Print("");
					l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - 1.0 * g_point_176, OrderTakeProfit());
					
					if (l_bool_4 == false)
						return (-1);
				}
			}
		}
	}
	
	return (0);
}

int Licker::gettimecontrol(int ai_0, int ai_4) {
	if (Hour() >= ai_0 && Hour() <= ai_4)
		return (0);
		
	return (1);
}

}
