#include "Overlook.h"

#if 0

// stop trade

namespace Overlook {

Starter::Starter() {

}

void Starter::InitEA() {
	gi_unused_144 = 0;
	gi_unused_148 = 0;
	gd_136 = Ask - Bid;
	Comment("");
}

void Starter::StartEA(int pos) {
	double ld_16;
	double l_ord_lots_24;
	double l_ord_profit_32;
	double l_lotstep_40;
	double l_bid_48;
	/*if (AccountNumber() != gi_76 && !IsDemo()) {
	   Comment("Ñîâåòíèê ìîæåò ðàáîòàòü òîëüêî íà ñ÷¸òå " + gi_76 + ", äëÿ áåñïëàòíîãî ïîäêëþ÷åíèÿ ê äðóãîìó ñ÷¸òó ïåðåéäèòå íà ñàéò forex-investor.net");
	   return;
	}
	if ((!IsOptimization() && !IsTesting() && !IsVisualMode()) || (ShowTableOnTesting && IsTesting() && !IsOptimization())) {
	   DrawLogo();
	   DrawStats();
	}*/
	gd_124 = LotsOptimized();
	int li_0 = TotalBuy();
	int li_4 = TotalSell();
	int li_8 = BuyStopsTotal();
	int li_12 = SellStopsTotal();
	
	if (li_0 == 0 && li_4 == 0 && li_8 == 0 && li_12 == 0) {
		GetLastOrderParameters(l_ord_lots_24, l_ord_profit_32);
		
		if (ND(l_ord_profit_32) >= 0.0)
			ld_16 = gd_124;
		else {
			l_lotstep_40 = MarketInfo(Symbol(), MODE_LOTSTEP);
			
			if (l_lotstep_40 == 0.1)
				ld_16 = NormalizeDouble(l_ord_lots_24 * Multiplier, 1);
			else
				ld_16 = NormalizeDouble(l_ord_lots_24 * Multiplier, 2);
		}
		
		l_bid_48 = Bid;
		
		SetBuyStop(ND(l_bid_48 + Distance * Point), ld_16);
		SetSellStop(ND(l_bid_48 - Distance * Point), ld_16);
	}
	
	else {
		DoTrail();
		
		if ((li_8 != 0 && li_12 == 0) || (li_8 == 0 && li_12 != 0))
			DeleteAllPending();
	}
	
	return (0);
}

int Starter::TotalBuy() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (!(OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderType() == OP_BUY && OrderMagicNumber() == Magic)
			l_count_0++;
	}
	
	return (l_count_0);
}

int Starter::TotalSell() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (!(OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderType() == OP_SELL && OrderMagicNumber() == Magic)
			l_count_0++;
	}
	
	return (l_count_0);
}

int Starter::BuyStopsTotal() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (!(OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_BUYSTOP)
			l_count_0++;
	}
	
	return (l_count_0);
}

int Starter::SellStopsTotal() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (!(OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_SELLSTOP)
			l_count_0++;
	}
	
	return (l_count_0);
}

void Starter::DeleteAllPending() {
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		if (!(OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_SELLSTOP || OrderType() == OP_BUYSTOP) {
			while (IsTradeContextBusy())
				Sleep(500);
				
			Print("Óäàëÿåì îðäåð #" + OrderTicket() + ".");
			
			if (!OrderDelete(OrderTicket()))
				Print("Íå óäàëîñü óäàëèòü îðäåð #" + OrderTicket() + ". Îøèáêà: " + ErrorDescription(GetLastError()));
		}
	}
}

void Starter::SetSellStop(double a_price_0, double a_lots_8) {
	int l_error_36;
	double l_price_16 = a_price_0 + StopLoss * Point;
	double l_price_24 = a_price_0 - TakeProfit * Point;
	int l_ticket_32 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_8, a_price_0, g_slippage_132, l_price_16, l_price_24, "", Magic, 0, Red);
	
	if (l_ticket_32 < 1) {
		l_error_36 = GetLastError();
		Print("Íå óäàëîñü óñòàíîâèòü îòëîæåííûé îðäåð SELLSTOP îáúåìîì " + a_lots_8 + ". Îøèáêà #" + l_error_36 + " " + ErrorDescription(l_error_36));
		Print(Bid + " " + a_price_0 + " " + l_price_16 + " " + l_price_24);
	}
}

void Starter::SetBuyStop(double a_price_0, double a_lots_8) {
	int l_error_36;
	double l_price_16 = a_price_0 - StopLoss * Point;
	double l_price_24 = a_price_0 + TakeProfit * Point;
	int l_ticket_32 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_8, a_price_0, g_slippage_132, l_price_16, l_price_24, "", Magic, 0, Blue);
	
	if (l_ticket_32 < 1) {
		l_error_36 = GetLastError();
		Print("Íå óäàëîñü óñòàíîâèòü îòëîæåííûé îðäåð BUYSTOP îáúåìîì " + a_lots_8 + ". Îøèáêà #" + l_error_36 + " " + ErrorDescription(l_error_36));
		Print(Bid + " " + a_price_0 + " " + l_price_16 + " " + l_price_24);
	}
}

double Starter::ND(double ad_0) {
	return (NormalizeDouble(ad_0, Digits));
}

void Starter::GetLastOrderParameters(double &a_ord_lots_0, double &a_ord_profit_8) {
	a_ord_profit_8 = 0;
	int l_datetime_16 = 0;
	
	for (int l_pos_20 = 0; l_pos_20 < OrdersHistoryTotal(); l_pos_20++) {
		if (OrderSelect(l_pos_20, SELECT_BY_POS, MODE_HISTORY)) {
			if (OrderSymbol() == Symbol() && OrderType() == OP_BUY || OrderType() == OP_SELL && OrderMagicNumber() == Magic) {
				if (OrderOpenTime() > l_datetime_16) {
					a_ord_profit_8 = OrderProfit();
					a_ord_lots_0 = OrderLots();
					l_datetime_16 = OrderOpenTime();
				}
			}
		}
	}
}

void Starter::DoTrail() {
	for (int l_pos_0 = 0; l_pos_0 < OrdersTotal(); l_pos_0++) {
		if (!OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES))
			Print("Íå ïîëó÷èëîñü âûáðàòü îðäåð. Îøèáêà " + ErrorDescription(GetLastError()));
		else {
			if (OrderSymbol() == Symbol()) {
				if (OrderType() == OP_BUY) {
					if (OrderMagicNumber() == Magic) {
						if (Bid - TrailingStop * Point > OrderOpenPrice() && Bid - TrailingStop * Point - OrderStopLoss() > TrailingStep * Point || OrderStopLoss() == 0.0)
							if (!OrderModify(OrderTicket(), OrderOpenPrice(), Bid - TrailingStop * Point, OrderTakeProfit(), OrderExpiration(), Pink))
								Print("Íå ïîëó÷èëîñü ìîäèôèöèðîâàòü îðäåð ¹" + OrderTicket() + ". Îøèáêà " + ErrorDescription(GetLastError()));
					}
				}
				
				else {
					if (OrderType() == OP_SELL) {
						if (OrderMagicNumber() == Magic) {
							if (Ask + TrailingStop * Point < OrderOpenPrice() && OrderStopLoss() - (Ask + TrailingStop * Point) > TrailingStep * Point || OrderStopLoss() == 0.0)
								if (!OrderModify(OrderTicket(), OrderOpenPrice(), Ask + TrailingStop * Point, OrderTakeProfit(), OrderExpiration(), Pink))
									Print("Íå ïîëó÷èëîñü ìîäèôèöèðîâàòü îðäåð ¹" + OrderTicket() + ". Îøèáêà " + ErrorDescription(GetLastError()));
						}
					}
				}
			}
		}
	}
}

double Starter::GetProfitForDay(int ai_0) {
	double ld_ret_4 = 0;
	
	for (int l_pos_12 = 0; l_pos_12 < OrdersHistoryTotal(); l_pos_12++) {
		if (!(OrderSelect(l_pos_12, SELECT_BY_POS, MODE_HISTORY)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic)
			if (OrderCloseTime() >= iTime(Symbol(), PERIOD_D1, ai_0) && OrderCloseTime() < iTime(Symbol(), PERIOD_D1, ai_0) + 86400)
				ld_ret_4 += OrderProfit();
	}
	
	return (ld_ret_4);
}

double Starter::LotsOptimized() {
	double l_lotstep_0 = MarketInfo(Symbol(), MODE_LOTSTEP);
	double l_minlot_8 = MarketInfo(Symbol(), MODE_MINLOT);
	int li_16 = 0;
	
	if (l_lotstep_0 == 0.01)
		li_16 = 2;
	else
		li_16 = 1;
		
	double l_minlot_20 = NormalizeDouble(MathFloor(AccountBalance() * RiskPercent / 100.0) / 10000.0, li_16);
	
	if (l_minlot_20 < l_minlot_8)
		l_minlot_20 = l_minlot_8;
		
	return (l_minlot_20);
}

}

#endif
