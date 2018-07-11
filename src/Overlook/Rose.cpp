#include "Overlook.h"

// lerosa

namespace Overlook {

Rose::Rose() {
	
}

void Rose::InitEA() {
	if (CONSERVATIVE > 0) {
		gd_224 = 0.025;
		g_close_596 = GetInputBuffer(0,0).Get(0);
	}
	
	if (MODERATE > 0)
		gd_224 = 0.045;
		
	if (AGGRESSIVE > 0)
		gd_224 = 0.065;
		
	if (gi_112 > 0)
		gd_224 = 0.9;
		
	if (gi_116 > 0)
		gd_224 = 1.5;
	
}

void Rose::StartEA(int pos) {
	int l_digits_0;
	int l_ticket_4;
	double l_close_16;
	double l_close_24;
	double ld_32;
	double l_price_40;
	double l_price_48;
	int li_64;
	int li_68;
	double l_price_72;
	double l_price_80;
	
	if (pos < 1)
		return;
	
	CompatBuffer Close(GetInputBuffer(0, 0), pos, 1);
	CompatBuffer Low(GetInputBuffer(0, 1), pos, 0);
	CompatBuffer High(GetInputBuffer(0, 2), pos, 0);
	
	
	while (gi_624 == 10) {
		RefreshRates();
		Sleep(10);
		l_digits_0 = MarketInfo("EURUSD", MODE_DIGITS);
		
		if (l_digits_0 == 5)
			gi_620 = 10;
		else
			gi_620 = 1;
			
		if (l_digits_0 != 0 && gi_620 != 0)
			gi_624 = 13;
	}
	
	if (gi_444 == false && gi_212 == false) {
		gd_424 = gd_288;
		gi_444 = true;
	}
	
	if (gi_444 == false && gi_212 == true && gi_356 == true) {
		if (MarketInfo(Symbol(), MODE_MINLOT) == 0.1)
			gi_644 = 1;
		else
			if (MarketInfo(Symbol(), MODE_MINLOT) == 0.01)
				gi_644 = 2;
				
		gd_628 = NormalizeDouble(MarketInfo(Symbol(), MODE_MINLOT), gi_644);
		
		gd_636 = NormalizeDouble(MarketInfo(Symbol(), MODE_MAXLOT), gi_644);
		
		gi_644 = 2;
		
		gd_636 = 100;
		
		gd_628 = 0.2;
		
		if (MarketInfo(Symbol(), MODE_LOTSTEP) == 0.1)
			gi_644 = 1;
		else
			if (MarketInfo(Symbol(), MODE_LOTSTEP) == 0.01)
				gi_644 = 2;
				
		gd_628 = NormalizeDouble(MarketInfo(Symbol(), MODE_MINLOT), gi_644);
		
		gd_636 = NormalizeDouble(MarketInfo(Symbol(), MODE_MAXLOT), gi_644);
		
		gd_636 = 100;
		
		if (microlot)
			gd_628 = 0.02;
		else
			gd_628 = 0.2;
			
		gd_424 = NormalizeDouble(AccountEquity() * gd_224 / 1000.0, gi_644);
		
		if (gd_424 > gd_636)
			gd_424 = gd_636;
			
		if (gd_424 < gd_628)
			gd_424 = gd_628;
			
		gi_444 = true;
	}
	
	if (Bars < 100) {
		Print("bars less than 100");
		return;
	}
	
	int li_8 = 1;
	
	g_close_596 = gd_604;
	gd_512 = gd_520;
	
	if (g_time_676 != Now) {
		gd_668 = 0;
		
		for (int li_12 = 1; li_12 <= 50; li_12++) {
			l_close_16 = Close[li_12 + 1];
			gd_660 = MathMax(High[li_12], l_close_16) - MathMin(Low[li_12], l_close_16);
			gd_668 += gd_660;
		}
		
		gd_588 = NormalizeDouble(gd_668 / 50.0 * gd_364, 4);
		
		g_time_676 = Now;
	}
	
	if (Close[li_8] == g_close_596)
		gd_612 = g_close_596;
	else {
		if (Close[li_8 + 1] <= g_close_596 && Close[li_8] < g_close_596)
			gd_612 = MathMin(g_close_596, Close[li_8] + gd_588);
		else {
			if (Close[li_8 + 1] >= g_close_596 && Close[li_8] > g_close_596)
				gd_612 = MathMax(g_close_596, Close[li_8] - gd_588);
			else {
				if (Close[li_8] > g_close_596)
					gd_612 = Close[li_8] - gd_588;
				else
					if (Close[li_8] < g_close_596)
						gd_612 = Close[li_8] + gd_588;
			}
		}
	}
	
	gd_604 = gd_612;
	
	if (g_time_680 != Now) {
		gd_668 = 0;
		
		for (int li_12 = 1; li_12 <= gd_688; li_12++) {
			l_close_24 = Close[li_12 + 1];
			gd_660 = MathMax(High[li_12], l_close_24) - MathMin(Low[li_12], l_close_24);
			gd_668 += gd_660;
		}
		
		gd_504 = NormalizeDouble(gd_668 / gd_688 * gd_696, 4);
		
		g_time_680 = Now;
	}
	
	if (Close[li_8] == gd_512)
		gd_528 = gd_512;
	else {
		if (Close[li_8 + 1] <= gd_512 && Close[li_8] < gd_512)
			gd_528 = MathMin(gd_512, Close[li_8] + gd_504);
		else {
			if (Close[li_8 + 1] >= gd_512 && Close[li_8] > gd_512)
				gd_528 = MathMax(gd_512, Close[li_8] - gd_504);
			else {
				if (Close[li_8] > gd_512)
					gd_528 = Close[li_8] - gd_504;
				else
					if (Close[li_8] < gd_512)
						gd_528 = Close[li_8] + gd_504;
			}
		}
	}
	
	gd_520 = gd_528;
	
	gi_496 = gi_172 + 1 + gi_176 + gi_180 + gi_184 + gi_188;
	gi_500 = gi_192 + 1 + gi_196 + gi_200 + gi_204 + gi_208;
	
	if (gi_356 == true && gi_212 == true && g_ticket_432 != g_ticket_396 && CountOpenPositions() < 1) {
		if (Minus() < 13 || Loss() > gi_232) {
			gd_424 = NormalizeDouble(AccountEquity() * gd_224 / 1000.0, gi_644);
			
			if (gd_424 > gd_636)
				gd_424 = gd_636;
				
			if (gd_424 < gd_628)
				gd_424 = gd_628;
		}
		
		if (Minus() == 13 && Loss() <= gi_232)
			gd_424 = NormalizeDouble(gd_424 * gd_216, gi_644);
			
		if (gd_424 >= gd_636) {
			gd_424 = NormalizeDouble(AccountEquity() * gd_224 / 1000.0, gi_644);
			
			if (gd_424 > gd_636)
				gd_424 = gd_636;
				
			if (gd_424 < gd_628)
				gd_424 = gd_628;
		}
		
		g_ticket_432 = g_ticket_396;
	}
	
	if (gi_356 == true && gi_212 == false && g_ticket_432 != g_ticket_396 && CountOpenPositions() < 1) {
		g_ticket_432 = g_ticket_396;
		
		if (Minus() == 13)
			gd_424 = NormalizeDouble(gd_424 * gd_216, 2);
			
		if (Minus() < 13)
			gd_424 = gd_288;
			
		if (gd_424 > gd_408)
			gd_424 = gd_408;
			
		if (gd_424 < 0.01)
			gd_424 = 0.01;
	}
	
	if (gi_212 == true && gi_356 == false) {
		gd_424 = NormalizeDouble(AccountEquity() * gd_224 / 1000.0, 2);
		
		if (gd_424 > gd_408)
			gd_424 = gd_408;
			
		if (gd_424 < 0.01)
			gd_424 = 0.01;
	}
	
	if (Close[1] >= gd_612)
		gi_unused_772 = true;
	else
		gi_unused_772 = false;
		
	if (Close[1] <= gd_612)
		gi_unused_776 = true;
	else
		gi_unused_776 = false;
		
	if (Close[1] > gd_612 && Close[2] <= g_close_596) {
		gd_704 = 13;
		g_bars_736 = Bars;
		g_bars_752 = g_bars_736;
	}
	
	else
		gd_704 = 10;
		
	if (Close[1] > gd_612 && Close[1] > gd_372 && Close[2] <= gd_380)
		gd_720 = 13;
	else
		gd_720 = 10;
		
	if (Close[1] < gd_612 && Close[2] >= g_close_596) {
		gd_712 = 13;
		g_bars_740 = Bars;
		g_bars_756 = g_bars_740;
	}
	
	else
		gd_712 = 10;
		
	if (Close[1] < gd_612 && Close[1] < gd_372 && Close[2] >= gd_380)
		gd_728 = 13;
	else
		gd_728 = 10;
		
	if (CountOpenPositions() < 1) {
		gi_816 = 10;
		gi_820 = 10;
		gi_808 = 10;
		gi_812 = 10;
		
		if ((gd_704 == 13.0 && gi_420 != Now) || (gd_720 == 13.0 && gi_420 != Now && g_bars_764 != g_bars_752)) {
			gd_436 = gd_424;
			g_lots_652 = NormalizeDouble(gd_436 / gi_496, gi_644);
			ld_32 = NormalizeDouble(gd_628 / 2.0, gi_644);
			
			if (g_lots_652 < ld_32)
				g_lots_652 = ld_32;
				
			if (g_lots_652 > 50.0)
				g_lots_652 = 50;
				
			l_price_40 = NormalizeDouble(Ask - gd_296 * gi_620 * Point, Digits);
			
			l_price_48 = NormalizeDouble(Ask + gd_236 * gi_620 * Point, Digits);
			
			if (ECN) {
				l_price_40 = 0;
				l_price_48 = 0;
			}
			
			l_ticket_4 = OrderSend(Symbol(), OP_BUY, RightLots(g_lots_652), ND(Ask), g_slippage_344 * gi_620, l_price_40, l_price_48, "lur", g_magic_148, 0, Green);
			
			if (l_ticket_4 > 0) {
				if (gd_720 == 13.0 && gd_704 != 13.0)
					g_bars_764 = g_bars_752;
					
				gi_unused_580 = 10;
				
				g_ticket_396 = l_ticket_4;
				
				gi_420 = Now;
				
				gi_unused_388 = 10;
				
				gi_404 = 10;
				
				gi_448 = 13;
				
				gi_808 = 13;
				
				gi_unused_792 = l_ticket_4;
				
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES)) {
					Print("BUY order opened");
					g_ord_open_price_536 = OrderOpenPrice();
				}
			}
			
			else
				Print("Error opening BUY order : " + GetLastError());
				
			return;
		}
		
		if ((gd_712 == 13.0 && gi_420 != Now) || (gd_728 == 13.0 && gi_420 != Now && g_bars_768 != g_bars_756)) {
			gd_436 = gd_424;
			g_lots_652 = NormalizeDouble(gd_436 / gi_496, gi_644);
			ld_32 = NormalizeDouble(gd_628 / 2.0, gi_644);
			
			if (g_lots_652 < ld_32)
				g_lots_652 = ld_32;
				
			if (g_lots_652 > 50.0)
				g_lots_652 = 50;
				
			l_price_40 = NormalizeDouble(Bid + gd_296 * gi_620 * Point, Digits);
			
			l_price_48 = NormalizeDouble(Bid - gd_236 * gi_620 * Point, Digits);
			
			if (ECN) {
				l_price_40 = 0;
				l_price_48 = 0;
			}
			
			l_ticket_4 = OrderSend(Symbol(), OP_SELL, RightLots(g_lots_652), ND(Bid), g_slippage_344 * gi_620, l_price_40, l_price_48, "lur", g_magic_148, 0, Red);
			
			if (l_ticket_4 > 0) {
				if (gd_728 == 13.0 && gd_712 != 13.0)
					g_bars_768 = g_bars_756;
					
				gi_unused_584 = 10;
				
				g_ticket_396 = l_ticket_4;
				
				gi_420 = Now;
				
				gi_unused_388 = 10;
				
				gi_404 = 10;
				
				gi_472 = 13;
				
				gi_816 = 13;
				
				gi_unused_800 = l_ticket_4;
				
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES)) {
					Print("SELL order opened");
					g_ord_open_price_552 = OrderOpenPrice();
				}
			}
			
			else
				Print("Error opening SELL order : " + GetLastError());
				
			return;
		}
		
		return;
	}
	
	if (gi_452 == 13 && gi_176 == 1) {
		gd_436 = gd_424;
		l_ticket_4 = OrderSend(Symbol(), OP_BUY, RightLots(NormalizeDouble(gd_436 / gi_496, 2)), ND(Ask), g_slippage_344, ND(Ask - g_pips_312 * Point), ND(Ask + g_pips_252 * Point), "lur", g_magic_156, 0, Green);
		
		if (l_ticket_4 > 0) {
			gi_452 = 10;
			gi_456 = 13;
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
				Print("BUY order opened");
		}
		
		else
			Print("Error opening BUY order : " + GetLastError());
			
		return;
	}
	
	if (gi_456 == 13 && gi_180 == 1) {
		gd_436 = gd_424;
		l_ticket_4 = OrderSend(Symbol(), OP_BUY, RightLots(NormalizeDouble(gd_436 / gi_496, 2)), ND(Ask), g_slippage_344, ND(Ask - g_pips_320 * Point), ND(Ask + g_pips_260 * Point), "lur", g_magic_160, 0, Green);
		
		if (l_ticket_4 > 0) {
			gi_460 = 13;
			gi_456 = 10;
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
				Print("BUY order opened");
		}
		
		else
			Print("Error opening BUY order : " + GetLastError());
			
		return;
	}
	
	if (gi_460 == 13 && gi_184 == 1) {
		gd_436 = gd_424;
		l_ticket_4 = OrderSend(Symbol(), OP_BUY, RightLots(NormalizeDouble(gd_436 / gi_496, 2)), ND(Ask), g_slippage_344, ND(Ask - g_pips_328 * Point), ND(Ask + g_pips_268 * Point), "lur", g_magic_164, 0, Green);
		
		if (l_ticket_4 > 0) {
			gi_460 = 10;
			gi_464 = 13;
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
				Print("BUY order opened");
		}
		
		else
			Print("Error opening BUY order : " + GetLastError());
			
		return;
	}
	
	if (gi_464 == 13 && gi_188 == 1) {
		gd_436 = gd_424;
		l_ticket_4 = OrderSend(Symbol(), OP_BUY, RightLots(NormalizeDouble(gd_436 / gi_496, 2)), ND(Ask), g_slippage_344, ND(Ask - g_pips_336 * Point), ND(Ask + g_pips_276 * Point), "lur", g_magic_168, 0, Green);
		
		if (l_ticket_4 > 0) {
			gi_464 = 10;
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
				Print("BUY order opened");
		}
		
		else
			Print("Error opening BUY order : " + GetLastError());
			
		return;
	}
	
	if (gi_472 == 13 && gi_192 == 1) {
		gd_436 = gd_424;
		g_lots_652 = NormalizeDouble(gd_436 / gi_496, gi_644);
		ld_32 = NormalizeDouble(gd_628 / 2.0, gi_644);
		
		if (g_lots_652 < ld_32)
			g_lots_652 = ld_32;
			
		if (g_lots_652 > 50.0)
			g_lots_652 = 50;
			
		l_price_40 = NormalizeDouble(Bid + gd_304 * gi_620 * Point, Digits);
		
		l_price_48 = NormalizeDouble(Bid - gd_244 * gi_620 * Point, Digits);
		
		if (ECN) {
			l_price_40 = 0;
			l_price_48 = 0;
		}
		
		l_ticket_4 = OrderSend(Symbol(), OP_SELL, RightLots(g_lots_652), ND(Bid), g_slippage_344 * gi_620, l_price_40, l_price_48, "lur", g_magic_152, 0, Red);
		
		if (l_ticket_4 > 0) {
			gi_unused_804 = l_ticket_4;
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			gi_472 = 10;
			gi_476 = 13;
			gi_820 = 13;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES)) {
				Print("SELL order opened");
				g_ord_open_price_560 = OrderOpenPrice();
			}
		}
		
		else
			Print("Error opening SELL order : " + GetLastError());
			
		return;
	}
	
	if (gi_476 == 13 && gi_196 == 1) {
		gd_436 = gd_424;
		l_ticket_4 = OrderSend(Symbol(), OP_SELL, RightLots(NormalizeDouble(gd_436 / gi_500, 2)), ND(Bid), g_slippage_344, ND(Bid + g_pips_312 * Point), ND(Bid - g_pips_252 * Point), "lur", g_magic_156, 0, Red);
		
		if (l_ticket_4 > 0) {
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			gi_480 = 13;
			gi_476 = 10;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
				Print("SELL order opened");
		}
		
		else
			Print("Error opening SELL order : " + GetLastError());
			
		return;
	}
	
	if (gi_480 == 13 && gi_200 == 1) {
		gd_436 = gd_424;
		l_ticket_4 = OrderSend(Symbol(), OP_SELL, RightLots(NormalizeDouble(gd_436 / gi_500, 2)), ND(Bid), g_slippage_344, ND(Bid + g_pips_320 * Point), ND(Bid - g_pips_260 * Point), "lur", g_magic_160, 0, Red);
		
		if (l_ticket_4 > 0) {
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			gi_480 = 10;
			gi_484 = 13;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
				Print("SELL order opened");
		}
		
		else
			Print("Error opening SELL order : " + GetLastError());
			
		return;
	}
	
	if (gi_484 == 13 && gi_200 == 1) {
		gd_436 = gd_424;
		l_ticket_4 = OrderSend(Symbol(), OP_SELL, RightLots(NormalizeDouble(gd_436 / gi_500, 2)), ND(Bid), g_slippage_344, ND(Bid + g_pips_328 * Point), ND(Bid - g_pips_268 * Point), "lur", g_magic_164, 0, Red);
		
		if (l_ticket_4 > 0) {
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			gi_484 = 10;
			gi_488 = 13;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
				Print("SELL order opened");
		}
		
		else
			Print("Error opening SELL order : " + GetLastError());
			
		return;
	}
	
	if (gi_488 == 13 && gi_204 == 1) {
		gd_436 = gd_424;
		l_ticket_4 = OrderSend(Symbol(), OP_SELL, RightLots(NormalizeDouble(gd_436 / gi_500, 2)), ND(Bid), g_slippage_344, ND(Bid + g_pips_336 * Point), ND(Bid - g_pips_276 * Point), "lur", g_magic_168, 0, Red);
		
		if (l_ticket_4 > 0) {
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			gi_488 = 10;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
				Print("SELL order opened");
		}
		
		else
			Print("Error opening SELL order : " + GetLastError());
			
		return;
	}
	
	if (gi_448 == 13 && gi_172 == 1) {
		gd_436 = gd_424;
		g_lots_652 = NormalizeDouble(gd_436 / gi_496, gi_644);
		ld_32 = NormalizeDouble(gd_628 / 2.0, gi_644);
		
		if (g_lots_652 < ld_32)
			g_lots_652 = ld_32;
			
		if (g_lots_652 > 50.0)
			g_lots_652 = 50;
			
		l_price_40 = NormalizeDouble(Ask - gd_304 * gi_620 * Point, Digits);
		
		l_price_48 = NormalizeDouble(Ask + gd_244 * gi_620 * Point, Digits);
		
		if (ECN) {
			l_price_40 = 0;
			l_price_48 = 0;
		}
		
		l_ticket_4 = OrderSend(Symbol(), OP_BUY, RightLots(g_lots_652), ND(Ask), g_slippage_344 * gi_620, l_price_40, l_price_48, "lur", g_magic_152, 0, Green);
		
		if (l_ticket_4 > 0) {
			gi_unused_796 = l_ticket_4;
			gi_448 = 10;
			gi_452 = 13;
			gi_420 = Now;
			gi_unused_388 = 10;
			gi_404 = 10;
			gi_812 = 13;
			
			if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES)) {
				Print("BUY order opened");
				g_ord_open_price_544 = OrderOpenPrice();
			}
		}
		
		else
			Print("Error opening BUY order : " + GetLastError());
			
		return;
	}
	
	int l_ord_total_56 = OrdersTotal();
	
	for (int l_pos_60 = 0; l_pos_60 < l_ord_total_56; l_pos_60++) {
		OrderSelect(l_pos_60, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_148 || OrderMagicNumber() == g_magic_152 || OrderMagicNumber() == g_magic_156 ||
			OrderMagicNumber() == g_magic_160 || OrderMagicNumber() == g_magic_164 || OrderMagicNumber() == g_magic_168) {
			if (OrderType() == OP_BUY) {
				if (ECN && OrderMagicNumber() == g_magic_148 && gi_808 == 13) {
					li_64 = 70;
					li_68 = 1000;
					l_price_72 = NormalizeDouble(OrderOpenPrice() - Point * gi_620 * li_64, Digits);
					l_price_80 = NormalizeDouble(OrderOpenPrice() + Point * gi_620 * li_68, Digits);
					
					if (OrderModify(OrderTicket(), OrderOpenPrice(), l_price_72, l_price_80, 0, Green))
						gi_808 = 10;
						
					return;
				}
				
				if (ECN && OrderMagicNumber() == g_magic_152 && gi_812 == 13) {
					li_64 = 70;
					li_68 = 90;
					l_price_72 = NormalizeDouble(OrderOpenPrice() - Point * gi_620 * li_64, Digits);
					l_price_80 = NormalizeDouble(OrderOpenPrice() + Point * gi_620 * li_68, Digits);
					
					if (OrderModify(OrderTicket(), OrderOpenPrice(), l_price_72, l_price_80, 0, Green))
						gi_812 = 10;
						
					return;
				}
				
				if (Close[1] < gd_612 && Close[2] >= g_close_596) {
					OrderClose(OrderTicket(), OrderLots(), ND(Bid), 3);
					return;
				}
				
				if (OrderMagicNumber() == g_magic_148 && Bid - g_ord_open_price_536 >= 1000 * gi_620 * Point && gi_648 > 0) {
					OrderClose(OrderTicket(), OrderLots(), ND(Bid), g_slippage_344 * gi_620);
					return;
				}
				
				if (OrderMagicNumber() == g_magic_152 && Bid - g_ord_open_price_544 >= 56 * gi_620 * Point && gi_648 > 0) {
					OrderClose(OrderTicket(), OrderLots(), ND(Bid), g_slippage_344 * gi_620);
					return;
				}
				
				if (OrderMagicNumber() == g_magic_148 && g_ord_open_price_536 - Bid >= 46 * gi_620 * Point && gi_648 > 0) {
					gi_unused_580 = 13;
					OrderClose(OrderTicket(), OrderLots(), ND(Bid), g_slippage_344 * gi_620);
					return;
				}
				
				if (OrderMagicNumber() == g_magic_152 && g_ord_open_price_544 - Bid >= 46 * gi_620 * Point && gi_648 > 0) {
					OrderClose(OrderTicket(), OrderLots(), ND(Bid), g_slippage_344 * gi_620);
					return;
				}
				
				if (gi_284 > 0) {
					if (Bid - OrderOpenPrice() >= Point * gi_620 * gi_284)
						gi_404 = 13;
						
					if (gi_404 == 13 && Bid - OrderOpenPrice() <= Point * gi_620 * gi_416) {
						OrderClose(OrderTicket(), OrderLots(), ND(Bid), g_slippage_344 * gi_620);
						return;
					}
				}
				
				if (gd_348 <= 0.0)
					continue;
					
				if (Bid - OrderOpenPrice() <= Point * gi_620 * gd_348)
					continue;
					
				if (OrderStopLoss() >= Bid - Point * gi_620 * gd_348)
					continue;
					
				OrderModify(OrderTicket(), OrderOpenPrice(), ND(Bid - Point * gi_620 * gd_348), OrderTakeProfit(), 0, Green);
				
				return;
			}
			
			if (ECN && OrderMagicNumber() == g_magic_148 && gi_816 == 13) {
				li_64 = 70;
				li_68 = 1000;
				l_price_72 = NormalizeDouble(OrderOpenPrice() + Point * gi_620 * li_64, Digits);
				l_price_80 = NormalizeDouble(OrderOpenPrice() - Point * gi_620 * li_68, Digits);
				
				if (OrderModify(OrderTicket(), OrderOpenPrice(), l_price_72, l_price_80, 0, Green))
					gi_816 = 10;
					
				return;
			}
			
			if (ECN && OrderMagicNumber() == g_magic_152 && gi_820 == 13) {
				li_64 = 70;
				li_68 = 90;
				l_price_72 = NormalizeDouble(OrderOpenPrice() + Point * gi_620 * li_64, Digits);
				l_price_80 = NormalizeDouble(OrderOpenPrice() - Point * gi_620 * li_68, Digits);
				
				if (OrderModify(OrderTicket(), OrderOpenPrice(), l_price_72, l_price_80, 0, Green))
					gi_820 = 10;
					
				return;
			}
			
			if (Close[1] > gd_612 && Close[2] <= g_close_596) {
				OrderClose(OrderTicket(), OrderLots(), ND(Ask), g_slippage_344 * gi_620);
				return;
			}
			
			if (OrderMagicNumber() == g_magic_148 && g_ord_open_price_552 - Ask >= 1000 * gi_620 * Point && gi_648 > 0) {
				OrderClose(OrderTicket(), OrderLots(), ND(Ask), g_slippage_344 * gi_620);
				return;
			}
			
			if (OrderMagicNumber() == g_magic_152 && g_ord_open_price_560 - Ask >= 56 * gi_620 * Point && gi_648 > 0) {
				OrderClose(OrderTicket(), OrderLots(), ND(Ask), g_slippage_344 * gi_620);
				return;
			}
			
			if (OrderMagicNumber() == g_magic_148 && Ask - g_ord_open_price_552 >= 46 * gi_620 * Point && gi_648 > 0) {
				OrderClose(OrderTicket(), OrderLots(), ND(Ask), g_slippage_344 * gi_620);
				gi_unused_584 = 13;
				return;
			}
			
			if (OrderMagicNumber() == g_magic_152 && Ask - g_ord_open_price_560 >= 46 * gi_620 * Point && gi_648 > 0) {
				OrderClose(OrderTicket(), OrderLots(), ND(Ask), g_slippage_344 * gi_620);
				return;
			}
			
			if (gi_284 > 0) {
				if (OrderOpenPrice() - Ask >= Point * gi_620 * gi_284)
					gi_404 = 13;
					
				if (gi_404 == 13 && OrderOpenPrice() - Ask <= Point * gi_620 * gi_416) {
					OrderClose(OrderTicket(), OrderLots(), ND(Bid), g_slippage_344 * gi_620);
					return;
				}
			}
			
			if (gd_348 > 0.0) {
				if (OrderOpenPrice() - Ask > Point * gi_620 * gd_348) {
					if (OrderStopLoss() > Ask + Point * gi_620 * gd_348 || OrderStopLoss() == 0.0) {
						OrderModify(OrderTicket(), OrderOpenPrice(), ND(Ask + Point * gi_620 * gd_348), OrderTakeProfit(), 0, Red);
						return;
					}
				}
			}
		}
	}
}

int Rose::CountOpenPositions() {
	int l_cmd_12;
	int l_ord_total_0 = OrdersTotal();
	int l_count_4 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < l_ord_total_0; l_pos_8++) {
		OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES);
		l_cmd_12 = OrderType();
		
		if (l_cmd_12 == OP_SELL || l_cmd_12 == OP_BUY && OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_148 || OrderMagicNumber() == g_magic_152 || OrderMagicNumber() == g_magic_156 ||
			OrderMagicNumber() == g_magic_160 || OrderMagicNumber() == g_magic_164 || OrderMagicNumber() == g_magic_168)
			l_count_4++;
	}
	
	return (l_count_4);
}

int Rose::Minus() {
	int li_ret_0 = 0;
	
	if (g_ticket_396 > 0 && OrderSelect(g_ticket_396, SELECT_BY_TICKET, MODE_TRADES) == true)
		if (OrderProfit() <= 0.0)
			li_ret_0 = 13;
			
	return (li_ret_0);
}

int Rose::Loss() {
	int l_hist_total_0 = OrdersHistoryTotal();
	int l_count_4 = 0;
	
	for (int l_pos_8 = l_hist_total_0 - 1; l_pos_8 >= 0; l_pos_8--) {
		if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_HISTORY) == false) {
			Print("Error in history!");
			break;
		}
		
		if (OrderSymbol() != Symbol() || OrderType() > OP_SELL || OrderMagicNumber() != g_magic_148)
			continue;
			
		if (OrderProfit() > 0.0)
			break;
			
		if (OrderProfit() < 0.0)
			l_count_4++;
	}
	
	return (l_count_4);
}



double Rose::RightLots(double lots) {
	double minlot = MarketInfo(Symbol(), MODE_MINLOT);
	double maxlot = MarketInfo(Symbol(), MODE_MAXLOT);
	
	return (MathMin(maxlot, MathMax(minlot, lots)));
}

double Rose::ND(double price) {
	return (NormalizeDouble(price, Digits));
}

}
