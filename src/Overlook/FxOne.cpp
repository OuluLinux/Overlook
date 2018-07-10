#include "Overlook.h"

#if 0

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
	return (0);
}

void FxOne::StartEA(int pos) {
	if (gi_280) {
		if (Period() != gi_288) {
			f0_12("DiagMesg1", 1, 10, 17, gs_316, 10, White);
			return (0);
		}
	}
	
	if (gi_284) {
		if (Symbol() != gs_eurusd_292) {
			f0_12("DiagMesg1", 1, 10, 17, gs_316, 10, White);
			return (0);
		}
	}
	
	if (!gi_340) {
		f0_10();
		gi_340 = true;
	}
	
	else
		if (TimeCurrent() - g_datetime_352 > gi_348)
			f0_10();
			
	if (gi_356 == true)
		f0_12("FxMsg1", 1, 10, 17, gs_300, 10, White);
	else
		f0_12("FxMsg1", 1, 10, 17, gs_308, 10, White);
		
	if (g_time_212 == Time[0])
		return (0);
		
	g_time_212 = Time[0];
	
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
			gi_256 = f0_1(1, gd_220, NormalizeDouble(Bid, Digits), gd_136, NormalizeDouble(Ask, Digits), 0, 0, gs_fxdiler_204 + "-" + gi_216, MagicNumber, 0, HotPink);
			
			if (gi_256 < 0) {
				Print("Error: ", GetLastError());
				return (0);
			}
			
			gd_184 = f0_8();
			
			gi_244 = false;
			gi_260 = true;
		}
		
		else {
			if (gi_248) {
				gi_216 = gi_240;
				gd_220 = NormalizeDouble(gd_192 * MathPow(gd_108, gi_216), gi_116);
				gi_256 = f0_1(0, gd_220, NormalizeDouble(Ask, Digits), gd_136, NormalizeDouble(Bid, Digits), 0, 0, gs_fxdiler_204 + "-" + gi_216, MagicNumber, 0, Lime);
				
				if (gi_256 < 0) {
					Print("Error: ", GetLastError());
					return (0);
				}
				
				gd_176 = f0_7();
				
				gi_244 = false;
				gi_260 = true;
			}
		}
	}
	
	if (gi_244 && gi_240 < 1) {
		g_ihigh_264 = iHigh(Symbol(), 0, 1);
		g_ilow_272 = iLow(Symbol(), 0, 2);
		gd_152 = NormalizeDouble(Bid, Digits);
		gd_160 = NormalizeDouble(Ask, Digits);
		
		if ((!gi_252) && !gi_248) {
			gi_216 = gi_240;
			gd_220 = BaseLot;
			
			if (g_ihigh_264 > g_ilow_272) {
				if (iRSI(NULL, PERIOD_H1, 14, PRICE_CLOSE, 1) > 30.0) {
					gi_256 = f0_1(1, gd_220, gd_152, gd_136, gd_152, 0, 0, gs_fxdiler_204 + "-" + gi_216, MagicNumber, 0, HotPink);
					
					if (gi_256 < 0) {
						Print("Error: ", GetLastError());
						return (0);
					}
					
					gd_176 = f0_7();
					
					gi_260 = true;
				}
			}
			
			else {
				if (iRSI(NULL, PERIOD_H1, 14, PRICE_CLOSE, 1) < 70.0) {
					gi_256 = f0_1(0, gd_220, gd_160, gd_136, gd_160, 0, 0, gs_fxdiler_204 + "-" + gi_216, MagicNumber, 0, Lime);
					
					if (gi_256 < 0) {
						Print("Error: ", GetLastError());
						return (0);
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
	
	return (0);
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

int FxOne::f0_1(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_32, int ai_36, string a_comment_40, int a_magic_48, int a_datetime_52, color a_color_56) {
	int ticket_60 = 0;
	int error_64 = 0;
	int count_68 = 0;
	int li_72 = 100;
	
	if (gi_356 == false)
		return (0);
		
	switch (ai_0) {
	
	case 2:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_BUYLIMIT, a_lots_4, a_price_12, a_slippage_20, f0_2(ad_24, ai_32), f0_4(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52,
						a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(1000);
		}
		
		break;
		
	case 4:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_4, a_price_12, a_slippage_20, f0_2(ad_24, ai_32), f0_4(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52,
						a_color_56);
			error_64 = GetLastError();
			
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
			ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, f0_2(Bid, ai_32), f0_4(Ask, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 3:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_SELLLIMIT, a_lots_4, a_price_12, a_slippage_20, f0_3(ad_24, ai_32), f0_5(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52,
						a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 5:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_4, a_price_12, a_slippage_20, f0_3(ad_24, ai_32), f0_5(a_price_12, ai_36), a_comment_40, a_magic_48, a_datetime_52,
						a_color_56);
			error_64 = GetLastError();
			
			if (error_64 == 0/* NO_ERROR */)
				break;
				
			if (!((error_64 == 4/* SERVER_BUSY */ || error_64 == 137/* BROKER_BUSY */ || error_64 == 146/* TRADE_CONTEXT_BUSY */ || error_64 == 136/* OFF_QUOTES */)))
				break;
				
			Sleep(5000);
		}
		
		break;
		
	case 1:
	
		for (count_68 = 0; count_68 < li_72; count_68++) {
			ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, f0_3(Ask, ai_32), f0_5(Bid, ai_36), a_comment_40, a_magic_48, a_datetime_52, a_color_56);
			error_64 = GetLastError();
			
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

string FxOne::f0_9(string as_0) {
	int lia_8[256];
	int lia_12[1];
	int li_16;
	
	if (!IsDllsAllowed()) {
		Comment("Íåîáõîäèìî â íàñòðîéêàõ ðàçðåøèòü èñïîëüçîâàíèå DLL");
		return ("error");
	}
	
	int li_28 = InternetAttemptConnect(0);
	
	if (li_28 != 0) {
		Print("Îøèáêà ïðè âûçîâå ñîåäèíåíèÿ");
		return ("error");
	}
	
	int li_32 = InternetOpenA("Fxdiler Expert Advisor", 1, 0, 0, 0);
	
	if (li_32 <= 0) {
		Print("Îøèáêà ïðè âûçîâå ñîåäèíåíèÿ");
		return ("error");
	}
	
	int li_36 = InternetOpenUrlA(li_32, as_0, "", 0, 1024, 0);
	
	if (li_36 <= 0) {
		Print("Îøèáêà ïðè âûçîâå ñîåäèíåíèÿ");
		InternetCloseHandle(li_32);
		return ("error");
	}
	
	string ls_ret_20 = "";
	
	while (!IsStopped()) {
		li_16 = InternetReadFile(li_36, lia_8, 1024, lia_12);
		
		if (lia_12[0] == false)
			break;
			
		for (int index_40 = 0; index_40 < 256; index_40++) {
			ls_ret_20 = ls_ret_20 + CharToStr(lia_8[index_40] & 255);
			
			if (StringLen(ls_ret_20) == lia_12[0])
				break;
				
			ls_ret_20 = ls_ret_20 + CharToStr(lia_8[index_40] >> 8 & 255);
			
			if (StringLen(ls_ret_20) == lia_12[0])
				break;
				
			ls_ret_20 = ls_ret_20 + CharToStr(lia_8[index_40] >> 16 & 255);
			
			if (StringLen(ls_ret_20) == lia_12[0])
				break;
				
			ls_ret_20 = ls_ret_20 + CharToStr(lia_8[index_40] >> 24 & 255);
			
			if (StringLen(ls_ret_20) == lia_12[0])
				break;
		}
		
		Sleep(1);
	}
	
	InternetCloseHandle(li_32);
	
	if (ls_ret_20 == "") {
		Print("Îøèáêà ñîåäèíåíèÿ");
		return ("error");
	}
	
	return (ls_ret_20);
}

void FxOne::f0_10() {
	bool li_0 = false;
	int li_unused_4 = 0;
	string ls_8 = "";
	string ls_16 = "";
	g_datetime_352 = TimeCurrent();
	
	if (IsDemo())
		li_0 = false;
	else
		li_0 = true;
		
	gs_324 = f0_9(gs_332 + "?p1=" + AccountNumber() + "&p2=" + Serial + "&p3=" + li_0 + "&p4=" + AccountCompany());
	
	if (gs_324 == "error") {
		if (gi_344) {
			gi_356 = true;
			return;
		}
		
		//gi_356 = false;
		gi_356 = true;
		
		return;
	}
	
	ls_8 = StringSubstr(gs_324, 0, 32);
	
	int li_24 = f0_11(ls_8);
	ls_16 = StringSubstr(gs_324, 32);
	
	if (li_24 == StrToInteger(ls_16)) {
		gi_356 = true;
		return;
	}
	
	//gi_356 = false;
	gi_356 = true;
}

int FxOne::f0_11(string as_0) {
	int li_12;
	int li_16 = 0;
	
	for (int li_8 = 0; li_8 < StringLen(as_0); li_8++) {
		li_12 = StringGetChar(as_0, li_8);
		li_16 += li_12;
	}
	
	return (5 * li_16 + 31791);
}

int FxOne::f0_12(string a_name_0, int a_corner_8, int a_x_12, int a_y_16, string a_text_20, int a_fontsize_28, color a_color_32) {
	if (ObjectFind(a_name_0) == -1)
		ObjectCreate(a_name_0, OBJ_LABEL, 0, 0, 0);
		
	ObjectSet(a_name_0, OBJPROP_CORNER, a_corner_8);
	
	ObjectSet(a_name_0, OBJPROP_XDISTANCE, a_x_12);
	
	ObjectSet(a_name_0, OBJPROP_YDISTANCE, a_y_16);
	
	ObjectSetText(a_name_0, a_text_20, a_fontsize_28, "Tahoma", a_color_32);
	
	return (0);
}

}

#endif
