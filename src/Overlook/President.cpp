#include "Overlook.h"

#if 0

namespace Overlook {

// fx ultimate

President::President() {
	
}

void President::InitEA() {
	g_lots_188 = Lots;
	
	if (Digits == 5) {
		gi_156 = 10 * gi_156;
		gi_160 = 10 * gi_160;
	}
	
	return (0);
}

void President::StartEA(int pos) {
	/*
	gs_164 = AccountNumber() + IsDemo() + 1000;
	if (MD5(gs_164) != password) {
	   Print("=========================================");
	   Print("Password is incorrect");
	   Print("AccountNumber: " + AccountNumber());
	   Print("AccountNumber IsDemo: " + gs_164);
	   Print("Password: " + password);
	   Print("=========================================");
	   return (0);
	}
	*/
	
	if (Aggr_level == 0)
		Aggr_level = 24;
	else {
		if (Aggr_level == 1)
			Aggr_level = 5;
		else
			Aggr_level = 24;
	}
	
	LotHandle();
	
	if (StealthMode == true) {
		if (TimeCurrent() <= gi_172)
			return (0);
			
		gi_172 = TimeCurrent() + 3420;
	}
	
	else {
		if (Now == gi_172)
			return (0);
			
		gi_172 = Now;
		
		if (!IsTradeAllowed()) {
			gi_172 = Time[1];
			MathSrand(TimeCurrent());
			Sleep(MathRand() + 30000); //wait randomN + 30 seconds
		}
	}
	
	if (TradeOnFridays == false && TimeDayOfWeek(TimeCurrent()) == 5)
		return (0);
		
	if (In_BUY && MA_Signal() == 1)
		Trade_BUY();
		
	if (In_SELL && MA_Signal() == 2)
		Trade_SELL();
		
	return (0);
}

void President::LotHandle() {
	if (MM == true) {
		gi_196 = AccountBalance() / 1000.0;
		
		if (gi_196 == false)
			gi_196 = true;
	}
	
	else
		gi_196 = true;
		
	if (RecoveryMode == true) {
		if (LastClosedProfit() < 0.0) {
			Lots = 2.0 * g_lots_188 * gi_196;
			return;
		}
		
		Lots = g_lots_188 * gi_196;
		
		return;
	}
	
	Lots = g_lots_188 * gi_196;
}

int President::MA_Signal() {
	double l_ima_0 = iMA(Symbol(), 0, Period_Filtring, 0, MODE_EMA, PRICE_CLOSE, 1); //MAperiod 0.18
	double l_ima_8 = iMA(Symbol(), 0, Period_Filtring, 0, MODE_EMA, PRICE_CLOSE, 2); //MAperiod 0.18
	
	if (l_ima_0 > l_ima_8)
		return (1);
		
	if (l_ima_0 < l_ima_8)
		return (2);
		
	return (0);
}


void President::Trade_BUY() {
	int l_ord_total_0 = OrdersTotal();
	
	for (int l_pos_4 = 0; l_pos_4 < l_ord_total_0; l_pos_4++) {
		OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_180)
			return;
	}
	
	g_ticket_176 = -1;
	
	double l_price_8 = Out(Aggr_level, f1);
	
	if (l_price_8 > Bid + 15 * gi_156 * Point && IsTradeAllowed()) {
		g_ticket_176 = OrderSend(Symbol(), OP_BUY, lot(gi_152), Ask, 5, Bid - gi_160 * SL_bs * Point, l_price_8, 0, g_magic_180, 0, Blue);
		RefreshRates();
		
		if (g_ticket_176 < 0) {
			Sleep(30000);
			gi_172 = Time[1];
		}
	}
}

void President::Trade_SELL() {
	int l_ord_total_0 = OrdersTotal();
	
	for (int l_pos_4 = 0; l_pos_4 < l_ord_total_0; l_pos_4++) {
		OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_184)
			return;
	}
	
	g_ticket_176 = -1;
	
	double l_price_8 = Out(Aggr_level, f2);
	
	if (l_price_8 < Ask - 15 * gi_156 * Point && IsTradeAllowed()) {
		g_ticket_176 = OrderSend(Symbol(), OP_SELL, lot(gi_152), Bid, 5, Ask + gi_160 * SL_bs * Point, l_price_8, 0, g_magic_184, 0, Red);
		RefreshRates();
		
		if (g_ticket_176 < 0) {
			Sleep(30000);
			gi_172 = Time[1];
		}
	}
}

double President::Out(int a_period_0, double ad_4) {
	double lda_12[1000];
	double lda_16[1000];
	double l_ima_20 = iMA(NULL, 0, a_period_0, 0, MODE_LWMA, PRICE_CLOSE, 1); //a_period_0 = Aggr_level = 5 or 24
	double l_ima_28 = iMA(NULL, 0, a_period_0, 0, MODE_SMA, PRICE_CLOSE, 1);
	lda_12[a_period_0] = (6.0 * l_ima_20 - 6.0 * l_ima_28) / (a_period_0 - 1);
	lda_16[a_period_0] = 4.0 * l_ima_28 - 3.0 * l_ima_20 - lda_12[a_period_0];
	
	for (int li_36 = a_period_0 - 1; li_36 > 0; li_36--) {
		lda_16[li_36] = ad_4 * Close[li_36] + (1 - ad_4) * (lda_16[li_36 + 1] + (lda_12[li_36 + 1]));
		lda_12[li_36] = ad_4 * (lda_16[li_36] - (lda_16[li_36 + 1])) + (1 - ad_4) * (lda_12[li_36 + 1]);
	}
	
	return (NormalizeDouble(lda_16[1] + lda_12[1], MarketInfo(Symbol(), MODE_DIGITS)));
}

double President::lot(int ai_0) {
	int li_20;
	double l_lots_24;
	double l_minlot_4 = MarketInfo(Symbol(), MODE_MINLOT);
	double l_maxlot_12 = MarketInfo(Symbol(), MODE_MAXLOT);
	
	if (Lots == 0.0) {
		li_20 = MathAbs(MathLog(l_minlot_4) / 2.0) + 0.5;
		l_lots_24 = NormalizeDouble(0.00001 * AccountFreeMargin() * ai_0, li_20);
		
		if (AccountFreeMargin() < l_lots_24 * MarketInfo(Symbol(), MODE_MARGINREQUIRED))
			l_lots_24 = NormalizeDouble(AccountFreeMargin() / MarketInfo(Symbol(), MODE_MARGINREQUIRED), li_20);
	}
	
	else
		l_lots_24 = Lots;
		
	return (MathMax(MathMin(l_lots_24, l_maxlot_12), l_minlot_4));
}

double President::LastClosedProfit() {
	int l_ticket_0 = LastClosedTicket();
	
	if (l_ticket_0 > 0)
		if (OrderSelect(l_ticket_0, SELECT_BY_TICKET, MODE_HISTORY))
			return (OrderProfit());
			
	return (0.0);
}

int President::LastClosedTicket() {
	int l_datetime_0 = 0;
	int l_ticket_4 = -1;
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersHistoryTotal(); l_pos_8++) {
		if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_HISTORY)) {
			if (OrderType() <= OP_SELL) {
				if (OrderCloseTime() > l_datetime_0) {
					l_datetime_0 = OrderCloseTime();
					l_ticket_4 = OrderTicket();
				}
			}
		}
	}
	
	return (l_ticket_4);
}

}

#endif
