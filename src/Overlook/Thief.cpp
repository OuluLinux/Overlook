#include "Overlook.h"


// perevorot

namespace Overlook {

Thief::Thief() {
	
}

void Thief::InitEA() {
	gi_unused_152 = 0;
	gi_unused_156 = 0;
	gi_unused_160 = 0;
	gi_unused_164 = 0;
	g_count_168 = 0;
	gd_144 = Ask - Bid;
	Comment("");
}

void Thief::StartEA(int pos) {
	double l_ord_open_price_0;
	double l_ord_takeprofit_8;
	double l_ord_stoploss_16;
	double l_ord_lots_24;
	double l_ord_lots_32;
	double l_ord_open_price_40;
	double l_ord_takeprofit_48;
	double l_ord_stoploss_56;
	double l_ord_lots_64;
	double l_ord_open_price_72;
	double l_ord_takeprofit_80;
	double l_ord_stoploss_88;
	double l_ord_lots_96;
	double ld_104;
	int li_112;
	DoTrail();
	
	int li_116 = TotalBuy();
	int li_120 = TotalSell();
	int li_124 = BuyStopsTotal();
	int li_128 = SellStopsTotal();
	g_lots_172 = LotsOptimized();
	
	if (li_116 == 0 && li_120 == 0) {
		if (li_124 != 0 || li_128 != 0) {
			DeleteAllBuyStops();
			DeleteAllSellStops();
		}
		
		if (Type == 0)
			OpenBuy();
		else
			if (Type == 1)
				OpenSell();
				
		g_count_168 = 1;
	}
	
	else {
		if (li_116 != 0) {
			GetBuyParameters(l_ord_open_price_0, l_ord_takeprofit_8, l_ord_stoploss_16, l_ord_lots_24);
			
			if (li_124 == 0)
				SetBuyStop(l_ord_takeprofit_8 + gd_144, TakeProfit, StopLoss, g_lots_172);
				
			if (li_128 == 0) {
				ld_104 = l_ord_lots_24;
				
				if (g_count_168 >= OrderToIncLots) {
					ld_104 = l_ord_lots_24 * LotIncrement;
					g_count_168 = 1;
				}
				
				SetSellStop(l_ord_stoploss_16, TakeProfit, StopLoss, ld_104);
			}
			
			else {
				if (l_ord_stoploss_16 > l_ord_open_price_0) {
					GetSellStopParameters(l_ord_open_price_40, l_ord_takeprofit_48, l_ord_stoploss_56, l_ord_lots_64);
					
					if (NormalizeDouble(l_ord_lots_64, 2) != NormalizeDouble(g_lots_172, 2)) {
						DeleteAllSellStops();
						SetSellStop(l_ord_stoploss_16, TakeProfit, StopLoss, g_lots_172);
					}
					
					else
						if (ND(l_ord_open_price_40) != ND(l_ord_stoploss_16))
							TrailSellStop();
				}
				
				else {
					GetSellStopParameters(l_ord_open_price_40, l_ord_takeprofit_48, l_ord_stoploss_56, l_ord_lots_64);
					
					if (l_ord_open_price_40 != l_ord_stoploss_16) {
						li_112 = GetLastClosedOrderType();
						
						if (li_112 == 1)
							g_count_168++;
						else
							g_count_168 = 1;
							
						DeleteAllSellStops();
					}
				}
			}
		}
		
		else {
			if (li_120 != 0) {
				GetSellParameters(l_ord_open_price_0, l_ord_takeprofit_8, l_ord_stoploss_16, l_ord_lots_32);
				
				if (li_128 == 0)
					SetSellStop(l_ord_takeprofit_8 - gd_144, TakeProfit, StopLoss, g_lots_172);
					
				if (li_124 == 0) {
					ld_104 = l_ord_lots_32;
					
					if (g_count_168 >= OrderToIncLots) {
						ld_104 = l_ord_lots_32 * LotIncrement;
						g_count_168 = 1;
					}
					
					SetBuyStop(l_ord_stoploss_16, TakeProfit, StopLoss, ld_104);
				}
				
				else {
					if (l_ord_stoploss_16 < l_ord_open_price_0) {
						GetBuyStopParameters(l_ord_open_price_72, l_ord_takeprofit_80, l_ord_stoploss_88, l_ord_lots_96);
						
						if (NormalizeDouble(l_ord_lots_96, 2) != NormalizeDouble(g_lots_172, 2)) {
							DeleteAllBuyStops();
							SetBuyStop(l_ord_stoploss_16, TakeProfit, StopLoss, g_lots_172);
						}
						
						else
							if (ND(l_ord_open_price_72) != ND(l_ord_stoploss_16))
								TrailBuyStop();
					}
					
					else {
						GetBuyStopParameters(l_ord_open_price_72, l_ord_takeprofit_80, l_ord_stoploss_88, l_ord_lots_96);
						
						if (l_ord_open_price_72 != l_ord_stoploss_16) {
							li_112 = GetLastClosedOrderType();
							
							if (li_112 == 0)
								g_count_168++;
							else
								g_count_168 = 1;
								
							DeleteAllBuyStops();
						}
					}
				}
			}
		}
	}
	
	gi_unused_152 = li_116;
	
	gi_unused_156 = li_120;
	gi_unused_160 = li_124;
	gi_unused_164 = li_128;
	
}

int Thief::TotalBuy() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (!(OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderType() == OP_BUY && OrderMagicNumber() == Magic)
			l_count_0++;
	}
	
	return (l_count_0);
}

int Thief::TotalSell() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (!(OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderType() == OP_SELL && OrderMagicNumber() == Magic)
			l_count_0++;
	}
	
	return (l_count_0);
}

int Thief::BuyStopsTotal() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (!(OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_BUYSTOP)
			l_count_0++;
	}
	
	return (l_count_0);
}

int Thief::SellStopsTotal() {
	int l_count_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (!(OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_SELLSTOP)
			l_count_0++;
	}
	
	return (l_count_0);
}

void Thief::SetSellStop(double a_price_0, int ai_8, int ai_12, double a_lots_16) {
	double l_price_28 = 0;
	
	if (ai_12 != 0)
		l_price_28 = a_price_0 + ai_12 * Point;
		
	double l_price_36 = a_price_0 - ai_8 * Point;
	
	int l_ticket_44 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_16, a_price_0, g_slippage_140, l_price_28, l_price_36, "", Magic, 0, Red);
	
	if (l_ticket_44 < 1) {
		
	}
}

void Thief::SetBuyStop(double a_price_0, int ai_8, double a_pips_12, double a_lots_20) {
	double l_price_32 = 0;
	
	if (a_pips_12 != 0.0)
		l_price_32 = a_price_0 - a_pips_12 * Point;
		
	double l_price_40 = a_price_0 + ai_8 * Point;
	
	int l_ticket_48 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_20, a_price_0, g_slippage_140, l_price_32, l_price_40, "", Magic, 0, Blue);
	
	if (l_ticket_48 < 1) {
		
	}
}

double Thief::ND(double ad_0) {
	return (NormalizeDouble(ad_0, Digits));
}

void Thief::TrailSellStop() {
	double l_price_0;
	double l_price_8;
	double l_ord_stoploss_16 = 0;
	
	for (int l_pos_24 = 0; l_pos_24 < OrdersTotal(); l_pos_24++) {
		if (!(OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_BUY) {
			l_ord_stoploss_16 = OrderStopLoss();
			break;
		}
	}
	
	if (l_ord_stoploss_16 != 0.0) {
		for (int l_pos_24 = 0; l_pos_24 < OrdersTotal(); l_pos_24++) {
			if (!(OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES)))
				break;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_SELLSTOP) {
				if (OrderOpenPrice() >= l_ord_stoploss_16)
					break;
					
				l_price_0 = 0;
				
				l_price_8 = 0;
				
				if (TakeProfit > 0)
					l_price_8 = l_ord_stoploss_16 - TakeProfit * Point;
					
				if (StopLoss > 0)
					l_price_0 = l_ord_stoploss_16 + StopLoss * Point;
					
				while (IsTradeContextBusy())
					Sleep(1000);
					
				RefreshRates();
				
				if (l_ord_stoploss_16 >= Bid - MarketInfo(Symbol(), MODE_STOPLEVEL) * Point)
					break;
					
				if (!((!OrderModify(OrderTicket(), l_ord_stoploss_16, l_price_0, l_price_8, 0, Yellow))))
					break;
					
				return;
			}
		}
	}
}

void Thief::TrailBuyStop() {
	double l_price_0;
	double l_price_8;
	double l_ord_stoploss_16 = 0;
	
	for (int l_pos_24 = 0; l_pos_24 < OrdersTotal(); l_pos_24++) {
		if (!(OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_SELL) {
			l_ord_stoploss_16 = OrderStopLoss();
			break;
		}
	}
	
	if (l_ord_stoploss_16 != 0.0) {
		for (int l_pos_24 = 0; l_pos_24 < OrdersTotal(); l_pos_24++) {
			if (!(OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES)))
				break;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_BUYSTOP) {
				if (OrderOpenPrice() <= l_ord_stoploss_16)
					break;
					
				l_price_0 = 0;
				
				l_price_8 = 0;
				
				if (TakeProfit > 0)
					l_price_8 = l_ord_stoploss_16 + TakeProfit * Point;
					
				if (StopLoss > 0)
					l_price_0 = l_ord_stoploss_16 - StopLoss * Point;
					
				while (IsTradeContextBusy())
					Sleep(1000);
					
				RefreshRates();
				
				if (l_ord_stoploss_16 <= Ask + MarketInfo(Symbol(), MODE_STOPLEVEL) * Point)
					break;
					
				if (!((!OrderModify(OrderTicket(), l_ord_stoploss_16, l_price_0, l_price_8, 0, Yellow))))
					break;
					
				return;
			}
		}
	}
}

void Thief::OpenSell() {
	int l_error_0;
	double l_price_4 = 0;
	double l_price_12 = 0;
	RefreshRates();
	double l_bid_20 = Bid;
	
	if (StopLoss != 0)
		l_price_12 = l_bid_20 + StopLoss * Point;
		
	if (TakeProfit != 0)
		l_price_4 = l_bid_20 - TakeProfit * Point;
		
	Time l_datetime_28 = TimeCurrent();
	
	int l_ticket_32 = OrderSend(Symbol(), OP_SELL, g_lots_172, l_bid_20, g_slippage_140, l_price_12, l_price_4, "", Magic, 0, Red);
	
	if (l_ticket_32 == -1) {
		while (l_ticket_32 == -1 && TimeCurrent() - l_datetime_28 <= 60 && !IsTesting()) {
			Sleep(1000);

			RefreshRates();
			l_bid_20 = Bid;
			
			if (StopLoss != 0)
				l_price_12 = l_bid_20 + StopLoss * Point;
				
			if (TakeProfit != 0)
				l_price_4 = l_bid_20 - TakeProfit * Point;
				
			l_ticket_32 = OrderSend(Symbol(), OP_SELL, g_lots_172, l_bid_20, g_slippage_140, l_price_12, l_price_4, "", Magic, 0, Blue);
		}
	}
	
	if (l_ticket_32 != -1) {
		return;
	}
	
}

void Thief::OpenBuy() {
	int l_error_0;
	double l_price_4 = 0;
	double l_price_12 = 0;
	RefreshRates();
	double l_ask_20 = Ask;
	
	if (StopLoss != 0)
		l_price_12 = l_ask_20 - StopLoss * Point;
		
	if (TakeProfit != 0)
		l_price_4 = l_ask_20 + TakeProfit * Point;
		
	Time l_datetime_28 = TimeCurrent();
	
	int l_ticket_32 = OrderSend(Symbol(), OP_BUY, g_lots_172, l_ask_20, g_slippage_140, l_price_12, l_price_4, "", Magic, 0, Blue);
	
	if (l_ticket_32 == -1) {
		while (l_ticket_32 == -1 && TimeCurrent() - l_datetime_28 <= 60 && !IsTesting()) {
			Sleep(1000);
			
			RefreshRates();
			l_ask_20 = Ask;
			
			if (StopLoss != 0)
				l_price_12 = l_ask_20 - StopLoss * Point;
				
			if (TakeProfit != 0)
				l_price_4 = l_ask_20 + TakeProfit * Point;
				
			l_ticket_32 = OrderSend(Symbol(), OP_BUY, g_lots_172, l_ask_20, g_slippage_140, l_price_12, l_price_4, "", Magic, 0, Blue);
		}
	}
	
	if (l_ticket_32 != -1) {
		return;
	}
	
	
	
}

void Thief::GetBuyParameters(double &a_ord_open_price_0, double &a_ord_takeprofit_8, double &a_ord_stoploss_16, double &a_ord_lots_24) {
	for (int l_pos_32 = 0; l_pos_32 < OrdersTotal(); l_pos_32++) {
		if (!(OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderType() == OP_BUY && OrderMagicNumber() == Magic) {
			a_ord_takeprofit_8 = OrderTakeProfit();
			a_ord_stoploss_16 = OrderStopLoss();
			a_ord_lots_24 = OrderLots();
			a_ord_open_price_0 = OrderOpenPrice();
			return;
		}
	}
}

void Thief::GetSellParameters(double &a_ord_open_price_0, double &a_ord_takeprofit_8, double &a_ord_stoploss_16, double &a_ord_lots_24) {
	for (int l_pos_32 = 0; l_pos_32 < OrdersTotal(); l_pos_32++) {
		if (!(OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderType() == OP_SELL && OrderMagicNumber() == Magic) {
			a_ord_takeprofit_8 = OrderTakeProfit();
			a_ord_stoploss_16 = OrderStopLoss();
			a_ord_lots_24 = OrderLots();
			a_ord_open_price_0 = OrderOpenPrice();
			return;
		}
	}
}

void Thief::GetSellStopParameters(double &a_ord_open_price_0, double &a_ord_takeprofit_8, double &a_ord_stoploss_16, double &a_ord_lots_24) {
	for (int l_pos_32 = 0; l_pos_32 < OrdersTotal(); l_pos_32++) {
		if (!(OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderType() == OP_SELLSTOP && OrderMagicNumber() == Magic) {
			a_ord_takeprofit_8 = OrderTakeProfit();
			a_ord_stoploss_16 = OrderStopLoss();
			a_ord_lots_24 = OrderLots();
			a_ord_open_price_0 = OrderOpenPrice();
			return;
		}
	}
}

void Thief::GetBuyStopParameters(double &a_ord_open_price_0, double &a_ord_takeprofit_8, double &a_ord_stoploss_16, double &a_ord_lots_24) {
	for (int l_pos_32 = 0; l_pos_32 < OrdersTotal(); l_pos_32++) {
		if (!(OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderType() == OP_BUYSTOP && OrderMagicNumber() == Magic) {
			a_ord_takeprofit_8 = OrderTakeProfit();
			a_ord_stoploss_16 = OrderStopLoss();
			a_ord_lots_24 = OrderLots();
			a_ord_open_price_0 = OrderOpenPrice();
			return;
		}
	}
}

void Thief::DeleteAllSellStops() {
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		if (!(OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_SELLSTOP) {
			while (IsTradeContextBusy())
				Sleep(500);
				
			
			if (!OrderDelete(OrderTicket()))
				;
		}
	}
}

void Thief::DeleteAllBuyStops() {
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		if (!(OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_BUYSTOP) {
			while (IsTradeContextBusy())
				Sleep(500);
			
			
			if (!OrderDelete(OrderTicket()))
				;
		}
	}
}

void Thief::DoTrail() {
	for (int l_pos_0 = 0; l_pos_0 < OrdersTotal(); l_pos_0++) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol()) {
			if (OrderType() == OP_BUY) {
				if (OrderMagicNumber() == Magic) {
					if (!UseTrail && Bid - MinProfit * Point > OrderOpenPrice() && OrderOpenPrice() > OrderStopLoss() || OrderStopLoss() == 0.0) {
						if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + BEPoints * Point, OrderTakeProfit(), 0, Pink))
							;
					}
					
					else {
						if (UseTrail && (Bid - OrderOpenPrice() > TrailingStop * Point && OrderStopLoss() < OrderOpenPrice()) || (Bid - OrderStopLoss() >= TrailingStop * Point && OrderStopLoss() > OrderOpenPrice()))
							if (!OrderModify(OrderTicket(), OrderOpenPrice(), Bid - TrailingStop * Point, OrderTakeProfit(), 0, Pink))
								;
					}
				}
			}
			
			else {
				if (OrderType() == OP_SELL) {
					if (OrderMagicNumber() == Magic) {
						if (!UseTrail && Ask + MinProfit * Point < OrderOpenPrice() && OrderStopLoss() > OrderOpenPrice() || OrderStopLoss() == 0.0) {
							if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - BEPoints * Point, OrderTakeProfit(), 0, Pink))
								;
						}
						
						else {
							if (UseTrail && (OrderOpenPrice() - Ask > TrailingStop * Point && OrderStopLoss() > OrderOpenPrice()) || (OrderStopLoss() - Ask >= TrailingStop * Point && OrderStopLoss() < OrderOpenPrice()))
								if (!OrderModify(OrderTicket(), OrderOpenPrice(), Ask + TrailingStop * Point, OrderTakeProfit(), 0, Pink))
									;
						}
					}
				}
			}
		}
	}
}

int Thief::GetLastClosedOrderType() {
	int l_cmd_0 = -1;
	Time l_datetime_4(1970,1,1);
	
	for (int l_pos_8 = 0; l_pos_8 < OrdersHistoryTotal(); l_pos_8++) {
		if (OrderSelect(l_pos_8, SELECT_BY_POS, MODE_HISTORY)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic && OrderType() == OP_BUY || OrderType() == OP_SELL) {
				if (OrderCloseTime() > l_datetime_4) {
					l_datetime_4 = OrderCloseTime();
					l_cmd_0 = OrderType();
				}
			}
		}
	}
	
	return (l_cmd_0);
}

double Thief::LotsOptimized() {
	double l_lotstep_0 = MarketInfo(Symbol(), MODE_LOTSTEP);
	double l_minlot_8 = MarketInfo(Symbol(), MODE_MINLOT);
	int li_16 = 0;
	
	if (l_lotstep_0 == 0.01)
		li_16 = 2;
	else
		li_16 = 1;
		
	double l_minlot_20 = NormalizeDouble(MathFloor(AccountBalance() / 100.0 * MaxLossPercent * RiskPercent / 100.0) / 10000.0, li_16);
	
	if (l_minlot_20 < l_minlot_8)
		l_minlot_20 = l_minlot_8;
		
	return (l_minlot_20);
}

double Thief::GetProfitForDay(int ai_0) {
	double ld_ret_4 = 0;
	
	Time today = Now;
	today.hour = 0;
	today.minute = 0;
	today.second = 0;
	
	for (int l_pos_12 = 0; l_pos_12 < OrdersHistoryTotal(); l_pos_12++) {
		if (!(OrderSelect(l_pos_12, SELECT_BY_POS, MODE_HISTORY)))
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic)
			if (OrderCloseTime() >= today && OrderCloseTime() < today + 86400)
				ld_ret_4 += OrderProfit();
	}
	
	return (ld_ret_4);
}

}

