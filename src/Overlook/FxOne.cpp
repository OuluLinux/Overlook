#include "Overlook.h"

// fdallpair

namespace Overlook {

FxOne::FxOne() {
	
}

void FxOne::InitEA() {
	g_lotstep_228 = MarketInfo(Symbol(), MODE_LOTSTEP);
	
	if (g_lotstep_228 >= 0.01 && g_lotstep_228 < 0.1)
		gi_116 = 2;
		
	if (g_lotstep_228 >= 0.1 && g_lotstep_228 < 1.0)
		gi_116 = 1;
		
	if (Digits == 4) {
		g_pips_120 = 10.0;
		g_pips_128 = 30.0;
		gd_136 = 3.0;
	}
	
	if (Digits == 5) {
		g_pips_120 = 100.0;
		g_pips_128 = 300.0;
		gd_136 = 30.0;
	}
	
	gd_192 = BaseLot;
	
	gd_108 = Multiplier;
	
	AddSubCore<RelativeStrengthIndex>()
		.Set("period", rsi_period);
	
}

void FxOne::StartEA(int pos) {
	
	if (pos < 2)
		return;
	
	if (!gi_340) {
		f0_10();
		gi_340 = true;
	}
	
	else
		if (TimeCurrent().Get() - g_datetime_352.Get() > gi_348)
			f0_10();
		
	
	double ld_0 = f0_6();
	
	gi_240 = f0_0();
	
	if (gi_240 == 0)
		gi_200 = false;
		
	for (g_pos_236 = OrdersTotal() - 1; g_pos_236 >= 0; g_pos_236--) {
		OrderSelect(g_pos_236, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			if (OrderType() == OP_BUY) {
				gi_248 = true;
				gi_252 = false;
				break;
			}
		}
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			if (OrderType() == OP_SELL) {
				gi_248 = false;
				gi_252 = true;
				break;
			}
		}
	}
	
	if (gi_240 > 0 && gi_240 < MaxTrades) {
		RefreshRates();
		gd_176 = f0_7();
		gd_184 = f0_8();
		
		if (gi_248 && gd_176 - Ask >= g_pips_128 * Point)
			gi_244 = true;
			
		if (gi_252 && Bid - gd_184 >= g_pips_128 * Point)
			gi_244 = true;
	}
	
	if (gi_240 < 1) {
		gi_252 = false;
		gi_248 = false;
		gi_244 = true;
	}
	
	if (gi_244) {
		RefreshRates();
		gd_176 = f0_7();
		gd_184 = f0_8();
		
		if (gi_252) {
			gi_216 = gi_240;
			gd_220 = NormalizeDouble(gd_192 * MathPow(gd_108, gi_216), gi_116);
			gi_256 = f0_1(1, gd_220, NormalizeDouble(Bid, Digits), gd_136, NormalizeDouble(Ask, Digits), 0, 0, gs_fxdiler_204 + "-" + gi_216, MagicNumber, 0);
			
			if (gi_256 < 0) {
				return;
			}
			
			gd_184 = f0_8();
			
			gi_244 = false;
			gi_260 = true;
		}
		
		else {
			if (gi_248) {
				gi_216 = gi_240;
				gd_220 = NormalizeDouble(gd_192 * MathPow(gd_108, gi_216), gi_116);
				gi_256 = f0_1(0, gd_220, NormalizeDouble(Ask, Digits), gd_136, NormalizeDouble(Bid, Digits), 0, 0, gs_fxdiler_204 + "-" + gi_216, MagicNumber, 0);
				
				if (gi_256 < 0) {
					return;
				}
				
				gd_176 = f0_7();
				
				gi_244 = false;
				gi_260 = true;
			}
		}
	}
	
	if (gi_244 && gi_240 < 1) {
		CompatBuffer Low(GetInputBuffer(0, 1), pos, 0);
		CompatBuffer High(GetInputBuffer(0, 2), pos, 0);
		g_ihigh_264 = High[1];
		g_ilow_272 = Low[1];
		gd_152 = NormalizeDouble(Bid, Digits);
		gd_160 = NormalizeDouble(Ask, Digits);
		
		if ((!gi_252) && !gi_248) {
			gi_216 = gi_240;
			gd_220 = BaseLot;
			
			double rsi = At(0).GetBuffer(0).Get(pos);
			
			if (g_ihigh_264 > g_ilow_272) {
				if (rsi > 30.0) {
					gi_256 = f0_1(1, gd_220, gd_152, gd_136, gd_152, 0, 0, gs_fxdiler_204 + "-" + gi_216, MagicNumber, 0);
					
					if (gi_256 < 0) {
						return;
					}
					
					gd_176 = f0_7();
					
					gi_260 = true;
				}
			}
			
			else {
				if (rsi < 70.0) {
					gi_256 = f0_1(0, gd_220, gd_160, gd_136, gd_160, 0, 0, gs_fxdiler_204 + "-" + gi_216, MagicNumber, 0);
					
					if (gi_256 < 0) {
						return;
					}
					
					gd_184 = f0_8();
					
					gi_260 = true;
				}
			}
			
			gi_244 = false;
		}
	}
	
	gi_240 = f0_0();
	
	g_price_144 = 0;
	double ld_8 = 0;
	
	for (g_pos_236 = OrdersTotal() - 1; g_pos_236 >= 0; g_pos_236--) {
		OrderSelect(g_pos_236, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
				g_price_144 += OrderOpenPrice() * OrderLots();
				ld_8 += OrderLots();
			}
		}
	}
	
	if (gi_240 > 0)
		g_price_144 = NormalizeDouble(g_price_144 / ld_8, Digits);
		
	if (gi_260) {
		for (g_pos_236 = OrdersTotal() - 1; g_pos_236 >= 0; g_pos_236--) {
			OrderSelect(g_pos_236, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY) {
					g_price_168 = g_price_144 + g_pips_120 * Point;
					gi_200 = true;
				}
				
				if (OrderType() == OP_SELL) {
					g_price_168 = g_price_144 - g_pips_120 * Point;
					gi_200 = true;
				}
			}
		}
	}
	
	g_price_168 = NormalizeDouble(g_price_168, Digits);
	
	if (gi_260) {
		if (gi_200 == true) {
			for (g_pos_236 = OrdersTotal() - 1; g_pos_236 >= 0; g_pos_236--) {
				OrderSelect(g_pos_236, SELECT_BY_POS, MODE_TRADES);
				
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
					continue;
					
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
					OrderModify(OrderTicket(), g_price_144, OrderStopLoss(), g_price_168, 0, Yellow);
					
				gi_260 = false;
			}
		}
	}
}

int FxOne::f0_0() {
	int count_0 = 0;
	
	for (int pos_4 = OrdersTotal() - 1; pos_4 >= 0; pos_4--) {
		OrderSelect(pos_4, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
			if (OrderType() == OP_SELL || OrderType() == OP_BUY)
				count_0++;
	}
	
	return (count_0);
}

int FxOne::f0_1(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_32, int ai_36, String a_comment_40, int a_magic_48, int a_datetime_52) {
	int ticket_60 = 0;
	int error_64 = 0;
	int count_68 = 0;
	int li_72 = 100;
	
	if (gi_356 == false)
		return (0);
		
	switch (ai_0) {
	
	case 2:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_BUYLIMIT, a_lots_4, a_price_12, a_slippage_20, f0_2(ad_24, ai_32), f0_4(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(1000);
		}
		
		break;
		
	case 4:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_4, a_price_12, a_slippage_20, f0_2(ad_24, ai_32), f0_4(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 0:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			RefreshRates();
			ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, f0_2(Bid, ai_32), f0_4(Ask, ai_36), a_comment_40, a_magic_48);
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 3:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_SELLLIMIT, a_lots_4, a_price_12, a_slippage_20, f0_3(ad_24, ai_32), f0_5(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 5:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_4, a_price_12, a_slippage_20, f0_3(ad_24, ai_32), f0_5(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52);
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 1:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, f0_3(Ask, ai_32), f0_5(Bid, ai_36), a_comment_40, a_magic_48);
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
	}
	
	return (ticket_60);
}

double FxOne::f0_2(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 - ai_8 * Point);
}

double FxOne::f0_3(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 + ai_8 * Point);
}

double FxOne::f0_4(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 + ai_8 * Point);
}

double FxOne::f0_5(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 - ai_8 * Point);
}

double FxOne::f0_6() {
	double ld_ret_0 = 0;
	
	for (g_pos_236 = OrdersTotal() - 1; g_pos_236 >= 0; g_pos_236--) {
		OrderSelect(g_pos_236, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
			if (OrderType() == OP_BUY || OrderType() == OP_SELL)
				ld_ret_0 += OrderProfit();
	}
	
	return (ld_ret_0);
}

double FxOne::f0_7() {
	double order_open_price_0;
	int ticket_8;
	double ld_unused_12 = 0;
	int ticket_20 = 0;
	
	for (int pos_24 = OrdersTotal() - 1; pos_24 >= 0; pos_24--) {
		OrderSelect(pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber && OrderType() == OP_BUY) {
			ticket_8 = OrderTicket();
			
			if (ticket_8 > ticket_20) {
				order_open_price_0 = OrderOpenPrice();
				ld_unused_12 = order_open_price_0;
				ticket_20 = ticket_8;
			}
		}
	}
	
	return (order_open_price_0);
}

double FxOne::f0_8() {
	double order_open_price_0;
	int ticket_8;
	double ld_unused_12 = 0;
	int ticket_20 = 0;
	
	for (int pos_24 = OrdersTotal() - 1; pos_24 >= 0; pos_24--) {
		OrderSelect(pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber && OrderType() == OP_SELL) {
			ticket_8 = OrderTicket();
			
			if (ticket_8 > ticket_20) {
				order_open_price_0 = OrderOpenPrice();
				ld_unused_12 = order_open_price_0;
				ticket_20 = ticket_8;
			}
		}
	}
	
	return (order_open_price_0);
}

String FxOne::f0_9(String as_0) {
	int lia_8[256];
	int lia_12[1];
	int li_16;
	

	
	return "";
}

void FxOne::f0_10() {
	bool li_0 = false;
	int li_unused_4 = 0;
	String ls_8 = "";
	String ls_16 = "";
	g_datetime_352 = TimeCurrent();
	
	if (IsDemo())
		li_0 = false;
	else
		li_0 = true;
	
	gi_356 = true;
}

int FxOne::f0_11(String as_0) {
	
	return 0;
}

int FxOne::f0_12(String a_name_0, int a_corner_8, int a_x_12, int a_y_16, String a_text_20, int a_fontsize_28) {
	
	return (0);
}

}
