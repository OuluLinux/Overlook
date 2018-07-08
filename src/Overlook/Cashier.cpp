#include "Overlook.h"

namespace Overlook {

Cashier::Cashier() {
	
}

void Cashier::InitEA() {
	if (Point == 0.00001)
		g_point_400 = 0.0001;
	else {
		if (Point == 0.001)
			g_point_400 = 0.01;
		else
			g_point_400 = Point;
	}
	
	AddSubCore<MovingAverage>()
		.Set("period", g_period_312).Set("method", 1);
	
	AddSubCore<MovingAverage>()
		.Set("period", g_period_328);
	
	AddSubCore<MovingAverage>()
		.Set("period", g_period_320);
	
	AddSubCore<MovingAverage>()
		.Set("period", g_period_376);
	
	AddSubCore<StandardDeviation>()
		.Set("period", g_period_336);
	
	AddSubCore<StandardDeviation>()
		.Set("period", g_period_376);
	
	AddSubCore<WilliamsPercentRange>()
		.Set("period", g_period_344);
	
	AddSubCore<DeMarker>()
		.Set("period", g_period_344);
	
	AddSubCore<StochasticOscillator>()
		.Set("k_period", g_period_360)
		.Set("d_period", 6)
		.Set("slowing", 7);
	
	AddSubCore<StochasticOscillator>()
		.Set("k_period", g_period_368)
		.Set("d_period", 6)
		.Set("slowing", 7);
	
	AddSubCore<Channel>()
		.Set("period", g_period_376);
	
	AddSubCore<MovingAverage>()
		.Set("period", g_period_284);
	
	AddSubCore<StandardDeviation>()
		.Set("period", g_period_284);
	
	
}

void Cashier::StartEA(int pos) {
	this->pos = pos;
	
	if (pos < 6)
		return;
	
	int l_ticket_4;
	double l_price_12;
	double ld_148;
	bool li_252;
	bool li_256;
	bool li_260;
	bool li_264;
	bool li_268;
	bool li_272;
	bool li_276;
	bool li_280;
	int l_cmd_352;
	bool l_ord_close_356;
	double l_lots_364;
	double l_price_372;
	double l_price_380;
	bool li_392;
	int li_unused_396;
	double l_price_400;
	double l_ima_44 = At(0).GetBuffer(0).Get(pos - 0);
	double l_ima_52 = At(0).GetBuffer(0).Get(pos - 1);
	double l_ima_28 = l_ima_44;
	double l_ima_100 = l_ima_52;
	double l_ima_20 = l_ima_44;
	double l_ima_92 = l_ima_52;
	double l_ima_36 = At(1).GetBuffer(0).Get(pos - 0);
	double l_ima_108 = At(1).GetBuffer(0).Get(pos - 5);
	double l_ima_116 = At(1).GetBuffer(0).Get(pos - 1);
	double l_ima_60 = At(2).GetBuffer(0).Get(pos - 0);
	double l_ima_68 = At(2).GetBuffer(0).Get(pos - 1);
	double l_ima_76 = At(2).GetBuffer(0).Get(pos - 2);
	double l_ima_124 = l_ima_68;
	double l_ima_84 = At(2).GetBuffer(0).Get(pos - 5);
	double l_ima_284 = At(3).GetBuffer(0).Get(pos - 0);
	double l_istddev_156 = At(4).GetBuffer(0).Get(pos - 0);
	double l_istddev_164 = At(4).GetBuffer(0).Get(pos - 1);
	double l_istddev_292 = At(5).GetBuffer(0).Get(pos - 0);
	double l_iwpr_132 = At(6).GetBuffer(0).Get(pos - 0);
	double l_iwpr_140 = At(6).GetBuffer(0).Get(pos - 1);
	double l_idemarker_172 = At(7).GetBuffer(0).Get(pos - 0);
	double l_idemarker_180 = At(7).GetBuffer(0).Get(pos - 1);
	double l_istochastic_188 = At(8).GetBuffer(0).Get(pos - 0);
	double l_istochastic_196 = At(8).GetBuffer(0).Get(pos - 1);
	double l_istochastic_204 = At(8).GetBuffer(1).Get(pos - 0);
	double l_istochastic_212 = At(8).GetBuffer(1).Get(pos - 1);
	
	if (l_istochastic_196 < l_istochastic_212 - 5.0 && l_istochastic_188 >= l_istochastic_204)
		li_252 = true;
		
	if (l_istochastic_196 > l_istochastic_212 + 5.0 && l_istochastic_188 <= l_istochastic_204)
		li_256 = true;
		
	if (l_istochastic_196 > l_istochastic_212 && l_istochastic_188 > l_istochastic_204)
		li_260 = true;
		
	if (l_istochastic_196 < l_istochastic_212 && l_istochastic_188 < l_istochastic_204)
		li_264 = true;
		
	double l_istochastic_220 = At(9).GetBuffer(0).Get(pos - 0);
	double l_istochastic_228 = At(9).GetBuffer(0).Get(pos - 1);
	double l_istochastic_236 = At(9).GetBuffer(1).Get(pos - 0);
	double l_istochastic_244 = At(9).GetBuffer(1).Get(pos - 1);
	
	if (l_istochastic_228 < l_istochastic_244 && l_istochastic_220 >= l_istochastic_236)
		li_268 = true;
		
	if (l_istochastic_228 > l_istochastic_244 && l_istochastic_220 <= l_istochastic_236)
		li_272 = true;
		
	if (l_istochastic_228 > l_istochastic_244 && l_istochastic_220 > l_istochastic_236)
		li_276 = true;
		
	if (l_istochastic_228 < l_istochastic_244 && l_istochastic_220 < l_istochastic_236)
		li_280 = true;
		
	double l_high_300 = At(10).GetBuffer(1).Get(pos);
	
	double l_low_308 = At(10).GetBuffer(0).Get(pos);
	
	        
	if (Bars < 100) {
		Print("bars less than 100");
		return;
	}
	
	double ld_316 = AccountBalance();
	
	double l_price_324 = StopLoss;
	int l_hour_332 = TimeHour(TimeCurrent());
	
	if (l_price_324 <= 0.0)
		l_price_324 = 0;
		
	int l_count_336 = 0;
	
	int l_count_340 = 0;
	
	int l_ord_total_8 = OrdersTotal();
	
	if ((DayOfWeek() == 5 && TradeOnFriday == false) || l_hour_332 < StartTime_Hr || l_hour_332 > EndTime_Hr)
		return;
		
	if (l_ord_total_8 > 0) {
		for (int l_pos_344 = 0; l_pos_344 < l_ord_total_8; l_pos_344++) {
			OrderSelect(l_pos_344, SELECT_BY_POS);
			
			if (OrderMagicNumber() == MagicNo) {
				if (OrderType() <= OP_SELL && OrderSymbol() == Symbol())
					l_count_336++;
				else
					l_count_340++;
			}
		}
	}
	
	if (l_count_336 > 0) {
		OrderSelect(l_count_336 - 1, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == MagicNo)
			if (TimeCurrent() - OrderOpenTime() <= 60.0 * StopTime)
				return;
	}
	
	if (l_count_336 < MaxOrder) {
		if (AccountFreeMargin() < 100.0 * LotsOptimized()) {
			Print("We have no money.");
			return;
		}
		
		ConstBuffer& open_buf = GetInputBuffer(0, 0);
		ConstBuffer& low_buf = GetInputBuffer(0, 1);
		ConstBuffer& high_buf = GetInputBuffer(0, 2);
		double o0 = open_buf.Get(pos - 0);
		double o1 = open_buf.Get(pos - 1);
		double o2 = open_buf.Get(pos - 2);
		double l1 = low_buf.Get(pos - 1);
		double h1 = high_buf.Get(pos - 1);
		
		if (Predict() == 3.0 && ((l_iwpr_132 > l_iwpr_140 + 9.0 && li_276) || (l_iwpr_132 > l_iwpr_140 + 9.0 && li_260) && l_idemarker_172 > l_idemarker_180 + 0.05 && l_ima_68 < l_ima_116) ||
			(l_istochastic_188 < 40.0 && o1 < l_ima_92 && l_ima_36 > l_ima_84) || (l_istochastic_228 < 40.0 && l_ima_36 > l_ima_116 && l_ima_124 > l_ima_68) || (o2 > l_ima_68 &&
					o1 < o0 && li_260 && l_iwpr_132 > l_iwpr_140 + 7.0 && l_idemarker_172 > l_idemarker_180 + 0.05)) {
			l_ticket_4 = OrderSend(Symbol(), OP_BUY, LotsOptimized(), Ask, g_slippage_380, 0, 0, "", MagicNo);
			
			if (l_ticket_4 > 0) {
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
					Print("BUY order opened");
					
				RefreshRates();
				
				if (l_price_324 > 0.0)
					l_price_324 = Ask - StopLoss * g_point_400;
					
				g_price_384 = Ask + gd_392 * g_point_400;
				
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_324, g_price_384);
			}
			
			else
				Print("Error opening BUY order : " + GetLastError());
				
			return;
		}
		
		if (Predict() == 1.0 && ((l_iwpr_132 > l_iwpr_140 + 9.0 && l_istochastic_220 < 60.0 && li_276) || (l_iwpr_132 > l_iwpr_140 + 18.0 && l_istochastic_188 < 70.0 && li_260) &&
				l_iwpr_132 < -15.0 && l_idemarker_172 > l_idemarker_180 && l_ima_68 > l_ima_116 && l_ima_36 > l_ima_116) || (l_istochastic_188 < 40.0 && o1 > l_ima_92 && l_ima_60 < l_ima_84) ||
			(l_istochastic_228 < 40.0 && l_ima_36 > l_ima_116 && l_ima_124 > l_ima_68) || (o2 > l_ima_68 && o1 < o0 && li_260 && l_iwpr_132 > l_iwpr_140 +
					7.0 && l_idemarker_172 > l_idemarker_180 + 0.1)) {
			l_ticket_4 = OrderSend(Symbol(), OP_BUY, LotsOptimized(), Ask, g_slippage_380, 0, 0, "", MagicNo);
			
			if (l_ticket_4 > 0) {
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
					Print("BUY order opened");
					
				RefreshRates();
				
				if (l_price_324 > 0.0)
					l_price_324 = Ask - StopLoss * g_point_400;
					
				g_price_384 = Ask + gd_392 * g_point_400;
				
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_324, g_price_384);
			}
			
			else
				Print("Error opening BUY order : " + GetLastError());
				
			return;
		}
		
		if ((Predict2() == 1.0 && (l_istochastic_188 < 50.0 && o1 > l_ima_92 && l_ima_60 < l_ima_84 && li_260 && o1 > o0)) || (Predict2() == 1.0 &&
				l_istochastic_188 < 50.0 && li_260 && l_iwpr_132 > l_iwpr_140 + 7.0 && o2 < o1 && o1 > o0) || (Predict2() == 3.0 && o1 < o0)) {
			l_ticket_4 = OrderSend(Symbol(), OP_BUY, LotsOptimized(), Ask, g_slippage_380, 0, 0, "", MagicNo);
			
			if (l_ticket_4 > 0) {
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
					Print("BUY order opened");
					
				RefreshRates();
				
				if (l_price_324 > 0.0)
					l_price_324 = Ask - StopLoss * g_point_400;
					
				g_price_384 = Ask + gd_392 * g_point_400;
				
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_324, g_price_384);
			}
			
			else
				Print("Error opening BUY order : " + GetLastError());
				
			return;
		}
		
		if (Predict() == 4.0 && ((l_iwpr_132 < l_iwpr_140 - 9.0 && li_280) || (l_iwpr_132 < l_iwpr_140 - 9.0 && li_264) && l_idemarker_172 < l_idemarker_180 - 0.05 && l_ima_68 > l_ima_116) ||
			(l_istochastic_188 > 60.0 && o1 > l_ima_100 && l_ima_36 < l_ima_84) || (l_istochastic_228 > 60.0 && l_ima_36 < l_ima_116 && l_ima_124 < l_ima_68) || (o2 < l_ima_68 &&
					o1 > o0 && li_264 && l_iwpr_132 < l_iwpr_140 - 7.0 && l_idemarker_172 < l_idemarker_180 - 0.05)) {
			l_ticket_4 = OrderSend(Symbol(), OP_SELL, LotsOptimized(), Bid, g_slippage_380, 0, 0, "", MagicNo);
			
			if (l_ticket_4 > 0) {
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
					Print("SELL order opened");
					
				RefreshRates();
				
				if (l_price_324 > 0.0)
					l_price_324 = Bid + StopLoss * g_point_400;
					
				g_price_384 = Bid - gd_392 * g_point_400;
				
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_324, g_price_384);
			}
			
			else
				Print("Error opening SELL order : " + GetLastError());
				
			return;
		}
		
		if (Predict() == 2.0 && ((l_iwpr_132 < l_iwpr_140 - 9.0 && l_istochastic_220 > 40.0 && li_280) || (l_iwpr_132 < l_iwpr_140 - 18.0 && l_istochastic_188 > 30.0 && li_264) &&
				l_iwpr_132 > -85.0 && l_idemarker_172 < l_idemarker_180 && l_ima_68 < l_ima_116 && l_ima_36 < l_ima_116) || (l_istochastic_188 > 60.0 && o1 < l_ima_100 && l_ima_60 > l_ima_84) ||
			(l_istochastic_228 > 60.0 && l_ima_36 < l_ima_116 && l_ima_124 < l_ima_68) || (o2 < l_ima_68 && o1 > o0 && li_264 && l_iwpr_132 < l_iwpr_140 - 7.0 &&
					l_idemarker_172 < l_idemarker_180 - 0.1)) {
			l_ticket_4 = OrderSend(Symbol(), OP_SELL, LotsOptimized(), Bid, g_slippage_380, 0, 0, "", MagicNo);
			
			if (l_ticket_4 > 0) {
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
					Print("SELL order opened");
					
				RefreshRates();
				
				if (l_price_324 > 0.0)
					l_price_324 = Bid + StopLoss * g_point_400;
					
				g_price_384 = Bid - gd_392 * g_point_400;
				
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_324, g_price_384);
			}
			
			else
				Print("Error opening SELL order : " + GetLastError());
				
			return;
		}
		
		if ((Predict2() == 2.0 && (l_istochastic_188 > 50.0 && o1 < l_ima_100 && l_ima_60 > l_ima_84 && li_264 && o1 < o0)) || (Predict2() == 2.0 &&
				li_264 && l_istochastic_188 > 50.0 && l_iwpr_132 < l_iwpr_140 - 7.0 && o2 > o1 && o1 < o0) || (Predict2() == 4.0 && o1 > o0)) {
			l_ticket_4 = OrderSend(Symbol(), OP_SELL, LotsOptimized(), Bid, g_slippage_380, 0, 0, "", MagicNo);
			
			if (l_ticket_4 > 0) {
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
					Print("SELL order opened");
					
				RefreshRates();
				
				if (l_price_324 > 0.0)
					l_price_324 = Bid + StopLoss * g_point_400;
					
				g_price_384 = Bid - gd_392 * g_point_400;
				
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_324, g_price_384);
			}
			
			else
				Print("Error opening SELL order : " + GetLastError());
				
			return;
		}
		
		if ((l_ima_44 > l_ima_52 && l_ima_60 > l_ima_68 && l_ima_36 > l_ima_116 && l_ima_36 > l_ima_108 + 0.0005 && l_ima_60 - l_ima_68 > l_ima_36 - l_ima_116 && l_ima_44 - l_ima_36 > l_ima_52 - l_ima_116 &&
			 li_260 && l_iwpr_132 < -15.0 && l_iwpr_132 > l_iwpr_140 + 15.0 && l_ima_44 > l_ima_60 && l_ima_60 > l_ima_36 && o0 > l_ima_44) || (l_ima_44 > l_ima_52 && l_ima_36 > l_ima_116 + 0.0002 && o0 < l_ima_36 + l_istddev_156 && (l_istochastic_220 > l_istochastic_228 && l_istochastic_236 > l_istochastic_244) ||
					 (l_istochastic_188 > l_istochastic_196 && l_istochastic_204 > l_istochastic_212) && l_iwpr_132 < -15.0 && l_iwpr_132 > l_iwpr_140 + 3.0 && li_260 && l_ima_60 > l_ima_36 && o0 > l_ima_44) ||
			(li_268 && l_istochastic_228 < 25.0 && l_ima_36 > l_ima_116 + 0.0002 && l_ima_124 > l_ima_68) || (li_252 && l_istochastic_188 < 30.0 && o1 > l_ima_92 && o0 > l_ima_84 &&
					o0 > l_ima_28 && l_ima_60 < l_ima_84 && l_ima_60 < l_ima_68) || (li_252 && o1 > l_ima_92 && o0 > l_ima_28 && o1 < o0 && l_iwpr_132 > l_iwpr_140 + 7.0 && l_idemarker_172 > l_idemarker_180 + 0.08 && l_istddev_156 > l_istddev_164) ||
			(l1 < l_ima_36 - 3.2 * l_istddev_164 && l_iwpr_132 > l_iwpr_140 + 7.0 && l_iwpr_132 < -75.0 && li_260 && (l_istochastic_188 >= 20.0 && l_istddev_156 > l_istddev_164) ||
			 l_iwpr_140 < ld_148) || (o2 > l_ima_84 && o2 > l_ima_68 && o1 < o0 && o1 > o1 && o0 > l_ima_36 + 2.2 * l_istddev_164 && o0 > o0 && l_istddev_156 > l_istddev_164 && li_260 && l_iwpr_132 > l_iwpr_140 + 9.0 && l_iwpr_132 < -10.0) ||
			(l_ima_68 < l_ima_76 - 0.0001 && l_ima_60 > l_ima_68 + 0.0002 && l_ima_60 > l_ima_76 + 0.0001 && l_ima_68 < l_ima_116 && li_260 || li_276 && o0 > o0) ||
			(l_ima_116 - l_ima_284 < l_ima_36 - l_ima_284 && l_ima_284 < (l_high_300 + l_low_308) / 2.0 - 2.0 * l_istddev_292 && l_ima_36 - l_ima_284 > l_istddev_292 && o0 > o0 &&
			 li_260)) {
			l_ticket_4 = OrderSend(Symbol(), OP_BUY, LotsOptimized(), Ask, g_slippage_380, 0, 0, "", MagicNo);
			
			if (l_ticket_4 > 0) {
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
					Print("BUY order opened");
					
				RefreshRates();
				
				if (l_price_324 > 0.0)
					l_price_324 = Ask - StopLoss * g_point_400;
					
				g_price_384 = Ask + TakeProfit * g_point_400;
				
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_324, g_price_384);
			}
			
			else
				Print("Error opening BUY order : " + GetLastError());
				
			return;
		}
		
		if ((l_ima_44 < l_ima_52 && l_ima_60 < l_ima_68 && l_ima_36 < l_ima_116 && l_ima_36 < l_ima_108 - 0.0005 && l_ima_68 - l_ima_60 > l_ima_116 - l_ima_36 && l_ima_36 - l_ima_44 > l_ima_116 - l_ima_52 &&
			 li_264 && l_iwpr_132 > -85.0 && l_iwpr_132 < l_iwpr_140 - 15.0 && l_ima_44 < l_ima_60 && l_ima_60 < l_ima_36 && o0 < l_ima_44) || (l_ima_44 < l_ima_52 && l_ima_36 < l_ima_116 - 0.0002 && o0 >= l_ima_36 - l_istddev_156 && (l_istochastic_220 < l_istochastic_228 && l_istochastic_236 < l_istochastic_244) ||
					 (l_istochastic_188 < l_istochastic_196 && l_istochastic_204 < l_istochastic_212) && l_iwpr_132 > -85.0 && l_iwpr_132 < l_iwpr_140 - 3.0 && li_264 && l_ima_60 < l_ima_36 && o0 < l_ima_44) ||
			(li_272 && l_istochastic_228 > 75.0 && l_ima_36 < l_ima_116 - 0.0002 && l_ima_124 < l_ima_68) || (li_256 && l_istochastic_188 > 70.0 && o1 < l_ima_100 && o0 < l_ima_84 &&
					o0 < l_ima_20 && l_ima_60 > l_ima_84 && l_ima_60 > l_ima_68) || (li_256 && o1 < l_ima_100 && o0 < l_ima_20 && o1 > o0 && l_iwpr_132 < l_iwpr_140 - 7.0 && l_idemarker_172 < l_idemarker_180 - 0.08 && l_istddev_156 > l_istddev_164) ||
			(h1 > l_ima_36 + 3.2 * l_istddev_164 && l_iwpr_132 < l_iwpr_140 - 7.0 && l_iwpr_132 > -25.0 && li_264 && (l_istochastic_188 <= 80.0 && l_istddev_156 > l_istddev_164) ||
			 l_iwpr_140 > ld_148) || (o2 < l_ima_84 && o2 < l_ima_68 && o1 > o0 && o1 < o1 && o0 < l_ima_36 - 2.2 * l_istddev_164 && l_istddev_156 > l_istddev_164 && li_264 && l_iwpr_132 < l_iwpr_140 - 9.0 && l_iwpr_132 > -90.0) ||
			(l_ima_68 > l_ima_76 + 0.0001 && l_ima_60 < l_ima_68 - 0.0002 && l_ima_60 < l_ima_76 - 0.0001 && l_ima_68 > l_ima_116 && li_264 || li_280) ||
			(l_ima_116 - l_ima_284 > l_ima_36 - l_ima_284 && l_ima_284 > (l_high_300 + l_low_308) / 2.0 + 2.0 * l_istddev_292 && l_ima_284 - l_ima_36 > l_istddev_292 &&
			 li_264)) {
			l_ticket_4 = OrderSend(Symbol(), OP_SELL, LotsOptimized(), Bid, g_slippage_380, 0, 0, "", MagicNo);
			
			if (l_ticket_4 > 0) {
				if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
					Print("SELL order opened");
					
				RefreshRates();
				
				if (l_price_324 > 0.0)
					l_price_324 = Bid + StopLoss * g_point_400;
					
				g_price_384 = Bid - TakeProfit * g_point_400;
				
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_324, g_price_384);
			}
			
			else
				Print("Error opening SELL order : " + GetLastError());
				
			return;
		}
		
		return;
	}
	
	if (ProtectProfit && AccountEquity() > AccountBalance() * (Percent_Over_Balance / 100.0 + 1.0)) {
		for (int l_pos_348 = OrdersTotal() - 1; l_pos_348 >= 0; l_pos_348--) {
			OrderSelect(l_pos_348, SELECT_BY_POS);
			l_cmd_352 = OrderType();
			l_ord_close_356 = false;
			
			switch (l_cmd_352) {
			
			case OP_BUY:
				l_ord_close_356 = OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), g_slippage_380);
				break;
				
			case OP_SELL:
				l_ord_close_356 = OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), g_slippage_380);
			}
			
			if (l_ord_close_356 == 0)
				Sleep(1000);
		}
		
		Print("Account Profit Reached. All Open Trades Have Been Closed");
		
		return;
	}
	
	for (int l_pos_0 = 0; l_pos_0 < l_count_336; l_pos_0++) {
		li_392 = false;
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() <= OP_SELL && OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNo) {
			li_unused_396 = MarketInfo(Symbol(), MODE_STOPLEVEL);
			l_price_400 = OrderStopLoss();
			
			if (OrderType() == OP_BUY) {
				if (AccountFreeMargin() <= 0.0 || (ProtectBalance && AccountEquity() + AccountMargin() < AccountBalance() * (1 - MaximumRisk / 50.0))) {
					OrderClose(OrderTicket(), OrderLots(), Bid, g_slippage_380);
					return;
				}
				
				if (Bid + Hedge_Trigger * g_point_400 <= OrderOpenPrice() && !li_392 && l_count_336 <= MaxOrder && l_count_340 < l_count_336 && Hedge) {
					l_lots_364 = OrderLots() * HedgeLotSize;
					l_price_324 = Bid + HedgeStopLoss * g_point_400;
					
					if (HedgeStopLoss <= 0.0)
						l_price_324 = 0;
						
					l_price_380 = Bid - HedgeProfit * g_point_400;
					
					l_ticket_4 = OrderSend(Symbol(), OP_SELL, l_lots_364, Bid, g_slippage_380, l_price_324, l_price_380, "Hedging", MagicNo);
					
					li_392 = true;
					
					if (l_ticket_4 > 0) {
						if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
							Print("SELL order opened");
					}
					
					else
						Print("Error opening SELL order : " + GetLastError());
						
					if (HighRisk && li_392) {
						l_lots_364 = OrderLots() * LtSize;
						l_price_372 = OrderOpenPrice() + PipDistance * g_point_400;
						l_price_324 = l_price_372 - HR_StopLoss * g_point_400;
						
						if (HR_StopLoss <= 0.0)
							l_price_324 = 0;
							
						l_price_380 = l_price_372 + (Hedge_Trigger - PipDistance + TakeProfit) * g_point_400;
						
						l_ticket_4 = OrderSend(Symbol(), OP_BUYSTOP, l_lots_364, l_price_372, g_slippage_380, l_price_324, l_price_380, "High Risk", MagicNo);
						
						if (l_ticket_4 > 0) {
							if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
								Print("BUY_STOP order opened");
						}
						
						else
							Print("Error opening BUY_STOP order : " + GetLastError());
					}
					
					return;
				}
				
				if (TrailingStop <= 0.0)
					continue;
					
				if (Bid - OrderOpenPrice() <= g_point_400 * TrailingStop)
					continue;
					
				if (l_price_400 >= Bid - g_point_400 * TrailingStop)
					continue;
					
				l_price_400 = Bid - g_point_400 * TrailingStop;
				
				l_price_12 = OrderTakeProfit();
				
				if (TrailingProfit && li_260 && l_istochastic_188 < 70.0)
					l_price_12 = OrderTakeProfit() + TrailingPips * g_point_400;
					
				OrderModify(OrderTicket(), OrderOpenPrice(), l_price_400, l_price_12);
				
				return;
			}
			
			if (AccountFreeMargin() <= 0.0 || (ProtectBalance && AccountEquity() + AccountMargin() < AccountBalance() * (1 - MaximumRisk / 50.0))) {
				OrderClose(OrderTicket(), OrderLots(), Ask, g_slippage_380);
				return;
			}
			
			if (OrderOpenPrice() <= Ask - Hedge_Trigger * g_point_400 && !li_392 && l_count_336 <= MaxOrder && l_count_340 < l_count_336 && Hedge) {
				l_lots_364 = OrderLots() * HedgeLotSize;
				l_price_324 = Ask - HedgeStopLoss * g_point_400;
				
				if (HedgeStopLoss <= 0.0)
					l_price_324 = 0;
					
				l_price_380 = Ask + HedgeProfit * g_point_400;
				
				l_ticket_4 = OrderSend(Symbol(), OP_BUY, l_lots_364, Ask, g_slippage_380, l_price_324, l_price_380, "Hedging", MagicNo);
				
				li_392 = true;
				
				if (l_ticket_4 > 0) {
					if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
						Print("BUY order opened");
				}
				
				else
					Print("Error opening BUY order : " + GetLastError());
					
				if (HighRisk && li_392) {
					l_lots_364 = OrderLots() * LtSize;
					l_price_372 = OrderOpenPrice() - PipDistance * g_point_400;
					l_price_324 = l_price_372 + HR_StopLoss * g_point_400;
					
					if (HR_StopLoss <= 0.0)
						l_price_324 = 0;
						
					l_price_380 = l_price_372 - (Hedge_Trigger - PipDistance + TakeProfit) * g_point_400;
					
					l_ticket_4 = OrderSend(Symbol(), OP_SELLSTOP, l_lots_364, l_price_372, g_slippage_380, l_price_324, l_price_380, "High Risk", MagicNo);
					
					if (l_ticket_4 > 0) {
						if (OrderSelect(l_ticket_4, SELECT_BY_TICKET, MODE_TRADES))
							Print("SELL order opened");
					}
					
					else
						Print("Error opening SELL order : " + GetLastError());
				}
				
				return;
			}
			
			if (TrailingStop > 0.0) {
				if (OrderOpenPrice() - Ask > g_point_400 * TrailingStop) {
					if (l_price_400 > Ask + g_point_400 * TrailingStop || l_price_400 == 0.0) {
						l_price_400 = Ask + g_point_400 * TrailingStop;
						l_price_12 = OrderTakeProfit();
						
						if (TrailingProfit && li_264 && l_istochastic_188 > 30.0)
							l_price_12 = OrderTakeProfit() - TrailingPips * g_point_400;
							
						OrderModify(OrderTicket(), OrderOpenPrice(), l_price_400, l_price_12);
						
						return;
					}
				}
			}
		}
	}
}

double Cashier::LotsOptimized() {
	double ld_ret_0 = MaxLots;
	int l_hist_total_8 = OrdersHistoryTotal();
	int l_count_12 = 0;
	ld_ret_0 = NormalizeDouble(AccountFreeMargin() * MaximumRisk / 100000.0, 1);
	
	if (ld_ret_0 < 0.1)
		ld_ret_0 = 0.1;
		
	if (MinLots < 0.1) {
		ld_ret_0 = NormalizeDouble(AccountFreeMargin() * MaximumRisk / 100000.0, 2);
		
		if (ld_ret_0 < 0.01)
			ld_ret_0 = 0.01;
	}
	
	if (DecreaseFactor > 0.0) {
		for (int l_pos_16 = l_hist_total_8 - 1; l_pos_16 >= 0; l_pos_16--) {
			if (OrderSelect(l_pos_16, SELECT_BY_POS, MODE_HISTORY) == false) {
				Print("Error in history!");
				break;
			}
			
			if (OrderSymbol() != Symbol() || OrderType() > OP_SELL)
				continue;
				
			if (OrderProfit() > 0.0)
				break;
				
			if (OrderProfit() < 0.0)
				l_count_12++;
		}
		
		if (l_count_12 > 1)
			ld_ret_0 = NormalizeDouble(ld_ret_0 - ld_ret_0 * l_count_12 / DecreaseFactor, 1);
	}
	
	if (ld_ret_0 < MinLots)
		ld_ret_0 = MinLots;
		
	if (ld_ret_0 > MaxLots)
		ld_ret_0 = MaxLots;
		
	if (!UseAutoLot)
		ld_ret_0 = MaxLots;
		
	return (ld_ret_0);
}

double Cashier::Predict() {
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	ConstBuffer& low_buf = GetInputBuffer(0, 1);
	ConstBuffer& high_buf = GetInputBuffer(0, 2);
	double o0 = open_buf.Get(pos - 0);
	double o1 = open_buf.Get(pos - 1);
	double o2 = open_buf.Get(pos - 2);
	double l1 = low_buf.Get(pos - 1);
	double h1 = high_buf.Get(pos - 1);
	double l_ima_4;
	double l_ima_28;
	double l_ima_36;
	double l_istddev_132;
	double ld_ret_172;
	double ld_44 = 0;
	double ld_52 = 0;
	double ld_68 = 0;
	double ld_76 = 0;
	double ld_unused_84 = 0;
	double ld_unused_92 = 0;
	double ld_100 = 0;
	double ld_108 = 0;
	double ld_116 = 0;
	double ld_124 = 0;
	double ld_140 = 0;
	double ld_unused_60 = 0;
	double ld_156 = 0;
	double ld_164 = 0;
	double ld_180 = 0;
	double ld_20 = 0;
	double l_ima_12 = At(11).GetBuffer(0).Get(pos);
	
	for (int li_0 = 1; li_0 <= g_period_284; li_0++) {
		l_ima_4 = At(11).GetBuffer(0).Get(pos - li_0);
		l_ima_28 = l_ima_4;
		l_ima_36 = l_ima_36;
		l_istddev_132 = At(12).GetBuffer(0).Get(pos - li_0);
		ld_44 += (high_buf.Get(pos - li_0) + low_buf.Get(pos - li_0)) / 2.0;
		ld_52 += open_buf.Get(pos - li_0 + 1);
		ld_68 += ld_44 - ld_52;
		ld_76 += l_ima_4;
		ld_124 += l_istddev_132;
		ld_156 += open_buf.Get(pos - li_0 + 1) - open_buf.Get(pos - li_0) - (open_buf.Get(pos - li_0) - (open_buf.Get(pos - li_0 -1)));
		ld_20 = ld_20 + (l_ima_28 - l_ima_4) + (l_ima_36 - l_ima_4);
	}
	
	ld_100 = ld_44 / g_period_284;
	
	ld_108 = ld_52 / g_period_284;
	ld_116 = ld_76 / g_period_284;
	ld_140 = ld_124 / g_period_284;
	ld_180 = ld_68 / g_period_284;
	ld_164 = ld_156 / g_period_284;
	
	if (ld_180 > 0.0 && l_ima_12 > ld_116 && ld_164 > 0.0 && o0 < l_ima_12 + ld_140 && o0 > l_ima_12) {
		ld_ret_172 = 1;
		gd_392 = 10000.0 * (gd_296 * ld_140) + gd_288;
	}
	
	if (ld_180 < 0.0 && l_ima_12 < ld_116 && ld_164 < 0.0 && o0 > l_ima_12 - ld_140 && o0 < l_ima_12) {
		ld_ret_172 = 2;
		gd_392 = 10000.0 * (gd_296 * ld_140) + gd_288;
	}
	
	if (ld_180 > 0.0 && l_ima_12 > ld_116 && ld_164 > 0.0 && o0 < l_ima_12 - ld_140) {
		ld_ret_172 = 3;
		gd_392 = 10000.0 * (2.0 * ld_140) + 10.0;
	}
	
	if (ld_180 < 0.0 && l_ima_12 < ld_116 && ld_164 < 0.0 && o0 > l_ima_12 + ld_140) {
		ld_ret_172 = 4;
		gd_392 = 10000.0 * (2.0 * ld_140) + 10.0;
	}
	
	return (ld_ret_172);
}

double Cashier::Predict2() {
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	ConstBuffer& low_buf = GetInputBuffer(0, 1);
	ConstBuffer& high_buf = GetInputBuffer(0, 2);
	double o0 = open_buf.Get(pos - 0);
	double o1 = open_buf.Get(pos - 1);
	double o2 = open_buf.Get(pos - 2);
	double l1 = low_buf.Get(pos - 1);
	double h1 = high_buf.Get(pos - 1);
	
	double ld_ret_68;
	double ld_28 = 0;
	double ld_36 = 0;
	double ld_44 = 0;
	double ld_52 = 0;
	double ld_60 = 0;
	double ld_76 = 0;
	double ld_100 = 0;
	double ld_116 = 0;
	double ld_132 = 0;
	double l_istddev_84 = 0;
	double ld_148 = 0;
	double ld_156 = 0;
	double ld_164 = 0;
	l_istddev_84 = At(4).GetBuffer(0).Get(pos);
	double l_istddev_92 = At(5).GetBuffer(0).Get(pos);
	double l_ima_4 = At(3).GetBuffer(0).Get(pos);
	double l_ima_12 = At(3).GetBuffer(0).Get(pos-1);
	double l_ima_20 = At(3).GetBuffer(0).Get(pos-2);
	
	for (int li_0 = 1; li_0 <= 20; li_0++) {
		ld_28 += open_buf.Get(pos - li_0+1);
		ld_36 += high_buf.Get(pos - li_0) - low_buf.Get(pos - li_0);
		ld_44 += open_buf.Get(pos - li_0+1) - open_buf.Get(pos - li_0);
		
		if (li_0 <= 10) {
			ld_148 += open_buf.Get(li_0+1);
			ld_156 += high_buf.Get(pos - li_0) - low_buf.Get(pos - li_0);
			ld_164 += open_buf.Get(pos - li_0+1) - open_buf.Get(pos - li_0);
		}
	}
	
	ld_52 = ld_28 / 20.0;
	
	ld_60 = ld_36 / 20.0;
	ld_76 = ld_44 / 20.0;
	double ld_172 = ld_148 / 10.0;
	double ld_180 = ld_156 / 10.0;
	double ld_188 = ld_164 / 10.0;
	ld_132 = (o0 - ld_172) / l_istddev_84;
	double ld_140 = (o0 - ld_52) / l_istddev_92;
	ld_100 = ld_172 + 0.308 * ld_180;
	double ld_108 = ld_52 + 0.18 * ld_60;
	ld_116 = ld_172 - 0.308 * ld_180;
	double ld_124 = ld_52 - 0.18 * ld_60;
	
	if ((l_ima_4 - l_ima_20 > l_ima_12 - l_ima_20 + 0.0002 && ld_188 > 0.0 && ld_164 > 0.0 && o0 > ld_116 && o0 <= ld_172 + ld_132 * ld_188 && ld_132 < 0.0 &&
		 h1 - l1 < 1.777 * ld_180) || (l_ima_4 - l_ima_20 > l_ima_12 - l_ima_20 + 0.0002 && ld_76 > 0.0 && ld_44 > 0.0 && o0 > ld_124 && o0 <= ld_52 + ld_132 * ld_76 && ld_140 < 0.0 && h1 - l1 < 1.586 * ld_180)) {
		ld_ret_68 = 1;
		gd_392 = 10000.0 * l_istddev_92 + 10.0;
	}
	
	if ((l_ima_4 - l_ima_20 < l_ima_12 - l_ima_20 - 0.0002 && ld_188 < 0.0 && ld_164 < 0.0 && o0 < ld_100 && o0 >= ld_172 + ld_132 * ld_188 && ld_132 > 0.0 &&
		 h1 - l1 < 1.777 * ld_180) || (l_ima_4 - l_ima_20 < l_ima_12 - l_ima_20 - 0.0002 && ld_76 < 0.0 && ld_44 < 0.0 && o0 < ld_108 && o0 >= ld_52 + ld_132 * ld_76 && ld_140 > 0.0 && h1 - l1 < 1.586 * ld_180)) {
		ld_ret_68 = 2;
		gd_392 = 10000.0 * l_istddev_92 + 10.0;
	}
	
	return (ld_ret_68);
}

}
