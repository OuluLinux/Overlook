#include "Overlook.h"

namespace Overlook {

MoneyTime::MoneyTime() {
	
}

void MoneyTime::InitEA() {

	if (Digits == 5) {
		Slippage = Slippage * 10;
		Distance = Distance * 10;
		MinProfit =  MinProfit * 10;
		TrailingStop =  TrailingStop * 10;
		TrailingStep = TrailingStep * 10;
		MinProfitB = MinProfitB * 10;
		NoLossLevel = NoLossLevel * 10;
		
	}
	
	AddSubCore<AverageTrueRange>()
		.Set("period", RSIPeriod);
	
	AddSubCore<RelativeStrengthIndex>()
		.Set("period", RSIPeriod);
	
	AddSubCore<MovingAverage>()
		.Set("period", RSIMA);
	
	AddSubCore<RelativeStrengthIndex>()
		.Set("period", RSIPeriod2);
	
	AddSubCore<MovingAverage>()
		.Set("period", RSIMA2);
	
	AddSubCore<AverageTrueRange>()
		.Set("period", RSIMA);
	
	AddSubCore<RelativeStrengthIndex>()
		.Set("period", RSIPeriod3);
	
	AddSubCore<MovingAverage>()
		.Set("period", MAFilter);
	
	LongTicket = -1;
	ShortTicket = -1;
	buyOrders = 0;
	sellOrders = 0;
	allOrders = 0;
	CountBuyOrders = 0;
	CountSellOrders = 0;
}


void MoneyTime::StartEA(int pos) {
	this->pos = pos;
	int i;
	bool BuySignal = false;
	bool SellSignal = false;
	
	if (Trailing)
		TrailPositions();
		
	if (NoLoss)
		CreateNoLoss();
		
	//---- check for history and trading
	if (Bars < 100)
		return;
		
	GetCurrentOrders(Symbol());
	
	if (AccountFreeMargin() > 0)
		CheckForOpen();
	
}


int MoneyTime::GetCurrentOrders(String symbol) {

	int i = 0;
	buyOrders = 0;
	sellOrders = 0;
	allOrders = OrdersTotal();
	
	for (i = 0; i < allOrders; i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES) == false)
			break;
			
		if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
			if (OrderType() == OP_BUY)
				buyOrders++;
				
			if (OrderType() == OP_SELL)
				sellOrders++;
		}
	}
	
	
   if(buyOrders > 0) return( buyOrders );
      else return(-sellOrders);
	
}


double MoneyTime::GetLotSize() {
	double lot = 0;
	int orders = 0, losses = 0;
	
	if (!FixedLot) {
		lot = Lots;
		orders = HistoryTotal();     // history orders total
		losses = 0;                  // number of losses orders without a break
		lot = NormalizeDouble(AccountFreeMargin() * MaximumRisk / (MarketInfo(Symbol(), MODE_LOTSIZE) / 10), 2);
	}
	
	else {
		lot = Lots;
	}
	
	if (DecreaseFactor > 0) {
		for (int i = orders - 1; i >= 0; i--) {
			if (OrderSelect(i, SELECT_BY_POS, MODE_HISTORY) == false) {
				Print("Error in history!");
				break;
			}
			
			if (OrderSymbol() != Symbol() || OrderType() > OP_SELL)
				continue;
				
			//----
			if (OrderProfit() > 0)
				break;
				
			if (OrderProfit() < 0)
				losses++;
		}
		
		if (losses > 1)
			lot = NormalizeDouble(lot - lot * losses / DecreaseFactor, 2);
	}
	
	
	double MinLot = MarketInfo(Symbol(), MODE_MINLOT);
	
	double MaxLot = MarketInfo(Symbol(), MODE_MAXLOT);
	
	if (MinLot == 0)
		MinLot = 0.01;
		
	if (MaxLot == 0)
		MaxLot = 10000;
		
	if (lot < MinLot)
		lot = MinLot;
		
	if (lot > MaxLot)
		lot = MaxLot;
		
	//---- return lot size
	return(lot);
	
}

void MoneyTime::CheckForOpen() {
	double Lot;
	int    i;
	bool   tradeAllow = false;
	
	
	if (!TradeOnFriday)
		if (DayOfWeek() == 5)
			return;
			
			
	if (BaginTradeHour > 0 && EndTradeHour > 0) {
		if (Hour() >= BaginTradeHour || Hour() <= EndTradeHour)
			tradeAllow = true;
			
	}
	
	else {
		tradeAllow = true;
	}
	
	if (allOrders < AccountOrders || !AccountOrders && tradeAllow) {
		Lot = GetLotSize();
		double MaxLot = MarketInfo(Symbol(), MODE_MINLOT);
		double MinLot = MarketInfo(Symbol(), MODE_MAXLOT);
		double LotSize = MarketInfo(Symbol(), MODE_LOTSIZE);
		double LockLot;
		
		
		double atr, TakeProfit, StopLoss;
		
		if (Digits == 5)
			atr = At(0).GetBuffer(0).Get(pos) * 100000;
		else
			atr = At(0).GetBuffer(0).Get(pos) * 10000;
			
		TakeProfit = atr * ProfitFactor;
		
		StopLoss = atr * StopLossFactor;
		
		double SpreadPoints = MarketInfo(Symbol(), MODE_SPREAD) + 3;
		
		if (TakeProfit < SpreadPoints)
			TakeProfit = SpreadPoints;
			
		if (StopLoss < SpreadPoints)
			StopLoss = SpreadPoints;
			
			
		//---- buy signal
		if (buyOrders < 1) {
			if (GetBuySignal()) {
				LongTicket = OrderSend(Symbol(), OP_BUY, Lot, NormalizeDouble(Ask, Digits), Slippage, 0, 0, "", MagicNumber);
				
				if (LongTicket > 0) {
					OrderModify(LongTicket, Lot, NormalizeDouble(Ask - (StopLoss*Point), Digits), NormalizeDouble(Ask + (TakeProfit*Point), Digits));
					//buyOrders++;
				}
				
				
				
				return;
			}
		}
		
		else {
		
			//&& buyOrders < 2
			if (IsBuyLock()) {
				LockLot =  LongOrderPrevProfit / TakeProfit ;
				LockLot = NormalizeDouble(LockLot, 2);
				
				if (LockLot < MinLot)
					LockLot = MinLot;
					
				if (LockLot > MaxLot)
					LockLot = MaxLot;
					
				if (LockLot < Lot)
					LockLot = Lot;
					
					
					
				LongTicket = OrderSend(Symbol(), OP_BUY, LockLot, NormalizeDouble(Ask, Digits), Slippage, 0, 0, "", MagicNumber);
				
				if (LongTicket > 0) {
					OrderModify(LongTicket, LockLot, NormalizeDouble(Ask - (StopLoss*Point), Digits), NormalizeDouble(Ask + (TakeProfit*Point), Digits));
					//buyOrders++;
				}
				
				if (LongOrderPrevTP !=  NormalizeDouble(Ask + (TakeProfit*Point), Digits))
					OrderModify(LongOrderPrevTicket, LongOrderPrevLot, LongOrderPrevSL, NormalizeDouble(Ask + (TakeProfit*Point), Digits));
					
				return;
			}
			
			else {
				//if (AccountFreeMargin() > 0) {
				bool succ = OrderSelect(LongTicket, SELECT_BY_TICKET);
				
				if (succ && (MathAbs(OrderOpenPrice() - Ask) / Point) >=  Distance)
					if (GetBuySignal()) {
					
						LongTicket = OrderSend(Symbol(), OP_BUY, Lot, NormalizeDouble(Ask, Digits), Slippage, 0, 0, "", MagicNumber);
						
						if (LongTicket > 0) {
							OrderModify(LongTicket, Lot, NormalizeDouble(Ask - (StopLoss*Point), Digits), NormalizeDouble(Ask + (TakeProfit*Point), Digits));
							//buyOrders++;
						}
						
						return;
					}
					
				//}// if (AccountFreeMargin() > 0)
			}// else
			
		}// else
		
		
		
		//--------------- sell signal -----------------
		
		if (sellOrders < 1) {
			if (GetSellSignal()) {
			
				ShortTicket = OrderSend(Symbol(), OP_SELL, Lot, NormalizeDouble(Bid, Digits), Slippage, 0, 0, "", MagicNumber);
				
				if (ShortTicket > 0) {
					OrderModify(ShortTicket, Lot, NormalizeDouble(Bid + (StopLoss*Point), Digits), NormalizeDouble(Bid - (TakeProfit*Point), Digits));
					//sellOrders++;
				}
				
				
				return;
			}
		}
		
		else {
		
			//&& sellOrders < 2
			if (IsSellLock()) {
				LockLot =  ShortOrderPrevProfit / TakeProfit ;
				LockLot = NormalizeDouble(LockLot, 2);
				
				if (LockLot < MinLot)
					LockLot = MinLot;
					
				if (LockLot > MaxLot)
					LockLot = MaxLot;
					
				if (LockLot < Lot)
					LockLot = Lot;
					
				ShortTicket = OrderSend(Symbol(), OP_SELL, LockLot, NormalizeDouble(Bid, Digits), Slippage, 0, 0, "", MagicNumber);
				
				if (ShortTicket > 0) {
					OrderModify(ShortTicket, LockLot, NormalizeDouble(Bid + (StopLoss*Point), Digits), NormalizeDouble(Bid - (TakeProfit*Point), Digits));
					//sellOrders++;
				}
				
				if (ShortOrderPrevTP != NormalizeDouble(Bid - (TakeProfit*Point), Digits))
					OrderModify(ShortOrderPrevTicket, ShortOrderPrevLot, ShortOrderPrevSL, NormalizeDouble(Bid - (TakeProfit*Point), Digits));
					
				return;
			}
			
			else {
				//if (AccountFreeMargin() > 0) {
				if (IsOrderSelected() && (MathAbs(OrderOpenPrice() - Bid) / Point) >=  Distance)
					if (GetSellSignal()) {
						ShortTicket = OrderSend(Symbol(), OP_SELL, Lot, NormalizeDouble(Bid, Digits), Slippage, 0, 0, "", MagicNumber);
						
						if (ShortTicket > 0) {
							OrderModify(ShortTicket, Lot, NormalizeDouble(Bid + (StopLoss*Point), Digits), NormalizeDouble(Bid - (TakeProfit*Point), Digits));
							//sellOrders++;
						}
						
						return;
					}
					
				//}// if (AccountFreeMargin() > 0)
			} //else
			
		}// else
		
	}
	
//----
}




bool MoneyTime::IsBuyLock() {

	if (LongTicket > 0 && AccountFreeMargin() > 0) {
		if (!OrderSelect(LongTicket, SELECT_BY_TICKET))
			return false;
		LongOrderPrevProfit = OrderProfit();
		LongOrderPrevTicket = LongTicket;
		LongOrderPrevLot = OrderLots();
		LongOrderPrevSL = OrderStopLoss();
		LongOrderPrevTP = OrderTakeProfit();
		
		if (LongOrderPrevProfit < 0 && GetBuySignal() && MathAbs((OrderOpenPrice() - Ask) / Point) >= Distance) {
			return (true);
		}
		
		return (false);
	}
	return false;
}

bool MoneyTime::IsSellLock() {
	if (ShortTicket > 0 && AccountFreeMargin() > 0) {
		if (!OrderSelect(ShortTicket, SELECT_BY_TICKET))
			return false;
		ShortOrderPrevProfit = OrderProfit();
		ShortOrderPrevTicket = ShortTicket;
		ShortOrderPrevSL = OrderStopLoss();
		ShortOrderPrevTP = OrderTakeProfit();
		
		if (ShortOrderPrevProfit < 0  && GetSellSignal() &&  MathAbs((OrderOpenPrice() - Bid) / Point) >= Distance) {
			return (true);
		}
		
		return (false);
	}
	return false;
}


//------------------------------------------------------------------
// Check for buy conditions
//------------------------------------------------------------------
bool MoneyTime::GetBuySignal() {

	double RSInow = At(1).GetBuffer(0).Get(pos);
	double RSIprev = At(1).GetBuffer(0).Get(pos-1);
	double RSIMAnow = At(2).GetBuffer(0).Get(pos);
	double RSIMApre = At(2).GetBuffer(0).Get(pos-1);
	
	double RSInow2 = At(3).GetBuffer(0).Get(pos);
	double RSIprev2 = At(3).GetBuffer(0).Get(pos-1);
	double RSIMAnow2 = At(4).GetBuffer(0).Get(pos);
	double RSIMApre2 = At(4).GetBuffer(0).Get(pos-1);
	
	double ATRnow = At(5).GetBuffer(0).Get(pos);
	double ATRPrev = At(5).GetBuffer(0).Get(pos-1);
	
	double RSInowH1 = At(6).GetBuffer(0).Get(pos);
	
	double MAFilterNow = At(7).GetBuffer(0).Get(pos);
	double MAFilterPrev = At(7).GetBuffer(0).Get(pos-1);
	
	if (((MAFilterNow > MAFilterPrev && TrendFilter) || (!TrendFilter)) &&
		(RsiTeacher2 && (RSIprev2 < RSISellLevel2) && (RSInow2 > RSIprev2) && (RSInowH1 < RSIBuyLevel2) && (RSIMAnow2 > RSIMApre2)) ||
		(RsiTeacher && (RSIprev < RSISellLevel) && (RSInow > RSIprev) && (RSInowH1 < RSIBuyLevel) && (RSIMAnow > RSIMApre))
	   ) { //if
	   
		return (true);
		
	}//---- buy if -------
	
	
	return (false);
}

//------------------------------------------------------------------
// Check for close sell conditions
//------------------------------------------------------------------
bool MoneyTime::GetSellSignal() {

	double RSInow = At(1).GetBuffer(0).Get(pos);
	double RSIprev = At(1).GetBuffer(0).Get(pos-1);
	double RSIMAnow = At(2).GetBuffer(0).Get(pos);
	double RSIMApre = At(2).GetBuffer(0).Get(pos-1);
	
	double RSInow2 = At(3).GetBuffer(0).Get(pos);
	double RSIprev2 = At(3).GetBuffer(0).Get(pos-1);
	double RSIMAnow2 = At(4).GetBuffer(0).Get(pos);
	double RSIMApre2 = At(4).GetBuffer(0).Get(pos-1);
	
	double ATRnow = At(5).GetBuffer(0).Get(pos);
	double ATRPrev = At(5).GetBuffer(0).Get(pos-1);
	
	double RSInowH1 = At(6).GetBuffer(0).Get(pos);
	
	double MAFilterNow = At(7).GetBuffer(0).Get(pos);
	double MAFilterPrev = At(7).GetBuffer(0).Get(pos-1);
	
	//---- sell conditions----------
	
	if (((MAFilterNow < MAFilterPrev && TrendFilter) || (!TrendFilter)) &&
		(RsiTeacher2 && (RSIprev2 > RSIBuyLevel2) && (RSInow2 < RSIprev2) && (RSInowH1 > RSISellLevel2) && (RSIMAnow2 < RSIMApre2)) ||
		(RsiTeacher && (RSIprev > RSIBuyLevel) && (RSInow < RSIprev) && (RSInowH1 > RSISellLevel) && (RSIMAnow < RSIMApre))
	   )// if
	   
	{
	
		return (true);
		
	}//---- sell if ------
	
	return (false);
}


//------------------------------------------------------------------
//  Trail positions
//------------------------------------------------------------------
void MoneyTime::TrailPositions() {

	int orders = OrdersTotal();
	
	for (int i = 0; i < orders; i++) {
		if (!(OrderSelect(i, SELECT_BY_POS, MODE_TRADES)))
			continue;
			
		if (OrderSymbol() != Symbol())
			continue;
			
		{
			if (OrderType() == OP_BUY) {
				if (Bid - OrderOpenPrice() > MinProfit * Point)  {
					if (OrderStopLoss() < Bid - (TrailingStop + TrailingStep - 1) * Point)  {
						OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Bid - TrailingStop * Point, Digits), OrderTakeProfit());
					}
				}
			}
			
			if (OrderType() == OP_SELL) {
				if (OrderOpenPrice() - Ask > MinProfit * Point) {
					if (OrderStopLoss() > Ask + (TrailingStop + TrailingStep - 1) * Point) {
						OrderModify(OrderTicket(), OrderOpenPrice(), NormalizeDouble(Ask + TrailingStop * Point, Digits), OrderTakeProfit());
					}
				}
			}
		}
	}
	
}

//------------------------------------------------------------------
// No Losee
//------------------------------------------------------------------
void MoneyTime::CreateNoLoss() {

	int orders = OrdersTotal();
	
	for (int i = 0; i < orders; i++) {
		if (!(OrderSelect(i, SELECT_BY_POS, MODE_TRADES)))
			continue;
			
		if (OrderSymbol() != Symbol())
			continue;
			
		{
			if (OrderType() == OP_BUY && OrderStopLoss() < OrderOpenPrice()) {
				if (Bid - OrderOpenPrice() > MinProfitB*Point) {
					if (OrderStopLoss() < Bid - (NoLossLevel - 1)*Point) {
						OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + NoLossLevel * Point, OrderTakeProfit());
					}
				}
			}
			
			if (OrderType() == OP_BUY && OrderStopLoss() == 0) {
				if (Bid - OrderOpenPrice() > MinProfitB * Point) {
					if (OrderStopLoss() < Bid - (NoLossLevel - 1) * Point) {
						OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + NoLossLevel * Point, OrderTakeProfit());
					}
				}
			}
			
			if (OrderType() == OP_SELL && OrderStopLoss() > OrderOpenPrice()) {
				if (OrderOpenPrice() - Ask > MinProfitB*Point) {
					if (OrderStopLoss() > Ask + (NoLossLevel - 1)*Point) {
						OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - NoLossLevel * Point, OrderTakeProfit());
					}
				}
			}
			
			if (OrderType() == OP_SELL && OrderStopLoss() == 0) {
				if (OrderOpenPrice() - Ask > MinProfitB * Point) {
					if (OrderStopLoss() > Ask + (NoLossLevel - 1) * Point) {
						OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - NoLossLevel * Point, OrderTakeProfit());
					}
				}
			}
		}
	}
	
	
}

}

