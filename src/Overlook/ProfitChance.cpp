#include "Overlook.h"

#if 0

// autoprofit

namespace Overlook {

ProfitChance::ProfitChance() {

}

void ProfitChance::InitEA() {
	gi_168 = false;
	gi_172 = false;
	gi_176 = false;
	gi_180 = false;
	double l_lotstep_0 = MarketInfo(Symbol(), MODE_LOTSTEP);
	g_minlot_160 = MarketInfo(Symbol(), MODE_MINLOT);
	gi_156 = 0;
	
	if (l_lotstep_0 == 0.01)
		gi_156 = 2;
	else
		gi_156 = 1;
		
	gi_148 = 2 * Magic;
	
	gi_152 = Magic * 2 + 1;
	
	gd_128 = Ask - Bid;
	
	gi_136 = NormalizeDouble(gd_128 / Point, 0);
	
	if (Digits == 5 || Digits == 3) {
		TakeProfit = 10 * TakeProfit;
		ProtectionTP = 10 * ProtectionTP;
	}
	
	Comment("");
}

void ProfitChance::Start(int pos) {
	double ld_48;
	double ld_56;
	double l_ord_takeprofit_64;
	double l_ord_stoploss_72;
	double l_ord_open_price_80;
	double l_ord_lots_88;
	double ld_96;
	double l_ord_takeprofit_104;
	double l_ord_stoploss_112;
	double l_ord_open_price_120;
	double l_ord_lots_128;
	
	if ((!IsOptimization() && !IsTesting() && !IsVisualMode()) || (ShowTableOnTesting && IsTesting() && !IsOptimization())) {
		DrawStats();
		DrawLogo();
	}
	
	gi_124 = AccountBalance() / 100.0 * gd_116 / (MarketInfo(Symbol(), MODE_TICKVALUE) * LotsOptimized());
	
	if (gi_124 > 5000)
		gi_124 = 5000;
		
	if (AccountBalance() - AccountEquity() > AccountBalance() / 100.0 * gd_116) {
		CloseAllBuy();
		CloseAllSell();
	}
	
	int li_0 = TotalBuy(gi_148);
	
	int li_4 = TotalSell(gi_148);
	int li_8 = BuyStopsTotal(gi_148);
	int li_12 = SellStopsTotal(gi_148);
	int li_16 = BuyLimitsTotal(gi_148);
	int li_20 = SellLimitsTotal(gi_148);
	int li_24 = TotalBuy(gi_152);
	int li_28 = TotalSell(gi_152);
	int li_32 = BuyStopsTotal(gi_152);
	int li_36 = SellStopsTotal(gi_152);
	int li_40 = BuyLimitsTotal(gi_152);
	int li_44 = SellLimitsTotal(gi_152);
	
	if (li_0 == 0 && li_4 == 0 && li_8 == 0 && li_12 == 0 && li_16 == 0 && li_20 == 0 && li_24 == 0 && li_28 == 0 && li_32 == 0 && li_36 == 0 && li_40 == 0 && li_44 == 0) {
		ld_48 = LotsOptimized();
		OpenBuy(ld_48, TakeProfit, gi_148);
		OpenSell(ld_48, TakeProfit, gi_148);
	}
	
	else {
		if (li_0 == 0 && li_4 == 0 && (li_24 == 0 && li_28 == 0)) {
			DeleteAllPending(gi_148);
			DeleteAllPending(gi_152);
		}
		
		else {
			if (li_0 == 0 && li_4 == 0 && (li_24 == 1 && li_28 == 1) && li_8 != 0 || li_12 != 0 || li_16 != 0 || li_20 != 0)
				DeleteAllPending(gi_148);
			else {
				if (li_24 == 0 && li_28 == 0 && (li_0 == 1 && li_4 == 1) && li_32 != 0 || li_36 != 0 || li_40 != 0 || li_44 != 0)
					DeleteAllPending(gi_152);
				else {
					if (li_0 > 0 && li_28 == 0 || li_28 == 1) {
						if (li_8 == 0 && li_20 == 0) {
							GetHighestBuyParameters(l_ord_takeprofit_64, l_ord_stoploss_72, l_ord_open_price_80, l_ord_lots_88, gi_148);
							gi_124 = AccountBalance() / 100.0 * gd_116 / (MarketInfo(Symbol(), MODE_TICKVALUE) * l_ord_lots_88 * LotMultiplicator);
							
							if (gi_124 > 5000)
								gi_124 = 5000;
								
							SetBuyStop(l_ord_open_price_80 + TakeProfit * Point + gd_128, NormalizeDouble(l_ord_lots_88 * LotMultiplicator, gi_156), TakeProfit, gi_124 - gi_136, gi_148);
							
							SetSellLimit(l_ord_open_price_80 + TakeProfit * Point, NormalizeDouble(l_ord_lots_88 * LotMultiplicator, gi_156), 0, gi_124, gi_148);
							
							ld_96 = GetWeightedBELevel(gi_148);
							
							if (ld_96 != -1.0) {
								ld_56 = ld_96 - ProtectionTP * Point;
								TrailBuySl(ld_56, gi_148);
								TrailSellTp(ld_56 + gd_128, gi_148);
							}
						}
						
						if ((li_8 == 0 && li_20 == 0) || (gi_176 != false && gi_180 != false && li_24 == 0 && li_28 == 0)) {
							if (li_4 > 1) {
								GetHighestBuyParameters(l_ord_takeprofit_64, l_ord_stoploss_72, l_ord_open_price_80, l_ord_lots_88, gi_148);
								
								if (li_24 == 0 && li_28 == 0 && li_36 == 0 && li_40 == 0) {
									ld_48 = LotsOptimized();
									SetSellStop(l_ord_open_price_80 - TakeProfit * Point - gd_128, ld_48, TakeProfit, TakeProfit + gi_136 * 2, gi_152);
									SetBuyLimit(l_ord_open_price_80 - TakeProfit * Point, ld_48, TakeProfit, 0, gi_152);
								}
								
								else {
									if (li_24 == 0 && li_28 == 0 && li_36 != 0 && li_40 != 0) {
										DeleteAllPending(gi_152);
										ld_48 = LotsOptimized();
										SetSellStop(l_ord_open_price_80 - TakeProfit * Point - gd_128, ld_48, TakeProfit, TakeProfit + gi_136 * 2, gi_152);
										SetBuyLimit(l_ord_open_price_80 - TakeProfit * Point, ld_48, TakeProfit, 0, gi_152);
									}
								}
							}
						}
					}
					
					li_0 = TotalBuy(gi_148);
					
					li_4 = TotalSell(gi_148);
					li_8 = BuyStopsTotal(gi_148);
					li_12 = SellStopsTotal(gi_148);
					li_16 = BuyLimitsTotal(gi_148);
					li_20 = SellLimitsTotal(gi_148);
					li_24 = TotalBuy(gi_152);
					li_28 = TotalSell(gi_152);
					li_32 = BuyStopsTotal(gi_152);
					li_36 = SellStopsTotal(gi_152);
					li_40 = BuyLimitsTotal(gi_152);
					li_44 = SellLimitsTotal(gi_152);
					
					if (li_4 > 0 && li_24 == 0 || li_24 == 1) {
						if (li_12 == 0 && li_16 == 0) {
							GetLowestSellParameters(l_ord_takeprofit_104, l_ord_stoploss_112, l_ord_open_price_120, l_ord_lots_128, gi_148);
							gi_124 = AccountBalance() / 100.0 * gd_116 / (MarketInfo(Symbol(), MODE_TICKVALUE) * l_ord_lots_128 * LotMultiplicator);
							
							if (gi_124 > 5000)
								gi_124 = 5000;
								
							SetSellStop(l_ord_open_price_120 - TakeProfit * Point - gd_128, NormalizeDouble(l_ord_lots_128 * LotMultiplicator, gi_156), TakeProfit, gi_124 + gi_136, gi_148);
							
							SetBuyLimit(l_ord_open_price_120 - TakeProfit * Point, NormalizeDouble(l_ord_lots_128 * LotMultiplicator, gi_156), 0, gi_124, gi_148);
							
							ld_96 = GetWeightedBELevel(gi_148);
							
							if (ld_96 != -1.0) {
								ld_56 = ld_96 + ProtectionTP * Point;
								TrailBuyTp(ld_56, gi_148);
								TrailSellSl(ld_56 + gd_128, gi_148);
							}
						}
						
						if ((li_12 == 0 && li_16 == 0) || (gi_176 != false && gi_180 != false && li_24 == 0 && li_28 == 0)) {
							if (li_0 > 1) {
								GetLowestSellParameters(l_ord_takeprofit_104, l_ord_stoploss_112, l_ord_open_price_120, l_ord_lots_128, gi_148);
								
								if (li_24 == 0 && li_28 == 0 && li_32 == 0 && li_44 == 0) {
									ld_48 = LotsOptimized();
									SetBuyStop(l_ord_open_price_120 + TakeProfit * Point + gd_128, ld_48, TakeProfit - gi_136, TakeProfit + gi_136 * 2, gi_152);
									SetSellLimit(l_ord_open_price_120 + TakeProfit * Point, ld_48, TakeProfit, 0, gi_152);
								}
								
								else {
									if (li_24 == 0 && li_28 == 0 && li_32 != 0 && li_44 != 0) {
										DeleteAllPending(gi_152);
										ld_48 = LotsOptimized();
										SetBuyStop(l_ord_open_price_120 + TakeProfit * Point + gd_128, ld_48, TakeProfit - gi_136, TakeProfit + gi_136 * 2, gi_152);
										SetSellLimit(l_ord_open_price_120 + TakeProfit * Point, ld_48, TakeProfit, 0, gi_152);
									}
								}
							}
						}
					}
					
					li_0 = TotalBuy(gi_148);
					
					li_4 = TotalSell(gi_148);
					li_8 = BuyStopsTotal(gi_148);
					li_12 = SellStopsTotal(gi_148);
					li_16 = BuyLimitsTotal(gi_148);
					li_20 = SellLimitsTotal(gi_148);
					li_24 = TotalBuy(gi_152);
					li_28 = TotalSell(gi_152);
					li_32 = BuyStopsTotal(gi_152);
					li_36 = SellStopsTotal(gi_152);
					li_40 = BuyLimitsTotal(gi_152);
					li_44 = SellLimitsTotal(gi_152);
					
					if (li_24 > 0 && li_4 == 0 || li_4 == 1) {
						if (li_32 == 0 && li_44 == 0) {
							GetHighestBuyParameters(l_ord_takeprofit_64, l_ord_stoploss_72, l_ord_open_price_80, l_ord_lots_88, gi_152);
							gi_124 = AccountBalance() / 100.0 * gd_116 / (MarketInfo(Symbol(), MODE_TICKVALUE) * l_ord_lots_88 * LotMultiplicator);
							
							if (gi_124 > 5000)
								gi_124 = 5000;
								
							SetBuyStop(l_ord_open_price_80 + TakeProfit * Point + gd_128, NormalizeDouble(l_ord_lots_88 * LotMultiplicator, gi_156), TakeProfit, gi_124 - gi_136, gi_152);
							
							SetSellLimit(l_ord_open_price_80 + TakeProfit * Point, NormalizeDouble(l_ord_lots_88 * LotMultiplicator, gi_156), 0, gi_124, gi_152);
							
							ld_96 = GetWeightedBELevel(gi_152);
							
							if (ld_96 != -1.0) {
								ld_56 = ld_96 - ProtectionTP * Point;
								TrailBuySl(ld_56, gi_152);
								TrailSellTp(ld_56 + gd_128, gi_152);
							}
						}
						
						if ((li_32 == 0 && li_44 == 0) || (gi_168 != false && gi_172 != false && li_0 == 0 && li_4 == 0)) {
							if (li_28 > 1) {
								GetHighestBuyParameters(l_ord_takeprofit_64, l_ord_stoploss_72, l_ord_open_price_80, l_ord_lots_88, gi_152);
								
								if (li_0 == 0 && li_4 == 0 && li_12 == 0 && li_16 == 0) {
									ld_48 = LotsOptimized();
									SetSellStop(l_ord_open_price_80 - TakeProfit * Point - gd_128, ld_48, TakeProfit, TakeProfit + gi_136 * 2, gi_148);
									SetBuyLimit(l_ord_open_price_80 - TakeProfit * Point, ld_48, TakeProfit, 0, gi_148);
								}
								
								else {
									if (li_0 == 0 && li_4 == 0 && li_12 != 0 && li_16 != 0) {
										DeleteAllPending(gi_148);
										ld_48 = LotsOptimized();
										SetSellStop(l_ord_open_price_80 - TakeProfit * Point - gd_128, ld_48, TakeProfit, TakeProfit + gi_136 * 2, gi_148);
										SetBuyLimit(l_ord_open_price_80 - TakeProfit * Point, ld_48, TakeProfit, 0, gi_148);
									}
								}
							}
						}
					}
					
					li_0 = TotalBuy(gi_148);
					
					li_4 = TotalSell(gi_148);
					li_8 = BuyStopsTotal(gi_148);
					li_12 = SellStopsTotal(gi_148);
					li_16 = BuyLimitsTotal(gi_148);
					li_20 = SellLimitsTotal(gi_148);
					li_24 = TotalBuy(gi_152);
					li_28 = TotalSell(gi_152);
					li_32 = BuyStopsTotal(gi_152);
					li_36 = SellStopsTotal(gi_152);
					li_40 = BuyLimitsTotal(gi_152);
					li_44 = SellLimitsTotal(gi_152);
					
					if (li_28 > 0 && li_0 == 0 || li_0 == 1) {
						if (li_36 == 0 && li_40 == 0) {
							GetLowestSellParameters(l_ord_takeprofit_104, l_ord_stoploss_112, l_ord_open_price_120, l_ord_lots_128, gi_152);
							gi_124 = AccountBalance() / 100.0 * gd_116 / (MarketInfo(Symbol(), MODE_TICKVALUE) * l_ord_lots_128 * LotMultiplicator);
							
							if (gi_124 > 5000)
								gi_124 = 5000;
								
							SetSellStop(l_ord_open_price_120 - TakeProfit * Point - gd_128, NormalizeDouble(l_ord_lots_128 * LotMultiplicator, gi_156), TakeProfit, gi_124 + gi_136, gi_152);
							
							SetBuyLimit(l_ord_open_price_120 - TakeProfit * Point, NormalizeDouble(l_ord_lots_128 * LotMultiplicator, gi_156), 0, gi_124, gi_152);
							
							ld_96 = GetWeightedBELevel(gi_152);
							
							if (ld_96 != -1.0) {
								ld_56 = ld_96 + ProtectionTP * Point;
								TrailBuyTp(ld_56, gi_152);
								TrailSellSl(ld_56 + gd_128, gi_152);
							}
						}
						
						if ((li_36 == 0 && li_40 == 0) || (gi_168 != false && gi_172 != false && li_0 == 0 && li_4 == 0)) {
							if (li_24 > 1) {
								GetLowestSellParameters(l_ord_takeprofit_104, l_ord_stoploss_112, l_ord_open_price_120, l_ord_lots_128, gi_152);
								
								if (li_0 == 0 && li_4 == 0 && li_8 == 0 && li_20 == 0) {
									ld_48 = LotsOptimized();
									SetBuyStop(l_ord_open_price_120 + TakeProfit * Point + gd_128, ld_48, TakeProfit - gi_136, TakeProfit + gi_136 * 2, gi_148);
									SetSellLimit(l_ord_open_price_120 + TakeProfit * Point, ld_48, TakeProfit, 0, gi_148);
								}
								
								else {
									if (li_0 == 0 && li_4 == 0 && li_8 != 0 && li_20 != 0) {
										DeleteAllPending(gi_148);
										ld_48 = LotsOptimized();
										SetBuyStop(l_ord_open_price_120 + TakeProfit * Point + gd_128, ld_48, TakeProfit - gi_136, TakeProfit + gi_136 * 2, gi_148);
										SetSellLimit(l_ord_open_price_120 + TakeProfit * Point, ld_48, TakeProfit, 0, gi_148);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	gi_168 = li_0;
	
	gi_172 = li_4;
	gi_176 = li_24;
	gi_180 = li_28;
	return (0);
}

int ProfitChance::TotalBuy(int a_magic_0) {
	int l_count_4 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		if (!(OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_BUY)
			l_count_4++;
	}
	
	return (l_count_4);
}

int ProfitChance::TotalSell(int a_magic_0) {
	int l_count_4 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		if (!(OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_SELL)
			l_count_4++;
	}
	
	return (l_count_4);
}

int ProfitChance::BuyStopsTotal(int a_magic_0) {
	int l_count_4 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		if (!(OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_BUYSTOP)
			l_count_4++;
	}
	
	return (l_count_4);
}

int ProfitChance::SellStopsTotal(int a_magic_0) {
	int l_count_4 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		if (!(OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_SELLSTOP)
			l_count_4++;
	}
	
	return (l_count_4);
}

int ProfitChance::BuyLimitsTotal(int a_magic_0) {
	int l_count_4 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		if (!(OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_BUYLIMIT)
			l_count_4++;
	}
	
	return (l_count_4);
}

int ProfitChance::SellLimitsTotal(int a_magic_0) {
	int l_count_4 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersTotal(); l_pos_8++) {
		if (!(OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_SELLLIMIT)
			l_count_4++;
	}
	
	return (l_count_4);
}

void ProfitChance::DeleteAllPending(int a_magic_0) {
	for (int l_pos_4 = OrdersTotal() - 1; l_pos_4 >= 0; l_pos_4--) {
		if (!(OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_SELLSTOP || OrderType() == OP_BUYSTOP || OrderType() == OP_BUYLIMIT || OrderType() == OP_SELLLIMIT) {
			while (IsTradeContextBusy())
				Sleep(500);
				
			Print("Óäàëÿåì îðäåð #" + OrderTicket() + ".");
			
			if (!OrderDelete(OrderTicket()))
				Print("Íå óäàëîñü óäàëèòü îðäåð #" + OrderTicket() + ". Îøèáêà: ");
		}
	}
}

void ProfitChance::SetSellStop(double a_price_0, double a_lots_8, int ai_16, int ai_20, int a_magic_24) {
	double l_price_28;
	double l_price_36;
	int l_error_48;
	
	if (ai_20 != 0)
		l_price_28 = a_price_0 + ai_20 * Point;
		
	if (ai_16 != 0)
		l_price_36 = a_price_0 - ai_16 * Point;
		
	int l_ticket_44 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_8, a_price_0, Slippage, l_price_28, l_price_36, "", a_magic_24, 0, Red);
	
	if (l_ticket_44 < 1) {
		l_error_48 = GetLastError();
		Print("Íå óäàëîñü óñòàíîâèòü îòëîæåííûé îðäåð SELLSTOP îáúåìîì ");
		Print(Bid + " " + a_price_0 + " " + l_price_28 + " " + l_price_36);
	}
}

void ProfitChance::SetBuyStop(double a_price_0, double a_lots_8, int ai_16, int ai_20, int a_magic_24) {
	double l_price_28;
	double l_price_36;
	int l_error_48;
	
	if (ai_20 != 0)
		l_price_28 = a_price_0 - ai_20 * Point;
		
	if (ai_16 != 0)
		l_price_36 = a_price_0 + ai_16 * Point;
		
	int l_ticket_44 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_8, a_price_0, Slippage, l_price_28, l_price_36, "", a_magic_24, 0, Blue);
	
	if (l_ticket_44 < 1) {
		l_error_48 = GetLastError();
		Print("Íå óäàëîñü óñòàíîâèòü îòëîæåííûé îðäåð BUYSTOP îáúåìîì " + a_lots_8 + ". Îøèáêà #");
		Print(Bid + " " + a_price_0 + " " + l_price_28 + " " + l_price_36);
	}
}

void ProfitChance::SetSellLimit(double a_price_0, double a_lots_8, int ai_16, int ai_20, int a_magic_24) {
	double l_price_36;
	int l_error_48;
	double l_price_28 = 0;
	
	if (ai_20 != 0)
		l_price_28 = a_price_0 + ai_20 * Point;
		
	if (ai_16 != 0)
		l_price_36 = a_price_0 - ai_16 * Point;
		
	int l_ticket_44 = OrderSend(Symbol(), OP_SELLLIMIT, a_lots_8, a_price_0, Slippage, l_price_28, l_price_36, "", a_magic_24, 0, CLR_NONE);
	
	if (l_ticket_44 < 1) {
		l_error_48 = GetLastError();
		Print("Íå óäàëîñü óñòàíîâèòü îòëîæåííûé îðäåð SELLLIMIT îáúåìîì " + a_lots_8 + ". Îøèáêà #");
		Print(Bid + " " + a_price_0 + " " + l_price_28 + " " + l_price_36);
	}
}

void ProfitChance::SetBuyLimit(double a_price_0, double a_lots_8, int ai_16, int ai_20, int a_magic_24) {
	double l_price_36;
	int l_error_48;
	double l_price_28 = 0;
	
	if (ai_20 != 0)
		l_price_28 = a_price_0 - ai_20 * Point;
		
	if (ai_16 != 0)
		l_price_36 = a_price_0 + ai_16 * Point;
		
	int l_ticket_44 = OrderSend(Symbol(), OP_BUYLIMIT, a_lots_8, a_price_0, Slippage, l_price_28, l_price_36, "", a_magic_24, 0, CLR_NONE);
	
	if (l_ticket_44 < 1) {
		l_error_48 = GetLastError();
		Print("Íå óäàëîñü óñòàíîâèòü îòëîæåííûé îðäåð BUYLIMIT îáúåìîì " + a_lots_8 + ". Îøèáêà #");
		Print(Bid + " " + a_price_0 + " " + l_price_28 + " " + l_price_36);
	}
}

void ProfitChance::OpenBuy(double a_lots_0, int ai_8, int a_magic_12) {
	int l_error_56;
	int l_ord_total_20 = OrdersTotal();
	double l_price_32 = 0;
	double l_price_40 = 0;
	RefreshRates();
	double l_ask_24 = Ask;
	
	if (ai_8 != 0)
		l_price_32 = l_ask_24 + ai_8 * Point;
		
	if (gi_124 != 0)
		l_price_40 = l_ask_24 - gi_124 * Point;
		
	int l_datetime_48 = TimeCurrent();
	
	int l_ticket_52 = OrderSend(Symbol(), OP_BUY, a_lots_0, l_ask_24, Slippage, l_price_40, l_price_32, "", a_magic_12, 0, Blue);
	
	if (l_ticket_52 == -1) {
		while (l_ticket_52 == -1 && TimeCurrent() - l_datetime_48 < 40 && !IsTesting()) {
			l_error_56 = GetLastError();
			Print("Îøèáêà îòêðûòèÿ îðäåðà BUY. ");
			Sleep(100);
			Print("Ïîâòîð");
			RefreshRates();
			l_ask_24 = Ask;
			
			if (ai_8 != 0)
				l_price_32 = l_ask_24 + ai_8 * Point;
				
			if (gi_124 != 0)
				l_price_40 = l_ask_24 - gi_124 * Point;
				
			l_ticket_52 = OrderSend(Symbol(), OP_BUY, a_lots_0, l_ask_24, Slippage, l_price_40, l_price_32, "", a_magic_12, 0, Blue);
		}
		
		if (l_ticket_52 == -1) {
			l_error_56 = GetLastError();
			Print("Îøèáêà îòêðûòèÿ îðäåðà BUY. ");
		}
	}
	
	if (l_ticket_52 != -1)
		Print("Îòêðûòî BUY íà " + Symbol() + " " + Period() + " îáúåìîì " + a_lots_0 + "!");
}

void ProfitChance::OpenSell(double a_lots_0, int ai_8, int a_magic_12) {
	int l_error_56;
	int l_ord_total_20 = OrdersTotal();
	double l_price_32 = 0;
	double l_price_40 = 0;
	RefreshRates();
	double l_bid_24 = Bid;
	
	if (ai_8 != 0)
		l_price_32 = l_bid_24 - ai_8 * Point;
		
	if (gi_124 != 0)
		l_price_40 = l_bid_24 + gi_124 * Point;
		
	int l_datetime_48 = TimeCurrent();
	
	int l_ticket_52 = OrderSend(Symbol(), OP_SELL, a_lots_0, l_bid_24, Slippage, l_price_40, l_price_32, "", a_magic_12, 0, Red);
	
	if (l_ticket_52 == -1) {
		while (l_ticket_52 == -1 && TimeCurrent() - l_datetime_48 < 40 && !IsTesting()) {
			l_error_56 = GetLastError();
			Print("Îøèáêà îòêðûòèÿ îðäåðà SELL. ");
			Sleep(100);
			Print("Ïîâòîð");
			RefreshRates();
			l_bid_24 = Bid;
			
			if (ai_8 != 0)
				l_price_32 = l_bid_24 - ai_8 * Point;
				
			if (gi_124 != 0)
				l_price_40 = l_bid_24 + gi_124 * Point;
				
			l_ticket_52 = OrderSend(Symbol(), OP_SELL, a_lots_0, l_bid_24, Slippage, l_price_40, l_price_32, "", a_magic_12, 0, Red);
		}
		
		if (l_ticket_52 == -1) {
			l_error_56 = GetLastError();
			Print("Îøèáêà îòêðûòèÿ îðäåðà SELL. ");
		}
	}
	
	if (l_ticket_52 != -1)
		Print("Îòêðûòî SELL íà " + Symbol() + " " + Period() + " îáúåìîì " + a_lots_0 + "!");
}

void ProfitChance::TrailBuyTp(double a_price_0, int a_magic_8) {
	for (int l_pos_12 = 0; l_pos_12 < OrdersTotal(); l_pos_12++) {
		if (!(OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_8 && OrderType() == OP_BUY) {
			if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), a_price_0, OrderExpiration(), HotPink)) {
				Print("Íå óäàëîñü ìîäèôèöèðîâàòü îðäåð #" + OrderTicket() + ". Îøèáêà:");
				Print("SL=" + OrderStopLoss() + " TP=" + a_price_0);
			}
		}
	}
}

void ProfitChance::TrailBuySl(double a_price_0, int a_magic_8) {
	for (int l_pos_12 = 0; l_pos_12 < OrdersTotal(); l_pos_12++) {
		if (!(OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_8 && OrderType() == OP_BUY) {
			if (!OrderModify(OrderTicket(), OrderOpenPrice(), a_price_0, OrderTakeProfit(), OrderExpiration(), HotPink)) {
				Print("Íå óäàëîñü ìîäèôèöèðîâàòü îðäåð #" + OrderTicket() + ". Îøèáêà:");
				Print("SL=" + a_price_0 + " TP=" + OrderTakeProfit());
			}
		}
	}
}

void ProfitChance::TrailSellTp(double a_price_0, int a_magic_8) {
	for (int l_pos_12 = 0; l_pos_12 < OrdersTotal(); l_pos_12++) {
		if (!(OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_8 && OrderType() == OP_SELL) {
			if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), a_price_0, OrderExpiration(), HotPink)) {
				Print("Íå óäàëîñü ìîäèôèöèðîâàòü îðäåð #" + OrderTicket() + ". Îøèáêà:");
				Print("SL=" + OrderStopLoss() + " TP=" + a_price_0);
			}
		}
	}
}

void ProfitChance::TrailSellSl(double a_price_0, int a_magic_8) {
	for (int l_pos_12 = 0; l_pos_12 < OrdersTotal(); l_pos_12++) {
		if (!(OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_8 && OrderType() == OP_SELL) {
			if (!OrderModify(OrderTicket(), OrderOpenPrice(), a_price_0, OrderTakeProfit(), OrderExpiration(), HotPink)) {
				Print("Íå óäàëîñü ìîäèôèöèðîâàòü îðäåð #" + OrderTicket() + ". Îøèáêà:");
				Print("SL=" + a_price_0 + " TP=" + OrderTakeProfit());
			}
		}
	}
}

double ProfitChance::GetWeightedBELevel(int ai_0) {
	double ld_12 = NormalizeDouble(GetBuyLotsSum(ai_0), 2);
	double ld_28 = NormalizeDouble(GetSellLotsSum(ai_0), 2);
	double ld_36 = ld_12 - ld_28;
	double ld_44 = -1;
	
	if (ld_36 == 0.0)
		ld_44 = -1;
	else
		ld_44 = GetWeightedPrice(ai_0) / ld_36;
		
	return (NormalizeDouble(ld_44, Digits));
}

double ProfitChance::GetWeightedPrice(int a_magic_0) {
	double ld_ret_8;
	
	for (int l_pos_4 = 0; l_pos_4 <= OrdersTotal(); l_pos_4++) {
		OrderSelect(l_pos_4, SELECT_BY_POS);
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_SELL)
			ld_ret_8 -= OrderLots() * OrderOpenPrice();
		else
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0 && OrderType() == OP_BUY)
				ld_ret_8 += OrderLots() * OrderOpenPrice();
	}
	
	return (ld_ret_8);
}

double ProfitChance::GetSellLotsSum(int a_magic_0) {
	int l_ord_total_4 = OrdersTotal();
	double ld_ret_12 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < l_ord_total_4; l_pos_8++) {
		OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() == OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0)
			ld_ret_12 += OrderLots();
	}
	
	return (ld_ret_12);
}

double ProfitChance::GetBuyLotsSum(int a_magic_0) {
	int l_ord_total_4 = OrdersTotal();
	double ld_ret_12 = 0;
	
	for (int l_pos_8 = 0; l_pos_8 < l_ord_total_4; l_pos_8++) {
		OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() == OP_BUY && OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_0)
			ld_ret_12 += OrderLots();
	}
	
	return (ld_ret_12);
}

void ProfitChance::GetHighestBuyParameters(double &a_ord_takeprofit_0, double &a_ord_stoploss_8, double &a_ord_open_price_16, double &a_ord_lots_24, int a_magic_32) {
	a_ord_open_price_16 = 0;
	
	for (int l_pos_36 = 0; l_pos_36 < OrdersTotal(); l_pos_36++) {
		if (!(OrderSelect(l_pos_36, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_32 && OrderType() == OP_BUY && OrderOpenPrice() > a_ord_open_price_16) {
			a_ord_open_price_16 = OrderOpenPrice();
			a_ord_takeprofit_0 = OrderTakeProfit();
			a_ord_stoploss_8 = OrderStopLoss();
			a_ord_lots_24 = OrderLots();
		}
	}
}

void ProfitChance::GetLowestSellParameters(double &a_ord_takeprofit_0, double &a_ord_stoploss_8, double &a_ord_open_price_16, double &a_ord_lots_24, int a_magic_32) {
	a_ord_open_price_16 = 1410065407;
	
	for (int l_pos_36 = 0; l_pos_36 < OrdersTotal(); l_pos_36++) {
		if (!(OrderSelect(l_pos_36, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == a_magic_32 && OrderType() == OP_SELL && OrderOpenPrice() < a_ord_open_price_16) {
			a_ord_open_price_16 = OrderOpenPrice();
			a_ord_takeprofit_0 = OrderTakeProfit();
			a_ord_stoploss_8 = OrderStopLoss();
			a_ord_lots_24 = OrderLots();
		}
	}
}

double ProfitChance::GetProfitForDay(int ai_0) {
	double ld_ret_4 = 0;
	
	for (int l_pos_12 = 0; l_pos_12 < OrdersHistoryTotal(); l_pos_12++) {
		if (!(OrderSelect(l_pos_12, SELECT_BY_POS, MODE_HISTORY)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == gi_148 || OrderMagicNumber() == gi_152)
			if (OrderCloseTime() >= iTime(Symbol(), PERIOD_D1, ai_0) && OrderCloseTime() < iTime(Symbol(), PERIOD_D1, ai_0) + 86400)
				ld_ret_4 += OrderProfit();
	}
	
	return (ld_ret_4);
}

double ProfitChance::LotsOptimized() {
	double l_minlot_0 = NormalizeDouble(MathFloor(AccountBalance() / 100.0 * gd_116 * RiskPercent / 100.0) / 10000.0, gi_156);
	
	if (l_minlot_0 < g_minlot_160)
		l_minlot_0 = g_minlot_160;
		
	return (l_minlot_0);
}

void ProfitChance::CloseAllSell() {
	int l_datetime_12;
	int l_error_16;
	int l_ord_total_4 = OrdersTotal();
	
	if (l_ord_total_4 > 0) {
		for (int l_pos_0 = l_ord_total_4 - 1; l_pos_0 >= 0; l_pos_0--) {
			if (OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() == Symbol() && OrderType() == OP_SELL && OrderMagicNumber() == gi_148 || OrderMagicNumber() == gi_152) {
					while (IsTradeContextBusy())
						Sleep(1000);
						
					l_datetime_12 = TimeCurrent();
					
					RefreshRates();
					
					for (int l_ord_close_8 = OrderClose(OrderTicket(), OrderLots(), Ask, Slippage, Yellow); !l_ord_close_8 && TimeCurrent() - l_datetime_12 <= 120 && !IsTesting(); l_ord_close_8 = OrderClose(OrderTicket(), OrderLots(), Ask, Slippage, Yellow)) {
						if (!l_ord_close_8) {
							l_error_16 = GetLastError();
							Print("Îøèáêà çàêðûòèÿ îðäåðà SELL #" + OrderTicket() + " ");
						}
						
						Sleep(1000);
						
						while (IsTradeContextBusy())
							Sleep(500);
							
						RefreshRates();
					}
					
					if (!l_ord_close_8) {
						l_error_16 = GetLastError();
						Print("Îøèáêà çàêðûòèÿ îðäåðà SELL #" + OrderTicket() + " ");
					}
				}
			}
			
			else
				Print("Íå óäàëîñü âûáðàòü îòêðûòûé îðäåð:");
		}
	}
}

void ProfitChance::CloseAllBuy() {
	int l_datetime_12;
	int l_error_16;
	int l_ord_total_4 = OrdersTotal();
	
	if (l_ord_total_4 > 0) {
		for (int l_pos_0 = l_ord_total_4 - 1; l_pos_0 >= 0; l_pos_0--) {
			if (OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() == Symbol() && OrderType() == OP_BUY && OrderMagicNumber() == gi_148 || OrderMagicNumber() == gi_152) {
					while (IsTradeContextBusy())
						Sleep(1000);
						
					l_datetime_12 = TimeCurrent();
					
					RefreshRates();
					
					for (int l_ord_close_8 = OrderClose(OrderTicket(), OrderLots(), Bid, Slippage, Yellow); !l_ord_close_8 && TimeCurrent() - l_datetime_12 <= 120 && !IsTesting(); l_ord_close_8 = OrderClose(OrderTicket(), OrderLots(), Bid, Slippage, Yellow)) {
						if (!l_ord_close_8) {
							l_error_16 = GetLastError();
							Print("Îøèáêà çàêðûòèÿ îðäåðà BUY #" + OrderTicket() + " ");
						}
						
						Sleep(1000);
						
						while (IsTradeContextBusy())
							Sleep(1000);
							
						RefreshRates();
					}
					
					if (!l_ord_close_8) {
						l_error_16 = GetLastError();
						Print("Îøèáêà çàêðûòèÿ îðäåðà BUY #" + OrderTicket() + " ");
					}
				}
			}
			
			else
				Print("Íå óäàëîñü âûáðàòü îòêðûòûé îðäåð:");
		}
	}
}

}

#endif
