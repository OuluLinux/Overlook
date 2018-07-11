#include "Overlook.h"

// scalp investor

namespace Overlook {

Sculptor::Sculptor() {

}

void Sculptor::InitEA() {
	String ls_0;
	String ls_unused_8;
	String ls_unused_16;
	/*if (!(IsTesting() || IsOptimization() || IsDemo())) {
	   if (AccountNumber() != 69803) {
	      ls_0 = "Äàííàÿ êîïèÿ ñîâåòíèêà íå ÿâëÿåòñÿ ëèöåíçèîííîé. Äàëüíåéøàÿ ðàáîòà íåâîçìîæíà.";
	      Print(ls_0);
	      MessageBox(ls_0, "Îøèáêà", MB_ICONHAND);
	      gi_368 = true;
	      Sleep(86400000);
	   }
	}*/
	
	if (gi_196)
		g_lots_200 = 100000.0 * Lots;
	else
		g_lots_200 = Lots;
		
}

void Sculptor::StartEA(int pos) {
	if (pos < 1)
		return;
	
	CompatBuffer Low(GetInputBuffer(0, 1), pos, 0);
	CompatBuffer High(GetInputBuffer(0, 2), pos, 0);
	
	double l_price_0;
	double l_price_8;
	double l_price_16;
	double l_price_24;
	double l_price_32;
	double l_lots_40;
	double l_pips_48;
	double l_pips_56;
	//if (!myCheckAllowWorking()) return (0);
	double ld_64 = (High[1] - Low[1]) / Point;
	int l_ticket_72 = 0;
	int l_ticket_76 = 0;
	int l_ticket_80 = 0;
	int l_ticket_84 = 0;
	int l_ticket_88 = 0;
	int l_ticket_92 = 0;
	double l_ord_lots_96 = 0;
	double l_ord_lots_104 = 0;
	double l_ord_open_price_112 = 0;
	double l_ord_open_price_120 = 0;
	double l_ord_takeprofit_128 = 0;
	double l_ord_takeprofit_136 = 0;
	double ld_unused_144 = 0;
	double l_ord_lots_152 = 0;
	double ld_unused_160 = 0;
	double l_ord_lots_168 = 0;
	double ld_176 = 0;
	double ld_184 = 0;
	gi_192 = false;
	
	if (gi_140 == true) {
		switch (DayOfWeek()) {
		
		case 0:
		
			if (Sunday == true)
				gi_192 = true;
				
			break;
			
		case 1:
			if (Monday == true)
				gi_192 = true;
				
			break;
			
		case 2:
			if (Tuesday == true)
				gi_192 = true;
				
			break;
			
		case 3:
			if (Wednesday == true)
				gi_192 = true;
				
			break;
			
		case 4:
			if (Thursday == true)
				gi_192 = true;
				
			break;
			
		case 5:
			if (Friday == true)
				gi_192 = true;
				
			break;
			
		case 6:
			if (Saturday == true)
				gi_192 = true;
		}
	}
	
	else
		gi_192 = true;
		
	for (int l_pos_196 = 0; l_pos_196 < OrdersTotal(); l_pos_196++) {
		OrderSelect(l_pos_196, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			if (OrderType() == OP_BUY && l_ticket_72 < OrderTicket()) {
				l_ord_lots_96 = OrderLots();
				l_ord_open_price_112 = OrderOpenPrice();
				l_ticket_72 = OrderTicket();
				l_ord_takeprofit_136 = OrderTakeProfit();
				
				if (OrderLots() == g_lots_200)
					ld_176 = OrderOpenPrice() - Stop_Los * Point;
			}
			
			if (OrderType() == OP_SELL && l_ticket_76 < OrderTicket()) {
				l_ord_lots_104 = OrderLots();
				l_ord_open_price_120 = OrderOpenPrice();
				l_ticket_76 = OrderTicket();
				l_ord_takeprofit_128 = OrderTakeProfit();
				
				if (OrderLots() == g_lots_200)
					ld_184 = OrderOpenPrice() + Stop_Los * Point;
			}
			
			if (OrderType() == OP_BUYLIMIT) {
				l_ticket_80 = OrderTicket();
				l_ord_lots_168 = OrderLots();
			}
			
			if (OrderType() == OP_SELLLIMIT) {
				l_ticket_84 = OrderTicket();
				l_ord_lots_168 = OrderLots();
			}
			
			if (OrderType() == OP_BUYSTOP) {
				l_ticket_88 = OrderTicket();
				l_ord_lots_152 = OrderLots();
			}
			
			if (OrderType() == OP_SELLSTOP) {
				l_ticket_92 = OrderTicket();
				l_ord_lots_152 = OrderLots();
			}
		}
	}
	
	int li_200 = sum_buy() / (AccountBalance() / 100.0);
	
	int li_204 = sum_sell() / (AccountBalance() / 100.0);
	int li_unused_208 = (sum_sell() + sum_buy()) / (AccountBalance() / 100.0);
	
	if (li_200 < (-1.0 * Closeprocent) && Closeorders == true) {
		for (int l_pos_212 = OrdersTotal() - 1; l_pos_212 >= 0; l_pos_212--) {
			if (!(OrderSelect(l_pos_212, SELECT_BY_POS, MODE_TRADES)))
				break;
				
			if (OrderType() == OP_BUY && OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
				OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), 2);
				
			if (OrderType() == OP_BUYSTOP && OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
				OrderDelete(OrderTicket());
				
			if (OrderType() == OP_BUYLIMIT && OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
				OrderDelete(OrderTicket());
		}
	}
	
	if (li_204 < (-1.0 * Closeprocent) && Closeorders == true) {
		for (int l_pos_216 = OrdersTotal() - 1; l_pos_216 >= 0; l_pos_216--) {
			if (!(OrderSelect(l_pos_216, SELECT_BY_POS, MODE_TRADES)))
				break;
				
			if (OrderType() == OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
				OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 2);
				
			if (OrderType() == OP_SELLSTOP && OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
				OrderDelete(OrderTicket());
				
			if (OrderType() == OP_SELLLIMIT && OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
				OrderDelete(OrderTicket());
		}
	}
	
	int li_unused_220 = MarketInfo(Symbol(), MODE_SPREAD);
	
	int li_unused_224 = MarketInfo(Symbol(), MODE_POINT);
	int li_unused_228 = MarketInfo(Symbol(), MODE_DIGITS);
	int li_unused_232 = MarketInfo(Symbol(), MODE_TICKVALUE);
	
	if (l_ticket_72 == 0 && gi_192) {
		if (l_ticket_80 != 0)
			OrderDelete(l_ticket_80);
			
		if (l_ticket_88 != 0)
			OrderDelete(l_ticket_88);
			
		l_price_16 = Ask - Stop_Los * Point;
		
		if (Stop_Los <= 0.0)
			l_price_16 = 0;
			
		l_price_32 = Ask + Profit * Point;
		
		if ((TimeHour(TimeCurrent()) >= timestart1 && TimeHour(TimeCurrent()) < timestop1) || (TimeHour(TimeCurrent()) >= timestart2 && TimeHour(TimeCurrent()) < timestop2))
			OrderSend(Symbol(), OP_BUY, g_lots_200, Ask, 3, l_price_16, l_price_32, "", magic, 0, Blue);
	}
	
	if (l_ticket_76 == 0 && gi_192) {
		if (l_ticket_84 != 0)
			OrderDelete(l_ticket_84);
			
		if (l_ticket_92 != 0)
			OrderDelete(l_ticket_92);
			
		l_price_8 = Bid + Stop_Los * Point;
		
		if (Stop_Los <= 0.0)
			l_price_8 = 0;
			
		l_price_24 = Bid - Profit * Point;
		
		if ((TimeHour(TimeCurrent()) >= timestart1 && TimeHour(TimeCurrent()) < timestop1) || (TimeHour(TimeCurrent()) >= timestart2 && TimeHour(TimeCurrent()) < timestop2))
			OrderSend(Symbol(), OP_SELL, g_lots_200, Bid, 3, l_price_8, l_price_24, "", magic, 0, Red);
	}
	
	if (l_ticket_72 != 0 && l_ticket_80 == 0) {
		l_pips_56 = LevelMM(l_ord_lots_96);
		l_pips_48 = ProfitMM(l_ord_lots_96);
		l_price_0 = l_ord_open_price_112 - l_pips_56 * Point;
		
		if (gi_188 == false)
			l_price_16 = l_price_0 - Stop_Los * Point;
		else
			l_price_16 = ld_176;
			
		if (Stop_Los <= 0.0)
			l_price_16 = 0;
			
		l_price_32 = l_price_0 + l_pips_48 * Point;
		
		l_lots_40 = NormalizeDouble(2.0 * l_ord_lots_96, l_pips_48);
		
		if (gi_192 && l_lots_40 < MaxLots)
			OrderSend(Symbol(), OP_BUYLIMIT, l_lots_40, l_price_0, 3, l_price_16, l_price_32, "", magic, 0, Blue);
	}
	
	if (l_ticket_76 != 0 && l_ticket_84 == 0) {
		l_pips_56 = LevelMM(l_ord_lots_104);
		l_pips_48 = ProfitMM(l_ord_lots_104);
		l_price_0 = l_ord_open_price_120 + l_pips_56 * Point;
		
		if (gi_188 == false)
			l_price_8 = l_price_0 + Stop_Los * Point;
		else
			l_price_8 = ld_184;
			
		if (Stop_Los <= 0.0)
			l_price_8 = 0;
			
		l_price_24 = l_price_0 - l_pips_48 * Point;
		
		l_lots_40 = NormalizeDouble(2.0 * l_ord_lots_104, l_pips_48);
		
		if (gi_192 && l_lots_40 < MaxLots)
			OrderSend(Symbol(), OP_SELLLIMIT, l_lots_40, l_price_0, 3, l_price_8, l_price_24, "", magic, 0, Red);
	}
	  
	for (int l_pos_196 = 0; l_pos_196 < OrdersTotal(); l_pos_196++) {
		OrderSelect(l_pos_196, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderType() == OP_BUY && l_ticket_72 != 0 && OrderMagicNumber() == magic) {
			if (gi_188 == false)
				l_price_16 = l_ord_open_price_112 - Stop_Los * Point;
			else
				l_price_16 = ld_176;
				
			if (Stop_Los <= 0.0)
				l_price_16 = 0;
				
			if (l_ticket_72 != OrderTicket())
				l_price_32 = l_ord_takeprofit_136;
			else
				l_price_32 = OrderTakeProfit();
				
			if (l_price_32 != OrderTakeProfit() || l_price_16 != OrderStopLoss())
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_16, l_price_32, 0, Blue);
		}
		
		else if (OrderSymbol() == Symbol() && OrderType() == OP_SELL && l_ticket_76 != 0 && OrderMagicNumber() == magic) {
			if (gi_188 == false)
				l_price_8 = l_ord_open_price_120 + Stop_Los * Point;
			else
				l_price_8 = ld_184;
				
			if (Stop_Los <= 0.0)
				l_price_8 = 0;
				
			if (l_ticket_76 != OrderTicket())
				l_price_24 = l_ord_takeprofit_128;
			else
				l_price_24 = OrderTakeProfit();
				
			if (l_price_24 != OrderTakeProfit() || l_price_8 != OrderStopLoss())
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_8, l_price_24, 0, Red);
		}
	}
	
}

int Sculptor::sum_buy() {
	bool li_ret_0 = false;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES) == false)
			break;
			
		if (OrderType() == OP_BUY && OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
			li_ret_0 = li_ret_0 + OrderProfit() + OrderSwap() + OrderCommission();
	}
	
	return (li_ret_0);
}

int Sculptor::sum_sell() {
	bool li_ret_0 = false;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES) == false)
			break;
			
		if (OrderType() == OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic)
			li_ret_0 = li_ret_0 + OrderProfit() + OrderSwap() + OrderCommission();
	}
	
	return (li_ret_0);
}

double Sculptor::getDayProfit(int ai_0, int ai_4) {
	int l_year_8;
	int l_month_12;
	int l_day_16;
	Time l_datetime_20(1970,1,1);
	int l_hist_total_24 = OrdersHistoryTotal();
	double ld_ret_28 = 0;
	
	for (int l_pos_36 = 0; l_pos_36 < l_hist_total_24; l_pos_36++) {
		OrderSelect(l_pos_36, SELECT_BY_POS, MODE_HISTORY);
		l_datetime_20 = OrderCloseTime();
		l_year_8 = TimeYear(l_datetime_20);
		l_month_12 = TimeMonth(l_datetime_20);
		l_day_16 = TimeDay(l_datetime_20);
		
		if (l_year_8 == Year() && l_month_12 == Month() && l_day_16 == Day() - ai_4 && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			if (ai_0 == 1)
				if (OrderType() == OP_BUY || OrderType() == OP_SELL)
					ld_ret_28 = ld_ret_28 + OrderProfit() + OrderSwap() + OrderCommission();
		}
	}
	
	return (ld_ret_28);
}

double Sculptor::getDayProfitall(int ai_0, int ai_4) {
	int l_year_8;
	int l_month_12;
	int l_day_16;
	Time l_datetime_20(1970,1,1);
	int l_hist_total_24 = OrdersHistoryTotal();
	double ld_ret_28 = 0;
	
	for (int l_pos_36 = 0; l_pos_36 < l_hist_total_24; l_pos_36++) {
		OrderSelect(l_pos_36, SELECT_BY_POS, MODE_HISTORY);
		l_datetime_20 = OrderCloseTime();
		l_year_8 = TimeYear(l_datetime_20);
		l_month_12 = TimeMonth(l_datetime_20);
		l_day_16 = TimeDay(l_datetime_20);
		
		if (l_year_8 == Year() && l_month_12 == Month() && l_day_16 == Day() - ai_4 && OrderMagicNumber() == magic) {
			if (ai_0 == 1)
				if (OrderType() == OP_BUY || OrderType() == OP_SELL)
					ld_ret_28 = ld_ret_28 + OrderProfit() + OrderSwap() + OrderCommission();
		}
	}
	
	return (ld_ret_28);
}

int Sculptor::LevelMM(double ad_0) {
	int ad_02 = NormalizeDouble(ad_0 / g_lots_200, 0);
	
	switch (ad_02) {
	
	case 1:
		return (level3);
		
	case 2:
		return (level3);
		
	case 4:
		return (level4);
		
	case 8:
		return (level5);
		
	case 16:
		return (level6);
		
	case 32:
		return (level7);
		
	case 64:
		return (level8);
		
	case 128:
		return (level9);
		
	case 256:
		return (level10);
		
	case 512:
		return (level11);
	}
	
	return (level11);
}

int Sculptor::ProfitMM(double ad_0) {
	int ad_02 = NormalizeDouble(ad_0 / g_lots_200, 0);
	
	switch (ad_02) {
	
	case 1:
		return (profit2);
		
	case 2:
		return (profit3);
		
	case 4:
		return (profit4);
		
	case 8:
		return (profit5);
		
	case 16:
		return (profit6);
		
	case 32:
		return (profit7);
		
	case 64:
		return (profit8);
		
	case 128:
		return (profit9);
		
	case 256:
		return (profit10);
		
	case 512:
		return (profit11);
	}
	
	return (profit11);
}

}
