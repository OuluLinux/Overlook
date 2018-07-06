#include "Overlook.h"




namespace Overlook {

Snake::Snake() {
	
}

void Snake::InitEA() {
	
	AddSubCore<MovingAverage>()
		.Set("period", period);
	
}

void Snake::StartEA(int pos) {
	int order_count = OrdersTotal() - 1;
	
	double l_ima_8 = At(0).GetBuffer(0).Get(0);
	
	double l_minlot_16 = MarketInfo(Symbol(), MODE_MINLOT);
	
	double l_marginrequired_24 = MarketInfo(Symbol(), MODE_MARGINREQUIRED);
	
	double lots0 = l_minlot_16;
	
	int i;
	for (i = 0; lots0 < 1.0; i++)
		lots0 = 10.0 * lots0;
	minlots = NormalizeDouble(AccountFreeMargin() * Dynamic_lot / 1000.0 / l_marginrequired_24, i);
	
	if (minlots < l_minlot_16)
		minlots = l_minlot_16;
		
	int l_count_48 = 0;
	
	int l_count_56 = 0;
	
	for (int i = order_count; i >= 0; i--) {
		if (!OrderSelect(i, SELECT_BY_POS))
			continue;
		
		if (OrderMagicNumber() == MagicNumber && OrderSymbol() == Symbol())
			if (OrderType() < OP_BUYLIMIT)
				l_count_48++;
	}
	
	if (l_count_48 == 0) {
		for (int i = order_count; i >= 0; i--) {
			if (!OrderSelect(i, SELECT_BY_POS))
				continue;
			
			while (l_count_56 <= 25) {
				bool ret = OrderDelete(OrderTicket());
				
				if (ret) {
					Print("Close Limit Order.");
					break;
				}
				
				Print("OrderDelete failed " + Symbol() + ": " + GetLastError());
				
				Sleep(500);
				RefreshRates();
				l_count_56++;
			}
		}
	}
	
	bool li_64 = false;
	
	bool li_68 = false;
	double ld_88 = MarketInfo(Symbol(), MODE_STOPLEVEL);
	
	if (ld_88 < 15.0)
		ld_88 = 15;
		
	if (Ask > l_ima_8 + Op_step * Point)
		li_68 = true;
		
	if (Bid < l_ima_8 - Op_step * Point)
		li_64 = true;
		
	double ld_72 = NormalizeDouble(l_ima_8 - Op_step * Point, Digits);
	
	if (ld_72 > Bid - ld_88 * Point)
		ld_72 = Bid - ld_88 * Point;
		

	
	l_count_56 = 0;
	
	if (li_64 && l_count_48 == 0) {
		for (int i = 0; i < lim_setup; i++) {
			while (true) {
				int ret = OrderSend(Symbol(), OP_BUYLIMIT, minlots * (i + 1), ld_72 - lim_step * Point * i, 2, 0, 0, "Cobra ver 1.1", MagicNumber, 0);
				
				if (ret >= 0) {
					Print("Buy Limit open.");
					break;
				}
				
				Print("OrderSendFailed " + Symbol() + ": " + GetLastError());
				
				Sleep(5000);
				RefreshRates();
				l_count_56++;
				
				if (l_count_56 <= 25)
					continue;
					
				
				break;
			}
		}
	}
	
	double ld_80 = NormalizeDouble(l_ima_8 + Op_step * Point, Digits);
	
	if (ld_80 < Ask + ld_88 * Point)
		ld_80 = Ask + ld_88 * Point;
			
	l_count_56 = 0;
	
	if (li_68 && l_count_48 == 0) {
		for (int i = 0; i < lim_setup; i++) {
			while (true) {
				int ret = OrderSend(Symbol(), OP_SELLLIMIT, minlots * (i + 1), ld_80 + lim_step * Point * i, 2, 0, 0, "Cobra ver 1.1", MagicNumber, 0);
				
				if (ret >= 0) {
					Print("Sell Limit open.");
					break;
				}
				
				Print("OrderSend Failed " + Symbol() + ": " + GetLastError());
				
				Sleep(5000);
				RefreshRates();
				l_count_56++;
				
				if (l_count_56 <= 25)
					continue;
				
				break;
			}
		}
	}
	
	bool li_96 = false;
	
	double l_ord_open_price_100 = 1000000;
	double l_ord_open_price_108 = 0;
	double ld_116 = minlots;
	
	if (l_count_48 != count) {
		if (l_count_48 > count) {
			count = l_count_48;
			li_96 = true;
		}
		
		if (l_count_48 < count)
			count = l_count_48;
	}
	
	if (li_96 == true) {
		for (int i = order_count; i >= 0; i--) {
			OrderSelect(i, SELECT_BY_POS);
			
			if (OrderMagicNumber() == MagicNumber && OrderSymbol() == Symbol()) {
				if (OrderType() == OP_BUYLIMIT) {
					if (OrderOpenPrice() < l_ord_open_price_100) {
						l_ord_open_price_100 = OrderOpenPrice();
						ld_116 = OrderLots();
					}
				}
				
				if (OrderType() == OP_SELLLIMIT) {
					if (OrderOpenPrice() > l_ord_open_price_108) {
						l_ord_open_price_108 = OrderOpenPrice();
						ld_116 = OrderLots();
					}
				}
			}
		}
		
		l_count_56 = 0;
		
		if (l_ord_open_price_100 != 1000000.0) {
			while (true) {
				int ret = OrderSend(Symbol(), OP_BUYLIMIT, ld_116 + minlots, NormalizeDouble(l_ord_open_price_100, Digits) - lim_step * Point, 2, 0, 0, "Cobra ver 1.1", MagicNumber, 0);
				
				if (ret >= 0)
					Print("Buy Limit open.");
				else {
					Sleep(5000);
					RefreshRates();
					l_count_56++;
					
					Print("OrderSend Failed " + Symbol() + ": " + GetLastError());
					
					if (l_count_56 <= 25)
						continue;
				}
				
				break;
			}
		}
		
		l_count_56 = 0;
		
		if (l_ord_open_price_108 != 0.0) {
			while (true) {
				int ret = OrderSend(Symbol(), OP_SELLLIMIT, ld_116 + minlots, NormalizeDouble(l_ord_open_price_108, Digits) + lim_step * Point, 2, 0, 0, "Cobra ver 1.1", MagicNumber, 0);
				
				
				if (ret >= 0)
					Print("Sell Limit open.");
				else {
					Print("OrderSend Failed " + Symbol() + ": " + GetLastError());
					Sleep(5000);
					RefreshRates();
					l_count_56++;
					
					if (l_count_56 <= 25)
						continue;
				}
				
				break;
			}
			
		}
	}
	
	order_count = OrdersTotal() - 1;
	
	double ld_124 = 0;
	double ld_132 = 0;
	
	if (l_count_48 != 0) {
		for (i = order_count; i >= 0; i--) {
			if (!OrderSelect(i, SELECT_BY_POS))
				continue;
			
			if (OrderMagicNumber() == MagicNumber && OrderSymbol() == Symbol()) {
				if (OrderType() < OP_BUYLIMIT)
					ld_124 += OrderLots();
					
				ld_132 += OrderProfit() + OrderSwap();
			}
		}
		
		if (ld_132 >= l_marginrequired_24 * ld_124 / 2.0) {
			for (i = order_count; i >= 0; i--) {
				if (!OrderSelect(i, SELECT_BY_POS))
					continue;
				
				if (OrderMagicNumber() == MagicNumber && OrderSymbol() == Symbol()) {
					int type = OrderType();
					if (type == OP_BUY)
						CloseBuy();
						
					if (type == OP_SELL)
						CloseSell();
						
					if (type > OP_SELL)
						CloseLimit();
				}
			}
		}
	}
	        
}

void Snake::CloseBuy() {
	int l_count_8 = 0;
	
	while (true) {
		bool ret = OrderClose(OrderTicket(), OrderLots(), NormalizeDouble(MarketInfo(OrderSymbol(), MODE_BID), Digits), 2);
		
		if (ret) {
			Print("Close Buy.");
			return;
		}
		
		Print("OrderClose Failed " + Symbol() + ": " + GetLastError());
		
		Sleep(500);
		RefreshRates();
		l_count_8++;
		
		if (l_count_8 <= 25)
			continue;
			
		break;
	}
	
	Alert("Order failed to CLOSE - See Journal for errors");
}

void Snake::CloseSell() {
	int l_count_8 = 0;
	
	while (true) {
		bool ret = OrderClose(OrderTicket(), OrderLots(), NormalizeDouble(MarketInfo(OrderSymbol(), MODE_ASK), Digits), 2);
		
		if (ret) {
			Print("Close Sell. Ордер Селл закрыт успешно");
			return;
		}
		
		Print("OrderClose Failed " + Symbol() + ": " + GetLastError());
		
		Sleep(500);
		RefreshRates();
		l_count_8++;
		
		if (l_count_8 <= 25)
			continue;
			
		break;
	}
	
	Alert("Order failed to CLOSE - See Journal for errors");
}

void Snake::CloseLimit() {
	int l_count_8 = 0;
	
	while (true) {
		bool ret = OrderDelete(OrderTicket());
		
		if (ret) {
			Print("Close Limit Order. Лимит ордер закрыт");
			return;
		}
		
		Print("OrderClose Failed " + Symbol() + ": " + GetLastError());
		
		Sleep(500);
		RefreshRates();
		l_count_8++;
		
		if (l_count_8 <= 25)
			continue;
			
		break;
	}
	
	Alert("Order failed to CLOSE - See Journal for errors");
}

}
