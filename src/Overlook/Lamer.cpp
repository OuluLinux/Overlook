#include "Overlook.h"

namespace Overlook {

Lamer::Lamer() {
	
}

void Lamer::InitEA() {
	
	AddSubCore<StochasticOscillator>()
		.Set("k_period", stoch_k_period)
		.Set("d_period", stoch_d_period)
		.Set("slowing", stoch_slowing);
	
	AddSubCore<Envelopes>()
		.Set("period", env_period)
		.Set("deviation", env_deviation)
		.Set("method", 1);
}

void Lamer::StartEA(int pos) {
	this->pos = pos;
	if (pos < 1)
		return;
	
	int li_0;
	int li_4;
	int li_8;
	bool li_12;
	int l_timeframe_16;
	double ld_20;
	double ld_28;
	double ld_36;
	double l_str2dbl_44;
	double l_str2dbl_52;
	double l_str2dbl_60;
	int ls_68;
	String ls_76;
	double ld_unused_84;
	double ld_unused_92;
	int li_unused_100;
	int li_104;
	int li_unused_108;
	double ld_112;
	double l_ord_lots_120;
	double l_ord_lots_128;
	double l_iclose_136;
	double l_iclose_144;
	double ld_152;
	int li_160;
	int li_164;
	
	if (Year() == 2009 && Month() == 3 && (Day() > 17 && Day() < 26))
		gd_192 = 300;
	else {
		if (Year() == 2009 && Month() == 5 && (Day() > 15 && Day() < 30))
			gd_192 = 300;
		else
			gd_192 = Dist1;
	}
	
	if (true) {
		if (MarketInfo(Symbol(), MODE_LOTSTEP) == 0.1)
			gd_372 = 1;
		else
			gd_372 = 2;
			
		li_12 = false;
		
		li_0 = 14;
		
		li_4 = 10;
		
		li_8 = 2010;
		
		if (Year() < li_8)
			li_12 = true;
			
		if (Year() == li_8 && Month() < li_4)
			li_12 = true;
			
		if (Year() == li_8 && Month() == li_4 && Day() <= li_0)
			li_12 = true;
			
		if (true) {
			if (Year() > 2008) {
				gd_192 = Dist1;
				
				for (int l_pos_168 = 0; l_pos_168 <= OrdersTotal() - 1; l_pos_168++) {
					OrderSelect(l_pos_168, SELECT_BY_POS, MODE_TRADES);
					
					if (OrderSymbol() == Symbol() && OrderMagicNumber() == 31300 || OrderMagicNumber() == 31301) {
						if (OrderType() == OP_BUY)
							if (OrderOpenPrice() - Bid > Stoploss / 10000.0)
								OrderClose(OrderTicket(), OrderLots(), Bid, 2);
								
						if (OrderType() == OP_SELL)
							if (Ask - OrderOpenPrice() > Stoploss / 10000.0)
								OrderClose(OrderTicket(), OrderLots(), Ask, 2);
					}
				}
				
				if (TrailSL == true) {
					if (CountTradesB() == 1) {
						for (int l_pos_172 = 0; l_pos_172 <= OrdersTotal() - 1; l_pos_172++) {
							OrderSelect(l_pos_172, SELECT_BY_POS, MODE_TRADES);
							
							if (OrderSymbol() == Symbol() && OrderMagicNumber() == 31300 || OrderMagicNumber() == 31302) {
								if (OrderType() == OP_BUY)
									if (Bid - OrderOpenPrice() >= TrailPips / 10000.0 && OrderStopLoss() < Bid - TrailPips / 10000.0 - 0.0002)
										OrderModify(OrderTicket(), OrderOpenPrice(), Bid - TrailPips / 10000.0 + 0.0002, Bid - TrailPips / 10000.0 + 0.0002 + 0.002);
							}
						}
					}
					
					if (CountTradesS() == 1) {
						for (int l_pos_172 = 0; l_pos_172 <= OrdersTotal() - 1; l_pos_172++) {
							OrderSelect(l_pos_172, SELECT_BY_POS, MODE_TRADES);
							
							if (OrderSymbol() == Symbol() && OrderMagicNumber() == 31302 || OrderMagicNumber() == 31302) {
								if (OrderType() == OP_SELL)
									if (OrderOpenPrice() - Ask >= TrailPips / 10000.0 && OrderStopLoss() > Ask + TrailPips / 10000.0 - 0.0002)
										OrderModify(OrderTicket(), OrderOpenPrice(), Ask + TrailPips / 10000.0 - 0.0002, Ask + TrailPips / 10000.0 - 0.0002 - 0.002);
							}
						}
					}
				}
				
				l_timeframe_16 = Period();
				
				if (Hedging == true) {
					if (gi_184 == true)
						g_magic_268 = 31301;
					else
						g_magic_268 = 12378;
						
					if (true) {
						if (ld_20 == ld_28) {
							
							gi_unused_416 = 0;
							
							if (gi_184 == true) {
								gi_448 = false;
								
								gd_160 = AccountFreeMargin() / 1000.0;
								gd_168 = MarketInfo(Symbol(), MODE_MARGINREQUIRED) * MarketInfo(Symbol(), MODE_MINLOT);
								
								if (AutoLots == true) {
									Lots = 0;
									gd_176 = MathFloor(AccountFreeMargin() / (AccountLeverage() * gd_168)) * MarketInfo(Symbol(), MODE_MINLOT);
									
									if (gd_176 < MarketInfo(Symbol(), MODE_MINLOT))
										Lots = MarketInfo(Symbol(), MODE_MINLOT);
									else
										Lots = gd_176;
								}
								
								if (gd_168 <= gd_160)
									gi_unused_416 = 1;
								else {
									li_104 = AccountLeverage() * gd_168;
									ls_76 = ls_76
											+ "\nFree Margin should be around: " + li_104 + " [atleast]";
								}
								
							}
							
							li_unused_108 = TakeProfit;
							
							Comment(ls_76);
							
							
							ld_112 = CalculateProfit();
							
							gi_436 = CountTrades();
							
							if (gi_436 == 0)
								gi_388 = false;
								
							for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
								OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
								
								if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
									continue;
									
								if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
									if (OrderType() == OP_BUY) {
										gi_408 = true;
										gi_412 = false;
										l_ord_lots_120 = OrderLots();
										break;
									}
								}
								
								if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
									if (OrderType() == OP_SELL) {
										gi_408 = false;
										gi_412 = true;
										l_ord_lots_128 = OrderLots();
										break;
									}
								}
							}
							
							if (gi_436 > 0 && gi_436 <= MaxTrades) {
								RefreshRates();
								
								if (gi_436 == 1)
									gd_192 = Dist2;
									
								if (gi_436 > 2)
									gd_192 = Dist1;
									
								if (gi_436 > 4)
									gd_192 = Dist3;
									
								gd_340 = LastBuyPrice();
								
								gd_348 = LastSellPrice();
								
								if (gi_188 != false)
									gd_192 = 2.5 * (Dist1 * gi_436);
									
								if (gi_408 && gd_340 - Ask >= gd_192 / 10000.0)
									gi_404 = true;
									
								if (gi_412 && Bid - gd_348 >= gd_192 / 10000.0)
									gi_404 = true;
							}
							
							if (gi_436 < 1) {
								gi_412 = false;
								gi_408 = false;
								gi_404 = true;
								gd_280 = AccountEquity();
							}
							
							if (gi_404) {
								gd_340 = LastBuyPrice();
								gd_348 = LastSellPrice();
								
								if (gi_412) {
									if (gi_252) {
										fOrderCloseMarket(0, 1);
										gd_424 = NormalizeDouble(gd_260 * l_ord_lots_128, gd_372);
									}
									
									else
										gd_424 = fGetLots(OP_SELL);
										
									if (gi_256) {
										gi_400 = gi_436;
										
										if (gd_424 > 0.0) {
											RefreshRates();
											
											if (gi_420 < 0) {
												Print("Error: " + GetLastError());
												return;
											}
											
											gd_348 = LastSellPrice();
											
											gi_404 = false;
											gi_448 = true;
										}
									}
								}
								
								else {
									if (gi_408) {
										if (gi_252) {
											fOrderCloseMarket(1, 0);
											gd_424 = NormalizeDouble(gd_260 * l_ord_lots_120, gd_372);
										}
										
										else
											gd_424 = fGetLots(OP_BUY);
											
										if (gi_256) {
											gi_400 = gi_436;
											
											if (gd_424 > 0.0) {
												gi_420 = OpenPendingOrder(0, gd_424, Ask, gd_364, Bid, 0, 0, gs_eurusd_392 + "-" + gi_400, g_magic_268);
												
												if (gi_420 < 0) {
													Print("Error: " + GetLastError());
													return;
												}
												
												gd_340 = LastBuyPrice();
												
												gi_404 = false;
												gi_448 = true;
											}
										}
									}
								}
							}
							
							if (gi_404 && gi_436 < 1) {
								ConstBuffer& open_buf = GetInputBuffer(0, 0);
								l_iclose_136 = open_buf.Get(pos-1);
								l_iclose_144 = open_buf.Get(pos);
								g_bid_324 = Bid;
								g_ask_332 = Ask;
								
								if (!gi_412 && !gi_408) {
									gi_400 = gi_436;
									
									if (l_iclose_136 > l_iclose_144) {
										gd_424 = fGetLots(OP_SELL);
										
										if (gd_424 > 0.0) {
											if (gi_420 < 0) {
												Print("Error: " + GetLastError());
												return;
											}
											
											gd_340 = LastBuyPrice();
											
											gi_448 = true;
										}
									}
									
									else {
										gd_424 = fGetLots(OP_BUY);
										
										if (gd_424 > 0.0) {
											gi_420 = OpenPendingOrder(0, gd_424, g_ask_332, gd_364, g_ask_332, 0, 0, gs_eurusd_392 + "-" + gi_400, g_magic_268);
											
											if (gi_420 < 0) {
												Print("Error: " + GetLastError());
												return;
											}
											
											gd_348 = LastSellPrice();
											
											gi_448 = true;
										}
									}
								}
								
								gi_404 = false;
							}
							
							gi_436 = CountTrades();
							
							g_price_316 = 0;
							ld_152 = 0;
							
							for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
								OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
								
								if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
									continue;
									
								if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
									if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
										g_price_316 += OrderOpenPrice() * OrderLots();
										ld_152 += OrderLots();
									}
								}
							}
							
							if (gi_436 > 0)
								g_price_316 = NormalizeDouble(g_price_316 / ld_152, Digits);
								
							if (gi_448) {
								for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
									OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
									
									if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
										continue;
										
									if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
										if (OrderType() == OP_BUY) {
											g_price_272 = g_price_316 + TakeProfit / 10000.0;
											gd_unused_288 = g_price_272;
											gd_440 = g_price_316 - Stoploss * Point;
											gi_388 = true;
										}
									}
									
									if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
										if (OrderType() == OP_SELL) {
											g_price_272 = g_price_316 - TakeProfit / 10000.0;
											gd_unused_296 = g_price_272;
											gd_440 = g_price_316 + Stoploss * Point;
											gi_388 = true;
										}
									}
								}
							}
							
							if (gi_448) {
								if (gi_388 == true) {
									for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
										OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
										
										if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
											continue;
											
										if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
											if (OrderStopLoss() > 0.0)
												OrderModify(OrderTicket(), g_price_316, OrderStopLoss(), g_price_272);
											else
												OrderModify(OrderTicket(), g_price_316, OrderOpenPrice() - Stoploss / 10000.0, g_price_272);
										}
										
										gi_448 = false;
									}
								}
							}
						}
						
						else
							Alert("Please Activate your copy\nof FXA Trading Group www.forex-ea.org");
					}
					
					
					if (gi_184 == true) {
						gi_404 = false;
						g_magic_268 = 31302;
					}
					
					else
						g_magic_268 = 12378;
						
					if (true) {
						ld_20 = 0;
						ld_28 = 0;
						ld_36 = 0;
						l_str2dbl_44 = 0;
						l_str2dbl_52 = 0;
						l_str2dbl_60 = 0;
						
						if (true) {
							gi_unused_416 = 0;
							
							ld_unused_84 = 0;
							ld_unused_92 = 0;
							li_unused_100 = 0;
							
							if (gi_184 == true) {
								gi_448 = false;
								
								if (true) {
									gd_160 = AccountFreeMargin() / 500.0;
									gd_168 = MarketInfo(Symbol(), MODE_MARGINREQUIRED) * MarketInfo(Symbol(), MODE_MINLOT);
									
									if (AutoLots == true) {
										Lots = 0;
										gd_176 = MathFloor(AccountFreeMargin() / (1000.0 * gd_168)) * MarketInfo(Symbol(), MODE_MINLOT);
										
										if (gd_176 < MarketInfo(Symbol(), MODE_MINLOT))
											Lots = MarketInfo(Symbol(), MODE_MINLOT);
										else
											Lots = gd_176;
									}
									
									if (gd_168 <= gd_160)
										gi_unused_416 = 1;
									else {
										li_160 = 500.0 * gd_168;
										ls_76 = ls_76
												+ "\nFree Margin should be around: " + li_160 + " [atleast]";
									}
								}
							}
							
							li_unused_108 = TakeProfit;
							
							Comment(ls_76);
							
							ld_112 = CalculateProfit();
							
							gi_436 = CountTrades();
							
							if (gi_436 == 0)
								gi_388 = false;
								
							l_ord_lots_120 = 0;
							
							l_ord_lots_128 = 0;
							
							for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
								OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
								
								if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
									continue;
									
								if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
									if (OrderType() == OP_BUY) {
										gi_408 = true;
										gi_412 = false;
										l_ord_lots_120 = OrderLots();
										break;
									}
								}
								
								if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
									if (OrderType() == OP_SELL) {
										gi_408 = false;
										gi_412 = true;
										l_ord_lots_128 = OrderLots();
										break;
									}
								}
							}
							
							if (gi_436 > 0 && gi_436 <= MaxTrades) {
								RefreshRates();
								
								if (gi_436 == 1)
									gd_192 = Dist2;
									
								if (gi_436 > 2)
									gd_192 = Dist1;
									
								if (gi_436 > 4)
									gd_192 = Dist3;
									
								gd_340 = LastBuyPrice();
								
								gd_348 = LastSellPrice();
								
								if (gi_188 != false)
									gd_192 = 2.5 * (Dist1 * gi_436);
									
								if (gi_408 && gd_340 - Ask >= gd_192 / 10000.0)
									gi_404 = true;
									
								if (gi_412 && Bid - gd_348 >= gd_192 / 10000.0)
									gi_404 = true;
							}
							
							if (gi_436 < 1) {
								gi_412 = false;
								gi_408 = false;
								gi_404 = true;
								gd_280 = AccountEquity();
							}
							
							if (gi_404) {
								gd_340 = LastBuyPrice();
								gd_348 = LastSellPrice();
								
								if (gi_412) {
									if (gi_252) {
										fOrderCloseMarket(0, 1);
										gd_424 = NormalizeDouble(gd_260 * l_ord_lots_128, gd_372);
									}
									
									else
										gd_424 = fGetLots(OP_SELL);
										
									if (gi_256) {
										gi_400 = gi_436;
										
										if (gd_424 > 0.0) {
											RefreshRates();
											gi_420 = OpenPendingOrder(1, gd_424, Bid, gd_364, Ask, 0, 0, gs_eurusd_392 + "-" + gi_400, g_magic_268);
											
											if (gi_420 < 0) {
												Print("Error: " + GetLastError());
												return;
											}
											
											gd_348 = LastSellPrice();
											
											gi_404 = false;
											gi_448 = true;
										}
									}
								}
								
								else {
									if (gi_408) {
										if (gi_252) {
											fOrderCloseMarket(1, 0);
											gd_424 = NormalizeDouble(gd_260 * l_ord_lots_120, gd_372);
										}
										
										else
											gd_424 = fGetLots(OP_BUY);
											
										if (gi_256) {
											gi_400 = gi_436;
											
											if (gd_424 > 0.0) {
												if (gi_420 < 0) {
													Print("Error: " + GetLastError());
													return;
												}
												
												gd_340 = LastBuyPrice();
												
												gi_404 = false;
												gi_448 = true;
											}
										}
									}
								}
							}
							
							if (gi_404 && gi_436 < 1) {
								ConstBuffer& open_buf = GetInputBuffer(0, 0);
								l_iclose_136 = open_buf.Get(pos-1);
								l_iclose_144 = open_buf.Get(pos);
								g_bid_324 = Bid;
								g_ask_332 = Ask;
								
								if (!gi_412 && !gi_408) {
									gi_400 = gi_436;
									
									if (l_iclose_136 > l_iclose_144) {
										gd_424 = fGetLots(OP_SELL);
										
										if (gd_424 > 0.0) {
											gi_420 = OpenPendingOrder(1, gd_424, g_bid_324, gd_364, g_bid_324, 0, 0, gs_eurusd_392 + "-" + gi_400, g_magic_268);
											
											if (gi_420 < 0) {
												Print("Error: " + GetLastError());
												return;
											}
											
											gd_340 = LastBuyPrice();
											
											gi_448 = true;
										}
									}
									
									else {
										gd_424 = fGetLots(OP_BUY);
										
										if (gd_424 > 0.0) {
											if (gi_420 < 0) {
												Print("Error: " + GetLastError());
												return;
											}
											
											gd_348 = LastSellPrice();
											
											gi_448 = true;
										}
									}
								}
								
								gi_404 = false;
							}
							
							gi_436 = CountTrades();
							
							g_price_316 = 0;
							ld_152 = 0;
							
							for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
								OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
								
								if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
									continue;
									
								if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
									if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
										g_price_316 += OrderOpenPrice() * OrderLots();
										ld_152 += OrderLots();
									}
								}
							}
							
							if (gi_436 > 0)
								g_price_316 = NormalizeDouble(g_price_316 / ld_152, Digits);
								
							if (gi_448) {
								for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
									OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
									
									if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
										continue;
										
									if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
										if (OrderType() == OP_BUY) {
											g_price_272 = g_price_316 + TakeProfit / 10000.0;
											gd_unused_288 = g_price_272;
											gd_440 = g_price_316 - Stoploss * Point;
											gi_388 = true;
										}
									}
									
									if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
										if (OrderType() == OP_SELL) {
											g_price_272 = g_price_316 - TakeProfit / 10000.0;
											gd_unused_296 = g_price_272;
											gd_440 = g_price_316 + Stoploss * Point;
											gi_388 = true;
										}
									}
								}
							}
							
							if (gi_448) {
								if (gi_388 == true) {
									for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
										OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
										
										if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
											continue;
											
										if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
											if (OrderStopLoss() > 0.0)
												OrderModify(OrderTicket(), g_price_316, OrderStopLoss(), g_price_272);
											else
												OrderModify(OrderTicket(), g_price_316, OrderOpenPrice() + Stoploss / 10000.0, g_price_272);
										}
										
										gi_448 = false;
									}
								}
							}
						}
					}
				}
				
				else {
					if (gi_184 == true) {
						gi_404 = false;
						g_magic_268 = 31302;
					}
					
					else
						g_magic_268 = 12378;
						
					if (true) {
						ld_20 = 0;
						ld_28 = 0;
						ld_36 = 0;
						l_str2dbl_44 = 0;
						l_str2dbl_52 = 0;
						l_str2dbl_60 = 0;
						
						if (IsDemo() == true) {
							ls_68 = AccountNumber();
							l_str2dbl_44 = ls_68;
							l_str2dbl_52 = ls_68;
							l_str2dbl_60 = ls_68;
							ld_28 = AccountNumber() + 739;
							ld_28 = ld_28 + 189.0 * l_str2dbl_44 + 204.0 * l_str2dbl_52 + 118.0 * l_str2dbl_60;
						}
						
						else {
							ls_68 = AccountNumber();
							l_str2dbl_44 = ls_68;
							l_str2dbl_52 = ls_68;
							l_str2dbl_60 = ls_68;
							ld_28 = AccountNumber() + 839;
							ld_28 = ld_28 + 176.0 * l_str2dbl_44 + 298.0 * l_str2dbl_52 + 328.0 * l_str2dbl_60;
						}
						
						if (true) {
							if (gi_184 == true) {
								gi_448 = false;
								
								if (true) {
									gd_160 = AccountFreeMargin() / 500.0;
									gd_168 = MarketInfo(Symbol(), MODE_MARGINREQUIRED) * MarketInfo(Symbol(), MODE_MINLOT);
									
									if (AutoLots == true) {
										Lots = 0;
										gd_176 = MathFloor(AccountFreeMargin() / (500.0 * gd_168)) * MarketInfo(Symbol(), MODE_MINLOT);
										
										if (gd_176 < MarketInfo(Symbol(), MODE_MINLOT))
											Lots = MarketInfo(Symbol(), MODE_MINLOT);
										else
											Lots = gd_176;
									}
									
									if (gd_168 <= gd_160)
										gi_unused_416 = 1;
									else {
										li_164 = 500.0 * gd_168;
										ls_76 = ls_76
												+ "\nFree Margin should be around: " + li_164 + " [atleast]";
									}
								}
								
							}
							
							li_unused_108 = TakeProfit;
							
							
							ld_112 = CalculateProfit();
							
							gi_436 = CountTrades();
							
							if (gi_436 == 0)
								gi_388 = false;
								
							l_ord_lots_120 = 0;
							
							l_ord_lots_128 = 0;
							
							for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
								OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
								
								if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
									continue;
									
								if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
									if (OrderType() == OP_BUY) {
										gi_408 = true;
										gi_412 = false;
										l_ord_lots_120 = OrderLots();
										break;
									}
								}
								
								if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
									if (OrderType() == OP_SELL) {
										gi_408 = false;
										gi_412 = true;
										l_ord_lots_128 = OrderLots();
										break;
									}
								}
							}
							
							if (gi_436 > 0 && gi_436 <= MaxTrades) {
								RefreshRates();
								
								if (gi_436 == 1)
									gd_192 = Dist2;
									
								if (gi_436 > 2)
									gd_192 = Dist1;
									
								if (gi_436 > 4)
									gd_192 = Dist3;
									
								gd_340 = LastBuyPrice();
								
								gd_348 = LastSellPrice();
								
								if (gi_188 != false)
									gd_192 = 2.5 * (Dist1 * gi_436);
									
								if (gi_408 && gd_340 - Ask >= gd_192 / 10000.0)
									gi_404 = true;
									
								if (gi_412 && Bid - gd_348 >= gd_192 / 10000.0)
									gi_404 = true;
							}
							
							if (gi_436 < 1) {
								gi_412 = false;
								gi_408 = false;
								gi_404 = true;
								gd_280 = AccountEquity();
							}
							
							if (gi_404) {
								gd_340 = LastBuyPrice();
								gd_348 = LastSellPrice();
								
								if (gi_412) {
									if (gi_252) {
										fOrderCloseMarket(0, 1);
										gd_424 = NormalizeDouble(gd_260 * l_ord_lots_128, gd_372);
									}
									
									else
										gd_424 = fGetLots(OP_SELL);
										
									if (gi_256) {
										gi_400 = gi_436;
										
										if (gd_424 > 0.0) {
											RefreshRates();
											gi_420 = OpenPendingOrder(1, gd_424, Bid, gd_364, Ask, 0, 0, gs_eurusd_392 + "-" + gi_400, g_magic_268);
											
											if (gi_420 < 0) {
												Print("Error: " + GetLastError());
												return;
											}
											
											gd_348 = LastSellPrice();
											
											gi_404 = false;
											gi_448 = true;
										}
									}
								}
								
								else {
									if (gi_408) {
										if (gi_252) {
											fOrderCloseMarket(1, 0);
											gd_424 = NormalizeDouble(gd_260 * l_ord_lots_120, gd_372);
										}
										
										else
											gd_424 = fGetLots(OP_BUY);
											
										if (gi_256) {
											gi_400 = gi_436;
											
											if (gd_424 > 0.0) {
												gi_420 = OpenPendingOrder(0, gd_424, Ask, gd_364, Bid, 0, 0, gs_eurusd_392 + "-" + gi_400, g_magic_268);
												
												if (gi_420 < 0) {
													Print("Error: " + GetLastError());
													return;
												}
												
												gd_340 = LastBuyPrice();
												
												gi_404 = false;
												gi_448 = true;
											}
										}
									}
								}
							}
							
							if (gi_404 && gi_436 < 1) {
								ConstBuffer& open_buf = GetInputBuffer(0, 0);
								l_iclose_136 = open_buf.Get(pos-1);
								l_iclose_144 = open_buf.Get(pos);
								
								g_bid_324 = Bid;
								g_ask_332 = Ask;
								
								if (!gi_412 && !gi_408) {
									gi_400 = gi_436;
									
									if (l_iclose_136 > l_iclose_144) {
										gd_424 = fGetLots(OP_SELL);
										
										if (gd_424 > 0.0) {
											gi_420 = OpenPendingOrder(1, gd_424, g_bid_324, gd_364, g_bid_324, 0, 0, gs_eurusd_392 + "-" + gi_400, g_magic_268);
											
											if (gi_420 < 0) {
												Print("Error: " + GetLastError());
												return;
											}
											
											gd_340 = LastBuyPrice();
											
											gi_448 = true;
										}
									}
									
									else {
										gd_424 = fGetLots(OP_BUY);
										
										if (gd_424 > 0.0) {
											gi_420 = OpenPendingOrder(0, gd_424, g_ask_332, gd_364, g_ask_332, 0, 0, gs_eurusd_392 + "-" + gi_400, g_magic_268);
											
											if (gi_420 < 0) {
												Print("Error: " + GetLastError());
												return;
											}
											
											gd_348 = LastSellPrice();
											
											gi_448 = true;
										}
									}
								}
								
								gi_404 = false;
							}
							
							gi_436 = CountTrades();
							
							g_price_316 = 0;
							ld_152 = 0;
							
							for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
								OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
								
								if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
									continue;
									
								if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
									if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
										g_price_316 += OrderOpenPrice() * OrderLots();
										ld_152 += OrderLots();
									}
								}
							}
							
							if (gi_436 > 0)
								g_price_316 = NormalizeDouble(g_price_316 / ld_152, Digits);
								
							if (gi_448) {
								for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
									OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
									
									if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
										continue;
										
									if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
										if (OrderType() == OP_BUY) {
											g_price_272 = g_price_316 + TakeProfit / 10000.0;
											gd_unused_288 = g_price_272;
											gd_440 = g_price_316 - Stoploss * Point;
											gi_388 = true;
										}
									}
									
									if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
										if (OrderType() == OP_SELL) {
											g_price_272 = g_price_316 - TakeProfit / 10000.0;
											gd_unused_296 = g_price_272;
											gd_440 = g_price_316 + Stoploss * Point;
											gi_388 = true;
										}
									}
								}
							}
							
							if (gi_448) {
								if (gi_388 == true) {
									for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
										OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
										
										if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
											continue;
											
										if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
											if (OrderStopLoss() > 0.0)
												OrderModify(OrderTicket(), g_price_316, OrderStopLoss(), g_price_272);
											else {
												if (OrderType() == OP_SELL)
													OrderModify(OrderTicket(), g_price_316, OrderOpenPrice() + Stoploss / 10000.0, g_price_272);
												else
													if (OrderType() == OP_BUY)
														OrderModify(OrderTicket(), g_price_316, OrderOpenPrice() - Stoploss / 10000.0, g_price_272);
											}
										}
										
										gi_448 = false;
									}
								}
							}
						}
					}
				}
			}
		}
		
		else
			Alert("License Expired");
	}
	
}

double Lamer::ND(double ad_0) {
	return (NormalizeDouble(ad_0, Digits));
}

int Lamer::fOrderCloseMarket(bool ai_0, bool ai_4) {
	int li_ret_8 = 0;
	
	for (int l_pos_12 = OrdersTotal() - 1; l_pos_12 >= 0; l_pos_12--) {
		if (OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
				if (OrderType() == OP_BUY && ai_0) {
					RefreshRates();
					
					if (!OrderClose(OrderTicket(), OrderLots(), ND(Bid), 5)) {
						Print("Error close BUY " + OrderTicket());
						li_ret_8 = -1;
					}
				}
				
				if (OrderType() == OP_SELL && ai_4) {
					RefreshRates();
					
					if (!OrderClose(OrderTicket(), OrderLots(), ND(Ask), 5)) {
						Print("Error close SELL " + OrderTicket());
						li_ret_8 = -1;
					}
				}
			}
		}
	}
	
	return (li_ret_8);
}

double Lamer::fGetLots(int a_cmd_0) {
	double ld_ret_4;
	Time l_datetime_12(1970,1,1);
	
	switch (gi_248) {
	
	case 0:
		ld_ret_4 = Lots * risk;
		break;
		
	case 1:
		ld_ret_4 = NormalizeDouble(Lots * risk * MathPow(gd_260, CountTrades()), gd_372);
		break;
		
	case 2:
		l_datetime_12 = Time(1970,1,1);
		ld_ret_4 = Lots * risk;
		
		for (int l_pos_20 = OrdersHistoryTotal() - 1; l_pos_20 >= 0; l_pos_20--) {
			if (OrderSelect(l_pos_20, SELECT_BY_POS, MODE_HISTORY)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268) {
					if (l_datetime_12 < OrderCloseTime()) {
						l_datetime_12 = OrderCloseTime();
						
						if (OrderProfit() < 0.0)
							ld_ret_4 = NormalizeDouble(OrderLots() * gd_260, gd_372);
						else
							ld_ret_4 = Lots * risk;
					}
				}
			}
			
			else
				return (-3);
		}
	}
	
	if (AccountFreeMarginCheck(Symbol(), a_cmd_0, ld_ret_4) <= 0.0)
		return (-1);
	
	return (ld_ret_4);
}

int Lamer::CountTrades() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = OrdersTotal() - 1; l_pos_4 >= 0; l_pos_4--) {
		OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268)
			if (OrderType() == OP_SELL || OrderType() == OP_BUY)
				l_count_0++;
	}
	
	return (l_count_0);
}

int Lamer::CountTradesB() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = OrdersTotal() - 1; l_pos_4 >= 0; l_pos_4--) {
		OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == 31301 || OrderMagicNumber() == 31302)
			if (OrderType() == OP_BUY)
				l_count_0++;
	}
	
	return (l_count_0);
}

int Lamer::CountTradesS() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = OrdersTotal() - 1; l_pos_4 >= 0; l_pos_4--) {
		OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == 31301 || OrderMagicNumber() == 31302)
			if (OrderType() == OP_SELL)
				l_count_0++;
	}
	
	return (l_count_0);
}

int Lamer::OpenPendingOrder(int ai_0, double a_lots_4, double ad_unused_12, int a_slippage_20, double ad_unused_24, int ai_32, int ai_36, String a_comment_40, int a_magic_48) {
	int l_ticket_60 = 0;
	int l_error_64 = 0;
	int l_count_68 = 0;
	int li_72 = 100;
	double l_istochastic_76 = At(0).GetBuffer(0).Get(0);
	
	switch (ai_0) {
	
	case 0:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			RefreshRates();
			
			if (l_istochastic_76 < 20.0) {
				if (CountTrades() == 1) {
					g_ienvelopes_232 = At(1).GetBuffer(0).Get(pos);
					
					if (Ask < g_ienvelopes_232) {
						l_ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, StopLong(Bid, ai_32), TakeLong(Ask, ai_36), a_comment_40, a_magic_48);
						break;
					}
				}
				
				else {
					l_ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, StopLong(Bid, ai_32), TakeLong(Ask, ai_36), a_comment_40, a_magic_48);
					break;
				}
				
				if (CountTrades() == 2) {
					g_ienvelopes_232 = At(1).GetBuffer(0).Get(pos);
					
					if (Ask < g_ienvelopes_232) {
						l_ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, StopLong(Bid, ai_32), TakeLong(Ask, ai_36), a_comment_40, a_magic_48);
						break;
					}
				}
				
				else {
					l_ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, StopLong(Bid, ai_32), TakeLong(Ask, ai_36), a_comment_40, a_magic_48);
					break;
				}
				
				if (CountTrades() > 2) {
					g_ienvelopes_232 = At(1).GetBuffer(0).Get(pos);
					
					if (Ask < g_ienvelopes_232) {
						l_ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, StopLong(Bid, ai_32), TakeLong(Ask, ai_36), a_comment_40, a_magic_48);
						break;
					}
				}
				
				else {
					l_ticket_60 = OrderSend(Symbol(), OP_BUY, a_lots_4, Ask, a_slippage_20, StopLong(Bid, ai_32), TakeLong(Ask, ai_36), a_comment_40, a_magic_48);
					break;
				}
			}
			
			Sleep(5000);
		}
		
		break;
		
	case 1:
	
		for (l_count_68 = 0; l_count_68 < li_72; l_count_68++) {
			RefreshRates();
			
			if (l_istochastic_76 > 80.0) {
				if (CountTrades() == 1) {
					g_ienvelopes_224 = At(1).GetBuffer(0).Get(pos);
					
					ConstBuffer& open_buf = GetInputBuffer(0,0);
					ConstBuffer& high_buf = GetInputBuffer(0,2);
					double h0 = high_buf.Get(pos);
					double h1 = high_buf.Get(pos-1);
					double h2 = high_buf.Get(pos-2);
					double h3 = high_buf.Get(pos-2);
					double o0 = open_buf.Get(pos);
					double o1 = open_buf.Get(pos-1);
					double o2 = open_buf.Get(pos-2);
					double o3 = open_buf.Get(pos-3);
					if (Bid > g_ienvelopes_224) {
						if (h0 < h1 && h1 < h2 && h2 < h3 && o0 < o1 && o1 < o2 && o2 < o3) {
							l_ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, StopShort(Ask, ai_32), TakeShort(Bid, ai_36), a_comment_40, a_magic_48);
							break;
						}
					}
				}
				
				else {
					l_ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, StopShort(Ask, ai_32), TakeShort(Bid, ai_36), a_comment_40, a_magic_48);
					break;
				}
				
				if (CountTrades() == 2) {
					g_ienvelopes_224 = At(1).GetBuffer(0).Get(pos);
					
					ConstBuffer& open_buf = GetInputBuffer(0,0);
					ConstBuffer& high_buf = GetInputBuffer(0,2);
					double h0 = high_buf.Get(pos);
					double h1 = high_buf.Get(pos-1);
					double h2 = high_buf.Get(pos-2);
					double h3 = high_buf.Get(pos-2);
					double o0 = open_buf.Get(pos);
					double o1 = open_buf.Get(pos-1);
					double o2 = open_buf.Get(pos-2);
					double o3 = open_buf.Get(pos-3);
					if (Bid > g_ienvelopes_224) {
						if (h0 < h1 && h1 < h2 && h2 < h3 && o0 < o1 && o1 < o2 && o2 < o3) {
							l_ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, StopShort(Ask, ai_32), TakeShort(Bid, ai_36), a_comment_40, a_magic_48);
							break;
						}
					}
				}
				
				else {
					l_ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, StopShort(Ask, ai_32), TakeShort(Bid, ai_36), a_comment_40, a_magic_48);
					break;
				}
				
				if (CountTrades() > 2) {
					g_ienvelopes_224 = At(1).GetBuffer(0).Get(pos);
					
					ConstBuffer& open_buf = GetInputBuffer(0,0);
					ConstBuffer& high_buf = GetInputBuffer(0,2);
					double h0 = high_buf.Get(pos);
					double h1 = high_buf.Get(pos-1);
					double h2 = high_buf.Get(pos-2);
					double h3 = high_buf.Get(pos-2);
					double o0 = open_buf.Get(pos);
					double o1 = open_buf.Get(pos-1);
					double o2 = open_buf.Get(pos-2);
					double o3 = open_buf.Get(pos-3);
					if (Bid > g_ienvelopes_224) {
						if (h0 < h1 && h1 < h2 && h2 < h3 && o0 < o1 && o1 < o2 && o2 < o3) {
							l_ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, StopShort(Ask, ai_32), TakeShort(Bid, ai_36), a_comment_40, a_magic_48);
							break;
						}
					}
				}
				
				else {
					l_ticket_60 = OrderSend(Symbol(), OP_SELL, a_lots_4, Bid, a_slippage_20, StopShort(Ask, ai_32), TakeShort(Bid, ai_36), a_comment_40, a_magic_48);
					break;
				}
			}
			
		}
	}
	
	return (l_ticket_60);
}

double Lamer::StopLong(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 - ai_8 * Point);
}

double Lamer::StopShort(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 + ai_8 * Point);
}

double Lamer::TakeLong(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 + ai_8 * Point);
}

double Lamer::TakeShort(double ad_0, int ai_8) {
	if (ai_8 == 0)
		return (0);
		
	return (ad_0 - ai_8 * Point);
}

double Lamer::CalculateProfit() {
	double ld_ret_0 = 0;
	
	for (g_pos_432 = OrdersTotal() - 1; g_pos_432 >= 0; g_pos_432--) {
		OrderSelect(g_pos_432, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268)
			if (OrderType() == OP_BUY || OrderType() == OP_SELL)
				ld_ret_0 += OrderProfit();
	}
	
	return (ld_ret_0);
}

double Lamer::LastBuyPrice() {
	double l_ord_open_price_0;
	int l_ticket_8;
	double ld_unused_12 = 0;
	int l_ticket_20 = 0;
	
	for (int l_pos_24 = OrdersTotal() - 1; l_pos_24 >= 0; l_pos_24--) {
		OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268 && OrderType() == OP_BUY) {
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

double Lamer::LastSellPrice() {
	double l_ord_open_price_0;
	int l_ticket_8;
	double ld_unused_12 = 0;
	int l_ticket_20 = 0;
	
	for (int l_pos_24 = OrdersTotal() - 1; l_pos_24 >= 0; l_pos_24--) {
		OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != g_magic_268)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == g_magic_268 && OrderType() == OP_SELL) {
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

}
