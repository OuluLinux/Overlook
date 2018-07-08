#include "Overlook.h"

namespace Overlook {

SoundOfForex::SoundOfForex() {
	
}

void SoundOfForex::InitEA() {
	int li_0;
	g_lots_112 = lots;
	g_digits_352 = Digits;
	
	if (g_digits_352 == 3 || g_digits_352 == 5) {
		point0 = 10.0 * Point;
		gd_312 = 10;
	}
	
	else {
		point0 = Point;
		gd_312 = 1;
	}
	
	int l_ord_total_4 = OrdersTotal();
	
	if (li_0 == 0 && l_ord_total_4 > 0) {
		for (int l_pos_8 = 0; l_pos_8 < l_ord_total_4; l_pos_8++) {
			if (OrderSelect(l_pos_8, SELECT_BY_POS)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
					li_0 = NormalizeDouble(OrderLots() / lots, (MarketInfo(Symbol(), MODE_MINLOT) == 0.01) + 1);
					break;
				}
			}
		}
	}
	
	int l_hist_total_12 = OrdersHistoryTotal();
	
	if (li_0 == 0 && l_hist_total_12 > 0) {
		for (int i = 0; i < l_hist_total_12; i++) {
			if (OrderSelect(i, SELECT_BY_POS, MODE_HISTORY)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
					li_0 = NormalizeDouble(OrderLots() / lots, (MarketInfo(Symbol(), MODE_MINLOT) == 0.01) + 1);
					break;
				}
			}
		}
	}
	
	if (li_0 > 0)
		mart_lots0 = li_0;
	
	AddSubCore<Fractals>()
		.Set("left_bars", frac_left_bars)
		.Set("right_bars", frac_right_bars);
	
	AddSubCore<MovingAverageConvergenceDivergence>()
		.Set("fast_ema", macd_fast_ema)
		.Set("slow_ema", macd_slow_ema)
		.Set("signal_sma_period", macd_signal_sma_period);
		
}

void SoundOfForex::StartEA(int pos) {
	double l_ifractals_16;
	double l_ifractals_24;
	double l_ifractals_32;
	double l_ifractals_40;
	g_ord_total_364 = OrdersTotal();
	
	if (g_bars_356 != Bars || pos == 0) {
		g_count_360 = 0;
		g_bars_356 = Bars;
	}
	
	if (pos < 50)
		return;
	
	if (sl_factor4 > 0) {
		for (int i8 = 0; i8 < g_ord_total_364; i8++) {
			OrderSelect(i8, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_BUY) {
					if (NormalizeDouble(Bid - OrderOpenPrice(), g_digits_352) < NormalizeDouble(sl_factor4 * point0, g_digits_352))
						continue;
						
					if (NormalizeDouble(OrderStopLoss() - OrderOpenPrice(), g_digits_352) >= NormalizeDouble(tp_factor4 * point0, g_digits_352))
						continue;
						
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() + tp_factor4 * point0, g_digits_352), OrderTakeProfit());
					
					return;
				}
				
				if (NormalizeDouble(OrderOpenPrice() - Ask, g_digits_352) >= NormalizeDouble(sl_factor4 * point0, g_digits_352)) {
					if (NormalizeDouble(OrderOpenPrice() - OrderStopLoss(), g_digits_352) < NormalizeDouble(tp_factor4 * point0, g_digits_352)) {
						OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() - tp_factor4 * point0, g_digits_352), OrderTakeProfit());
						return;
					}
				}
			}
		}
	}
	
	if (sl_factor2 > 0) {
		for (int l_pos_52 = 0; l_pos_52 < g_ord_total_364; l_pos_52++) {
			OrderSelect(l_pos_52, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_BUY) {
					if (!((NormalizeDouble(Ask, g_digits_352) > NormalizeDouble(OrderOpenPrice() + tp_factor2 * point0, g_digits_352) && NormalizeDouble(OrderStopLoss(), g_digits_352) < NormalizeDouble(Bid - (sl_factor2 + sl_factor3) * point0, g_digits_352) || OrderStopLoss() == 0.0)))
						continue;
						
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Bid - sl_factor2 * point0, g_digits_352), OrderTakeProfit());
					
					return;
				}
				
				if (NormalizeDouble(Bid, g_digits_352) < NormalizeDouble(OrderOpenPrice() - tp_factor2 * point0, g_digits_352) && NormalizeDouble(OrderStopLoss(), g_digits_352) > NormalizeDouble(Ask +
						(sl_factor2 + sl_factor3) * point0, g_digits_352) || OrderStopLoss() == 0.0) {
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Ask + sl_factor2 * point0, g_digits_352), OrderTakeProfit());
					return;
				}
			}
		}
	}
	
	for (int i = 0; i < OrdersTotal(); i++) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			total_count++;
			g_ord_profit_336 = OrderProfit();
			
			if (OrderType() == OP_BUY) {
				buy_count++;
				g_ord_open_price_272 = OrderOpenPrice();
				gi_unused_384 = OrderProfit();
				g_datetime_256 = OrderOpenTime();
			}
			
			if (OrderType() == OP_SELL) {
				sell_count++;
				g_ord_open_price_280 = OrderOpenPrice();
				gi_unused_388 = OrderProfit();
				g_datetime_260 = OrderOpenTime();
			}
		}
	}
	
	ConstBuffer& up_buf = At(0).GetBuffer(2);
	ConstBuffer& down_buf = At(0).GetBuffer(3);
	
	for (int i = 0; i < 50; i++) {
		if (l_ifractals_16 == 0.0) {
			l_ifractals_32 = up_buf.Get(pos - i);
			
			if (l_ifractals_32 > 0.0 && l_ifractals_32 != 0)
				l_ifractals_16 = l_ifractals_32;
		}
	}
	
	for (int i = 0; i < 50; i++) {
		if (l_ifractals_24 == 0.0) {
			l_ifractals_40 = down_buf.Get(pos - i);
			
			if (l_ifractals_40 > 0.0 && l_ifractals_40 != 0)
				l_ifractals_24 = l_ifractals_40;
		}
	}
	
	ConstBuffer& macd_sig = At(1).GetBuffer(1);
	double l_imacd_64 = macd_sig.Get(pos - 0);
	double l_imacd_72 = macd_sig.Get(pos - 1);
	double l_imacd_80 = macd_sig.Get(pos - 2);
	bool li_88 = true;
	bool li_92 = true;
	bool li_96 = false;
	bool li_100 = false;
	
	if (use_martingale == false || (use_martingale && g_ord_profit_336 >= 0.0))
		lots = NormalizeDouble(AccountBalance() / 1000.0 * g_lots_112 * risk, lotdigits);
		
	if (lots < g_lots_112)
		lots = g_lots_112;
		
	if (lots > max_lots)
		lots = max_lots;
		
	if (TimeCurrent() - g_datetime_256 < Period() || TimeCurrent() - g_datetime_260 < Period() && 1)
		g_count_360 = 1;
		
	bool li_104 = false;
	
	bool li_108 = false;
	
	double ld_112 = 0;
	
	if (l_imacd_64 > l_imacd_72 && l_imacd_72 < l_imacd_80 && li_88) {
		if (use_macdclose0)
			li_108 = true;
		else
			li_104 = true;
			
		ld_112 = NormalizeDouble((l_ifractals_16 - Ask) / point0, 0);
	}
	
	if (l_imacd_64 < l_imacd_72 && l_imacd_72 > l_imacd_80 && li_92) {
		if (use_macdclose0)
			li_104 = true;
		else
			li_108 = true;
			
		ld_112 = NormalizeDouble((Bid - l_ifractals_24) / point0, 0);
	}
	
	if ((use_orderclose0 && li_108) || li_96) {
		for (g_pos_344 = g_ord_total_364 - 1; g_pos_344 >= 0; g_pos_344--) {
			OrderSelect(g_pos_344, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY)
				OrderClose(OrderTicket(), OrderLots(), Bid, slippage_factor * point0);
		}
	}
	
	if ((use_orderclose0 && li_104) || li_100) {
		for (int i = g_ord_total_364 - 1; i >= 0; i--) {
			OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL)
				OrderClose(OrderTicket(), OrderLots(), Ask, slippage_factor * point0);
		}
	}
	
	if (use_manual_sl) {
		for (int i = g_ord_total_364 - 1; i >= 0; i--) {
			OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY && sl_factor0 > 0 && Bid < OrderOpenPrice() - sl_factor0 * point0)
				OrderClose(OrderTicket(), OrderLots(), Bid, slippage_factor * point0);
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL && sl_factor1 > 0 && Ask > OrderOpenPrice() + sl_factor1 * point0)
				OrderClose(OrderTicket(), OrderLots(), Ask, slippage_factor * point0);
		}
	}
	
	if (use_manual_tp) {
		for (g_pos_348 = g_ord_total_364 - 1; g_pos_348 >= 0; g_pos_348--) {
			OrderSelect(g_pos_348, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY && tp_factor0 > 0 && Bid > OrderOpenPrice() + tp_factor0 * point0)
				OrderClose(OrderTicket(), OrderLots(), Bid, slippage_factor * point0);
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL && tp_factor1 > 0 && Ask < OrderOpenPrice() - tp_factor1 * point0)
				OrderClose(OrderTicket(), OrderLots(), Ask, slippage_factor * point0);
		}
	}
	
	begin_hour1 = begin_hour + gmtshift;
	
	if (begin_hour1 > 23)
		begin_hour1 -= 24;
		
	end_hour1 = end_hour + gmtshift;
	
	if (end_hour1 > 23)
		end_hour1 -= 24;
		
	if ((skip_sunday == false && DayOfWeek() == 0) || (use_checktime && DayOfWeek() > 0 && (begin_hour1 < end_hour1 && !(Hour() >= begin_hour1 && Hour() <= end_hour1)) || (begin_hour1 > end_hour1 && !((Hour() >= begin_hour1 &&
			Hour() <= 23) || (Hour() >= 0 && Hour() <= end_hour1)))) || (skip_friday && DayOfWeek() == 5 && !(Hour() < end_hour1 + gmtshift)))
		return;
	
	Time l_datetime_120(1970,1,1);
	
	if (expiration_hours > 0)
		l_datetime_120 = TimeCurrent() + 60 * expiration_hours - 5;
		
	if (count(OP_BUY, magic) + count(OP_SELL, magic) < max_countsum) {
		if (li_104 && g_count_360 < 1) {
			while (IsTradeContextBusy())
				Sleep(3000);
				
			if (use_manual_sl == false && sl_factor0 > 0)
				tp0 = Ask + slsl_factor1 * point0 + ld_112 * point0 - sl_factor0 * point0;
			else
				tp0 = 0;
				
			if (use_manual_tp == false && tp_factor0 > 0)
				sl0 = Ask + slsl_factor1 * point0 + ld_112 * point0 + tp_factor0 * point0;
			else
				sl0 = 0;
				
			if (use_martingale)
				mart_lots1 = NormalizeDouble(lots * martingalefactor(), 2);
			else
				mart_lots1 = lots;
				
			if (mart_lots1 < g_lots_112)
				mart_lots1 = g_lots_112;
				
			if (mart_lots1 > max_lots)
				mart_lots1 = max_lots;
				
			RefreshRates();
			
			g_ticket_368 = OrderSend(Symbol(), OP_BUYSTOP, mart_lots1, Ask + slsl_factor1 * point0 + ld_112 * point0, slippage_factor * gd_312, tp0, sl0, "", magic);
			               
			if (g_ticket_368 <= 0)
				Print("Error Occured : " + GetLastError());
			else {
				g_count_360++;
				Print("Order opened : " + Symbol() + " Buy @ " + ld_112 + " SL @ " + tp0 + " TP @" + sl0 + " ticket =" + g_ticket_368);
			}
		}
		
		if (li_108 && g_count_360 < 1) {
			while (IsTradeContextBusy())
				Sleep(3000);
				
			if (use_manual_sl == false && sl_factor1 > 0)
				tp0 = Bid - slsl_factor1 * point0 - ld_112 * point0 + sl_factor1 * point0;
			else
				tp0 = 0;
				
			if (use_manual_tp == false && tp_factor1 > 0)
				sl0 = Bid - slsl_factor1 * point0 - ld_112 * point0 - tp_factor1 * point0;
			else
				sl0 = 0;
				
			if (use_martingale)
				mart_lots1 = NormalizeDouble(lots * martingalefactor(), 2);
			else
				mart_lots1 = lots;
				
			if (mart_lots1 < g_lots_112)
				mart_lots1 = g_lots_112;
				
			if (mart_lots1 > max_lots)
				mart_lots1 = max_lots;
				
			RefreshRates();
			
			g_ticket_368 = OrderSend(Symbol(), OP_SELLSTOP, mart_lots1, Bid - slsl_factor1 * point0 - ld_112 * point0, slippage_factor * gd_312, tp0, sl0, "", magic);
			               
			if (g_ticket_368 <= 0)
				Print("Error Occured : " + GetLastError());
			else {
				g_count_360++;
				Print("Order opened : " + Symbol() + " Sell @ " + ld_112 + " SL @ " + tp0 + " TP @" + sl0 + " ticket =" + g_ticket_368);
			}
		}
	}
	
}

int SoundOfForex::count(int cmd, int a_magic_4) {
	int l_count_8 = 0;
	
	for (int l_pos_12 = 0; l_pos_12 < OrdersTotal(); l_pos_12++) {
		OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderType() == cmd && OrderMagicNumber() == a_magic_4 || a_magic_4 == 0)
			l_count_8++;
	}
	
	return (l_count_8);
}

int SoundOfForex::martingalefactor() {
	int l_hist_total_0 = OrdersHistoryTotal();
	
	if (l_hist_total_0 > 0) {
		for (int i = l_hist_total_0 - 1; i >= 0; i--) {
			if (OrderSelect(i, SELECT_BY_POS, MODE_HISTORY)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
					if (OrderProfit() < 0.0) {
						mart_lots0 = 2.0 * mart_lots0;
						return (mart_lots0);
					}
					
					mart_lots0 = mart_lots2;
					
					if (mart_lots0 <= 0.0)
						mart_lots0 = 1;
						
					return (mart_lots0);
				}
			}
		}
	}
	
	return (mart_lots0);
}

}

