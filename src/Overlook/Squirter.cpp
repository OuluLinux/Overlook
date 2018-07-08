#include "Overlook.h"

namespace Overlook {

Squirter::Squirter() {
	
}

void Squirter::InitEA() {
	g_digits_708 = Digits;
	
	if (g_digits_708 == 3 || g_digits_708 == 5) {
		gd_692 = 10.0 * Point;
		gd_700 = 10;
	}
	
	else {
		gd_692 = Point;
		gd_700 = 1;
	}
	
	if (minlot >= 1.0)
		gi_460 = 100000;
		
	if (minlot < 1.0)
		gi_460 = 10000;
		
	if (minlot < 0.1)
		gi_460 = 1000;
	
	AddSubCore<MovingAverage>()
		.Set("period", g_period_216);
	
	AddSubCore<RelativeStrengthIndex>()
		.Set("period", g_period_236);
	
	AddSubCore<StochasticOscillator>()
		.Set("k_period", g_period_256)
		.Set("d_period", g_period_260)
		.Set("slowing", g_slowing_264);
	
}

void Squirter::StartEA(int pos) {

	if (pos <= gi_224)
		return;
	
	if (gd_372 > 0.0)
		movebreakeven(gd_372, gd_380);
		
	if (gd_356 > 0.0)
		movetrailingstop(gd_348, gd_356);
		
	if (OrdersTotal() > 0) {
		for (g_pos_424 = 0; g_pos_424 < OrdersTotal(); g_pos_424++) {
			OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_BUY) {
					g_datetime_412 = OrderOpenTime();
					g_ord_open_price_640 = OrderOpenPrice();
				}
				
				if (OrderType() == OP_SELL) {
					g_datetime_416 = OrderOpenTime();
					g_ord_open_price_648 = OrderOpenPrice();
				}
			}
		}
	}
	
	if (tradesperbar == 1) {
		if (g_datetime_412 < Now)
			g_count_436 = 0;
		else
			g_count_436 = 1;
			
		if (g_datetime_416 < Now)
			g_count_440 = 0;
		else
			g_count_440 = 1;
	}
	
	if (tradesperbar != 1 && g_bars_428 != Bars) {
		g_count_436 = 0;
		g_count_440 = 0;
		g_bars_428 = Bars;
	}
	
	
	double l_ima_0 = At(0).GetBuffer(0).Get(pos - 0);
	
	int l_count_8 = 0;
	
	for (g_pos_424 = 1; g_pos_424 <= gi_224; g_pos_424++) {
		if (l_ima_0 >= At(0).GetBuffer(0).Get(pos - g_pos_424))
			l_count_8++;
	}
	
	int l_count_12 = 0;
	
	for (g_pos_424 = 1; g_pos_424 <= gi_224; g_pos_424++) {
		if (l_ima_0 <= At(0).GetBuffer(0).Get(pos - g_pos_424))
			l_count_12++;
	}
	
	double l_irsi_16 = At(1).GetBuffer(0).Get(pos - 0);
	
	double l_istochastic_24 = At(2).GetBuffer(0).Get(pos - 0);
	bool li_32 = false;
	bool li_36 = false;
	
	if (l_count_8 == gi_224 && gi_228 == false || (gi_228 && l_irsi_16 <= gi_240) && gi_248 == false || (gi_248 && l_istochastic_24 <= gi_268) && count(OP_BUY) == 0 ||
		(count(OP_BUY) > 0 && Ask < g_ord_open_price_640 - gd_172 * gd_692) && gi_208 == false || (gi_208 && gi_684)) {
		if (gi_344)
			li_36 = true;
		else
			li_32 = true;
			
		gi_684 = false;
		
		gi_688 = true;
	}
	
	if (l_count_12 == gi_224 && gi_228 == false || (gi_228 && l_irsi_16 >= gi_244) && gi_248 == false || (gi_248 && l_istochastic_24 >= gi_272) && count(OP_SELL) == 0 ||
		(count(OP_SELL) > 0 && Bid > g_ord_open_price_648 + gd_172 * gd_692) && gi_208 == false || (gi_208 && gi_688)) {
		if (gi_344)
			li_32 = true;
		else
			li_36 = true;
			
		gi_684 = true;
		
		gi_688 = false;
	}
	
	if (gi_340 && li_36)
		closebuy();
		
	if (gi_340 && li_32)
		closesell();
		
	if (gi_204 || gi_200) {
		hideclosesell();
		hideclosebuy();
	}
	
	if (Ask - Bid > gd_164 * gd_700 * gd_692)
		return;
		
	if (count(OP_BUY) + count(OP_SELL) >= maxtrades)
		return;
		
	if (checktime())
		return;
		
	if (mm)
		lots = lotsoptimized();
		
	Time li_40(1970,1,1);
	
	if (gi_152 > 0)
		li_40 = TimeCurrent() + 60 * gi_152 - 5;
		
	gi_484 = 0;
	
	g_count_480 = 0;
	
	if (li_32 && g_count_436 < tradesperbar) {
		if (gi_184) {
			Delete(OP_SELLSTOP);
			Delete(OP_SELLLIMIT);
		}
		
		g_lots_584 = lots;
		
		if (ecn == false) {
			if (gi_180) {
				while (gi_484 <= 0 && g_count_480 < gi_444) {
					RefreshRates();
					
					gi_484 = open(OP_BUY, g_lots_584, Ask, stoploss, takeprofit, li_40);
					
					if (gi_484 < 0)
						g_count_480++;
				}
			}
			
			if (gi_188) {
				RefreshRates();
				gi_484 = open(OP_BUYSTOP, g_lots_584, Ask + gi_196 * gd_692, stoploss, takeprofit, li_40);
			}
			
			if (gi_192) {
				RefreshRates();
				gi_484 = open(OP_BUYLIMIT, g_lots_584, Bid - gi_196 * gd_692, stoploss, takeprofit, li_40);
			}
		}
		
		if (ecn) {
			if (gi_180) {
				while (gi_484 <= 0 && g_count_480 < gi_444) {
						
					RefreshRates();
					
					gi_484 = open(OP_BUY, g_lots_584, Ask, 0, 0, li_40);
					
					if (gi_484 < 0)
						g_count_480++;
				}
			}
			
			if (gi_188) {
				RefreshRates();
				gi_484 = open(OP_BUYSTOP, g_lots_584, Ask + gi_196 * gd_692, 0, 0, li_40);
			}
			
			if (gi_192) {
				RefreshRates();
				gi_484 = open(OP_BUYLIMIT, g_lots_584, Bid - gi_196 * gd_692, 0, 0, li_40);
			}
			
			createlstoploss(stoploss);
			
			createltakeprofit(takeprofit);
		}
		
		if (gi_484 > 0)
			g_count_436++;
	}
	
	gi_484 = 0;
	
	if (li_36 && g_count_440 < tradesperbar) {
		if (gi_184) {
			Delete(OP_BUYSTOP);
			Delete(OP_BUYLIMIT);
		}
		
		g_lots_584 = lots;
		
		if (ecn == false) {
			if (gi_180) {
				while (gi_484 <= 0 && g_count_480 < gi_444) {
						
					RefreshRates();
					
					gi_484 = open(OP_SELL, g_lots_584, Bid, stoploss, takeprofit, li_40);
					
					if (gi_484 < 0)
						g_count_480++;
				}
			}
			
			if (gi_188) {
				RefreshRates();
				gi_484 = open(OP_SELLSTOP, g_lots_584, Bid - gi_196 * gd_692, stoploss, takeprofit, li_40);
			}
			
			if (gi_192) {
				RefreshRates();
				gi_484 = open(OP_SELLLIMIT, g_lots_584, Ask + gi_196 * gd_692, stoploss, takeprofit, li_40);
			}
		}
		
		if (ecn) {
			if (gi_180) {
				while (gi_484 <= 0 && g_count_480 < gi_444) {
					RefreshRates();
					
					gi_484 = open(OP_SELL, g_lots_584, Bid, 0, 0, li_40);
					
					if (gi_484 < 0)
						g_count_480++;
				}
			}
			
			if (gi_188) {
				RefreshRates();
				gi_484 = open(OP_SELLSTOP, g_lots_584, Bid - gi_196 * gd_692, 0, 0, li_40);
			}
			
			if (gi_192) {
				RefreshRates();
				gi_484 = open(OP_SELLLIMIT, g_lots_584, Ask + gi_196 * gd_692, 0, 0, li_40);
			}
			
			createsstoploss(stoploss);
			
			createstakeprofit(takeprofit);
		}
		
		if (gi_484 > 0)
			g_count_440++;
	}
	
	if (ecn) {
		createlstoploss(stoploss);
		createltakeprofit(takeprofit);
		createsstoploss(stoploss);
		createstakeprofit(takeprofit);
	}
	
}

int Squirter::open(int a_cmd_0, double ad_4, double ad_12, double ad_20, double ad_28, Time a_datetime_36) {
	int l_ticket_44 = 0;
	
	if (ad_4 < minlot)
		ad_4 = minlot;
		
	if (ad_4 > maxlot)
		ad_4 = maxlot;
		
	if (a_cmd_0 == OP_BUY || a_cmd_0 == OP_BUYSTOP || a_cmd_0 == OP_BUYLIMIT) {
		if (gi_200 == false && ad_20 > 0.0)
			g_price_568 = ad_12 - ad_20 * gd_692;
		else
			g_price_568 = 0;
			
		if (gi_204 == false && ad_28 > 0.0)
			g_price_576 = ad_12 + ad_28 * gd_692;
		else
			g_price_576 = 0;
	}
	
	if (a_cmd_0 == OP_SELL || a_cmd_0 == OP_SELLSTOP || a_cmd_0 == OP_SELLLIMIT) {
		if (gi_200 == false && ad_20 > 0.0)
			g_price_568 = ad_12 + ad_20 * gd_692;
		else
			g_price_568 = 0;
			
		if (gi_204 == false && ad_28 > 0.0)
			g_price_576 = ad_12 - ad_28 * gd_692;
		else
			g_price_576 = 0;
	}
	
	l_ticket_44 = OrderSend(Symbol(), a_cmd_0, NormalizeDouble(ad_4, gi_336), NormalizeDouble(ad_12, g_digits_708), gd_156 * gd_700, g_price_568, g_price_576, "", magic);
	return (l_ticket_44);
}

double Squirter::lotsoptimized() {
	double ld_ret_0;
	
	if (stoploss > 0.0)
		ld_ret_0 = AccountBalance() * (risk / 100.0) / (stoploss * gd_692 / MarketInfo(Symbol(), MODE_TICKSIZE) * MarketInfo(Symbol(), MODE_TICKVALUE));
	else
		ld_ret_0 = NormalizeDouble(AccountBalance() / gi_460 * minlot * risk, gi_336);
		
	return (ld_ret_0);
}

bool Squirter::checktime() {
	return false;
}

int Squirter::count(int a_cmd_0) {
	g_count_432 = 0;
	
	if (OrdersTotal() > 0) {
		for (g_pos_424 = OrdersTotal()-1; g_pos_424 >= 0; g_pos_424--) {
			OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderType() == a_cmd_0 && OrderMagicNumber() == magic)
				g_count_432++;
		}
		
		return (g_count_432);
	}
	
	return (0);
}

void Squirter::closebuy() {
	RefreshRates();
	
	if (OrdersTotal() > 0) {
		for (g_pos_424 = OrdersTotal() - 1; g_pos_424 >= 0; g_pos_424--) {
			OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY)
				OrderClose(OrderTicket(), OrderLots(), Bid, gd_156 * gd_700);
		}
	}
}

void Squirter::closesell() {
	RefreshRates();
	
	if (OrdersTotal() > 0) {
		for (g_pos_424 = OrdersTotal() - 1; g_pos_424 >= 0; g_pos_424--) {
			OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL)
				OrderClose(OrderTicket(), OrderLots(), Ask, gd_156 * gd_700);
		}
	}
}

void Squirter::hideclosebuy() {
	RefreshRates();
	
	if (OrdersTotal() > 0) {
		for (g_pos_424 = OrdersTotal() - 1; g_pos_424 >= 0; g_pos_424--) {
			OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY && (gi_200 && stoploss > 0.0 && OrderProfit() <= 10.0 * ((-1.0 * stoploss) * OrderLots()) - 10.0 * (MarketInfo(Symbol(), MODE_SPREAD) * OrderLots()) / gd_700) ||
				(gi_204 && takeprofit > 0.0 && OrderProfit() >= 10.0 * (takeprofit * OrderLots())))
				OrderClose(OrderTicket(), OrderLots(), Bid, gd_156 * gd_700);
		}
	}
}

void Squirter::hideclosesell() {
	RefreshRates();
	
	if (OrdersTotal() > 0) {
		for (g_pos_424 = OrdersTotal() - 1; g_pos_424 >= 0; g_pos_424--) {
			OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL && (gi_200 && stoploss > 0.0 && OrderProfit() <= 10.0 * ((-1.0 * stoploss) * OrderLots()) - 10.0 * (MarketInfo(Symbol(), MODE_SPREAD) * OrderLots()) / gd_700) ||
				(gi_204 && takeprofit > 0.0 && OrderProfit() >= 10.0 * (takeprofit * OrderLots())))
				OrderClose(OrderTicket(), OrderLots(), Ask, gd_156 * gd_700);
		}
	}
}

void Squirter::movebreakeven(double ad_0, double ad_8) {
	RefreshRates();
	
	if (OrdersTotal() > 0) {
		for (g_pos_424 = OrdersTotal()-1; g_pos_424 >= 0; g_pos_424--) {
			if (!OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES))
				continue;
			
			if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_BUY) {
					if (NormalizeDouble(Bid - OrderOpenPrice(), g_digits_708) < NormalizeDouble(ad_0 * gd_692, g_digits_708))
						continue;
						
					if (!(NormalizeDouble(OrderStopLoss() - OrderOpenPrice(), g_digits_708) < NormalizeDouble(ad_8 * gd_692, g_digits_708) || OrderStopLoss() == 0.0))
						continue;
						
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() + ad_8 * gd_692, g_digits_708), OrderTakeProfit());
					
					return;
				}
				
				if (NormalizeDouble(OrderOpenPrice() - Ask, g_digits_708) >= NormalizeDouble(ad_0 * gd_692, g_digits_708)) {
					if (NormalizeDouble(OrderOpenPrice() - OrderStopLoss(), g_digits_708) < NormalizeDouble(ad_8 * gd_692, g_digits_708) || OrderStopLoss() == 0.0) {
						OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() - ad_8 * gd_692, g_digits_708), OrderTakeProfit());
						return;
					}
				}
			}
		}
	}
}

void Squirter::movetrailingstop(double ad_0, double ad_8) {
	RefreshRates();
	
	if (OrdersTotal() > 0) {
		for (g_pos_424 = OrdersTotal()-1; g_pos_424 >= 0; g_pos_424--) {
			OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_BUY) {
					if (!(NormalizeDouble(Ask, g_digits_708) > NormalizeDouble(OrderOpenPrice() + ad_0 * gd_692, g_digits_708) && NormalizeDouble(OrderStopLoss(), g_digits_708) < NormalizeDouble(Bid - (ad_8 +
							gd_364) * gd_692, g_digits_708)))
						continue;
						
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Bid - ad_8 * gd_692, g_digits_708), OrderTakeProfit());
					
					return;
				}
				
				if (NormalizeDouble(Bid, g_digits_708) < NormalizeDouble(OrderOpenPrice() - ad_0 * gd_692, g_digits_708) && NormalizeDouble(OrderStopLoss(), g_digits_708) > NormalizeDouble(Ask +
						(ad_8 + gd_364) * gd_692, g_digits_708) || OrderStopLoss() == 0.0) {
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Ask + ad_8 * gd_692, g_digits_708), OrderTakeProfit());
					return;
				}
			}
		}
	}
}

void Squirter::createlstoploss(double ad_0) {
	RefreshRates();
	
	for (g_pos_424 = OrdersTotal()-1; g_pos_424 >= 0; g_pos_424--) {
		OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			if (OrderType() == OP_BUY) {
				if (OrderStopLoss() == 0.0) {
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Ask - ad_0 * gd_692, g_digits_708), OrderTakeProfit());
					return;
				}
			}
		}
	}
}

void Squirter::createsstoploss(double ad_0) {
	RefreshRates();
	
	for (g_pos_424 = OrdersTotal()-1; g_pos_424 >= 0; g_pos_424--) {
		OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			if (OrderType() == OP_SELL) {
				if (OrderStopLoss() == 0.0) {
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Bid + ad_0 * gd_692, g_digits_708), OrderTakeProfit());
					return;
				}
			}
		}
	}
}

void Squirter::createltakeprofit(double ad_0) {
	RefreshRates();
	
	for (g_pos_424 = OrdersTotal()-1; g_pos_424 >= 0; g_pos_424--) {
		OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			if (OrderType() == OP_BUY) {
				if (OrderTakeProfit() == 0.0) {
					OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), NormalizeDouble(Ask + ad_0 * gd_692, g_digits_708));
					return;
				}
			}
		}
	}
}

void Squirter::createstakeprofit(double ad_0) {
	RefreshRates();
	int l_ord_total_8 = OrdersTotal();
	
	for (g_pos_424 = OrdersTotal()-1; g_pos_424 >= 0; g_pos_424--) {
		OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			if (OrderType() == OP_SELL) {
				if (OrderTakeProfit() == 0.0) {
					OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), NormalizeDouble(Bid - ad_0 * gd_692, g_digits_708));
					return;
				}
			}
		}
	}
}

void Squirter::Delete(int a_cmd_0) {
	if (OrdersTotal() > 0) {
		for (g_pos_424 = OrdersTotal()-1; g_pos_424 >= 0; g_pos_424--) {
			OrderSelect(g_pos_424, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == a_cmd_0)
				OrderDelete(OrderTicket());
		}
	}
}

}
