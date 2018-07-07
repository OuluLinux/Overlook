#include "Overlook.h"


namespace Overlook {

Spiral::Spiral() {
	
}

void Spiral::InitEA() {
	double ld_0;
	g_bars_144 = Bars;
	
	if (use_bars_counter)
		gi_unused_148 = 0;
	else
		gi_unused_148 = 1;
		
	int l_digits_8 = MarketInfo(Symbol(), MODE_DIGITS);
	
	if (l_digits_8 > 4) {
		ld_0 = MathPow(10, l_digits_8 - 4);
		StopLoss = StopLoss * ld_0;
		TakeProfit = TakeProfit * ld_0;
		TrailingStop = TrailingStop * ld_0;
	}
	
	if (MM)
		Lots = CalculateMM(Risk);
	
	AddSubCore<Momentum>()
		.Set("period", mom_period);
	
	AddSubCore<RelativeStrengthIndex>()
		.Set("period", rsi_period);
	
	AddSubCore<AverageDirectionalMovement>()
		.Set("period", adx_period);
	
	AddSubCore<WilliamsPercentRange>()
		.Set("period", wpr_period);
	
	
}

void Spiral::StartEA(int pos) {
	int l_ticket_0;
	double l_price_4;
	double l_price_12;
	int li_20 = 0;
	
	if (use_bars_counter && Bars != g_bars_144)
		is_recent = false;
		
	int l_ord_total_24 = OrdersTotal();
	
	li_20 = 0;
	
	
	double l_imomentum_28 = At(0).GetBuffer(0).Get(pos);
	double l_irsi_36 = At(1).GetBuffer(0).Get(pos);
	double l_iadx_44 = At(2).GetBuffer(0).Get(pos);
	double l_iwpr_52 = At(3).GetBuffer(0).Get(pos);
	
	
	if (g_ticket_172 != -1 && OrderSelect(g_ticket_172, SELECT_BY_TICKET)) {
		if (true) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY) {
					if (UseTrailingStop && TrailingStop > 0) {
						if (Bid - OrderOpenPrice() > Point * TrailingStop) {
							if (use_manual_sltp) {
								if (price0 < Bid - Point * TrailingStop) {
									price0 = Bid - Point * TrailingStop;
									
									if (!use_bars_counter)
										g_bars_144 = Bars;
								}
							}
							
							else {
								if (OrderStopLoss() < Bid - Point * TrailingStop) {
									OrderModify(OrderTicket(), OrderOpenPrice(), Bid - Point * TrailingStop, OrderTakeProfit());
									
									if (!use_bars_counter)
										g_bars_144 = Bars;
								}
							}
						}
					}
					
					if (use_manual_sltp && Bid <= price0 || Bid >= price1) {
						if (OrderClose(OrderTicket(), OrderLots(), Bid, Slippage)) {
							price0 = 0;
							price1 = 0;
							g_ticket_172 = -1;
						}
						
							
						if (!use_bars_counter)
							g_bars_144 = Bars;
					}
				}
				
				else {
					if (OrderType() == OP_SELL) {
						if (UseTrailingStop && TrailingStop > 0) {
							if (OrderOpenPrice() - Ask > Point * TrailingStop) {
								if (use_manual_sltp) {
									if (price0 > Ask + Point * TrailingStop || price0 == 0.0) {
										price0 = Ask + Point * TrailingStop;
										
										if (!use_bars_counter)
											g_bars_144 = Bars;
									}
								}
								
								else {
									if (OrderStopLoss() > Ask + Point * TrailingStop || OrderStopLoss() == 0.0) {
										OrderModify(OrderTicket(), OrderOpenPrice(), Ask + Point * TrailingStop, OrderTakeProfit());
										
										if (!use_bars_counter)
											g_bars_144 = Bars;
									}
								}
							}
						}
						
						if (use_manual_sltp && Bid >= price0 || Bid <= price1) {
							if (OrderClose(OrderTicket(), OrderLots(), Ask, Slippage)) {
								price0 = 0;
								price1 = 0;
								g_ticket_172 = -1;
							}
							
							if (!use_bars_counter)
								g_bars_144 = Bars;
						}
					}
				}
			}
		}
	}
	
	bool li_60 = false;
	
	if (g_ticket_172 != -1) {
		if (OrderSelect(g_ticket_172, SELECT_BY_TICKET)) {
			if (true)
				li_60 = true;
			else {
				g_ticket_172 = -1;
				price0 = 0;
				price1 = 0;
			}
		}
	}
	
	if (l_irsi_36 < 30.0 && l_imomentum_28 < 100.0 && l_iadx_44 > 21.0 && l_iwpr_52 < -80.99)
		li_20 = 2;
		
	if (l_irsi_36 > 70.0 && l_imomentum_28 > 100.0 && l_iadx_44 < 21.0 && l_iwpr_52 > -20.99)
		li_20 = 1;
		
	if (li_20 == 1 && (use_bars_counter && !is_recent) || (!use_bars_counter && Bars != g_bars_144)) {
		if (!li_60) {
			if (AccountFreeMargin() < 1000.0 * Lots) {
				Print("We have no money. Free Margin = " + DblStr(AccountFreeMargin()));
				return;
			}
			
			if (use_sl)
				l_price_4 = Ask - StopLoss * Point;
			else
				l_price_4 = 0.0;
				
			if (use_tp)
				l_price_12 = Ask + TakeProfit * Point;
			else
				l_price_12 = 0.0;
				
			price0 = l_price_4;
			
			price1 = l_price_12;
			
			if (use_manual_sltp)
				l_ticket_0 = OrderSend(Symbol(), OP_BUY, Lots, Ask, Slippage, 0, 0, "", MagicNumber);
			else
				l_ticket_0 = OrderSend(Symbol(), OP_BUY, Lots, Ask, Slippage, l_price_4, l_price_12, "", MagicNumber);
				
			if (l_ticket_0 > 0) {
				if (OrderSelect(l_ticket_0, SELECT_BY_TICKET, MODE_TRADES)) {
					Print("BUY order opened : " + DblStr(OrderOpenPrice()));
											
					g_ticket_172 = l_ticket_0;
				}
				
				else
					Print("Error opening BUY order : " + GetLastError());
			}
			
			if (use_bars_counter)
				is_recent = true;
				
			if (!use_bars_counter)
				g_bars_144 = Bars;
				
			return;
		}
	}
	
	if (li_20 == 2 && (use_bars_counter && !is_recent) || (!use_bars_counter && Bars != g_bars_144)) {
		if (!li_60) {
			if (AccountFreeMargin() < 1000.0 * Lots) {
				Print("We have no money. Free Margin = " + DblStr(AccountFreeMargin()));
				return;
			}
			
			if (use_sl)
				l_price_4 = Bid + StopLoss * Point;
			else
				l_price_4 = 0.0;
				
			if (use_tp)
				l_price_12 = Bid - TakeProfit * Point;
			else
				l_price_12 = 0.0;
				
			price0 = l_price_4;
			
			price1 = l_price_12;
			
			if (use_manual_sltp)
				l_ticket_0 = OrderSend(Symbol(), OP_SELL, Lots, Bid, Slippage, 0, 0, "", MagicNumber);
			else
				l_ticket_0 = OrderSend(Symbol(), OP_SELL, Lots, Bid, Slippage, l_price_4, l_price_12, "", MagicNumber);
				
			if (l_ticket_0 > 0) {
				if (OrderSelect(l_ticket_0, SELECT_BY_TICKET, MODE_TRADES)) {
					Print("SELL order opened");
											
					g_ticket_172 = l_ticket_0;
				}
				
				else
					Print("Error opening SELL order : " + GetLastError());
			}
			
			if (use_bars_counter)
				is_recent = true;
				
			if (!use_bars_counter)
				g_bars_144 = Bars;
				
			return;
		}
	}
	
	if (!use_bars_counter)
		g_bars_144 = Bars;
	
}

double Spiral::CalculateMM(double ad_0) {
	double l_minlot_8 = MarketInfo(Symbol(), MODE_MINLOT);
	double l_maxlot_16 = MarketInfo(Symbol(), MODE_MAXLOT);
	double ld_ret_24 = AccountFreeMargin() / MarketInfo(Symbol(), MODE_MARGINREQUIRED) * ad_0;
	ld_ret_24 = MathMin(l_maxlot_16, MathMax(l_minlot_8, ld_ret_24));
	
	if (l_minlot_8 < 0.1)
		ld_ret_24 = NormalizeDouble(ld_ret_24, 2);
	else {
		if (l_minlot_8 < 1.0)
			ld_ret_24 = NormalizeDouble(ld_ret_24, 1);
		else
			ld_ret_24 = NormalizeDouble(ld_ret_24, 0);
	}
	
	if (ld_ret_24 < l_minlot_8)
		ld_ret_24 = l_minlot_8;
		
	if (ld_ret_24 > l_maxlot_16)
		ld_ret_24 = l_maxlot_16;
		
	return (ld_ret_24);
}

}

