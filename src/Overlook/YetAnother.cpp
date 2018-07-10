#include "Overlook.h"

#if 0

// robot

namespace Overlook {

YetAnother::YetAnother() {
	
}

void YetAnother::InitEA() {
	
}

void YetAnother::StartEA(int pos) {
	double l_irsi_8;
	double l_irsi_16;
	double l_irsi_24;
	double l_imacd_32;
	double l_irsi_40;
	string ls_48;
	string ls_56;
	string ls_64;
	string ls_72;
	double l_price_80;
	double l_price_88;
	int l_ticket_120;
	int l_ticket_124;
	
	if (TrailingStop > 0)
		MoveTrailingStop();
		
	if (BreakEven > 0)
		MoveBreakEven();
		
	int li_0 = 1;
	
	for (int li_4 = 1; li_4 <= li_0; li_4++) {
		l_irsi_8 = iRSI(NULL, g_timeframe_100, g_period_104, PRICE_CLOSE, li_4);
		l_irsi_16 = iRSI(NULL, g_timeframe_100, g_period_104, PRICE_CLOSE, li_4);
		l_irsi_24 = iRSI(NULL, g_timeframe_100, g_period_104, PRICE_CLOSE, li_4 + 1);
		l_imacd_32 = iMACD(NULL, g_timeframe_100, g_period_188, g_period_192, g_period_196, PRICE_CLOSE, MODE_MAIN, 0);
		l_irsi_40 = iRSI(NULL, g_timeframe_176, g_period_156, PRICE_CLOSE, li_4);
		ls_48 = "false";
		ls_56 = "false";
		
		if (l_irsi_8 > gi_116 && l_irsi_8 < gi_120 && l_irsi_16 > l_irsi_24 && l_imacd_32 < gi_200 && l_irsi_40 < gi_168 && Open[gi_84] > Open[gi_88])
			ls_48 = "true";
			
		if (l_irsi_8 < gi_132 && l_irsi_8 > gi_136 && l_irsi_16 < l_irsi_24 && l_imacd_32 > gi_204 && l_irsi_40 > gi_172 && Open[gi_84] < Open[gi_88])
			ls_56 = "true";
			
		ls_64 = "false";
		
		ls_72 = "false";
		
		if (ls_48 == "true") {
			if (ReverseSystem)
				ls_72 = "true";
			else
				ls_64 = "true";
		}
		
		if (ls_56 == "true") {
			if (ReverseSystem)
				ls_64 = "true";
			else
				ls_72 = "true";
		}
	}
	
	if (RiskMM)
		CalculateMM();
		
	int li_unused_132 = 0;
	
	if (OrdersTotal() < MaxOrders) {
		if (ls_64 == "true" && NewBarBuy()) {
			if (HideSL == false && StopLoss > 0)
				l_price_80 = Ask - StopLoss * Point;
			else
				l_price_80 = 0;
				
			if (l_price_80 > 0.0 && l_price_80 > Bid - MarketInfo(Symbol(), MODE_STOPLEVEL) * Point)
				l_price_80 = Bid - MarketInfo(Symbol(), MODE_STOPLEVEL) * Point;
				
			if (HideTP == false && TakeProfit > 0)
				l_price_88 = Ask + TakeProfit * Point;
			else
				l_price_88 = 0;
				
			l_ticket_120 = OrderSend(Symbol(), OP_BUY, Lots, Ask, Slippage, l_price_80, l_price_88, "Robot", Magic, 0, Blue);
			
			if (Hedge)
				l_ticket_124 = OrderSend(Symbol(), OP_SELL, Lots, Bid, Slippage, l_price_88, l_price_80, "Robot", Magic, 0, Red);
		}
		
		if (ls_72 == "true" && NewBarSell()) {
			if (HideSL == false && StopLoss > 0)
				l_price_80 = Bid + StopLoss * Point;
			else
				l_price_80 = 0;
				
			if (l_price_80 > 0.0 && l_price_80 < Ask + MarketInfo(Symbol(), MODE_STOPLEVEL) * Point)
				l_price_80 = Ask + MarketInfo(Symbol(), MODE_STOPLEVEL) * Point;
				
			if (HideTP == false && TakeProfit > 0)
				l_price_88 = Bid - TakeProfit * Point;
			else
				l_price_88 = 0;
				
			l_ticket_120 = OrderSend(Symbol(), OP_SELL, Lots, Bid, Slippage, l_price_80, l_price_88, "Robot", Magic, 0, Red);
			
			if (Hedge)
				l_ticket_124 = OrderSend(Symbol(), OP_BUY, Lots, Ask, Slippage, l_price_88, l_price_80, "Robot", Magic, 0, Blue);
		}
	}
	
	if (Hedge == false) {
		if (ls_56 == "true" || (ReverseSystem == false && HideSL && (OrderOpenPrice() - Ask) / Point >= StopLoss) || (ReverseSystem == false && HideTP && (Bid - OrderOpenPrice()) / Point >= TakeProfit) ||
			(ReverseSystem && HideSL && (Bid - OrderOpenPrice()) / Point >= StopLoss) || (ReverseSystem && HideTP && (OrderOpenPrice() - Ask) / Point >= TakeProfit)) {
			if (ReverseSystem)
				CloseSellOrders(Magic);
			else
				CloseBuyOrders(Magic);
		}
		
		if (ls_48 == "true" || (ReverseSystem == false && HideSL && (Bid - OrderOpenPrice()) / Point >= StopLoss) || (ReverseSystem == false && HideTP && (OrderOpenPrice() - Ask) / Point >= TakeProfit) ||
			(ReverseSystem && HideSL && (OrderOpenPrice() - Ask) / Point >= StopLoss) || (ReverseSystem && HideTP && (Bid - OrderOpenPrice()) / Point >= TakeProfit)) {
			if (ReverseSystem)
				CloseBuyOrders(Magic);
			else
				CloseSellOrders(Magic);
		}
	}
	
	else {
		if ((HideSL && StopLoss > 0 && (OrderOpenPrice() - Ask) / Point >= StopLoss) || (HideTP && TakeProfit > 0 && (Bid - OrderOpenPrice()) / Point >= TakeProfit))
			CloseBuyOrders(Magic);
			
		if ((HideSL && StopLoss > 0 && (Bid - OrderOpenPrice()) / Point >= StopLoss) || (HideTP && TakeProfit > 0 && (OrderOpenPrice() - Ask) / Point >= TakeProfit))
			CloseSellOrders(Magic);
	}
	
	int li_unused_136 = 0;
	
	if (l_ticket_120 < 0) {
		if (GetLastError() == 134/* NOT_ENOUGH_MONEY */) {
			li_unused_136 = 1;
			Print("Not enough money!");
		}
		
		return (-1);
	}
	
	return (0);
}

int YetAnother::CloseBuyOrders(int a_magic_0) {
	int l_ord_delete_4;
	int l_cmd_16;
	int l_ord_total_8 = OrdersTotal();
	
	for (int l_pos_12 = l_ord_total_8 - 1; l_pos_12 >= 0; l_pos_12--) {
		OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == a_magic_0 && OrderSymbol() == Symbol()) {
			if (OrderType() == OP_BUY) {
				OrderClose(OrderTicket(), OrderLots(), Bid, 3);
				l_cmd_16 = OrderType();
				
				if (l_cmd_16 != OP_BUYLIMIT)
					if (l_cmd_16 != OP_BUYSTOP)
						continue;
						
				l_ord_delete_4 = OrderDelete(OrderTicket());
			}
		}
	}
	
	return (0);
}

int YetAnother::CloseSellOrders(int a_magic_0) {
	int l_ord_delete_4;
	int l_cmd_16;
	int l_ord_total_8 = OrdersTotal();
	
	for (int l_pos_12 = l_ord_total_8 - 1; l_pos_12 >= 0; l_pos_12--) {
		OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == a_magic_0 && OrderSymbol() == Symbol()) {
			if (OrderType() == OP_SELL) {
				OrderClose(OrderTicket(), OrderLots(), Ask, 3);
				l_cmd_16 = OrderType();
				
				if (l_cmd_16 != OP_SELLLIMIT)
					if (l_cmd_16 != OP_SELLSTOP)
						continue;
						
				l_ord_delete_4 = OrderDelete(OrderTicket());
			}
		}
	}
	
	return (0);
}

void YetAnother::MoveTrailingStop() {
	int l_ord_total_4 = OrdersTotal();
	
	for (int l_pos_0 = 0; l_pos_0 < l_ord_total_4; l_pos_0++) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == Magic) {
			if (OrderType() == OP_BUY) {
				if (TrailingStop <= 0)
					continue;
					
				if (!(NormalizeDouble(OrderStopLoss(), Digits) < NormalizeDouble(Bid - Point * (TrailingStop + TrailingStep), Digits) || OrderStopLoss() == 0.0))
					continue;
					
				OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Bid - Point * TrailingStop, Digits), OrderTakeProfit(), 0, Blue);
				
				return;
			}
			
			if (TrailingStop > 0) {
				if (NormalizeDouble(OrderStopLoss(), Digits) > NormalizeDouble(Ask + Point * (TrailingStop + TrailingStep), Digits) || OrderStopLoss() == 0.0) {
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Ask + Point * TrailingStop, Digits), OrderTakeProfit(), 0, Red);
					return;
				}
			}
		}
	}
}

void YetAnother::MoveBreakEven() {
	int l_ord_total_4 = OrdersTotal();
	
	for (int l_pos_0 = 0; l_pos_0 < l_ord_total_4; l_pos_0++) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == Magic) {
			if (OrderType() == OP_BUY) {
				if (BreakEven <= 0)
					continue;
					
				if (NormalizeDouble(Bid - OrderOpenPrice(), Digits) <= BreakEven * Point)
					continue;
					
				if (NormalizeDouble(OrderStopLoss() - OrderOpenPrice(), Digits) >= 0.0)
					continue;
					
				OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() + 0.0 * Point, Digits), OrderTakeProfit(), 0, Blue);
				
				return;
			}
			
			if (BreakEven > 0) {
				if (NormalizeDouble(OrderOpenPrice() - Ask, Digits) > BreakEven * Point) {
					if (NormalizeDouble(OrderOpenPrice() - OrderStopLoss(), Digits) < 0.0) {
						OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() - 0.0 * Point, Digits), OrderTakeProfit(), 0, Red);
						return;
					}
				}
			}
		}
	}
}

int YetAnother::NewBarBuy() {
	if (g_time_300 < Time[0]) {
		g_time_300 = Time[0];
		return (1);
	}
	
	return (0);
}

int YetAnother::NewBarSell() {
	if (g_time_304 < Time[0]) {
		g_time_304 = Time[0];
		return (1);
	}
	
	return (0);
}

void YetAnother::CalculateMM() {
	double l_minlot_0 = MarketInfo(Symbol(), MODE_MINLOT);
	double l_maxlot_8 = MarketInfo(Symbol(), MODE_MAXLOT);
	Lots = AccountFreeMargin() / 100000.0 * RiskPercent;
	Lots = MathMin(l_maxlot_8, MathMax(l_minlot_0, Lots));
	
	if (l_minlot_0 < 0.1)
		Lots = NormalizeDouble(Lots, 2);
	else {
		if (l_minlot_0 < 1.0)
			Lots = NormalizeDouble(Lots, 1);
		else
			Lots = NormalizeDouble(Lots, 0);
	}
	
	if (Lots < l_minlot_0)
		Lots = l_minlot_0;
		
	if (Lots > l_maxlot_8)
		Lots = l_maxlot_8;
}

}

#endif
