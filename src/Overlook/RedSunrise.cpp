#include "Overlook.h"



namespace Overlook {

RedSunrise::RedSunrise() {
	
}

void RedSunrise::InitEA() {
	Spread = MarketInfo(Symbol(), MODE_SPREAD) * Point;
	
}

void RedSunrise::StartEA(int pos) {
	
	if (!pos)
		return;

	if (UseTrailingStop) {
		TrailingAlls(TrailStart, TrailStop, AveragePrice);
	}
	
	double CurrentPairProfit = CalculateProfit();
	
	if (UseEquityStop) {
		if (CurrentPairProfit < 0 && MathAbs(CurrentPairProfit) > (TotalEquityRisk / 100)*AccountEquityHigh()) {
			CloseThisSymbolAll();
			Print("Closed All due to Stop Out");
			NewOrdersPlaced = false;
		}
	}
	
	
	total = CountTrades();
	
	if (total == 0) {
		flag = 0;
	}
	
	double LastBuyLots;
	
	double LastSellLots;
	
	for (cnt = OrdersTotal() - 1;cnt >= 0;cnt--) {// ïîèñê ïîñëåäíåãî íàïðàâëåíèÿ
		OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
			if (OrderType() == OP_BUY) {
				LongTrade = true;
				ShortTrade = false;
				LastBuyLots = OrderLots();
				break;
			}
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
			if (OrderType() == OP_SELL) {
				LongTrade = false;
				ShortTrade = true;
				LastSellLots = OrderLots();
				break;
			}
	}
	
	if (total > 0 && total <= MaxTrades) {
		RefreshRates();
		LastBuyPrice = FindLastBuyPrice();
		LastSellPrice = FindLastSellPrice();
		
		if (LongTrade && (LastBuyPrice - Ask) >= (PipStep*Point)) {
			TradeNow = true;
		}
		
		if (ShortTrade && (Bid - LastSellPrice) >= (PipStep*Point)) {
			TradeNow = true;
		}
	}
	
	if (total < 1) {
		ShortTrade = false;
		LongTrade = false;
		TradeNow = true;
		StartEquity = AccountEquity();
	}
	
	if (TradeNow) {
		LastBuyPrice = FindLastBuyPrice();
		LastSellPrice = FindLastSellPrice();
		
		if (ShortTrade) {
			if (UseClose) {
				ForceOrderCloseMarket(false, true);
				iLots = NormalizeDouble(LotExponent * LastSellLots, LotsDigits);
			}
			
			else {
				iLots = GetLots(OP_SELL);
			}
			
			if (UseAdd) {
				NumOfTrades = total;
				
				if (iLots > 0) {
					RefreshRates();
					ticket = OpenPendingOrder(OP_SELL, iLots, Bid, slip, Ask, 0, 0, EAName + "-" + NumOfTrades, MagicNumber);
					
					if (ticket < 0) {
						Print("Error: " + GetLastError());
						return;
					}
					
					LastSellPrice = FindLastSellPrice();
					
					TradeNow = false;
					NewOrdersPlaced = true;
				}
			}
		}
		
		else
		if (LongTrade) {
			if (UseClose) {
				ForceOrderCloseMarket(true, false);
				iLots = NormalizeDouble(LotExponent * LastBuyLots, LotsDigits);
			}
			
			else {
				iLots = GetLots(OP_BUY);
			}
			
			if (UseAdd) {
				NumOfTrades = total;
				
				if (iLots > 0) {
					ticket = OpenPendingOrder(OP_BUY, iLots, Ask, slip, Bid, 0, 0, EAName + "-" + NumOfTrades, MagicNumber);
					
					if (ticket < 0) {
						Print("Error: " + GetLastError());
						return;
					}
					
					LastBuyPrice = FindLastBuyPrice();
					
					TradeNow = false;
					NewOrdersPlaced = true;
				}
			}
		}
	}
	
	if (TradeNow && total < 1) {
		ConstBuffer& open_buf = GetInputBuffer(0, 0);
		double PrevCl = open_buf.Get(pos - 1);
		double CurrCl = open_buf.Get(pos);
		SellLimit = Bid;
		BuyLimit = Ask;
		
		if (!ShortTrade && !LongTrade) {
			NumOfTrades = total;
			
			if (PrevCl > CurrCl) {
				iLots = GetLots(OP_SELL);
				
				if (iLots > 0) {
					ticket = OpenPendingOrder(OP_SELL, iLots, SellLimit, slip, SellLimit, 0, 0, EAName + "-" + NumOfTrades, MagicNumber);
					
					if (ticket < 0) {
						Print("Error: " + GetLastError());
						return;
					}
					
					LastBuyPrice = FindLastBuyPrice();
					
					NewOrdersPlaced = true;
				}
			}
			
			else {
				iLots = GetLots(OP_BUY);
				
				if (iLots > 0) {
					ticket = OpenPendingOrder(OP_BUY, iLots, BuyLimit, slip, BuyLimit, 0, 0, EAName + "-" + NumOfTrades, MagicNumber);
					
					if (ticket < 0) {
						Print("Error: " + GetLastError());
						return;
					}
					
					LastSellPrice = FindLastSellPrice();
					
					NewOrdersPlaced = true;
				}
			}
		}
		
		TradeNow = false;
	}
	
//----------------------- CALCULATE AVERAGE OPENING PRICE
	total = CountTrades();
	
	AveragePrice = 0;
	
	double Count = 0;
	
	for (cnt = OrdersTotal() - 1;cnt >= 0;cnt--) {
		OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
			if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
				AveragePrice = AveragePrice + OrderOpenPrice() * OrderLots();
				Count = Count + OrderLots();
			}
	}
	
	if (total > 0)
		AveragePrice = NormalizeDouble(AveragePrice / Count, Digits);
		
		
//----------------------- RECALCULATE STOPLOSS & PROFIT TARGET BASED ON AVERAGE OPENING PRICE
	if (NewOrdersPlaced)
		for (cnt = OrdersTotal() - 1;cnt >= 0;cnt--) {
			OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES);
			
			if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
				continue;
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
				if (OrderType() == OP_BUY) { // Calculate profit/stop target for long
					PriceTarget = AveragePrice + (TakeProfit * Point);
					BuyTarget = PriceTarget;
					Stopper = AveragePrice - (Stoploss * Point);
//      Stopper=0;
					flag = 1;
				}
				
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
				if (OrderType() == OP_SELL) { // Calculate profit/stop target for short
					PriceTarget = AveragePrice - (TakeProfit * Point);
					SellTarget = PriceTarget;
					Stopper = AveragePrice + (Stoploss * Point);
//      Stopper=0;
					flag = 1;
				}
		}
		
//----------------------- IF NEEDED CHANGE ALL OPEN ORDERS TO NEWLY CALCULATED PROFIT TARGET

	if (NewOrdersPlaced)
		if (flag == 1) { // check if average has really changed
			for (cnt = OrdersTotal() - 1;cnt >= 0;cnt--) {
//     PriceTarget=total;
				OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES);
				
				if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
					continue;
					
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
//      OrderModify(OrderTicket(),0,Stopper,PriceTarget,0,Yellow);// set all positions to averaged levels
					OrderModify(OrderTicket(), AveragePrice, OrderStopLoss(), PriceTarget);// set all positions to averaged levels
					
				NewOrdersPlaced = false;
			}
		}
}

double RedSunrise::ND(double v) {
	return(NormalizeDouble(v, Digits));
}

int RedSunrise::ForceOrderCloseMarket(bool aCloseBuy, bool aCloseSell) {
	int tErr = 0;
	
	for (int i = OrdersTotal() - 1;i >= 0;i--) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY && aCloseBuy) {
					RefreshRates();
					
					if (!OrderClose(OrderTicket(), OrderLots(), ND(Bid), 5)) {
						Print("Error close BUY " + OrderTicket());//+" "+fMyErDesc(GetLastError()));
						tErr = -1;
					}
				}
				
				if (OrderType() == OP_SELL && aCloseSell) {
					RefreshRates();
					
					if (!OrderClose(OrderTicket(), OrderLots(), ND(Ask), 5)) {
						Print("Error close SELL " + OrderTicket());//+" "+fMyErDesc(GetLastError()));
						tErr = -1;
					}
				}
			}
		}
	}
	
	return(tErr);
}


double RedSunrise::GetLots(int aTradeType) {
	double tLots;
	
	switch (MMType) {
	
	case 0:
		tLots = Lots;
		break;
		
	case 1:
		tLots = NormalizeDouble(Lots * MathPow(LotExponent, NumOfTrades), LotsDigits);
		break;
		
	case 2:
		Time LastClosedTime(3000,1,1);
		tLots = Lots;
		
		for (int i = OrdersHistoryTotal() - 1;i >= 0;i--) {
			if (OrderSelect(i, SELECT_BY_POS, MODE_HISTORY)) {
				if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
					if (LastClosedTime < OrderCloseTime()) {
						LastClosedTime = OrderCloseTime();
						
						if (OrderProfit() < 0) {
							tLots = NormalizeDouble(OrderLots() * LotExponent, LotsDigits);
						}
						
						else {
							tLots = Lots;
						}
					}
				}
			}
			
			else {
				return(-3);
			}
		}
		
		break;
	}
	
	if (AccountFreeMarginCheck(Symbol(), aTradeType, tLots) <= 0) {
		return(-1);
	}
	
	return(tLots);
}

int RedSunrise::CountTrades() {
	int count = 0;
	int trade;
	
	for (trade = OrdersTotal() - 1;trade >= 0;trade--) {
		OrderSelect(trade, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
			if (OrderType() == OP_SELL || OrderType() == OP_BUY)
				count++;
	}//for
	
	return(count);
}


void RedSunrise::CloseThisSymbolAll() {
	int trade;
	
	for (trade = OrdersTotal() - 1;trade >= 0;trade--) {
		OrderSelect(trade, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol())
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			if (OrderType() == OP_BUY)
				OrderClose(OrderTicket(), OrderLots(), Bid, slip);
				
			if (OrderType() == OP_SELL)
				OrderClose(OrderTicket(), OrderLots(), Ask, slip);
		}
		
		Sleep(1000);
	}
}

int RedSunrise::OpenPendingOrder(int pType, double pLots, double pLevel, int sp, double pr, int sl, int tp, String pComment, int pMagic) {
	int ticket = 0;
	int err = 0;
	int c = 0;
	int NumberOfTries = 100;
	
	switch (pType) {
	
	case OP_BUYLIMIT:
	
		ticket = OrderSend(Symbol(), OP_BUYLIMIT, pLots, pLevel, sp, StopLong(pr, sl), TakeLong(pLevel, tp), pComment, pMagic);
		
		
		break;
		
	case OP_BUYSTOP:
	
		ticket = OrderSend(Symbol(), OP_BUYSTOP, pLots, pLevel, sp, StopLong(pr, sl), TakeLong(pLevel, tp), pComment, pMagic);
		
		
		break;
		
	case OP_BUY:
	
		ticket = OrderSend(Symbol(), OP_BUY, pLots, Ask, sp, StopLong(Bid, sl), TakeLong(Ask, tp), pComment, pMagic);
		
		
		break;
		
	case OP_SELLLIMIT:
	
		ticket = OrderSend(Symbol(), OP_SELLLIMIT, pLots, pLevel, sp, StopShort(pr, sl), TakeShort(pLevel, tp), pComment, pMagic);
		
		
		break;
		
	case OP_SELLSTOP:
	
		ticket = OrderSend(Symbol(), OP_SELLSTOP, pLots, pLevel, sp, StopShort(pr, sl), TakeShort(pLevel, tp), pComment, pMagic);
		
		
		break;
		
	case OP_SELL:
	
		ticket = OrderSend(Symbol(), OP_SELL, pLots, Bid, sp, StopShort(Ask, sl), TakeShort(Bid, tp), pComment, pMagic);
		
		
		break;
	}
	
	return(ticket);
}

double RedSunrise::StopLong(double price, int stop) {
	if (stop == 0)
		return(0);
	else
		return(price -(stop*Point));
}

double RedSunrise::StopShort(double price, int stop) {
	if (stop == 0)
		return(0);
	else
		return(price + (stop*Point));
}

double RedSunrise::TakeLong(double price, int take) {
	if (take == 0)
		return(0);
	else
		return(price + (take*Point));
}

double RedSunrise::TakeShort(double price, int take) {
	if (take == 0)
		return(0);
	else
		return(price -(take*Point));
}


double RedSunrise::CalculateProfit() {

	double Profit = 0;
	
	for (cnt = OrdersTotal() - 1;cnt >= 0;cnt--) {
		OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber)
			if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
				Profit = Profit + OrderProfit();
			}
	}
	
	return(Profit);
}

void RedSunrise::TrailingAlls(int start, int stop, double AvgPrice) {
	int profit;
	double stoptrade;
	double stopcal;
	
	if (stop == 0)
		return;
		
	int trade;
	
	for (trade = OrdersTotal() - 1;trade >= 0;trade--) {
		if (!OrderSelect(trade, SELECT_BY_POS, MODE_TRADES))
			continue;
			
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() || OrderMagicNumber() == MagicNumber) {
			if (OrderType() == OP_BUY) {
				profit = NormalizeDouble((Bid - AvgPrice) / Point, 0);
				
				if (profit < start)
					continue;
					
				stoptrade = OrderStopLoss();
				
				stopcal = Bid - (stop * Point);
				
				if (stoptrade == 0 || (stoptrade != 0 && stopcal > stoptrade))
//     OrderModify(OrderTicket(),OrderOpenPrice(),stopcal,OrderTakeProfit(),0,Blue);
					OrderModify(OrderTicket(), AvgPrice, stopcal, OrderTakeProfit());
			}//Long
			
			if (OrderType() == OP_SELL) {
				profit = NormalizeDouble((AvgPrice - Ask) / Point, 0);
				
				if (profit < start)
					continue;
					
				stoptrade = OrderStopLoss();
				
				stopcal = Ask + (stop * Point);
				
				if (stoptrade == 0 || (stoptrade != 0 && stopcal < stoptrade))
//     OrderModify(OrderTicket(),OrderOpenPrice(),stopcal,OrderTakeProfit(),0,Red);
					OrderModify(OrderTicket(), AvgPrice, stopcal, OrderTakeProfit());
			}//Shrt
		}
		
		Sleep(1000);
	}//for
}




double RedSunrise::AccountEquityHigh() {
	static double AccountEquityHighAmt, PrevEquity;
	
	if (CountTrades() == 0)
		AccountEquityHighAmt = AccountEquity();
		
	if (AccountEquityHighAmt < PrevEquity)
		AccountEquityHighAmt = PrevEquity;
	else
		AccountEquityHighAmt = AccountEquity();
		
	PrevEquity = AccountEquity();
	
	return(AccountEquityHighAmt);
}


double RedSunrise::FindLastBuyPrice() {
	double oldorderopenprice = 0, orderprice;
	int cnt, oldticketnumber = 0, ticketnumber;
	
	for (cnt = OrdersTotal() - 1;cnt >= 0;cnt--) {
		OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber && OrderType() == OP_BUY) {
			ticketnumber = OrderTicket();
			
			if (ticketnumber > oldticketnumber) {
				orderprice = OrderOpenPrice();
				oldorderopenprice = orderprice;
				oldticketnumber = ticketnumber;
			}
		}
	}
	
	return(orderprice);
}

double RedSunrise::FindLastSellPrice() {
	double oldorderopenprice = 0, orderprice;
	int cnt, oldticketnumber = 0, ticketnumber;
	
	for (cnt = OrdersTotal() - 1;cnt >= 0;cnt--) {
		OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber && OrderType() == OP_SELL) {
			ticketnumber = OrderTicket();
			
			if (ticketnumber > oldticketnumber) {
				orderprice = OrderOpenPrice();
				oldorderopenprice = orderprice;
				oldticketnumber = ticketnumber;
			}
		}
	}
	
	return(orderprice);
}



}
