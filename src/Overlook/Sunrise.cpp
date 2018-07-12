#include "Overlook.h"

// proverka

namespace Overlook {
	
Sunrise::Sunrise() {
	
}

void Sunrise::InitEA() {
	
	LotDecimal = GetLotDecimal();
	
}

void Sunrise::StartEA(int pos) {
	this->pos = pos;
	
	if (pos < 1)
		return;
	
	int j;
	
	if (LotDecimal == 0)
		LotDecimal = GetLotDecimal();
		
	
	LongTradeNew = true;
	
	ShortTradeNew = true;
	
	/*if (UseInd)
	{
	
	   LongTradeNew=false;
	   ShortTradeNew=false;
	
	}*/
	//----
	
	if (timeprevMIN != Now) {
		CountTrades = GetCountTrades("buy");
		
		if (CountTrades == 0 && LongTradeNew == true) {
			CommentTrades = "Martini " + Symbol() + " - Buy " + (CountTrades + 1);
			Buy_NewLot = NewLot("buy");
			
			ticket = 0;
			
			for (j = 0; ticket < 1 && j < MaxAttempts; j++) {
				Print("Ïîêóïàåì. Íîâûé. Ïàðàìåòðû. ñèìâîë=" + Symbol() + ", ëîò=" + Buy_NewLot + ", öåíà=" + Ask + ", êîììåíò=" + CommentTrades + " ìàäæèê=" + MagicNumber);
				ticket = OrderSend(Symbol(), OP_BUY, Buy_NewLot, Ask, 3, 0, 0, CommentTrades, MagicNumber, 0, Blue);
				
				if (ticket < 0)
					Sleep(10000);
			}
			
			if (ticket < 0) {

			}
			
			else {
				RefreshRates();
			}
			
		}
		
		CountTrades = GetCountTrades("sell");
		
		if (CountTrades == 0 && ShortTradeNew == true) {
			CommentTrades = "Martini " + Symbol() + " - Sell " + (CountTrades + 1);
			Sell_NewLot = NewLot("sell");
			
			ticket = 0;
			
			for (j = 0; ticket < 1 && j < MaxAttempts; j++) {
				Print("Ïðîäàåì. Íîâûé. Ïàðàìåòðû. ñèìâîë=" + Symbol() + ", ëîò=" + Sell_NewLot + ", öåíà=" + Bid + ", êîììåíò=" + CommentTrades + " ìàäæèê=" + MagicNumber);
				ticket = OrderSend(Symbol(), OP_SELL, Sell_NewLot, Bid, 3, 0, 0, CommentTrades, MagicNumber, 0, Red);
				
				if (ticket < 0)
					Sleep(10000);
			}
			
			if (ticket < 0) {

			}
			
			else {
				RefreshRates();
			}
		}
		
		Norrect("buy");
		
		Norrect("sell");
		timeprevMIN = Now;
	}
	
	if (timeprevMAX != Now) {
		CountTrades = GetCountTrades("buy");
		
		if (CountTrades > 0 && NextOrder("buy")) {
			CommentTrades = "Martini " + Symbol() + " - Buy " + (CountTrades + 1);
			Buy_NextLot = NextLot("buy");
			ticket = 0;
			
			for (j = 0; ticket < 1 && j < MaxAttempts; j++) {
				Print("Ïîêóïàåì. Ñëåäóþùèé. Ïàðàìåòðû. ñèìâîë:" + Symbol() + ", ëîò=" + Buy_NextLot + ", öåíà =" + Ask + ", êîììåíò=" + CommentTrades + ", ìàäæèê=" + MagicNumber);
				ticket = OrderSend(Symbol(), OP_BUY, Buy_NextLot, Ask, 3, 0, 0, CommentTrades, MagicNumber, 0, Blue);
				
				if (ticket < 0)
					Sleep(10000);
			}
			
			if (ticket < 0) {

			}
			
			else {
				Buy_LastLot = Buy_NextLot;
				RefreshRates();
				Norrect("buy");
			}
		}
		
		CountTrades = GetCountTrades("sell");
		
		if (CountTrades > 0 && NextOrder("sell")) {
			CommentTrades = "Martini " + Symbol() + " - Sell " + (CountTrades + 1);
			Sell_NextLot = NextLot("sell");
			ticket = 0;
			
			for (j = 0; ticket < 1 && j < MaxAttempts; j++) {
				Print("Ïðîäàåì. Ñëåäóþùèé. Ïàðàìåòðû. ñèìâîë:" + Symbol() + ", ëîò=" + Sell_NextLot + ", öåíà=" + Bid + ", êîììåíò=" + CommentTrades + ", ìàäæèê=" + MagicNumber);
				ticket = OrderSend(Symbol(), OP_SELL, Sell_NextLot, Bid, 3, 0, 0, CommentTrades, MagicNumber, 0, Red);
				
				if (ticket < 0)
					Sleep(10000);
			}
			
			if (ticket < 0) {

			}
			
			else {
				Sell_LastLot = Sell_NextLot;
				RefreshRates();
				Norrect("sell");
			}
		}
		
		timeprevMAX = Now;
	}
	
}

//+------------------------------------------------------------------+
void Sunrise::Norrect(String OrdType) {
	int trade;
	double TP_all;
	double AveragePrice;
	
	AveragePrice = GetAveragePrice(OrdType);
	
	for (trade = OrdersTotal() - 1;trade >= 0;trade--) {
		OrderSelect(trade, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			//-----
			if (OrdType == "buy") {
				if (OrderType() == OP_BUY) {
					TP_all = AveragePrice + Buy_TP * Point;
					
					if (OrderTakeProfit() != TP_all) {

						OrderModify(OrderTicket(), AveragePrice, OrderStopLoss(), TP_all, 0, Yellow);
					}
				}
			}
			
			//----
			
			if (OrdType == "sell") {
				if (OrderType() == OP_SELL) {
					TP_all = AveragePrice - Sell_TP * Point;
					
					if (OrderTakeProfit() != TP_all) {
						OrderModify(OrderTicket(), AveragePrice, OrderStopLoss(), TP_all, 0, Yellow);
					}
					
				}
			}
			
			//----
		}
	}
}

double Sunrise::NewLot(String OrdType) {
	double tLots;
	double minlot = MarketInfo(Symbol(), MODE_MINLOT);
	
	if (OrdType == "buy") {
		if (FixLot)
			tLots = Buy_LotSize;
		else
			tLots = NormalizeDouble(Buy_LotSize * NormalizeDouble(AccountBalance() / LotStep, 0), LotDecimal);
	}
	
	if (OrdType == "sell") {
		if (FixLot)
			tLots = Sell_LotSize;
		else
			tLots = NormalizeDouble(Sell_LotSize * NormalizeDouble(AccountBalance() / LotStep, 0), LotDecimal);
	}
	
	if (tLots < minlot)
		tLots = minlot;
		
	return(tLots);
}

double Sunrise::NextLot(String OrdType) {

	double tLots;
	
	if (OrdType == "buy") {
		tLots = NormalizeDouble(FindLastOrder(OrdType, "Lots") * Buy_LotExponent, LotDecimal);
	}
	
	if (OrdType == "sell") {
		tLots = NormalizeDouble(FindLastOrder(OrdType, "Lots") * Sell_LotExponent, LotDecimal);
	}
	
	//tLots=NormalizeDouble(NewLot*MathPow(Sell_LotExponent,GetCountTrades(OrdType)),LotDecimal);
	return(tLots);
}

bool Sunrise::NextOrder(String OrdType) {
	bool NextOrd = false;
	int PipStepEX;
	
	CompatBuffer Open(GetInputBuffer(0, 0), pos);
	double CurrCl = Open[0];
	double CurrOp = Open[1];
	
	if (OrdType == "buy") {
		PipStepEX = NormalizeDouble(Buy_PipStep * MathPow(Buy_PipStepExponent, GetCountTrades(OrdType)), 0);
		
		if (FindLastOrder(OrdType, "Price") - Ask >= PipStepEX * Point && GetCountTrades(OrdType) < Buy_MaxTrades) {
			if (BackBar) {
				if (CurrOp <= CurrCl)
					NextOrd = true;
				else
					NextOrd = false;
			}
			
			else {
				NextOrd = true;
			}
		}
	}
	
	if (OrdType == "sell") {
		PipStepEX = NormalizeDouble(Sell_PipStep * MathPow(Sell_PipStepExponent, GetCountTrades(OrdType)), 0);
		
		if (Bid - FindLastOrder(OrdType, "Price") >= PipStepEX * Point && GetCountTrades(OrdType) < Sell_MaxTrades) {
			if (BackBar) {
				if (CurrOp >= CurrCl)
					NextOrd = true;
				else
					NextOrd = false;
			}
			
			else {
				NextOrd = true;
			}
		}
	}
	
	return(NextOrd);
}

double Sunrise::FindLastOrder(String OrdType, String inf) {
	double OrderPrice;
	double LastLot;
	int trade, oldticketnumber = 0;
	
	for (trade = OrdersTotal() - 1;trade >= 0;trade--) {
		OrderSelect(trade, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			//----
			if (OrdType == "buy") {
				if (OrderType() == OP_BUY) {
					if (OrderTicket() > oldticketnumber) {
						OrderPrice = OrderOpenPrice();
						LastLot = OrderLots();
						oldticketnumber = OrderTicket();
					}
				}
			}
			
			//----
			
			if (OrdType == "sell") {
				if (OrderType() == OP_SELL) {
					if (OrderTicket() > oldticketnumber) {
						OrderPrice = OrderOpenPrice();
						LastLot = OrderLots();
						oldticketnumber = OrderTicket();
					}
				}
			}
			
			//----
		}
	}
	
	if (inf == "Price")
		return(OrderPrice);
		
	if (inf == "Lots")
		return(LastLot);
	
	return 0;
}

int Sunrise::GetCountTrades(String OrdType) {
	int count = 0;
	int trade;
	
	for (trade = OrdersTotal() - 1;trade >= 0;trade--) {
		OrderSelect(trade, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			//----
			if (OrdType == "buy") {
				if (OrderType() == OP_BUY)
					count++;
			}
			
			//----
			
			if (OrdType == "sell") {
				if (OrderType() == OP_SELL)
					count++;
			}
			
			//----
		}
	}
	
	return(count);
}

double Sunrise::GetAveragePrice(String OrdType) {
	double AveragePrice = 0;
	double Count = 0;
	int trade;
	
	for (trade = OrdersTotal() - 1;trade >= 0;trade--) {
		OrderSelect(trade, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			//-----
			if (OrdType == "buy") {
				if (OrderType() == OP_BUY) {
					AveragePrice = AveragePrice + OrderOpenPrice() * OrderLots();
					Count = Count + OrderLots();
				}
			}
			
			//----
			
			if (OrdType == "sell") {
				if (OrderType() == OP_SELL) {
					AveragePrice = AveragePrice + OrderOpenPrice() * OrderLots();
					Count = Count + OrderLots();
				}
			}
			
			//----
		}
	}
	
	AveragePrice = NormalizeDouble(AveragePrice / Count, Digits);
	
	return(AveragePrice);
}

double Sunrise::Balance(String OrdType, String inf) {
	double result = 0;
	int trade;
	
	for (trade = OrdersTotal() - 1;trade >= 0;trade--) {
		OrderSelect(trade, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderSymbol() != Symbol() || OrderMagicNumber() != MagicNumber)
			continue;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			//-----
			if (OrdType == "buy") {
				if (OrderType() == OP_BUY) {
					if (inf == "Balance")
						result = result + OrderProfit() - OrderSwap() - OrderCommission();
						
					if (inf == "Lot")
						result = result + OrderLots();
				}
			}
			
			//----
			
			if (OrdType == "sell") {
				if (OrderType() == OP_SELL) {
					if (inf == "Balance")
						result = result + OrderProfit() - OrderSwap() - OrderCommission();
						
					if (inf == "Lot")
						result = result + OrderLots();
				}
			}
			
			//----
		}
	}
	
	return(result);
}

double Sunrise::GetLotDecimal() {
	double steplot = MarketInfo(Symbol(), MODE_LOTSTEP);
	int LotsDigits = MathCeil(MathAbs(MathLog(steplot) / MathLog(10)));
	
	return(LotsDigits);
}

}
