#include "Overlook.h"

namespace Overlook {



HighPromises::HighPromises() {

}

void HighPromises::InitEA() {
	
	AddSubCore<StochasticOscillator>()
		.Set("k_period", stoch_k_period)
		.Set("d_period", stoch_d_period)
		.Set("slowing", stoch_slowing);
	
	AddSubCore<CommodityChannelIndex>()
		.Set("period", cci_buy_period);
	
	AddSubCore<StochasticOscillator>()
		.Set("k_period", stoch2_k_period)
		.Set("d_period", stoch2_d_period)
		.Set("slowing", stoch2_slowing);
	
	AddSubCore<AverageDirectionalMovement>()
		.Set("period", adx_sell_period);
	
	AddSubCore<CommodityChannelIndex>()
		.Set("period", cci_sell_period);
	
	
}

void HighPromises::StartEA(int pos) {

	if (AccountFreeMargin() < 1000.0 * LotsOptimized(MAGIC))
		if (AccountFreeMargin() < 1000.0 * LotsOptimized(MAGIC2))
			return;
			
	if (Buy(pos))
		OpenBuyOrder();
		
	if (Sell(pos))
		OpenSellOrder();
}


int HighPromises::Buy(int pos) {
	const Core& stoch = At(0);
	const Core& cci = At(1);
	
	double stoch_value = stoch.GetBuffer(0).Get(pos);
	double cci_value = cci.GetBuffer(0).Get(pos);
	
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	int mins = GetMinutePeriod(), shift, begin;
	shift = 60*4/mins;
	begin = max(0, pos - shift);
	double open4h = open_buf.Get(begin);
	shift = 10080*3/mins;
	begin = max(0, pos - shift);
	double open3w = open_buf.Get(begin);
	double close = open_buf.Get(pos);
	
	
	if (open4h - close <= h4_limit * Point &&
		open3w <= close - w3_limit * Point &&
		stoch_value <= stoch_buy_limit &&
		cci_value <= cci_buy_limit)
		return 1;
	
	return 0;
}

int HighPromises::Sell(int pos) {
	const Core& stoch = At(2);
	const Core& adx = At(3);
	const Core& cci = At(4);
	double stoch_value = stoch.GetBuffer(0).Get(pos);
	double adx0_value = adx.GetBuffer(1).Get(max(0, pos - adx0_shift));
	double adx1_value = adx.GetBuffer(2).Get(pos);
	double cci_value = cci.GetBuffer(0).Get(max(0, pos - cci_shift));
	
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	int mins = GetMinutePeriod(), shift, begin;
	shift = 60*12/mins;
	begin = max(0, pos - shift);
	double open12h = open_buf.Get(begin);
	double close = open_buf.Get(pos);
	
	
	if (open12h - close >= h12_limit * Point &&
		stoch_value >= stoch_sell_limit &&
		adx0_value < adx1_value &&
		cci_value >= cci_sell_limit)
		return 1;
	
	return 0;
}

void HighPromises::OpenBuyOrder() {
	double sl = 0, tp = 0;
	
	if (!OpenPositions1()) {
		if (StopLoss > 0)
			sl = Ask - StopLoss * Point;
			
		tp = 0;
		
		if (TakeProfit > 0)
			tp = Ask + TakeProfit * Point;
			
		OrderSend(Symbol(), OP_BUY, LotsOptimized(MAGIC), Ask, 1, sl, tp, "", MAGIC, 0);
		
		PrintAlert("Open buy for " + DoubleToStr(Ask, 4));
	}
}

void HighPromises::OpenSellOrder() {
	double sl = 0, tp = 0;
	
	if (!OpenPositions2()) {
		if (StopLoss > 0)
			sl = Bid + StopLoss * Point;
			
		tp = 0;
		
		if (TakeProfit > 0)
			tp = Bid - TakeProfit * Point;
			
		OrderSend(Symbol(), OP_SELL, LotsOptimized(MAGIC2), Bid, 1, sl, tp, "", MAGIC2, 0);
		
		PrintAlert("Open sell for " + DoubleToStr(Bid, 4));
	}
}

void HighPromises::PrintAlert(String str) {
	Print(Symbol() + " - " + str);
}

bool HighPromises::OpenPositions1() {
	int count = 0;
	
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == MAGIC) {
			if (OrderSymbol() == Symbol())
				if (OrderType() == OP_BUY)
					count++;
		}
	}
	
	return count;
}

bool HighPromises::OpenPositions2() {
	int count = 0;
	
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == MAGIC2) {
			if (OrderSymbol() == Symbol())
				if (OrderType() == OP_SELL)
					count++;
		}
	}
	
	return (count);
}

double HighPromises::LotsOptimized(int magic) {
	double ret_lots = Lots;
	int history_count = OrdersHistoryTotal();
	int match_count = 0;
	ret_lots = NormalizeDouble(AccountFreeMargin() * max_risk / 1000.0, 1);
	
	if (use_ea_mgr == true)
		ret_lots = NormalizeDouble(AccountBalance() * max_risk / 1000.0, 1);
		
	if (decrease_factor > 0.0) {
		for (int i = history_count - 1; i >= 0; i--) {
			if (OrderSelect(i, SELECT_BY_POS, MODE_HISTORY) == false) {
				Print("Error in history!");
				break;
			}
			
			if (OrderSymbol() != Symbol() || OrderType() > OP_SELL || OrderMagicNumber() != magic)
				continue;
				
			if (OrderProfit() > 0.0)
				break;
				
			if (OrderProfit() < 0.0 && OrderMagicNumber() == magic)
				match_count++;
		}
		
		if (match_count > 2)
			ret_lots = NormalizeDouble(ret_lots - ret_lots * match_count / decrease_factor, 1);
	}
	
	if (ret_lots < 0.1)
		ret_lots = 0.1;
		
	if (use_mm == false)
		ret_lots = Lots;
		
	if (ret_lots > max_lots)
		ret_lots = max_lots;
		
	return (ret_lots);
}

}
