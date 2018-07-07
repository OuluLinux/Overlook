#include "Overlook.h"


namespace Overlook {

Novice::Novice() {
	
}

void Novice::InitEA() {
	
	AddSubCore<OsMA>()
		.Set("fast_ema_period", FastEMA)
		.Set("slow_ema_period", SlowEMA)
		.Set("signal_sma", SignalSMA);
			
	AddSubCore<MovingAverageConvergenceDivergence>()
		.Set("fast_ema", FastEMA)
		.Set("slow_ema", SlowEMA)
		.Set("signal_sma_period", 1)
		.Set("method", 2);
			
	AddSubCore<MovingAverageConvergenceDivergence>()
		.Set("fast_ema", FastEMA)
		.Set("slow_ema", SlowEMA)
		.Set("signal_sma_period", 1)
		.Set("method", 0);
	
	
}


void Novice::StartEA(int pos) {
	int v0 = 0;
	
	if (pos < 1)
		return;
	
	//268
	
	//2C4
	if (v250 != 0) {
		v2C0 = (AccountFreeMargin() / 1000) * (v258 / 100);
	}
	
	else {
		v2C0 = Lots;
	}
	
	//348
	
	if (v2C0 < v2A0) {
		v2C0 = v2A0;
	}
	
	else {
		if (v2C0 > v298) {
			v2C0 = v298;
		}
		
		else {
			v2C0 = NormalizeDouble(v2C0 / v2A8, 0) * v2A8;
		}
	}
	
	//3D0
	v26C = MarketInfo(Symbol(), MODE_STOPLEVEL);
	
	v2F0 = 0;
	
	v304 = 0;
	
	v25C = 0;
	
	//448
	for (v260 = 0; v260 < OrdersTotal(); v260++) {
		OrderSelect(v260, 0);
		
		
		if ((OrderSymbol() == Symbol()) || (OrderMagicNumber() > v270) || (OrderMagicNumber() < (v270 + 9))) {
			//518
			if (OrderType() > 1) {
				v304 = 1;
			}
			
			else {
				v0 = OrderTicket();
				v25C++;
				
				InitParam(OrderMagicNumber());
				
				if (OrderType() == OP_BUY) {
					//5A0
					v2F0 = v2F0 + ((OrderLots() * (OrderClosePrice() - OrderOpenPrice())) / Point);
					
					if ((OrderTakeProfit() == 0) && (OrderStopLoss() == 0) && (v278 != 0 || v27C != 0)) {
						if (v278 == 0) {
							v290 = 0;
						}
						
						else {
							if (OrderTakeProfit() == 0) {
								v290 = NormalizeDouble(OrderOpenPrice() + v278 * Point, Digits);
							}
							
							else {
								//758
								v290 = NormalizeDouble(OrderTakeProfit(), Digits);
							}
						}
						
						//798
						
						if (v27C == 0) {
							v288 = 0;
						}
						
						else {
							v288 = NormalizeDouble(OrderOpenPrice() - v27C * Point, Digits);
						}
						
						//828
						OrderModify(OrderTicket(), OrderOpenPrice(), v288, v290);
					}
				}
				
				//8A8
				
				if (OrderType() == OP_SELL) {
					//8C0
					v2F0 = v2F0 + ((OrderOpenPrice() - OrderClosePrice()) * OrderLots() / Point);
					
					if ((OrderTakeProfit() == 0) && (OrderStopLoss() == 0) && ((v278 != 0) || (v27C != 0))) {
						v290 = 0;
					}
					
					else {
						if (OrderTakeProfit() == 0) {
							v290 = NormalizeDouble(OrderOpenPrice() - v278 * Point, Digits);
						}
						
						{
							//A78
							v290 = NormalizeDouble(OrderTakeProfit(), Digits);
						}
					}
					
					//AB8
					
					if (v27C == 0) {
						v288 = 0;
					}
					
					else {
						v288 = NormalizeDouble(OrderOpenPrice() + v27C * Point, Digits);
					}
					
					//B48
					OrderModify(OrderTicket(), OrderOpenPrice(), v288, v290);
				}
			}
		}
	}
	
	//0BE0
	String v30C = ""; //
	
	v2F0 = v2F0 * v2F8;
	
	
	//D1C
	if (v268 > v25C) {
		v308 = 0;
		
		if (v60) {
			for (v260 = OrdersTotal() - 1; v260 >= 0; v260--) {
				//D74
				OrderSelect(v260, 0);
				
				if ((OrderSymbol() == Symbol()) || (OrderMagicNumber() > v270) || (OrderMagicNumber() < (v270 + 9))) {
					if (OrderType() == OP_BUY) {
						OrderClose(OrderTicket(), OrderLots(), Bid, v254);
					}
					
					else {
						//EB8
						if (OrderType() == OP_SELL) {
							OrderClose(OrderTicket(), OrderLots(), Ask, v254);
						}
						
						else {
							OrderDelete(OrderTicket());
						}
					}
					
					//F6C
					v308 = 1;
				}
			}
		}
		
		else {
			//F98
			v260 = 0;
			
			for (v260 = 0; v260 < OrdersTotal(); v260++) {
				OrderSelect(v260, 0);
				
				if ((OrderSymbol() == Symbol()) || (OrderMagicNumber() > v270) || OrderMagicNumber() < (v270 + 9)) {
					if (OrderType() == OP_BUY) {
						OrderClose(OrderTicket(), OrderLots(), Bid, v254);
					}
					
					else {
						//1108
						if (OrderType() == OP_SELL) {
							OrderClose(OrderTicket(), OrderLots(), Ask, v254);
						}
						
						else {
							//1194
							OrderDelete(OrderTicket());
						}
					}
					
					//11BC
					v308 = 1;
				}
			}
		}
		
		//11E8
		
		if (v308 == 1)
			return;
	}
	
	//1208
	v268 = v25C;
	
	if ((v25C >= (MaxTrades - Pips)) && v24C == 1) {
		if (v2F0 >= ProfitIndex) {
			if (!OrderSelect(v0, 1))
				return;
			
			if (OrderType() == OP_BUY) {
				OrderClose(OrderTicket(), OrderLots(), Bid, v254);
			}
			
			else {
				if (OrderType() == OP_SELL) {
					OrderClose(OrderTicket(), OrderLots(), Ask, v254);
				}
				
				v300 = 0;
			}
			
			//13C0
			return;
		}
	}
	
	//13D4
	
	if (v25C >= MaxTrades) {
		v300 = 0;
	}
	
	else {
		v300 = 1;
	}
	
	//1404
	
	if (v2D0 == 0) {
		v2D0 = 0;
		
		for (v260 = 0; v260 < OrdersTotal(); v260++) {
			OrderSelect(v260, 0);
			
			if ((OrderSymbol() == Symbol()) || (OrderMagicNumber() > v270) || (OrderMagicNumber() < (v270 + 9))) {
				if (OrderType() == OP_BUY || OrderType() == OP_BUYLIMIT) {
					v264 = 2;
					
					if (v2D0 > OrderOpenPrice() || v2D0 == 0) {
						v2D8 = OrderLots();
						v2D0 = OrderOpenPrice();
					}
				}
				
				else {
					if (OrderType() == OP_SELL || OrderType() == OP_SELLLIMIT) {
						v264 = 1;
						
						if (v2D0 < OrderOpenPrice()) {
							v2D8 = OrderLots();
							v2D0 = OrderOpenPrice();
						}
					}
				}
			}
		}
	}
	
	
	//1698
	
	if (v25C == 0) {
		v264 = 3;
		double v4 = At(0).GetBuffer(0).Get(pos);
		double vC = At(0).GetBuffer(0).Get(pos-1);
		double v14 = At(1).GetBuffer(0).Get(pos);
		double v1C = At(1).GetBuffer(0).Get(pos-1);
		double v24 = At(2).GetBuffer(0).Get(pos);
		double v2C = At(2).GetBuffer(0).Get(pos-1);
		
		//196C
		
		if ((v4 > vC) && (v14 > v1C) && (v24 > v2C)) {
			v264 = 2;
		}
		
		//19D4
		
		if ((v4 < vC) && (v14 < v1C) && (v24 < v2C)) {
			v264 = 1;
		}
		
		if (Revers == 1) {
			if (v264 == 1)
				v264 = 2;
				
			if (v264 == 2)
				v264 = 1;
		}
	}
	
	//1A8C
	
	if (v60) {
		for (v260 = OrdersTotal() - 1; v260 >= 0; v260--) {
			OrderSelect(v260, 0);
			
			if (OrderSymbol() == Symbol() || OrderMagicNumber() > v270 || OrderMagicNumber() < (v270 + 9) || OrderType() > 1) {
				InitParam(OrderMagicNumber());
				
				if (v280 != 0) {
					//1BE0
					if (OrderType() == OP_SELL) {
						if ((OrderOpenPrice() - Ask) >= (v280 + v284*Point) && (NormalizeDouble(OrderStopLoss(), Digits) > NormalizeDouble(Ask + Point*v280, Digits) || OrderStopLoss() == 0)) {
							OrderModify(OrderTicket(), OrderStopLoss(), NormalizeDouble(Ask + v280*Point, Digits), OrderTakeProfit());
						}
					}
					
					else {
						//1E20
						if (OrderType() == OP_BUY) {
							if ((Bid - OrderOpenPrice()) >= ((v280 + v284)*Point) && NormalizeDouble(OrderStopLoss(), Digits) < NormalizeDouble(Bid - Point*v280, Digits)) {
								OrderModify(OrderTicket(), OrderTakeProfit(), NormalizeDouble(Bid - Point*v280, Digits), OrderTakeProfit());
							}
						}
					}
				}
			}
			
			//2028
		}
	}
	
	else {
		//2048
		if (!v60) {
			for (v260 = 0; v260 < OrdersTotal(); v260++) {
				OrderSelect(v260, 0);
				
				if (OrderSymbol() == Symbol() || OrderMagicNumber() > v270 || OrderMagicNumber() < (v270 + 9) || OrderType() > 1) {
					InitParam(OrderMagicNumber());
					
					if (v280 != 0) {
						if (OrderType() == OP_SELL) {
							if ((OrderOpenPrice() - Ask) >= (v280 + v284*Point) && NormalizeDouble(OrderStopLoss(), Digits) > NormalizeDouble(Ask + Point*v280, Digits) || OrderStopLoss() == 0) {
								OrderModify(OrderTicket(), OrderStopLoss(), NormalizeDouble(Ask + v280*Point, Digits), OrderTakeProfit());
							}
						}
						
						else {
							//23E4
							if (OrderType() == OP_BUY) {
								if ((Bid - OrderOpenPrice()) >= (v280 + v284*Point) && NormalizeDouble(OrderStopLoss(), Digits) < NormalizeDouble(Bid - Point*v280, Digits)) {
									OrderModify(OrderTicket(), OrderTakeProfit(), NormalizeDouble(Bid - Point*v280, Digits), OrderTakeProfit());
								}
							}
						}
					}
				}
				
				//25EC
			}
		}
	}
	
	//260C
	
	if ((v264 == 1) && v300) {
		if (v25C == 0) {
			v2D0 = 0;
			v2C8 = v2C0;
			
			if (v2C8 < v2A0) {
				v2C8 = v2A0;
			}
			
			else {
				if (v2C8 > v298) {
					v2C8 = v298;
				}
				
				else {
					v2C8 = NormalizeDouble(v2C8 / v2A8, 0) * v2A8;
				}
			}
			
			//26E8
			OrderSend(Symbol(), OP_SELL, v2C8, Bid, v254, 0, 0, "Martin-Profit-Expert" + MAGIC, v270);
			
			return;
		}
		
		else {
			//27C4
			if (!v304) {
				InitParam(v270 + v25C);
				
				if (v2E0 == 0) {
					v2C8 = v2E8;
				}
				
				else {
					v2C8 = v2D8 * v2E0;
				}
				
				//2848
				
				if (v2C8 < v2A0) {
					v2C8 = v2A0;
				}
				
				else {
					if (v2C8 > v298) {
						v2C8 = v298;
					}
					
					else {
						v2C8 = NormalizeDouble(v2C8 / v2A8, 0) * v2A8;
					}
				}
				
				//28D0
				v2B8 = v2D0 + v284 * Point;
				
				if (((Bid + v26C*Point) <= v2B8) && !v54) {
					OrderSend(Symbol(), OP_SELLLIMIT, v2C8, v2B8, 0, 0, 0, "Martin-Profit-Expert" + MAGIC, v274);
				}
				
				else {
					//2A30
					if ((Bid - v2D0) >= (v284*Point)) {
						OrderSend(Symbol(), OP_SELL, v2C8, Bid, 0, 0, 0, "Martin-Profit-Expert" + MAGIC, v274);
					}
				}
				
				//2B34
				v2D0 = 0;
				
				return;
			}
		}
	}
	
	//2B54
	
	if ((v264 == 2) && v300) {
		if (v25C == 0) {
			v2D0 = 0;
			v2C8 = v2C0;
			
			if (v2C8 < v2A0) {
				v2C8 = v2A0;
			}
			
			else {
				if (v2C8 > v298) {
					v2C8 = v298;
				}
				
				else {
					v2C8 = NormalizeDouble(v2C8 / v2A8, 0) * v2A8;
				}
			}
			
			//2C30
			OrderSend(Symbol(), OP_BUY, v2C8, Ask, v254, 0, 0, "Martin-Profit-Expert" + MAGIC, v270);
			
			return;
		}
		
		else {
			//2D0C
			if (!v304) {
				InitParam(v270 + v25C);
				
				if (v2E0 == 0) {
					v2C8 = v2E8;
				}
				
				else {
					v2C8 = v2D8 * v2E0;
				}
				
				//2D90
				
				if (v2C8 < v2A0) {
					v2C8 = v2A0;
				}
				
				else {
					if (v2C8 > v298) {
						v2C8 = v298;
					}
					
					else {
						v2C8 = NormalizeDouble(v2C8 / v2A8, 0) * v2A8;
					}
				}
				
				//2E18
				v2B0 = v2D0 - v284 * Point;
				
				if ((Ask - v26C*Point) >= v2B0 && !v54) {
					OrderSend(Symbol(), OP_BUYLIMIT, v2C8, v2B0, 0, 0, 0, "Martin-Profit-Expert" + MAGIC, v274);
					return;
				}
				
				else {
					//2F78
					if ((v2D0 -Ask) >= (v284*Point)) {
						OrderSend(Symbol(), OP_BUY, v2C8, Ask, 0, 0, 0, "Martin-Profit-Expert" + MAGIC, v274);
					}
				}
				
				v2D0 = 0;
				
				return;
			}
		}
	}
	
}

void Novice::InitParam(int v0) {
	if (!v84) {
		v284 = Pips;
		v2E8 = Lots;
		v2E0 = Doble;
		v278 = TakeProfit1;
		v27C = InitialStop1;
		v280 = TrailingStop1;
		v274 = v270;
	}
	
	if (v0 == v270) {
		v278 = TakeProfit1;
		v2E8 = Lots;
		v27C = InitialStop1;
		v280 = TrailingStop1;
	}
	
	if (v0 == (v270 + 1)) {
		v284 = vD8;
		v2E8 = vDC;
		v2E0 = vE4;
		v278 = vEC;
		v27C = vF0;
		v280 = vF4;
		v274 = v270 + 1;
	}
	
	if (v0 == (v270 + 2)) {
		v284 = v100;
		v2E8 = v104;
		v2E0 = v10C;
		v278 = v114;
		v27C = v118;
		v280 = v11C;
		v274 = v270 + 2;
	}
	
	if (v0 == (v270 + 3)) {
		v284 = v128;
		v2E8 = v12C;
		v2E0 = v134;
		v278 = v13C;
		v27C = v140;
		v280 = v144;
		v274 = v270 + 3;
	}
	
	if (v0 == (v270 + 4)) {
		v284 = v150;
		v2E8 = v154;
		v2E0 = v15C;
		v278 = v164;
		v27C = v168;
		v280 = v16C;
		v274 = v270 + 4;
	}
	
	if (v0 == (v270 + 5)) {
		v284 = v178;
		v2E8 = v17C;
		v2E0 = v184;
		v278 = v18C;
		v27C = v190;
		v280 = v194;
		v274 = v270 + 5;
	}
	
	if (v0 == (v270 + 6)) {
		v284 = v1A0;
		v2E8 = v1A4;
		v2E0 = v1AC;
		v278 = v1B4;
		v27C = v1B8;
		v280 = v1BC;
		v274 = v270 + 6;
	}
	
	if (v0 == (v270 + 7)) {
		v284 = v1C8;
		v2E8 = v1CC;
		v2E0 = v1D4;
		v278 = v1DC;
		v27C = v1E0;
		v280 = v1E4;
		v274 = v270 + 7;
	}
	
	if (v0 == (v270 + 8)) {
		v284 = v1F0    ;
		v2E8 = v1F4;
		v2E0 = v1FC;
		v278 = v204;
		v27C = v208;
		v280 = v20C;
		v274 = v270 + 8;
	}
	
	v284 = v218;
	
	v2E8 = v21C;
	v2E0 = v224;
	v278 = v22C;
	v27C = v230;
	v280 = v234;
	v274 = v270 + 8;
}

}
