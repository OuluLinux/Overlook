#include "Overlook.h"

// royale wave

namespace Overlook {

Roulette::Roulette() {

}

void Roulette::InitEA() {
	
}

void Roulette::StartEA(int pos) {
	gd_108 = SizeLot;
	
	if (gi_236 == 1) {
		if (gi_228 != 0)
			gd_284 = MathCeil(AccountBalance() * gi_232 / 10000.0);
		else
			gd_284 = gd_108;
	}
	
	else {
		if (gi_228 != 0)
			gd_284 = MathCeil(AccountBalance() * gi_232 / 10000.0) / 10.0;
		else
			gd_284 = gd_108;
	}
	
	if (gd_284 > 100.0)
		gd_284 = 100;
		
	g_count_240 = 0;
	
	for (g_pos_244 = 0; g_pos_244 < OrdersTotal(); g_pos_244++) {
		OrderSelect(g_pos_244, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol())
			g_count_240++;
	}
	
	if (g_count_240 < 1) {
		if (TimeYear(TimeCurrent()) < gi_204)
			return;
			
		if (TimeMonth(TimeCurrent()) < gi_208)
			return;
			
		if (TimeYear(TimeCurrent()) > gi_212)
			return;
			
		if (TimeMonth(TimeCurrent()) > gi_216)
			return;
	}
	
	if (Symbol() == "EURUSD")
		gd_368 = gd_156;
		
	if (Symbol() == "GBPUSD")
		gd_368 = gd_164;
		
	if (Symbol() == "USDJPY")
		gd_368 = gd_180;
		
	if (Symbol() == "USDCHF")
		gd_368 = gd_172;
		
	if (Symbol() == "GBPJPY")
		gd_368 = gd_188;
		
	if (Symbol() == "EURJPY")
		gd_368 = gd_196;
		
	if (gd_368 == 0.0)
		gd_368 = 5;
		
	if (g_count_240 >= MaxTrades)
		gi_312 = false;
	else
		gi_312 = true;
		
	int li_0 = 0;
	
	if (TimeMinute(Time(1970,1,1) + gda_396[li_0][0]) == 0 && TimeHour(Time(1970,1,1) + gda_396[li_0][0]) == 0)
		g_ord_open_price_316 = 0;
		
	if (g_ord_open_price_316 == 0.0) {
		for (g_pos_244 = 0; g_pos_244 < OrdersTotal(); g_pos_244++) {
			OrderSelect(g_pos_244, SELECT_BY_POS, MODE_TRADES);
			g_cmd_300 = OrderType();
			
			if (OrderSymbol() == Symbol()) {
				g_ord_open_price_316 = OrderOpenPrice();
				
				if (g_cmd_300 == OP_BUY)
					gi_304 = 2;
					
				if (g_cmd_300 == OP_SELL)
					gi_304 = 1;
			}
		}
	}
	
	if (g_count_240 < 1) {
		gi_304 = 3;
		
		if (OpenPosition == true)
			gi_304 = 2;
			
		if (OpenPosition == false)
			gi_304 = 1;
			
		if (gi_152 == 1) {
			if (gi_304 == 1)
				gi_304 = 2;
			else
				if (gi_304 == 2)
					gi_304 = 1;
		}
	}
	
	if (!IsTesting() || IsTesting()) {
		if (gi_304 == 3)
			gs_376 = "Waiting for next Signal";
		else
			gs_376 = " ";
	}
	
	if (gi_304 == 1 && gi_312 == true) {
		if (Bid - g_ord_open_price_316 >= Pips * Point || g_count_240 < 1) {
			g_bid_276 = Bid;
			g_ord_open_price_316 = 0;
			
			if (TakeProfit == 0.0)
				g_price_260 = 0;
			else
				g_price_260 = g_bid_276 - TakeProfit * Point;
				
			if (StopLoss == 0.0)
				g_price_252 = 0;
			else
				g_price_252 = g_bid_276 + StopLoss * Point;
				
			if (g_count_240 != 0) {
				g_lots_292 = gd_284;
				
				for (g_pos_244 = 1; g_pos_244 <= g_count_240; g_pos_244++) {
					if (MaxTrades > 12)
						g_lots_292 = NormalizeDouble(1.0 * g_lots_292, 2);
					else
						g_lots_292 = NormalizeDouble(1.0 * g_lots_292, 2);
				}
			}
			
			else
				g_lots_292 = gd_284;
				
			if (g_lots_292 > 100.0)
				g_lots_292 = 100;
				
			OrderSend(Symbol(), OP_SELL, g_lots_292, g_bid_276, g_slippage_248, g_price_252, g_price_260, 0, 0, 0, Red);
			
			return;
		}
	}
	
	if (gi_304 == 2 && gi_312 == true) {
		if (g_ord_open_price_316 - Ask >= Pips * Point || g_count_240 < 1) {
			g_ask_268 = Ask;
			g_ord_open_price_316 = 0;
			
			if (TakeProfit == 0.0)
				g_price_260 = 0;
			else
				g_price_260 = g_ask_268 + TakeProfit * Point;
				
			if (StopLoss == 0.0)
				g_price_252 = 0;
			else
				g_price_252 = g_ask_268 - StopLoss * Point;
				
			if (g_count_240 != 0) {
				g_lots_292 = gd_284;
				
				for (g_pos_244 = 1; g_pos_244 <= g_count_240; g_pos_244++) {
					if (MaxTrades > 12)
						g_lots_292 = NormalizeDouble(1.0 * g_lots_292, 2);
					else
						g_lots_292 = NormalizeDouble(1.0 * g_lots_292, 2);
				}
			}
			
			else
				g_lots_292 = gd_284;
				
			if (g_lots_292 > 100.0)
				g_lots_292 = 100;
				
			OrderSend(Symbol(), OP_BUY, g_lots_292, g_ask_268, g_slippage_248, g_price_252, g_price_260, 0, 0, 0, Blue);
			
			return;
		}
	}
	
}

}
