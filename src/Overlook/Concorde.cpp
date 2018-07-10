#include "Overlook.h"

#if 0

// urdalatrol

namespace Overlook {

Concorde::Concorde() {
	
}

void Concorde::InitEA() {
	
}

void Concorde::StartEA(int pos) {
	int OB, OS, TB, TS;
	double ProfitBuy, ProfitSell, Lot;
	double LotBuy, LotSell;
	int PipBuy, PipSell;
	
	if (Lots != 0)
		Lot = NormalizeDouble(Lots, 2);
		
	if (LotsProc != 0)
		Lot = NormalizeDouble(AccountBalance() / 100000 * LotsProc, 2);
		
	if (Lot < MarketInfo(Symbol(), MODE_MINLOT))
		Lot = MarketInfo(Symbol(), MODE_MINLOT);
		
	if (Digits == 5 || Digits == 3)
		D = 10;
		
//--------------------------------------------------------------------
	for (int i = 0; i < OrdersTotal(); i++) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol()  && OrderType() == 0) {
			OB++;
			ProfitBuy += OrderProfit();
			TB = OrderTicket();
			LotBuy = OrderLots();
			PipBuy = (Bid - OrderOpenPrice()) / Point / D;
		}
		
		if (OrderSymbol() == Symbol() &&  OrderType() == 1) {
			OS++;
			ProfitSell += OrderProfit();
			TS = OrderTicket();
			LotSell = OrderLots();
			PipSell = (OrderOpenPrice() - Ask) / Point / D;
		}
	}
	
//--------------------------------------------------------------------

	if (OS > 1)
		ModSell();
		
	if (OB > 1)
		ModBuy();
		
//--------------------------------------------------------------------
	if (OB == 0 && OS == 0) {
		OpenOrder(Symbol(), 0, Lot, NormalizeDouble(Ask, Digits), 0, 0, "", Magic);
		OpenOrder(Symbol(), 1, Lot, NormalizeDouble(Bid, Digits), 0, 0, "", Magic);
		return(0);
	}
	
//--------------------------------------------------------------------

	if (OB == 1)
		Tral(TB);
		
	if (OS == 1)
		Tral(TS);
		
//--------------------------------------------------------------------
	if (OB == 0 && OS > 0) {
		RefreshRates();
		OpenOrder(Symbol(), 0, Lot, NormalizeDouble(Ask, Digits), 0, 0, "", Magic);
		RefreshRates();
		
		if (MathAbs(PipSell) >= Step && Kef == 0)
			OpenOrder(Symbol(), 1, LotSell + Lot, NormalizeDouble(Bid, Digits), 0, 0, "", Magic);
			
		if (MathAbs(PipSell) >= Step && Kef > 0)
			OpenOrder(Symbol(), 1, LotSell*Kef, NormalizeDouble(Bid, Digits), 0, 0, "", Magic);
			
	}
	
	if (OB > 0 && OS == 0) {
		RefreshRates();
		OpenOrder(Symbol(), 1, Lot, NormalizeDouble(Bid, Digits), 0, 0, "", Magic);
		RefreshRates();
		
		if (MathAbs(PipBuy) >= Step && Kef == 0)
			OpenOrder(Symbol(), 0, LotBuy + Lot, NormalizeDouble(Ask, Digits), 0, 0, "", Magic);
			
		if (MathAbs(PipBuy) >= Step && Kef > 0)
			OpenOrder(Symbol(), 0, LotBuy*Kef, NormalizeDouble(Ask, Digits), 0, 0, "", Magic);
			
	}
	
//--------------------------------------------------------------------
	return(0);
}

//--------------------------------------------------------------------
int Concorde::ModBuy() {
	double Lots, Summ, BU, Profit;
	
	for (int i = 0; i < OrdersTotal(); i++) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderType() == 0) {
			Lots = Lots + OrderLots();
			Summ = Summ + OrderLots() * OrderOpenPrice();
		}
	}
	
	BU = Summ / Lots;
	
	Profit = NormalizeDouble(BU + TP * D * Point, Digits);
	
	
	for (i = 0; i < OrdersTotal(); i++) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderType() == 0) {
			if (NormalizeDouble(OrderTakeProfit(), Digits) != Profit) {
				OrderModify(OrderTicket(), OrderOpenPrice(), 0, Profit, 0, CLR_NONE);
			}
		}
	}
}

//----------------------------------------------------------------------
int Concorde::ModSell() {
	double Lots, Summ, BU, Profit;
	
	for (int i = 0; i < OrdersTotal(); i++) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderType() == 1) {
			Lots = Lots - OrderLots();
			Summ = Summ - OrderLots() * OrderOpenPrice();
		}
	}
	
	BU = Summ / Lots;
	
	Profit = NormalizeDouble(BU - TP * D * Point, Digits);
	
	
	for (i = 0; i < OrdersTotal(); i++) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() == Symbol() && OrderType() == 1) {
			if (NormalizeDouble(OrderTakeProfit(), Digits) != Profit) {
				OrderModify(OrderTicket(), OrderOpenPrice(), 0, Profit, 0, CLR_NONE);
			}
		}
	}
}

//---------------------------------------------------------------
int Concorde::Tral(int Ticket) {
	double TralPrice, Punkt;
	OrderSelect(Ticket, SELECT_BY_TICKET, MODE_TRADES);
	
	if (OrderType() == 0) {
		Punkt = (Bid - OrderOpenPrice()) / Point;
		TralPrice = NormalizeDouble((Bid - TS * D * Point), Digits);
	}
	
	if (OrderType() == 1) {
		Punkt = (OrderOpenPrice() - Ask) / Point;
		TralPrice = NormalizeDouble((Ask + TS * D * Point), Digits);
	}
	
	if (Punkt < MarketInfo(Symbol(), MODE_STOPLEVEL))
		return(0);
		
	if (Punkt < TS*D)
		return(0);
		
	if (OrderType() == 0 && TralPrice <= NormalizeDouble(OrderStopLoss(), Digits))
		return(0);
		
	if (OrderType() == 1 && TralPrice >= NormalizeDouble(OrderStopLoss(), Digits) && OrderStopLoss() != 0)
		return(0);
		
	ModifyOrder(OrderTicket(), OrderOpenPrice(), TralPrice, OrderTakeProfit());
	
	return(0);
}

//----------------------------------------------------------------------------
int Concorde::OpenOrder(string symbol, int cmd, double volume, double price, double stoploss, double takeprofit, string comment, int magic) {
	RefreshRates();
	int err, i, ticket;
	
	while (i < 10) {
		if (cmd > 1)
			ticket = OrderSend(symbol, cmd, NormalizeDouble(volume, 2), NormalizeDouble(price, Digits), Slippage, 0, 0, comment, magic);
			
		if (cmd == 1)
			ticket = OrderSend(symbol, cmd, NormalizeDouble(volume, 2), NormalizeDouble(Bid, Digits), Slippage, 0, 0, comment, magic);
			
		if (cmd == 0)
			ticket = OrderSend(symbol, cmd, NormalizeDouble(volume, 2), NormalizeDouble(Ask, Digits), Slippage, 0, 0, comment, magic);
			
		err = GetLastError();
		
		if (err == 0)
			break;
			
		Alert(Symbol(), Error(err), "  ïðè îòêðûòèè îðäåðà");
		
		Sleep(1000);
		
		i++;
	}
	
	return(ticket);
}

int Concorde::ModifyOrder(int ticket, double price, double stoploss, double takeprofit) {
	int err, i;
	RefreshRates();
	
	while (i < 10) {
		OrderModify(ticket, NormalizeDouble(price, Digits), NormalizeDouble(stoploss, Digits), NormalizeDouble(takeprofit, Digits), 0, CLR_NONE);
		err = GetLastError();
		
		if (err == 0)
			break;
			
		Alert(Symbol(), Error(err), "  ïðè ìîäèôèêàöèè îðäåðà");
		
		Sleep(1000);
		
		i++;
	}
	
	return(0);
}

}


#endif
