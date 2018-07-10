#include "Overlook.h"

#if 0

// dozer

namespace Overlook {

Bulldozer::Bulldozer() {

}

void Bulldozer::InitEA() {
	//SetWindowHandle(WindowHandle(Symbol(), Period()));
	gi_260 = false;
	
	if (!IsTradeAllowed()) {
		Message("For normal operation, adviser to\n" + "Allow live trading");
		gi_260 = true;
		return;
	}
	
	if (!IsLibrariesAllowed()) {
		Message("For normal operation, adviser to\n" + "Allow import of external experts");
		gi_260 = true;
		return;
	}
	
	if (!IsTesting()) {
		if (IsExpertEnabled())
			Message("Adviser will be launched next tick");
		else
			Message("button is depressed \"Allow Advisors\"");
	}
	
	for (int l_index_0 = 0; l_index_0 < 256; l_index_0++)
		gsa_292[l_index_0] = CharToStr(l_index_0);
		
	GetAvtor();
	
	GetLogoType();
	
	return (0);
}

void Bulldozer::StartEA(int pos) {
	string ls_16;
	double ld_64;
	double ld_112;
	double ld_124;
	double ld_136;
	double ld_148;
	double l_lots_0 = 0;
	GetYTG();
	
	if (Lots > 0.0)
		l_lots_0 = Lots;
	else
		l_lots_0 = GetLot();
		
	double ld_8 = (AccountEquity() - AccountBalance()) / (AccountBalance() / 100.0);
	
	if (ld_8 < gd_284)
		gd_284 = ld_8;
		
	if (gi_256) {
		Message("Fatal Error! advisor is STOPPED!");
		return;
	}
	
	if (gi_260) {
		Message("Failed to initialize the adviser!");
		return;
	}
	
	if (ShowComment) {
		ls_16 = "CurTime=" + TimeToStr(TimeCurrent(), TIME_MINUTES) + "  Number of orders=" + DoubleToStr(quantity * 2, 0) + "  Factor. between orders=" + steps + "  TakeProfit=" + TakeProfit + "  StopLoss=" + StopLoss + "  Lots=" + DoubleToStr(Lots, 2) + "  Displacement=" + Displacement
				+ "\n+------------------------------+"
				+ "\n   Balance=" + DoubleToStr(AccountBalance(), 2)
				+ "\n   Equity=" + DoubleToStr(AccountEquity(), 2)
				+ "\n   profit=" + DoubleToStr(AccountEquity() - AccountBalance(), 3) + " $"
				+ "\n   profit=" + DoubleToStr(100.0 * (AccountEquity() / AccountBalance() - 1.0), 3) + " %"
				+ "\n   Maximum Drawdown=" + DoubleToStr(gd_284, 2) + "%"
				+ "\n   Slippage=" + DoubleToStr(Slippage * GetSlippage(), 0)
				+ "\n+------------------------------+";
		Comment(ls_16);
	}
	
	else
		Comment("");
		
	if (Traling)
		SimpleTrailing(Symbol(), -1, MagicNumber);
		
	double ld_24 = MarketInfo(Symbol(), MODE_STOPLEVEL) * Point + betta * GetPoint();
	
	double l_minlot_32 = MarketInfo(Symbol(), MODE_MINLOT);
	
	//double gd_276 = GetFunc(Digits, l_lots_0, TakeProfit_AV);
	
	double precision = 100;
	
	if (Digits == 3 || Digits >= 5)
		precision = 1000;
		
	int gd_276 = 1000.0 * l_lots_0 * TakeProfit_AV / precision;
	
	
	if (ExistPositions(Symbol(), OP_BUY, MN_b) && GetProfitOpenPosInCurrency(Symbol(), OP_BUY, MagicNumber) + GetProfitOpenPosInCurrency(Symbol(), OP_BUY, MN_b) > gd_276) {
		ClosePosFirstProfit(Symbol(), OP_BUY, MagicNumber);
		ClosePosFirstProfit(Symbol(), OP_BUY, MN_b);
		DeleteOrders(Symbol(), OP_BUYSTOP, MN_b);
	}
	
	if (ExistPositions(Symbol(), OP_SELL, MN_s) && GetProfitOpenPosInCurrency(Symbol(), OP_SELL, MagicNumber) + GetProfitOpenPosInCurrency(Symbol(), OP_SELL, MN_s) > gd_276) {
		ClosePosFirstProfit(Symbol(), OP_SELL, MagicNumber);
		ClosePosFirstProfit(Symbol(), OP_SELL, MN_s);
		DeleteOrders(Symbol(), OP_SELLSTOP, MN_s);
	}
	
	double ld_40 = NormalizeDouble(l_lots_0 * koef_lot, LotPoint());
	
	if (ld_40 <= l_lots_0)
		ld_40 = NormalizeDouble(l_lots_0 + l_minlot_32, LotPoint());
		
	double ld_48 = NormalizeDouble(Ask + otstyp * GetPoint(), Digits);
	
	double ld_56 = NormalizeDouble(Bid - otstyp * GetPoint(), Digits);
	
	if (!ExistORPS(Symbol(), -1, MN_b)) {
		if (PriceProsadka(Symbol(), OP_BUY, MagicNumber))
			SetOrder(Symbol(), OP_BUYSTOP, ld_40, ld_48, 0, 0, MN_b);
	}
	
	else {
		if (GetOrderOpenPrice(Symbol(), -1, MN_b) - Ask > (pips_prosadka + otstyp + GetCount(Symbol(), OP_BUY, MN_b)) * GetPoint() * exponents) {
			ld_64 = GetLotLastOrder(Symbol(), -1, MN_b);
			ld_40 = NormalizeDouble(koef_lot * ld_64, LotPoint());
			
			if (ld_40 <= ld_64)
				ld_40 = NormalizeDouble(ld_64 + l_minlot_32, LotPoint());
				
			SetOrder(Symbol(), OP_BUYSTOP, ld_40, ld_48, 0, 0, MN_b);
		}
	}
	
	if (!ExistORPS(Symbol(), -1, MN_s)) {
		if (PriceProsadka(Symbol(), OP_SELL, MagicNumber))
			SetOrder(Symbol(), OP_SELLSTOP, ld_40, ld_56, 0, 0, MN_s);
	}
	
	else {
		if (Bid - GetOrderOpenPrice(Symbol(), -1, MN_s) > (pips_prosadka + otstyp + GetCount(Symbol(), OP_SELL, MN_s)) * GetPoint() * exponents) {
			ld_64 = GetLotLastOrder(Symbol(), -1, MN_s);
			ld_40 = NormalizeDouble(koef_lot * ld_64, LotPoint());
			
			if (ld_40 <= l_lots_0)
				ld_40 = NormalizeDouble(ld_64 + l_minlot_32, LotPoint());
				
			SetOrder(Symbol(), OP_SELLSTOP, ld_40, ld_56, 0, 0, MN_s);
		}
	}
	
	int li_unused_72 = 0;
	
	double ld_76 = GetLevelStart();
	double ld_84 = ld_76 - steps * GetPoint();
	
	if (gi_272) {
		SetHLine(Lime, "r", ld_76);
		SetHLine(Red, "s", ld_84);
	}
	
	double ld_92 = 0;
	
	double ld_100 = 0;
	
	if (!ExistPositions(Symbol(), OP_BUY, MagicNumber)) {
		for (int l_count_108 = 0; l_count_108 < quantity; l_count_108++) {
			//ld_112 = GetA(ld_76, l_count_108, steps, GetPoint());
			ld_112 = ld_76 + l_count_108 * steps * Point;
			
			if (!ExistOrdersByPrice(Symbol(), OP_BUYSTOP, MagicNumber, ld_112) && !ExistPosByPrice(Symbol(), OP_BUY, MagicNumber, ld_112) && ld_112 - Ask > ld_24) {
				if (StopLoss > 0)
					ld_92 = ld_112 - StopLoss * GetPoint();
				else
					ld_92 = 0;
					
				if (TakeProfit > 0)
					ld_100 = ld_112 + TakeProfit * GetPoint();
				else
					ld_100 = 0;
					
				SetOrder(Symbol(), OP_BUYSTOP, l_lots_0, ld_112, ld_92, ld_100, MagicNumber);
			}
		}
		
		if (NevBar()) {
			for (int l_count_120 = 0; l_count_120 <= 50; l_count_120++) {
				ld_124 = ld_76 + (l_count_120 + quantity) * steps * GetPoint();
				
				if (ExistOrdersByPrice(Symbol(), OP_BUYSTOP, MagicNumber, ld_124))
					CloseOrderBySelect();
			}
		}
	}
	
	if (!ExistPositions(Symbol(), OP_SELL, MagicNumber)) {
		for (int l_count_132 = 0; l_count_132 < quantity; l_count_132++) {
			//ld_136 = GetB(ld_84, l_count_132, steps, GetPoint());
			ld_136 = ld_84 - l_count_132 * steps * Point;
			
			if (!ExistOrdersByPrice(Symbol(), OP_SELLSTOP, MagicNumber, ld_136) && !ExistPosByPrice(Symbol(), OP_SELL, MagicNumber, ld_136) && Bid - ld_136 > ld_24) {
				if (StopLoss > 0)
					ld_92 = ld_136 + StopLoss * GetPoint();
				else
					ld_92 = 0;
					
				if (TakeProfit > 0)
					ld_100 = ld_136 - TakeProfit * GetPoint();
				else
					ld_100 = 0;
					
				SetOrder(Symbol(), OP_SELLSTOP, l_lots_0, ld_136, ld_92, ld_100, MagicNumber);
			}
		}
		
		if (NevBar()) {
			for (int l_count_144 = 0; l_count_144 <= 50; l_count_144++) {
				ld_148 = ld_84 - (l_count_144 + quantity) * steps * GetPoint();
				
				if (ExistOrdersByPrice(Symbol(), OP_SELLSTOP, MagicNumber, ld_148))
					CloseOrderBySelect();
			}
		}
	}
	
	return (0);
}

void Bulldozer::SetHLine(color a_color_0, string a_dbl2str_4 = "", double a_bid_12 = 0.0, int a_style_20 = 0, int a_width_24 = 1) {
	if (a_dbl2str_4 == "")
		a_dbl2str_4 = DoubleToStr(Time[0], 0);
		
	if (a_bid_12 <= 0.0)
		a_bid_12 = Bid;
		
	if (ObjectFind(a_dbl2str_4) < 0)
		ObjectCreate(a_dbl2str_4, OBJ_HLINE, 0, 0, 0);
		
	ObjectSet(a_dbl2str_4, OBJPROP_PRICE1, a_bid_12);
	
	ObjectSet(a_dbl2str_4, OBJPROP_COLOR, a_color_0);
	
	ObjectSet(a_dbl2str_4, OBJPROP_STYLE, a_style_20);
	
	ObjectSet(a_dbl2str_4, OBJPROP_WIDTH, a_width_24);
}

bool Bulldozer::ExistOrdersByPrice(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1, double ad_16 = -1.0) {
	int l_digits_24;
	int l_ord_total_32 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_28 = 0; l_pos_28 < l_ord_total_32; l_pos_28++) {
		if (OrderSelect(l_pos_28, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
				if (OrderType() > OP_SELL && OrderType() < 6) {
					l_digits_24 = MarketInfo(OrderSymbol(), MODE_DIGITS);
					ad_16 = NormalizeDouble(ad_16, l_digits_24);
					
					if (MathAbs(ad_16 - OrderOpenPrice()) < GetPoint() / 10.0)
						if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
							return (true);
				}
			}
		}
	}
	
	return (false);
}

bool Bulldozer::ExistPosByPrice(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1, double ad_16 = 0.0) {
	double ld_24;
	double ld_32;
	int l_digits_40;
	int l_ord_total_48 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_44 = 0; l_pos_44 < l_ord_total_48; l_pos_44++) {
		if (OrderSelect(l_pos_44, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
						l_digits_40 = MarketInfo(OrderSymbol(), MODE_DIGITS);
						ld_24 = NormalizeDouble(ad_16, l_digits_40);
						ld_32 = NormalizeDouble(OrderOpenPrice(), l_digits_40);
						
						if (ad_16 <= 0.0 || ld_24 == ld_32)
							return (true);
					}
				}
			}
		}
	}
	
	return (false);
}

void Bulldozer::SetOrder(string a_symbol_0, int a_cmd_8, double a_lots_12, double a_price_20, double a_price_28 = 0.0, double a_price_36 = 0.0, int a_magic_44 = 0, int a_datetime_48 = 0) {
	color l_color_52;
	int l_datetime_56;
	double l_ask_60;
	double l_bid_68;
	double l_point_76;
	int l_error_84;
	int l_ticket_92;
	string l_comment_100 = WindowExpertName() + " " + GetNameTF(Period());
	
	if (a_symbol_0 == "" || a_symbol_0 == "0")
		a_symbol_0 = Symbol();
		
	int l_stoplevel_96 = MarketInfo(a_symbol_0, MODE_STOPLEVEL);
	
	if (a_cmd_8 == OP_BUYLIMIT || a_cmd_8 == OP_BUYSTOP)
		l_color_52 = Lime;
	else
		l_color_52 = Red;
		
	if (a_datetime_48 > 0 && a_datetime_48 < TimeCurrent())
		a_datetime_48 = 0;
		
	for (int li_88 = 1; li_88 <= NumberOfTry; li_88++) {
		if (!IsTesting() && !IsExpertEnabled() || IsStopped()) {
			Print("SetOrder(): Inactivation of the function");
			return;
		}
		
		while (!IsTradeAllowed())
			Sleep(5000);
			
		RefreshRates();
		
		l_datetime_56 = TimeCurrent();
		
		l_ticket_92 = OrderSend(a_symbol_0, a_cmd_8, a_lots_12, a_price_20, Slippage, a_price_28, a_price_36, l_comment_100, a_magic_44, a_datetime_48, l_color_52);
		
		if (l_ticket_92 > 0) {
			PlaySound("ok");
			return;
		}
		
		l_error_84 = GetLastError();
		
		if (l_error_84 == 128/* TRADE_TIMEOUT */ || l_error_84 == 142 || l_error_84 == 143) {
			Sleep(66000);
			
			if (ExistOrders(a_symbol_0, a_cmd_8, a_magic_44, l_datetime_56)) {
				PlaySound("alert2");
				return;
			}
			
			Print("Error(", l_error_84, ") set order: ", ErrorDescription(l_error_84), ", try ", li_88);
		}
		
		else {
			l_point_76 = MarketInfo(a_symbol_0, MODE_POINT);
			l_ask_60 = MarketInfo(a_symbol_0, MODE_ASK);
			l_bid_68 = MarketInfo(a_symbol_0, MODE_BID);
			
			if (l_error_84 == 130/* INVALID_STOPS */) {
				switch (a_cmd_8) {
				
				case OP_BUYLIMIT:
				
					if (a_price_20 > l_ask_60 - l_stoplevel_96 * l_point_76)
						a_price_20 = l_ask_60 - l_stoplevel_96 * l_point_76;
						
					if (a_price_28 > a_price_20 - (l_stoplevel_96 + 1) * l_point_76)
						a_price_28 = a_price_20 - (l_stoplevel_96 + 1) * l_point_76;
						
					if (a_price_36 > 0.0 && a_price_36 < a_price_20 + (l_stoplevel_96 + 1) * l_point_76)
						a_price_36 = a_price_20 + (l_stoplevel_96 + 1) * l_point_76;
						
					break;
					
				case OP_BUYSTOP:
					if (a_price_20 < l_ask_60 + (l_stoplevel_96 + 1) * l_point_76)
						a_price_20 = l_ask_60 + (l_stoplevel_96 + 1) * l_point_76;
						
					if (a_price_28 > a_price_20 - (l_stoplevel_96 + 1) * l_point_76)
						a_price_28 = a_price_20 - (l_stoplevel_96 + 1) * l_point_76;
						
					if (a_price_36 > 0.0 && a_price_36 < a_price_20 + (l_stoplevel_96 + 1) * l_point_76)
						a_price_36 = a_price_20 + (l_stoplevel_96 + 1) * l_point_76;
						
					break;
					
				case OP_SELLLIMIT:
					if (a_price_20 < l_bid_68 + l_stoplevel_96 * l_point_76)
						a_price_20 = l_bid_68 + l_stoplevel_96 * l_point_76;
						
					if (a_price_28 > 0.0 && a_price_28 < a_price_20 + (l_stoplevel_96 + 1) * l_point_76)
						a_price_28 = a_price_20 + (l_stoplevel_96 + 1) * l_point_76;
						
					if (a_price_36 > a_price_20 - (l_stoplevel_96 + 1) * l_point_76)
						a_price_36 = a_price_20 - (l_stoplevel_96 + 1) * l_point_76;
						
					break;
					
				case OP_SELLSTOP:
					if (a_price_20 > l_bid_68 - l_stoplevel_96 * l_point_76)
						a_price_20 = l_bid_68 - l_stoplevel_96 * l_point_76;
						
					if (a_price_28 > 0.0 && a_price_28 < a_price_20 + (l_stoplevel_96 + 1) * l_point_76)
						a_price_28 = a_price_20 + (l_stoplevel_96 + 1) * l_point_76;
						
					if (a_price_36 > a_price_20 - (l_stoplevel_96 + 1) * l_point_76)
						a_price_36 = a_price_20 - (l_stoplevel_96 + 1) * l_point_76;
				}
				
				Print("SetOrder(): Adjusted price levels");
			}
			
			Print("Error(", l_error_84, ") set order: ", ErrorDescription(l_error_84), ", try ", li_88);
			
			Print("Ask=", l_ask_60, "  Bid=", l_bid_68, "  sy=", a_symbol_0, "  ll=", a_lots_12, "  op=", GetNameOP(a_cmd_8), "  pp=", a_price_20, "  sl=", a_price_28, "  tp=", a_price_36, "  mn=", a_magic_44);
			
			if (l_ask_60 == 0.0 && l_bid_68 == 0.0)
				Message("SetOrder(): Check out the review of the market presence of the symbol " + a_symbol_0);
				
			if (l_error_84 == 2/* COMMON_ERROR */ || l_error_84 == 64/* ACCOUNT_DISABLED */ || l_error_84 == 65/* INVALID_ACCOUNT */ || l_error_84 == 133/* TRADE_DISABLED */) {
				gi_256 = true;
				return;
			}
			
			if (l_error_84 == 4/* SERVER_BUSY */ || l_error_84 == 131/* INVALID_TRADE_VOLUME */ || l_error_84 == 132/* MARKET_CLOSED */) {
				Sleep(300000);
				return;
			}
			
			if (l_error_84 == 8/* TOO_FREQUENT_REQUESTS */ || l_error_84 == 141/* TOO_MANY_REQUESTS */)
				Sleep(100000);
				
			if (l_error_84 == 139/* ORDER_LOCKED */ || l_error_84 == 140/* LONG_POSITIONS_ONLY_ALLOWED */ || l_error_84 == 148/* ERR_TRADE_TOO_MANY_ORDERS */)
				break;
				
			if (l_error_84 == 146/* TRADE_CONTEXT_BUSY */)
				while (IsTradeContextBusy())
					Sleep(11000);
					
			if (l_error_84 == 147/* ERR_TRADE_EXPIRATION_DENIED */)
				a_datetime_48 = 0;
			else
				if (l_error_84 != 135/* PRICE_CHANGED */ && l_error_84 != 138/* REQUOTE */)
					Sleep(7700.0);
		}
	}
}

string Bulldozer::GetNameTF(int a_timeframe_0 = 0) {
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

bool Bulldozer::ExistOrders(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1, int ai_16 = 0) {
	int l_cmd_28;
	int l_ord_total_24 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_20 = 0; l_pos_20 < l_ord_total_24; l_pos_20++) {
		if (OrderSelect(l_pos_20, SELECT_BY_POS, MODE_TRADES)) {
			l_cmd_28 = OrderType();
			
			if (l_cmd_28 > OP_SELL && l_cmd_28 < 6) {
				if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || l_cmd_28 == a_cmd_8) {
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
						if (ai_16 <= OrderOpenTime())
							return (true);
				}
			}
		}
	}
	
	return (false);
}

string Bulldozer::GetNameOP(int ai_0) {
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

void Bulldozer::Message(string as_0) {
	Comment(as_0);
	
	if (StringLen(as_0) > 0)
		Print(as_0);
}

void Bulldozer::CloseOrderBySelect() {
	bool l_ord_delete_4;
	int l_error_8;
	
	if (OrderSelect(OrderTicket(), SELECT_BY_TICKET, MODE_TRADES)) {
		for (int li_0 = 1; li_0 <= NumberOfTry; li_0++) {
			if (!IsTesting() && !IsExpertEnabled() || IsStopped())
				break;
				
			while (!IsTradeAllowed())
				Sleep(5000);
				
			l_ord_delete_4 = OrderDelete(OrderTicket(), Snow);
			
			if (l_ord_delete_4) {
				PlaySound("tick");
				return;
			}
			
			l_error_8 = GetLastError();
			
			Print("error (", l_error_8, ") when removing BUYSTOP", ": ", ErrorDescription(l_error_8), ", attempt ", li_0);
			Sleep(5000);
		}
	}
}

void Bulldozer::SimpleTrailing(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	double ld_16;
	double l_price_24;
	int l_ord_total_36 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_32 = 0; l_pos_32 < l_ord_total_36; l_pos_32++) {
		if (OrderSelect(l_pos_32, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
				ld_16 = GetPoint();
				
				if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
					if (OrderType() == OP_BUY) {
						l_price_24 = MarketInfo(OrderSymbol(), MODE_BID);
						
						if (!TSProfitOnly || l_price_24 - OrderOpenPrice() > TStop_Buy * ld_16)
							if (OrderStopLoss() < l_price_24 - (TStop_Buy + TrailingStep - 1) * ld_16)
								ModifyOrder(-1, l_price_24 - TStop_Buy * ld_16, -1);
					}
					
					if (OrderType() == OP_SELL) {
						l_price_24 = MarketInfo(OrderSymbol(), MODE_ASK);
						
						if (!TSProfitOnly || OrderOpenPrice() - l_price_24 > TStop_Sell * ld_16)
							if (OrderStopLoss() > l_price_24 + (TStop_Sell + TrailingStep - 1) * ld_16 || OrderStopLoss() == 0.0)
								ModifyOrder(-1, l_price_24 + TStop_Sell * ld_16, -1);
					}
				}
			}
		}
	}
}

void Bulldozer::ModifyOrder(double a_ord_open_price_0 = -1.0, double a_ord_stoploss_8 = 0.0, double a_ord_takeprofit_16 = 0.0, int a_datetime_24 = 0) {
	bool l_bool_28;
	color l_color_32;
	double l_ask_44;
	double l_bid_52;
	int l_error_80;
	int l_digits_76 = MarketInfo(OrderSymbol(), MODE_DIGITS);
	
	if (a_ord_open_price_0 <= 0.0)
		a_ord_open_price_0 = OrderOpenPrice();
		
	if (a_ord_stoploss_8 < 0.0)
		a_ord_stoploss_8 = OrderStopLoss();
		
	if (a_ord_takeprofit_16 < 0.0)
		a_ord_takeprofit_16 = OrderTakeProfit();
		
	a_ord_open_price_0 = NormalizeDouble(a_ord_open_price_0, l_digits_76);
	
	a_ord_stoploss_8 = NormalizeDouble(a_ord_stoploss_8, l_digits_76);
	
	a_ord_takeprofit_16 = NormalizeDouble(a_ord_takeprofit_16, l_digits_76);
	
	double ld_36 = NormalizeDouble(OrderOpenPrice(), l_digits_76);
	
	double ld_60 = NormalizeDouble(OrderStopLoss(), l_digits_76);
	
	double ld_68 = NormalizeDouble(OrderTakeProfit(), l_digits_76);
	
	if (a_ord_open_price_0 != ld_36 || a_ord_stoploss_8 != ld_60 || a_ord_takeprofit_16 != ld_68) {
		for (int li_84 = 1; li_84 <= NumberOfTry; li_84++) {
			if (!IsTesting() && !IsExpertEnabled() || IsStopped())
				break;
				
			while (!IsTradeAllowed())
				Sleep(5000);
				
			RefreshRates();
			
			l_bool_28 = OrderModify(OrderTicket(), a_ord_open_price_0, a_ord_stoploss_8, a_ord_takeprofit_16, a_datetime_24, l_color_32);
			
			if (l_bool_28) {
				PlaySound("wait");
				return;
			}
			
			l_error_80 = GetLastError();
			
			l_ask_44 = MarketInfo(OrderSymbol(), MODE_ASK);
			l_bid_52 = MarketInfo(OrderSymbol(), MODE_BID);
			Print("Error(", l_error_80, ") modifying order: ", ErrorDescription(l_error_80), ", try ", li_84);
			Print("Ask=", l_ask_44, "  Bid=", l_bid_52, "  sy=", OrderSymbol(), "  op=" + GetNameOP(OrderType()), "  pp=", a_ord_open_price_0, "  sl=", a_ord_stoploss_8, "  tp=", a_ord_takeprofit_16);
			Sleep(10000);
		}
	}
}

double Bulldozer::GetProfitOpenPosInCurrency(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	double ld_ret_16 = 0;
	int l_ord_total_28 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_24 = 0; l_pos_24 < l_ord_total_28; l_pos_24++) {
		if (OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL)
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
						ld_ret_16 += OrderProfit() + OrderCommission() + OrderSwap();
			}
		}
	}
	
	return (ld_ret_16);
}

int Bulldozer::ExistPositions(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1, int ai_16 = 0) {
	int l_ord_total_24 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_20 = 0; l_pos_20 < l_ord_total_24; l_pos_20++) {
		if (OrderSelect(l_pos_20, SELECT_BY_POS, MODE_TRADES)) {
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

bool Bulldozer::NevBar() {
	if (g_time_296 == Time[0])
		return (false);
		
	g_time_296 = Time[0];
	
	return (true);
}

void Bulldozer::DeleteOrders(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	bool l_ord_delete_16;
	int l_error_20;
	int l_cmd_36;
	int l_ord_total_32 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_24 = l_ord_total_32 - 1; l_pos_24 >= 0; l_pos_24--) {
		if (OrderSelect(l_pos_24, SELECT_BY_POS, MODE_TRADES)) {
			l_cmd_36 = OrderType();
			
			if (l_cmd_36 > OP_SELL && l_cmd_36 < 6) {
				if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || l_cmd_36 == a_cmd_8) {
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
						for (int li_28 = 1; li_28 <= NumberOfTry; li_28++) {
							if (!IsTesting() && !IsExpertEnabled() || IsStopped())
								break;
								
							while (!IsTradeAllowed())
								Sleep(5000);
								
							l_ord_delete_16 = OrderDelete(OrderTicket(), White);
							
							if (l_ord_delete_16) {
								PlaySound("timeout");
								break;
							}
							
							l_error_20 = GetLastError();
							
							Print("Error(", l_error_20, ") delete order ", GetNameOP(l_cmd_36), ": ", ErrorDescription(l_error_20), ", try ", li_28);
							Sleep(5000);
						}
					}
				}
			}
		}
	}
}

void Bulldozer::ClosePosFirstProfit(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	int l_ord_total_20 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_16 = l_ord_total_20 - 1; l_pos_16 >= 0; l_pos_16--) {
		if (OrderSelect(l_pos_16, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
						if (OrderProfit() + OrderSwap() > 0.0)
							ClosePosBySelect();
				}
			}
		}
	}
	
	l_ord_total_20 = OrdersTotal();
	
	for (l_pos_16 = l_ord_total_20 - 1; l_pos_16 >= 0; l_pos_16--) {
		if (OrderSelect(l_pos_16, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL)
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
						ClosePosBySelect();
			}
		}
	}
}

void Bulldozer::ClosePosBySelect() {
	bool l_ord_close_0;
	color l_color_4;
	double l_ord_lots_8;
	double l_ask_16;
	double l_bid_24;
	double l_price_32;
	int l_error_40;
	
	if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
		for (int li_44 = 1; li_44 <= NumberOfTry; li_44++) {
			if (!IsTesting() && !IsExpertEnabled() || IsStopped())
				break;
				
			while (!IsTradeAllowed())
				Sleep(5000);
				
			RefreshRates();
			
			l_ask_16 = MarketInfo(OrderSymbol(), MODE_ASK);
			
			l_bid_24 = MarketInfo(OrderSymbol(), MODE_BID);
			
			if (OrderType() == OP_BUY) {
				l_price_32 = l_bid_24;
				l_color_4 = Aqua;
			}
			
			else {
				l_price_32 = l_ask_16;
				l_color_4 = Gold;
			}
			
			l_ord_lots_8 = OrderLots();
			
			l_ord_close_0 = OrderClose(OrderTicket(), l_ord_lots_8, NormalizeDouble(l_price_32, MarketInfo(OrderSymbol(), MODE_DIGITS)), Slippage, l_color_4);
			
			if (l_ord_close_0) {
				PlaySound("news");
				return;
			}
			
			l_error_40 = GetLastError();
			
			if (l_error_40 == 146/* TRADE_CONTEXT_BUSY */)
				while (IsTradeContextBusy())
					Sleep(11000);
					
			Print("Error(", l_error_40, ") Close ", GetNameOP(OrderType()), " ", ErrorDescription(l_error_40), ", try ", li_44);
			
			Print(OrderTicket(), "  Ask=", l_ask_16, "  Bid=", l_bid_24, "  pp=", l_price_32);
			
			Print("sy=", OrderSymbol(), "  ll=", l_ord_lots_8, "  sl=", OrderStopLoss(), "  tp=", OrderTakeProfit(), "  mn=", OrderMagicNumber());
			
			Sleep(5000);
		}
	}
	
	else
		Print("Improper Trading. Close ", GetNameOP(OrderType()));
}

double Bulldozer::GetLot() {
	double l_free_magrin_0 = 0;
	
	if (Choice_method)
		l_free_magrin_0 = AccountBalance();
	else
		l_free_magrin_0 = AccountFreeMargin();
		
	double l_minlot_8 = MarketInfo(Symbol(), MODE_MINLOT);
	
	double l_maxlot_16 = MarketInfo(Symbol(), MODE_MAXLOT);
	
	double ld_24 = Risk / 100.0;
	
	double ld_ret_32 = MathFloor(l_free_magrin_0 * ld_24 / MarketInfo(Symbol(), MODE_MARGINREQUIRED) / MarketInfo(Symbol(), MODE_LOTSTEP)) * MarketInfo(Symbol(), MODE_LOTSTEP);
	
	if (ld_ret_32 < l_minlot_8)
		ld_ret_32 = l_minlot_8;
		
	if (ld_ret_32 > l_maxlot_16)
		ld_ret_32 = l_maxlot_16;
		
	return (ld_ret_32);
}

void Bulldozer::GetAvtor() {
	string lsa_0[256];
	
	for (int l_index_4 = 0; l_index_4 < 256; l_index_4++)
		lsa_0[l_index_4] = CharToStr(l_index_4);
		
	string ls_8 = lsa_0[70] + lsa_0[97] + lsa_0[99] + lsa_0[116] + lsa_0[111] + lsa_0[114] + lsa_0[121] + lsa_0[32] + lsa_0[111] + lsa_0[102] + lsa_0[32] + lsa_0[116] +
				  lsa_0[104] + lsa_0[101] + lsa_0[32] + lsa_0[97] + lsa_0[100] + lsa_0[118] + lsa_0[105] + lsa_0[115] + lsa_0[101] + lsa_0[114] + lsa_0[115] + lsa_0[58] + lsa_0[32] +
				  lsa_0[121] + lsa_0[117] + lsa_0[114] + lsa_0[105] + lsa_0[121] + lsa_0[116] + lsa_0[111] + lsa_0[107] + lsa_0[109] + lsa_0[97] + lsa_0[110] + lsa_0[64] + lsa_0[103] +
				  lsa_0[109] + lsa_0[97] + lsa_0[105] + lsa_0[108] + lsa_0[46] + lsa_0[99] + lsa_0[111] + lsa_0[109];
	              
	Label("label", ls_8, 2, 3, 15, 10);
}

void Bulldozer::GetDellName(string as_0 = "ytg_") {
	string l_name_8;
	
	for (int li_16 = ObjectsTotal() - 1; li_16 >= 0; li_16--) {
		l_name_8 = ObjectName(li_16);
		
		if (StringFind(l_name_8, as_0) != -1)
			ObjectDelete(l_name_8);
	}
}

double Bulldozer::GetPoint() {
	int li_0 = StringFind(Symbol(), "JPY");
	
	if (li_0 == -1)
		return (0.0001);
		
	return (0.01);
}

void Bulldozer::Label(string a_name_0, string a_text_8, int a_corner_16 = 2, int a_x_20 = 3, int a_y_24 = 15, int a_fontsize_28 = 10, string a_fontname_32 = "Arial", color a_color_40 = 3329330) {
	if (ObjectFind(a_name_0) != -1)
		ObjectDelete(a_name_0);
		
	ObjectCreate(a_name_0, OBJ_LABEL, 0, 0, 0, 0, 0);
	
	ObjectSet(a_name_0, OBJPROP_CORNER, a_corner_16);
	
	ObjectSet(a_name_0, OBJPROP_XDISTANCE, a_x_20);
	
	ObjectSet(a_name_0, OBJPROP_YDISTANCE, a_y_24);
	
	ObjectSetText(a_name_0, a_text_8, a_fontsize_28, a_fontname_32, a_color_40);
}

bool Bulldozer::ExistORPS(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1, int ai_16 = 0) {
	int l_cmd_28;
	int l_ord_total_24 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_20 = 0; l_pos_20 < l_ord_total_24; l_pos_20++) {
		if (OrderSelect(l_pos_20, SELECT_BY_POS, MODE_TRADES)) {
			l_cmd_28 = OrderType();
			
			if (l_cmd_28 >= OP_BUY && l_cmd_28 < 6) {
				if (OrderSymbol() == as_0 || as_0 == "" && a_cmd_8 < OP_BUY || l_cmd_28 == a_cmd_8) {
					if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
						if (ai_16 <= OrderOpenTime())
							return (true);
				}
			}
		}
	}
	
	return (false);
}

bool Bulldozer::PriceProsadka(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	int l_ord_total_20 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_16 = 0; l_pos_16 < l_ord_total_20; l_pos_16++) {
		if (OrderSelect(l_pos_16, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "") {
				if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
					if (OrderType() == a_cmd_8)
						if (OrderOpenPrice() - Ask > (pips_prosadka + otstyp) * GetPoint())
							return (true);
							
					if (OrderType() == a_cmd_8)
						if (Bid - OrderOpenPrice() > (pips_prosadka + otstyp) * GetPoint())
							return (true);
				}
			}
		}
	}
	
	return (false);
}

double Bulldozer::GetOrderOpenPrice(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	int l_datetime_16;
	double l_ord_open_price_20 = 0;
	int l_ord_total_32 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_28 = 0; l_pos_28 < l_ord_total_32; l_pos_28++) {
		if (OrderSelect(l_pos_28, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "") {
				if (OrderType() >= OP_BUY && OrderType() < 6) {
					if (a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
						if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
							if (l_datetime_16 < OrderOpenTime()) {
								l_datetime_16 = OrderOpenTime();
								l_ord_open_price_20 = OrderOpenPrice();
							}
						}
					}
				}
			}
		}
	}
	
	return (l_ord_open_price_20);
}

double Bulldozer::GetLotLastOrder(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	int l_datetime_16;
	double l_ord_lots_20 = -1;
	int l_ord_total_32 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_28 = 0; l_pos_28 < l_ord_total_32; l_pos_28++) {
		if (OrderSelect(l_pos_28, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "") {
				if (OrderType() >= OP_BUY && OrderType() < 6) {
					if (a_cmd_8 < OP_BUY || OrderType() == a_cmd_8) {
						if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12) {
							if (l_datetime_16 < OrderOpenTime()) {
								l_datetime_16 = OrderOpenTime();
								l_ord_lots_20 = OrderLots();
							}
						}
					}
				}
			}
		}
	}
	
	return (l_ord_lots_20);
}

int Bulldozer::GetCount(string as_0 = "", int a_cmd_8 = -1, int a_magic_12 = -1) {
	int li_ret_24;
	int l_ord_total_20 = OrdersTotal();
	
	if (as_0 == "0")
		as_0 = Symbol();
		
	for (int l_pos_16 = 0; l_pos_16 < l_ord_total_20; l_pos_16++) {
		if (OrderSelect(l_pos_16, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == as_0 || as_0 == "") {
				if (OrderType() >= OP_BUY && OrderType() < 6) {
					if (a_cmd_8 < OP_BUY || OrderType() == a_cmd_8)
						if (a_magic_12 < 0 || OrderMagicNumber() == a_magic_12)
							li_ret_24++;
				}
			}
		}
	}
	
	return (li_ret_24);
}

double Bulldozer::LotPoint() {
	double l_lotstep_0 = MarketInfo(Symbol(), MODE_LOTSTEP);
	int li_ret_8 = MathCeil(MathAbs(MathLog(l_lotstep_0) / MathLog(10)));
	return (li_ret_8);
}

double Bulldozer::GetSlippage() {
	int l_digits_0 = Digits;
	double ld_ret_4 = 1;
	
	if (l_digits_0 == 3 || l_digits_0 >= 5)
		ld_ret_4 = 10;
		
	return (ld_ret_4);
}

void Bulldozer::GetYTG() {
	g_count_300++;
	
	if (g_count_300 > 2)
		g_count_300 = 0;
		
	int li_0 = 255;
	
	int li_4 = 65280;
	
	int li_8 = 16711680;
	
	if (g_count_300 == 1) {
		li_0 = 3937500;
		li_4 = 3329330;
		li_8 = 16748574;
	}
	
	if (g_count_300 == 2) {
		li_0 = 17919;
		li_4 = 2263842;
		li_8 = 14772545;
	}
	
	Label("ytg_Y", "Y", 3, 40, 20, 25, "Arial Black", li_0);
	
	Label("ytg_T", "T", 3, 25, 5, 25, "Arial Black", li_4);
	Label("ytg_G", "G", 3, 13, 32, 25, "Arial Black", li_8);
}

double Bulldozer::GetLevelStart() {
	//double ld_0 = GetLevel(Bid, GetPoint(), Displacement, steps);
	//return (ld_0);
	for (double ld_0 = Displacement * GetPoint(); ld_0 < Bid; ld_0 += steps * GetPoint()) { }
	
	return (ld_0);
}

#endif
