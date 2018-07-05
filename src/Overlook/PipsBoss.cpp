#include "Overlook.h"


namespace Overlook {

PipsBoss::PipsBoss() {
	
}

void PipsBoss::InitEA() {
	day_of_year_0 = -100;
	day_of_year_1 = -100;
	day_of_year_2 = -100;
	day_of_year_3 = -100;
	ticket = -100;
	sig = -100;
	hi = 1000;
	lo = -1000;
	
	trail_factor = _trailing_stop_percentage * 0.01 * TakeProfit;
	
	AddSubCore<CommodityChannelIndex>()
		.Set("period", indicator1_period);
	
}

void PipsBoss::StartEA(int pos) {
	this->pos = pos;
	int li_20;
	int li_24;
	
	if (pos < 2) return;
	
	if (Bars < 100) {
		Print("bars less than 100");
		throw ConfExc();
	}
	
	if (TakeProfit < 10.0) {
		Print("TakeProfit less than 10");
		throw ConfExc();
	}
	
	if (account_risk_control && Value_At_Risk > 10.0) {
		Print("Too risky. The risk replaced with 10%.");
		Value_At_Risk = 10;
	}
	
	GetBoxSize();
	
	order_count = OrdersTotal();
	int order_count = 0;
	
	for (int i = 0; i < order_count; i++) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == magic) {
			order_count++;
			ticket = OrderTicket();
		}
	}
	
	if (TimeHour(TimeCurrent()) == _time_shift + 11 && TimeMinute(TimeCurrent()) >= 15)
		do_trade = true;
	else
		do_trade = false;
		
	if (TimeHour(TimeCurrent()) >= _time_shift + 12 && TimeHour(TimeCurrent()) < _time_shift + 12 + opt_active_time)
		do_trade2 = true;
	else
		do_trade2 = false;
		
	if (order_count == 0 && do_trade || do_trade2)
		if (day_of_year_0 == DayOfYear() && day_of_year_1 == DayOfWeek() && day_of_year_2 != DayOfYear() && day_of_year_3 != DayOfWeek())
			li_20 = GetInitialEntry();
			
	if (TimeHour(TimeCurrent()) == _time_shift + 11 && TimeMinute(TimeCurrent()) >= 15)
		do_trade = true;
	else
		do_trade = false;
		
	if (TimeHour(TimeCurrent()) >= _time_shift + 12 && TimeHour(TimeCurrent()) < _time_shift + 12 + opt_active_time)
		do_trade2 = true;
	else
		do_trade2 = false;
		
	if (OrderSelect(ticket, SELECT_BY_TICKET))
		close_price = OrderClosePrice();
	else
		return;
		
	if (reversetrade && order_count == 0 && do_trade || do_trade2)
		if (ticket > 0 && day_of_year_2 == DayOfYear() && day_of_year_3 == DayOfWeek())
			li_24 = GetReverseTrade();
			
	if (order_count == 1 && OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES)) {
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol()) {
			if (OrderType() == OP_BUY) {
				if (trail_factor > 0.0) {
					if (Bid - OrderOpenPrice() > Point * trail_factor) {
						if (OrderStopLoss() < Bid - Point * trail_factor) {
							OrderModify(OrderTicket(), OrderOpenPrice(), Bid - Point * trail_factor, OrderTakeProfit(), 0);
							return;
						}
					}
				}
			}
			
			if (OrderType() == OP_SELL) {
				if (trail_factor > 0.0) {
					if (OrderOpenPrice() - Ask > Point * trail_factor) {
						if (OrderStopLoss() > Ask + Point * trail_factor) {
							OrderModify(OrderTicket(), OrderOpenPrice(), Ask + Point * trail_factor, OrderTakeProfit(), 0);
							return;
						}
					}
				}
			}
		}
	}
	
}

double PipsBoss::CalculateLotSize(double a_pips_0) {
	double l_bid_16;
	
	if (Value_At_Risk <= 0.0)
		return (user_lot_size * 0.01);
		
	A = StringSubstr(Symbol(), 3, 3);
	
	if (A != "USD") {
		pair = StringConcatenate("USD", A);
		l_bid_16 = MarketInfo(pair, MODE_BID);
	}
	
	else
		l_bid_16 = 1.0;
		
	if (l_bid_16 <= 0.0)
		l_bid_16 = 116.45;
		
	double l_lotsize_24 = MarketInfo(Symbol(), MODE_LOTSIZE);
	
	double ld_ret_8 = Value_At_Risk / 100.0 * AccountFreeMargin() * l_bid_16 / (a_pips_0 * Point * l_lotsize_24);
	
	ld_ret_8 = MathRound(10.0 * ld_ret_8) / 10.0;
	
	double l_minlot_32 = MarketInfo(Symbol(), MODE_MINLOT);
	
	if (ld_ret_8 < l_minlot_32)
		ld_ret_8 = l_minlot_32;
		
	return (ld_ret_8);
}

int PipsBoss::GetBoxSize() {
	int l_highest_0;
	int l_lowest_4;
	
	if (TimeHour(TimeCurrent()) == _time_shift + 11 && TimeMinute(TimeCurrent()) > 1 && TimeMinute(TimeCurrent()) <= 14) {
		ConstBuffer& open_buf = GetInputBuffer(0, 0);
		hi = -DBL_MAX, lo = +DBL_MAX;
		for(int i = 0; i < 30; i++) {
			double d = open_buf.Get(max(0, pos - i));
			if (d < lo) lo = d;
			if (d > hi) hi = d;
		}
		tp_factor = (hi - lo) * _mid_range_factor * 0.01 / Point;
		day_of_year_0 = DayOfYear();
		day_of_year_1 = DayOfWeek();
	}
	
	return 0;
}

int PipsBoss::GetInitialEntry() {
	double ld_16;
	double ld_48;
	double l_pips_56;
	double l_spread_64;
	double l_lots_0 = CalculateLotSize(StopLoss);
	double ld_8 = StopLoss;
	String l_comment_32 = "";
	bool li_24 = false;
	bool li_28 = false;
	
	const Core& cci = At(0);
	double l_icci_40 = cci.GetBuffer(0).Get(pos);
	
	if (l_icci_40 >= indicator1_check && Bid >= hi + price_deviation * Point) {
		li_24 = true;
		l_comment_32 = "Normal Long Pips Leader";
	}
	
	if (counter_trade && l_icci_40 > (-1.0 * indicator1_check) && Bid <= lo - price_deviation * Point) {
		li_24 = true;
		l_comment_32 = "Counter Long Pips Leader";
	}
	
	if (l_icci_40 <= (-1.0 * indicator1_check) && Bid <= lo - price_deviation * Point) {
		li_28 = true;
		l_comment_32 = "Normal Short Pips Leader";
	}
	
	if (counter_trade && l_icci_40 < indicator1_check && Bid >= hi + price_deviation * Point) {
		li_28 = true;
		l_comment_32 = "Counter Short Pips Leader";
	}
	
	if (li_24) {
		ConstBuffer& low = GetInputBuffer(0, 1);
		ld_16 = (Ask - MathMin(low.Get(pos-1), low.Get(pos-2))) / Point;
		ld_8 = MathMax(ld_16, StopLoss);
		ld_8 = MathMax(ld_8, tp_factor);
		l_lots_0 = CalculateLotSize(ld_8);
		ld_48 = MathMax(tp_factor, TakeProfit);
		l_pips_56 = ld_48;
		trail_factor = MathMax(TrailingStop, _trailing_stop_percentage * 0.01 * l_pips_56);
		
		ticket = OrderSend(Symbol(), OP_BUY, l_lots_0, Ask, 2, Ask - ld_8 * Point, Ask + l_pips_56 * Point, l_comment_32, magic, 0);
		
		if (ticket > 0) {
			if (OrderSelect(ticket, SELECT_BY_TICKET)) {
				g_ord_stoploss_276 = OrderStopLoss();
				sig = 1;
				day_of_year_2 = DayOfYear();
				day_of_year_3 = DayOfWeek();
			}
		}
		
		else
			Print("Error opening BUY order : " + GetLastError());
			
		return 0;
	}
	
	if (li_28) {
		ConstBuffer& high = GetInputBuffer(0, 2);
		ld_16 = (MathMax(high.Get(pos-1), high.Get(pos-2)) - Bid) / Point;
		ld_8 = MathMax(ld_16, StopLoss);
		l_spread_64 = MarketInfo(Symbol(), MODE_SPREAD);
		ld_8 = MathMax(ld_8, tp_factor + l_spread_64);
		l_lots_0 = CalculateLotSize(ld_8);
		ld_48 = MathMax(tp_factor - l_spread_64, TakeProfit);
		l_pips_56 = ld_48;
		trail_factor = MathMax(TrailingStop, _trailing_stop_percentage * 0.01 * l_pips_56);
		
		
		ticket = OrderSend(Symbol(), OP_SELL, l_lots_0, Bid, 2, Bid + ld_8 * Point, Bid - l_pips_56 * Point, l_comment_32, magic, 0);
		
		if (ticket > 0) {
			if (OrderSelect(ticket, SELECT_BY_TICKET)) {
				g_ord_stoploss_276 = OrderStopLoss();
				sig = -1;
				day_of_year_2 = DayOfYear();
				day_of_year_3 = DayOfWeek();
			}
		}
		
		else
			Print("Error opening SELL order : " + GetLastError());
			
		return 0;
	}
	
	return 0;
}

int PipsBoss::GetReverseTrade() {
	double ld_16;
	double ld_24;
	double ld_32;
	double l_spread_40;
	double l_lots_0 = CalculateLotSize(StopLoss);
	double ld_8 = StopLoss;
	
	if (sig == -1 && MathAbs(close_price - g_ord_stoploss_276) <= 1.0 * Point) {
		ConstBuffer& low = GetInputBuffer(0, 1);
		ld_16 = (Ask - MathMin(low.Get(pos-1), low.Get(pos-2))) / Point;
		ld_8 = MathMax(ld_16, StopLoss);
		ld_8 = MathMax(ld_8, tp_factor);
		l_lots_0 = CalculateLotSize(ld_8);
		ld_24 = MathMax(tp_factor, TakeProfit);
		ld_32 = ld_16;
		trail_factor = MathMax(TrailingStop, _trailing_stop_percentage * 0.01 * ld_32);
		
		ticket = OrderSend(Symbol(), OP_BUY, l_lots_0, Ask, 2, Ask - ld_8 * Point, Ask + ld_32 * Point, "reversetrade Long Pips Leader", magic, 0);
		
		if (ticket > 0) {
			if (OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES)) {
				day_of_year_2 = -100;
				day_of_year_3 = -100;
				day_of_year_0 = -100;
				day_of_year_1 = -100;
			}
		}
		
		else
			Print("Error opening BUY order : " + GetLastError());
			
		return 0;
	}
	
	if (sig == 1 && MathAbs(close_price - g_ord_stoploss_276) <= 1.0 * Point) {
		ConstBuffer& high = GetInputBuffer(0, 2);
		ld_16 = (MathMax(high.Get(pos-1), high.Get(pos-2)) - Bid) / Point;
		ld_8 = MathMax(ld_16, StopLoss);
		l_spread_40 = MarketInfo(Symbol(), MODE_SPREAD);
		ld_8 = MathMax(ld_8, tp_factor + l_spread_40);
		l_lots_0 = CalculateLotSize(ld_8);
		ld_24 = MathMax(tp_factor - l_spread_40, TakeProfit);
		ld_32 = ld_24;
		trail_factor = MathMax(TrailingStop, _trailing_stop_percentage * 0.01 * ld_32);
		
		
		ticket = OrderSend(Symbol(), OP_SELL, l_lots_0, Bid, 2, Bid + ld_8 * Point, Bid - ld_32 * Point, "reversetrade Short Pip Pips Leader", magic, 0);
		
		if (ticket > 0) {
			if (OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES)) {
				day_of_year_2 = -100;
				day_of_year_3 = -100;
				day_of_year_0 = -100;
				day_of_year_1 = -100;
			}
		}
		
		else
			Print("Error opening SELL order : " + GetLastError());
			
		return 0;
	}
	
	return 0;
}

}
