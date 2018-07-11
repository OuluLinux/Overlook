#include "Overlook.h"

// isea

namespace Overlook {

Puma::Puma() {

}

void Puma::InitEA() {
	MinStopLoss = MarketInfo(Symbol(), MODE_STOPLEVEL);
	MinStopLoss = (MinStopLoss + 2.0) * Point;
	stoploss = StopLoss * Point;
	
	MA_distance = MA_PipsAway * Point;
	//TakeProfit = TakeProfit * Point;
	spread = Ask - Bid;
	stoplossReal = StopLoss * Point;
	
	AddSubCore<MovingAverage>()
		.Set("period", MA_Period).Set("method", 2);
	
	
}

void Puma::StartEA(int pos) {

	/* String ls_0 = "2009.07.07";
	int l_str2time_8 = StrToTime(ls_0);
	if (TimeCurrent() >= l_str2time_8)
	   { Alert("This Robot Has Been Expired. Please contact www.investiva.net for update."); return (0); }*/
	
	
	totalOrders = OrdersTotal();
	HasBuyLimitOrder = false;
	HasSellLimitOrder = false;
	HasBuyOrder = false;
	HasSellOrder = false;
	OpenLimitOrder();
	
	if (BuyLimitOrderEntered == true) {
		BL_openprice = NormalizeDouble(BuyLiMitOpenprice, Digits);
		HasBuyLimitOrder = true;
	}
	
	if (SellLimitOrderEntered == true) {
		SL_openprice = NormalizeDouble(SellLiMitOpenprice, Digits);
		HasSellLimitOrder = true;
	}
	
	
	MA_currentbar = At(0).GetBuffer(0).Get(pos - 0);
	
	Modify_order();
	
	for (int k = OrdersTotal(); k >= 0; k--) {
		if (OrderSelect(k, SELECT_BY_POS) == true && OrderSymbol() == Symbol() && (OrderMagicNumber() == MagicNumber)) {
			if (OrderType() == OP_BUY) {
				HasBuyOrder = true;
				Close_B(OrderTicket(), OrderLots());
			}
			
			else if (OrderType() == OP_SELL) {
				HasSellOrder = true;
				Close_S(OrderTicket(), OrderLots());
			}
			
		}
	}
	
	if (UseDelay == true) {
		StoppedOut = LastTradeStoppedOut();
		
		if (Now < StopTime) {
			return;
		}
	}
	
	if (UseHourTrade) {
		if (!((Hour() + GMTOffSet) >= FromHourTrade && (Hour() + GMTOffSet) <= ToHourTrade)) {
			Comment("This is not trading time.");
			return;
		}
	}
	
	OpenMarketOrder();
	
	return;
}

int Puma::CloseOrder(int ticket, double numLots, double price) {
	int CloseCnt, err;
	
	CloseCnt = 0;
	
	while (CloseCnt < 3) { // try to close 3 Times
		if (!OrderClose(ticket, numLots, price, 3, Yellow)) {
			CloseCnt++;
		}
		
		else {
			CloseCnt = 3;
		}
	}
	
	return CloseCnt;
}

void Puma::Close_B(int ticket, double lot) {
	bool closecondition = false;
	closecondition = NormalizeDouble(Bid - OrderOpenPrice(), Digits) >= (TakeProfit * Point) || MathAbs(NormalizeDouble(OrderOpenPrice() - Bid, Digits)) > (StopLoss * Point);
	
	if (closecondition == true) {
		CloseOrder(ticket, lot, Bid);
		//OrderClose(ticket, lot, Bid, 1, Yellow);
		HasBuyOrder = false;
	}
}

void Puma::Close_S(int ticket, double lot) {
	bool closecondition = false;
	closecondition = NormalizeDouble(OrderOpenPrice() - Ask, Digits) >= (TakeProfit * Point) || MathAbs(NormalizeDouble(Ask - OrderOpenPrice(), Digits)) > (StopLoss * Point);
	
	if (closecondition == true) {
		CloseOrder(ticket, lot, Ask);
		//OrderClose(ticket, lot, Ask, 1, Yellow);
		HasSellOrder = false;
	}
}

void Puma::Modify_order() {
	double AveragePrice = 0;
	
	if (HasBuyLimitOrder == true) {
		AveragePrice = MA_currentbar - (MA_PipsAway * Point);
		
		if (MathAbs(BL_openprice - AveragePrice) > Point / 2.0)
			//OrderModify(BL_ticket, AveragePrice, AveragePrice - stoploss, AveragePrice + TakeProfit*Point, 0, DeepSkyBlue);
			BuyLiMitOpenprice = MA_currentbar - (MA_PipsAway * Point);
			
		BL_openprice = MA_currentbar - (MA_PipsAway * Point);
	}
	
	if (HasSellLimitOrder == true) {
		AveragePrice = MA_currentbar + spread + (MA_PipsAway * Point);
		
		if (MathAbs(SL_openprice - AveragePrice) > Point / 2.0)
			//OrderModify(SL_ticket, AveragePrice, AveragePrice + stoploss, AveragePrice - TakeProfit*Point, 0, Pink);
			SellLiMitOpenprice = MA_currentbar + spread + (MA_PipsAway * Point);
			
		SL_openprice = MA_currentbar + spread + (MA_PipsAway * Point);
	}
}

void Puma::OpenLimitOrder() {
	double AveragePrice = 0;
	double SL = 0;
	double MA_dis_pips = 0;
	
	if (
		HasBuyOrder == false
		&& HasBuyLimitOrder == false
		&& Ask < MA_currentbar
	) {
		AveragePrice = MA_currentbar - MA_distance;
		
		if (
			AveragePrice > Ask - MinStopLoss)
			AveragePrice = Ask - MinStopLoss;
			
		AveragePrice = NormalizeDouble(AveragePrice, Digits);
		
		//OrderSend(Symbol(), OP_BUYLIMIT, Lots(), AveragePrice, 3, AveragePrice - stoploss, AveragePrice + TakeProfit*Point, "", 0, 0, Blue);
		BuyLimitOrderEntered = true;
		
		HasBuyLimitOrder = true;
		
		BuyLiMitOpenprice = NormalizeDouble((MA_currentbar - MA_distance), Digits);
	}
	
	if (
		HasSellOrder == false
		&& HasSellLimitOrder == false
		&& Bid   >  MA_currentbar
	) {
		AveragePrice = MA_currentbar + spread + MA_distance;
		
		if (AveragePrice < Bid + MinStopLoss)
			AveragePrice = Bid + MinStopLoss;
			
		AveragePrice = NormalizeDouble(AveragePrice, Digits);
		
		//OrderSend(Symbol(), OP_SELLLIMIT, Lots(), AveragePrice, 3, AveragePrice + stoploss, AveragePrice - TakeProfit*Point, "", 0, 0, Red);
		SellLimitOrderEntered = true;
		
		HasSellLimitOrder = true;
		
		SellLiMitOpenprice = NormalizeDouble((MA_currentbar + spread + MA_distance), Digits);
	}
}

double Puma::GetLots() {
	double Lots;
	if (UseMoneyManagement == true) {
		double SingleLot = MarketInfo(Symbol(), 15) / AccountLeverage();
		double LotValue = AccountEquity() * Risk / 100;
		
		if (MarketInfo(Symbol(), 23) == 0.1) {
			double Lots = NormalizeDouble(LotValue / SingleLot, 1);
			
			if (Lots < 0.1)
				Lots = 0.1;
				
			//if (Lots > 50.0) Lots = 50;
		}
		
		else
			Lots = NormalizeDouble(LotValue / SingleLot, 2);
	}
	
	else
		Lots = LotSize;
		
	if (Lots > MarketInfo(Symbol(), 25))
		Lots = MarketInfo(Symbol(), 25);
		
	return(Lots);
}

void Puma::OpenBuyOrder() {
	int err, ticket;
	int cnt = 0;
	
	// while (OrderSend(Symbol(),OP_BUY,Lots(),Ask,Slippage,Ask-stoploss,Ask+TakeProfitFaked,"",MagicNumber,0,Green)==false&&cnt<OrderAttemps)
	
	while (OrderSend(Symbol(), OP_BUY, GetLots(), Ask, Slippage, 0, 0, AddComment, MagicNumber, 0, Green) <= 0 && cnt < OrderAttemps) {
		cnt++;
		Sleep(100);
	}
}

void Puma::OpenSellOrder() {
	int err, ticket;
	int cnt = 0;
	
	
	//while (OrderSend(Symbol(),OP_SELL,Lots(),Bid,Slippage,Bid+stoploss,Bid-TakeProfitFaked,"",MagicNumber,0,Red)<=0 && cnt<OrderAttemps)
	
	while (OrderSend(Symbol(), OP_SELL, GetLots(), Bid, Slippage, 0, 0, AddComment, MagicNumber, 0, Red) <= 0 && cnt < OrderAttemps) {
		cnt++;
		Sleep(100);
	}
}


void Puma::OpenMarketOrder() {
//buy at ask, sell at bid
	if (Ask < (BuyLiMitOpenprice - (Slippage*Point)) && HasBuyOrder == false) {
		OpenBuyOrder();
		BuyLimitOrderEntered = false;
		BL_openprice = 0.0;
		BuyLiMitOpenprice = 0.0;
	}
	
	if (Bid > (SellLiMitOpenprice + (Slippage*Point)) && HasSellOrder == false) {
		OpenSellOrder();
		SellLimitOrderEntered = false;
		SL_openprice = 0.0;
		SellLiMitOpenprice = 0.0;
	}
}

bool Puma::LastTradeStoppedOut() {
	int cnt, total;
	bool Stopped = false;
	
	total = HistoryTotal();
	
	for (cnt = total - 1; cnt >= 0; cnt--) {
		OrderSelect(cnt, SELECT_BY_POS, MODE_HISTORY);
		
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			Stopped = false;
			
			if (OrderType() == OP_BUY) {
				if (OrderClosePrice() - OrderOpenPrice() < 0) {
					Stopped = true;
				}
				
				cnt = 0;
			}
			
			if (OrderType() == OP_SELL) {
				if (OrderOpenPrice() - OrderClosePrice() < 0) {
					Stopped = true;
				}
				
				cnt = 0;
			}
		}
	}
	
	if (Stopped) {
		StopTime = OrderCloseTime() + MinutesToDelay * 60;
	}
	
	return (Stopped);
}

}
