#include "Overlook.h"

namespace Overlook {
	
StochPower::StochPower() {
	
}

void StochPower::InitEA() {
	
	AddSubCore<MovingAverage>()
		.Set("period", ma_period);
}


void StochPower::StartEA(int pos) {
	this->pos = pos;
	int cnt = 0;
	bool result;
	String text = "";
	String version = "VERSI INI DAH EXPIRED...SILA YM xxul_gunturean utk penggantian";
	
	// skrip masa trading
	
	if (UseTradingHours) {
	
		YesStop = true;
// Check trading Asian Market

		if (TradeAsianMarket) {
			if (StartHour1 > 18) {
// Check broker that uses Asian open before 0:00

				if (Hour() >= StartHour1)
					YesStop = false;
					
				if (!YesStop) {
					if (StopHour1 < 24) {
						if (Hour() <= StopHour1)
							YesStop = false;
					}
					
// These cannot be combined even though the code looks the same

					if (StopHour1 >= 0) {
						if (Hour() <= StopHour1)
							YesStop = false;
					}
				}
			}
			
			else {
				if (Hour() >= StartHour1 && Hour() <= StopHour1)
					YesStop = false;
			}
		}
		
		if (YesStop) {
// Check trading European Market
			if (TradeEuropeanMarket) {
				if (Hour() >= StartHour2 && Hour() <= StopHour2)
					YesStop = false;
			}
		}
		
		if (YesStop) {
// Check trading European Market
			if (TradeNewYorkMarket) {
				if (Hour() >= StartHour3 && Hour() <= StopHour3)
					YesStop = false;
			}
		}
		
		if (YesStop) {
//      Comment ("Trading has been stopped as requested - wrong time of day");
			return;
		}
	}
	
	// skrip masa trading
	
	if (AccountType == 0) {
		if (mm != 0) {
			lotsi = MathCeil(AccountBalance() * risk / 10000);
		}
		
		else {
			lotsi = Lots;
		}
	}
	
	if (AccountType == 1) { // then is mini
		if (mm != 0) {
			lotsi = MathCeil(AccountBalance() * risk / 10000) / 10;
		}
		
		else {
			lotsi = Lots;
		}
	}
	
	if (AccountType == 2) {
		if (mm != 0) {
			lotsi = MathCeil(AccountBalance() * risk / 10000) / 100;
		}
		
		else {
			lotsi = Lots;
		}
	}
	
	if (lotsi < 0.01)
		lotsi = 0.01;
		
	if (lotsi > 100)
		lotsi = 100;
		
	OpenOrders = 0;
	
	MarketOpenOrders = 0;
	
	LimitOpenOrders = 0;
	
	for (cnt = 0;cnt < OrdersTotal();cnt++) {
		if (OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					MarketOpenOrders++;
					LastOrderOpenTime = OrderOpenTime();
				}
				
				if (OrderType() == OP_SELLLIMIT || OrderType() == OP_BUYLIMIT)
					LimitOpenOrders++;
					
				OpenOrders++;
			}
		}
	}
	
	if (OpenOrders < 1) {
		if (!TradeOnFriday && DayOfWeek() == 5) {
			Comment("TradeOnfriday is false");
			return;
		}
	}
	
	PipValue = MarketInfo(Symbol(), MODE_TICKVALUE);
	
	if (PipValue == 0) {
		PipValue = 5;
	}
	
	if (AccountMoneyProtection && AccountBalance() - AccountEquity() >= AccountMoneyProtectionValue) {
		text = text + "\nClosing all orders and stop trading because account money protection activated..";
		Print("Closing all orders and stop trading because account money protection activated..");
		PreviousOpenOrders = OpenOrders + 1;
		ContinueOpening = false;
		return;
	}
	
	////aku masukkan time
	
	if (UseHourTrade) {
		if (!(Hour() >= FromHourTrade && Hour() <= ToHourTrade)) {
			text = text + "\nORDER TELAH DI TUTUP KERANA MASA DAH TAMAT.";
			Print("ORDER TELAH DI TUTUP KERANA MASA TAMAT.");
			PreviousOpenOrders = OpenOrders + 1;
			ContinueOpening = false;
			return;
		}
	}
	
	//set my profit for one Day
	
	if (MyMoneyProfitTarget && AccountProfit() >= My_Money_Profit_Target) {
		text = text + "\nClosing all orders and stop trading because mymoney profit target reached..";
		Print("Closing all orders and stop trading because mymoney profit target reached.. ");
		PreviousOpenOrders = OpenOrders + 1;
		ContinueOpening = false;
		return;
	}
	
	// Account equity protection
	
	if (EquityProtection && AccountEquity() <= AccountBalance()*AccountEquityPercentProtection / 100) {
		text = text + "\nClosing all orders and stop trading because account money protection activated.";
		Print("Closing all orders and stop trading because account money protection activated.");
		//Comment("Closing orders because account equity protection was triggered. Balance: ",AccountBalance()," Equity: ", AccountEquity());
		//OrderClose(LastTicket,LastLots,LastClosePrice,slippage,Orange);
		PreviousOpenOrders = OpenOrders + 1;
		ContinueOpening = false;
		return;
	}
	
	// if dont trade at fridays then we close all
	
	if (!TradeOnFriday && DayOfWeek() == 5) {
		PreviousOpenOrders = OpenOrders + 1;
		ContinueOpening = false;
		text = text + "\nClosing all orders and stop trading because TradeOnFriday protection.";
		Print("Closing all orders and stop trading because TradeOnFriday protection.");
	}
	
	// Orders Time alive protection
	
	if (OrdersTimeAlive > 0 && Now - LastOrderOpenTime > OrdersTimeAlive) {
		PreviousOpenOrders = OpenOrders + 1;
		ContinueOpening = false;
		text = text + "\nClosing all orders because OrdersTimeAlive protection.";
		Print("Closing all orders because OrdersTimeAlive protection.");
	}
	
	if (PreviousOpenOrders > OpenOrders) {
		for (cnt = OrdersTotal() - 1;cnt >= 0;cnt--) {
			if (OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES)) {
				mode = OrderType();
				
				if ((OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) || AllSymbolsProtect) {
					if (mode == OP_BUY || mode == OP_SELL) {
						OrderClose(OrderTicket(), OrderLots(), OrderClosePrice(), slippage);
						return;
					}
				}
			}
		}
		
		for (cnt = 0;cnt < OrdersTotal();cnt++) {
			if (OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES)) {
				mode = OrderType();
				
				if ((OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) || AllSymbolsProtect) {
					if (mode == OP_SELLLIMIT || mode == OP_BUYLIMIT || mode == OP_BUYSTOP || mode == OP_SELLSTOP) {
						OrderDelete(OrderTicket());
						return;
					}
				}
			}
		}
	}
	
	PreviousOpenOrders = OpenOrders;
	
	if (OpenOrders >= MaxTrades) {
		ContinueOpening = false;
	}
	
	else {
		ContinueOpening = true;
	}
	
	if (LastPrice == 0) {
		for (cnt = 0;cnt < OrdersTotal();cnt++) {
			if (OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES)) {
				mode = OrderType();
				
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
					LastPrice = OrderOpenPrice();
					
					if (mode == OP_BUY) {
						myOrderType = 2;
					}
					
					if (mode == OP_SELL) {
						myOrderType = 1;
					}
				}
			}
		}
	}
	
	// SecureProfit protection
	//if (SecureProfitProtection && MarketOpenOrders>=(MaxTrades-OrderstoProtect)
	// Modified to make easy to understand
	
	if (SecureProfitProtection && MarketOpenOrders >= OrderstoProtect) {
		Profit = 0;
		
		for (cnt = 0;cnt < OrdersTotal();cnt++) {
			if (OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES)) {
				if ((OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) || AllSymbolsProtect)
					Profit = Profit + OrderProfit();
			}
		}
		
		if (Profit >= SecureProfit) {
			text = text + "\nClosing orders because account protection with SecureProfit was triggered.";
			Print("Closing orders because account protection with SeureProfit was triggered.");
			PreviousOpenOrders = OpenOrders + 1;
			ContinueOpening = false;
			return;
		}
	}
	
	myOrderTypetmp = 3;
	
	switch (OpenOrdersBasedOn) {
	
	case 16:
		myOrderTypetmp = OpenOrdersBasedOnSTOCH();
		break;
		
	default:
		myOrderTypetmp = OpenOrdersBasedOnSTOCH();
		break;
	}
	
	
	
	
	if (OpenOrders < 1 && Manual == 0) {
		myOrderType = myOrderTypetmp;
		
		if (ReverseCondition) {
			if (myOrderType == 1) {
				myOrderType = 2;
			}
			
			else {
				if (myOrderType == 2) {
					myOrderType = 1;
				}
			}
		}
	}
	
	if (ReverseCondition) {
		if (myOrderTypetmp == 1) {
			myOrderTypetmp = 2;
		}
		
		else {
			if (myOrderTypetmp == 2) {
				myOrderTypetmp = 1;
			}
		}
	}
	
	// if we have opened positions we take care of them
	cnt = OrdersTotal() - 1;
	
	while (cnt >= 0) {
		if (OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES) == false)
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) { // && Reversed==false)
			//Print("Ticket ",OrderTicket()," modified.");
			if (OrderType() == OP_SELL) {
				if (ExitWithSTOCH && myOrderTypetmp == 2) {
					PreviousOpenOrders = OpenOrders + 1;
					ContinueOpening = false;
					text = text + "\nClosing all orders because Indicator triggered another signal.";
					Print("Closing all orders because Indicator triggered another signal.");
					//return(0);
				}
				
				if (TrailingStop > 0) {
					if ((OrderOpenPrice() - OrderClosePrice()) >= (TrailingStop*Point + Pips*Point)) {
						if (OrderStopLoss() > (OrderClosePrice() + TrailingStop*Point)) {
							result = OrderModify(OrderTicket(), OrderOpenPrice(), OrderClosePrice() + TrailingStop * Point, OrderClosePrice() - TakeProfit * Point - TrailingStop * Point);
							
							if (result != true)
								Print("LastError = " + GetLastError());
							
							return;
						}
					}
				}
			}
			
			if (OrderType() == OP_BUY) {
				if (ExitWithSTOCH && myOrderTypetmp == 1) {
					PreviousOpenOrders = OpenOrders + 1;
					ContinueOpening = false;
					text = text + "\nClosing all orders because Indicator triggered another signal.";
					Print("Closing all orders because Indicator triggered another signal.");
					//return(0);
				}
				
				if (TrailingStop > 0) {
					if ((OrderClosePrice() - OrderOpenPrice()) >= (TrailingStop*Point + Pips*Point)) {
						if (OrderStopLoss() < (OrderClosePrice() - TrailingStop*Point)) {
							result = OrderModify(OrderTicket(), OrderOpenPrice(), OrderClosePrice() - TrailingStop * Point, OrderClosePrice() + TakeProfit * Point + TrailingStop * Point);
							
							if (result != true)
								Print("LastError = " + GetLastError());
								
							return;
						}
					}
				}
			}
		}
		
		cnt--;
	}
	
	if (!IsTesting()) {
		if (myOrderType == 3 && OpenOrders < 1) {
			text = text + "\nTIADA KONDISI UTK OPEN ORDER";
		}
		
		//else { text= text + "\n                         "; }
		// Comment("HARGA TERAKHIR=",LastPrice," OPEN ODER YG LEPAS=",PreviousOpenOrders,"\nBUKA LAGI=",ContinueOpening," JENISORDER=",myOrderType,"\nLOT=",lotsi,text);
	}
	
	if (OpenOrders < 1)
		OpenMarketOrders();
	else
	if (SetLimitOrders)
		OpenLimitOrders();
	else
		OpenMarketOrders();
	
}



void StochPower::OpenMarketOrders() {
	int cnt = 0;
	
	if (myOrderType == 1 && ContinueOpening) {
		if ((Bid - LastPrice) >= Pips*Point || OpenOrders < 1) {
			SellPrice = Bid;
			LastPrice = 0;
			
			if (TakeProfit == 0) {
				tp = 0;
			}
			
			else {
				tp = SellPrice - TakeProfit * Point;
			}
			
			if (StopLoss == 0) {
				sl = 0;
			}
			
			else {
				sl = SellPrice + StopLoss * Point;
			}
			
			if (OpenOrders != 0) {
				mylotsi = lotsi;
				
				for (cnt = 0;cnt < OpenOrders;cnt++) {
					if (MaxTrades > 12) {
						mylotsi = NormalizeDouble(mylotsi * multiply, 2);
					}
					
					else {
						mylotsi = NormalizeDouble(mylotsi * multiply, 2);
					}
				}
			}
			
			else {
				mylotsi = lotsi;
			}
			
			if (mylotsi > 100) {
				mylotsi = 100;
			}
			
			OrderSend(Symbol(), OP_SELL, mylotsi, SellPrice, slippage, sl, tp, "", MagicNumber, 0);
			
			return;
		}
		
		//   Sleep(6000);   ////aku letak
		//      RefreshRates();
	}
	
	if (myOrderType == 2 && ContinueOpening) {
		if ((LastPrice - Ask) >= Pips*Point || OpenOrders < 1) {
			BuyPrice = Ask;
			LastPrice = 0;
			
			if (TakeProfit == 0) {
				tp = 0;
			}
			
			else {
				tp = BuyPrice + TakeProfit * Point;
			}
			
			if (StopLoss == 0)  {
				sl = 0;
			}
			
			else {
				sl = BuyPrice - StopLoss * Point;
			}
			
			if (OpenOrders != 0) {
				mylotsi = lotsi;
				
				for (cnt = 0;cnt < OpenOrders;cnt++) {
					if (MaxTrades > 12) {
						mylotsi = NormalizeDouble(mylotsi * multiply, 2);
					}
					
					else {
						mylotsi = NormalizeDouble(mylotsi * multiply, 2);
					}
				}
			}
			
			else {
				mylotsi = lotsi;
			}
			
			if (mylotsi > 100) {
				mylotsi = 100;
			}
			
			OrderSend(Symbol(), OP_BUY, mylotsi, BuyPrice, slippage, sl, tp, "", MagicNumber, 0);
			
			return;
		}
	}
}

void StochPower::OpenLimitOrders() {
	int cnt = 0;
	
	if (myOrderType == 1 && ContinueOpening) {
		//if ((Bid-LastPrice)>=Pips*Point || OpenOrders<1)
		//{
		//SellPrice=Bid;
		SellPrice = LastPrice + Pips * Point;
		LastPrice = 0;
		
		if (TakeProfit == 0) {
			tp = 0;
		}
		
		else {
			tp = SellPrice - TakeProfit * Point;
		}
		
		if (StopLoss == 0) {
			sl = 0;
		}
		
		else {
			sl = SellPrice + StopLoss * Point;
		}
		
		if (OpenOrders != 0) {
			mylotsi = lotsi;
			
			for (cnt = 0;cnt < OpenOrders;cnt++) {
				if (MaxTrades > 12) {
					mylotsi = NormalizeDouble(mylotsi * multiply, 2);
				}
				
				else {
					mylotsi = NormalizeDouble(mylotsi * multiply, 2);
				}
			}
		}
		
		else {
			mylotsi = lotsi;
		}
		
		if (mylotsi > 100) {
			mylotsi = 100;
		}
		
		OrderSend(Symbol(), OP_SELLLIMIT, mylotsi, SellPrice, slippage, sl, tp, "", MagicNumber, 0);
		
		return;
		//}
	}
	
	if (myOrderType == 2 && ContinueOpening) {
		//if ((LastPrice-Ask)>=Pips*Point || OpenOrders<1)
		//{
		//BuyPrice=Ask;
		BuyPrice = LastPrice - Pips * Point;
		LastPrice = 0;
		
		if (TakeProfit == 0) {
			tp = 0;
		}
		
		else {
			tp = BuyPrice + TakeProfit * Point;
		}
		
		if (StopLoss == 0)  {
			sl = 0;
		}
		
		else {
			sl = BuyPrice - StopLoss * Point;
		}
		
		if (OpenOrders != 0) {
			mylotsi = lotsi;
			
			for (cnt = 0;cnt < OpenOrders;cnt++) {
				if (MaxTrades > 12) {
					mylotsi = NormalizeDouble(mylotsi * multiply, 2);
				}
				
				else {
					mylotsi = NormalizeDouble(mylotsi * multiply, 2);
				}
			}
		}
		
		else {
			mylotsi = lotsi;
		}
		
		if (mylotsi > 100) {
			mylotsi = 100;
		}
		
		OrderSend(Symbol(), OP_BUYLIMIT, mylotsi, BuyPrice, slippage, sl, tp, "", MagicNumber, 0);
		
		return;
		//}
	}
	
}

void StochPower::DeleteAllObjects() {
	
}

// 16
int StochPower::OpenOrdersBasedOnSTOCH() {
	int myOrderType = 3;
	
	//---- long Stochastic indicator
	double Var4 = At(0).GetBuffer(0).Get(pos);
	
	//---- lot setting and modifications
//Sell order

	if (Bid  > Var4 + (90*Point)) {
		myOrderType = 1;//SELL
	}
	
//buy order

	if (Bid  < Var4 - (90 * Point)) {
		myOrderType = 2;//BUY
	}
	
	return(myOrderType);
}

}
