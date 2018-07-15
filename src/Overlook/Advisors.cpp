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





















Explorer::Explorer() {
	
}

void Explorer::InitEA() {
	
	AddSubCore<ParabolicSAR>()
		.Set("step", stepslow)
		.Set("maximum", maximumslow);
	
	AddSubCore<ParabolicSAR>()
		.Set("step", stepfast)
		.Set("maximum", maximumfast);
	
	AddSubCore<Channel>()
		.Set("period", barsearch);
}

void Explorer::StartEA(int pos) {
	this->pos = pos;
	
	if (!pos)
		return;
	
	if (g_bars_184 != Bars && (GetTimeControl(starttime, stoptime) != 1 && timecontrol == 1) || timecontrol == 0) {
		g_bars_184 = Bars;
		ret0 = ScalpParabolicPattern(pos);
	}
	
	if (TrailingStop > 0)
		ret0 = GetTrailingStop();
		
	if (BBUSize > 0)
		ret0 = BBU();
	
}

int Explorer::ScalpParabolicPattern(int pos) {
	int l_ticket_0;
	double ld_4;
	double l_price_12;
	double l_price_20;
	double l_high_28;
	double ld_36;
	double ld_44;
	double ld_52;
	double l_isar_60 = At(0).GetBuffer(0).Get(pos);
	double l_isar_68 = At(1).GetBuffer(0).Get(pos);
	
	if (l_isar_68 > Bid && l_isar_60 < Bid)
		ret1 = true;
		
	if (l_isar_60 > Bid)
		ret1 = false;
		
	if (l_isar_68 < Bid && l_isar_60 > Bid)
		ret2 = true;
		
	if (l_isar_60 < Bid)
		ret2 = false;
		
	bool l_bool_84 = RiskManagement;
	
	if (l_bool_84) {
		if (RiskPercent < 0.1 || RiskPercent > 100.0) {
			Comment("Invalid Risk Value.");
			return 0;
		}
		
		Lots = MathFloor(100.0 * (AccountFreeMargin() * AccountLeverage() * RiskPercent * Point) / (Ask * MarketInfo(Symbol(), MODE_LOTSIZE) * MarketInfo(Symbol(), MODE_MINLOT))) * MarketInfo(Symbol(), MODE_MINLOT);
	}
	
	if (l_bool_84 == false) {
	}
	
	if (l_isar_60 < Bid && l_isar_68 < Bid && ret1 == true) {
		ret1 = false;
		ld_36 = MaximumMinimum(0, barsearch);
		l_high_28 = GetInputBuffer(0, 2).Get(pos - 1);
		ld_44 = GetFiboUr(l_high_28, ld_36, Ur1 / 100.0);
		ld_52 = GetFiboUr(l_high_28, ld_36, Ur2 / 100.0);
		ld_4 = ld_44;
		l_price_12 = ld_36 - otstup * Point;
		l_price_20 = ld_52;
		
		if (Ask - ld_4 < 5.0 * Point || Ask - l_price_12 < 5.0 * Point || l_price_20 - Ask < 5.0 * Point)
			return 0;
			
		l_ticket_0 = OrderSend(Symbol(), OP_BUYLIMIT, Lots, NormalizeDouble(ld_4, Digits), 3, l_price_12, l_price_20, "", 0);
		
		if (l_ticket_0 < 0) {
			return (-1);
		}
		
		gi_unused_176 = 0;
	}
	
	if (l_isar_60 > Bid && l_isar_68 > Bid && ret2 == true) {
		ret2 = false;
		l_high_28 = MaximumMinimum(1, barsearch);
		ld_36 = GetInputBuffer(0, 1).Get(pos-1);
		ld_44 = GetFiboUr(ld_36, l_high_28, Ur1 / 100.0);
		ld_52 = GetFiboUr(ld_36, l_high_28, Ur2 / 100.0);
		ld_4 = ld_44;
		l_price_12 = l_high_28 + otstup * Point;
		l_price_20 = ld_52;
		
		if (ld_4 - Ask < 5.0 * Point || l_price_12 - Ask < 5.0 * Point || Ask - l_price_20 < 5.0 * Point)
			return 0;
			
		l_ticket_0 = OrderSend(Symbol(), OP_SELLLIMIT, Lots, NormalizeDouble(ld_4, Digits), 3, l_price_12, l_price_20, "", 0);
		
		if (l_ticket_0 < 0) {
			return (-1);
		}
		
		gi_unused_180 = 0;
	}
	
	if (l_isar_68 > Bid && ChLimitOrder(1) > 0)
		l_ticket_0 = deletelimitorder(1);
		
	if (l_isar_68 < Bid && ChLimitOrder(0) > 0)
		l_ticket_0 = deletelimitorder(0);
		
	return 0;
}

int Explorer::deletelimitorder(int ai_0) {
	int l_ord_delete_8;
	
	for (int li_4 = 1; li_4 <= OrdersTotal(); li_4++) {
		if (OrderSelect(li_4 - 1, SELECT_BY_POS) == true) {
			if (OrderType() == OP_BUYLIMIT && OrderSymbol() == Symbol() && ai_0 == 1)
				l_ord_delete_8 = OrderDelete(OrderTicket());
				
			else if (OrderType() == OP_SELLLIMIT && OrderSymbol() == Symbol() && ai_0 == 0)
				l_ord_delete_8 = OrderDelete(OrderTicket());
		}
	}
	
	return (l_ord_delete_8);
}

int Explorer::ChLimitOrder(int ai_0) {
	for (int li_4 = 1; li_4 <= OrdersTotal(); li_4++) {
		if (OrderSelect(li_4 - 1, SELECT_BY_POS) == true) {
			if (OrderType() == OP_BUYLIMIT && OrderSymbol() == Symbol() && ai_0 == 1)
				return (1);
				
			else if (OrderType() == OP_SELLLIMIT && OrderSymbol() == Symbol() && ai_0 == 0)
				return (1);
		}
	}
	
	return 0;
}

double Explorer::MaximumMinimum(int ai_0, int ai_4) {
	double ld_ret_16;
	int li_8 = 0;
	bool li_12 = false;
	
	Core& c = At(2);
	
	if (ai_0 == 0) {
		while (li_12 == false) {
			int p = pos - ai_4 - li_8;
			if (p < 0) break;
			ld_ret_16 = c.GetBuffer(0).Get(pos - li_8);
			double d = c.GetBuffer(0).Get(p);
			
			if (ld_ret_16 > d) {
				ld_ret_16 = d;
				li_8 += ai_4;
			}
			
			else {
				li_12 = true;
				return (ld_ret_16);
			}
		}
	}
	
	if (ai_0 == 1) {
		while (li_12 == false) {
			int p = pos - ai_4 - li_8;
			if (p < 0) break;
			ld_ret_16 = c.GetBuffer(1).Get(pos - li_8);
			double d = c.GetBuffer(1).Get(p);
			
			if (ld_ret_16 < d) {
				ld_ret_16 = d;
				li_8 += ai_4;
			}
			
			else {
				li_12 = true;
				return (ld_ret_16);
			}
		}
	}
	
	return 0;
}

double Explorer::GetFiboUr(double ad_0, double ad_8, double ad_16) {
	double ld_28;
	int l_digits_24 = MarketInfo(Symbol(), MODE_DIGITS);
	ld_28 = NormalizeDouble(ad_8 + (ad_0 - ad_8) * ad_16, l_digits_24);
	return (ld_28);
}

int Explorer::GetTrailingStop() {
	bool l_bool_4;
	
	for (int li_0 = 1; li_0 <= OrdersTotal(); li_0++) {
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (TrailingStop > 0 && OrderType() == OP_BUY && OrderSymbol() == Symbol()) {
				if (Bid - OrderOpenPrice() >= TrailingStop * Point && TrailingStop > 0 && Bid - Point * TrailingStop > OrderStopLoss()) {
					if (Bid - Point * TrailingStop - OrderStopLoss() >= TrailingShag * Point) {
						Print("ÒÐÅÉËÈÌ");
						l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), Bid - Point * TrailingStop, OrderTakeProfit());
						
						if (l_bool_4 == false)
							return (-1);
					}
				}
			}
		}
		
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (OrderType() == OP_SELL && OrderSymbol() == Symbol()) {
				if (OrderOpenPrice() - Ask >= TrailingStop * Point && TrailingStop > 0 && OrderStopLoss() > Ask + TrailingStop * Point) {
					if (OrderStopLoss() - (Ask + TrailingStop * Point) > TrailingShag * Point) {
						Print("ÒÐÅÉËÈÌ");
						l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), Ask + TrailingStop * Point, OrderTakeProfit());
						
						if (l_bool_4 == false)
							return (-1);
					}
				}
			}
		}
	}
	
	return 0;
}

int Explorer::BBU() {
	bool l_bool_4;
	
	for (int li_0 = 1; li_0 <= OrdersTotal(); li_0++) {
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (BBUSize > 0 && OrderType() == OP_BUY && OrderSymbol() == Symbol() && OrderStopLoss() < OrderOpenPrice()) {
				if (Bid - OrderOpenPrice() >= BBUSize * Point && BBUSize > 0) {
					Print("ÏÎÐÀ Â ÁåçóÁûòîê");
					l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + 1.0 * Point, OrderTakeProfit());
					
					if (l_bool_4 == false)
						return (-1);
				}
			}
		}
		
		if (OrderSelect(li_0 - 1, SELECT_BY_POS) == true) {
			if (BBUSize > 0 && OrderType() == OP_SELL && OrderSymbol() == Symbol() && OrderStopLoss() > OrderOpenPrice() || OrderStopLoss() == 0.0) {
				if (OrderOpenPrice() - Ask >= BBUSize * Point && BBUSize > 0) {
					Print("ÏÎÐÀ Â ÁåçóÁûòîê");
					l_bool_4 = OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - 1.0 * Point, OrderTakeProfit());
					
					if (l_bool_4 == false)
						return (-1);
				}
			}
		}
	}
	
	return 0;
}

int Explorer::GetTimeControl(int ai_0, int ai_4) {
	if (Hour() >= ai_0 && Hour() <= ai_4)
		return 0;
		
	return (1);
}



















ModestTry::ModestTry() {
	
}

void ModestTry::InitEA() {
	gi_124 = TrailingStop + TrailingKeep;
	
	if (TrailingStep < 1)
		TrailingStep = 1;
		
	TrailingStep--;
	
	g_datetime_132 = Time(1970,1,1);
	g_datetime_136 = Time(1970,1,1);
	g_datetime_140 = Time(1970,1,1);
	g_datetime_144 = Time(1970,1,1);
	g_datetime_148 = Time(1970,1,1);
	g_datetime_152 = Time(1970,1,1);
	g_datetime_156 = Time(1970,1,1);
	g_datetime_160 = Time(1970,1,1);
	g_datetime_164 = Time(1970,1,1);
}

void ModestTry::StartEA(int pos) {
	if (pos < 1)
		return;
	
	double ld_76;
	double ld_84;
	double ld_92;
	double ld_100;
	double l_ihigh_0 = GetInputBuffer(0, 2).Get(pos-1);
	double l_ilow_8 = GetInputBuffer(0, 1).Get(pos-1);
	Time l_datetime_16 = Now;
	int l_count_20 = 0;
	int l_count_24 = 0;
	int l_count_28 = 0;
	int l_count_32 = 0;
	int l_count_36 = 0;
	int l_count_40 = 0;
	int li_unused_44 = 0;
	int li_unused_48 = 0;
	int l_count_52 = 0;
	int l_count_56 = 0;
	
	for (int l_pos_60 = 0; l_pos_60 < OrdersTotal(); l_pos_60++) {
		if (OrderSelect(l_pos_60, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderMagicNumber() == Magic) {
				if (OrderSymbol() == Symbol()) {
					if (OrderOpenTime() < l_datetime_16) {
						switch (OrderType()) {
						
						case OP_BUYSTOP:
							fDeletePendig(OrderTicket());
							break;
							
						case OP_SELLSTOP:
							fDeletePendig(OrderTicket());
							break;
							
						case OP_BUY:
							l_count_52++;
							break;
							
						case OP_SELL:
							l_count_56++;
						}
					}
				}
			}
		}
	}
	
	if (l_count_52 + l_count_56 < MaxOrdersCount || MaxOrdersCount == -1) {
		RefreshRates();
		
		for (int l_pos_60 = 0; l_pos_60 < OrdersTotal(); l_pos_60++) {
			if (OrderSelect(l_pos_60, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderMagicNumber() == Magic) {
					if (OrderSymbol() == Symbol()) {
						switch (OrderType()) {
						
						case OP_BUY:
							l_count_20++;
							break;
							
						case OP_SELL:
							l_count_24++;
							break;
							
						case OP_BUYSTOP:
							l_count_28++;
							break;
							
						case OP_SELLSTOP:
							l_count_32++;
						}
						
						if (OrderOpenTime() >= l_datetime_16) {
							switch (OrderType()) {
							
							case OP_BUY:
								l_count_36++;
								break;
								
							case OP_SELL:
								l_count_40++;
								break;
								
							case OP_BUYSTOP:
								l_count_36++;
								break;
								
							case OP_SELLSTOP:
								l_count_40++;
							}
						}
					}
				}
			}
			
			else
				return;
		}
		
		if (l_count_36 == 0) {
			for (int l_pos_60 = OrdersHistoryTotal() - 1; l_pos_60 >= 0; l_pos_60--) {
				if (OrderSelect(l_pos_60, SELECT_BY_POS, MODE_HISTORY)) {
					if (OrderMagicNumber() != Magic)
						continue;
						
					if (OrderSymbol() != Symbol())
						continue;
						
					if (OrderOpenTime() < l_datetime_16)
						continue;
						
					if (OrderType() == OP_BUYSTOP) {
						l_count_36++;
						break;
					}
					
					if (OrderType() != OP_BUY)
						continue;
						
					l_count_36++;
					
					break;
				}
				
				return;
			}
		}
		
		if (l_count_40 == 0) {
			for (int l_pos_60 = OrdersHistoryTotal() - 1; l_pos_60 >= 0; l_pos_60--) {
				if (OrderSelect(l_pos_60, SELECT_BY_POS, MODE_HISTORY)) {
					if (OrderMagicNumber() != Magic)
						continue;
						
					if (OrderSymbol() != Symbol())
						continue;
						
					if (OrderOpenTime() < l_datetime_16)
						continue;
						
					if (OrderType() == OP_SELLSTOP) {
						l_count_40++;
						break;
					}
					
					if (OrderType() != OP_SELL)
						continue;
						
					l_count_40++;
					
					break;
				}
				
				return;
			}
		}
		
		if (TimeCurrent() - l_datetime_16 <= 60 * TryMinutes) {
			if (l_count_36 == 0) {
				ld_76 = ND(l_ihigh_0 + (Ask - Bid) + Point * AddToLevel);
				ld_84 = ND(Ask + Point * (MarketInfo(Symbol(), MODE_STOPLEVEL) + gi_128));
				ld_76 = MathMax(ld_76, ld_84);
				fOrderSetBuyStop(ld_76);
			}
			
			if (l_count_40 == 0) {
				ld_92 = ND(l_ihigh_0 + (Ask - Bid) + Point * AddToLevel);
				ld_100 = ND(Bid - Point * (MarketInfo(Symbol(), MODE_STOPLEVEL) + gi_128));
				ld_92 = MathMin(ld_92, ld_100);
				fOrderSetSellStop(ld_92);
			}
		}
	}
	
	if (TrailingStop > 0)
		fTrailingWithStart();
		
	return;
}

int ModestTry::fDeletePendig(int a_ticket_0) {
	if (!OrderDelete(a_ticket_0)) {
		return (-1);
	}

	
	return (0);
}

double ModestTry::ND(double ad_0) {
	return (NormalizeDouble(ad_0, Digits));
}

int ModestTry::fOrderSetBuyStop(double ad_0, int a_datetime_8) {
	double l_lots_28;
	double l_price_36;
	double l_price_44;
	int l_ticket_52;
	RefreshRates();
	double l_price_12 = ND(ad_0);
	double ld_20 = ND(Ask + Point * MarketInfo(Symbol(), MODE_STOPLEVEL));
	
	if (l_price_12 >= ld_20) {
		l_lots_28 = fGetLotsSimple(OP_BUY);
		
		if (l_lots_28 > 0.0) {
			if (!IsTradeContextBusy()) {
				l_price_36 = ND(l_price_12 - Point * StopLoss);
				
				if (StopLoss == 0)
					l_price_36 = 0;
					
				l_price_44 = ND(l_price_12 + Point * TakeProfit);
				
				if (TakeProfit == 0)
					l_price_44 = 0;
					
				l_ticket_52 = OrderSend(Symbol(), OP_BUYSTOP, l_lots_28, l_price_12, 0, l_price_36, l_price_44, 0, Magic, a_datetime_8);
				
				if (l_ticket_52 > 0)
					return (l_ticket_52);
					
				Print("Error set BUYSTOP. ");
				
				return (-1);
			}
			
			if (TimeCurrent() > g_datetime_136 + 20) {
				g_datetime_136 = TimeCurrent();
				Print("Need set BUYSTOP. Trade Context Busy");
			}
			
			return (-2);
		}
		
		if (TimeCurrent() > g_datetime_140 + 20) {
			g_datetime_140 = TimeCurrent();
			
			if (l_lots_28 == -1.0)
				Print("Need set BUYSTOP. No money");
				
			if (l_lots_28 == -2.0)
				Print("Need set BUYSTOP. Wrong lots size");
		}
		
		return (-3);
	}
	
	if (TimeCurrent() > g_datetime_144 + 20) {
		g_datetime_144 = TimeCurrent();
		Print("Need set BUYSTOP. Wrong price level ");
	}
	
	return (-4);
}

int ModestTry::fOrderSetSellStop(double ad_0, int a_datetime_8) {
	double l_lots_28;
	double l_price_36;
	double l_price_44;
	int l_ticket_52;
	RefreshRates();
	double l_price_12 = ND(ad_0);
	double ld_20 = ND(Bid - Point * MarketInfo(Symbol(), MODE_STOPLEVEL));
	
	if (l_price_12 <= ld_20) {
		l_lots_28 = fGetLotsSimple(OP_SELL);
		
		if (l_lots_28 > 0.0) {
			if (!IsTradeContextBusy()) {
				l_price_36 = ND(l_price_12 + Point * StopLoss);
				
				if (StopLoss == 0)
					l_price_36 = 0;
					
				l_price_44 = ND(l_price_12 - Point * TakeProfit);
				
				if (TakeProfit == 0)
					l_price_44 = 0;
					
				l_ticket_52 = OrderSend(Symbol(), OP_SELLSTOP, l_lots_28, l_price_12, 0, l_price_36, l_price_44, 0, Magic, a_datetime_8);
				
				if (l_ticket_52 > 0)
					return (l_ticket_52);
					
				Print("Error set SELLSTOP. ");
				
				return (-1);
			}
			
			if (TimeCurrent() > g_datetime_148 + 20) {
				g_datetime_148 = TimeCurrent();
				Print("Need set SELLSTOP. Trade Context Busy");
			}
			
			return (-2);
		}
		
		if (TimeCurrent() > g_datetime_152 + 20) {
			g_datetime_152 = TimeCurrent();
			
			if (l_lots_28 == -1.0)
				Print("Need set SELLSTOP. No money");
				
			if (l_lots_28 == -2.0)
				Print("Need set SELLSTOP. Wrong lots size");
		}
		
		return (-3);
	}
	
	if (TimeCurrent() > g_datetime_156 + 20) {
		g_datetime_156 = TimeCurrent();
		Print("Need set SELLSTOP. Wrong price level");
	}
	
	return (-4);
}

double ModestTry::fGetLotsSimple(int a_cmd_0) {
	if (AccountFreeMarginCheck(Symbol(), a_cmd_0, Lots) <= 0.0)
		return (-1);
			
	return (Lots);
}

String ModestTry::fMyErDesc(int ai_0) {
	String ls_4 = "Err Num: " + IntStr(ai_0) + " - ";
	
	switch (ai_0) {
	
	case 0:
		return (ls_4 + "NO ERROR");
		
	case 1:
		return (ls_4 + "NO RESULT");
		
	case 2:
		return (ls_4 + "COMMON ERROR");
		
	case 3:
		return (ls_4 + "INVALID TRADE PARAMETERS");
		
	case 4:
		return (ls_4 + "SERVER BUSY");
		
	case 5:
		return (ls_4 + "OLD VERSION");
		
	case 6:
		return (ls_4 + "NO CONNECTION");
		
	case 7:
		return (ls_4 + "NOT ENOUGH RIGHTS");
		
	case 8:
		return (ls_4 + "TOO FREQUENT REQUESTS");
		
	case 9:
		return (ls_4 + "MALFUNCTIONAL TRADE");
		
	case 64:
		return (ls_4 + "ACCOUNT DISABLED");
		
	case 65:
		return (ls_4 + "INVALID ACCOUNT");
		
	case 128:
		return (ls_4 + "TRADE TIMEOUT");
		
	case 129:
		return (ls_4 + "INVALID PRICE");
		
	case 130:
		return (ls_4 + "INVALID STOPS");
		
	case 131:
		return (ls_4 + "INVALID TRADE VOLUME");
		
	case 132:
		return (ls_4 + "MARKET CLOSED");
		
	case 133:
		return (ls_4 + "TRADE DISABLED");
		
	case 134:
		return (ls_4 + "NOT ENOUGH MONEY");
		
	case 135:
		return (ls_4 + "PRICE CHANGED");
		
	case 136:
		return (ls_4 + "OFF QUOTES");
		
	case 137:
		return (ls_4 + "BROKER BUSY");
		
	case 138:
		return (ls_4 + "REQUOTE");
		
	case 139:
		return (ls_4 + "ORDER LOCKED");
		
	case 140:
		return (ls_4 + "LONG POSITIONS ONLY ALLOWED");
		
	case 141:
		return (ls_4 + "TOO MANY REQUESTS");
		
	case 145:
		return (ls_4 + "TRADE MODIFY DENIED");
		
	case 146:
		return (ls_4 + "TRADE CONTEXT BUSY");
		
	case 147:
		return (ls_4 + "TRADE EXPIRATION DENIED");
		
	case 148:
		return (ls_4 + "TRADE TOO MANY ORDERS");
		
	case 4000:
		return (ls_4 + "NO MQLERROR");
		
	case 4001:
		return (ls_4 + "WRONG FUNCTION POINTER");
		
	case 4002:
		return (ls_4 + "ARRAY INDEX OUT OF RANGE");
		
	case 4003:
		return (ls_4 + "NO MEMORY FOR FUNCTION CALL STACK");
		
	case 4004:
		return (ls_4 + "RECURSIVE STACK OVERFLOW");
		
	case 4005:
		return (ls_4 + "NOT ENOUGH STACK FOR PARAMETER");
		
	case 4006:
		return (ls_4 + "NO MEMORY FOR PARAMETER STRING");
		
	case 4007:
		return (ls_4 + "NO MEMORY FOR TEMP STRING");
		
	case 4008:
		return (ls_4 + "NOT INITIALIZED STRING");
		
	case 4009:
		return (ls_4 + "NOT INITIALIZED ARRAYSTRING");
		
	case 4010:
		return (ls_4 + "NO MEMORY FOR ARRAYSTRING");
		
	case 4011:
		return (ls_4 + "TOO LONG STRING");
		
	case 4012:
		return (ls_4 + "REMAINDER FROM ZERO DIVIDE");
		
	case 4013:
		return (ls_4 + "ZERO DIVIDE");
		
	case 4014:
		return (ls_4 + "UNKNOWN COMMAND");
		
	case 4015:
		return (ls_4 + "WRONG JUMP");
		
	case 4016:
		return (ls_4 + "NOT INITIALIZED ARRAY");
		
	case 4017:
		return (ls_4 + "DLL CALLS NOT ALLOWED");
		
	case 4018:
		return (ls_4 + "CANNOT LOAD LIBRARY");
		
	case 4019:
		return (ls_4 + "CANNOT CALL FUNCTION");
		
	case 4020:
		return (ls_4 + "EXTERNAL EXPERT CALLS NOT ALLOWED");
		
	case 4021:
		return (ls_4 + "NOT ENOUGH MEMORY FOR RETURNED STRING");
		
	case 4022:
		return (ls_4 + "SYSTEM BUSY");
		
	case 4050:
		return (ls_4 + "INVALID FUNCTION PARAMETERS COUNT");
		
	case 4051:
		return (ls_4 + "INVALID FUNCTION PARAMETER VALUE");
		
	case 4052:
		return (ls_4 + "STRING FUNCTION INTERNAL ERROR");
		
	case 4053:
		return (ls_4 + "SOME ARRAY ERROR");
		
	case 4054:
		return (ls_4 + "INCORRECT SERIES ARRAY USING");
		
	case 4055:
		return (ls_4 + "CUSTOM INDICATOR ERROR");
		
	case 4056:
		return (ls_4 + "INCOMPATIBLE ARRAYS");
		
	case 4057:
		return (ls_4 + "GLOBAL VARIABLES PROCESSING ERROR");
		
	case 4058:
		return (ls_4 + "GLOBAL VARIABLE NOT FOUND");
		
	case 4059:
		return (ls_4 + "FUNCTION NOT ALLOWED IN TESTING MODE");
		
	case 4060:
		return (ls_4 + "FUNCTION NOT CONFIRMED");
		
	case 4061:
		return (ls_4 + "SEND MAIL ERROR");
		
	case 4062:
		return (ls_4 + "STRING PARAMETER EXPECTED");
		
	case 4063:
		return (ls_4 + "INTEGER PARAMETER EXPECTED");
		
	case 4064:
		return (ls_4 + "DOUBLE PARAMETER EXPECTED");
		
	case 4065:
		return (ls_4 + "ARRAY AS PARAMETER EXPECTED");
		
	case 4066:
		return (ls_4 + "HISTORY WILL UPDATED");
		
	case 4067:
		return (ls_4 + "TRADE ERROR");
		
	case 4099:
		return (ls_4 + "END OF FILE");
		
	case 4100:
		return (ls_4 + "SOME FILE ERROR");
		
	case 4101:
		return (ls_4 + "WRONG FILE NAME");
		
	case 4102:
		return (ls_4 + "TOO MANY OPENED FILES");
		
	case 4103:
		return (ls_4 + "CANNOT OPEN FILE");
		
	case 4104:
		return (ls_4 + "INCOMPATIBLE ACCESS TO FILE");
		
	case 4105:
		return (ls_4 + "NO ORDER SELECTED");
		
	case 4106:
		return (ls_4 + "UNKNOWN SYMBOL");
		
	case 4107:
		return (ls_4 + "INVALID PRICE PARAM");
		
	case 4108:
		return (ls_4 + "INVALID TICKET");
		
	case 4109:
		return (ls_4 + "TRADE NOT ALLOWED");
		
	case 4110:
		return (ls_4 + "LONGS  NOT ALLOWED");
		
	case 4111:
		return (ls_4 + "SHORTS NOT ALLOWED");
		
	case 4200:
		return (ls_4 + "OBJECT ALREADY EXISTS");
		
	case 4201:
		return (ls_4 + "UNKNOWN OBJECT PROPERTY");
		
	case 4202:
		return (ls_4 + "OBJECT DOES NOT EXIST");
		
	case 4203:
		return (ls_4 + "UNKNOWN OBJECT TYPE");
		
	case 4204:
		return (ls_4 + "NO OBJECT NAME");
		
	case 4205:
		return (ls_4 + "OBJECT COORDINATES ERROR");
		
	case 4206:
		return (ls_4 + "NO SPECIFIED SUBWINDOW");
		
	case 4207:
		return (ls_4 + "SOME OBJECT ERROR");
	}
	
	return (ls_4 + "WRONG ERR NUM");
}

int ModestTry::fTrailingWithStart() {
	double l_price_4;
	int l_error_16;
	int li_ret_0 = 0;
	
	for (int l_pos_12 = 0; l_pos_12 < OrdersTotal(); l_pos_12++) {
		if (OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic) {
				if (OrderType() == OP_BUY) {
					RefreshRates();
					
					if (ND(Bid - OrderOpenPrice()) >= ND(Point * gi_124)) {
						l_price_4 = ND(Bid - Point * TrailingStop);
						
						if (ND(OrderStopLoss() + Point * TrailingStep) < l_price_4) {
							if (!IsTradeContextBusy()) {
								if (!OrderModify(OrderTicket(), OrderOpenPrice(), l_price_4, OrderTakeProfit())) {
									li_ret_0 = -1;
								}
							}
							
							else {
								li_ret_0 = -2;
								
								if (g_datetime_160 + 15 < TimeCurrent()) {
									g_datetime_160 = TimeCurrent();
								}
							}
						}
					}
				}
				
				if (OrderType() == OP_SELL) {
					RefreshRates();
					
					if (ND(OrderOpenPrice() - Ask) >= ND(Point * gi_124)) {
						l_price_4 = ND(Ask + Point * TrailingStop);
						
						if (!IsTradeContextBusy()) {
							if (ND(OrderStopLoss() - Point * TrailingStep) > l_price_4 || ND(OrderStopLoss()) == 0.0) {
								if (!OrderModify(OrderTicket(), OrderOpenPrice(), l_price_4, OrderTakeProfit())) {
									li_ret_0 = -1;
								}
							}
						}
						
						else {
							li_ret_0 = -2;
							
							if (g_datetime_164 + 15 < TimeCurrent()) {
								g_datetime_164 = TimeCurrent();
							}
						}
					}
				}
			}
		}
	}
	
	return (li_ret_0);
}


































// perevorot

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
