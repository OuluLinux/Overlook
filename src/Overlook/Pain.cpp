#include "Overlook.h"

#if 0

// comfort scalping

namespace Overlook {
	
Pain::Pain() {
	
}

void Pain::InitEA() {
	FatalError = False;
// - 1 - == Ñáîð èíôîðìàöèè îá óñëîâèÿõ òîðãîâëè ========================================
	Tick = MarketInfo(Symbol(), MODE_TICKSIZE);                         // ìèíèìàëüíûé òèê
	Spread = ND(MarketInfo(Symbol(), MODE_SPREAD) * Point);               // òåêóùèé ñïðýä
	StopLevel = ND(MarketInfo(Symbol(), MODE_STOPLEVEL) * Point);  // òåêóùèé óðîâåíü ñòîïîâ
	FreezeLevel = ND(MarketInfo(Symbol(), MODE_FREEZELEVEL) * Point); // óðîâåíü çàìîðîçêè
	MinLot = MarketInfo(Symbol(), MODE_MINLOT);    // ìèíèìàëüíûé ðàçðåøåííûé îáúåì ñäåëêè
	MaxLot = MarketInfo(Symbol(), MODE_MAXLOT);   // ìàêñèìàëüíûé ðàçðåøåííûé îáúåì ñäåëêè
	LotStep = MarketInfo(Symbol(), MODE_LOTSTEP);          // øàã ïðèðàùåíèÿ îáúåìà ñäåëêè
// - 1 - == Îêîí÷àíèå áëîêà =============================================================

// - 2 - == Ïðèâåäåíèå îáúåìà ñäåëêè ê äîïóñòèìîìó è ïðîâåðêà êîððåêòíîñòè îáúåìà =======
	Lots = LotRound(Lots);                  // îêðóãëåíèå îáúåìà äî áëèæàéøåãî äîïóñòèìîãî
// - 2 - == Îêîí÷àíèå áëîêà =============================================================

// - 3 - ==================== Ïðîâåðêà êîððåêòíîñòè âõîäíûõ ïàðàìåòðîâ ==================

	if (FastPeriod < 1) {
		Comment("Çíà÷åíèå FastPeriod äîëæíî áûòü ïîëîæèòåëüíûì. Ñîâåòíèê îòêëþ÷åí!");
		Print("Çíà÷åíèå FastPeriod äîëæíî áûòü ïîëîæèòåëüíûì. Ñîâåòíèê îòêëþ÷åí!");
		return(0);
	}
	
	if (SlowPeriod < 1) {
		Comment("Çíà÷åíèå FastPeriod äîëæíî áûòü ïîëîæèòåëüíûì. Ñîâåòíèê îòêëþ÷åí!");
		Print("Çíà÷åíèå FastPeriod äîëæíî áûòü ïîëîæèòåëüíûì. Ñîâåòíèê îòêëþ÷åí!");
		return(0);
	}
	
	if (MAMethod < 0 || MAMethod > 3) {
		Comment("Çíà÷åíèå MAMethod äîëæíî áûòü îò 0 äî 3. Ñîâåòíèê îòêëþ÷åí!");
		Print("Çíà÷åíèå MAMethod äîëæíî áûòü îò 0 äî 3. Ñîâåòíèê îòêëþ÷åí!");
		return(0);
	}
	
	if (MAPrice < 0 || MAPrice > 6) {
		Comment("Çíà÷åíèå MAPrice äîëæíî áûòü îò 0 äî 6. Ñîâåòíèê îòêëþ÷åí!");
		Print("Çíà÷åíèå MAPrice äîëæíî áûòü îò 0 äî 6. Ñîâåòíèê îòêëþ÷åí!");
		return(0);
	}
	
	if (FlatDuration < 2) {
		Comment("Çíà÷åíèå FlatDuration äîëæíî áûòü 2 è áîëåå. Ñîâåòíèê îòêëþ÷åí!");
		Print("Çíà÷åíèå FlatDuration äîëæíî áûòü 2 è áîëåå. Ñîâåòíèê îòêëþ÷åí!");
		return(0);
	}
	
	if (StopMistake <= 0) {
		Comment("Çíà÷åíèå StopMistake äîëæíî áûòü ïîëîæèòåëüíûì. Ñîâåòíèê îòêëþ÷åí!");
		Print("Çíà÷åíèå StopMistake äîëæíî áûòü ïîëîæèòåëüíûì. Ñîâåòíèê îòêëþ÷åí!");
		return(0);
	}
	
// - 3 - =========================== Îêîí÷àíèå áëîêà ====================================

	Activate = True; // Âñå ïðîâåðêè óñïåøíî çàâåðøåíû, âîçâîäèì ôëàã àêòèâèçàöèè ýêñïåðòà
	
	return(0);
}

//+-------------------------------------------------------------------------------------+
//| Ôóíêöèÿ start ýêñïåðòà                                                              |
//+-------------------------------------------------------------------------------------+
void Pain::StartEA(int pos) {
// - 1 - ========================== Ìîæíî ëè ðàáîòàòü ýêñïåðòó? =========================
	if (!Activate || FatalError)
		return(0);
		
// - 1 - ============================= Îêîí÷àíèå áëîêà ==================================

// - 2 - ========================== Êîíòðîëü îòêðûòèÿ íîâîãî áàðà =======================
	if (LastBar == Time[0])                         // Åñëè íà òåêóùåì áàðå óæå áûëè..
		return(0);                                   // ..ïðîèçâåäåíû íåîáõîäèìûå äåéñòâèÿ,
		
	// ..òî ïðåðûâàåì ðàáîòó äî..
	// ..ñëåäóþùåãî òèêà
// - 2 - ============================= Îêîí÷àíèå áëîêà ==================================

// - 3 - ====================== Îáíîâëåíèå èíôîðìàöèè î òîðãîâûõ óñëîâèÿõ ===============
	if (!IsTesting()) {
		Tick = MarketInfo(Symbol(), MODE_TICKSIZE);  // ìèíèìàëüíûé òèê
		Spread = ND(MarketInfo(Symbol(), MODE_SPREAD) * Point);// òåêóùèé ñïðýä
		StopLevel = ND(MarketInfo(Symbol(), MODE_STOPLEVEL) * Point);//òåêóùèé óðîâåíü ñòîïîâ
		FreezeLevel = ND(MarketInfo(Symbol(), MODE_FREEZELEVEL) * Point);// óðîâåíü çàìîðîçêè
	}
	
// - 3 - ============================= Îêîí÷àíèå áëîêà ==================================

// - 4 - ========================== Ðàñ÷åò ñèãíàëîâ îòêðûòèÿ è çàêðûòèÿ =================

	if (LastSignal != Time[0])                      // Íà òåêóùåì áàðå åùå íå áûë..
	{                                               // ..ïðîèçâåäåí ïîèñê ôëýòà
		FlatDetect();                                // Îïðåäåëÿåì, åñòü ëè ôëýò
		LastSignal = Time[0];                        // Îïðåäåëåíèå ôëýòà íà òåêóùåì áàðå..
	}                                               // ..ïðîèçâåäåíî
	
// - 4 - ============================= Îêîí÷àíèå áëîêà ==================================

// - 5 - ================== Âûïîëíåíèå îïåðàöèé ïðè îáíàðóæåíèè íîâîãî áîêñà ============

	if (Signal && !FindOrders())                    // Åñëè åñòü ñèãíàë è íåò ñâîåé ñäåëêè
		if (!Trade())
			return(0);                     // Îòêðûòèå ñäåëêè
			
// - 5 - ============================= Îêîí÷àíèå áëîêà ==================================

	LastBar = Time[0];
	
	return(0);
}


double Pain::LotRound(double L) {
	return(MathRound(MathMin(MathMax(L, MinLot), MaxLot) / LotStep)*LotStep);
}

double Pain::ND(double A) {
	return(NormalizeDouble(A, Digits));
}

bool Pain::WaitForTradeContext() {
	int P = 0;
// öèêë "ïîêà"

	while (IsTradeContextBusy() && P < 5) {
		P++;
		Sleep(1000);
	}
	
// -------------

	if (P == 5)
		return(False);
		
	return(True);
}

int Pain::OpenOrderCorrect(int Type, double Lot, double Price, double SL, double TP, bool Redefinition)
// Redefinition - ïðè True äîîïðåäåëÿòü ïàðàìåòðû äî ìèíèìàëüíî äîïóñòèìûõ
//                ïðè False - âîçâðàùàòü îøèáêó
{
// - 1 - == Ïðîâåðêà äîñòàòî÷íîñòè ñâîáîäíûõ ñðåäñòâ ====================================
	if (AccountFreeMarginCheck(Symbol(), OP_BUY, Lot) <= 0 || GetLastError() == 134) {
		if (!FreeMarginAlert) {
			Print("Íåäîñòàòî÷íî ñðåäñòâ äëÿ îòêðûòèÿ ïîçèöèè. Free Margin = ",
				  AccountFreeMargin());
			FreeMarginAlert = True;
		}
		
		return(5);
	}
	
	FreeMarginAlert = False;
	
// - 1 - == Îêîí÷àíèå áëîêà =============================================================

// - 2 - == Êîððåêòèðîâêà çíà÷åíèé Price, SL è TP èëè âîçâðàò îøèáêè ====================
	RefreshRates();
	
	switch (Type) {
	
	case OP_BUY:
		string S = "BUY";
		
		if (MathAbs(Price - Ask) / Point > 3)
			if (Redefinition)
				Price = ND(Ask);
			else
				return(2);
				
		if (ND(TP -Bid) <= StopLevel && TP != 0)
			if (Redefinition)
				TP = ND(Bid + StopLevel + Tick);
			else
				return(4);
				
		if (ND(Bid -SL) <= StopLevel)
			if (Redefinition)
				SL = ND(Bid - StopLevel - Tick);
			else
				return(3);
				
		break;
		
	case OP_SELL:
		S = "SELL";
		
		if (MathAbs(Price - Bid) / Point > 3)
			if (Redefinition)
				Price = ND(Bid);
			else
				return(2);
				
		if (ND(Ask -TP) <= StopLevel)
			if (Redefinition)
				TP = ND(Ask - StopLevel - Tick);
			else
				return(4);
				
		if (ND(SL -Ask) <= StopLevel && SL != 0)
			if (Redefinition)
				SL = ND(Ask + StopLevel + Tick);
			else
				return(3);
				
		break;
		
	case OP_BUYSTOP:
		S = "BUYSTOP";
		
		if (ND(Price - Ask) <= StopLevel)
			if (Redefinition)
				Price = ND(Ask + StopLevel + Tick);
			else
				return(2);
				
		if (ND(TP -Price) <= StopLevel && TP != 0)
			if (Redefinition)
				TP = ND(Price + StopLevel + Tick);
			else
				return(4);
				
		if (ND(Price -SL) <= StopLevel)
			if (Redefinition)
				SL = ND(Price - StopLevel - Tick);
			else
				return(3);
				
		break;
		
	case OP_SELLSTOP:
		S = "SELLSTOP";
		
		if (ND(Bid - Price) <= StopLevel)
			if (Redefinition)
				Price = ND(Bid - StopLevel - Tick);
			else
				return(2);
				
		if (ND(Price -TP) <= StopLevel)
			if (Redefinition)
				TP = ND(Price - StopLevel - Tick);
			else
				return(4);
				
		if (ND(SL -Price) <= StopLevel && SL != 0)
			if (Redefinition)
				SL = ND(Price + StopLevel + Tick);
			else
				return(3);
				
		break;
		
	case OP_BUYLIMIT:
		S = "BUYLIMIT";
		
		if (ND(Ask - Price) <= StopLevel)
			if (Redefinition)
				Price = ND(Ask - StopLevel - Tick);
			else
				return(2);
				
		if (ND(TP -Price) <= StopLevel && TP != 0)
			if (Redefinition)
				TP = ND(Price + StopLevel + Tick);
			else
				return(4);
				
		if (ND(Price -SL) <= StopLevel)
			if (Redefinition)
				SL = ND(Price - StopLevel - Tick);
			else
				return(3);
				
		break;
		
	case OP_SELLLIMIT:
		S = "SELLLIMIT";
		
		if (ND(Price - Bid) <= StopLevel)
			if (Redefinition)
				Price = ND(Bid + StopLevel + Tick);
			else
				return(2);
				
		if (ND(Price -TP) <= StopLevel)
			if (Redefinition)
				TP = ND(Price - StopLevel - Tick);
			else
				return(4);
				
		if (ND(SL -Price) <= StopLevel && SL != 0)
			if (Redefinition)
				SL = ND(Price + StopLevel + Tick);
			else
				return(3);
				
		break;
	}
	
// - 2 - == Îêîí÷àíèå áëîêà =============================================================

// - 3 - == Îòêðûòèå îðäåðà ñ îæèäàíèå òîðãîâîãî ïîòîêà =================================

	if (WaitForTradeContext()) { // îæèäàíèå îñâîáîæäåíèÿ òîðãîâîãî ïîòîêà
		Comment("Îòïðàâëåí çàïðîñ íà îòêðûòèå îðäåðà ", S, " ...");
		int ticket = OrderSend(Symbol(), Type, Lot, Price, 3,
					 SL, TP, NULL, MagicNumber, 0);// îòêðûòèå ïîçèöèè
		// Ïîïûòêà îòêðûòèÿ ïîçèöèè çàâåðøèëàñü íåóäà÷åé
		
		if (ticket < 0) {
			int Error = GetLastError();
			
			if (Error == 2 || Error == 5 || Error == 6 || Error == 64
				|| Error == 132 || Error == 133 || Error == 149) {   // ñïèñîê ôàòàëüíûõ îøèáîê
				Comment("Ôàòàëüíàÿ îøèáêà ïðè îòêðûòèè ïîçèöèè ò. ê. " +
						ErrorToString(Error) + " Ñîâåòíèê îòêëþ÷åí!");
				FatalError = True;
			}
			
			else
				Comment("Îøèáêà îòêðûòèÿ ïîçèöèè ", S, ": ", Error);       // íåôàòàëüíàÿ îøèáêà
				
			return(1);
		}
		
		// ---------------------------------------------
		
		// Óäà÷íîå îòêðûòèå ïîçèöèè
		Comment("Ïîçèöèÿ ", S, " îòêðûòà óñïåøíî!");
		
		PlaySound(OpenOrderSound);
		
		return(0);
		
		// ------------------------
	}
	
	else {
		Comment("Âðåìÿ îæèäàíèÿ îñâîáîæäåíèÿ òîðãîâîãî ïîòîêà èñòåêëî!");
		return(1);
	}
	
// - 3 - == Îêîí÷àíèå áëîêà =============================================================
}

//+-------------------------------------------------------------------------------------+
//| Ïðèâåäåíèå çíà÷åíèé ê òî÷íîñòè îäíîãî òèêà                                          |
//+-------------------------------------------------------------------------------------+
double Pain::NP(double A) {
	return(MathRound(A / Tick)*Tick);
}

//+-------------------------------------------------------------------------------------+
//| Îïðåäåëåíèå ôëýòîâîé ñîñòàâëÿþùåé ðûíêà                                             |
//+-------------------------------------------------------------------------------------+
void Pain::FlatDetect() {
// - 1 - ========================= Âû÷èñëåíèå ðàçíîñòè ñðåäíèõ ==========================
	int i = 1;
	
	while (true)                                    // Öèêë âûïîëíÿåòñÿ, ïîêà..
	{                                               // ..ðåãèñòðèðóåòñÿ ôëýò
		double MAFast = iMA(NULL, 0, FastPeriod, 0, MAMethod, MAPrice, i);// Ìåäëåííàÿ..
		// ..ñðåäíÿÿ
		double MASlow = iMA(NULL, 0, SlowPeriod, 0, MAMethod, MAPrice, i);// Áûñòðàÿ..
		// ..ñðåäíÿÿ
		
		if (MathAbs(MAFast - MASlow) > FlatPoints*Point) // Åñëè ðàçíîñòü ñðåäíèõ áîëüøå..
			break;                                    // ..äîïóñòèìîãî ïðåäåëà, òî..
			
		// ..ðåãèñòðàöèÿ ôëýòà ïðåêðàùàåòñÿ
		i++;
	}
	
// - 1 - ================================ Îêîí÷àíèå áëîêà ===============================

// - 2 - =========================== Ãåíåðàöèÿ áîêñà ïîêóïêè ============================

	if (i >= FlatDuration + 1)                     // Åñëè ðàçíîñòü ñðåäíèõ íà áëèæàéøèõ..
	{                                              // ..FlatDuration áàðàõ áûëà â ïðåäåëàõ
		Signal = true;                              // ..äîïóñòèìîé âåëè÷èíû, òî..
		Maximum = High[iHighest(NULL, 0, MODE_HIGH, i-1, 1)];// ..ðåãèñòðèðóåòñÿ ôëýò ñ..
		Minimum = Low[iLowest(NULL, 0, MODE_LOW, i-1, 1)];// ..ðàñ÷åòîì åãî ãðàíèö
	}
	
	else                                           // Èíà÷å ôëýò íå ðåãèñòðèðóåòñÿ
		Signal = false;
		
// - 2 - ================================ Îêîí÷àíèå áëîêà ===============================
}

//+-------------------------------------------------------------------------------------+
//| Ôóíêöèÿ ïîèñêà ñâîèõ îðäåðîâ                                                        |
//+-------------------------------------------------------------------------------------+
bool Pain::FindOrders() {
// - 1 - ====================== Èíèöèàëèçàöèÿ ïåðåìåííûõ ïåðåä ïîèñêîì ==================
	int total = OrdersTotal() - 1;
// - 1 - ================================== Îêîí÷àíèå áëîêà =============================

// - 2 - ================================ Ïðîèçâåäåíèå ïîèñêà ===========================

	for (int i = total; i >= 0; i--)                // Èñïîëüçóåòñÿ âåñü ñïèñîê îðäåðîâ
		if (OrderSelect(i, SELECT_BY_POS))           // Óáåäèìñÿ, ÷òî îðäåð âûáðàí
			if (OrderMagicNumber() == MagicNumber &&  // Îðäåð îòêðûò ýêñïåðòîì,
				OrderSymbol() == Symbol())            // ..êîòîðûé ïðèêðåïëåí ê òåêóùåé..
				return(true);                          // Âåðíåì - îðäåð ñóùåñòâóåò
				
// - 2 - ================================== Îêîí÷àíèå áëîêà =============================
	return(false);                                  // Âåðíåì - îðäåð íå ñóùåñòâóåò
}

//+-------------------------------------------------------------------------------------+
//| Îòêðûòèå ïîçèöèé                                                                    |
//+-------------------------------------------------------------------------------------+
bool Pain::Trade() {
// - 1 - ========================= Ïîäãîòîâêà èñõîäíûõ çíà÷åíèé =========================
	double average = (Maximum + Minimum) / 2;       // Óðîâåíü ñåðåäèíû êàíàëà
	double bid = MarketInfo(Symbol(), MODE_BID);    // Òåêóùàÿ öåíà Bid
	double ask = MarketInfo(Symbol(), MODE_ASK);    // Òåêóùàÿ öåíà Ask
	double slmistake = (Maximum - Minimum) * StopMistake / 100; // Çàïàñ äëÿ ñòîïà
	double tpmistake = (Maximum - Minimum) * TakeProfitMistake / 100; // Çàïàñ äëÿ ïðîôèòà
	int type = -1;                                  // Òèï ñäåëêè íå îïðåäåëåí
// - 1 - ================================== Îêîí÷àíèå áëîêà =============================

// - 2 - ======================= Ïîäãîòîâêà äàííûõ äëÿ îòêðûòèÿ äëèííîé =================

	if (bid < average) {                            // Òåêóùàÿ öåíà â íèæíåé ÷àñòè êàíàëà
		type = OP_BUY;                               // Ïîýòîìó òèï ñäåëêè - Buy
		double price = NP(ask);                      // Öåíà îòêðûòèÿ ñäåëêè - Ask
		double sl = NP(Minimum - slmistake);         // Ñòîï-ïðèêàç - çà ìèíèìóìîì êàíàëà
		double tp = NP(Maximum + tpmistake);         // Ïðîôèò - íèæå ìàêñèìóìà èëè âûøå íà
		// ..TakeProfitMistake ïðîöåíòîâ
		
		if (tp - price <= StopLevel)                 // Åñëè ðàçìåð ïðîôèòà ñëèøêîì ìàë, òî
			type = -1;                                // ..ñäåëêà ñîâåðøåíà íå áóäåò
	}
	
// - 2 - ============================= Îêîí÷àíèå áëîêà ==================================

// - 3 - ====================== Ïîäãîòîâêà äàííûõ äëÿ îòêðûòèÿ êîðîòêîé =================

	if (bid > average) {                            // Òåêóùàÿ öåíà â âåðõíåé ÷àñòè êàíàëà
		type = OP_SELL;                              // Ïîýòîìó òèï ñäåëêè - Sell
		price = NP(bid);                             // Öåíà îòêðûòèÿ ñäåëêè - Bid
		sl = NP(Maximum + Spread + slmistake);       // Ñòîï-ïðèêàç - çà ìàêñèìóìîì êàíàëà
		tp = NP(Minimum + Spread - tpmistake);       // Ïðîôèò - âûøå èëè íèæå ìèíèìóìà íà
		// ..TakeProfitMistake ïðîöåíòîâ
		
		if (price - tp <= StopLevel)                 // Åñëè ðàçìåð ïðîôèòà ñëèøêîì ìàë, òî
			type = -1;                                // ..ñäåëêà ñîâåðøåíà íå áóäåò
	}
	
// - 3 - ============================= Îêîí÷àíèå áëîêà ==================================

// - 4 - ============================ Ñîâåðøåíèå ñäåëêè =================================

	if (type >= 0)
		if (OpenOrderCorrect(type, Lots, price, sl, tp) != 0) // Åñëè ñäåëêà íå áûëà..
			return(false);                            // ..ñîâåðøåíà, òî âåðíåì îøèáêó
			
// - 4 - ============================= Îêîí÷àíèå áëîêà ==================================

	return(True);                                   // Âñå îïåðàöèè çàâåðøåíû
}

}

#endif
