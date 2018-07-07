#include "Overlook.h"


namespace Overlook {

DoubleDecker::DoubleDecker() {
	
}

void DoubleDecker::InitEA() {
	AddSubCore<Channel>()
		.Set("period", ch_period);
}

void DoubleDecker::StartEA(int pos) {
	int cnt, ticket, err, i, j, cmd;
	int MagicNumber;
	double ts, tp, LowestPrice, HighestPrice, Price;
	bool Order[5];
	String setup;
	
	if (IsTesting() && pos < 100)
		return;
		
	MagicNumber = 50123;
	
	setup = "H123_" + Symbol();
	
	
	///////////////// MODIFICATIONS ON OPEN ORDERS   ////////////////////////////////////////////////////////////////////
	
	for (cnt = OrdersTotal()-1; cnt >= 0; cnt--) {
		if (!OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES))
			continue;
		
		if (OrderType() == OP_BUY && (OrderMagicNumber() == MagicNumber + 1 || OrderMagicNumber() == MagicNumber + 3)) {
			if (TimeDay(OrderOpenTime()) != TimeDay(Now)) {
				Print(".");
				OrderClose(OrderTicket(), Lots, Bid, 3);
				
				if (err > 1) {
					Print("Error closing buy order: " + GetLastError());
				}
			}
			
			else
			if (OrderStopLoss() == 0) {
				if (TakeProfit > 0) {
					tp = OrderOpenPrice() + TakeProfit * Point;
				}
				
				else {
					tp = 0;
				}
				
				if (InitialStopLoss > 0) {
					ts = OrderOpenPrice() - InitialStopLoss * Point;
				}
				
				else {
					ts = 0;
				}
				
				OrderModify(OrderTicket(), OrderOpenPrice(), ts, tp);
				
				if (err > 1) {
					Print("Error modifying Buy order: " + GetLastError());
				}
			}
			
			else
			if (TrailingStop > 0) {
				ts = Bid - Point * TrailingStop;
				
				if (OrderStopLoss() < ts && OrderProfit() > 0)
					OrderModify(OrderTicket(), OrderOpenPrice(), ts, OrderTakeProfit());
			}
		}
		
		else
		if (OrderType() == OP_SELL && (OrderMagicNumber() == MagicNumber + 2 || OrderMagicNumber() == MagicNumber + 4)) {
			if (TimeDay(OrderOpenTime()) != TimeDay(Now)) {
				Print(".");
				OrderClose(OrderTicket(), Lots, Ask, 3);
				
				if (err > 1) {
					Print("Error closing Sell order: " + GetLastError());
				}
			}
			
			else
			if (OrderStopLoss() == 0) {
				if (TakeProfit > 0) {
					tp = OrderOpenPrice() - TakeProfit * Point;
				}
				
				else {
					tp = 0;
				}
				
				if (InitialStopLoss > 0) {
					ts = OrderOpenPrice() + InitialStopLoss * Point;
				}
				
				else {
					ts = 0;
				}
				
				OrderModify(OrderTicket(), OrderOpenPrice(), ts, tp);
				
				if (err > 1) {
					Print("Error modifying Sell order: " + GetLastError());
				}
			}
			
			else
			if (TrailingStop > 0) {
				ts = Ask + Point * TrailingStop;
				
				if (OrderStopLoss() > ts && OrderProfit() > 0)
					OrderModify(OrderTicket(), OrderOpenPrice(), ts, OrderTakeProfit());
			}
		}
	}
	
	///////////////// SETTING ORDERS ////////////////////////////////////////////////////////////////////
	
	if (AccountFreeMargin() < (1000*Lots))
		return;
		
	
//----
	if (TimeHour(Now) == EndSession1 && TimeMinute(Now) == 0) {
		Core& c = At(0);
		LowestPrice = c.GetBuffer(0).Get(pos);
		HighestPrice = c.GetBuffer(1).Get(pos);
		
		//// the following is necessary, to avoid a BUYSTOP/SELLSTOP Price which is too close to Bid/Ask,
		//// in which case we get a 130 invalid stops.
		//// I experimented to change to proper OP_BUY and OP_SELL, but the results where not satisfying
		//if (HighestPrice+5*Point<Ask+Spread*Point) {
		//	cmd=OP_BUY;
		//	Price=Ask;
		//} else {
		cmd = OP_BUYSTOP;
		Price = HighestPrice + 5 * Point;
		//}
		ticket = OrderSendExtended(Symbol(), cmd, Lots, Price, Slippage, 0, 0, setup, MagicNumber + 1);
		
		if (ticket == -1) {
			Print("Error modifying Sell order: " + GetLastError());
		}
		
		//if (LowestPrice-5*Point>Bid-Spread*Point) {
		//	cmd=OP_SELL;
		//	Price=Bid;
		//} else {
		cmd = OP_SELLSTOP;
		
		Price = LowestPrice - 5 * Point;
		
		//}
		ticket = OrderSendExtended(Symbol(), OP_SELLSTOP, Lots, Price, Slippage, 0, 0, setup, MagicNumber + 2);
		
		
		if (ticket == -1) {
			Print("Error modifying Sell order: " + GetLastError());
		}
	}
	
	if (TimeHour(Now) == EndSession2 && TimeMinute(Now) == 0) {
		Core& c = At(0);
		LowestPrice = c.GetBuffer(0).Get(pos);
		HighestPrice = c.GetBuffer(1).Get(pos);
		//if (HighestPrice+5*Point<Ask+Spread*Point) {
		//	cmd=OP_BUY;
		//	Price=Ask;
		//} else {
		cmd = OP_BUYSTOP;
		Price = HighestPrice + 5 * Point;
		//}
		ticket = OrderSendExtended(Symbol(), cmd, Lots, Price, Slippage, 0, 0, setup, MagicNumber + 3);
		
		if (ticket == -1) {
			Print("Error modifying Sell order: " + GetLastError());
		}
		
		//if (LowestPrice-5*Point>Bid-Spread*Point) {
		//	cmd=OP_SELL;
		//	Price=Bid;
		//} else {
		cmd = OP_SELLSTOP;
		
		Price = LowestPrice - 5 * Point;
		
		//}
		ticket = OrderSendExtended(Symbol(), cmd, Lots, Price, Slippage, 0, 0, setup, MagicNumber + 4);
		
		if (ticket == -1) {
			Print("Error modifying Sell order: " + GetLastError());
		}
	}
}


int DoubleDecker::OrderSendExtended(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, String comment, int magic) {
	int ticket;
	ticket = OrderSend(symbol, cmd, volume, price, slippage, stoploss, takeprofit, comment, magic);
	return(ticket);
}

}
