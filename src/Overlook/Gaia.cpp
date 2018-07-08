#include "Overlook.h"

namespace Overlook {

Gaia::Gaia() {
	
}

void Gaia::InitEA() {
		
	if (Acc5Digits) {
		gi_96 = 10 * gi_96;
		gi_100 = 10 * gi_100;
	}
	
	AddSubCore<MovingAverage>()
		.Set("period", g_period_200);
	
	AddSubCore<MovingAverage>()
		.Set("period", g_period_184);
	
}

void Gaia::StartEA(int pos) {
	int l_cmd_72;
	double l_price_76;
	double ld_84;
	double ld_92;
	
	if (pos < 1)
		return;
	
	double l_icustom_16 = GetInputBuffer(1, 0).Get(pos);
	double l_ima_24 = At(0).GetBuffer(0).Get(pos);
	double l_ima_32 = At(1).GetBuffer(0).Get(pos);
	double l_ima_40 = At(0).GetBuffer(0).Get(pos - 1);
	double l_ima_48 = At(1).GetBuffer(0).Get(pos - 1);
	int l_count_56 = 0;
	int l_count_60 = 0;
	int l_ord_total_64 = OrdersTotal();
	
	for (int l_pos_68 = 0; l_pos_68 < l_ord_total_64; l_pos_68++) {
		if (OrderSelect(l_pos_68, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderMagicNumber() == g_magic_108) {
					l_cmd_72 = OrderType();
					
					if (l_cmd_72 == OP_BUY)
						l_count_56++;
						
					if (l_cmd_72 == OP_SELL)
						l_count_60++;
				}
			}
		}
	}
	
	if (l_ima_48 <= l_ima_40 && l_ima_32 > l_ima_24 && l_icustom_16 < gd_168) {
		if (l_count_56 <= 0) {
			if (DayOfWeek() != 3) {
				if (l_count_60 > 0) {
					l_count_60 = CloseOrders(OP_SELL);
					
					if (l_count_60 > 0)
						return;
				}
				
				l_price_76 = Ask;
				
				ld_84 = 0;
				ld_92 = 0;
				
				if (gi_96 > 0)
					ld_84 = l_price_76 - gi_96 * Point;
					
				if (gi_100 > 0)
					ld_92 = l_price_76 + gi_100 * Point;
					
				Buy(Symbol(), GetLots(), l_price_76, ld_84, ld_92, g_magic_108);
			}
		}
	}
	
	else {
		if (l_ima_48 >= l_ima_40 && l_ima_32 < l_ima_24 && l_icustom_16 < gd_168) {
			if (l_count_60 <= 0) {
				if (DayOfWeek() != 2) {
					if (DayOfWeek() != 3) {
						if (DayOfWeek() != 5) {
							if (l_count_56 > 0) {
								l_count_56 = CloseOrders(OP_BUY);
								
								if (l_count_56 > 0)
									return;
							}
							
							l_price_76 = Bid;
							
							ld_84 = 0;
							ld_92 = 0;
							
							if (gi_96 > 0)
								ld_84 = l_price_76 + gi_96 * Point;
								
							if (gi_100 > 0)
								ld_92 = l_price_76 - gi_100 * Point;
								
							Sell(Symbol(), GetLots(), l_price_76, ld_84, ld_92, g_magic_108);
						}
					}
				}
			}
		}
	}
}

double Gaia::GetLots() {
	if (!EnableMM)
		return (Lots);
		
	double ld_0 = NormalizeDouble(LotBalancePcnt / 100.0 * AccountBalance() / 1000.0, LotPrec);
	
	ld_0 = MathMax(ld_0, MinLot);
	
	ld_0 = MathMin(ld_0, MaxLot);
	
	return (ld_0);
}

int Gaia::CloseOrders(int a_cmd_0) {
	int l_ord_total_4 = OrdersTotal();
	
	for (int l_pos_8 = l_ord_total_4 - 1; l_pos_8 >= 0; l_pos_8--) {
		if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderMagicNumber() == g_magic_108) {
					if (OrderType() == a_cmd_0) {
						if (OrderType() == OP_BUY)
							CloseOrder(OrderTicket(), OrderLots(), Bid);
						else
							if (OrderType() == OP_SELL)
								CloseOrder(OrderTicket(), OrderLots(), Ask);
					}
				}
			}
		}
	}
	
	int l_count_12 = 0;
	
	l_ord_total_4 = OrdersTotal();
	
	for (int l_pos_8 = 0; l_pos_8 < l_ord_total_4; l_pos_8++) {
		if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderMagicNumber() == g_magic_108)
					if (OrderType() == a_cmd_0)
						l_count_12++;
			}
		}
	}
	
	return (l_count_12);
}

int Gaia::OrdersCount0() {
	int l_count_0 = 0;
	int li_4 = OrdersTotal();
	
	for (int l_pos_8 = 0; l_pos_8 < li_4; l_pos_8++) {
		if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderMagicNumber() == g_magic_108)
					if (OrderOpenTime() >= Now)
						l_count_0++;
			}
		}
	}
	
	li_4 = OrdersHistoryTotal();
	
	for (int l_pos_8 = 0; l_pos_8 < li_4; l_pos_8++) {
		if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_HISTORY)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderMagicNumber() == g_magic_108)
					if (OrderOpenTime() >= Now)
						l_count_0++;
			}
		}
	}
	
	return (l_count_0);
}

int Gaia::Buy(String a_symbol_0, double a_lots_8, double a_price_16, double a_price_24, double a_price_32, int a_magic_40, String a_comment_44) {
	RefreshRates();
	int l_digits_52 = MarketInfo(a_symbol_0, MODE_DIGITS);
	a_price_16 = NormalizeDouble(a_price_16, l_digits_52);
	a_price_24 = NormalizeDouble(a_price_24, l_digits_52);
	a_price_32 = NormalizeDouble(a_price_32, l_digits_52);
	String l_dbl2str_56 = DoubleToStr(a_lots_8, 1);
	String l_dbl2str_64 = DoubleToStr(a_price_16, l_digits_52);
	String l_dbl2str_72 = DoubleToStr(a_price_24, l_digits_52);
	String l_dbl2str_80 = DoubleToStr(a_price_32, l_digits_52);
	int l_ticket_88 = OrderSend(a_symbol_0, OP_BUY, a_lots_8, a_price_16, g_slippage_104, a_price_24, a_price_32, a_comment_44, a_magic_40);
	
	if (l_ticket_88 >= 0) {
		return (l_ticket_88);
	}
	
	return (-1);
}

int Gaia::Sell(String a_symbol_0, double a_lots_8, double a_price_16, double a_price_24, double a_price_32, int a_magic_40, String a_comment_44) {
	RefreshRates();
	int l_digits_52 = MarketInfo(a_symbol_0, MODE_DIGITS);
	a_price_16 = NormalizeDouble(a_price_16, l_digits_52);
	a_price_24 = NormalizeDouble(a_price_24, l_digits_52);
	a_price_32 = NormalizeDouble(a_price_32, l_digits_52);
	String l_dbl2str_56 = DoubleToStr(a_lots_8, 1);
	String l_dbl2str_64 = DoubleToStr(a_price_16, l_digits_52);
	String l_dbl2str_72 = DoubleToStr(a_price_24, l_digits_52);
	String l_dbl2str_80 = DoubleToStr(a_price_32, l_digits_52);
	int l_ticket_88 = OrderSend(a_symbol_0, OP_SELL, a_lots_8, a_price_16, g_slippage_104, a_price_24, a_price_32, a_comment_44, a_magic_40);
	
	if (l_ticket_88 >= 0) {
		return (l_ticket_88);
	}
	
	return (-1);
}

int Gaia::CloseOrder(int a_ticket_0, double a_lots_4, double a_price_12) {
	RefreshRates();
	
	if (!OrderSelect(a_ticket_0, SELECT_BY_TICKET, MODE_TRADES))
		return (0);
		
	int l_digits_20 = MarketInfo(OrderSymbol(), MODE_DIGITS);
	
	String l_dbl2str_24 = DoubleToStr(a_lots_4, 1);
	
	String l_dbl2str_32 = DoubleToStr(a_price_12, l_digits_20);
	
	
	bool l_ord_close_40 = OrderClose(a_ticket_0, a_lots_4, a_price_12, g_slippage_104);
	
	if (l_ord_close_40) {
		return (l_ord_close_40);
	}
	
	return (0);
}

}
