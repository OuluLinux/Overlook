#include "Overlook.h"

namespace Overlook {

ModestTry::ModestTry() {
	
}

void ModestTry::InitEA() {
	gi_124 = TrailingStop + TrailingKeep;
	
	if (TrailingStep < 1)
		TrailingStep = 1;
		
	TrailingStep--;
	
	g_datetime_132 = Time(1970,1,1);
	g_datetime_136 = Time(1970,1,1);
	g_datetime_140 = Time(1970,1,1);
	g_datetime_144 = Time(1970,1,1);
	g_datetime_148 = Time(1970,1,1);
	g_datetime_152 = Time(1970,1,1);
	g_datetime_156 = Time(1970,1,1);
	g_datetime_160 = Time(1970,1,1);
	g_datetime_164 = Time(1970,1,1);
}

void ModestTry::StartEA(int pos) {
	if (pos < 1)
		return;
	
	double ld_76;
	double ld_84;
	double ld_92;
	double ld_100;
	double l_ihigh_0 = GetInputBuffer(0, 2).Get(pos-1);
	double l_ilow_8 = GetInputBuffer(0, 1).Get(pos-1);
	Time l_datetime_16 = Now;
	int l_count_20 = 0;
	int l_count_24 = 0;
	int l_count_28 = 0;
	int l_count_32 = 0;
	int l_count_36 = 0;
	int l_count_40 = 0;
	int li_unused_44 = 0;
	int li_unused_48 = 0;
	int l_count_52 = 0;
	int l_count_56 = 0;
	
	for (int l_pos_60 = 0; l_pos_60 < OrdersTotal(); l_pos_60++) {
		if (OrderSelect(l_pos_60, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderMagicNumber() == Magic) {
				if (OrderSymbol() == Symbol()) {
					if (OrderOpenTime() < l_datetime_16) {
						switch (OrderType()) {
						
						case OP_BUYSTOP:
							fDeletePendig(OrderTicket());
							break;
							
						case OP_SELLSTOP:
							fDeletePendig(OrderTicket());
							break;
							
						case OP_BUY:
							l_count_52++;
							break;
							
						case OP_SELL:
							l_count_56++;
						}
					}
				}
			}
		}
	}
	
	if (l_count_52 + l_count_56 < MaxOrdersCount || MaxOrdersCount == -1) {
		RefreshRates();
		
		for (int l_pos_60 = 0; l_pos_60 < OrdersTotal(); l_pos_60++) {
			if (OrderSelect(l_pos_60, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderMagicNumber() == Magic) {
					if (OrderSymbol() == Symbol()) {
						switch (OrderType()) {
						
						case OP_BUY:
							l_count_20++;
							break;
							
						case OP_SELL:
							l_count_24++;
							break;
							
						case OP_BUYSTOP:
							l_count_28++;
							break;
							
						case OP_SELLSTOP:
							l_count_32++;
						}
						
						if (OrderOpenTime() >= l_datetime_16) {
							switch (OrderType()) {
							
							case OP_BUY:
								l_count_36++;
								break;
								
							case OP_SELL:
								l_count_40++;
								break;
								
							case OP_BUYSTOP:
								l_count_36++;
								break;
								
							case OP_SELLSTOP:
								l_count_40++;
							}
						}
					}
				}
			}
			
			else
				return;
		}
		
		if (l_count_36 == 0) {
			for (int l_pos_60 = OrdersHistoryTotal() - 1; l_pos_60 >= 0; l_pos_60--) {
				if (OrderSelect(l_pos_60, SELECT_BY_POS, MODE_HISTORY)) {
					if (OrderMagicNumber() != Magic)
						continue;
						
					if (OrderSymbol() != Symbol())
						continue;
						
					if (OrderOpenTime() < l_datetime_16)
						continue;
						
					if (OrderType() == OP_BUYSTOP) {
						l_count_36++;
						break;
					}
					
					if (OrderType() != OP_BUY)
						continue;
						
					l_count_36++;
					
					break;
				}
				
				return;
			}
		}
		
		if (l_count_40 == 0) {
			for (int l_pos_60 = OrdersHistoryTotal() - 1; l_pos_60 >= 0; l_pos_60--) {
				if (OrderSelect(l_pos_60, SELECT_BY_POS, MODE_HISTORY)) {
					if (OrderMagicNumber() != Magic)
						continue;
						
					if (OrderSymbol() != Symbol())
						continue;
						
					if (OrderOpenTime() < l_datetime_16)
						continue;
						
					if (OrderType() == OP_SELLSTOP) {
						l_count_40++;
						break;
					}
					
					if (OrderType() != OP_SELL)
						continue;
						
					l_count_40++;
					
					break;
				}
				
				return;
			}
		}
		
		if (TimeCurrent() - l_datetime_16 <= 60 * TryMinutes) {
			if (l_count_36 == 0) {
				ld_76 = ND(l_ihigh_0 + (Ask - Bid) + Point * AddToLevel);
				ld_84 = ND(Ask + Point * (MarketInfo(Symbol(), MODE_STOPLEVEL) + gi_128));
				ld_76 = MathMax(ld_76, ld_84);
				fOrderSetBuyStop(ld_76);
			}
			
			if (l_count_40 == 0) {
				ld_92 = ND(l_ihigh_0 + (Ask - Bid) + Point * AddToLevel);
				ld_100 = ND(Bid - Point * (MarketInfo(Symbol(), MODE_STOPLEVEL) + gi_128));
				ld_92 = MathMin(ld_92, ld_100);
				fOrderSetSellStop(ld_92);
			}
		}
	}
	
	if (TrailingStop > 0)
		fTrailingWithStart();
		
	return;
}

int ModestTry::fDeletePendig(int a_ticket_0) {
	if (!OrderDelete(a_ticket_0)) {
		return (-1);
	}

	
	return (0);
}

double ModestTry::ND(double ad_0) {
	return (NormalizeDouble(ad_0, Digits));
}

int ModestTry::fOrderSetBuyStop(double ad_0, int a_datetime_8) {
	double l_lots_28;
	double l_price_36;
	double l_price_44;
	int l_ticket_52;
	RefreshRates();
	double l_price_12 = ND(ad_0);
	double ld_20 = ND(Ask + Point * MarketInfo(Symbol(), MODE_STOPLEVEL));
	
	if (l_price_12 >= ld_20) {
		l_lots_28 = fGetLotsSimple(OP_BUY);
		
		if (l_lots_28 > 0.0) {
			if (!IsTradeContextBusy()) {
				l_price_36 = ND(l_price_12 - Point * StopLoss);
				
				if (StopLoss == 0)
					l_price_36 = 0;
					
				l_price_44 = ND(l_price_12 + Point * TakeProfit);
				
				if (TakeProfit == 0)
					l_price_44 = 0;
					
				l_ticket_52 = OrderSend(Symbol(), OP_BUYSTOP, l_lots_28, l_price_12, 0, l_price_36, l_price_44, 0, Magic, a_datetime_8);
				
				if (l_ticket_52 > 0)
					return (l_ticket_52);
					
				Print("Error set BUYSTOP. ");
				
				return (-1);
			}
			
			if (TimeCurrent() > g_datetime_136 + 20) {
				g_datetime_136 = TimeCurrent();
				Print("Need set BUYSTOP. Trade Context Busy");
			}
			
			return (-2);
		}
		
		if (TimeCurrent() > g_datetime_140 + 20) {
			g_datetime_140 = TimeCurrent();
			
			if (l_lots_28 == -1.0)
				Print("Need set BUYSTOP. No money");
				
			if (l_lots_28 == -2.0)
				Print("Need set BUYSTOP. Wrong lots size");
		}
		
		return (-3);
	}
	
	if (TimeCurrent() > g_datetime_144 + 20) {
		g_datetime_144 = TimeCurrent();
		Print("Need set BUYSTOP. Wrong price level ");
	}
	
	return (-4);
}

int ModestTry::fOrderSetSellStop(double ad_0, int a_datetime_8) {
	double l_lots_28;
	double l_price_36;
	double l_price_44;
	int l_ticket_52;
	RefreshRates();
	double l_price_12 = ND(ad_0);
	double ld_20 = ND(Bid - Point * MarketInfo(Symbol(), MODE_STOPLEVEL));
	
	if (l_price_12 <= ld_20) {
		l_lots_28 = fGetLotsSimple(OP_SELL);
		
		if (l_lots_28 > 0.0) {
			if (!IsTradeContextBusy()) {
				l_price_36 = ND(l_price_12 + Point * StopLoss);
				
				if (StopLoss == 0)
					l_price_36 = 0;
					
				l_price_44 = ND(l_price_12 - Point * TakeProfit);
				
				if (TakeProfit == 0)
					l_price_44 = 0;
					
				l_ticket_52 = OrderSend(Symbol(), OP_SELLSTOP, l_lots_28, l_price_12, 0, l_price_36, l_price_44, 0, Magic, a_datetime_8);
				
				if (l_ticket_52 > 0)
					return (l_ticket_52);
					
				Print("Error set SELLSTOP. ");
				
				return (-1);
			}
			
			if (TimeCurrent() > g_datetime_148 + 20) {
				g_datetime_148 = TimeCurrent();
				Print("Need set SELLSTOP. Trade Context Busy");
			}
			
			return (-2);
		}
		
		if (TimeCurrent() > g_datetime_152 + 20) {
			g_datetime_152 = TimeCurrent();
			
			if (l_lots_28 == -1.0)
				Print("Need set SELLSTOP. No money");
				
			if (l_lots_28 == -2.0)
				Print("Need set SELLSTOP. Wrong lots size");
		}
		
		return (-3);
	}
	
	if (TimeCurrent() > g_datetime_156 + 20) {
		g_datetime_156 = TimeCurrent();
		Print("Need set SELLSTOP. Wrong price level");
	}
	
	return (-4);
}

double ModestTry::fGetLotsSimple(int a_cmd_0) {
	if (AccountFreeMarginCheck(Symbol(), a_cmd_0, Lots) <= 0.0)
		return (-1);
			
	return (Lots);
}

String ModestTry::fMyErDesc(int ai_0) {
	String ls_4 = "Err Num: " + IntStr(ai_0) + " - ";
	
	switch (ai_0) {
	
	case 0:
		return (ls_4 + "NO ERROR");
		
	case 1:
		return (ls_4 + "NO RESULT");
		
	case 2:
		return (ls_4 + "COMMON ERROR");
		
	case 3:
		return (ls_4 + "INVALID TRADE PARAMETERS");
		
	case 4:
		return (ls_4 + "SERVER BUSY");
		
	case 5:
		return (ls_4 + "OLD VERSION");
		
	case 6:
		return (ls_4 + "NO CONNECTION");
		
	case 7:
		return (ls_4 + "NOT ENOUGH RIGHTS");
		
	case 8:
		return (ls_4 + "TOO FREQUENT REQUESTS");
		
	case 9:
		return (ls_4 + "MALFUNCTIONAL TRADE");
		
	case 64:
		return (ls_4 + "ACCOUNT DISABLED");
		
	case 65:
		return (ls_4 + "INVALID ACCOUNT");
		
	case 128:
		return (ls_4 + "TRADE TIMEOUT");
		
	case 129:
		return (ls_4 + "INVALID PRICE");
		
	case 130:
		return (ls_4 + "INVALID STOPS");
		
	case 131:
		return (ls_4 + "INVALID TRADE VOLUME");
		
	case 132:
		return (ls_4 + "MARKET CLOSED");
		
	case 133:
		return (ls_4 + "TRADE DISABLED");
		
	case 134:
		return (ls_4 + "NOT ENOUGH MONEY");
		
	case 135:
		return (ls_4 + "PRICE CHANGED");
		
	case 136:
		return (ls_4 + "OFF QUOTES");
		
	case 137:
		return (ls_4 + "BROKER BUSY");
		
	case 138:
		return (ls_4 + "REQUOTE");
		
	case 139:
		return (ls_4 + "ORDER LOCKED");
		
	case 140:
		return (ls_4 + "LONG POSITIONS ONLY ALLOWED");
		
	case 141:
		return (ls_4 + "TOO MANY REQUESTS");
		
	case 145:
		return (ls_4 + "TRADE MODIFY DENIED");
		
	case 146:
		return (ls_4 + "TRADE CONTEXT BUSY");
		
	case 147:
		return (ls_4 + "TRADE EXPIRATION DENIED");
		
	case 148:
		return (ls_4 + "TRADE TOO MANY ORDERS");
		
	case 4000:
		return (ls_4 + "NO MQLERROR");
		
	case 4001:
		return (ls_4 + "WRONG FUNCTION POINTER");
		
	case 4002:
		return (ls_4 + "ARRAY INDEX OUT OF RANGE");
		
	case 4003:
		return (ls_4 + "NO MEMORY FOR FUNCTION CALL STACK");
		
	case 4004:
		return (ls_4 + "RECURSIVE STACK OVERFLOW");
		
	case 4005:
		return (ls_4 + "NOT ENOUGH STACK FOR PARAMETER");
		
	case 4006:
		return (ls_4 + "NO MEMORY FOR PARAMETER STRING");
		
	case 4007:
		return (ls_4 + "NO MEMORY FOR TEMP STRING");
		
	case 4008:
		return (ls_4 + "NOT INITIALIZED STRING");
		
	case 4009:
		return (ls_4 + "NOT INITIALIZED ARRAYSTRING");
		
	case 4010:
		return (ls_4 + "NO MEMORY FOR ARRAYSTRING");
		
	case 4011:
		return (ls_4 + "TOO LONG STRING");
		
	case 4012:
		return (ls_4 + "REMAINDER FROM ZERO DIVIDE");
		
	case 4013:
		return (ls_4 + "ZERO DIVIDE");
		
	case 4014:
		return (ls_4 + "UNKNOWN COMMAND");
		
	case 4015:
		return (ls_4 + "WRONG JUMP");
		
	case 4016:
		return (ls_4 + "NOT INITIALIZED ARRAY");
		
	case 4017:
		return (ls_4 + "DLL CALLS NOT ALLOWED");
		
	case 4018:
		return (ls_4 + "CANNOT LOAD LIBRARY");
		
	case 4019:
		return (ls_4 + "CANNOT CALL FUNCTION");
		
	case 4020:
		return (ls_4 + "EXTERNAL EXPERT CALLS NOT ALLOWED");
		
	case 4021:
		return (ls_4 + "NOT ENOUGH MEMORY FOR RETURNED STRING");
		
	case 4022:
		return (ls_4 + "SYSTEM BUSY");
		
	case 4050:
		return (ls_4 + "INVALID FUNCTION PARAMETERS COUNT");
		
	case 4051:
		return (ls_4 + "INVALID FUNCTION PARAMETER VALUE");
		
	case 4052:
		return (ls_4 + "STRING FUNCTION INTERNAL ERROR");
		
	case 4053:
		return (ls_4 + "SOME ARRAY ERROR");
		
	case 4054:
		return (ls_4 + "INCORRECT SERIES ARRAY USING");
		
	case 4055:
		return (ls_4 + "CUSTOM INDICATOR ERROR");
		
	case 4056:
		return (ls_4 + "INCOMPATIBLE ARRAYS");
		
	case 4057:
		return (ls_4 + "GLOBAL VARIABLES PROCESSING ERROR");
		
	case 4058:
		return (ls_4 + "GLOBAL VARIABLE NOT FOUND");
		
	case 4059:
		return (ls_4 + "FUNCTION NOT ALLOWED IN TESTING MODE");
		
	case 4060:
		return (ls_4 + "FUNCTION NOT CONFIRMED");
		
	case 4061:
		return (ls_4 + "SEND MAIL ERROR");
		
	case 4062:
		return (ls_4 + "STRING PARAMETER EXPECTED");
		
	case 4063:
		return (ls_4 + "INTEGER PARAMETER EXPECTED");
		
	case 4064:
		return (ls_4 + "DOUBLE PARAMETER EXPECTED");
		
	case 4065:
		return (ls_4 + "ARRAY AS PARAMETER EXPECTED");
		
	case 4066:
		return (ls_4 + "HISTORY WILL UPDATED");
		
	case 4067:
		return (ls_4 + "TRADE ERROR");
		
	case 4099:
		return (ls_4 + "END OF FILE");
		
	case 4100:
		return (ls_4 + "SOME FILE ERROR");
		
	case 4101:
		return (ls_4 + "WRONG FILE NAME");
		
	case 4102:
		return (ls_4 + "TOO MANY OPENED FILES");
		
	case 4103:
		return (ls_4 + "CANNOT OPEN FILE");
		
	case 4104:
		return (ls_4 + "INCOMPATIBLE ACCESS TO FILE");
		
	case 4105:
		return (ls_4 + "NO ORDER SELECTED");
		
	case 4106:
		return (ls_4 + "UNKNOWN SYMBOL");
		
	case 4107:
		return (ls_4 + "INVALID PRICE PARAM");
		
	case 4108:
		return (ls_4 + "INVALID TICKET");
		
	case 4109:
		return (ls_4 + "TRADE NOT ALLOWED");
		
	case 4110:
		return (ls_4 + "LONGS  NOT ALLOWED");
		
	case 4111:
		return (ls_4 + "SHORTS NOT ALLOWED");
		
	case 4200:
		return (ls_4 + "OBJECT ALREADY EXISTS");
		
	case 4201:
		return (ls_4 + "UNKNOWN OBJECT PROPERTY");
		
	case 4202:
		return (ls_4 + "OBJECT DOES NOT EXIST");
		
	case 4203:
		return (ls_4 + "UNKNOWN OBJECT TYPE");
		
	case 4204:
		return (ls_4 + "NO OBJECT NAME");
		
	case 4205:
		return (ls_4 + "OBJECT COORDINATES ERROR");
		
	case 4206:
		return (ls_4 + "NO SPECIFIED SUBWINDOW");
		
	case 4207:
		return (ls_4 + "SOME OBJECT ERROR");
	}
	
	return (ls_4 + "WRONG ERR NUM");
}

int ModestTry::fTrailingWithStart() {
	double l_price_4;
	int l_error_16;
	int li_ret_0 = 0;
	
	for (int l_pos_12 = 0; l_pos_12 < OrdersTotal(); l_pos_12++) {
		if (OrderSelect(l_pos_12, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == Magic) {
				if (OrderType() == OP_BUY) {
					RefreshRates();
					
					if (ND(Bid - OrderOpenPrice()) >= ND(Point * gi_124)) {
						l_price_4 = ND(Bid - Point * TrailingStop);
						
						if (ND(OrderStopLoss() + Point * TrailingStep) < l_price_4) {
							if (!IsTradeContextBusy()) {
								if (!OrderModify(OrderTicket(), OrderOpenPrice(), l_price_4, OrderTakeProfit())) {
									li_ret_0 = -1;
								}
							}
							
							else {
								li_ret_0 = -2;
								
								if (g_datetime_160 + 15 < TimeCurrent()) {
									g_datetime_160 = TimeCurrent();
								}
							}
						}
					}
				}
				
				if (OrderType() == OP_SELL) {
					RefreshRates();
					
					if (ND(OrderOpenPrice() - Ask) >= ND(Point * gi_124)) {
						l_price_4 = ND(Ask + Point * TrailingStop);
						
						if (!IsTradeContextBusy()) {
							if (ND(OrderStopLoss() - Point * TrailingStep) > l_price_4 || ND(OrderStopLoss()) == 0.0) {
								if (!OrderModify(OrderTicket(), OrderOpenPrice(), l_price_4, OrderTakeProfit())) {
									li_ret_0 = -1;
								}
							}
						}
						
						else {
							li_ret_0 = -2;
							
							if (g_datetime_164 + 15 < TimeCurrent()) {
								g_datetime_164 = TimeCurrent();
							}
						}
					}
				}
			}
		}
	}
	
	return (li_ret_0);
}

}
