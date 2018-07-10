#include "Overlook.h"

#if 0

// crazescalper

namespace Overlook {

ElectroBitch::ElectroBitch() {
	
}

void ElectroBitch::InitEA() {
	
}

void ElectroBitch::StartEA(int pos) {
	int li_0 = MarketInfo(Symbol(), MODE_STOPLEVEL);
	
	if (TakeProfit < li_0)
		TakeProfit = li_0;
		
	if (OR_Level < li_0)
		OR_Level = li_0;
		
	double ld_4 = li_0 * Point;
	
	int li_12 = MagicNumber + 1;
	
	int li_16 = MagicNumber + 2;
	
	double ld_20 = 0;
	
	double order_open_price_28 = -1;
	
	double order_lots_36 = -1;
	
	double order_open_price_44 = -1;
	
	double order_lots_52 = -1;
	
	double ld_60 = 0;
	
	double ld_68 = 0;
	
	int ticket_76 = -1;
	
	int ticket_80 = -1;
	
	double order_open_price_84 = -1;
	
	double order_lots_92 = -1;
	
	int ticket_100 = -1;
	
	int li_104 = 0;
	
	double order_open_price_108 = -1;
	
	double order_lots_116 = -1;
	
	int ticket_124 = -1;
	
	int li_128 = 0;
	
	trade = true;
	
	if (UseTradingHours) {
		Time currenttime = TimeHour(TimeCurrent());
		
		if (EndHour < StartHour) {
			EndHour += 24;
			
			if (currenttime < 24)
				currenttime += 24;
		}
		
		if (currenttime < StartHour || currenttime > EndHour)
			trade = false;
	}
	
	if (UseTradingHours)
		if (trade == false)
			Print(currenttime, "  ============ Trading is ***NOT***allowed now!");
		else
			Print(currenttime, "  ************ Trading ***IS ALLOWED*** now!!!");
			
	for (int pos_132 = 0; pos_132 <= OrdersTotal() - 1; pos_132++) {
		if (OrderSelect(pos_132, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderMagicNumber() == MagicNumber) {
					if (OrderType() == OP_BUY) {
						order_open_price_28 = OrderOpenPrice();
						order_lots_36 = OrderLots();
						ticket_76 = OrderTicket();
						ld_60 += OrderProfit() + OrderCommission() + OrderSwap();
					}
					
					if (OrderType() == OP_SELL) {
						order_open_price_44 = OrderOpenPrice();
						order_lots_52 = OrderLots();
						ticket_80 = OrderTicket();
						ld_68 += OrderProfit() + OrderCommission() + OrderSwap();
					}
				}
				
				if (OrderMagicNumber() == li_12) {
					order_open_price_84 = OrderOpenPrice();
					order_lots_92 = OrderLots();
					ticket_100 = OrderTicket();
					li_104 += OrderProfit() + OrderCommission() + OrderSwap();
				}
				
				if (OrderMagicNumber() == li_16) {
					order_open_price_108 = OrderOpenPrice();
					order_lots_116 = OrderLots();
					ticket_124 = OrderTicket();
					li_128 += OrderProfit() + OrderCommission() + OrderSwap();
				}
			}
		}
	}
	
	if (A(ld_60, li_104, f0_11(Lotos))) {
		if (ticket_76 > 0 && ticket_100 > 0)
			f0_5(ticket_76);
			
		f0_0(Symbol(), -1, MagicNumber + 1);
		
		f0_6(Symbol(), -1, MagicNumber + 1);
	}
	
	if (B(ticket_100, Bid, order_open_price_84, AV_Level, Point))
		f0_7(Symbol(), OP_SELLSTOP, NormalizeDouble(order_lots_92 * koef_av, lot_digits), NormalizeDouble(Bid - OR_Level * Point, Digits), 0, 0, MagicNumber + 1);
		
	if (C(ticket_100, ticket_76, order_open_price_28, Ask, Lock_Level, Point))
		f0_8(NULL, OP_SELL, NormalizeDouble(order_lots_36 * koef_l, lot_digits), 0, 0, MagicNumber + 1);
		
	if (ticket_76 < 0) {
		if (TakeProfit > 0)
			ld_20 = Ask + TakeProfit * Point;
		else
			ld_20 = 0;
			
		if (Lotos == 0.0)
			Lotos = f0_4(OP_BUY);
			
		f0_8(NULL, OP_BUY, Lotos, 0, ld_20, MagicNumber);
	}
	
	if (A(ld_68, li_128, f0_11(Lotos))) {
		if (ticket_80 > 0 && ticket_124 > 0)
			f0_5(ticket_80);
			
		f0_0(Symbol(), -1, MagicNumber + 2);
		
		f0_6(Symbol(), -1, MagicNumber + 2);
	}
	
	if (B(ticket_124, order_open_price_108, Ask, AV_Level, Point))
		f0_7(Symbol(), OP_BUYSTOP, NormalizeDouble(order_lots_116 * koef_av, lot_digits), NormalizeDouble(Ask + OR_Level * Point, Digits), 0, 0, MagicNumber + 2);
		
	if (C(ticket_124, ticket_80, Bid, order_open_price_44, Lock_Level, Point))
		f0_8(NULL, OP_BUY, NormalizeDouble(order_lots_52 * koef_l, lot_digits), 0, 0, MagicNumber + 2);
		
	if (ticket_80 < 0) {
		if (TakeProfit > 0)
			ld_20 = Bid - TakeProfit * Point;
		else
			ld_20 = 0;
			
		if (Lotos == 0.0)
			Lotos = f0_4(OP_SELL);
			
		f0_8(NULL, OP_SELL, Lotos, 0, ld_20, MagicNumber);
	}
	
	return (0);
}

void ElectroBitch::f0_8(string a_symbol_0, int a_cmd_8, double ad_12, double a_price_20, double a_price_28, int a_magic_36, string as_40) {
	color color_48;
	int datetime_52;
	double price_56;
	double price_64;
	double price_72;
	int digits_80;
	int error_84;
	int ticket_92 = 0;
	string comment_96 = as_40 + "DigitalSaleShop.com  /" + WindowExpertName() + " " + f0_9(Period());
	
	if (a_symbol_0 == "" || a_symbol_0 == "0")
		a_symbol_0 = Symbol();
		
	if (a_cmd_8 == OP_BUY)
		color_48 = Lime;
	else
		color_48 = Red;
		
	for (int li_88 = 1; li_88 <= 5; li_88++) {
		if (!IsTesting() && (!IsExpertEnabled()) || IsStopped()) {
			if (Log)
				Print("OpenPosition(): Stopping function");
				
			break;
		}
		
		while (!IsTradeAllowed())
			Sleep(5000);
			
		RefreshRates();
		
		digits_80 = MarketInfo(a_symbol_0, MODE_DIGITS);
		
		price_64 = MarketInfo(a_symbol_0, MODE_ASK);
		
		price_72 = MarketInfo(a_symbol_0, MODE_BID);
		
		if (a_cmd_8 == OP_BUY)
			price_56 = price_64;
		else
			price_56 = price_72;
			
		price_56 = NormalizeDouble(price_56, digits_80);
		
		datetime_52 = TimeCurrent();
		
		if (AccountFreeMarginCheck(Symbol(), a_cmd_8, ad_12) <= 0.0 || GetLastError() == 134/* NOT_ENOUGH_MONEY */) {
			//Alert("ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ", f0_10(a_cmd_8), ", ï¿½ï¿½ï¿½ï¿½ï¿½=", ad_12, ", ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.");
			return;
		}
		
		if (trade) {
			if (MarketWatch)
				ticket_92 = OrderSend(a_symbol_0, a_cmd_8, ad_12, price_56, Slippage, 0, 0, comment_96, a_magic_36, 0, color_48);
			else
				ticket_92 = OrderSend(a_symbol_0, a_cmd_8, ad_12, price_56, Slippage, a_price_20, a_price_28, comment_96, a_magic_36, 0, color_48);
		}
		
		if (ticket_92 > 0) {
			PlaySound("ok");
			break;
		}
		
		error_84 = GetLastError();
		
		if (price_64 == 0.0 && price_72 == 0.0)
			f0_3("Check in the Review of the market presence of the symbol " + a_symbol_0);
			
		if (Log)
			Print("Error(", error_84, ") opening position: ", ErrorDescription(error_84), ", try ", li_88);
			
		if (Log)
			Print("Ask=", price_64, " Bid=", price_72, " sy=", a_symbol_0, " ll=", ad_12, " op=", f0_10(a_cmd_8), " pp=", price_56, " sl=", a_price_20, " tp=", a_price_28, " mn=",
				  a_magic_36);
			      
//      if (error_84 == 2/* COMMON_ERROR */ || error_84 == 64/* ACCOUNT_DISABLED */ || error_84 == 65/* INVALID_ACCOUNT */ || error_84 == 133/* TRADE_DISABLED */) {
//         gi_unused_156 = true;
//         break;
//      }
		if (error_84 == 4/* SERVER_BUSY */ || error_84 == 131/* INVALID_TRADE_VOLUME */ || error_84 == 132/* MARKET_CLOSED */) {
			Sleep(300000);
			break;
		}
		
		if (error_84 == 128/* TRADE_TIMEOUT */ || error_84 == 142 || error_84 == 143) {
			Sleep(66666.0);
			
			if (f0_2(a_symbol_0, a_cmd_8, a_magic_36, datetime_52)) {
				PlaySound("ok");
				break;
			}
		}
		
		if (error_84 == 140/* LONG_POSITIONS_ONLY_ALLOWED */ || error_84 == 148/* TRADE_TOO_MANY_ORDERS */ || error_84 == 4110/* LONGS__NOT_ALLOWED */ || error_84 == 4111/* SHORTS_NOT_ALLOWED */)
			break;
			
		if (error_84 == 141/* TOO_MANY_REQUESTS */)
			Sleep(100000);
			
		if (error_84 == 145/* TRADE_MODIFY_DENIED */)
			Sleep(17000);
			
		if (error_84 == 146/* TRADE_CONTEXT_BUSY */)
			while (IsTradeContextBusy())
				Sleep(11000);
				
		if (error_84 != 135/* PRICE_CHANGED */)
			Sleep(7700.0);
	}
	
	if (MarketWatch && ticket_92 > 0 && a_price_20 > 0.0 || a_price_28 > 0.0)
		if (OrderSelect(ticket_92, SELECT_BY_TICKET))
			f0_12(-1, a_price_20, a_price_28);
}

void ElectroBitch::f0_5(int a_ticket_0) {
	bool is_closed_4;
	color color_8;
	double order_lots_12;
	double price_20;
	double price_28;
	double price_36;
	int error_44;
	
	if (OrderSelect(a_ticket_0, SELECT_BY_TICKET, MODE_TRADES)) {
		for (int li_48 = 1; li_48 <= 5; li_48++) {
			if (!IsTesting() && (!IsExpertEnabled()) || IsStopped())
				break;
				
			while (!IsTradeAllowed())
				Sleep(5000);
				
			RefreshRates();
			
			price_20 = NormalizeDouble(MarketInfo(OrderSymbol(), MODE_ASK), Digits);
			
			price_28 = NormalizeDouble(MarketInfo(OrderSymbol(), MODE_BID), Digits);
			
			if (OrderType() == OP_BUY) {
				price_36 = price_28;
				color_8 = Aqua;
			}
			
			else {
				price_36 = price_20;
				color_8 = Gold;
			}
			
			order_lots_12 = OrderLots();
			
			is_closed_4 = OrderClose(OrderTicket(), order_lots_12, price_36, Slippage, color_8);
			
			if (is_closed_4) {
				PlaySound("tick");
				return;
			}
			
			error_44 = GetLastError();
			
			if (error_44 == 146/* TRADE_CONTEXT_BUSY */)
				while (IsTradeContextBusy())
					Sleep(11000);
					
			if (Log)
				Print("Error(", error_44, ") Close ", OrderType(), " ", ", try ", li_48);
				
			if (Log)
				Print(OrderTicket(), "  Ask=", price_20, "  Bid=", price_28, "  pp=", price_36);
				
			if (Log)
				Print("sy=", OrderSymbol(), "  ll=", order_lots_12, "  sl=", OrderStopLoss(), "  tp=", OrderTakeProfit(), "  mn=", OrderMagicNumber());
				
			Sleep(5000);
		}
	}
	
	else
		if (Log)
			Print("Incorrect trade operation. Close ", OrderType());
}

double ElectroBitch::f0_11(double ad_0) {
	int digits_8 = Digits;
	double ld_12 = 100;
	
	if (digits_8 == 3 || digits_8 >= 5)
		ld_12 = 1000;
		
	double li_ret_20 = 1000.0 * ad_0 * TakeProfit_Av / ld_12;
	
	return (li_ret_20);
}

void ElectroBitch::f0_0(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	int order_total_20 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int pos_16 = order_total_20 - 1; pos_16 >= 0; pos_16--) {
		if (OrderSelect(pos_16, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL)
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
						f0_5(OrderTicket());
			}
		}
	}
}

void ElectroBitch::f0_7(string a_symbol_0, int a_cmd_8, double ad_12, double a_price_20, double a_price_28 = 0.0, double a_price_36 = 0.0, int a_magic_44 = 0, int a_datetime_48 = 0) {
	color color_52;
	int datetime_56;
	double ask_60;
	double bid_68;
	double point_76;
	int error_84;
	int ticket_92;
	int cmd_100;
	
	string comment_104 = "YTG";
	
	if (a_symbol_0 == "" || a_symbol_0 == "0")
		a_symbol_0 = Symbol();
		
	int stoplevel_96 = MarketInfo(a_symbol_0, MODE_STOPLEVEL);
	
	if (a_cmd_8 == OP_BUYLIMIT || a_cmd_8 == OP_BUYSTOP) {
		color_52 = Lime;
		cmd_100 = 0;
	}
	
	else {
		color_52 = Red;
		cmd_100 = 1;
	}
	
	if (a_datetime_48 > 0 && a_datetime_48 < TimeCurrent())
		a_datetime_48 = 0;
		
	for (int li_88 = 1; li_88 <= 5; li_88++) {
		if (!IsTesting() && (!IsExpertEnabled()) || IsStopped()) {
			if (Log)
				Print("SetOrder(): Stop");
				
			return;
		}
		
		while (!IsTradeAllowed())
			Sleep(5000);
			
		RefreshRates();
		
		datetime_56 = TimeCurrent();
		
		if (AccountFreeMarginCheck(Symbol(), cmd_100, ad_12) <= 0.0 || GetLastError() == 134/* NOT_ENOUGH_MONEY */) {
			//Alert(WindowExpertName() + " " + Symbol(), " ", " ", "For opening a position ", a_cmd_8, ", Lots=", ad_12, ", The free means do not suffice.");
			return;
		}
		
		if (trade)
			ticket_92 = OrderSend(a_symbol_0, a_cmd_8, ad_12, a_price_20, Slippage, a_price_28, a_price_36, comment_104, a_magic_44, a_datetime_48, color_52);
			
		if (ticket_92 > 0) {
			PlaySound("ok");
			return;
		}
		
		error_84 = GetLastError();
		
		if (error_84 == 128/* TRADE_TIMEOUT */ || error_84 == 142 || error_84 == 143) {
			Sleep(66000);
			
			if (f0_1(a_symbol_0, a_cmd_8, a_magic_44, datetime_56)) {
				PlaySound("alert2");
				return;
			}
			
			if (Log)
				Print("Error(", error_84, ") set order: ", error_84, ", try ", li_88);
		}
		
		else {
			point_76 = MarketInfo(a_symbol_0, MODE_POINT);
			ask_60 = MarketInfo(a_symbol_0, MODE_ASK);
			bid_68 = MarketInfo(a_symbol_0, MODE_BID);
			
			if (error_84 == 130/* INVALID_STOPS */) {
				switch (a_cmd_8) {
				
				case OP_BUYLIMIT:
				
					if (a_price_20 > ask_60 - stoplevel_96 * point_76)
						a_price_20 = ask_60 - stoplevel_96 * point_76;
						
					if (a_price_28 > a_price_20 - (stoplevel_96 + 1) * point_76)
						a_price_28 = a_price_20 - (stoplevel_96 + 1) * point_76;
						
					if (!(a_price_36 > 0.0 && a_price_36 < a_price_20 + (stoplevel_96 + 1) * point_76))
						break;
						
					a_price_36 = a_price_20 + (stoplevel_96 + 1) * point_76;
					
					break;
					
				case OP_BUYSTOP:
					if (a_price_20 < ask_60 + (stoplevel_96 + 1) * point_76)
						a_price_20 = ask_60 + (stoplevel_96 + 1) * point_76;
						
					if (a_price_28 > a_price_20 - (stoplevel_96 + 1) * point_76)
						a_price_28 = a_price_20 - (stoplevel_96 + 1) * point_76;
						
					if (!(a_price_36 > 0.0 && a_price_36 < a_price_20 + (stoplevel_96 + 1) * point_76))
						break;
						
					a_price_36 = a_price_20 + (stoplevel_96 + 1) * point_76;
					
					break;
					
				case OP_SELLLIMIT:
					if (a_price_20 < bid_68 + stoplevel_96 * point_76)
						a_price_20 = bid_68 + stoplevel_96 * point_76;
						
					if (a_price_28 > 0.0 && a_price_28 < a_price_20 + (stoplevel_96 + 1) * point_76)
						a_price_28 = a_price_20 + (stoplevel_96 + 1) * point_76;
						
					if (a_price_36 <= a_price_20 - (stoplevel_96 + 1) * point_76)
						break;
						
					a_price_36 = a_price_20 - (stoplevel_96 + 1) * point_76;
					
					break;
					
				case OP_SELLSTOP:
					if (a_price_20 > bid_68 - stoplevel_96 * point_76)
						a_price_20 = bid_68 - stoplevel_96 * point_76;
						
					if (a_price_28 > 0.0 && a_price_28 < a_price_20 + (stoplevel_96 + 1) * point_76)
						a_price_28 = a_price_20 + (stoplevel_96 + 1) * point_76;
						
					if (a_price_36 <= a_price_20 - (stoplevel_96 + 1) * point_76)
						break;
						
					a_price_36 = a_price_20 - (stoplevel_96 + 1) * point_76;
				}
				
				if (Log)
					Print("SetOrder(): The price levels are corrected");
			}
			
			if (Log)
				Print("Error(", error_84, ") set order: ", error_84, ", try ", li_88);
				
			if (Log)
				Print("Ask=", ask_60, "  Bid=", bid_68, "  sy=", a_symbol_0, "  ll=", ad_12, "  op=", a_cmd_8, "  pp=", a_price_20, "  sl=", a_price_28, "  tp=", a_price_36, "  mn=",
					  a_magic_44);
				      
			if (ask_60 == 0.0 && bid_68 == 0.0)
				if (Log)
					Print("SetOrder(): Check up in the review of the market presence of a symbol " + a_symbol_0);
					
			if (error_84 == 4/* SERVER_BUSY */ || error_84 == 131/* INVALID_TRADE_VOLUME */ || error_84 == 132/* MARKET_CLOSED */) {
				Sleep(300000);
				return;
			}
			
			if (error_84 == 8/* TOO_FREQUENT_REQUESTS */ || error_84 == 141/* TOO_MANY_REQUESTS */)
				Sleep(100000);
				
			if (error_84 == 139/* ORDER_LOCKED */ || error_84 == 140/* LONG_POSITIONS_ONLY_ALLOWED */ || error_84 == 148/* TRADE_TOO_MANY_ORDERS */)
				break;
				
			if (error_84 == 146/* TRADE_CONTEXT_BUSY */)
				while (IsTradeContextBusy())
					Sleep(11000);
					
			if (error_84 == 147/* TRADE_EXPIRATION_DENIED */)
				a_datetime_48 = 0;
			else
				if (error_84 != 135/* PRICE_CHANGED */ && error_84 != 138/* REQUOTE */)
					Sleep(7700.0);
		}
	}
	
}

int ElectroBitch::f0_1(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1, int ai_16 = 0) {
	int cmd_28;
	int order_total_24 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int pos_20 = 0; pos_20 < order_total_24; pos_20++) {
		if (OrderSelect(pos_20, SELECT_BY_POS, MODE_TRADES)) {
			cmd_28 = OrderType();
			
			if (cmd_28 > OP_SELL && cmd_28 < 6) {
				if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || cmd_28 == a_cmd_8) {
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
						if (ai_16 <= OrderOpenTime())
							return (1);
				}
			}
		}
	}
	
	return (0);
}

void ElectroBitch::f0_6(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	bool is_deleted_16;
	int error_20;
	int cmd_36;
	int order_total_32 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int pos_24 = order_total_32 - 1; pos_24 >= 0; pos_24--) {
		if (OrderSelect(pos_24, SELECT_BY_POS, MODE_TRADES)) {
			cmd_36 = OrderType();
			
			if (cmd_36 > OP_SELL && cmd_36 < 6) {
				if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || cmd_36 == a_cmd_8) {
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
						for (int li_28 = 1; li_28 <= 5; li_28++) {
							if (!IsTesting() && (!IsExpertEnabled()) || IsStopped())
								break;
								
							while (!IsTradeAllowed())
								Sleep(5000);
								
							is_deleted_16 = OrderDelete(OrderTicket(), White);
							
							if (is_deleted_16) {
								PlaySound("timeout");
								break;
							}
							
							error_20 = GetLastError();
							
							if (Log)
								Print("Error(", error_20, ") delete order ", cmd_36, ": ", error_20, ", try ", li_28);
								
							Sleep(5000);
						}
					}
				}
			}
		}
	}
}

double ElectroBitch::f0_4(int a_cmd_0 = 0) {
	double free_magrin_4 = AccountFreeMargin();
	
	if (Choice_method)
		free_magrin_4 = AccountBalance();
		
	double ld_12 = MarketInfo(Symbol(), MODE_MINLOT);
	
	double ld_20 = MarketInfo(Symbol(), MODE_MAXLOT);
	
	double ld_28 = free_magrin_4 * (100 - Risk) / 100.0;
	
	for (double minlot_36 = ld_12; AccountFreeMarginCheck(Symbol(), a_cmd_0, minlot_36) > ld_28; minlot_36 += ld_12) {
	}
	
	if (minlot_36 > ld_20)
		minlot_36 = ld_20;
		
	return (minlot_36);
}

void ElectroBitch::f0_3(string as_0) {
	Comment(as_0);
	
	if (StringLen(as_0) > 0)
		if (Log)
			Print(as_0);
}

string ElectroBitch::f0_9(int a_timeframe_0 = 0) {
	if (a_timeframe_0 == 0)
		a_timeframe_0 = Period();
		
	switch (a_timeframe_0) {
	
	case PERIOD_M1:
		return ("M1");
		
	case PERIOD_M5:
		return ("M5");
		
	case PERIOD_M15:
		return ("M15");
		
	case PERIOD_M30:
		return ("M30");
		
	case PERIOD_H1:
		return ("H1");
		
	case PERIOD_H4:
		return ("H4");
		
	case PERIOD_D1:
		return ("Daily");
		
	case PERIOD_W1:
		return ("Weekly");
		
	case PERIOD_MN1:
		return ("Monthly");
	}
	
	return ("UnknownPeriod");
}

string ElectroBitch::f0_10(int ai_0) {
	switch (ai_0) {
	
	case 0:
		return ("Buy");
		
	case 1:
		return ("Sell");
		
	case 2:
		return ("Buy Limit");
		
	case 3:
		return ("Sell Limit");
		
	case 4:
		return ("Buy Stop");
		
	case 5:
		return ("Sell Stop");
	}
	
	return ("Unknown Operation");
}

void ElectroBitch::f0_12(double a_order_open_price_0 = -1.0, double a_order_stoploss_8 = 0.0, double a_order_takeprofit_16 = 0.0, int a_datetime_24 = 0) {
	bool bool_28;
	color color_32;
	double ask_44;
	double bid_52;
	int error_80;
	int digits_76 = MarketInfo(OrderSymbol(), MODE_DIGITS);
	
	if (a_order_open_price_0 <= 0.0)
		a_order_open_price_0 = OrderOpenPrice();
		
	if (a_order_stoploss_8 < 0.0)
		a_order_stoploss_8 = OrderStopLoss();
		
	if (a_order_takeprofit_16 < 0.0)
		a_order_takeprofit_16 = OrderTakeProfit();
		
	a_order_open_price_0 = NormalizeDouble(a_order_open_price_0, digits_76);
	
	a_order_stoploss_8 = NormalizeDouble(a_order_stoploss_8, digits_76);
	
	a_order_takeprofit_16 = NormalizeDouble(a_order_takeprofit_16, digits_76);
	
	double ld_36 = NormalizeDouble(OrderOpenPrice(), digits_76);
	
	double ld_60 = NormalizeDouble(OrderStopLoss(), digits_76);
	
	double ld_68 = NormalizeDouble(OrderTakeProfit(), digits_76);
	
	if (a_order_open_price_0 != ld_36 || a_order_stoploss_8 != ld_60 || a_order_takeprofit_16 != ld_68) {
		for (int li_84 = 1; li_84 <= 5; li_84++) {
			if (!IsTesting() && (!IsExpertEnabled()) || IsStopped())
				break;
				
			while (!IsTradeAllowed())
				Sleep(5000);
				
			RefreshRates();
			
			bool_28 = OrderModify(OrderTicket(), a_order_open_price_0, a_order_stoploss_8, a_order_takeprofit_16, a_datetime_24, color_32);
			
			if (bool_28) {
				PlaySound("alert");
				return;
			}
			
			error_80 = GetLastError();
			
			ask_44 = MarketInfo(OrderSymbol(), MODE_ASK);
			bid_52 = MarketInfo(OrderSymbol(), MODE_BID);
			
			if (Log)
				Print("Error(", error_80, ") modifying order: ", ErrorDescription(error_80), ", try ", li_84);
				
			if (Log)
				Print("Ask=", ask_44, "  Bid=", bid_52, "  sy=", OrderSymbol(), "  op=" + f0_10(OrderType()), "  pp=", a_order_open_price_0, "  sl=", a_order_stoploss_8, "  tp=",
					  a_order_takeprofit_16);
				      
			Sleep(10000);
		}
	}
}

int ElectroBitch::f0_2(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1, int ai_16 = 0) {
	int order_total_24 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int pos_20 = 0; pos_20 < order_total_24; pos_20++) {
		if (OrderSelect(pos_20, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "") {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					if (a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
						if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
							if (ai_16 <= OrderOpenTime())
								return (1);
					}
				}
			}
		}
	}
	
	return (0);
}


bool ElectroBitch::A(double a0, double a1, double a2) {

	if (a0 + a1 > a2)
		return (true);
		
	return (false);
}

bool ElectroBitch::B(int a0, double a1, double a2, int a3, double a4) {


	if (a0 > 0) {
		if (a3 * a4 < a1 - a2)
			return (true);
	}
	
	return(false);
}

bool ElectroBitch::C(int a0, int a1, double a2, double a3, int a4, double a5) {


	if (a0 < 0) {
		if (a1 > 0) {
			if (a4 * a5 < a2 - a3)
				return (true);
				
		}
	}
	
	return(false);
	
}

}

#endif
