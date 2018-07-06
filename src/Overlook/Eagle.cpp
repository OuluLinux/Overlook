#include "Overlook.h"

namespace Overlook {

Eagle::Eagle() {
	
}

void Eagle::InitEA() {
	
	
	AddSubCore<Channel>()
		.Set("fast_ema", macd_fast_ema)
		.Set("slow_ema", macd_slow_ema)
		.Set("signal_sma", macd_signal_sma)
		;
	
	AddSubCore<StochasticOscillator>()
		.Set("k_period", stoch0_k_period)
		.Set("d_period", stoch0_d_period)
		.Set("slowing", stoch0_slowing);
	
	AddSubCore<StochasticOscillator>()
		.Set("k_period", stoch1_k_period)
		.Set("d_period", stoch1_d_period)
		.Set("slowing", stoch1_slowing);
	
	AddSubCore<ParabolicSAR>()
		.Set("step", sar_step)
		.Set("maximum", sar_max);
	
	AddSubCore<Momentum>()
		.Set("period", mom_period);
	
}

void Eagle::StartEA(int pos) {
	String ls_0;
	
	if (pos < 1)
		return;
	
	if (use_trailing)
		TrailingPositions();
		
	HandleOpenPositions1();
	
	gi_408 = openPositions();
	
	if (gi_408 > 0)
		return;
		
	if (AccountFreeMargin() < 1000.0 * GetMoneyManagement()) {
		Print("Not enough margin= " + DblStr(AccountFreeMargin()));
		return;
	}
	
	if (CheckEntryConditionBUY(pos))
		OpenBuyOrder();
		
	if (CheckEntryConditionSELL(pos))
		OpenSellOrder();
	
}

bool Eagle::CheckExitCondition(String as_unused_0) {
	bool li_ret_8 = false;
	return (li_ret_8);
}

bool Eagle::CheckEntryConditionBUY(int pos) {
	
	if (use_macd) {
		Core& c = At(0);
		g_imacd_312 = c.GetBuffer(0).Get(pos);
		g_imacd_320 = c.GetBuffer(1).Get(pos);
		
		if (g_imacd_312 >= g_imacd_320)
			return (false);
	}
	
	if (use_stoch) {
		Core& c = At(1);
		g_istochastic_336 = c.GetBuffer(0).Get(pos);
		
		if (g_istochastic_336 >= stoch_hi_limit)
			return (false);
	}
	
	if (use_stoch2) {
		Core& c = At(2);
		g_istochastic_336 = c.GetBuffer(0).Get(pos);
		g_istochastic_344 = c.GetBuffer(0).Get(pos - 1);
		g_istochastic_352 = c.GetBuffer(1).Get(pos);
		g_istochastic_360 = c.GetBuffer(1).Get(pos - 1);
		
		if (g_istochastic_336 >= stoch_hi_limit)
			return (false);
			
		if (g_istochastic_352 >= g_istochastic_336)
			return (false);
			
		if (g_istochastic_360 <= g_istochastic_344)
			return (false);
	}
	
	if (use_sar) {
		Core& c = At(3);
		g_isar_368 = c.GetBuffer(0).Get(pos);
		
		if (g_isar_368 > Ask)
			return (false);
			
		g_isar_376 = c.GetBuffer(0).Get(pos - 1);
		
		if (g_isar_376 < g_isar_368)
			return (false);
	}
	
	if (use_momentum) {
		Core& c = At(4);
		g_imomentum_384 = c.GetBuffer(0).Get(pos);
		
		if (g_imomentum_384 >= mom_limit)
			return (false);
	}
	
	if (Fridaymode == true && DayOfWeek() == 5)
		return (false);
		
	return (true);
}

bool Eagle::CheckEntryConditionSELL(int pos) {
	if (use_macd) {
		Core& c = At(0);
		g_imacd_312 = c.GetBuffer(0).Get(pos);
		g_imacd_320 = c.GetBuffer(1).Get(pos);
		
		if (g_imacd_312 <= g_imacd_320)
			return (false);
	}
	
	if (use_stoch) {
		Core& c = At(1);
		g_istochastic_336 = c.GetBuffer(0).Get(pos);
		
		if (g_istochastic_336 <= stoch_hi_limit)
			return (false);
	}
	
	if (use_stoch2) {
		Core& c = At(2);
		g_istochastic_336 = c.GetBuffer(0).Get(pos);
		g_istochastic_344 = c.GetBuffer(0).Get(pos - 1);
		g_istochastic_352 = c.GetBuffer(1).Get(pos);
		g_istochastic_360 = c.GetBuffer(1).Get(pos - 1);
		
		if (g_istochastic_336 <= stoch_hi_limit)
			return (false);
			
		if (g_istochastic_352 <= g_istochastic_336)
			return (false);
			
		if (g_istochastic_360 >= g_istochastic_344)
			return (false);
	}
	
	if (use_sar) {
		Core& c = At(3);
		g_isar_368 = c.GetBuffer(0).Get(pos);
		
		if (g_isar_368 < Bid)
			return (false);
			
		g_isar_376 = c.GetBuffer(0).Get(pos - 1);
		
		if (g_isar_376 >= g_isar_368)
			return (false);
	}
	
	if (use_momentum) {
		Core& c = At(4);
		g_imomentum_384 = c.GetBuffer(0).Get(pos);
		
		if (g_imomentum_384 <= mom_limit)
			return (false);
	}
	
	if (Fridaymode == true && DayOfWeek() == 5)
		return (false);
		
	return (true);
}

void Eagle::OpenBuyOrder() {
	double l_price_0 = 0;
	double l_price_8 = 0;
	l_price_0 = 0;
	
	if (sl_factor > 0)
		l_price_0 = Ask - sl_factor * Point;
		
	l_price_8 = 0;
	
	if (tp_factor > 0)
		l_price_8 = Ask + tp_factor * Point;
		
	OrderSend(Symbol(), OP_BUY, GetMoneyManagement(), Ask, Slippage, l_price_0, l_price_8, "", MagicNumber);
	
	prtAlert("GRIFFIN Ultimate buy @" + DoubleToStr(Ask, 4));
}

void Eagle::OpenSellOrder() {
	double l_price_0 = 0;
	double l_price_8 = 0;
	l_price_0 = 0;
	
	if (sl_factor > 0)
		l_price_0 = Bid + sl_factor * Point;
		
	l_price_8 = 0;
	
	if (tp_factor > 0)
		l_price_8 = Bid - tp_factor * Point;
		
	OrderSend(Symbol(), OP_SELL, GetMoneyManagement(), Bid, Slippage, l_price_0, l_price_8, "", MagicNumber);
	
	prtAlert("GRIFFIN Ultimate sell  @" + DoubleToStr(Bid, 4));
}

int Eagle::openPositions() {
	int l_count_0 = 0;
	
	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == MagicNumber) {
			if (OrderSymbol() == Symbol()) {
				if (OrderType() == OP_BUY)
					l_count_0++;
					
				if (OrderType() == OP_SELL)
					l_count_0++;
			}
		}
	}
	
	return (l_count_0);
}

void Eagle::prtAlert(String as_0) {
	Print(Symbol() + " - " + as_0);
}

void Eagle::CloseOrder(int a_ticket_0, double a_lots_4, double a_price_12) {
	int l_error_20;
	int l_count_24 = 0;
	
	while (l_count_24 < 3) {
		if (OrderClose(a_ticket_0, a_lots_4, a_price_12, Slippage))
			l_count_24 = 3;
		else {
			Print("Error closing order");
			l_count_24++;
		}
	}
}

void Eagle::TrailingPositions() {
	for (int l_pos_0 = 0; l_pos_0 < OrdersTotal(); l_pos_0++) {
		if (OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY) {
					if (!use_orders || Bid - OrderOpenPrice() > tp_limit * Point)
						if (OrderStopLoss() < Bid - (tp_limit + sl_limit - 1) * Point)
							ModifyStopLoss(Bid - tp_limit * Point);
				}
				
				if (OrderType() == OP_SELL) {
					if (!use_orders || OrderOpenPrice() - Ask > tp_limit * Point)
						if (OrderStopLoss() > Ask + (tp_limit + sl_limit - 1) * Point || OrderStopLoss() == 0.0)
							ModifyStopLoss(Ask + tp_limit * Point);
				}
			}
		}
	}
}

void Eagle::ModifyStopLoss(double a_price_0) {
	double l_ord_open_price_12 = OrderOpenPrice();
	double l_ord_takeprofit_20 = OrderTakeProfit();
	OrderModify(OrderTicket(), l_ord_open_price_12, a_price_0, l_ord_takeprofit_20);
}

int Eagle::HandleOpenPositions1() {
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol()) {
			if (OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY)
					if (CheckExitCondition("BUY"))
						CloseOrder(OrderTicket(), OrderLots(), Bid);
			}
		}
	}
	
	if (OrderSelect(0, SELECT_BY_POS, MODE_TRADES))
		if (OrderType() == OP_SELL)
			if (CheckExitCondition("SELL"))
				CloseOrder(OrderTicket(), OrderLots(), Ask);
			
	return 0;
}

double Eagle::GetMoneyManagement() {
	double ld_ret_0 = Lots;
	int l_hist_total_8 = OrdersHistoryTotal();
	int order_count = 0;
	ld_ret_0 = NormalizeDouble(AccountFreeMargin() * MaximumRisk / 1000.0, 1);
	
	if (use_mm_lots == true)
		ld_ret_0 = NormalizeDouble(AccountBalance() / mm_lots_divider * MaximumRisk / 1000.0, 1);
		
	if (lot_divider > 0.0) {
		for (int l_pos_16 = l_hist_total_8 - 1; l_pos_16 >= 0; l_pos_16--) {
			if (OrderSelect(l_pos_16, SELECT_BY_POS, MODE_HISTORY) == false) {
				Print("Error in history data");
				break;
			}
			
			if (!(OrderSymbol() != Symbol() || OrderType() > OP_SELL) && !(OrderMagicNumber() != MagicNumber)) {
				if (OrderProfit() > 0.0)
					break;
					
				if (OrderProfit() < 0.0 && OrderMagicNumber() == MagicNumber)
					order_count++;
			}
		}
		
		if (order_count > 1)
			ld_ret_0 = NormalizeDouble(ld_ret_0 - ld_ret_0 * order_count / lot_divider, 1);
	}
	
	if (ld_ret_0 < 0.1)
		ld_ret_0 = 0.1;
		
	if (Moneymanagement == false)
		ld_ret_0 = Lots;
		
	if (micro_lots == true)
		ld_ret_0 = Lots / 10.0;
		
	if (ld_ret_0 > MaxTradesize)
		ld_ret_0 = MaxTradesize;
		
	return (ld_ret_0);
}

}
