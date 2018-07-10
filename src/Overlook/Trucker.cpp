#include "Overlook.h"

#if 0

// Lock & pipsin

namespace Overlook {

Trucker::Trucker() {

}

void Trucker::InitEA() {
	MaxOrd_1 = TotalOrders;
	StartDepo = AccountBalance();
	
	if (GlobalVariableCheck("stop")) {
		GlobalVariableDel("stop");
	}
	
	if (!GlobalVariableCheck("multi")) {
		GlobalVariableSet("multi", 1);
	}
	
	else {
		double set = GlobalVariableGet("multi");
		set++;
		GlobalVariableSet("multi", set);
	}
	
	GlobalVariableSet("Risc", Risk);
	
	GlobalVariableSet("disable", 0);
	
	if (GlobalVariableGet("multi") > 1) {
		SetArrow = false;
	}
	
	if (!GlobalVariableCheck("step")) {
		GlobalVariableSet("step", EqvTralStep);
	}
	
	else {
		if (GlobalVariableGet("step") != EqvTralStep) {
			GlobalVariableSet("step", EqvTralStep);
		}
	}
	
	GlobalVariableSet("pause", MinTradePause);
	
	return;
}

void Trucker::StartEA(int pos) {
	if (OrdersTotal() < 1) {
		Sredstva = AccountBalance();
		
		if (!GlobalVariableCheck("sredstva")) {
			GlobalVariableSet("sredstva", Sredstva);
		}
		
		else {
			if (GlobalVariableGet("sredstva") != Sredstva) {
				GlobalVariableSet("sredstva", Sredstva);
			}
		}
	}
	
	if (GlobalVariableGet("multi") > 1) {
		SetArrow = false;
	}
	
	if ((GlobalVariableGet("multi") - GlobalVariableGet("disable")) > 2) {
		TradeOfTime = false;
	}
	
	if (MargineVarning) {
		if (Varning()) {
			Sleep(10000);
			return(0);
		}
	}
	
	if (!IsDemo() && !IsOptimization() && !IsTesting()) {
		//if(AccountNumber()!=38325){
		Comment("Bad Account!!!");
		Sleep(500);
		Comment("  ");
		return(0);
		//}
	}
	
	eqvtrade = false;
	
//-----------------------------------------------------------------------------+
// Ðàñ÷åò êîëè÷åñòâà îðäåðîâ îò ðèñêà è êîëè÷åñòâà óñòàíîâëåííûõ ñîâåòíèêîâ    |
//-----------------------------------------------------------------------------+
	MaxOrd_1 = TotalOrders;
	
	if (MaxOrd_1 > AccountBalance() / (MarketInfo(Symbol(), MODE_MARGINREQUIRED)*CalcLotsAuto()*2*(GlobalVariableGet("multi") - GlobalVariableGet("disable")))) {
		MaxOrd_1 = (AccountBalance() / (MarketInfo(Symbol(), MODE_MARGINREQUIRED) * CalcLotsAuto() * 2 * (GlobalVariableGet("multi") - GlobalVariableGet("disable"))));
//----------------------------------------------------------------------------
	}
	
	if (CountOpOrd("0") >= MaxOrd_1) {
		if (AccountEquity() == StartDepo) {
			CloseAll();
		}
	}
	
	if (gEqviti == 0) {
		gEqviti = AccountBalance();
	}
	
	if (OrdersTotal() < 1) {
		if (GlobalVariableCheck("stop")) {
			GlobalVariableDel("stop");
		}
		
		gEqviti = AccountBalance();
	}
	
	//if(GrPorfit()<0){if(GlobalVariableCheck("stop")){GlobalVariableDel("stop");}}
	
	if (EcvitiTral3(EqvTralStep)) {
		if (eqvtrade) {
			lot = CalcLotsAuto();
			
			if (OrdersTotal() > 2) {
				if (Martin) {
					lot = OrdersLot();
				}
			}
			
			//if(Typ()==0){OpenPosition("",0,lot,0,0,BuyMagic1);}
			//if(Typ()==1){OpenPosition("",1,lot,0,0,SellMagic1);}
		}
		
		if (!TradeTime()) {
			if (GrPorfit() >= -10) {
				CloseAll();
			}
		}
		
		return(0);
	}
	
	if (GlobalVariableCheck("stop")) {
		return(0);
	}
	
//-----------------------------------------------------------------------------+
// Ðàñ÷åò êîëè÷åñòâà ñîâåòíèêîâ â çàâèñèìîñòè îò ðèñêà íà îäíó ñäåëêó          |
//-----------------------------------------------------------------------------+

	if (GlobalVariableGet("multi") > 1) {
		if (MaxOrd_1 < 15) {
			if (!GlobalVariableCheck("disable")) {
				GlobalVariableSet("disable", 1);
			}
			
			else {
				if (!Expdis) {
					double dis = GlobalVariableGet("disable");
					dis++;
					GlobalVariableSet("disable", dis);
					Expdis = true;
				}
			}
			
			Comment("\nMulti Chart Mode \n",
			
					"STOPPED\n",
					"Because AccountFreeMargin is few"
				   );
			return(0);
		}
		
		if (MaxOrd_1 > 18) {
			if (Expdis) {
				double Exdis = GlobalVariableGet("disable");
				Exdis--;
				
				if (Exdis <= 0) {
					Exdis = 0;
				}
				
				GlobalVariableSet("disable", Exdis);
				
				Expdis = false;
			}
		}
	}
	
//----------------------------------------------------------------------------
	BuySell_1 = 0;
	
	BuySell_2 = 0;
	
	BuySell_3 = 0;
	
	Info();
	
	if (GlobalVariableGet("multi") < 2) {
		if (CountOpOrd("0") >= MaxOrd_1 / 2) {
			//DelBigestLoss2();
			//DelDamage();
			//if(AccountBalance()+GrPorfit()<AccountBalance()-(AccountBalance()/100*10)){CloseAll();}
		}
	}
	
//-----------------------------------------------------------------------------------------------------
	if (NevBar()) {
		if (SetArrow) {
			if (CountOpOrd("0") > 0) {
				SetArrow("NullProfitDot", 4, 1, 0, NullDot(), White);
			}
		}
		
		if (TradeTime()) {
			IfZZ();
			
			if (BuySell_1 < 0) {
				if (CountOpOrd("0") < MaxOrd_1 && TimeOfTrade()) {
					lot = CalcLotsAuto();
					
					if (CountOpOrd("0") > 2) {
						if (Martin) {
							lot = OrdersLot();
						}
					}
					
					if (PorfitBuySell(1) >= PorfitBuySell(0)) {
						OpenPosition("", 0, lot, 0, 0, BuyMagic1);
					}
					
					if (PorfitBuySell(1) < PorfitBuySell(0)) {
						OpenPosition("", 1, lot, 0, 0, SellMagic1);
					}
				}
			}
			
			if (BuySell_1 > 0) {
				if (CountOpOrd("0") < MaxOrd_1 && TimeOfTrade()) {
					lot = CalcLotsAuto();
					
					if (CountOpOrd("0") > 2) {
						if (Martin) {
							lot = OrdersLot();
						}
					}
					
					if (PorfitBuySell(0) >= PorfitBuySell(1)) {
						OpenPosition("", 1, lot, 0, 0, SellMagic1);
					}
					
					if (PorfitBuySell(0) < PorfitBuySell(1)) {
						OpenPosition("", 0, lot, 0, 0, BuyMagic1);
					}
				}
			}
		}
	}
	
//-------------------------------------------------------------------------------------------------------
	return(0);
}



void Trucker::DelBigestLoss() {
	static double gBigLoss = 0;
	double BigLoss = 0, profit = 0, Lossticket, FarLot, FarProfit;
	int Ticket[];
	int count = 0;
	
	if (OrdersTotal() < 2) {
		return;
		gBigLoss = 0;
	}
	
	double MinLot = MarketInfo(Symbol(), MODE_MINLOT);
	
	for (int i = 0;i < OrdersTotal();i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderProfit() < 0) {
					if ((gBigLoss < 0 && OrderProfit() > gBigLoss) || (gBigLoss == 0)) {
						if (OrderProfit() < BigLoss) {
							BigLoss = OrderProfit();
							Lossticket = OrderTicket();
							FarLot = OrderLots();
							FarProfit = OrderProfit();
						}
					}
				}
			}
		}
	}
	
	if (FarLot <= MinLot) {
		gBigLoss = 0;
		return;
	}
	
	for (int n = 0;n < OrdersTotal();n++) {
		if (OrderSelect(n, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderProfit() > 0) {
					profit = profit + OrderProfit();
					count++;
					Ticket[count] = OrderTicket();
					profit = NormalizeDouble(profit, 1);
				}
			}
		}
	}
	
//if(count==1){OneOrdTral(TralStep);return;}

	if (profit + BigLoss < 0) {
		gBigLoss = BigLoss;
		return;
	}
	
	int lot = (FarLot / MinLot);
	
	double MinFarProfit = FarProfit / lot;
	int y;
	double lotstep;
	
	for (y = lot;y > 0;y--) {
		if (profit + (MinFarProfit*y) > MathAbs(MinFarProfit)) {
			lotstep = MinLot * y;
			break;
		}
	}
	
	if (lotstep >= MinLot) {
		ArrayResize(Ticket, count + 1);
		
		for (int x = 0;x < count + 1;x++) {
			if (OrderSelect(Ticket[x], SELECT_BY_TICKET)) {
				if (OrderCloseTime() == 0) {
					del(OrderTicket());
				}
			}
		}
		
		if (OrderSelect(Lossticket, SELECT_BY_TICKET)) {
			if (OrderCloseTime() == 0) {
				dellot(Lossticket, lotstep);
				return(true);
			}
		}
	}
	
	return;
}

//-----------------------------------------------------------------------------+
//Óäàëÿåò ðûíî÷íûé îðäåð ñ óêàçàííûì åé òèêåòîì è ëîòîì                        |
//+----------------------------------------------------------------------------+
void Trucker::dellot(int ticket, double lot) {
	int err;
	int step;
	
	if (MarketInfo(Symbol(), MODE_LOTSTEP) == 0.1) {
		step = 1;
	}
	
	if (MarketInfo(Symbol(), MODE_LOTSTEP) == 0.01) {
		step = 2;
	}
	
	lot = NormalizeDouble(lot, step);
	
	for (int i = 0;i < 1;i++) {
		GetLastError();//îáíóëÿåì îøèêó
		OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES);
		String symbol = OrderSymbol();
		
		if (OrderType() == OP_BUY) {
			RefreshRates();
			double prise = MarketInfo(symbol, MODE_BID);
			
			if (!OrderClose(ticket, lot, prise, 3, Green)) {
				err = GetLastError();
			}
		}
		
		if (OrderType() == OP_SELL) {
			RefreshRates();
			prise = MarketInfo(symbol, MODE_ASK);
			
			if (!OrderClose(ticket, lot, prise, 3, Green)) {
				err = GetLastError();
			}
		}
		
		if (err == 0) {
			PlaySound("expert.wav");
			break;
		}
		
		if (err != 0) {
			PlaySound("timeout.wav");
			Print("Error for Close Funtion =", err);
		}
		
		while (!IsTradeAllowed()) {
			Sleep(5000);    // åñëè ðûíîê çàíÿò òî ïîäîæäåì 5 ñåê
		}
		
		if (err == 146)
			while (IsTradeContextBusy())
				Sleep(1000*11);
	}
}

//-----------------------------------------------------------------------------+
// Èñùåò ñàìûé äàëüíèé îðäåð ñ îòðèöàòåëüíîé ïðèáûëüþ                          |
//-----------------------------------------------------------------------------+
int Trucker::FarOrder() {
	double dist = 0;
	int tick = 0;
//if(OrdersTotal()<1)return;

	for (int i = 0;i < OrdersTotal();i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderProfit() < 0) {
					RefreshRates();
					
					if ((MathAbs(OrderOpenPrice() - Ask) / Point) > dist) {
						dist = (MathAbs(OrderOpenPrice() - Ask)) / Point;
						tick = OrderTicket();
					}
				}
			}
		}
	}
	
	return(tick);
}

//+----------------------------------------------------------------------------+
// Óäàëåíèå ñàìîãî äàëüíåãî îðäåðà                                             +
//-----------------------------------------------------------------------------+
bool Trucker::DelDamage() {
	double FarProfit = 1, FarLot = 0, profit = 0;
	int count = -1, x;
//if(OrdersTotal()<1)return;
	double spred = NormalizeDouble(MarketInfo(Symbol(), MODE_SPREAD) * MarketInfo(Symbol(), MODE_TICKVALUE), Digits);
	double PipsPrise = MarketInfo(Symbol(), MODE_TICKVALUE);
	double MinLot = MarketInfo(Symbol(), MODE_MINLOT);
	int Ticket[];
	
	if (FarOrder() > 0) {
		if (OrderSelect(FarOrder(), SELECT_BY_TICKET)) {
			if (OrderCloseTime() == 0) {
				FarLot = OrderLots();
				FarProfit = OrderProfit();
			}
		}
	}
	
	if (FarProfit < 0) {
		profit = 0;
		
		for (int n = 0;n < OrdersTotal();n++) {
			if (OrderSelect(n, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderSymbol() == Symbol()) {
					if (OrderProfit() > 0) {
						profit = profit + OrderProfit();
						count++;
						Ticket[count] = OrderTicket();
						profit = NormalizeDouble(profit, 1);
						FarProfit = NormalizeDouble(FarProfit, 1);
					}
				}
			}
		}
		
		if (profit + FarProfit > spred + (MInProfit*Bid*PipsPrise)) {
			ArrayResize(Ticket, count + 1);
			
			for (x = 0;x < count + 1;x++) {
				if (OrderSelect(Ticket[x], SELECT_BY_TICKET)) {
					if (OrderCloseTime() == 0) {
						del(OrderTicket());
					}
				}
			}
			
			if (OrderSelect(FarOrder(), SELECT_BY_TICKET)) {
				if (OrderCloseTime() == 0) {
					del(FarOrder());
					return(true);
				}
			}
		}
		
		if (profit + FarProfit < 0 && profit > 0) {
			if (FarLot < MinLot*2) {
				return(false);
			}
			
			int lot = (FarLot / MinLot);
			
			double MinFarProfit = FarProfit / lot;
			int y;
			double lotstep;
			
			for (y = lot;y > 0;y--) {
				if (profit + (MinFarProfit*y) > 0) {
					lotstep = MinLot * y;
					break;
				}
			}
			
			if (lotstep >= MinLot) {
				ArrayResize(Ticket, count + 1);
				
				for (x = 0;x < count + 1;x++) {
					if (OrderSelect(Ticket[x], SELECT_BY_TICKET)) {
						if (OrderCloseTime() == 0) {
							del(OrderTicket());
						}
					}
				}
				
				if (OrderSelect(FarOrder(), SELECT_BY_TICKET)) {
					if (OrderCloseTime() == 0) {
						dellot(FarOrder(), lotstep);
						return(true);
					}
				}
			}
		}
		
	}
	
	return(false);
}

//-----------------------------------------------------------------------------+
// Òèò ïîñëåäíåãî óñòàíîâëåííîãî îðäåðà                                        |
//-----------------------------------------------------------------------------+
int Trucker::Typ() {
	int time = 0, tip = -1;
	
	for (int i = 0;i < OrdersTotal();i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderOpenTime() > time) {
					time = OrderOpenTime();
					tip = OrderMagicNumber();
				}
			}
		}
	}
	
	if (tip == BuyMagic1) {
		return(0);
	}
	
	if (tip == SellMagic1) {
		return(1);
	}
	
	return(-1);
}

//-----------------------------------------------------------------------------+
// Òî÷êà ðàâíîâåñèÿ                                                            |
//-----------------------------------------------------------------------------+
double Trucker::NullDot() {
	double prise = 0;
	int count = 0, countlot = 0;
	
	if (OrdersTotal() < 1) {
		return(Bid);
	}
	
	double MinLot = MarketInfo(Symbol(), MODE_MINLOT);
	
	for (int i = 0;i < OrdersTotal();i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				countlot = (OrderLots() / MinLot);
				count = count + countlot;
				prise = prise + (OrderOpenPrice() * countlot);
			}
		}
	}
	
	prise = prise / count;
	
	return(prise);
}

//+----------------------------------------------------------------------------+
//       Óäàëÿåò ñàìîãî áîëüøîãî ëîñÿ                                          +
//+----------------------------------------------------------------------------+
void Trucker::DelBigestLoss2() {
	if (CountOpOrd("0") < 2) {
		return;
	}
	
	double loss = 0, profit = 0, GrProfit = 0;
	
	int n, x, y, ticket = 0, count = 0;
	int Ticket[50];
	ArrayInitialize(Ticket, 0);
	
	for (n = 0;n < OrdersTotal();n++) {
		if (OrderSelect(n, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderProfit() + OrderSwap() + OrderCommission() > 0) {
					profit = profit + OrderProfit() + OrderSwap() + OrderCommission();
				}
				
				else {
					if (OrderProfit() + OrderSwap() + OrderCommission() < loss) {
						loss = OrderProfit() + OrderSwap() + OrderCommission();
						ticket = OrderTicket();
					}
				}
			}
		}
	}
	
	if (ticket == 0) {
		return;
	}
	
	if (profit + loss >= 0) {
		profit = 0;
		
		for (x = 0;x < OrdersTotal();x++) {
			for (n = 0;n < OrdersTotal();n++) {
				if (OrderSelect(n, SELECT_BY_POS, MODE_TRADES)) {
					if (OrderProfit() + OrderSwap() + OrderCommission() > 0) {
						profit = profit + OrderProfit() + OrderSwap() + OrderCommission();
						
						if (OrderProfit() + OrderSwap() + OrderCommission() < profit) {
							profit = OrderProfit() + OrderSwap() + OrderCommission();
						}
					}
				}
			}
			
			if (profit != 0) {
				Ticket[count] = OrderTicket();
				count++;
				
				for (n = 0;n < 1;n++) {
					GrProfit = GrProfit + profit;
				}
				
				if (GrProfit + loss >= 0) {
					ArrayResize(Ticket, count + 1);
					Comment(GrProfit, " = ", loss);
					
					for (y = 0;y < count + 1;y++) {
						if (OrderSelect(Ticket[y], SELECT_BY_TICKET)) {
							if (OrderCloseTime() == 0) {
								del(OrderTicket());
							}
						}
					}
					
					if (OrderSelect(ticket, SELECT_BY_TICKET)) {
						if (OrderCloseTime() == 0) {
							del(ticket);
							return(true);
						}
					}
				}
				
			}//if(profit!=0){
		}//for(x=0;x<OrdersTotal();x++){
	}//if(profit+loss>=0){
}

//+----------------------------------------------------------------------------+
// Ïîäñ÷èòûâàåò Îáùóþ íå ðåàëèçîâàííóþ ïðèáûëü íà äàííîì èíñòðóìåíòå           +
//-----------------------------------------------------------------------------+
double Trucker::GrPorfit() {
	double grpr = 0;
	
	for (int n = 0;n < OrdersTotal();n++) {
		if (OrderSelect(n, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				grpr = grpr + OrderProfit() + OrderSwap() + OrderCommission();
			}
		}
	}
	
	return(grpr);
}

//-----------------------------------------------------------------------------+
// Ñðåäíåå àðèôìåòè÷åñêîå ëîòà óñòàíîâëåííûõ îðäåðîâ                           |
//-----------------------------------------------------------------------------+
double Trucker::OrdersLot() {
	int count = 0;
	double Lt = 0;
//if(OrdersTotal()<5){return(CalcLotsAuto());}

	for (int i = 0;i < OrdersTotal();i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderCloseTime() == 0) {
					Lt = Lt + OrderLots();
					count++;
				}
			}
		}
	}
	
	return(NormalizeDouble((Lt / (count*MartinKoff)), 2));
}

//+------------------------------------------------------------------+
//| Ðàñ÷åò ëîòà ñîîòâ ðèñêó è áàëàíñó                                |
//+------------------------------------------------------------------+
double Trucker::CalcLotsAuto() {
	double LotOpt, LotNeOpt, Zalog;
	RefreshRates();
	double Set = GlobalVariableGet("multi") - GlobalVariableGet("disable");
	double lott = MarketInfo(Symbol(), MODE_MARGINREQUIRED) / 1000;
	double Marga = AccountFreeMargin();
	double Balans = AccountBalance();
	double LotMin = MarketInfo(Symbol(), MODE_MINLOT);
	double LotMax = MarketInfo(Symbol(), MODE_MAXLOT);
	double StepLot = MarketInfo(Symbol(), MODE_LOTSTEP);
	double StopLv = AccountStopoutLevel();
	int PrsMinLot = 1000 * LotMin;
	
	if (GlobalVariableGet("multi") > 1) {
		Risk = GlobalVariableGet("Risc");
	}
	
	if (Risk < 0)
		Risk = 0;
		
	if (Risk > 100)
		Risk = 100;
		
	if (StepLot == 0.01) {
		int step = 2;
	}
	
	else {
		step = 1;
	}
	
	if (Set < 1) {
		Set = 1;
	}
	
//---------------------------
	Zalog = (Balans * (Bid * Risk / 100)) / Set;
	
	LotOpt = NormalizeDouble((Zalog / 1000), step);
	
	if (LotOpt > LotMax)
		LotOpt = LotMax;
		
	if (LotOpt < LotMin)
		LotOpt = LotMin;
		
	//if(Marga<Sredstva/2){return(0);}
	return(LotOpt);
}

//+------------------------------------------------------------------+
//| Óñòàíîâêà ñòðåëîê                                                |
//+------------------------------------------------------------------+
void Trucker::SetArrow(String nm, int kod, int razm, int bar, double prs, color col) {
//if(!DriveArrow)return;
	String Name = StringConcatenate(nm, TimeToStr(TimeLocal(), TIME_DATE | TIME_SECONDS));
	ObjectCreate(Name, OBJ_ARROW, 0, 0, 0, 0, 0, 0, 0);
	ObjectSet(Name, OBJPROP_ARROWCODE, kod);
	ObjectSet(Name, OBJPROP_STYLE, DRAW_ARROW);
	ObjectSet(Name, OBJPROP_TIME1, Time[bar]);
	ObjectSet(Name, OBJPROP_PRICE1, prs);
	ObjectSet(Name, OBJPROP_WIDTH, razm);
	ObjectSet(Name, OBJPROP_COLOR, col);
	return;
}

//-----------------------------------------------------------------------------+
// Äåòåêòîð ìóëüòè ðåæèìà ñ÷åò÷èê óñòàíîâëåííûõ ñîâåòíèêîâ                     |
//-----------------------------------------------------------------------------+
String Trucker::MutiDet() {
	double set = GlobalVariableGet("multi") - GlobalVariableGet("disable");
	String mode = StringConcatenate("MULTI CHART MODE \n", DoubleToStr(set, 0), " EXPERT ENABLES");
	String mode2 = "SINGLE CHART MODE";
	
	if (GlobalVariableGet("multi") < 2) {
		return(mode2);
	}
	
	if (GlobalVariableGet("multi") > 1) {
		return(mode);
	}
}


//-----------------------------------------------------------------------------+
// Âûâîäèò ïðåäóïðåæäàþùåå ñîîáùåíèå                                           |
//-----------------------------------------------------------------------------+
bool Trucker::Varning() {
	int ord;
	double depo;
	
	if ((MarketInfo(Symbol(), MODE_MARGINREQUIRED)*CalcLotsAuto()*MaxOrd_1*2) > StartDepo) {
		depo = MarketInfo(Symbol(), MODE_MARGINREQUIRED) * CalcLotsAuto() * MaxOrd_1 * 2;
		ord = StartDepo / (MarketInfo(Symbol(), MODE_MARGINREQUIRED) * CalcLotsAuto() * 2);
		Alert("THERE IS NOT ENOUGH MEANS FOR THE ACCOUNT" + "\n" +
			  "For such quantity of warrants it is necessary" + "\n" +
			  "The starting Deposit =" + depo + " $\n" +
			  "On your means it is possible to open " + ord + " Orders" + "\n" +
			  "Fill up your account please" + "\n" +
			  "Or reduce quantity of warrants" + "\n" +
			  "Or reduce a traded lot" + "\n" +
			  "Or disconnect the alarm system");
		return(true);
	}
	
	return(false);
}

//-----------------------------------------------------------------------------+
// Ôóíêöèÿ êîíòðîëÿ âðåìåíè òîðãîâëè                                           |
//-----------------------------------------------------------------------------+
bool Trucker::TradeTime() {
	int StartTradeHour = StartTrade;
	int EndTradeHour = EndTrade;
	
	if (!TradeOfTime) {
		gTime = StringConcatenate("the end of trading on Friday in ", DoubleToStr(EndTradeHour, 0), "hours");
//if(DayOfWeek()==5){if(TimeHour(TimeCurrent())>EndTradeHour){return(false);}}
		return(true);
	}
	
//if(DayOfWeek()==5){StartTradeHour=0;if(TimeHour(TimeCurrent())>EndTradeHour){return(false);}}else{StartTradeHour=StartTrade;}

	if (DayOfWeek() == 1) {
		if (TimeHour(TimeCurrent()) < 1) {
			StartTradeHour = 1;
		}
	}
	
	else {
		StartTradeHour = StartTrade;
	}
	
	if (StartTradeHour < EndTradeHour) {
		if (TimeHour(TimeCurrent()) >= StartTradeHour && TimeHour(TimeCurrent()) < EndTradeHour
			|| (CountOpOrd("0") + CountOpOrd("1"))) {
			gTime = StringConcatenate("By the end of trading session :", DoubleToStr((EndTradeHour - TimeHour(TimeCurrent())), 0), " hours");
			return(true);
		}
	}
	
	if (StartTradeHour > EndTradeHour) {
		if (TimeHour(TimeCurrent()) >= StartTradeHour || TimeHour(TimeCurrent()) < EndTradeHour
			|| (CountOpOrd("0") + CountOpOrd("1"))) {
			gTime = StringConcatenate("By the end of trading session :", DoubleToStr((EndTradeHour - TimeHour(TimeCurrent())), 0), " hours");
			return(true);
		}
	}
	
	gTime = StringConcatenate("resting to : ", StartTradeHour, ": 00");
	
	return(false);
}

//-----------------------------------------------------------------------------+
// Óñëîâèÿ âõîäà â ðûíîê                                                       |
//-----------------------------------------------------------------------------+
int Trucker::IfZZ() {
//if(iClose(Symbol(),30,1)>iOpen(Symbol(),30,0)){
	if (iClose(Symbol(), 15, 1) < iOpen(Symbol(), 15, 0)) {
		if (iClose(Symbol(), 5, 1) < iOpen(Symbol(), 5, 0)) {
			BuySell_1 = -1;
			gCountBuy_1++;
			gCountSell_1 = 1;
			return(1);
		}
	}//}
	
//if(iClose(Symbol(),30,1)<iOpen(Symbol(),30,0)){

	if (iClose(Symbol(), 15, 1) > iOpen(Symbol(), 15, 0)) {
		if (iClose(Symbol(), 5, 1) > iOpen(Symbol(), 5, 0)) {
			BuySell_1 = 1;
			gCountBuy_1 = 1;
			gCountSell_1++;
			return(-1);
		}
	}//}
	
	BuySell_1 = 0;
	
	return(0);
}

//+------------------------------------------------------------------+
//|Òðàë ïî ýêâèòè                                                    |
//+------------------------------------------------------------------+
bool Trucker::EcvitiTral3(double EqvTralStep) {
	if (OrdersTotal() < 1) {
		gEqviti = AccountBalance();
		return(false);
	}
	
	if (!GlobalVariableCheck("step")) {
		EqvTralStep = EqvTralStep;
	}
	
	else {
		EqvTralStep = GlobalVariableGet("step");
	}
	
	EqvTralStep = EqvTralStep * (GlobalVariableGet("multi") - GlobalVariableGet("disable"));
	
	if (CountOpOrd("0") >= MaxOrd_1 / 2) {
		if (AccountEquity() >= AccountBalance()) {
			if (AccountEquity() - AccountBalance() > ((AccountBalance() / 100)*(EqvTralStep*2))) {
				EqvTralStep = EqvTralStep * 2;
			}
			
			if (AccountEquity() > (gEqviti + (gEqviti / 100*EqvTralStep))) {
				gEqviti = gEqviti + (gEqviti / 200 * EqvTralStep);
				eqvtrade = true;
				
				if (!GlobalVariableCheck("stop")) {
					GlobalVariableSet("stop", 0);
				}
			}
			
			if (AccountEquity() <= gEqviti) {
				CloseAll();
				Print("CloseAll");
				GlobalVariableDel("stop");
				return(false);
			}
			
			else {
				Comment(AccountFreeMargin() + AccountMargin(), "\n",
						"EQUITY TRALING MODE\n",
						"EQUITY TRALING STEP   =", EqvTralStep, " %",
						"\nACCOUNT BALANS         = ", AccountBalance(),
						"\nCLOSE LEVEL                  = ", gEqviti,
						"\nACCOUNT EQUITY        = ", AccountEquity(),
						"\nNEXT STEP                     = ", gEqviti + (gEqviti / 100*EqvTralStep));
				return(true);
			}
		}
		
		else {
			GlobalVariableDel("stop");
		}
	}
	
	else {
		if (AccountEquity() > AccountBalance()) {
			if (AccountEquity() - AccountBalance() > ((AccountBalance() / 100)*(EqvTralStep*2))) {
				EqvTralStep = EqvTralStep * 2;
			}
			
			if (AccountEquity() > (gEqviti + (gEqviti / 100*EqvTralStep))) {
				gEqviti = gEqviti + (gEqviti / 200 * EqvTralStep);
				eqvtrade = true;
				
				if (!GlobalVariableCheck("stop")) {
					GlobalVariableSet("stop", 0);
				}
			}
			
			if (AccountEquity() <= gEqviti) {
				CloseAll();
				Print("CloseAll");
				GlobalVariableDel("stop");
				return(false);
			}
			
			else {
				Comment(AccountFreeMargin() + AccountMargin(), "\n",
						"EQUITY TRALING MODE\n",
						"EQUITY TRALING STEP   =", EqvTralStep, " %",
						"\nACCOUNT BALANS         = ", AccountBalance(),
						"\nCLOSE LEVEL                  = ", gEqviti,
						"\nACCOUNT EQUITY        = ", AccountEquity(),
						"\nNEXT STEP                     = ", gEqviti + (gEqviti / 100*EqvTralStep));
				return(true);
			}
		}
		
		else {
			GlobalVariableDel("stop");
		}
	}
	
	return(false);
}

//+------------------------------------------------------------------+
//| ïðîâåðÿåò ðûíî÷íûé îðäåð íà ïðèíàäëåæíîñòü ñîãëàñíî óñëîâèÿì     |
//+------------------------------------------------------------------+
bool Trucker::IfOrder(String Sy, int Typ, int Magik) {
	if (Sy == "0") {
		Sy = Symbol();
	}
	
	if (OrderSymbol() == Sy || Sy == "") {
		if (OrderType() == Typ || Typ == (-1)) {
			if (OrderMagicNumber() == Magik || Magik == (-1)) {
				return(true);
			}
		}
	}
	
	return(false);
}

//+------------------------------------------------------------------+
//| Ïîäñ÷èòûâàåò êîëè÷åñòâî îòêðûòûõ îðäåðîâ ñîãëàñíî óñëîâèÿì       |
//+------------------------------------------------------------------+
int Trucker::CountOpOrd(String Sy, int Typ, int Magik) {
	int count = 0;
	
	if (Sy == "0") {
		Sy = Symbol();
	}
	
	for (int i = 0;i < OrdersTotal();i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Sy || Sy == "") {
				if (OrderType() == Typ || Typ == (-1)) {
					if (OrderMagicNumber() == Magik || Magik == (-1)) {
						count++;
					}
				}
			}
		}
	}
	
	return(count);
}

//+----------------------------------------------------------------------------+
// Ïîäñ÷èòûâàåò Îáùóþ íå ðåàëèçîâàííóþ ïðèáûëü íà äàííîì èíñòðóìåíòå           +
//-----------------------------------------------------------------------------+
double Trucker::PorfitBuySell(int tip) {
	double grpr = 0;
	
	if (OrdersTotal() < 1) {
		return(-1);
	}
	
	for (int n = OrdersTotal() + 1;n >= 0;n--) {
		if (OrderSelect(n, SELECT_BY_POS, MODE_TRADES)) {
			if (IfOrder("0", tip)) {
				grpr = grpr + OrderProfit();
			}
		}
	}
	
	return(grpr);
}

//+----------------------------------------------------------------------------+
// Çàêðûâàåò âñå îðäåðà íà äàííîì èíñòðóìåíòå                                  +
//-----------------------------------------------------------------------------+
void Trucker::CloseAll() {
	while (OrdersTotal() > 0) {
		double profit = 1000000;
		
		for (int n = OrdersTotal() + 1;n >= 0;n--) {
			if (OrderSelect(n, SELECT_BY_POS, MODE_TRADES)) {
				if (OrderProfit() < profit) {
					profit = OrderProfit();
				}
			}
		}
		
		del(OrderTicket());
	}
	
	return;
}

//+----------------------------------------------------------------------------+
// Çàäàåò ïàóçó ìåæäó óñòàíîâêàìè îðäåðîâ                                      +
//-----------------------------------------------------------------------------+
bool Trucker::TimeOfTrade() {
	int count = 0, distanse = 0;
	double OpPrs = 0;
	
	if (CountOpOrd("0") < 1) {
		return(true);
	}
	
	if (GlobalVariableCheck("pause")) {
		int pause = GlobalVariableGet("pause");
	}
	
	else {
		pause = MinTradePause;
		GlobalVariableSet("pause", MinTradePause);
	}
	
	for (int n = 0;n < OrdersTotal();n++) {
		if (OrderSelect(n, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol()) {
				if (OrderOpenTime() > count) {
					count = OrderOpenTime();
					OpPrs = OrderOpenPrice();
				}
			}
		}
	}
	
	if ((MathAbs(OpPrs - Bid) / Point) < 10) {
		return(false);
	}
	
	if (OpPrs != 0) {
		distanse = (MathAbs(OpPrs - Bid) / Point) / 20 * (Bid * Point);
	}
	
	if ((TimeCurrent() - count) > ((MinTradePause*(distanse + 1))*60)) {
		return(true);
	}
	
	return(false);
}

//-----------------------------------------------------------------------------+
// Ôóíêöèÿ êîíòðîëÿ íîâîãî áàðà                                                |
//-----------------------------------------------------------------------------+
bool Trucker::NevBar() {
	static int PrevTime = 0;
	
	if (PrevTime == Now)
		return(false);
		
	PrevTime = Now;
	
	return(true);
}

//-----------------------------------------------------------------------------+
//Óäàëÿåò ðûíî÷íûé îðäåð ñ óêàçàííûì åé òèêåòîì                                |
//+----------------------------------------------------------------------------+
void Trucker::del(int ticket) {
	int err;
	
	for (int i = 0;i < 1;i++) {
		GetLastError();//îáíóëÿåì îøèêó
		OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES);
		String symbol = OrderSymbol();
		
		if (OrderType() == OP_BUY) {
			RefreshRates();
			double prise = MarketInfo(symbol, MODE_BID);
			
			if (!OrderClose(ticket, OrderLots(), prise, 3, Green)) {
				err = GetLastError();
			}
		}
		
		if (OrderType() == OP_SELL) {
			RefreshRates();
			prise = MarketInfo(symbol, MODE_ASK);
			
			if (!OrderClose(ticket, OrderLots(), prise, 3, Green)) {
				err = GetLastError();
			}
		}
		
		if (err == 0) {
			PlaySound("expert.wav");
			break;
		}
		
		if (err != 0) {
			PlaySound("timeout.wav");
			Print("Error for Close Funtion =", err);
		}
		
		while (!IsTradeAllowed()) {
			Sleep(5000);    // åñëè ðûíîê çàíÿò òî ïîäîæäåì 5 ñåê
		}
		
		if (err == 146)
			while (IsTradeContextBusy())
				Sleep(1000*11);
	}
}

//-----------------------------------------------------------------------------+
//___________________ÊÈÌÓ_______ÐÅÑÏÅÊÒ________È_____ÓÂÀÆÓÕÀ___________________|
//+----------------------------------------------------------------------------+
//|  Àâòîð    : Êèì Èãîðü Â. aka KimIV,  http://www.kimiv.ru                   |
//+----------------------------------------------------------------------------+
//|  Âåðñèÿ   : 01.09.2005                                                     |
//|  Îïèñàíèå : Âûâîä ñîîáùåíèÿ â êîììåíò è â æóðíàë                           |
//+----------------------------------------------------------------------------+
//|  Ïàðàìåòðû:                                                                |
//|    m - òåêñò ñîîáùåíèÿ                                                     |
//+----------------------------------------------------------------------------+
void Trucker::Message(String m) {
	//Comment(m);
	if (StringLen(m) > 0)
		Print(m);
}

//+----------------------------------------------------------------------------+
//|  Àâòîð    : Êèì Èãîðü Â. aka KimIV,  http://www.kimiv.ru                   |
//+----------------------------------------------------------------------------+
//|  Âåðñèÿ   : 28.11.2006                                                     |
//|  Îïèñàíèå : Ìîäèôèêàöèÿ îäíîãî ïðåäâàðèòåëüíî âûáðàííîãî îðäåðà.           |
//+----------------------------------------------------------------------------+
//|  Ïàðàìåòðû:                                                                |
//|    pp - öåíà óñòàíîâêè îðäåðà                                              |
//|    sl - öåíîâîé óðîâåíü ñòîïà                                              |
//|    tp - öåíîâîé óðîâåíü òåéêà                                              |
//|    ex - äàòà èñòå÷åíèÿ                                                     |
//+----------------------------------------------------------------------------+
void Trucker::ModifyOrder(double pp, double sl, double tp, Time ex) {
	bool   fm;
	color  cl = White;
	double op, pa, pb, os, ot;
	int    dg = MarketInfo(OrderSymbol(), MODE_DIGITS), er, it;
	
	if (pp <= 0)
		pp = OrderOpenPrice();
		
	if (sl < 0)
		sl = OrderStopLoss();
		
	if (tp < 0)
		tp = OrderTakeProfit();
		
	pp = NormalizeDouble(pp, dg);
	
	sl = NormalizeDouble(sl, dg);
	
	tp = NormalizeDouble(tp, dg);
	
	op = NormalizeDouble(OrderOpenPrice() , dg);
	
	os = NormalizeDouble(OrderStopLoss()  , dg);
	
	ot = NormalizeDouble(OrderTakeProfit(), dg);
	
	//if(OrderType()==0||OrderType()==2||OrderType()==4){if(Ask-stoplevel<sl||Bid+stoplevel>tp){return;}}
	//if(OrderType()==1||OrderType()==3||OrderType()==5){if(Bid+stoplevel>sl||Ask-stoplevel<tp){return;}}
	//if(MathAbs(sl-Ask)<stoplevel||MathAbs(tp-Ask)<stoplevel){return;}
	if (pp == op && sl == os && tp == ot) {
		return;
	}
	
	if ((sl == os && (os != 0 || sl != 0)) && (tp == ot && (ot != 0 || tp != 0))) {
		return;
	}
	
	for (it = 1; it <= NumberOfTry; it++) {
		//if ((!IsExpertEnabled() || IsStopped())) break;
		while (!IsTradeAllowed())
			Sleep(5000);
			
		RefreshRates();
		
		fm = OrderModify(OrderTicket(), pp, sl, tp, ex, cl);
		
		if (fm) {
			if (UseSound)
				PlaySound(NameFileSound);
				
			break;
		}
		
		else {
			er = GetLastError();
			pa = MarketInfo(OrderSymbol(), MODE_ASK);
			pb = MarketInfo(OrderSymbol(), MODE_BID);
			
			if (print) {
				Print("Error(", er, ") modifying order: ", ErrorDescription(er), ", try ", it);
				Print("Ask=", pa, "  Bid=", pb, "  sy=", OrderSymbol(),
					  "  op=" + GetNameOP(OrderType()), "  pp=", pp, "  sl=", sl, "  tp=", tp);
			}
			
			Sleep(1000*10);
			
			//if(er==130){del(OrderTicket());}
		}
	}
	
	//}
}

//+----------------------------------------------------------------------------+
//|  Àâòîð    : Êèì Èãîðü Â. aka KimIV,  http://www.kimiv.ru                   |
//+----------------------------------------------------------------------------+
//|  Âåðñèÿ   : 06.03.2008                                                     |
//|  Îïèñàíèå : Âîçâðàùàåò ôëàã ñóùåñòâîâàíèÿ ïîçèöèé                          |
//+----------------------------------------------------------------------------+
//|  Ïàðàìåòðû:                                                                |
//|    sy - íàèìåíîâàíèå èíñòðóìåíòà   (""   - ëþáîé ñèìâîë,                   |
//|                                     NULL - òåêóùèé ñèìâîë)                 |
//|    op - îïåðàöèÿ                   (-1   - ëþáàÿ ïîçèöèÿ)                  |
//|    mn - MagicNumber                (-1   - ëþáîé ìàãèê)                    |
//|    ot - âðåìÿ îòêðûòèÿ             ( 0   - ëþáîå âðåìÿ îòêðûòèÿ)           |
//+----------------------------------------------------------------------------+
bool Trucker::ExistPositions(String sy, int op, int mn, Time ot) {
	int i, k = OrdersTotal();
	
	if (sy == "0")
		sy = Symbol();
		
	for (i = 0; i < k; i++) {
		if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == sy || sy == "") {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					if (op < 0 || OrderType() == op) {
						if (mn < 0 || OrderMagicNumber() == mn) {
							if (ot <= OrderOpenTime())
								return(true);
						}
					}
				}
			}
		}
	}
	
	return(false);
}

//+----------------------------------------------------------------------------+
//|  Àâòîð    : Êèì Èãîðü Â. aka KimIV,  http://www.kimiv.ru                   |
//+----------------------------------------------------------------------------+
//|  Âåðñèÿ   : 01.09.2005                                                     |
//|  Îïèñàíèå : Âîçâðàùàåò íàèìåíîâàíèå òîðãîâîé îïåðàöèè                      |
//+----------------------------------------------------------------------------+
//|  Ïàðàìåòðû:                                                                |
//|    op - èäåíòèôèêàòîð òîðãîâîé îïåðàöèè                                    |
//+----------------------------------------------------------------------------+
String Trucker::GetNameOP(int op) {
	switch (op) {
	
	case OP_BUY      :
		return("Buy");
		
	case OP_SELL     :
		return("Sell");
		
	case OP_BUYLIMIT :
		return("BuyLimit");
		
	case OP_SELLLIMIT:
		return("SellLimit");
		
	case OP_BUYSTOP  :
		return("BuyStop");
		
	case OP_SELLSTOP :
		return("SellStop");
		
	default          :
		return("Unknown Operation");
	}
}

//+----------------------------------------------------------------------------+
//|  Àâòîð    : Êèì Èãîðü Â. aka KimIV,  http://www.kimiv.ru                   |
//+----------------------------------------------------------------------------+
//|  Âåðñèÿ   : 10.04.2008                                                     |
//|  Îïèñàíèå : Îòêðûâàåò ïîçèöèþ ïî ðûíî÷íîé öåíå.                            |
//+----------------------------------------------------------------------------+
//|  Ïàðàìåòðû:                                                                |
//|    sy - íàèìåíîâàíèå èíñòðóìåíòà   (NULL èëè "" - òåêóùèé ñèìâîë)          |
//|    op - îïåðàöèÿ                                                           |
//|    ll - ëîò                                                                |
//|    sl - óðîâåíü ñòîï                                                       |
//|    tp - óðîâåíü òåéê                                                       |
//|    mn - MagicNumber                                                        |
//+----------------------------------------------------------------------------+
void Trucker::OpenPosition(String sy, int op, double ll, double Sl, double Tp, int mn) {
	color    clOpen;
	Time ot;
	double   pp, pa, pb, tp , sl;
	int      dg, err, it, ticket = 0;
	String   lsComm = "";
	
	
	
	if (sy == "" || sy == "0")
		sy = Symbol();
		
	int stoplevel = MarketInfo(sy, MODE_STOPLEVEL);
	
	int spread = MarketInfo(sy, MODE_SPREAD);
	
	if (Tp != 0 && Tp < stoplevel + spread) {
		Tp = stoplevel;
	}
	
	if (op < 1) {
		if (Sl > 0) {
			sl = MarketInfo(Symbol(), MODE_ASK) - Sl * Point;
		}
		
		else {
			sl = 0;
		}
		
		if (Tp > 0) {
			tp = MarketInfo(Symbol(), MODE_ASK) + Tp * Point;
		}
		
		else {
			tp = 0;
		}
	}
	
	else {
		if (Sl > 0) {
			sl = MarketInfo(Symbol(), MODE_BID) + Sl * Point;
		}
		
		else {
			sl = 0;
		}
		
		if (Tp > 0) {
			tp = MarketInfo(Symbol(), MODE_BID) - Tp * Point;
		}
		
		else {
			tp = 0;
		}
	}
	
	if (op == OP_BUY)
		clOpen = clOpenBuy;
	else
		clOpen = clOpenSell;
		
	for (it = 1; it <= NumberOfTry; it++) {
		if (IsStopped()) {
			Print("OpenPosition(): Îñòàíîâêà ðàáîòû ôóíêöèè");
			break;
		}
		
		while (!IsTradeAllowed())
			Sleep(5000);
			
		RefreshRates();
		
		dg = MarketInfo(sy, MODE_DIGITS);
		
		pa = MarketInfo(sy, MODE_ASK);
		
		pb = MarketInfo(sy, MODE_BID);
		
		if (op == OP_BUY)
			pp = pa;
		else
			pp = pb;
			
		pp = NormalizeDouble(pp, dg);
		
		ot = TimeCurrent();
		
		if (MarketWatch)
			ticket = OrderSend(sy, op, ll, pp, Slippage, 0, 0, lsComm, mn, 0, clOpen);
		else
			ticket = OrderSend(sy, op, ll, pp, Slippage, sl, tp, lsComm, mn, 0, clOpen);
			
		if (ticket > 0) {
			if (UseSound)
				PlaySound(NameFileSound);
				
			break;
		}
		
		else {
			err = GetLastError();
			
			if (pa == 0 && pb == 0)
				Message("Ïðîâåðüòå â Îáçîðå ðûíêà íàëè÷èå ñèìâîëà " + sy);
				
			// Âûâîä ñîîáùåíèÿ îá îøèáêå
			if (print) {
				Print("Error(", err, ") opening position: ", ErrorDescription(err), ", try ", it);
				Print("Ask=", pa, " Bid=", pb, " sy=", sy, " ll=", ll, " op=", GetNameOP(op),
					  " pp=", pp, " sl=", sl, " tp=", tp, " mn=", mn);
			}
			
			// Áëîêèðîâêà ðàáîòû ñîâåòíèêà
			
			if (err == 2 || err == 64 || err == 65 || err == 133) {
				gbDisabled = true;
				break;
			}
			
			// Äëèòåëüíàÿ ïàóçà
			
			if (err == 4 || err == 131 || err == 132) {
				Sleep(1000*300);
				break;
			}
			
			if (err == 128 || err == 142 || err == 143) {
				Sleep(1000*66.666);
				
				if (ExistPositions(sy, op, mn, ot)) {
					if (UseSound)
						PlaySound(NameFileSound);
						
					break;
				}
			}
			
			if (err == 140 || err == 148 || err == 4110 || err == 4111)
				break;
				
			if (err == 141)
				Sleep(1000*100);
				
			if (err == 145)
				Sleep(1000*17);
				
			if (err == 146)
				while (IsTradeContextBusy())
					Sleep(1000*11);
					
			if (err != 135)
				Sleep(1000*7.7);
		}
	}
	
	if (MarketWatch && ticket > 0 && (sl > 0 || tp > 0)) {
		if (OrderSelect(ticket, SELECT_BY_TICKET))
			ModifyOrder(-1, sl, tp);
	}
}

}

#endif
