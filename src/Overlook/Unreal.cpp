#include "Overlook.h"

// two face

namespace Overlook {

Unreal::Unreal() {

}

void Unreal::InitEA() {
	gi_244 = true;
	gi_248 = true;
	gi_332 = TP;
	
	AddSubCore<QuantitativeQualitativeEstimation>()
		.Set("period", SF);
		
}

void Unreal::StartEA(int pos) {
	Time l_datetime_4;
	double l_minlot_20;
	double l_minlot_32;
	double ld_40;
	double ld_48;
	double ld_56;
	double ld_64;
	int li_72;
	double l_tickvalue_80;
	bool l_bool_88;
	bool l_bool_92;
	bool l_bool_96;
	bool l_bool_100;
	
	if (Vremya != 0.0 && gi_160 == true) {
		for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
			OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number) {
				if (OrderType() == OP_SELL) {
					if (OrderOpenTime() + 60.0 * Vremya < TimeCurrent()) {
						g_ticket_256 = 0;
						g_count_260 = 0;
						
						while (g_ticket_256 < 1) {
							g_count_260++;
							RefreshRates();
							g_ticket_256 = OrderSend(Symbol(), OP_BUY, Lot, NormalizeDouble(Ask, Digits), 3, 0, 0, "Two-Face v1.2", Magic_number, 0, Lime);
							
							if (g_ticket_256 < 1) {
								Print("Îøèáêà: " + GetLastError());
								Sleep(1000);
							}
							
							else
								gi_160 = false;
								
							if (g_count_260 == 3)
								break;
						}
					}
				}
			}
		}
	}
	
	if (Vremya != 0.0 && gi_168 == true || gi_172 == true && gi_176 == false) {
		l_datetime_4 = Time(1970,1,1);
		
		for (int l_pos_8 = OrdersTotal() - 1; l_pos_8 >= 0; l_pos_8--) {
			OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number)
				if (OrderOpenTime() > l_datetime_4)
					l_datetime_4 = OrderOpenTime();
		}
		
		if (l_datetime_4 + 60.0 * Vremya < TimeCurrent()) {
			gi_176 = true;
		}
	}
	
	if (OpenALL() < 1)
		gi_156 = true;
	
	double qqe_val = At(0).GetBuffer(0).Get(pos);
	double qqe_ma = At(0).GetBuffer(1).Get(pos);
	if (qqe_val > qqe_ma)
		gi_244 = true;
		
	if (qqe_val < qqe_ma)
		gi_248 = true;
		
	if (gi_156) {
		g_ticket_256 = 0;
		g_count_260 = 0;
		
		while (g_ticket_256 < 1) {
			g_count_260++;
			RefreshRates();
			g_ticket_256 = OrderSend(Symbol(), OP_SELL, Lot, NormalizeDouble(Bid, Digits), 3, 0, 0, "Two-Face v1.2", Magic_number);
			
			if (g_ticket_256 < 1) {
				Sleep(1000);
			}
			
			else {
				gi_156 = false;
				gi_160 = true;
			}
			
			if (g_count_260 == 3)
				break;
		}
	}
	
	if (gi_160 == true && Vremya == 0.0) {
		g_ticket_256 = 0;
		g_count_260 = 0;
		
		while (g_ticket_256 < 1) {
			g_count_260++;
			RefreshRates();
			g_ticket_256 = OrderSend(Symbol(), OP_BUY, Lot, NormalizeDouble(Ask, Digits), 3, 0, 0, "Two-Face v1.2", Magic_number);
			
			if (g_ticket_256 < 1) {
				Print("Îøèáêà: " + GetLastError());
				Sleep(1000);
			}
			
			else
				gi_160 = false;
				
			if (g_count_260 == 3)
				break;
		}
	}
	
	if (OpenALL() == 2 && FindLastBuyPrice() - Ask >= MinStep * Point && QQE == false || (gi_248 == true && QQE == true && qqe_val > qqe_ma)) {
		gs_276 = "buy";
		
		if (OpenBuy() >= 1)
			g_lots_316 = NormalizeDouble(FindLastOrder(gs_276, "Lots") * Multiplier, DigitsAfterDot);
			
		if (DoubleOne == true && OpenBuy() == 1)
			g_lots_316 = Lot;
			
		g_ticket_256 = 0;
		
		g_count_260 = 0;
		
		while (g_ticket_256 < 1) {
			g_count_260++;
			RefreshRates();
			g_ticket_256 = OrderSend(Symbol(), OP_BUY, g_lots_316, NormalizeDouble(Ask, Digits), 3, 0, 0, "Two-Face v1.2", Magic_number, 0, Lime);
			
			if (g_ticket_256 < 1) {
				Print("Îøèáêà: " + GetLastError());
				Sleep(1000);
			}
			
			else {
				if (SecondSide == true)
					gi_172 = true;
					
				if (SecondSide == false)
					gd_228 = 1;
					
				gi_248 = false;
			}
			
			if (g_count_260 == 3)
				break;
		}
	}
	
	if (OpenALL() == 2 && Bid - FindLastSellPrice() >= MinStep * Point && QQE == false || (gi_244 == true && QQE == true && qqe_val < qqe_ma)) {
		gs_276 = "sell";
		
		if (OpenSell() >= 1)
			g_lots_316 = NormalizeDouble(FindLastOrder(gs_276, "Lots") * Multiplier, DigitsAfterDot);
			
		if (DoubleOne == true && OpenSell() == 1)
			g_lots_316 = Lot;
			
		g_ticket_256 = 0;
		
		g_count_260 = 0;
		
		while (g_ticket_256 < 1) {
			g_count_260++;
			RefreshRates();
			g_ticket_256 = OrderSend(Symbol(), OP_SELL, g_lots_316, NormalizeDouble(Bid, Digits), 3, 0, 0, "Two-Face v1.2", Magic_number);
			
			if (g_ticket_256 < 1) {
				Print("Îøèáêà: " + GetLastError());
				Sleep(1000);
			}
			
			else {
				if (SecondSide == true)
					gi_168 = true;
					
				if (SecondSide == false)
					gd_236 = 1;
					
				gi_244 = false;
			}
			
			if (g_count_260 == 3)
				break;
		}
	}
	
	gd_196 = 0;
	
	gd_204 = 0;
	
	for (int l_pos_12 = OrdersTotal() - 1; l_pos_12 >= 0; l_pos_12--) {
		OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number) {
			if (OrderType() == OP_SELL)
				gd_196 += OrderLots();
				
			if (OrderType() == OP_BUY)
				gd_204 += OrderLots();
		}
	}
	
	if (gd_204 > gd_196 && FindLastBuyPrice() - Ask >= MinStep * Point && (DoubleOne == true && OpenBuy() <= MaxTrades) || (DoubleOne == false && OpenBuy() < MaxTrades) &&
		QQE == false || (gi_248 == true && QQE == true && qqe_val > qqe_ma)) {
		if (OpenBuy() >= 1)
			g_lots_316 = NormalizeDouble(FindLastOrder("buy", "Lots") * Multiplier, DigitsAfterDot);
			
		g_ticket_256 = 0;
		
		g_count_260 = 0;
		
		while (g_ticket_256 < 1) {
			g_count_260++;
			RefreshRates();
			g_ticket_256 = OrderSend(Symbol(), OP_BUY, g_lots_316, NormalizeDouble(Ask, Digits), 3, 0, 0, "Two-Face v1.2", Magic_number, 0, Lime);
			
			if (g_ticket_256 < 1) {
				Print("Îøèáêà: " + GetLastError());
				Sleep(1000);
			}
			
			else {
				if (SecondSide == true)
					gi_172 = true;
					
				if (SecondSide == false)
					gd_228 = 1;
					
				gi_248 = false;
			}
			
			if (g_count_260 == 3)
				break;
		}
	}
	
	if (gd_204 < gd_196 && Bid - FindLastSellPrice() >= MinStep * Point && (DoubleOne == true && OpenSell() <= MaxTrades) || (DoubleOne == false && OpenSell() < MaxTrades) &&
		QQE == false || (gi_244 == true && QQE == true && qqe_val < qqe_ma)) {
		if (OpenSell() >= 1)
			g_lots_316 = NormalizeDouble(FindLastOrder("sell", "Lots") * Multiplier, DigitsAfterDot);
			
		g_ticket_256 = 0;
		
		g_count_260 = 0;
		
		while (g_ticket_256 < 1) {
			g_count_260++;
			RefreshRates();
			g_ticket_256 = OrderSend(Symbol(), OP_SELL, g_lots_316, NormalizeDouble(Bid, Digits), 3, 0, 0, "Two-Face v1.2", Magic_number);
			
			if (g_ticket_256 < 1) {
				Print("Îøèáêà: " + GetLastError());
				Sleep(1000);
			}
			
			else {
				if (SecondSide == true)
					gi_168 = true;
					
				if (SecondSide == false)
					gd_236 = 1;
					
				gi_244 = false;
			}
			
			if (g_count_260 == 3)
				break;
		}
	}
	
	TP = gi_332;
	
	if (gi_104 == true)
		TP /= (OpenBuy() - 1);
		
	if (gi_172 == true || gd_228 == 1.0) {
		gd_296 = 0;
		gd_308 = 0;
		gd_180 = 0;
		gd_188 = 0;
		
		for (int l_pos_16 = OrdersTotal() - 1; l_pos_16 >= 0; l_pos_16--) {
			OrderSelect(l_pos_16, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number) {
				if (OrderType() == OP_SELL) {
					gd_296 += OrderOpenPrice() * OrderLots();
					gd_180 += OrderLots();
				}
				
				if (OrderType() == OP_BUY) {
					gd_308 += OrderOpenPrice() * OrderLots();
					gd_188 += OrderLots();
				}
			}
		}
		
		gd_220 = NormalizeDouble(gd_308 / gd_188, Digits);
	}
	
	if (gi_172 == true && Vremya == 0.0 || (Vremya != 0.0 && gi_176 == true)) {
		RefreshRates();
		l_minlot_20 = NormalizeDouble(((gd_220 + TP * 2 * Point) * gd_180 - gd_296) / (Bid - (gd_220 + (TP * 2 + MarketInfo(Symbol(), MODE_SPREAD)) * Point)), DigitsAfterDot);
		
		if (MultiplierSS != 0.0)
			l_minlot_20 = NormalizeDouble(FindLastOrder("sell", "Lots") * MultiplierSS, DigitsAfterDot);
			
		if (l_minlot_20 < MarketInfo(Symbol(), MODE_MINLOT))
			l_minlot_20 = MarketInfo(Symbol(), MODE_MINLOT);
			
		g_ticket_256 = 0;
		
		g_count_260 = 0;
		
		while (g_ticket_256 < 1) {
			g_count_260++;
			RefreshRates();
			g_ticket_256 = OrderSend(Symbol(), OP_SELL, l_minlot_20, NormalizeDouble(Bid, Digits), 3, 0, 0, "Two-Face v1.2", Magic_number);
			
			if (g_ticket_256 < 1) {
				Print("Îøèáêà: " + GetLastError());
				Sleep(1000);
			}
			
			else {
				gi_172 = false;
				gd_228 = 1;
				gi_176 = false;
			}
			
			if (g_count_260 == 3)
				break;
		}
	}
	
	if (gi_168 == true || gd_236 == 1.0) {
		gd_296 = 0;
		gd_308 = 0;
		gd_180 = 0;
		gd_188 = 0;
		
		for (int l_pos_28 = OrdersTotal() - 1; l_pos_28 >= 0; l_pos_28--) {
			OrderSelect(l_pos_28, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number) {
				if (OrderType() == OP_SELL) {
					gd_296 += OrderOpenPrice() * OrderLots();
					gd_180 += OrderLots();
				}
				
				if (OrderType() == OP_BUY) {
					gd_308 += OrderOpenPrice() * OrderLots();
					gd_188 += OrderLots();
				}
			}
		}
		
		gd_212 = NormalizeDouble(gd_296 / gd_180, Digits);
	}
	
	if (gi_168 == true && Vremya == 0.0 || (Vremya != 0.0 && gi_176 == true)) {
		RefreshRates();
		l_minlot_32 = NormalizeDouble(((gd_212 - TP * 2 * Point) * gd_188 - gd_308) / (Ask - (gd_212 - (TP * 2 + MarketInfo(Symbol(), MODE_SPREAD)) * Point)), DigitsAfterDot);
		
		if (MultiplierSS != 0.0)
			l_minlot_32 = NormalizeDouble(FindLastOrder("buy", "Lots") * MultiplierSS, DigitsAfterDot);
			
		if (l_minlot_32 < MarketInfo(Symbol(), MODE_MINLOT))
			l_minlot_32 = MarketInfo(Symbol(), MODE_MINLOT);
			
		g_ticket_256 = 0;
		
		g_count_260 = 0;
		
		while (g_ticket_256 < 1) {
			g_count_260++;
			RefreshRates();
			g_ticket_256 = OrderSend(Symbol(), OP_BUY, l_minlot_32, NormalizeDouble(Ask, Digits), 3, 0, 0, "Two-Face v1.2", Magic_number, 0, Lime);
			
			if (g_ticket_256 < 1) {
				Print("Îøèáêà: " + GetLastError());
				Sleep(1000);
			}
			
			else {
				gi_168 = false;
				gd_236 = 1;
				gi_176 = false;
			}
			
			if (g_count_260 == 3)
				break;
		}
	}
	
	if (gd_236 == 1.0 || gd_228 == 1.0 && Abs0 == true) {
		ld_40 = 0;
		ld_48 = 0;
		ld_56 = 0;
		ld_64 = 0;
		li_72 = OpenALL();
		
		for (int l_pos_76 = li_72 - 1; l_pos_76 >= 0; l_pos_76--) {
			if (OrderSelect(l_pos_76, SELECT_BY_POS)) {
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
					continue;
					
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number) {
					if (OrderType() == OP_BUY) {
						ld_40 += OrderLots();
						ld_56 = ld_56 + OrderProfit() + OrderCommission() + OrderSwap();
					}
					
					if (OrderType() == OP_SELL) {
						ld_48 += OrderLots();
						ld_64 = ld_64 + OrderProfit() + OrderCommission() + OrderSwap();
					}
				}
			}
		}
		
		l_tickvalue_80 = MarketInfo(Symbol(), MODE_TICKVALUE);
		
		if (ld_40 - ld_48 > 0.0)
			gd_220 = NormalizeDouble(Bid - (ld_56 + ld_64) / (l_tickvalue_80 * (ld_40 - ld_48)) * Point, Digits);
			
		if (ld_48 - ld_40 > 0.0)
			gd_212 = NormalizeDouble(Ask + (ld_56 + ld_64) / (l_tickvalue_80 * (ld_48 - ld_40)) * Point, Digits);
	}
	
	if (gd_228 == 1.0) {
		if (gi_104 == true)
			TP /= (OpenBuy() - 1);
			
		for (g_pos_264 = OrdersTotal() - 1; g_pos_264 >= 0; g_pos_264--) {
			OrderSelect(g_pos_264, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number) {
				if (OrderType() == OP_SELL) {
					g_count_260 = 0;
					l_bool_88 = false;
					
					while (l_bool_88 == false) {
						g_count_260++;
						RefreshRates();
						l_bool_88 = OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(gd_220 + (TP + MarketInfo(Symbol(), MODE_SPREAD)) * Point, Digits), OrderTakeProfit(), 0, Blue);
						
						if (l_bool_88 == false) {
							Print("Îøèáêà: " + GetLastError());
							Sleep(1000);
						}
						
						if (g_count_260 == 3)
							break;
					}
				}
			}
			
			if (OrderType() == OP_BUY) {
				g_count_260 = 0;
				l_bool_92 = false;
				
				while (l_bool_92 == false) {
					g_count_260++;
					RefreshRates();
					l_bool_92 = OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), NormalizeDouble(gd_220 + TP * Point, Digits), 0, Blue);
					
					if (l_bool_92 == false) {
						Print("Îøèáêà: " + GetLastError());
						Sleep(1000);
					}
					
					if (g_count_260 == 3)
						break;
				}
			}
		}
		
		gd_228 = 0;
	}
	
	if (gd_236 == 1.0) {
		for (g_pos_264 = OrdersTotal() - 1; g_pos_264 >= 0; g_pos_264--) {
			OrderSelect(g_pos_264, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number) {
				if (OrderType() == OP_SELL) {
					g_count_260 = 0;
					l_bool_96 = false;
					
					while (l_bool_96 == false) {
						g_count_260++;
						RefreshRates();
						l_bool_96 = OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), NormalizeDouble(gd_212 - TP * Point, Digits), 0, Blue);
						
						if (l_bool_96 == false) {
							Print("Îøèáêà: " + GetLastError());
							Sleep(1000);
						}
						
						if (g_count_260 == 3)
							break;
					}
				}
			}
			
			if (OrderType() == OP_BUY) {
				g_count_260 = 0;
				l_bool_100 = false;
				
				while (l_bool_100 == false) {
					g_count_260++;
					RefreshRates();
					l_bool_100 = OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(gd_212 - (TP + MarketInfo(Symbol(), MODE_SPREAD)) * Point, Digits), OrderTakeProfit(), 0, Blue);
					
					if (l_bool_100 == false) {
						Print("Îøèáêà: " + GetLastError());
						Sleep(1000);
					}
					
					if (g_count_260 == 3)
						break;
				}
			}
		}
		
		gd_236 = 0;
	}
	
}

double Unreal::FindLastBuyPrice() {
	double l_ord_open_price_0;
	int l_ticket_8;
	double ld_unused_12 = 0;
	int l_ticket_20 = 0;
	
	for (int l_pos_24 = OrdersTotal() - 1; l_pos_24 >= 0; l_pos_24--) {
		OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number && OrderType() == OP_BUY) {
			l_ticket_8 = OrderTicket();
			
			if (l_ticket_8 > l_ticket_20) {
				l_ord_open_price_0 = OrderOpenPrice();
				ld_unused_12 = l_ord_open_price_0;
				l_ticket_20 = l_ticket_8;
			}
		}
	}
	
	return (l_ord_open_price_0);
}

double Unreal::FindLastSellPrice() {
	double l_ord_open_price_0;
	int l_ticket_8;
	double ld_unused_12 = 0;
	int l_ticket_20 = 0;
	
	for (int l_pos_24 = OrdersTotal() - 1; l_pos_24 >= 0; l_pos_24--) {
		OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number && OrderType() == OP_SELL) {
			l_ticket_8 = OrderTicket();
			
			if (l_ticket_8 > l_ticket_20) {
				l_ord_open_price_0 = OrderOpenPrice();
				ld_unused_12 = l_ord_open_price_0;
				l_ticket_20 = l_ticket_8;
			}
		}
	}
	
	return (l_ord_open_price_0);
}

double Unreal::FindLastOrder(String as_0, String as_8) {
	double l_ord_open_price_16;
	double l_ord_lots_24;
	int l_ticket_36 = 0;
	
	for (int l_pos_32 = OrdersTotal() - 1; l_pos_32 >= 0; l_pos_32--) {
		OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number) {
			if (as_0 == "buy") {
				if (OrderType() == OP_BUY) {
					if (OrderTicket() > l_ticket_36) {
						l_ord_open_price_16 = OrderOpenPrice();
						l_ord_lots_24 = OrderLots();
						l_ticket_36 = OrderTicket();
					}
				}
			}
			
			if (as_0 == "sell") {
				if (OrderType() == OP_SELL) {
					if (OrderTicket() > l_ticket_36) {
						l_ord_open_price_16 = OrderOpenPrice();
						l_ord_lots_24 = OrderLots();
						l_ticket_36 = OrderTicket();
					}
				}
			}
		}
	}
	
	if (as_8 == "Price")
		return (l_ord_open_price_16);
		
	if (as_8 == "Lots")
		return (l_ord_lots_24);
		
	return (0.0);
}

int Unreal::OpenBuy() {
	g_count_272 = 0;
	
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number)
			if (OrderType() == OP_BUY)
				g_count_272++;
	}
	
	return (g_count_272);
}

int Unreal::OpenSell() {
	g_count_272 = 0;
	
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number)
			if (OrderType() == OP_SELL)
				g_count_272++;
	}
	
	return (g_count_272);
}

int Unreal::OpenALL() {
	g_count_272 = 0;
	
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number)
			if (OrderType() == OP_SELL || OrderType() == OP_BUY)
				g_count_272++;
	}
	
	return (g_count_272);
}

double Unreal::Balance(String as_0, String as_8) {
	double ld_ret_16 = 0;
	
	for (int l_pos_24 = OrdersTotal() - 1; l_pos_24 >= 0; l_pos_24--) {
		OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != Magic_number)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic_number) {
			if (as_0 == "buy") {
				if (OrderType() == OP_BUY) {
					if (as_8 == "Balance")
						ld_ret_16 = ld_ret_16 + OrderProfit() - OrderSwap() - OrderCommission();
						
					if (as_8 == "Lot")
						ld_ret_16 += OrderLots();
				}
			}
			
			if (as_0 == "sell") {
				if (OrderType() == OP_SELL) {
					if (as_8 == "Balance")
						ld_ret_16 = ld_ret_16 + OrderProfit() - OrderSwap() - OrderCommission();
						
					if (as_8 == "Lot")
						ld_ret_16 += OrderLots();
				}
			}
		}
	}
	
	return (ld_ret_16);
}

}
