#include "Overlook.h"


namespace Overlook {

ChannelScalper::ChannelScalper() {
	
}

void ChannelScalper::InitEA() {
	g_digits_500 = Digits;
	
	if (g_digits_500 == 3 || g_digits_500 == 5) {
		price_point = 10.0 * Point;
		point_factor = 10;
	}
	
	else {
		price_point = Point;
		point_factor = 1;
	}
	
	AddSubCore<Channel>()
		.Set("period", timeframe1period);
	
	AddSubCore<BollingerBands>()
		.Set("period", bb_period)
		.Set("deviation", bb_deviation)
		.Set("shift", 0);
	
}

void ChannelScalper::StartEA(int pos) {
	double ld_0;
	double ld_8;

	if (g_bars_504 != Bars || pos == 0) {
		g_count_508 = 0;
		g_bars_504 = Bars;
	}
	
	if (pos < 1)
		return;
	
	g_ord_total_512 = OrdersTotal();
	
	if (breakevengain > 0) {
		for (int l_pos_16 = 0; l_pos_16 < g_ord_total_512; l_pos_16++) {
			OrderSelect(l_pos_16, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_BUY) {
					if (NormalizeDouble(Bid - OrderOpenPrice(), g_digits_500) < NormalizeDouble(breakevengain * price_point, g_digits_500))
						continue;
						
					if (NormalizeDouble(OrderStopLoss() - OrderOpenPrice(), g_digits_500) >= NormalizeDouble(breakeven * price_point, g_digits_500))
						continue;
						
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() + breakeven * price_point, g_digits_500), OrderTakeProfit());
					
					return;
				}
				
				if (NormalizeDouble(OrderOpenPrice() - Ask, g_digits_500) >= NormalizeDouble(breakevengain * price_point, g_digits_500)) {
					if (NormalizeDouble(OrderOpenPrice() - OrderStopLoss(), g_digits_500) < NormalizeDouble(breakeven * price_point, g_digits_500)) {
						OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(OrderOpenPrice() - breakeven * price_point, g_digits_500), OrderTakeProfit());
						return;
					}
				}
			}
		}
	}
	
	if (trailingstop > 0) {
		for (int l_pos_20 = 0; l_pos_20 < g_ord_total_512; l_pos_20++) {
			OrderSelect(l_pos_20, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
				if (OrderType() == OP_BUY) {
					if (!((NormalizeDouble(Ask, g_digits_500) > NormalizeDouble(OrderOpenPrice() + trailingstart * price_point, g_digits_500) && NormalizeDouble(OrderStopLoss(), g_digits_500) < NormalizeDouble(Bid - (trailingstop +
							trailingstep) * price_point, g_digits_500) || OrderStopLoss() == 0.0)))
						continue;
						
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Bid - trailingstop * price_point, g_digits_500), OrderTakeProfit());
					
					return;
				}
				
				if (NormalizeDouble(Bid, g_digits_500) < NormalizeDouble(OrderOpenPrice() - trailingstart * price_point, g_digits_500) && NormalizeDouble(OrderStopLoss(), g_digits_500) > NormalizeDouble(Ask +
						(trailingstop + trailingstep) * price_point, g_digits_500) || OrderStopLoss() == 0.0) {
					OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Ask + trailingstop * price_point, g_digits_500), OrderTakeProfit());
					return;
				}
			}
		}
	}
	
	if (basketpercent) {
		ld_0 = profit * (AccountBalance() / 100.0);
		ld_8 = loss * (AccountBalance() / 100.0);
		equity_diff = AccountEquity() - AccountBalance();
		
		if (equity_diff > max_equity_diff)
			max_equity_diff = equity_diff;
			
		if (equity_diff < min_equity_diff)
			min_equity_diff = equity_diff;
			
		if (equity_diff >= ld_0 || equity_diff <= (-1.0 * ld_8)) {
			for (int i = g_ord_total_512 - 1; i >= 0; i--) {
				OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
				
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY)
					OrderClose(OrderTicket(), OrderLots(), Bid, slippage * price_point);
					
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL)
					OrderClose(OrderTicket(), OrderLots(), Ask, slippage * price_point);
			}
			
			return;
		}
	}
	
	for (int i = 0; i < OrdersTotal(); i++) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic) {
			order_count++;
			
			if (OrderType() == OP_BUY) {
				buy_count++;
				g_ord_open_price_420 = OrderOpenPrice();
				gi_unused_532 = OrderProfit();
				g_datetime_404 = OrderOpenTime();
			}
			
			if (OrderType() == OP_SELL) {
				sell_count++;
				g_ord_open_price_428 = OrderOpenPrice();
				gi_unused_536 = OrderProfit();
				g_datetime_408 = OrderOpenTime();
			}
		}
	}
	
	Core& c = At(0);
	ConstBuffer& lo_buf = c.GetBuffer(0);
	ConstBuffer& hi_buf = c.GetBuffer(1);
	
	Core& bb = At(1);
	ConstBuffer& bblo_buf = bb.GetBuffer(2);
	ConstBuffer& bbhi_buf = bb.GetBuffer(1);
	double bb_lo0 = bblo_buf.Get(pos);
	double bb_lo1 = bblo_buf.Get(pos - 1);
	double bb_hi0 = bbhi_buf.Get(pos);
	double bb_hi1 = bbhi_buf.Get(pos - 1);
	
	double l_ihigh_24 = hi_buf.Get(pos - 1);
	double l_ilow_32 = lo_buf.Get(pos - 1);
	double l_ihigh_40 = hi_buf.Get(pos);
	double l_ilow_48 = lo_buf.Get(pos);
	bool li_280 = true;
	bool li_284 = true;
	bool li_288 = false;
	bool li_292 = false;
	
	if (bb_lo0 > bb_lo1 && bb_hi0 < bb_hi1) {
		li_280 = false;
		li_284 = false;
	}
	
	if (lotsoptimized)
		lots = NormalizeDouble(AccountBalance() / 1000.0 * minlot * risk, lotdigits);
		
	if (lots < minlot)
		lots = minlot;
		
	if (lots > maxlot)
		lots = maxlot;
		
	if (tradesperbar == 1 && TimeCurrent() - g_datetime_404 < Period() || TimeCurrent() - g_datetime_408 < Period())
		g_count_508 = 1;
		
	bool li_296 = false;
	
	bool li_300 = false;
	
	if (l_ihigh_40 > l_ihigh_24 && li_280) {
		if (reversesignals)
			li_300 = true;
		else
			li_296 = true;
	}
	
	if (l_ilow_48 < l_ilow_32 && li_284) {
		if (reversesignals)
			li_296 = true;
		else
			li_300 = true;
	}
	
	if ((oppositeclose && li_300) || li_288) {
		for (int i = g_ord_total_512 - 1; i >= 0; i--) {
			if (!OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) continue;
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY)
				OrderClose(OrderTicket(), OrderLots(), Bid, slippage * price_point);
		}
	}
	
	if ((oppositeclose && li_296) || li_292) {
		for (int i = g_ord_total_512 - 1; i >= 0; i--) {
			if (!OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) continue;
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL)
				OrderClose(OrderTicket(), OrderLots(), Ask, slippage * price_point);
		}
	}
	
	if (hidestop) {
		for (int i = g_ord_total_512 - 1; i >= 0; i--) {
			if (!OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) continue;
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY && buystop > 0 && Bid < OrderOpenPrice() - buystop * price_point)
				OrderClose(OrderTicket(), OrderLots(), Bid, slippage * price_point);
				
			else if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL && sellstop > 0 && Ask > OrderOpenPrice() + sellstop * price_point)
				OrderClose(OrderTicket(), OrderLots(), Ask, slippage * price_point);
		}
	}
	
	if (hidetarget) {
		for (int i = g_ord_total_512 - 1; i >= 0; i--) {
			if (!OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) continue;
			
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_BUY && buytarget > 0 && Bid > OrderOpenPrice() + buytarget * price_point)
				OrderClose(OrderTicket(), OrderLots(), Bid, slippage * price_point);
				
			else if (OrderSymbol() == Symbol() && OrderMagicNumber() == magic && OrderType() == OP_SELL && selltarget > 0 && Ask < OrderOpenPrice() - selltarget * price_point)
				OrderClose(OrderTicket(), OrderLots(), Ask, slippage * price_point);
		}
	}
	
	if (count(OP_BUY, magic) + count(OP_SELL, magic) < maxtrades) {
		if (li_296 && g_count_508 < tradesperbar) {
			while (IsTradeContextBusy())
				Sleep(3000);
				
			if (hidestop == false && buystop > 0)
				g_price_436 = Ask - buystop * price_point;
			else
				g_price_436 = 0;
				
			if (hidetarget == false && buytarget > 0)
				g_price_444 = Ask + buytarget * price_point;
			else
				g_price_444 = 0;
				
			RefreshRates();
			
			g_ticket_516 = OrderSend(Symbol(), OP_BUY, lots, Ask, slippage * point_factor, g_price_436, g_price_444, "", magic, 0);
			
			if (g_ticket_516 <= 0)
				Print("Error Occured : " + GetLastError());
			else {
				g_count_508++;
				Print("Order opened : " + Symbol() + " Buy @ " + Ask + " SL @ " + g_price_436 + " TP @" + g_price_444 + " ticket =" + g_ticket_516);
			}
		}
		
		if (li_300 && g_count_508 < tradesperbar) {
			while (IsTradeContextBusy())
				Sleep(3000);
				
			if (hidestop == false && sellstop > 0)
				g_price_436 = Bid + sellstop * price_point;
			else
				g_price_436 = 0;
				
			if (hidetarget == false && selltarget > 0)
				g_price_444 = Bid - selltarget * price_point;
			else
				g_price_444 = 0;
				
			RefreshRates();
			
			g_ticket_516 = OrderSend(Symbol(), OP_SELL, lots, Bid, slippage * point_factor, g_price_436, g_price_444, "", magic, 0);
			
			if (g_ticket_516 <= 0)
				Print("Error Occured : " + GetLastError());
			else {
				g_count_508++;
				Print("Order opened : " + Symbol() + " Sell @ " + Ask + " SL @ " + g_price_436 + " TP @" + g_price_444 + " ticket =" + g_ticket_516);
			}
		}
	}
}

int ChannelScalper::count(int cmd, int a_magic_4) {
	int l_count_8 = 0;
	
	for (int l_pos_12 = 0; l_pos_12 < OrdersTotal(); l_pos_12++) {
		OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderType() == cmd && OrderMagicNumber() == a_magic_4 || a_magic_4 == 0)
			l_count_8++;
	}
	
	return (l_count_8);
}

}
