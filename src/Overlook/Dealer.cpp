#include "Overlook.h"


namespace Overlook {
	
Dealer::Dealer() {
	
}

void Dealer::InitEA() {
	// êîíòðîëü ïåðåìåííîé ïåðåä èñïîëüçîâàíèåì
	if (AutoLot)
		LotSize();
}

void Dealer::StartEA(int pos) {
	if ((TimeHour(TimeCurrent()) > TradeTime))
		cantrade = true;
	
	this->pos = pos;
	if (pos < t1 || pos < t2)
		return;
	
	// ïðîâåðÿåì åñòü ëè îòêðûòûå îðäåðà ...
	total = OrdersTotal();
	
	if (total < Orders) {
		// ... åñëè íåò íè îäíîãî îòêðûòîãî îðäåðà, òî èäåì äàëüøå
		// ïðîâåðÿåì íàñòàëî ëè âðåìÿ äëÿ òîðãîâëè
		if ((TimeHour(TimeCurrent()) == TradeTime) && (cantrade)) {
			// ... åñëè íàñòàëî âðåìÿ, òî
			ConstBuffer& open_buf = GetInputBuffer(0, 0);
			double o1 = open_buf.Get(pos - t1);
			double o2 = open_buf.Get(pos - t2);
			if (((o1 - o2) > delta_S*Point)) { //Åñëè öåíà óìåíüøèëàñü íà âåëè÷èíó delta
				//óñëîâèå âûïîëíåíî çíà÷èò âõîäèì â êîðîòêóþ ïîçèöèþ:
				// ïðîâåðÿåì åñòü ëè ñâîáîäíûå äåíüãè äëÿ îòêðûòèÿ êîðîòêîé ïîçèöèè
				if (AccountFreeMarginCheck(Symbol(), OP_SELL, lot) <= 0) {
					Print("Not enough money");
					return;
				}
				
				OpenShort(lot);
				
				cantrade = false; //çàïðåùàåì òîðãîâàòü ïîâòîðíî äî ñëåäóþùåãî áàðà
				return;
			}
			
			if (((o2 - o1) > delta_L*Point)) { //Åñëè öåíà èçìåíèëàñü íà âåëè÷èíó delta
				// óñëîâèå âûïîëíåíî çíà÷èò âõîäèì â äëèííóþ ïîçèöèþ
				// ïðîâåðÿåì, åñòü ëè ñâîáîäíûå äåíüãè íà ñ÷åòó
				if (AccountFreeMarginCheck(Symbol(), OP_BUY, lot) <= 0) {
					Print("Not enough money");
					return;
				}
				
				OpenLong(lot);
				
				cantrade = false;
				return;
			}
		}
	}
	
// áëîê ïðîâåðêè âðåìåíè æèçíè ñäåëêè, åñëè MaxOpenTime=0, òî ïðîâåðêó íå ïðîâîäèì.

	if (MaxOpenTime > 0) {
		for (cnt = 0;cnt < total;cnt++) {
			if (OrderSelect(cnt, SELECT_BY_POS, MODE_TRADES)) {
				tmp = (TimeCurrent() - OrderOpenTime()) / 3600.0;
				
				if (((NormalizeDouble(tmp, 8) - MaxOpenTime) >= 0)) {
					RefreshRates();
					
					if (OrderType() == OP_BUY)
						closeprice = Bid;
					else
						closeprice = Ask;
						
					if (OrderClose(OrderTicket(), OrderLots(), closeprice, 10)) {

					}
					
				}
			}
		}
	}
}



double Dealer::LotSize()
// ôóíêöèÿ îòêðûâàåò êîðîòêóþ ïîçèöèþ ðàçìåðîì ëîòà=volume
{
	return 0.01;
}

int Dealer::OpenLong(double volume)
// ôóíêöèÿ îòêðûâàåò äëèííóþ ïîçèöèþ ðàçìåðîì ëîòà=volume
{
	int slippage = 10;
	String comment = "20/200 expert v2 (Long)";
	int magic = 0;
			
	//  if (GlobalVariableGet("globalBalans")>AccountBalance()) if (AutoLot) LotSize();
	volume = lot * BigLotSize;
	
	ticket = OrderSend(Symbol(), OP_BUY, volume, Ask, slippage, Ask - StopLoss_L * Point,
					   Ask + TakeProfit_L * Point, comment, magic);
	                   
	
	
//  if (GlobalVariableGet("globalPosic")>25)
//  {
	
	if (AutoLot)
		LotSize();
		
//  }

	if (ticket > 0) {
		if (OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES)) {
			return(0);
		}
		
		else {
			return(-1);
		}
	}
	
	else {
		return(-1);
	}
}

int Dealer::OpenShort(double volume)
// ôóíêöèÿ îòêðûâàåò êîðîòêóþ ïîçèöèþ ðàçìåðîì ëîòà=volume
{
	int slippage = 10;
	String comment = "20/200 expert v2 (Short)";
	int magic = 0;
	
	volume = lot * BigLotSize;
		
	ticket = OrderSend(Symbol(), OP_SELL, volume, Bid, slippage, Bid + StopLoss_S * Point,
					   Bid - TakeProfit_S * Point, comment, magic);
	                   
	
	if (AutoLot)
		LotSize();
		
//  }

	if (ticket > 0) {
		if (OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES)) {
			return(0);
		}
		
		else {
			return(-1);
		}
	}
	
	else {
		return(-1);
	}
}

}
