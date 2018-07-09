#include "Overlook.h"

namespace Overlook {

Outsider::Outsider() {
	
}

void Outsider::InitEA() {
	g_bars_236 = Bars;
	
	if (Digits < 4)
		gd_244 = 0.01;
	else
		gd_244 = 0.0001;
		
	if (gd_244 > gd_244)
		Slippage = 10 * Slippage;
		
	g_lotstep_252 = MarketInfo(Symbol(), MODE_LOTSTEP);
	
	g_minlot_260 = MarketInfo(Symbol(), MODE_MINLOT);
	
	g_maxlot_268 = MarketInfo(Symbol(), MODE_MAXLOT);
	
	g_lotsize_276 = MarketInfo(Symbol(), MODE_LOTSIZE);
	
	AddSubCore<MovingAverage>()
		.Set("period", ma_period);
		
}

void Outsider::StartEA(int pos) {
	bool l_bool_0;
	bool l_bool_4;
	int li_8;
	double ld_12;
	
	this->pos = pos;
	
	if (pos < gi_200)
		return;
	        
	if (MyOrdersTotal() > 0) {
		for (int l_pos_28 = 0; l_pos_28 < OrdersTotal(); l_pos_28++) {
			OrderSelect(l_pos_28, SELECT_BY_POS);
			
			if (OrderMagicNumber() == TradeMagic1 || OrderMagicNumber() == TradeMagic2 || OrderMagicNumber() == TradeMagic3 && OrderSymbol() == Symbol()) {
				if (OrderType() == OP_BUYLIMIT) {
					l_bool_0 = gi_168 > 0 && Ask > OrderOpenPrice() + gi_168 * gd_244;
					l_bool_4 = gi_168 == 0 && IsFIPDownCrossXCandle(1);
					
					if (l_bool_0 || l_bool_4)
						OrderDelete(OrderTicket());
				}
				
				else {
					if (OrderType() == OP_SELLLIMIT) {
						l_bool_0 = gi_168 > 0 && Bid < OrderOpenPrice() - gi_168 * gd_244;
						l_bool_4 = gi_168 == 0 && IsFIPUpCrossXCandle(1);
						
						if (l_bool_0 || l_bool_4)
							OrderDelete(OrderTicket());
					}
				}
			}
		}
	}
	
	if (MyOrdersTotal() == 1 && MyOrderBuy(TradeMagic3))
		TrailOrder(TradeMagic3, 0);
		
	if (MyOrdersTotal() == 1 && MyOrderSell(TradeMagic3))
		TrailOrder(TradeMagic3, 1);
		
	if (!IsBarEnd())
		return;
		
	bool li_32 = IsFIPUpCrossXCandle(gi_156);
	
	bool li_36 = IsFIPDownCrossXCandle(gi_156);
	
	if (li_32 && MyOrdersTotal() == 0) {
		li_8 = GetSLPips(0);
		ld_12 = LotsOptimized(li_8);
		
		if (gi_160) {
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_BUYLIMIT, ld_12, Ask - gi_164 * gd_244, Slippage, NormalizeSL("OP_BUY", li_8) - gi_164 * gd_244, NormalizeTP("OP_BUY", Limit1) - gi_164 * gd_244, Symbol() +
						 "Buy1", TradeMagic1);
				RefreshRates();
			}
			
			ld_12 = LotsOptimized(li_8);
			
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_BUYLIMIT, ld_12, Ask - gi_164 * gd_244, Slippage, NormalizeSL("OP_BUY", li_8) - gi_164 * gd_244, NormalizeTP("OP_BUY", Limit2) - gi_164 * gd_244, Symbol() +
						 "Buy2", TradeMagic2);
				RefreshRates();
			}
			
			ld_12 = LotsOptimized(li_8);
			
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_BUYLIMIT, ld_12, Ask - gi_164 * gd_244, Slippage, NormalizeSL("OP_BUY", li_8) - gi_164 * gd_244, 0.0, Symbol() + "Buy3", TradeMagic3);
				RefreshRates();
			}
			
		}
		
		else {
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_BUY, ld_12, Ask, Slippage, NormalizeSL("OP_BUY", li_8), NormalizeTP("OP_BUY", Limit1), Symbol() + "Buy1", TradeMagic1);
				RefreshRates();
			}
			
			ld_12 = LotsOptimized(li_8);
			
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_BUY, ld_12, Ask, Slippage, NormalizeSL("OP_BUY", li_8), NormalizeTP("OP_BUY", Limit2), Symbol() + "Buy2", TradeMagic2);
				RefreshRates();
			}
			
			ld_12 = LotsOptimized(li_8);
			
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_BUY, ld_12, Ask, Slippage, NormalizeSL("OP_BUY", li_8), 0.0, Symbol() + "Buy3", TradeMagic3);
				RefreshRates();
			}
			
		}
	}
	
	if (li_36 && MyOrdersTotal() == 0) {
		li_8 = GetSLPips(1);
		ld_12 = LotsOptimized(li_8);
		
		if (gi_160) {
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_SELLLIMIT, ld_12, Bid + gi_164 * gd_244, Slippage, NormalizeSL("OP_SELL", li_8) + gi_164 * gd_244, NormalizeTP("OP_SELL", Limit1) +
						 gi_164 * gd_244, Symbol() + "Sell1", TradeMagic1);
				RefreshRates();
			}
			
			ld_12 = LotsOptimized(li_8);
			
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_SELLLIMIT, ld_12, Bid + gi_164 * gd_244, Slippage, NormalizeSL("OP_SELL", li_8) + gi_164 * gd_244, NormalizeTP("OP_SELL", Limit2) +
						 gi_164 * gd_244, Symbol() + "Sell2", TradeMagic2);
				RefreshRates();
			}
			
			ld_12 = LotsOptimized(li_8);
			
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_SELLLIMIT, ld_12, Bid + gi_164 * gd_244, Slippage, NormalizeSL("OP_SELL", li_8) + gi_164 * gd_244, 0.0, Symbol() + "Sell3", TradeMagic3);
				RefreshRates();
			}
			
		}
		
		else {
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_SELL, ld_12, Bid, Slippage, NormalizeSL("OP_SELL", li_8), NormalizeTP("OP_SELL", Limit1), Symbol() + "Sell1", TradeMagic1);
				RefreshRates();
			}
			
			ld_12 = LotsOptimized(li_8);
			
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_SELL, ld_12, Bid, Slippage, NormalizeSL("OP_SELL", li_8), NormalizeTP("OP_SELL", Limit2), Symbol() + "Sell2", TradeMagic2);
				RefreshRates();
			}
			
			ld_12 = LotsOptimized(li_8);
			
			gi_240 = -1;
			
			for (int l_pos_28 = 0; l_pos_28 < 10 && gi_240 <= 0; l_pos_28++) {
				gi_240 = OrderSendReliable(Symbol(), OP_SELL, ld_12, Bid, Slippage, NormalizeSL("OP_SELL", li_8), 0.0, Symbol() + "Sell3", TradeMagic3);
				RefreshRates();
			}
			
		}
	}
	
	if (gi_172) {
		if (MyOrdersTotal() > 0 && MyOrderBuy(TradeMagic1) || MyOrderBuy(TradeMagic2) || MyOrderBuy(TradeMagic3)) {
			if (li_36) {
				if (MyOrderBuy(TradeMagic1))
					CloseOrder("OP_BUY", TradeMagic1);
					
				if (MyOrderBuy(TradeMagic2))
					CloseOrder("OP_BUY", TradeMagic2);
					
				if (MyOrderBuy(TradeMagic3))
					CloseOrder("OP_BUY", TradeMagic3);
			}
		}
		
		if (MyOrdersTotal() > 0 && MyOrderSell(TradeMagic1) || MyOrderSell(TradeMagic2) || MyOrderSell(TradeMagic3)) {
			if (li_32) {
				if (MyOrderSell(TradeMagic1))
					CloseOrder("OP_SELL", TradeMagic1);
					
				if (MyOrderSell(TradeMagic2))
					CloseOrder("OP_SELL", TradeMagic2);
					
				if (MyOrderSell(TradeMagic3))
					CloseOrder("OP_SELL", TradeMagic3);
			}
		}
	}
	
}

int Outsider::OrderSendReliable(String a_symbol_0, int a_cmd_8, double a_lots_12, double a_price_20, int a_slippage_28, double a_price_32, double a_price_40, String a_comment_48, int a_magic_56) {
	double ld_68;
	gs_104 = "OrderSendReliable";
	
	
	int l_digits_80 = MarketInfo(a_symbol_0, MODE_DIGITS);
	
	if (l_digits_80 > 0) {
		a_price_20 = NormalizeDouble(a_price_20, l_digits_80);
		a_price_32 = NormalizeDouble(a_price_32, l_digits_80);
		a_price_40 = NormalizeDouble(a_price_40, l_digits_80);
	}
	
	if (a_price_32 != 0.0)
		OrderReliable_EnsureValidStop(a_symbol_0, a_price_20, a_price_32);
		
	gi_unused_112 = 0;
	
	bool li_88 = false;
	
	
	int l_ticket_96 = -1;
	
	if (a_cmd_8 == OP_BUYSTOP || a_cmd_8 == OP_SELLSTOP || a_cmd_8 == OP_BUYLIMIT || a_cmd_8 == OP_SELLLIMIT) {
		int l_count_76 = 0;
		
		
		l_ticket_96 = OrderSend(a_symbol_0, a_cmd_8, a_lots_12, a_price_20, a_slippage_28, a_price_32, a_price_40, a_comment_48, a_magic_56);
		if (l_ticket_96 >= 0)
			return l_ticket_96;
		
		OrderReliablePrint("going from limit order to market order because market is too close.");
		
		if (a_cmd_8 == OP_BUYSTOP || a_cmd_8 == OP_BUYLIMIT) {
			a_cmd_8 = 0;
			RefreshRates();
			a_price_20 = Ask;
		}
		
		else {
			if (a_cmd_8 == OP_SELLSTOP || a_cmd_8 == OP_SELLLIMIT) {
				a_cmd_8 = 1;
				RefreshRates();
				a_price_20 = Bid;
			}
		}
	}
	
	
	gi_unused_112 = 0;
	l_ticket_96 = -1;
	
	if (a_cmd_8 == OP_BUY || a_cmd_8 == OP_SELL) {
		l_ticket_96 = OrderSend(a_symbol_0, a_cmd_8, a_lots_12, a_price_20, a_slippage_28, 0, 0, a_comment_48, a_magic_56);
		return (l_ticket_96);
	}
	
	return (0);
}

int Outsider::OrderModifyReliable(int a_ticket_0, double a_price_4, double a_price_12, double a_price_20) {
	String ls_36;
	gs_104 = "OrderModifyReliable";
	
	
	gi_unused_112 = 0;
	bool li_52 = false;
	int l_count_44 = 0;
	bool l_bool_56 = false;
	
	l_bool_56 = OrderModify(a_ticket_0, a_price_4, a_price_12, a_price_20);
	
	if (l_bool_56 == true)
		li_52 = true;
	
	if (l_count_44 > gi_84)
		li_52 = true;
	
	
	if (l_bool_56 == true) {
		OrderReliablePrint("apparently successful modification order, updated trade details follow.");
		return (1);
	}
	
	return (0);
}

void Outsider::OrderModifyReliableSymbol(String a_symbol_0, int ai_8, double ad_12, double ad_20, double ad_28, int ai_36, int ai_40) {
	int l_digits_44 = MarketInfo(a_symbol_0, MODE_DIGITS);
	
	if (l_digits_44 > 0) {
		ad_12 = NormalizeDouble(ad_12, l_digits_44);
		ad_20 = NormalizeDouble(ad_20, l_digits_44);
		ad_28 = NormalizeDouble(ad_28, l_digits_44);
	}
	
	if (ad_20 != 0.0)
		OrderReliable_EnsureValidStop(a_symbol_0, ad_12, ad_20);
		
	OrderModifyReliable(ai_8, ad_12, ad_20, ad_28);
}

int Outsider::OrderCloseReliable(int a_ticket_0, double a_lots_4, double a_price_12, int a_slippage_20) {
	
	gi_unused_112 = 0;
	bool li_36 = false;
	bool l_ord_close_40 = false;
	
	l_ord_close_40 = OrderClose(a_ticket_0, a_lots_4, a_price_12, a_slippage_20);
	
	
	if (l_ord_close_40 == 1) {
		return (1);
	}
	
	return (0);
}

String Outsider::OrderReliableErrTxt(int ai_0) {
	return "";
}

void Outsider::OrderReliablePrint(String as_0) {
	Print(gs_104 + " " + gs_76 + ":" + as_0);
}

String Outsider::OrderReliable_CommandString(int ai_0) {
	if (ai_0 == 0)
		return ("OP_BUY");
		
	if (ai_0 == 1)
		return ("OP_SELL");
		
	if (ai_0 == 4)
		return ("OP_BUYSTOP");
		
	if (ai_0 == 5)
		return ("OP_SELLSTOP");
		
	if (ai_0 == 2)
		return ("OP_BUYLIMIT");
		
	if (ai_0 == 3)
		return ("OP_SELLLIMIT");
		
	return ("(CMD==" + IntStr(ai_0) + ")");
}

void Outsider::OrderReliable_EnsureValidStop(String a_symbol_0, double ad_8, double &ad_16) {
	double ld_24;
	
	if (ad_16 != 0.0) {
		ld_24 = MarketInfo(a_symbol_0, MODE_STOPLEVEL) * gd_244;
		
		if (MathAbs(ad_8 - ad_16) <= ld_24) {
			if (ad_8 > ad_16)
				ad_16 = ad_8 - ld_24;
			else {
				if (ad_8 < ad_16)
					ad_16 = ad_8 + ld_24;
				else
					OrderReliablePrint("EnsureValidStop: error, passed in price == sl, cannot adjust");
			}
			
			ad_16 = NormalizeDouble(ad_16, MarketInfo(a_symbol_0, MODE_DIGITS));
		}
	}
}

void Outsider::OrderReliable_SleepRandomTime(double ad_0, double ad_8) {
	double ld_16;
	int li_24;
	double ld_28;
	
	if (IsTesting() == false) {
		ld_16 = MathCeil(ad_0 / 0.1);
		
		if (ld_16 > 0.0) {
			li_24 = MathRound(ad_8 / 0.1);
			ld_28 = 1.0 - 1.0 / ld_16;
			Sleep(1000);
			
			for (int l_count_36 = 0; l_count_36 < li_24; l_count_36++) {
				if (MathRand() > 32768.0 * ld_28)
					break;
					
				Sleep(1000);
			}
		}
	}
}

double Outsider::LotsOptimized(int ai_0) {
	double ld_ret_4 = Lots;
	
	if (Lots == 0.0)
		ld_ret_4 = Percent_Risk * AccountFreeMargin() / 100.0 / (MarketInfo(Symbol(), MODE_LOTSIZE) * gd_244 * ai_0);
		
	ld_ret_4 = MathFloor(ld_ret_4 / g_lotstep_252) * g_lotstep_252;
	
	if (ld_ret_4 < g_minlot_260)
		ld_ret_4 = g_minlot_260;
		
	if (ld_ret_4 > g_maxlot_268)
		ld_ret_4 = g_maxlot_268;
		
	return (ld_ret_4);
}

bool Outsider::IsBarEnd() {
	bool li_ret_0 = false;
	
	if (g_bars_236 != Bars) {
		li_ret_0 = true;
		g_bars_236 = Bars;
	}
	
	return (li_ret_0);
}

int Outsider::IsFIPUpCrossXCandle(int ai_0) {
	/*int li_4 = 1;
	bool li_8 = false;
	double l_icustom_12 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 0, 0);
	double l_icustom_20 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 1, 0);
	double l_icustom_28 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 0, 1);
	double l_icustom_36 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 1, 1);
	double l_icustom_44 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 0, 2);
	double l_icustom_52 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 1, 2);
	
	if (l_icustom_20 > 1000.0) {
		while (!li_8 && li_4 <= ai_0) {
			l_icustom_28 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 0, li_4);
			l_icustom_36 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 1, li_4);
			
			if (l_icustom_28 > 1000.0 || l_icustom_28 == l_icustom_36) {
				li_8 = true;
				return (1);
			}
			
			li_4++;
		}
	}
	
	else {
		if (l_icustom_28 == l_icustom_36 && l_icustom_28 > 1000.0 && l_icustom_20 > 1000.0)
			return (1);
			
		return (0);
	}
	
	return (0);
	*/
	
	double m0 = At(0).GetBuffer(0).Get(pos - 0);
	double m1 = At(0).GetBuffer(0).Get(pos - 1);
	double m2 = At(0).GetBuffer(0).Get(pos - 2);
	
	return m0 >= m1 && m1 < m2;
}

int Outsider::IsFIPDownCrossXCandle(int ai_0) {
	/*int li_4 = 1;
	bool li_8 = false;
	double l_icustom_12 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 0, 0);
	double l_icustom_20 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 1, 0);
	double l_icustom_28 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 0, 1);
	double l_icustom_36 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 1, 1);
	double l_icustom_44 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 0, 2);
	double l_icustom_52 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 1, 2);
	
	if (l_icustom_12 > 1000.0) {
		while (!li_8 && li_4 <= ai_0) {
			l_icustom_28 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 0, li_4);
			l_icustom_36 = iCustom(Symbol(), 0, "FIP_Line", gi_124, gi_128, 1, li_4);
			
			if (l_icustom_28 < 1000.0 || l_icustom_28 == l_icustom_36) {
				li_8 = true;
				return (1);
			}
			
			li_4++;
		}
	}
	
	else {
		if (l_icustom_28 == l_icustom_36 && l_icustom_36 > 1000.0 && l_icustom_12 > 1000.0)
			return (1);
			
		return (0);
	}
	return (0);
	*/
	
	double m0 = At(0).GetBuffer(0).Get(pos - 0);
	double m1 = At(0).GetBuffer(0).Get(pos - 1);
	double m2 = At(0).GetBuffer(0).Get(pos - 2);
	
	return m0 <= m1 && m1 > m2;
}

int Outsider::MyOrdersTotal() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES))
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == TradeMagic1 || OrderMagicNumber() == TradeMagic2 || OrderMagicNumber() == TradeMagic3)
				l_count_0++;
	}
	
	return (l_count_0);
}

int Outsider::MyOrdersTotalMagic(int a_magic_0) {
	int l_count_4 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES))
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0)
				l_count_4++;
	}
	
	return (l_count_4);
}

bool Outsider::MyOrderBuy(int a_magic_0) {
	int l_pos_4 = 0;
	bool li_ret_8 = false;
	
	while (l_pos_4 < OrdersTotal() && !li_ret_8) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES))
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_BUY)
				li_ret_8 = true;
				
		l_pos_4++;
	}
	
	return (li_ret_8);
}

bool Outsider::MyOrderSell(int a_magic_0) {
	int l_pos_4 = 0;
	bool li_ret_8 = false;
	
	while (l_pos_4 < OrdersTotal() && !li_ret_8) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES))
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_SELL)
				li_ret_8 = true;
				
		l_pos_4++;
	}
	
	return (li_ret_8);
}

double Outsider::NormalizeTP(String as_0, int ai_8) {
	if (ai_8 < 0)
		return (-1);
		
	if (as_0 == "OP_SELL") {
		if (ai_8 > 0.0)
			return (Bid - ai_8 * gd_244);
			
		return (0.0);
	}
	
	if (as_0 == "OP_BUY") {
		if (ai_8 > 0.0)
			return (Ask + ai_8 * gd_244);
			
		return (0.0);
	}
	
	return (0.0);
}

int Outsider::GetSLPips(int ai_0) {
	if (gi_196)
		return (gi_204);
		
	bool li_ret_4 = false;
	
	if (ai_0 == 1)
		li_ret_4 = (HighestCandles(gi_200) - Bid) / gd_244;
	else
		if (ai_0 == 0)
			li_ret_4 = (Ask - LowestCandles(gi_200)) / gd_244;
			
	if (li_ret_4 < MarketInfo(Symbol(), MODE_STOPLEVEL))
		li_ret_4 = MarketInfo(Symbol(), MODE_STOPLEVEL);
		
	return (li_ret_4);
}

double Outsider::NormalizeSL(String as_0, int ai_8) {
	if (ai_8 < 0)
		return (-1);
		
	if (as_0 == "OP_SELL") {
		if (ai_8 > 0.0)
			return (Bid + ai_8 * gd_244);
			
		return (0.0);
	}
	
	if (as_0 == "OP_BUY") {
		if (ai_8 > 0.0)
			return (Ask - ai_8 * gd_244);
			
		return (0.0);
	}
	
	return (0.0);
}

void Outsider::TrailOrder(int a_magic_0, int ai_unused_4) {
	if (Trailing_Stop > 0) {
		for (int l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
			if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES) && OrderMagicNumber() == a_magic_0 && OrderSymbol() == Symbol()) {
				if (OrderType() == OP_BUY) {
					if (Bid - gd_244 * Trailing_Stop > OrderOpenPrice() && Bid - gd_244 * Trailing_Stop > OrderStopLoss())
						OrderModifyReliableSymbol(Symbol(), OrderTicket(), OrderOpenPrice(), Bid - gd_244 * Trailing_Stop, OrderTakeProfit(), 0);
				}
				
				else {
					if (OrderType() == OP_SELL)
						if (Ask + gd_244 * Trailing_Stop < OrderOpenPrice() && Ask + gd_244 * Trailing_Stop < OrderStopLoss())
							OrderModifyReliableSymbol(Symbol(), OrderTicket(), OrderOpenPrice(), Ask + gd_244 * Trailing_Stop, OrderTakeProfit(), 0);
				}
			}
		}
	}
}

int Outsider::CloseOrder(String as_0, int a_magic_8) {
	double ld_12;
	int l_cmd_20;
	int li_unused_24 = 0;
	bool li_28 = true;
	
	if (as_0 == "OP_BUY")
		l_cmd_20 = 0;
	else {
		if (as_0 == "OP_SELL")
			l_cmd_20 = 1;
		else
			l_cmd_20 = 9;
	}
	
	while (MyOrdersTotalMagic(a_magic_8) > 0 && li_28) {
		for (int l_pos_32 = 0; l_pos_32 < OrdersTotal(); l_pos_32++) {
			OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_8) {
				if (OrderType() == l_cmd_20 && l_cmd_20 == OP_BUY)
					ld_12 = NormalizeDouble(MarketInfo(Symbol(), MODE_BID), Digits);
					
				if (OrderType() == l_cmd_20 && l_cmd_20 == OP_SELL)
					ld_12 = NormalizeDouble(MarketInfo(Symbol(), MODE_ASK), Digits);
					
				OrderCloseReliable(OrderTicket(), OrderLots(), ld_12, Slippage);
			}
		}
		
		if (MyOrdersTotalMagic(a_magic_8) > 0) {
			int l_pos_32 = 0;
			li_28 = false;
			
			while (l_pos_32 < OrdersTotal() && !li_28) {
				OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES);
				
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_8 && OrderType() == l_cmd_20)
					li_28 = true;
					
				l_pos_32++;
			}
		}
	}
	
	return (0);
}

double Outsider::LowestCandles(int ai_0) {
	ConstBuffer& low = GetInputBuffer(0, 1);
	double l_low_4 = low.Get(pos - 0);
	
	for (int li_12 = 1; li_12 <= ai_0; li_12++) {
		double d = low.Get(pos - li_12);
		if (d < l_low_4)
			l_low_4 = d;
	}
			
	return (l_low_4);
}

double Outsider::HighestCandles(int ai_0) {
	ConstBuffer& high = GetInputBuffer(0, 2);
	double l_high_4 = high.Get(pos - 0);
	
	for (int li_12 = 1; li_12 <= ai_0; li_12++) {
		double d = high.Get(pos - li_12);
		if (d < l_high_4)
			l_high_4 = d;
	}
			
	return (l_high_4);
}


}
