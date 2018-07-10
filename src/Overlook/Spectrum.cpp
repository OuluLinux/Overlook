#include "Overlook.h"

namespace Overlook {

Spectrum::Spectrum() {
	
}

void Spectrum::InitEA() {
	
	AddSubCore<Momentum>()
		.Set("period", g_period_132);
	
	AddSubCore<AverageDirectionalMovement>()
		.Set("period", adx_period);
	
	AddSubCore<RelativeStrengthIndex>()
		.Set("period", rsi_period);
	
}

void Spectrum::StartEA(int pos) {
	this->pos = pos;
	if (pos < 4)
		return;
	
	if (Timetrade) {
		if (!(Hour() >= StartTime && Hour() <= EndTime)) {
			Comment("Close Trading");
			return;
		}
	}
	
	Procces();
	
}

void Spectrum::OpenBUYOrder(double a_lots_0, int a_magic_8) {
	double l_imomentum_12 = At(0).GetBuffer(0).Get(pos - 0);
	double l_imomentum_20 = At(0).GetBuffer(0).Get(pos - 1);
	double l_iadx_28 = At(1).GetBuffer(0).Get(pos - 0);
	double l_irsi_36 = At(2).GetBuffer(0).Get(pos - 0);
	double l_price_44 = Ask - StopLoss * Point;
	double l_price_52 = Ask + TakeProfit * Point;
	
	ConstBuffer& low = GetInputBuffer(0, 1);
	
	if (low.Get(pos-2) > low.Get(pos - 3) && l_imomentum_20 >= Level && l_imomentum_12 >= Level && l_iadx_28 > 21.0 && l_irsi_36 > 70.0) {
		g_ticket_136 = OrderSend(Symbol(), OP_BUY, a_lots_0, Ask, 3, l_price_44, l_price_52, "", a_magic_8);
		
		if (g_ticket_136 > 0) {
			g_ticket_148 = 0;
			
			while (g_ticket_148 == 0) {
				Sleep(5000);
				g_ticket_148 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_0, l_price_44, 3, l_price_44 + StopLoss * Point, l_price_44 - TakeProfit * Point, "", a_magic_8);
				Sleep(5000);
			}
			
			g_ticket_152 = 0;
			
			while (g_ticket_152 == 0) {
				Sleep(5000);
				g_ticket_152 = OrderSend(Symbol(), OP_SELLSTOP, a_lots_0, l_price_44, 3, l_price_44 + StopLoss * Point, l_price_44 - TakeProfit * Point, "", a_magic_8);
				Sleep(5000);
			}
		}
	}
}

void Spectrum::OpenSELLOrder(double a_lots_0, int a_magic_8) {
	double l_imomentum_12 = At(0).GetBuffer(0).Get(pos - 0);
	double l_imomentum_20 = At(0).GetBuffer(0).Get(pos - 1);
	double l_iadx_28 = At(1).GetBuffer(0).Get(pos - 0);
	double l_irsi_36 = At(2).GetBuffer(0).Get(pos - 0);
	double l_price_44 = Bid + StopLoss * Point;
	double l_price_52 = Bid - TakeProfit * Point;
	
	ConstBuffer& high = GetInputBuffer(0, 2);
	
	if (high.Get(pos - 2) < high.Get(pos - 3) && l_imomentum_20 <= Level && l_imomentum_12 <= Level && l_iadx_28 < 21.0 && l_irsi_36 < 30.0) {
		g_ticket_136 = OrderSend(Symbol(), OP_SELL, a_lots_0, Bid, 3, l_price_44, l_price_52, "", a_magic_8);
		
		if (g_ticket_136 > 0) {
			g_ticket_148 = 0;
			
			while (g_ticket_148 == 0) {
				Sleep(5000);
				g_ticket_148 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_0, l_price_44, 3, l_price_44 - StopLoss * Point, l_price_44 + TakeProfit * Point, "", a_magic_8);
				Sleep(5000);
			}
			
			g_ticket_152 = 0;
			
			while (g_ticket_152 == 0) {
				Sleep(5000);
				g_ticket_152 = OrderSend(Symbol(), OP_BUYSTOP, a_lots_0, l_price_44, 3, l_price_44 - StopLoss * Point, l_price_44 + TakeProfit * Point, "", a_magic_8);
				Sleep(5000);
			}
		}
		
	}
}

int Spectrum::Procces() {
	double l_lots_0;
	double l_ord_stoploss_8;
	double l_ord_takeprofit_16;
	
	if (MyRealOrdersTotal(MagicNumber) == 0 && MyPendingOrdersTotal(MagicNumber) == 2) {
		DeletePendingOrders(MagicNumber);
		return (0);
	}
	
	if (MyRealOrdersTotal(MagicNumber) == 0 && MyPendingOrdersTotal(MagicNumber) == 0) {
		if (OpenBuy)
			OpenBUYOrder(StartLot, MagicNumber);
			
		if (OpenSell)
			OpenSELLOrder(StartLot, MagicNumber);
			
		return (0);
	}
	
	if (MyRealOrdersTotal(MagicNumber) == 2 && MyPendingOrdersTotal(MagicNumber) == 0) {
		OrderSelect(g_ticket_140, SELECT_BY_TICKET, MODE_TRADES);
		l_lots_0 = OrderLots() * LotsDouble;
		l_ord_stoploss_8 = OrderStopLoss();
		l_ord_takeprofit_16 = OrderTakeProfit();
		
		if (OrderType() == OP_BUY) {
			g_ticket_148 = 0;
			
			while (g_ticket_148 == 0) {
				Sleep(5000);
				g_ticket_148 = OrderSend(Symbol(), OP_SELLSTOP, l_lots_0, l_ord_stoploss_8, 3, l_ord_stoploss_8 + StopLoss * Point, l_ord_stoploss_8 - TakeProfit * Point, "", MagicNumber);
				Sleep(5000);
			}
			
			g_ticket_152 = 0;
			
			while (g_ticket_152 == 0) {
				Sleep(5000);
				g_ticket_152 = OrderSend(Symbol(), OP_SELLSTOP, l_lots_0, l_ord_stoploss_8, 3, l_ord_stoploss_8 + StopLoss * Point, l_ord_stoploss_8 - TakeProfit * Point, "", MagicNumber);
				Sleep(5000);
			}
		}
		
		if (OrderType() == OP_SELL) {
			g_ticket_148 = 0;
			
			while (g_ticket_148 == 0) {
				Sleep(5000);
				g_ticket_148 = OrderSend(Symbol(), OP_BUYSTOP, l_lots_0, l_ord_stoploss_8, 3, l_ord_stoploss_8 - StopLoss * Point, l_ord_stoploss_8 + TakeProfit * Point, "", MagicNumber);
				Sleep(5000);
			}
			
			g_ticket_152 = 0;
			
			while (g_ticket_152 == 0) {
				Sleep(5000);
				g_ticket_152 = OrderSend(Symbol(), OP_BUYSTOP, l_lots_0, l_ord_stoploss_8, 3, l_ord_stoploss_8 - StopLoss * Point, l_ord_stoploss_8 + TakeProfit * Point, "", MagicNumber);
				Sleep(5000);
			}
		}
	}
	
	return (0);
}

int Spectrum::MyRealOrdersTotal(int a_magic_0) {
	int l_count_4 = 0;
	int l_ord_total_8 = OrdersTotal();
	g_ticket_140 = -1;
	g_ticket_144 = -1;
	
	for (int l_pos_12 = 0; l_pos_12 < l_ord_total_8; l_pos_12++) {
		OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == a_magic_0 && OrderSymbol() == Symbol() && OrderType() == OP_BUY || OrderType() == OP_SELL) {
			if (g_ticket_140 == -1)
				g_ticket_140 = OrderTicket();
			else
				if (g_ticket_144 == -1)
					g_ticket_144 = OrderTicket();
					
			l_count_4++;
		}
	}
	
	return (l_count_4);
}

int Spectrum::MyPendingOrdersTotal(int a_magic_0) {
	int l_count_4 = 0;
	int l_ord_total_8 = OrdersTotal();
	
	for (int l_pos_12 = 0; l_pos_12 < l_ord_total_8; l_pos_12++) {
		OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == a_magic_0 && OrderSymbol() == Symbol() && OrderType() == OP_BUYSTOP || OrderType() == OP_SELLSTOP)
			l_count_4++;
	}
	
	return (l_count_4);
}

int Spectrum::DeletePendingOrders(int a_magic_0) {
	int l_ord_total_4 = OrdersTotal();
	
	for (int l_pos_8 = l_ord_total_4 - 1; l_pos_8 >= 0; l_pos_8--) {
		OrderSelect(l_pos_8, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderMagicNumber() == a_magic_0 && OrderSymbol() == Symbol() && OrderType() == OP_BUYSTOP || OrderType() == OP_SELLSTOP)
			OrderDelete(OrderTicket());
	}
	
	return (0);
}

}
