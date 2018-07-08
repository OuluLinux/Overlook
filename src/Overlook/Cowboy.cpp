#include "Overlook.h"

namespace Overlook {

Cowboy::Cowboy() {
	
}

void Cowboy::InitEA() {
	if (StartWorkTimeHour < 0 || StartWorkTimeHour > 23)
		StartWorkTimeHour = 0;
		
	if (StartWorkTimeMin < 0 || StartWorkTimeMin > 59)
		StartWorkTimeMin = 0;
		
	if (EndWorkTimeHour < 0 || EndWorkTimeHour > 23)
		EndWorkTimeHour = 0;
		
	if (EndWorkTimeMin < 0 || EndWorkTimeMin > 59)
		EndWorkTimeMin = 0;
	
	AddSubCore<MovingAverage>()
		.Set("period", ma_period0);
	
	AddSubCore<MovingAverage>()
		.Set("period", ma_period1);
	
	AddSubCore<MovingAverage>()
		.Set("period", ma_period2);
	
	AddSubCore<Fractals>()
		.Set("left_bars", frac_left_bars)
		.Set("right_bars", 0);
	
	AddSubCore<DeMarker>()
		.Set("period", aaa);
		
	AddSubCore<WilliamsPercentRange>()
		.Set("period", bbb);
		
	AddSubCore<MovingAverage>()
		.Set("period", PeriodMA);
	
}

void Cowboy::StartEA(int pos) {
	this->pos = pos;
	
	int li_8 = 0;

	if (!CheckParams())
		return;
		
	if (!PrepareIndicators())
		return;
		
	int li_4 = CalculateCurrentOrders();
	
	if (li_4 > 0) {
		for (int li_0 = li_4 - 1; li_0 >= 0; li_0--) {
			if (GetOrderByPos(li_0)) {
				if (OrderMagicNumber() == OrderMagic) {
					if (TradeSignalCloseOrder() || TradeSignalCloseOrderOnTime(li_0))
						CloseOrder();
					else {
						if (OrderTakeProfit() == 0.0) {
							if (OrderType() == OP_BUY)
								li_8 = (Bid - OrderOpenPrice()) / Point;
							else
								li_8 = (OrderOpenPrice() - Ask) / Point;
								
							if (li_8 >= TakeProfit && TakeProfit != 0.0) {
								CloseOrder();
								return;
							}
							
							if (TakeProfit > MarketInfo(Symbol(), MODE_STOPLEVEL))
								ModifyOrder();
						}
						
						if (TrailingStop > 0.0)
							TrailOrderStop();
					}
				}
			}
		}
		
		if (li_4 < MaxOrders)
			trueOpen();
				
		return;
	}
	
	trueOpen();
	
}

int Cowboy::CalculateCurrentOrders() {
	int li_ret_0 = 0;
	
	for (int l_pos_4 = 0; l_pos_4 < OrdersTotal(); l_pos_4++) {
		if (OrderSelect(l_pos_4, SELECT_BY_POS, MODE_TRADES))
			if (OrderMagicNumber() == OrderMagic && OrderSymbol() == Symbol())
				li_ret_0++;
	}
	
	return (li_ret_0);
}

int Cowboy::trueOpen() {
	int li_0 = TradeSignalOpenOrder();
	
	if (li_0 == 0)
		return (0);
		
	if (OneTrade != 0) {
		if (HaveTrade()) {
			if (WriteDebugLog)
				Print("Already have one trade inside this interval of time.");
				
			return (0);
		}
	}
	
	double ld_4 = CalcLotsVolume();
	
	if (ld_4 == 0.0 || !CheckAccount(DirectionOrderType(li_0), ld_4))
		return (0);
		
	int li_12 = OpenOrder(li_0, ld_4);
	
	return (0);
}

bool Cowboy::CheckParams() {
	if (Bars < 100) {
		Print("Bars less than 100");
		return (false);
	}
	
	if (TakeProfit < 10.0) {
		Print("TakeProfit is less than 10");
		return (false);
	}
	
	if (Lots == 0.0 && LotsRiskReductor < 1.0) {
		Print("LotsRiskReductor is less than 1");
		return (false);
	}
	
	return (true);
}

bool Cowboy::CheckAccount(int a_cmd_0, double ad_4) {
	bool li_ret_12 = true;
	double ld_16 = AccountFreeMarginCheck(Symbol(), a_cmd_0, ad_4);
	
	if (ld_16 <= 0)
		li_ret_12 = false;
	
	return (li_ret_12);
}

double Cowboy::CalcLotsVolume() {
	double ld_ret_0;
	double l_maxlot_8;
	double l_minlot_16;
	double l_lotstep_24;
	
	if (Lots > 0.0)
		ld_ret_0 = NormalizeLot(Lots, 0, "");
	else {
		l_maxlot_8 = MarketInfo(Symbol(), MODE_MAXLOT);
		l_minlot_16 = MarketInfo(Symbol(), MODE_MINLOT);
		l_lotstep_24 = MarketInfo(Symbol(), MODE_LOTSTEP);
		ld_ret_0 = NormalizeDouble(MathFloor(AccountFreeMargin() * LotsRiskReductor / 100.0 / (MarketInfo(Symbol(), MODE_MARGINREQUIRED) * l_lotstep_24)) * l_lotstep_24, 2);
		
		if (ld_ret_0 < l_minlot_16)
			ld_ret_0 = l_minlot_16;
			
		if (ld_ret_0 > l_maxlot_8)
			ld_ret_0 = l_maxlot_8;
	}
	
	if (ld_ret_0 > MaxLots)
		ld_ret_0 = MaxLots;
		
	return (ld_ret_0);
}

bool Cowboy::PrepareIndicators() {
	double l_ifractals_20;
	g_icustom_220 = At(0).GetBuffer(0).Get(pos);
	
	if (IsError("Alligator Jaw"))
		return (false);
		
	g_icustom_228 = At(1).GetBuffer(0).Get(pos);
	
	if (IsError("Alligator Teeth"))
		return (false);
		
	g_icustom_236 = At(2).GetBuffer(0).Get(pos);
	
	if (IsError("Alligator Lips"))
		return (false);
		
	int li_8 = 3;
	
	g_ifractals_260 = 0;
	
	g_ifractals_268 = 0;
	
	for (int li_4 = 0; li_4 <= li_8; li_4++) {
		l_ifractals_20 = At(3).GetBuffer(2).Get(pos - li_4);
		
		if (IsError("Lower Fractals"))
			return (false);
			
		if (l_ifractals_20 != 0.0)
			g_ifractals_260 = l_ifractals_20;
			
		l_ifractals_20 = At(3).GetBuffer(3).Get(pos - li_4);
		
		if (IsError("Upper Fractals"))
			return (false);
			
		if (l_ifractals_20 != 0.0)
			g_ifractals_268 = l_ifractals_20;
	}
	
	li_8 = 0;
	
	gda_300.SetCount(li_8 + 1);
	gda_304.SetCount(li_8 + 1);
	
	for (int li_4 = 0; li_4 <= li_8; li_4++) {
		gda_300[li_4] = At(4).GetBuffer(2).Get(pos - li_4);
		
		if (IsError("DeMarker"))
			return (false);
			
		gda_304[li_4] = (-At(5).GetBuffer(0).Get(pos - li_4)) / 100.0;
		
		if (IsError("WPR"))
			return (false);
	}
	
	if (UseMAControl != 0) {
		g_ima_276 = At(6).GetBuffer(0).Get(pos - 1);
		g_ima_284 = At(6).GetBuffer(0).Get(pos - 0);
	}
	
	return (true);
}

int Cowboy::TradeSignalOpenOrder() {
	if (UseMAControl == 0)
		if (!IsGatorActiveUp())
			return (0);
			
	if (g_ima_276 < g_ima_284)
		if (!IsGatorActiveUp())
			return (0);
			
	if (g_ima_276 > g_ima_284)
		if (!IsGatorActiveDown())
			return (0);
			
	if (!IsGatorActiveUp() && !IsGatorActiveDown())
		return (0);
		
	if (WasWPROverBuy() || WasWPROverSell())
		return (0);
		
	if (IsFractalLower() && WasDemarkerHigh())
		return (1);
		
	if (IsFractalUpper() && WasDemarkerLow())
		return (-1);
		
	return (0);
}

int Cowboy::TradeSignalCloseOrder() {
	return (!IsOrderProfitable());
}

double Cowboy::NormalizeLot(double ad_0, bool ai_8, String a_symbol_12) {
	double ld_ret_20 = 0;
	double ld_28;
	
	if (a_symbol_12 == "" || a_symbol_12 == "0")
		a_symbol_12 = Symbol();
		
	double l_lotstep_36 = MarketInfo(a_symbol_12, MODE_LOTSTEP);
	
	double l_minlot_44 = MarketInfo(a_symbol_12, MODE_MINLOT);
	
	double l_maxlot_52 = MarketInfo(a_symbol_12, MODE_MAXLOT);
	
	if (l_minlot_44 == 0.0)
		l_minlot_44 = 0.1;
		
	if (l_maxlot_52 == 0.0)
		l_maxlot_52 = 100;
		
	if (l_lotstep_36 > 0.0)
		ld_28 = 1 / l_lotstep_36;
	else
		ld_28 = 1 / l_minlot_44;
		
	if (ai_8)
		ld_ret_20 = MathCeil(ad_0 * ld_28) / ld_28;
	else
		ld_ret_20 = MathFloor(ad_0 * ld_28) / ld_28;
		
	if (ld_ret_20 < l_minlot_44)
		ld_ret_20 = l_minlot_44;
		
	if (ld_ret_20 > l_maxlot_52)
		ld_ret_20 = l_maxlot_52;
		
	return (ld_ret_20);
}

int Cowboy::ModifyOrder() {
	int li_8 = OrderTypeDirection();
	Print("TakeProfit value is not set due to server connection error. Please modify the order and set it manually");
	double l_price_0 = OrderOpenPrice() + TakeProfit * Point * li_8;
	OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), l_price_0);
	return (0);
}

int Cowboy::TradeSignalCloseOrderOnTime(int ai_unused_0) {
	if (TimeMonth(OrderOpenTime()) != TimeMonth(TimeCurrent()))
		return (1);
		
	return (0);
}

bool Cowboy::IsGatorActiveUp() {
	return (g_icustom_236 - g_icustom_228 >= SpanGator * Point && g_icustom_228 - g_icustom_220 >= SpanGator * Point && g_icustom_236 - g_icustom_220 >= SpanGator * Point);
}

bool Cowboy::IsGatorActiveDown() {
	return (g_icustom_228 - g_icustom_236 >= SpanGator * Point && g_icustom_220 - g_icustom_228 >= SpanGator * Point && g_icustom_220 - g_icustom_236 >= SpanGator * Point);
}

int Cowboy::IsFractalLower() {
	return (g_ifractals_260 != 0.0);
}

int Cowboy::IsFractalUpper() {
	return (g_ifractals_268 != 0.0);
}

bool Cowboy::IsOrderProfitable() {
	return (true);
}

int Cowboy::WasDemarkerLow() {
	return (ArrayMinValue(gda_300) < 0.5);
}

int Cowboy::WasDemarkerHigh() {
	return (ArrayMaxValue(gda_300) > 0.5);
}

int Cowboy::WasWPROverBuy() {
	return (ArrayMinValue(gda_304) <= 0.25);
}

int Cowboy::WasWPROverSell() {
	return (ArrayMaxValue(gda_304) >= 0.75);
}

int Cowboy::OpenOrder(int ai_0, double a_lots_4, String a_comment_12) {
	int li_48;
	double l_price_28 = 0;
	double l_price_36 = 0;
	double l_price_20 = PriceOpen(ai_0);
	l_price_36 = l_price_20 + TakeProfit * Point * ai_0;
	
	if (StopLoss > 0.0)
		l_price_28 = PriceClose(ai_0) - StopLoss * Point * ai_0;
		
	int l_ticket_44 = OrderSend(Symbol(), DirectionOrderType(ai_0), a_lots_4, l_price_20, SlipPage, 0, 0, a_comment_12, OrderMagic);
	
	Sleep(1000);
	
	if (l_ticket_44 > 0) {
		li_48 = MarketInfo(Symbol(), MODE_STOPLEVEL);
		
		if (TakeProfit > li_48 || TakeProfit == 0.0)
			if (StopLoss > li_48 || StopLoss == 0.0)
				OrderModify(l_ticket_44, l_price_20, l_price_28, l_price_36);
	}
	
	g_datetime_308 = Now;
	
	return (0);
}

void Cowboy::CloseOrder() {
	Print("Closing!");
	OrderClose(OrderTicket(), OrderLots(), Bid, SlipPage);
}

bool Cowboy::GetOrderByPos(int a_pos_0) {
	return (OrderSelect(a_pos_0, SELECT_BY_POS, MODE_TRADES) && OrderType() <= OP_SELL && OrderSymbol() == Symbol());
}

void Cowboy::TrailOrderStop() {
	int li_0;
	double ld_4;
	double ld_12;
	double l_price_20;
	double ld_28;
	double ld_36;
	int li_44;
	
	if (OrderTicket() > 0) {
		li_0 = OrderTypeDirection();
		ld_4 = NormalizeDouble(TrailingStop * Point * li_0, Digits);
		ld_12 = NormalizeDouble(iif(li_0 > 0 || OrderStopLoss() != 0.0, OrderStopLoss(), 999999), Digits);
		l_price_20 = NormalizeDouble(PriceClose(li_0) - ld_4, Digits);
		ld_28 = NormalizeDouble(l_price_20 - OrderOpenPrice(), Digits);
		ld_36 = NormalizeDouble(l_price_20 - ld_12, Digits);
		
		if (ld_28 * li_0 > 0.0 && ld_36 * li_0 >= Point) {
			if (OrderType() == OP_BUY)
				li_44 = (Bid - l_price_20) / Point;
			else
				li_44 = (l_price_20 - Ask) / Point;
				
			if (li_44 > MarketInfo(Symbol(), MODE_STOPLEVEL)) {
				bool succ = OrderModify(OrderTicket(), OrderOpenPrice(), l_price_20, OrderTakeProfit());
				
			}
		}
	}
}

int Cowboy::OrderTypeDirection() {
	int li_ret_0 = 0;
	
	if (OrderType() == OP_BUY)
		li_ret_0 = 1;
		
	if (OrderType() == OP_SELL)
		li_ret_0 = -1;
		
	return (li_ret_0);
}

int Cowboy::DirectionOrderType(int ai_0) {
	return (iif(ai_0 > 0, 0, 1));
}

int Cowboy::ColorOpen(int ai_0) {
	return (iif(ai_0 > 0, 32768, 255));
}

double Cowboy::PriceOpen(int ai_0) {
	return (iif(ai_0 > 0, Ask, Bid));
}

double Cowboy::PriceClose(int ai_0) {
	return (iif(ai_0 > 0, Bid, Ask));
}

double Cowboy::iif(bool ai_0, double ad_4, double ad_12) {
	if (ai_0)
		return (ad_4);
		
	return (ad_12);
}

bool Cowboy::IsError(String as_0) {
	
	return false;
}

double Cowboy::ArrayMinValue(Vector<double>& ada_0) {
	double min = ada_0[0];
	for(int i = 1; i < ada_0.GetCount(); i++) {
		min = Upp::min(min, ada_0[i]);
	}
	return min;
}

double Cowboy::ArrayMaxValue(Vector<double>& ada_0) {
	double max = ada_0[0];
	for(int i = 1; i < ada_0.GetCount(); i++) {
		max = Upp::max(max, ada_0[i]);
	}
	return max;
}

bool Cowboy::IsTradeTime(int ai_0) {
	return (true);
}

bool Cowboy::HaveTrade() {
	Time l_datetime_28;
	
	if (StartWorkTimeHour == EndWorkTimeHour && StartWorkTimeMin == EndWorkTimeMin)
		return (false);
		
	Time l_datetime_0 = TimeCurrent();
	
	Time today = Now;
	today.hour = 0;
	today.minute = 0;
	today.second = 0;
	Time li_4 = today;
	
	int l_hour_8 = TimeHour(l_datetime_0);
	
	bool li_12 = false;
	
	if (StartWorkTimeHour > EndWorkTimeHour) {
		if (l_hour_8 < StartWorkTimeHour)
			li_12 = true;
	}
	
	else {
		if (StartWorkTimeHour == EndWorkTimeHour) {
			if (StartWorkTimeMin > EndWorkTimeMin) {
				if (l_hour_8 < StartWorkTimeHour) {
					if (l_hour_8 < StartWorkTimeHour)
						li_12 = true;
				}
				
				else {
					if (l_hour_8 == StartWorkTimeHour)
						if (TimeMinute(l_datetime_0) < EndWorkTimeMin)
							li_12 = true;
				}
			}
		}
	}
	
	if (li_12)
		li_4 -= 86400;
		
	li_4 += 3600 * StartWorkTimeHour + 60 * StartWorkTimeMin;
	
	int li_16 = OrdersTotal() - 1;
	
	Time l_datetime_20(1970,1,1);
	
	for (int l_pos_24 = li_16; l_pos_24 >= 0; l_pos_24--) {
		if (OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderMagicNumber() == OrderMagic) {
				if (OrderSymbol() == Symbol()) {
					l_datetime_28 = OrderOpenTime();
					
					if (l_datetime_20 < l_datetime_28)
						l_datetime_20 = l_datetime_28;
				}
			}
		}
	}
	
	if (l_datetime_20 >= li_4)
		return (true);
		
	li_16 = OrdersHistoryTotal() - 1;
	
	l_datetime_20 = Time(1970,1,1);
	
	for (int l_pos_24 = li_16; l_pos_24 >= 0; l_pos_24--) {
		if (OrderSelect(l_pos_24, SELECT_BY_POS, MODE_HISTORY)) {
			if (OrderMagicNumber() == OrderMagic) {
				if (OrderSymbol() == Symbol()) {
					l_datetime_28 = OrderOpenTime();
					
					if (l_datetime_20 < l_datetime_28)
						l_datetime_20 = l_datetime_28;
				}
			}
		}
	}
	
	if (l_datetime_20 >= li_4)
		return (true);
		
	return (false);
}

}
