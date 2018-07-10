#include "Overlook.h"

#if 0

// forex detector

namespace Overlook {

Sherlock::Sherlock() {

}

void Sherlock::InitEA() {
	if (Digits == 2)
		gd_604 = gd_612;
	else
		gd_604 = gd_620;
		
	gi_392 = 1000 * DayOfYear();
	
	gi_396 = gi_392;
	
	if (gi_408 == false && gi_444 == false) {
	}
	
	return (0);
}

void Sherlock::StartEA(int pos) {
	int l_ord_total_0;
	int l_ord_total_4;
	int l_ticket_8;
	int l_ticket_12;
	int l_ticket_16;
	int l_ticket_40;
	int l_error_24 = GetLastError();
	
	if (l_error_24 == 140/* LONG_POSITIONS_ONLY_ALLOWED */) {
		g_ord_total_628 = OrdersTotal();
		nr_total_orders = g_ord_total_628 - 2;
	}
	
	for (gi_540 = Bars - 2; gi_540 >= 0; gi_540--) {
		if (High[gi_540 + 1] > gd_552)
			gd_552 = High[gi_540 + 1];
			
		if (Low[gi_540 + 1] < gd_544)
			gd_544 = Low[gi_540 + 1];
			
		if (TimeDay(Time[gi_540]) != TimeDay(Time[gi_540 + 1])) {
			gd_468 = gd_460;
			gd_460 = gd_452;
			gd_452 = (gd_552 + gd_544 + (Close[gi_540 + 1])) / 3.0;
			gd_484 = 2.0 * gd_452 - gd_544;
			gd_476 = 2.0 * gd_452 - gd_552;
			gd_500 = gd_452 + (gd_552 - gd_544);
			gd_492 = gd_452 - (gd_552 - gd_544);
			gd_516 = gd_500 + (gd_552 - gd_544);
			gd_508 = gd_492 - (gd_552 - gd_544);
			gd_524 = gd_508 + (gd_516 - gd_508) / 2.0;
			gd_544 = Open[gi_540];
			gd_552 = Open[gi_540];
		}
	}
	
	if (TimeDay(Now) != TimeDay(Time[1]) && gi_584 == false) {
		ObjectsDeleteAll();
		gi_168 = true;
		gi_172 = true;
		gi_176 = true;
		gi_180 = true;
		gi_184 = true;
		gi_188 = true;
		gi_unused_592 = false;
		gi_unused_596 = false;
		gi_584 = true;
		gi_392 = 1000 * DayOfYear();
		gi_396 = gi_392;
	}
	
	if (TimeDay(Now) == TimeDay(Time[1]) && gi_584 == true)
		gi_584 = false;
		
	gd_unused_532 = gd_452;
	
	if (size_of_lot > 0.0)
		g_lots_156 = NormalizeDouble(size_of_lot, 3);
		
	if (Close_open_trades == true) {
		for (g_pos_224 = 0; g_pos_224 < OrdersTotal(); g_pos_224++) {
			if (OrderSelect(g_pos_224, SELECT_BY_POS, MODE_TRADES) != false) {
				if (OrderType() == OP_BUY && OrderSymbol() == Symbol())
					OrderClose(OrderTicket(), OrderLots(), Bid, 3, Red);
					
				if (OrderType() == OP_SELL && OrderSymbol() == Symbol())
					OrderClose(OrderTicket(), OrderLots(), Ask, 3, Lime);
			}
		}
	}
	
	if (Close_pending_trades == true) {
		for (g_pos_224 = 0; g_pos_224 < OrdersTotal(); g_pos_224++) {
			if (OrderSelect(g_pos_224, SELECT_BY_POS, MODE_TRADES) != false)
				if (OrderSymbol() == Symbol())
					OrderDelete(OrderTicket());
		}
	}
	
	if (Close_open_trades == true && Close_pending_trades == true) {
		l_ord_total_4 = OrdersTotal();
		
		if (l_ord_total_4 == 0)
			gi_588 = false;
			
		for (int l_pos_28 = 0; l_pos_28 < OrdersTotal(); l_pos_28++)
			if (OrderSelect(l_pos_28, SELECT_BY_POS, MODE_TRADES) == true)
				gi_588 = true;
				
		if (gi_588 == false)
			ObjectsDeleteAll();
	}
	
	if (Use_trail == true) {
		l_ord_total_0 = OrdersTotal();
		
		for (g_pos_580 = 0; g_pos_580 <= l_ord_total_0; g_pos_580++) {
			if (OrderSelect(g_pos_580, SELECT_BY_POS, MODE_TRADES) == true) {
				if (OrderSymbol() == Symbol() && OrderType() == OP_BUY) {
					if (OrderStopLoss() > OrderOpenPrice()) {
						if (Ask - OrderStopLoss() > Point * (trail_in_pips + trail_space) && Ask > OrderStopLoss() + Point * trail_in_pips)
							OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderStopLoss() + Point * trail_in_pips, Digits), OrderTakeProfit(), 0, Blue);
					}
					
					else
						if (Ask - OrderOpenPrice() > Point * (trail_in_pips + trail_space) && Ask > OrderOpenPrice() + Point * trail_in_pips)
							OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() + Point * trail_in_pips, Digits), OrderTakeProfit(), 0, Blue);
				}
			}
			
			if (OrderSymbol() == Symbol() && OrderType() == OP_SELL) {
				if (OrderStopLoss() < OrderOpenPrice()) {
					if (OrderStopLoss() - Bid > Point * (trail_in_pips + trail_space) && Bid < OrderStopLoss() - Point * trail_in_pips)
						OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderStopLoss() - Point * trail_in_pips, Digits), OrderTakeProfit(), 0, Blue);
				}
				
				else
					if (OrderOpenPrice() - Bid > Point * (trail_in_pips + trail_space) && Bid < OrderOpenPrice() - Point * trail_in_pips)
						OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() - Point * trail_in_pips, Digits), OrderTakeProfit(), 0, Blue);
			}
		}
	}
	
	if (Close_open_trades == false && Close_pending_trades == false) {
		l_ord_total_0 = OrdersTotal();
		
		for (g_pos_576 = 0; g_pos_576 <= l_ord_total_0; g_pos_576++) {
			if (OrderSelect(g_pos_576, SELECT_BY_POS, MODE_TRADES) == true) {
				if (OrderMagicNumber() < gi_392 && OrderSymbol() == Symbol() && OrderType() == OP_BUYLIMIT || OrderType() == OP_BUYSTOP)
					OrderDelete(OrderTicket());
					
				if (OrderMagicNumber() < gi_392 && OrderSymbol() == Symbol() && OrderType() == OP_SELLLIMIT || OrderType() == OP_SELLSTOP)
					OrderDelete(OrderTicket());
			}
		}
		
		if (Use_trend_change == true) {
			gi_unused_592 = true;
			
			for (int l_pos_32 = 0; l_pos_32 < OrdersTotal(); l_pos_32++) {
				if (OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES) != false) {
					if (OrderOpenPrice() < gd_524 && Close[0] > gd_524 + pips_over_trend)
						if (OrderType() == OP_SELL && OrderSymbol() == Symbol())
							OrderClose(OrderTicket(), OrderLots(), Ask, 3, Lime);
				}
			}
			
			gd_216 = gd_604 * (gd_484 - gd_452) / Grid_space_pips;
			
			for (gi_344 = 1; gi_344 < gd_216; gi_344++) {
				gi_168 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_348 = 0; g_pos_348 < l_ord_total_0; g_pos_348++) {
					if (OrderSelect(g_pos_348, SELECT_BY_POS, MODE_TRADES) != false)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_344 + 1 + gi_392)
							gi_168 = false;
				}
				
				if (gi_168 == true) {
					g_price_232 = NormalizeDouble(gd_452 + gi_344 * Grid_space_pips / gd_604, Digits);
					g_price_320 = NormalizeDouble(g_price_232 - SL * Point, Digits);
					
					if (g_price_232 >= gd_452 && g_price_232 < gd_484 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_232 < Ask) {
							l_ticket_8 = OrderSend(Symbol(), OP_BUYLIMIT, g_lots_156, g_price_232, 3, g_price_320, 0, g_comment_400, gi_344 + 1 + gi_392, 0, Green);
							
							if (l_ticket_8 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_500 - gd_484) / Grid_space_pips;
			
			for (g_count_352 = 0; g_count_352 < gd_216; g_count_352++) {
				gi_172 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_356 = 1; g_pos_356 < l_ord_total_0; g_pos_356++) {
					if (OrderSelect(g_pos_356, SELECT_BY_POS, MODE_TRADES) != false)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_352 + 50 + gi_392)
							gi_172 = false;
				}
				
				if (gi_172 == true) {
					g_price_240 = NormalizeDouble(gd_484 + g_count_352 * Grid_space_pips / gd_604, Digits);
					g_price_328 = NormalizeDouble(g_price_240 - SL * Point, Digits);
					
					if (g_price_240 >= gd_484 && g_price_240 < gd_500 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_240 < Ask) {
							l_ticket_8 = OrderSend(Symbol(), OP_BUYLIMIT, g_lots_156, g_price_240, 3, g_price_328, 0, g_comment_400, g_count_352 + 50 + gi_392, 0, Green);
							
							if (l_ticket_8 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_516 - gd_500) / Grid_space_pips;
			
			for (gi_360 = 1; gi_360 < gd_216; gi_360++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_364 = 0; g_pos_364 < l_ord_total_0; g_pos_364++) {
					if (OrderSelect(g_pos_364, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_360 + 100 + gi_392)
							gi_176 = false;
				}
				
				if (gi_176 == true) {
					g_price_248 = NormalizeDouble(gd_500 + (gi_360 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_336 = NormalizeDouble(g_price_248 - SL * Point, Digits);
					
					if (g_price_248 >= gd_500 && g_price_248 <= gd_516 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_248 < Ask) {
							l_ticket_12 = OrderSend(Symbol(), OP_BUYLIMIT, g_lots_156, g_price_248, 3, g_price_336, 0, g_comment_400, gi_360 + 100 + gi_392, 0, Green);
							
							if (l_ticket_12 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_476 - gd_492) / Grid_space_pips;
			
			for (g_count_376 = 0; g_count_376 < gd_216; g_count_376++) {
				gi_184 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_380 = 1; g_pos_380 < l_ord_total_0; g_pos_380++) {
					if (OrderSelect(g_pos_380, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_376 + 200 + gi_392)
							gi_184 = false;
				}
				
				if (gi_184 == true) {
					g_price_264 = NormalizeDouble(gd_476 - (g_count_376 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_304 = NormalizeDouble(g_price_264 + SL * Point, Digits);
					
					if (g_price_264 <= gd_476 && g_price_264 >= gd_492 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_264 > Bid) {
							l_ticket_12 = OrderSend(Symbol(), OP_SELLLIMIT, g_lots_156, g_price_264, 3, g_price_304, 0, g_comment_400, g_count_376 + 200 + gi_392, 0, Red);
							
							if (l_ticket_12 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_492 - gd_508) / Grid_space_pips;
			
			for (g_count_384 = 0; g_count_384 < gd_216; g_count_384++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_388 = 1; g_pos_388 < l_ord_total_0; g_pos_388++) {
					if (OrderSelect(g_pos_388, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_384 + 250 + gi_392)
							gi_188 = false;
				}
				
				if (gi_188 == true) {
					g_price_272 = NormalizeDouble(gd_492 - (g_count_384 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_312 = NormalizeDouble(g_price_272 + SL * Point, Digits);
					
					if (g_price_272 <= gd_492 && g_price_272 >= gd_508 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_272 > Bid) {
							l_ticket_12 = OrderSend(Symbol(), OP_SELLLIMIT, g_lots_156, g_price_272, 3, g_price_312, 0, g_comment_400, g_count_384 + 250 + gi_392, 0, Red);
							
							if (l_ticket_12 > 0)
								gi_396++;
						}
					}
				}
			}
		}
		
		if (gd_452 < gd_460 && gd_460 > gd_468) {
			gd_216 = gd_604 * (gd_500 - gd_484) / Grid_space_pips;
			
			for (g_count_352 = 1; g_count_352 < gd_216; g_count_352++) {
				gi_172 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_356 = 0; g_pos_356 < l_ord_total_0; g_pos_356++) {
					if (OrderSelect(g_pos_356, SELECT_BY_POS, MODE_TRADES) != false)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_352 + 50 + gi_392)
							gi_172 = false;
				}
				
				if (gi_172 == true) {
					g_price_240 = NormalizeDouble(gd_484 + g_count_352 * Grid_space_pips / gd_604, Digits);
					g_price_328 = NormalizeDouble(g_price_240 - SL * Point, Digits);
					
					if (g_price_240 >= gd_484 && g_price_240 < gd_500 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_240 >= Ask) {
							l_ticket_8 = OrderSend(Symbol(), OP_BUYSTOP, g_lots_156, g_price_240, 3, g_price_328, 0, g_comment_400, g_count_352 + 50 + gi_392, 0, Green);
							
							if (l_ticket_8 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_516 - gd_500) / Grid_space_pips;
			
			for (gi_360 = 1; gi_360 < gd_216; gi_360++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_364 = 0; g_pos_364 < l_ord_total_0; g_pos_364++) {
					if (OrderSelect(g_pos_364, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_360 + 100 + gi_392)
							gi_176 = false;
				}
				
				if (gi_176 == true) {
					g_price_248 = NormalizeDouble(gd_500 + (gi_360 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_336 = NormalizeDouble(g_price_248 - SL * Point, Digits);
					
					if (g_price_248 >= gd_500 && g_price_248 <= gd_516 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_248 >= Ask) {
							l_ticket_12 = OrderSend(Symbol(), OP_BUYSTOP, g_lots_156, g_price_248, 3, g_price_336, 0, g_comment_400, gi_360 + 100 + gi_392, 0, Green);
							
							if (l_ticket_12 > 0)
								gi_396++;
								
							if (l_ticket_12 < 0)
								Print("R2 - R3CD Pending BUY Trade failed " + GetLastError());
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_452 - gd_476) / Grid_space_pips;
			
			for (gi_368 = 1; gi_368 < gd_216; gi_368++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_372 = 0; g_pos_372 < l_ord_total_0; g_pos_372++) {
					if (OrderSelect(g_pos_372, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_368 + 150 + gi_392)
							gi_180 = false;
				}
				
				if (gi_180 == true) {
					g_price_256 = NormalizeDouble(gd_452 - (gi_368 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_296 = NormalizeDouble(g_price_256 + SL * Point, Digits);
					
					if (g_price_256 <= gd_452 && g_price_256 >= gd_476 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_256 > Bid) {
							l_ticket_12 = OrderSend(Symbol(), OP_SELLLIMIT, g_lots_156, g_price_256, 3, g_price_296, 0, g_comment_400, gi_368 + 150 + gi_392, 0, Red);
							
							if (l_ticket_12 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_476 - gd_492) / Grid_space_pips;
			
			for (g_count_376 = 1; g_count_376 < gd_216; g_count_376++) {
				gi_184 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_380 = 0; g_pos_380 < l_ord_total_0; g_pos_380++) {
					if (OrderSelect(g_pos_380, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_376 + 200 + gi_392)
							gi_184 = false;
				}
				
				if (gi_184 == true) {
					g_price_264 = NormalizeDouble(gd_476 - (g_count_376 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_304 = NormalizeDouble(g_price_264 + SL * Point, Digits);
					
					if (g_price_264 <= gd_476 && g_price_264 >= gd_492 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_264 > Bid) {
							l_ticket_12 = OrderSend(Symbol(), OP_SELLLIMIT, g_lots_156, g_price_264, 3, g_price_304, 0, g_comment_400, g_count_376 + 200 + gi_392, 0, Red);
							
							if (l_ticket_12 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_492 - gd_508) / Grid_space_pips;
			
			for (g_count_384 = 1; g_count_384 < gd_216; g_count_384++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_388 = 0; g_pos_388 < l_ord_total_0; g_pos_388++) {
					if (OrderSelect(g_pos_388, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_384 + 250 + gi_392)
							gi_188 = false;
				}
				
				if (gi_188 == true) {
					g_price_272 = NormalizeDouble(gd_492 - (g_count_384 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_312 = NormalizeDouble(g_price_272 + SL * Point, Digits);
					
					if (g_price_272 <= gd_492 && g_price_272 >= gd_508 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_272 > Bid) {
							l_ticket_12 = OrderSend(Symbol(), OP_SELLLIMIT, g_lots_156, g_price_272, 3, g_price_312, 0, g_comment_400, g_count_384 + 250 + gi_392, 0, Red);
							
							if (l_ticket_12 > 0)
								gi_396++;
						}
					}
				}
			}
		}
		
		if (gd_452 > gd_460 && gd_460 < gd_468) {
			gd_216 = gd_604 * (gd_484 - gd_452) / Grid_space_pips;
			
			for (gi_344 = 1; gi_344 < gd_216; gi_344++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_348 = 0; g_pos_348 < l_ord_total_0; g_pos_348++) {
					if (OrderSelect(g_pos_348, SELECT_BY_POS, MODE_TRADES) != false)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_344 + 1 + gi_392)
							gi_168 = false;
				}
				
				if (gi_168 == true) {
					g_price_232 = NormalizeDouble(gd_452 + gi_344 * Grid_space_pips / gd_604, Digits);
					g_price_320 = NormalizeDouble(g_price_232 - SL * Point, Digits);
					
					if (g_price_232 >= gd_452 && g_price_232 < gd_484 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_232 < Ask) {
							l_ticket_8 = OrderSend(Symbol(), OP_BUYLIMIT, g_lots_156, g_price_232, 3, g_price_320, 0, g_comment_400, gi_344 + 1 + gi_392, 0, Green);
							
							if (l_ticket_8 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_500 - gd_484) / Grid_space_pips;
			
			for (g_count_352 = 1; g_count_352 < gd_216; g_count_352++) {
				gi_172 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_356 = 0; g_pos_356 < l_ord_total_0; g_pos_356++) {
					if (OrderSelect(g_pos_356, SELECT_BY_POS, MODE_TRADES) != false)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_352 + 50 + gi_392)
							gi_172 = false;
				}
				
				if (gi_172 == true) {
					g_price_240 = NormalizeDouble(gd_484 + g_count_352 * Grid_space_pips / gd_604, Digits);
					g_price_328 = NormalizeDouble(g_price_240 - SL * Point, Digits);
					
					if (g_price_240 >= gd_484 && g_price_240 < gd_500 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_240 < Ask) {
							l_ticket_8 = OrderSend(Symbol(), OP_BUYLIMIT, g_lots_156, g_price_240, 3, g_price_328, 0, g_comment_400, g_count_352 + 50 + gi_392, 0, Green);
							
							if (l_ticket_8 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_516 - gd_500) / Grid_space_pips;
			
			for (gi_360 = 1; gi_360 < gd_216; gi_360++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_364 = 0; g_pos_364 < l_ord_total_0; g_pos_364++) {
					if (OrderSelect(g_pos_364, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_360 + 100 + gi_392)
							gi_176 = false;
				}
				
				if (gi_176 == true) {
					g_price_248 = NormalizeDouble(gd_500 + (gi_360 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_336 = NormalizeDouble(g_price_248 - SL * Point, Digits);
					
					if (g_price_248 >= gd_500 && g_price_248 <= gd_516 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_248 < Ask) {
							l_ticket_12 = OrderSend(Symbol(), OP_BUYLIMIT, g_lots_156, g_price_248, 3, g_price_336, 0, g_comment_400, gi_360 + 100 + gi_392, 0, Green);
							
							if (l_ticket_12 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_476 - gd_492) / Grid_space_pips;
			
			for (g_count_376 = 1; g_count_376 < gd_216; g_count_376++) {
				gi_184 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_380 = 0; g_pos_380 < l_ord_total_0; g_pos_380++) {
					if (OrderSelect(g_pos_380, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_376 + 200 + gi_392)
							gi_184 = false;
				}
				
				if (gi_184 == true) {
					g_price_264 = NormalizeDouble(gd_476 - (g_count_376 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_304 = NormalizeDouble(g_price_264 + SL * Point, Digits);
					
					if (g_price_264 <= gd_476 && g_price_264 >= gd_492 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_264 <= Bid) {
							l_ticket_12 = OrderSend(Symbol(), OP_SELLSTOP, g_lots_156, g_price_264, 3, g_price_304, 0, g_comment_400, g_count_376 + 200 + gi_392, 0, Red);
							
							if (l_ticket_12 > 0)
								gi_396++;
								
							if (l_ticket_12 < 0)
								Print("S1 - S2D Pending Sell Trade failed " + GetLastError());
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_492 - gd_508) / Grid_space_pips;
			
			for (g_count_384 = 1; g_count_384 < gd_216; g_count_384++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_388 = 0; g_pos_388 < l_ord_total_0; g_pos_388++) {
					if (OrderSelect(g_pos_388, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_384 + 250 + gi_392)
							gi_188 = false;
				}
				
				if (gi_188 == true) {
					g_price_272 = NormalizeDouble(gd_492 - (g_count_384 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_312 = NormalizeDouble(g_price_272 + SL * Point, Digits);
					
					if (g_price_272 <= gd_492 && g_price_272 >= gd_508 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_272 <= Bid) {
							l_ticket_12 = OrderSend(Symbol(), OP_SELLSTOP, g_lots_156, g_price_272, 3, g_price_312, 0, g_comment_400, g_count_384 + 250 + gi_392, 0, Red);
							
							if (l_ticket_12 > 0)
								gi_396++;
								
							if (l_ticket_12 < 0)
								Print("S1 - S2D Pending Sell Trade failed " + GetLastError());
						}
					}
				}
			}
		}
		
		if (Use_trend_change == true) {
			gi_unused_596 = true;
			
			for (int l_pos_36 = 0; l_pos_36 < OrdersTotal(); l_pos_36++) {
				if (OrderSelect(l_pos_36, SELECT_BY_POS, MODE_TRADES) != false) {
					if (OrderOpenPrice() > gd_524 && Close[0] < gd_524 - pips_over_trend)
						if (OrderType() == OP_BUY && OrderSymbol() == Symbol())
							OrderClose(OrderTicket(), OrderLots(), Bid, 3, Red);
				}
			}
			
			gd_216 = gd_604 * (gd_500 - gd_484) / Grid_space_pips;
			
			for (g_count_352 = 1; g_count_352 < gd_216; g_count_352++) {
				gi_172 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_356 = 0; g_pos_356 < l_ord_total_0; g_pos_356++) {
					if (OrderSelect(g_pos_356, SELECT_BY_POS, MODE_TRADES) != false)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_352 + 50 + gi_392)
							gi_172 = false;
				}
				
				if (gi_172 == true) {
					g_price_240 = NormalizeDouble(gd_484 + (g_count_352 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_280 = NormalizeDouble(g_price_240 - SL * Point, Digits);
					
					if (g_price_240 >= gd_484 && g_price_240 <= gd_500 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_240 >= Ask) {
							l_ticket_8 = OrderSend(Symbol(), OP_BUYSTOP, g_lots_156, g_price_240, 3, g_price_280, 0, g_comment_400, g_count_352 + 50 + gi_392, 0, Green);
							
							if (l_ticket_8 > 0)
								gi_396++;
								
							if (l_ticket_8 < 0)
								Print("R1 - R2D Pending buy Trade failed " + GetLastError());
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_516 - gd_500) / Grid_space_pips;
			
			for (gi_360 = 1; gi_360 < gd_216; gi_360++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_364 = 0; g_pos_364 < l_ord_total_0; g_pos_364++) {
					if (OrderSelect(g_pos_364, SELECT_BY_POS, MODE_TRADES) != false)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_360 + 100 + gi_392)
							gi_176 = false;
				}
				
				if (gi_176 == true) {
					g_price_248 = NormalizeDouble(gd_500 + (gi_360 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_288 = NormalizeDouble(g_price_248 - SL * Point, Digits);
					
					if (g_price_248 >= gd_500 && g_price_248 <= gd_516 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_248 >= Ask) {
							l_ticket_16 = OrderSend(Symbol(), OP_BUYSTOP, g_lots_156, g_price_248, 3, g_price_288, 0, g_comment_400, gi_360 + 100 + gi_392, 0, Green);
							
							if (l_ticket_16 > 0)
								gi_396++;
								
							if (l_ticket_16 < 0)
								Print("R2 - R3D Pending buy Trade failed " + GetLastError());
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_452 - gd_476) / Grid_space_pips;
			
			for (gi_368 = 1; gi_368 < gd_216; gi_368++) {
				gi_180 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_372 = 0; g_pos_372 < l_ord_total_0; g_pos_372++) {
					if (OrderSelect(g_pos_372, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_368 + 150 + gi_392)
							gi_180 = false;
				}
				
				if (gi_180 == true) {
					g_price_256 = NormalizeDouble(gd_452 - (gi_368 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_296 = NormalizeDouble(g_price_256 + SL * Point, Digits);
					
					if (g_price_256 <= gd_452 && g_price_256 >= gd_476 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_256 > Bid) {
							l_ticket_12 = OrderSend(Symbol(), OP_SELLLIMIT, g_lots_156, g_price_256, 3, g_price_296, 0, g_comment_400, gi_368 + 150 + gi_392, 0, Red);
							
							if (l_ticket_12 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_476 - gd_492) / Grid_space_pips;
			
			for (g_count_376 = 1; g_count_376 < gd_216; g_count_376++) {
				gi_184 = true;
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_380 = 0; g_pos_380 < l_ord_total_0; g_pos_380++) {
					if (OrderSelect(g_pos_380, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_376 + 200 + gi_392)
							gi_184 = false;
				}
				
				if (gi_184 == true) {
					g_price_264 = NormalizeDouble(gd_476 - (g_count_376 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_304 = NormalizeDouble(g_price_264 + SL * Point, Digits);
					
					if (g_price_264 <= gd_476 && g_price_264 >= gd_492 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_264 > Bid) {
							l_ticket_12 = OrderSend(Symbol(), OP_SELLLIMIT, g_lots_156, g_price_264, 3, g_price_304, 0, g_comment_400, g_count_376 + 200 + gi_392, 0, Red);
							
							if (l_ticket_12 > 0)
								gi_396++;
						}
					}
				}
			}
			
			gd_216 = gd_604 * (gd_492 - gd_508) / Grid_space_pips;
			
			for (g_count_384 = 1; g_count_384 < gd_216; g_count_384++) {
				l_ord_total_0 = OrdersTotal();
				
				for (g_pos_388 = 0; g_pos_388 < l_ord_total_0; g_pos_388++) {
					if (OrderSelect(g_pos_388, SELECT_BY_POS, MODE_TRADES) == true)
						if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_count_384 + 250 + gi_392)
							gi_188 = false;
				}
				
				if (gi_188 == true) {
					g_price_272 = NormalizeDouble(gd_492 - (g_count_384 + 1) * Grid_space_pips / gd_604, Digits);
					g_price_312 = g_price_272 + SL * Point;
					
					if (g_price_272 <= gd_492 && g_price_272 >= gd_508 && MathFloor(OrdersTotal() / 2) < nr_total_orders) {
						if (g_price_272 > Bid) {
							l_ticket_40 = OrderSend(Symbol(), OP_SELLLIMIT, g_lots_156, g_price_272, 3, g_price_312, 0, g_comment_400, g_count_384 + 250 + gi_392, 0, Red);
							
							if (l_ticket_40 > 0)
								gi_396++;
						}
					}
				}
			}
		}
	}
	
	gi_396 = gi_392;
	
	gi_unused_448 = true;
	return (0);
}

}

#endif
