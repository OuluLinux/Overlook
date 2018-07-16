#include "Overlook.h"


namespace Overlook {

NetPressureSolver::NetPressureSolver() {
	
	currencies.Add("AUD");
	currencies.Add("EUR");
	currencies.Add("USD");
	currencies.Add("CHF");
	currencies.Add("JPY");
	currencies.Add("NZD");
	currencies.Add("GBP");
	currencies.Add("CAD");
	
	// These arbitrages are not avoided generally by brokers
	/*currencies.Add("SGD");
	currencies.Add("NOK");
	currencies.Add("SEK");
	currencies.Add("DKK");
	currencies.Add("ZAR");
	currencies.Add("MXN");
	currencies.Add("HKD");
	currencies.Add("HUF");
	currencies.Add("CZK");
	currencies.Add("PLN");
	currencies.Add("RUR");
	currencies.Add("TRY");*/
	
	spreads.GetAdd("EURUSD", 2);
	spreads.GetAdd("GBPUSD", 3);
	spreads.GetAdd("USDCHF", 3);
	spreads.GetAdd("USDJPY", 2);
	spreads.GetAdd("USDCAD", 3);
	spreads.GetAdd("AUDUSD", 3);
	spreads.GetAdd("NZDUSD", 4);
	spreads.GetAdd("EURAUD", 10);
	spreads.GetAdd("EURCHF", 3);
	spreads.GetAdd("EURGBP", 3);
	spreads.GetAdd("USDHKD", 14);
	spreads.GetAdd("GBPCHF", 6);
	spreads.GetAdd("NZDJPY", 8);
	spreads.GetAdd("NZDCHF", 10);
	spreads.GetAdd("NZDCAD", 10);
	spreads.GetAdd("USDNOK", 75);
	spreads.GetAdd("GBPSEK", 200);
	spreads.GetAdd("GBPNZD", 30);
	spreads.GetAdd("GBPNOK", 200);
	spreads.GetAdd("GBPJPY", 6);
	spreads.GetAdd("AUDCAD", 8);
	spreads.GetAdd("GBPCAD", 10);
	spreads.GetAdd("GBPAUD", 10);
	spreads.GetAdd("AUDCHF", 6);
	spreads.GetAdd("EURSEK", 100);
	spreads.GetAdd("EURNZD", 20);
	spreads.GetAdd("EURNOK", 100);
	spreads.GetAdd("EURJPY", 3);
	spreads.GetAdd("EURHKD", 45);
	spreads.GetAdd("EURDKK", 50);
	spreads.GetAdd("EURCAD", 8);
	spreads.GetAdd("CHFJPY", 6);
	spreads.GetAdd("CADJPY", 6);
	spreads.GetAdd("CADCHF", 8);
	spreads.GetAdd("AUDNZD", 20);
	spreads.GetAdd("AUDJPY", 6);
	spreads.GetAdd("USDSEK", 100);
	spreads.GetAdd("EURSGD", 20);
	spreads.GetAdd("GBPSGD", 30);
	spreads.GetAdd("NZDSGD", 30);
	spreads.GetAdd("SGDJPY", 16);
	spreads.GetAdd("USDMXN", 150);
	spreads.GetAdd("USDSGD", 7);
	spreads.GetAdd("USDZAR", 200);
	spreads.GetAdd("AUDSEK", 150);
	spreads.GetAdd("AUDSGD", 20);
	spreads.GetAdd("CHFSGD", 30);
	spreads.GetAdd("EURTRY", 100);
	spreads.GetAdd("HKDJPY", 20);
	spreads.GetAdd("USDPLN", 100);
	spreads.GetAdd("USDRUB", 2000);
	spreads.GetAdd("USDTRY", 100);
}

void NetPressureSolver::Init() {
	System& sys = GetSystem();
	
	
	pressures.SetCount(sys.GetSymbolCount(), 0);
	solved_pressures.SetCount(sys.GetSymbolCount(), 0);
	ask.SetCount(sys.GetSymbolCount(), 0);
	solved_ask.SetCount(sys.GetSymbolCount(), 0);
	point.SetCount(sys.GetSymbolCount(), 0);
	ASSERT(!pressures.IsEmpty());
	
	
	GetRealSymbols();
	GetAllSymbols();
	GetPipsD();
	
	
	max_arbitrage_count = 0;
	
	for (int i = 0; i < real_sym_count; i++)
		position[i] = 0;
		
	
	PrintBeginInfo();
	
	
}

void NetPressureSolver::GetRealSymbols() {
	real_sym_count = 0;
	
	MetaTrader& mt = GetMetaTrader();
	const Vector<libmt::Symbol>& symbols = mt.GetSymbols();
	for(int i = 0; i < symbols.GetCount(); i++) {
		const libmt::Symbol& sym = symbols[i];
		
		point[i] = sym.point;
		
		real_symbols.Add(sym.name);
		real_sym_count++;
	}
}

void NetPressureSolver::GetRealBidAsk() {
	MetaTrader& mt = GetMetaTrader();
	const Vector<Price>& prices = mt.GetAskBid();
	const Vector<libmt::Symbol>& symbols = mt.GetSymbols();
	
	for (int i = 0; i < prices.GetCount(); i++) {
		const Price& p = prices[i];
		const libmt::Symbol& sym = symbols[i];
		
		
		double ask;
		if (use_mtask)
			this->ask[i] = p.ask;
		ask = this->ask[i];
		
		double pressure = (pressures[i] + solved_pressures[i]) * sym.point;
		ask += pressure;
		
		asks_real[i] = ask;
		//bids_real[i] = p.bid;
		
		int j = spreads.Find(sym.name);
		int spread_point = spreads[j];
		bids_real[i] = ask - sym.point * spread_point;
	}
}

bool NetPressureSolver::IsRealSymbol(const String& s) {
	return GetSystem().FindSymbol(s) != -1;
}

void NetPressureSolver::GetAllSymbols() {
	String pair, pair1[4], pair2[4];
	
	//symbols.GetCount() = 0;
	symbols.SetCount(0);
	
	for (int i = 0; i < currencies.GetCount(); i++) {
		for (int j = 0; j < currencies.GetCount(); j++) {
			if (i != j) {
				Symbol s;
				
				pair = currencies[i] + currencies[j];
				
				if (IsRealSymbol(pair)) {
					VariantSym& vs = s.sym.Add();
					vs.all_sym = GetNumIsRealSymbol(pair);
					vs.math = -1; // -2 - "1 / S"; -1 - "S"; 0 - "S1 / S2"; 1 - "S1 * S2"; 2 - "1 / (S1 * S2)"; 3 - "S2 / S1"
				}
				
				pair = currencies[j] + currencies[i];
				
				if (IsRealSymbol(pair)) {
					VariantSym& vs = s.sym.Add();
					vs.all_sym = GetNumIsRealSymbol(pair);
					vs.math = -2; // -2 - "1 / S"; -1 - "S"; 0 - "S1 / S2"; 1 - "S1 * S2"; 2 - "1 / (S1 * S2)"; 3 - "S2 / S1"
				}
				
				for (int k = 0; k < currencies.GetCount(); k++) {
					if ((k != i) && (k != j)) {
						
						pair1[0] = currencies[i] + currencies[k];
						pair1[1] = currencies[i] + currencies[k];
						pair1[2] = currencies[k] + currencies[i];
						pair1[3] = currencies[k] + currencies[i];
						
						pair2[0] = currencies[j] + currencies[k];
						pair2[1] = currencies[k] + currencies[j];
						pair2[2] = currencies[j] + currencies[k];
						pair2[3] = currencies[k] + currencies[j];
						
						for (int m = 0; m < 4; m++) {
							if (IsRealSymbol(pair1[m]) && IsRealSymbol(pair2[m])) {
								VariantSym& vs = s.sym.Add();
								vs.all_sym = GetNumIsRealSymbol(pair1[m]) * real_sym_count + GetNumIsRealSymbol(pair2[m]);
								vs.math = m; // 0 - "S1 / S2"; 1 - "S1 * S2"; 2 - "1 / (S1 * S2)"; 3 - "S2 / S1"
							}
						}
					}
				}
				
				if (s.sym.GetCount() >= 2) {
					s.pair.SetCount(s.sym.GetCount() * s.sym.GetCount());
					symbols.Add(s);
				}
			}
		}
	}
}

void NetPressureSolver::GetPipsD() {
	GetRealBidAsk();
	
	for (int i = 0; i < symbols.GetCount(); i++) {
		Symbol& s = symbols[i];
		
		GetBidAsk(i, 0);
		
		if (s.sym[0].bid > 70) {
			s.point = 0.01;
			s.min_pips = min_pips * 0.01;
		}
		
		else {
			s.point = 0.0001;
			s.min_pips = min_pips * 0.0001;
		}
	}
}

String NetPressureSolver::SymbolToStr(int i, int j) {
	VariantSym& s = symbols[i].sym[j];
	
	String str = "", S1, S2;
	
	if (s.math == -1)
		str = real_symbols[s.all_sym];
	else if (s.math == -2)
		str = "1 / " + real_symbols[s.all_sym];
	else
	{
		S1 = real_symbols[s.all_sym / real_sym_count];
		S2 = real_symbols[s.all_sym % real_sym_count];
		
		switch (s.math)
		{
		case 0: // 0 - "S1 / S2"
			str = S1 + " / " + S2;
			break;
		case 1: // 1 - "S1 * S2"
			str = S1 + " * " + S2;
			break;
		case 2: // 2 - "1 / (S1 * S2)";
			str = "1 / (" + S1 + " * " + S2 + ")";
			break;
		case 3: // 3 - "S2 / S1"
			str = S2 + " / " + S1;
			break;
		}
	}
	
	return str;
}

void NetPressureSolver::RefreshPositions() {
	bool Flag = false;
	double vol;
	
	for (int i = 0; i < real_sym_count; i++) {
		//position[i] = NormalizeDouble(position[i], DigitsLot);
		//position[i] = ((int)(position[i] / 0.01)) * 0.01;
		
		if (position[i] > ALPHA) {
			//vol = position[i];
			
			//GetSystem().SetSignal(i, position[i] / 0.01 / 10000 * SIGNALSCALE);
			/*if (!MyOrderSend(REALSymbols[i], OP_BUY, Vol, SlipPage, MaxLot, Lock))
				Print("Vol = ", Vol);
				
			position[i] = Vol;*/
			
			Flag = true;
		}
		
		else if (position[i] < -ALPHA) {
			//vol = -position[i];
			
			//GetSystem().SetSignal(i, position[i] / 0.01 / 10000 * SIGNALSCALE);
			/*if (!MyOrderSend(REALSymbols[i], OP_SELL, Vol, SlipPage, MaxLot, Lock))
				Print("Vol = ", Vol);
				
			position[i] = -Vol;*/
			
			Flag = true;
		}
	}
	
	//if (Flag) LOG(ArbitragePositions());
		
	//CloseLock();
}

String NetPressureSolver::ArbitragePositions() {
	String str = "Arbitrage: min_pips = " + DblStr(min_pips);
	
	for (int i = 0; i < symbols.GetCount(); i++) {
		Symbol& s = symbols[i];
		int pos = 0;
		
		for (int j = 0; j < s.sym.GetCount() - 1; j++) {
			pos += j + 1;
			VariantPair& p = s.pair[pos];
			
			for (int k = j + 1; k < s.sym.GetCount() ; k++) {
				if (p.xtrade) {
					if (p.xposition < -ALPHA)
						str += "\n" + SymbolToStr(i, j) + " (SELL) && " + SymbolToStr(i, k) + " (BUY)";
					else if (p.xposition > ALPHA)
						str += "\n" + SymbolToStr(i, j) + " (BUY) && " + SymbolToStr(i, k) + " (SELL)";
				}
				
				pos++;
			}
		}
	}
	
	return str;
}


void NetPressureSolver::MonitoringArbitrage(int sym_id, int variant1_id, int variant2_id) {
	int V;
	String str;
	Symbol& s = symbols[sym_id];
	
	if (variant1_id < variant2_id)
		V = variant1_id * s.sym.GetCount() + variant2_id;
	else
		V = variant2_id * s.sym.GetCount() + variant1_id;
	
	VariantPair& vp = s.pair[V];
	VariantSym& v1 = s.sym[variant1_id];
	VariantSym& v2 = s.sym[variant2_id];
	vp.arbitrage_count++;
	
	if (vp.arbitrage_count > max_arbitrage_count)
		max_arbitrage_count = vp.arbitrage_count;
		
	/*if (true) {
		// E.g. Bid "GBPSEK / AUDSEK" (1.8346142314) > (1.7611442434) Ask "GBPCAD / AUDCAD", Difference = 734.6998797784 pips
		str = "Time = " + Format("%", GetSysTime()) + "\n";
		str = str + "Bid \"" + SymbolToStr(sym_id, variant1_id) + "\" (" + DblStr(v1.bid) +
			  ") > (" + DblStr(v2.ask) + ") Ask \"" + SymbolToStr(sym_id, variant2_id) +
			  "\", Difference = " + DblStr((v1.bid - v2.ask) / s.point) + " pips";
		str = str + "\n" + SymbolToFile(sym_id, variant1_id) + "\n" + SymbolToFile(sym_id, variant2_id) + "\n";
		str = str + "Count = " + IntStr(vp.arbitrage_count);
		
		LOG(str);
		//stringToFile("Arbitrage.txt", str);
	}*/
}

String NetPressureSolver::SymbolToFile( int i, int j )
{
	String str = "";
	int S1, S2;
	Symbol& s = symbols[i];
	VariantSym& vs = s.sym[j];
	
	if (vs.math < 0)
	{
		S1 = vs.all_sym;
		
		str = real_symbols[S1] + ": " + DblStr(bids_real[S1]) + " " +
		DblStr(asks_real[S1]);
	}
	else
	{
		S1 = vs.all_sym / real_sym_count;
		S2 = vs.all_sym % real_sym_count;
		str = real_symbols[S1] + ": " + DblStr(bids_real[S1]) + " " + DblStr(asks_real[S1]) + "\n" +
			  real_symbols[S2] + ": " + DblStr(bids_real[S2]) + " " +
			  DblStr(asks_real[S2]);
	}
	
	return str;
}

void NetPressureSolver::OpenSymbolPosition(int sym_id, int V, int Type, double Vol) {
	int S1, S2;
	double tmp = 0, tmp1 = 0, tmp2 = 0;
	Symbol& s = symbols[sym_id];
	VariantSym& vs = s.sym[V];
	int symb = vs.all_sym;
	int mth = vs.math;
	
	if (Type == OP_SELL) {
		if (mth == -2) // -2 - "1 / S"
			tmp = Vol / asks_real[symb];
		else if (mth == -1)  //  -1 - "S"
			tmp = -Vol;
		else {
			S1 = symb / real_sym_count;
			S2 = symb % real_sym_count;
			
			switch (mth) {
			
			case 0: // 0 - "S1 / S2"
				tmp1 = -Vol;
				tmp2 = Vol * vs.bid;
				break;
				
			case 1: // 1 - "S1 * S2"
				tmp1 = -Vol;
				tmp2 = -Vol * bids_real[S1];
				break;
				
			case 2: // 2 - "1 / (S1 * S2)";
				tmp1 = Vol / asks_real[S1];
				tmp2 = Vol * vs.bid;
				break;
				
			case 3: // 3 - "S2 / S1"
				tmp1 = Vol / asks_real[S1];
				tmp2 = -tmp1;
				break;
			}
		}
	}
	
	else {
		if (mth == -2) // -2 - "1 / S"
			tmp = -Vol / bids_real[symb];
		else
			if (mth == -1)  //  -1 - "S"
				tmp = Vol;
			else {
				S1 = symb / real_sym_count;
				S2 = symb % real_sym_count;
				
				switch (mth) {
				
				case 0: // 0 - "S1 / S2"
					tmp1 = Vol;
					tmp2 = -Vol * vs.ask;
					break;
					
				case 1: // 1 - "S1 * S2"
					tmp1 = Vol;
					tmp2 = Vol * asks_real[S1];
					break;
					
				case 2: // 2 - "1 / (S1 * S2)";
					tmp1 = -Vol / bids_real[S1];
					tmp2 = -Vol * vs.ask;
					break;
					
				case 3: // 3 - "S2 / S1"
					tmp1 = -Vol / bids_real[S1];
					tmp2 = -tmp1;
					break;
				}
			}
	}
	
	SymbolDone(tmp, symb);
	
	SymbolDone(tmp1, S1);
	SymbolDone(tmp2, S2);
}

void NetPressureSolver::SymbolDone(double Vol, int Symb) {
	if (Vol == 0)
		return;
		
	position[Symb] += Vol;
	
	/*
	if (Vol > 0)
		str_out = str_out + "; BUY " + real_sym[Symb] + "(" + DblStr(Vol, DigitsLot) + ") = " +
				 DblStr(asks_real[Symb], MarketInfo(REALSymbols[Symb], MODE_DIGITS)) + " Ask";
	else // Vol < 0
		str_out = str_out + "; SELL " + real_sym[Symb] + "(" + DblStr(Vol, DigitsLot) + ") = " +
				 DblStr(bids_real[Symb], MarketInfo(REALSymbols[Symb], MODE_DIGITS)) + " Bid";
	*/
}

void NetPressureSolver::GetBidAsk(int i, int j) {
	double Bid1, Bid2;
	double Ask1, Ask2;
	int mth, symb;
	int S1, S2;
	
	Symbol& s = symbols[i];
	VariantSym& vs = s.sym[j];
	symb = vs.all_sym;
	mth = vs.math;
	
	if (mth == -2) {
		// -2 - "1 / S"
		vs.bid = 1 / asks_real[symb];
		vs.ask = 1 / bids_real[symb];
	}
	
	else
	if (mth == -1) {
		//  -1 - "S"
		vs.bid = bids_real[symb];
		vs.ask = asks_real[symb];
	}
	
	else {
		S1 = symb / real_sym_count;
		S2 = symb % real_sym_count;
		
		Bid1 = bids_real[S1];
		Bid2 = bids_real[S2];
		Ask1 = asks_real[S1];
		Ask2 = asks_real[S2];
		
		switch (mth) {
		
		case 0: // 0 - "S1 / S2"
			vs.bid = Bid1 / Ask2;
			vs.ask = Ask1 / Bid2;
			break;
			
		case 1: // 1 - "S1 * S2"
			vs.bid = Bid1 * Bid2;
			vs.ask = Ask1 * Ask2;
			break;
			
		case 2: // 2 - "1 / (S1 * S2)";
			vs.bid = 1 / (Ask1 * Ask2);
			vs.ask = 1 / (Bid1 * Bid2);
			break;
			
		case 3: // 3 - "S2 / S1"
			vs.bid = Bid2 / Ask1;
			vs.ask = Ask2 / Bid1;
		}
	}
}

void NetPressureSolver::PrintBeginInfo() {
	
	LOG("MAX_CURRENCY = " << currencies.GetCount());
	LOG("MAX_REALSYMBOLS = " << real_sym_count);
	LOG("MAX_ALLSYMBOLS = " << symbols.GetCount());
	
	int max = 0;
	
	for (int i = 0; i < symbols.GetCount(); i++)
		if (symbols[i].sym.GetCount() > max)
			max = symbols[i].sym.GetCount();
			
	LOG("MAX_VARIANTSYMBOLS = " << max);
	
	max = 0;
	
	for (int i = 0; i < symbols.GetCount(); i++) {
		int count = symbols[i].sym.GetCount();
		if (count * (count - 1) > max)
			max = count * (count - 1);
	}
			
	LOG("MAX_VARIANTPAIRS = " << max);
	
	max = 0;
	
	for (int i = 0; i < symbols.GetCount(); i++)
		max += symbols[i].sym.GetCount();
		
	LOG("SumAllCounts = " << max);
	
	max = 0;
	
	for (int i = 0; i < symbols.GetCount(); i++) {
		int count = symbols[i].sym.GetCount();
		max += count * (count - 1);
	}
		
	LOG("SumAllVariants = " << max);
	
}

void NetPressureSolver::TradeArbitrage() {
	
	GetRealBidAsk();
	
	for (int i = 0; i < real_sym_count; i++)
		position[i] = 0;
	for (int i = 0; i < symbols.GetCount(); i++) {
		Symbol& s = symbols[i];
		for(int j = 0; j < s.pair.GetCount(); j++)
			s.pair[j].xposition = 0;
	}
		
	
	for (int i = 0; i < symbols.GetCount(); i++) {
		Symbol& s = symbols[i];
		int count = s.sym.GetCount();
		
		for (int j = 0; j < count; j++)
			GetBidAsk(i, j);
			
		for (int j = 0; j < count - 1; j++) {
			VariantSym& vs = s.sym[j];
			double bid1 = vs.bid - s.min_pips;
			double ask1 = vs.ask + s.min_pips;
			
			for (int k = j + 1; k < count; k++) {
				VariantSym& vs = s.sym[k];
				double bid2 = vs.bid;
				double ask2 = vs.ask;
				
				//LOG(i << "\t" << j << "\t" << k << "\t" << bid1 << "\t" << ask1 << "\t" << bid2 << "\t" << ask2 << "\t" << (bid1-ask2) << "\t" << (ask1-bid2));
				
				if (bid1 > ask2)
					OpenArbitragePosition(i, j, k, lots);
				else if (ask1 < bid2)
					OpenArbitragePosition(i, k, j, lots);
			}
		}
	}
	
	RefreshPositions();
	
	for(int i = 0; i < MAX_REALSYMBOLS; i++)
		prev_position[i] = position[i];
}

void NetPressureSolver::OpenArbitragePosition(int sym_id, int variant1_id, int variant2_id, double vol) {
	int V;
	double xpos;
	
	Symbol& s = symbols[sym_id];
	int count = s.sym.GetCount();
	
	if (variant1_id < variant2_id) {
		V = variant1_id * count + variant2_id;
		VariantPair& vp = s.pair[V];
		
		xpos = vp.xposition;
		
		//if (xpos < -ALPHA)
		//	return;
			
		xpos += vol;
		
		vp.xposition = -vol;
	}
	
	else {
		V = variant2_id * count + variant1_id;
		VariantPair& vp = s.pair[V];
		
		xpos = vp.xposition;
		
		//if (xpos > ALPHA)
		//	return;
			
		xpos = vol - xpos;
		
		vp.xposition = vol;
	}
	
	MonitoringArbitrage(sym_id, variant1_id, variant2_id);
	
	VariantPair& vp = s.pair[V];
	VariantSym& v1 = s.sym[variant1_id];
	VariantSym& v2 = s.sym[variant2_id];
	
	if (/*vp.xtrade*/true) {
		OpenSymbolPosition(sym_id, variant1_id, OP_SELL, xpos);
		OpenSymbolPosition(sym_id, variant2_id, OP_BUY, xpos);
		
		/*LOG("variant1_id = " + SymbolToStr(sym_id, variant1_id) + " (Bid = " + DblStr(v1.bid) +
			  "), variant2_id = " + SymbolToStr(sym_id, variant2_id) + " (Ask = " + DblStr(v2.ask) +
			  "), Difference = " + DblStr((v1.bid - v2.ask) / s.point) + " pips");
		LOG(str_out);*/
		
	}
}

void NetPressureSolver::RandomPressure() {
	for(int i = 0; i < solved_pressures.GetCount(); i++) {
		pressures[i] = (Randomf() * 2 -1) * scale;
	}
}

void NetPressureSolver::Solve() {
	TimeStop ts;
	for(int i = 0; i < solved_pressures.GetCount(); i++) {
		position[i] = 0;
		solved_pressures[i] = 0;
	}
	
	int i1 = -1, i0 = -1, a0, a1;
	
	int limit = 5000;
	for (int iter = 0; iter < limit; iter++) {
		TradeArbitrage();
		
		double max_arbitrage = -DBL_MAX;
		for(int i = 0; i < symbols.GetCount(); i++) {
			NetPressureSolver::Symbol& s = symbols[i];
			int sym_count = s.sym.GetCount();
			for(int j = 0; j < s.pair.GetCount(); j++) {
				NetPressureSolver::VariantPair& pair = s.pair[j];
				int variant1_id = j / sym_count;
				int variant2_id = j % sym_count;
				NetPressureSolver::VariantSym& v1 = s.sym[variant1_id];
				NetPressureSolver::VariantSym& v2 = s.sym[variant2_id];
				
				double diff1 = (v1.bid - v2.ask) / s.point;
				double diff2 = (v2.bid - v1.ask) / s.point;
				if (diff1 > max_arbitrage)
					max_arbitrage = diff1;
				if (diff2 > max_arbitrage)
					max_arbitrage = diff2;
			}
		}
		
		double max_signal = -DBL_MAX;
		int max_i = -1;
		for(int i = 0; i < solved_pressures.GetCount(); i++) {
			double d = fabs(position[i]);
			if (d > max_signal) {
				if (i0 == i && i1 == i && a0 * a1 < 0) {
					limit = min(limit, iter + 10);
					continue;
				}
				max_i = i;
				max_signal = d;
			}
		}
		
		if (max_arbitrage < 0.0 && max_signal < 15.0 || max_i == -1)
			break;
		
		double d = position[max_i];
		int a = (d > 0 ? +1 : -1);
		solved_pressures[max_i] += a /** scale * 0.1*/;
		
		a1 = a0;
		i1 = i0;
		a0 = a;
		i0 = max_i;
	}
	
	//LOG("Solve took " + ts.ToString());
	
	
	for(int i = 0; i < solved_pressures.GetCount(); i++) {
		double pres = pressures[i] + solved_pressures[i];
		double ask = this->ask[i];
		double diff = pres * point[i];
		double solved_ask = ask + diff;
		this->solved_ask[i] = solved_ask;
	}
}

void NetPressureSolver::Serialize(Stream& s) {
	s % pressures % solved_pressures % ask % solved_ask % point
	  % scale
	  % use_mtask
	  
	  % spreads
	  % symbols
	  % currencies % real_symbols
	  % min_pips;
	for(int i = 0; i < MAX_REALSYMBOLS; i++) {
		s % bids_real[i] % asks_real[i] % position[i] % prev_position[i];
	}
	s % lots % ALPHA % max_arbitrage_count % real_sym_count;
}











Net::Net() {
	
	LoadThis();
	
	if (mom_periods.IsEmpty()) {
		mom_periods.Add(1);
		mom_periods.Add(2);
		mom_periods.Add(3);
		mom_periods.Add(5);
		mom_periods.Add(10);
		mom_periods.Add(30);
		mom_periods.Add(100);
		
		scale = 50;
	}
}

Net::~Net() {
	running = false;
	while (!stopped) Sleep(100);
}

void Net::Data() {
	System& sys = GetSystem();
	
	if (best_comb.GetCount() && open_bufs.GetCount()) {
		
		int row = 0;
		for(int j = 0; j < open_bufs.GetCount(); j++) {
			ConstBuffer& buf = *open_bufs[j];
			int cursor = buf.GetCount() - 1;
			double point = solver.GetPoint(j);
			
			double o0 = buf.Get(cursor);
			
			double pres_sum = 0.0;
			for(int k = 0; k < mom_periods.GetCount(); k++) {
				double o1 = buf.Get(max(0, cursor - mom_periods[k]));
				
				double mul = best_comb[row++];
				double diff = (o0 - o1) / point;
				double pres = mul * diff;
				pres_sum += pres;
			}
			pres_sum /= mom_periods.GetCount() * 10000;
			
			if (pres_sum < -scale) pres_sum = -scale;
			if (pres_sum > +scale) pres_sum = +scale;
			
			solver.SetAsk(j, o0);
			solver.SetPressure(j, pres_sum);
		}
		
		solver.Solve();
		
		for(int j = 0; j < open_bufs.GetCount(); j++) {
			String symstr = sys.GetSymbol(j);
			ConstBuffer& buf = *open_bufs[j];
			int cursor = buf.GetCount() - 1;
			double ask = buf.Get(cursor);
			double forecast = solver.GetSolvedAsk(j);
			double diff = forecast - ask;
			int signal = diff <= 0.0;
			if (symstr == "EURUSD" ||
				symstr == "GBPUSD" ||
				symstr == "USDCHF" ||
				symstr == "USDJPY" ||
				symstr == "USDCAD" ||
				symstr == "AUDUSD" ||
				symstr == "NZDUSD" ||
				symstr == "EURCHF" ||
				symstr == "EURGBP")
					sys.SetSignal(j, signal);
		}
		
	}
	
	if (!running) {
		running = true;
		stopped = false;
		Thread::Start(THISBACK(Process));
	}
}

void Net::Process() {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	
	
	Index<int> sym_ids, tf_ids;
	Vector<FactoryDeclaration> indi_ids;
	for(int i = 0; i < mt.GetSymbolCount(); i++)
		sym_ids.Add(i);
	tf_ids.Add(4);
	indi_ids.Add().factory = sys.Find<DataBridge>();
	sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	ASSERT(!work_queue.IsEmpty());
	
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
		
		Core& c = *ci.core;
		open_bufs.Add(ci.sym, &c.GetBuffer(0));
	}
	SortByKey(open_bufs, StdLess<int>());
	ASSERT(open_bufs.GetCount() == GetMetaTrader().GetSymbolCount());
	int c = open_bufs[0]->GetCount();
	for(int i = 0; i < open_bufs.GetCount(); i++) {
		int c2 = open_bufs[i]->GetCount();
		LOG(c << " == " << c2);
		if (c != c2)
			Panic("Symbol open count mismatch.");
	}
	
	
	while (IsRunning()) {
		
		
		Optimize();
		
	}
	
	running = false;
	stopped = true;
}

void Net::Optimize() {
	System& sys = GetSystem();
	
	
	for(int i = 0; i < 2; i++)
		for(int i = 0; i < work_queue.GetCount(); i++)
			sys.Process(*work_queue[i], true);
	
	
	int end = open_bufs[0]->GetCount() - 1;
	#ifdef flagDEBUG
	int begin = max(0, end - 10);
	#else
	int begin = max(0, end - 10);
	#endif
	
	solver.SetScale(scale);
	TimeStop ts;
	
	if (opt.GetRound() == 0 || opt.GetRound() >= opt.GetMaxRounds()) {
		
		int total = open_bufs.GetCount() * mom_periods.GetCount();
		opt.Min().SetCount(total);
		opt.Max().SetCount(total);
		int row = 0;
		for(int i = 0; i < open_bufs.GetCount(); i++) {
			for(int j = 0; j < mom_periods.GetCount(); j++) {
				opt.Min()[row] = -scale;
				opt.Max()[row] = +scale;
				row++;
			}
		}
		
		#ifdef flagDEBUG
		opt.SetMaxGenerations(5);
		#else
		opt.SetMaxGenerations(5000);
		#endif
		opt.Init(total, 33);
		
		solver.Init();
		solver.UseMtAsk(false);
		
		training_pts.SetCount(opt.GetMaxRounds(), 0.0);
	}
	
	
	
	while (opt.GetRound() < opt.GetMaxRounds() && IsRunning()) {
		
		opt.Start();
		
		const Vector<double>& trial = opt.GetTrialSolution();
		
		
		double diff_sum = 0.0;
		for(int i = begin; i < end; i++) {
			
			int row = 0;
			for(int j = 0; j < open_bufs.GetCount(); j++) {
				ConstBuffer& buf = *open_bufs[j];
				double point = solver.GetPoint(j);
				
				double o0 = buf.Get(i);
				
				double pres_sum = 0.0;
				for(int k = 0; k < mom_periods.GetCount(); k++) {
					double o1 = buf.Get(max(0, i - mom_periods[k]));
					
					double mul = trial[row++];
					double diff = (o0 - o1) / point;
					double pres = mul * diff;
					pres_sum += pres;
				}
				pres_sum /= mom_periods.GetCount() * 10000;
				
				if (pres_sum < -scale) pres_sum = -scale;
				if (pres_sum > +scale) pres_sum = +scale;
				
				solver.SetAsk(j, o0);
				solver.SetPressure(j, pres_sum);
			}
			
			solver.Solve();
			
			for(int j = 0; j < open_bufs.GetCount(); j++) {
				ConstBuffer& buf = *open_bufs[j];
				double ask = buf.Get(i+1);
				double forecast = solver.GetSolvedAsk(j);
				double point = solver.GetPoint(j);
				double diff = fabs(ask - forecast) / point;
				diff_sum += diff;
				
				if (once_diff) {
					double forecast = buf.Get(i);
					straight_diff += fabs(ask - forecast) / point;
				}
			}
			
		}
		
		if (once_diff) {
			
			once_diff = false;
		}
		
		training_pts[opt.GetRound()] = -diff_sum;
		
		opt.Stop(-diff_sum);
		
		
		if (ts.Elapsed() >= 60 *1000) {
			StoreThis();
			ts.Reset();
		}
	}
	
	best_comb <<= opt.GetBestSolution();
	StoreThis();
}









NetCircle::NetCircle() {
	
}

void NetCircle::LeftDown(Point p, dword keyflags) {
	mode++;
	if (mode >= MODE_COUNT) mode = 0;
	Refresh();
}

void NetCircle::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	int curcount = sys.GetMajorCurrencyCount();
	int symcount = sys.GetSymbolCount();
	
	double angle = 2*M_PI / curcount;
	
	Point center(sz.cx/2, sz.cy/2);
	
	Font fnt = Monospace(14);
	for(int i = 0; i < curcount; i++) {
		double a = i * angle;
		
		int x = center.x + cos(a) * sz.cx/2* 3/4;
		int y = center.y + sin(a) * sz.cy/2* 3/4;
		
		id.DrawRect(x - 2, y - 2, 5, 5, Green());
		
		x = center.x + cos(a) * sz.cx/2* 7/8;
		y = center.y + sin(a) * sz.cy/2* 7/8;
		
		String cur = sys.GetMajorCurrency(i);
		Size sz = GetTextSize(cur, fnt);
		x -= sz.cx / 2;
		y -= sz.cy / 2;
		id.DrawText(x, y, cur, fnt, Black());
		
	}
	
	if (solver) {
		int scale = solver->GetScale();
		
		if (solver->GetSolvedPressureCount() > 0)
		for(int i = 0; i < symcount; i++) {
			int cur0 = sys.FindMajorCurrency(sys.GetSymbolCurrency(i, 0));
			int cur1 = sys.FindMajorCurrency(sys.GetSymbolCurrency(i, 1));
			if (cur1 == -1) continue;
			
			double a0 = cur0 * angle;
			int x0 = center.x + cos(a0) * sz.cx/2* 3/4;
			int y0 = center.y + sin(a0) * sz.cy/2* 3/4;
			
			double a1 = cur1 * angle;
			int x1 = center.x + cos(a1) * sz.cx/2* 3/4;
			int y1 = center.y + sin(a1) * sz.cy/2* 3/4;
			
			int pres;
			if (mode == MODE_PRES) {
				pres = solver->GetPressure(i);
			}
			else if (mode == MODE_SOLVEDPRES) {
				pres = solver->GetSolvedPressure(i);
			}
			else {
				pres = solver->GetPressure(i) + solver->GetSolvedPressure(i);
			}
			
			int line_width = 1 + abs(pres) * 10 / scale;
			Color clr = Blend(Red(), Green(), Upp::max(0, Upp::min(255, pres * 128 / scale + 128)));
			
			id.DrawLine(x0, y0, x1, y1, line_width, clr);
		}
		
		if (mode == MODE_PRES) {
			id.DrawText(2, 2, "Mode: Input pressure", fnt, Black());
		}
		else if (mode == MODE_SOLVEDPRES) {
			id.DrawText(2, 2, "Mode: Solved balancing pressure", fnt, Black());
		}
		else {
			id.DrawText(2, 2, "Mode: sum of input pressure and solved balancing pressure", fnt, Black());
		}
		
	}
	
	w.DrawImage(0, 0, id);
}








NetCtrl::NetCtrl() {
	Title("NetCtrl");
	
	Add(tabs.SizePos());
	
	tabs.Add(testctrl.SizePos(), "Testing");
	tabs.Add(optctrl.SizePos(), "Optimization");
	
	testctrl.Add(testsymbol.TopPos(0,30).LeftPos(0,200));
	testctrl.Add(testslider.TopPos(0,30).HSizePos(200, 200));
	testctrl.Add(testrandompres.TopPos(0,30).RightPos(0,190));
	testctrl.Add(testcircle.HSizePos().VSizePos(30, 200));
	testctrl.Add(testarbitrage.HSizePos(0, 200).BottomPos(0, 200));
	testctrl.Add(testsiglist.RightPos(0, 200).BottomPos(0, 200));
	
	optctrl.Add(optcircle.HSizePos().VSizePos(0, 200));
	optctrl.Add(optdraw.HSizePos().BottomPos(0, 200));
	
	testcircle.solver = &testsolver;
	testslider.MinMax(-10, +10);
	testrandompres.SetLabel("Random pressure");
	testrandompres.Set(false);
	testrandompres <<= THISBACK(Data);
	testsolver.SetScale(10);
	testsymbol <<= THISBACK(SetSymbol);
	testslider <<= THISBACK(SetPressure);
	testarbitrage.AddColumn("Real");
	testarbitrage.AddColumn("A");
	testarbitrage.AddColumn("Bid");
	testarbitrage.AddColumn("B");
	testarbitrage.AddColumn("Ask");
	testarbitrage.AddColumn("Diff");
	testarbitrage.AddColumn("Count");
	testarbitrage.ColumnWidths("1 3 1 3 1 1 1");
	testsiglist.AddColumn("Symbol");
	testsiglist.AddColumn("Signal");
	testsolver.UseMtAsk();
	
	optcircle.solver = &GetNet().solver;
	
}

void NetCtrl::SetSymbol() {
	int i = testsymbol.GetIndex();
	testslider.SetData(testsolver.GetPressure(i));
}

void NetCtrl::SetPressure() {
	int i = testsymbol.GetIndex();
	testsolver.SetPressure(i, testslider.GetData());
	testsolver.Solve();
	Data();
}

	
void NetCtrl::Data() {
	Net& net = GetNet();
	System& sys = GetSystem();
	
	if (GetTitle() == "") Title("NetCtrl");
	
	net.Data();
	
	int tab = tabs.Get();
	
	if (tab == 0) {
		if (testsymbol.GetCount() == 0) {
			testsolver.Init();
			for(int i = 0; i < sys.GetSymbolCount(); i++) {
				testsymbol.Add(sys.GetSymbol(i));
			}
			testsymbol.SetIndex(0);
			SetSymbol();
		}
		
		bool b = testrandompres.Get();
		if (b) {
			testsolver.RandomPressure();
			testsolver.Solve();
		}
		
		int row = 0;
		for(int i = 0; i < testsolver.symbols.GetCount(); i++) {
			NetPressureSolver::Symbol& s = testsolver.symbols[i];
			
			int sym_count = s.sym.GetCount();
			for(int j = 0; j < s.pair.GetCount(); j++) {
				NetPressureSolver::VariantPair& pair = s.pair[j];
				if (pair.arbitrage_count > 0) {
					int variant1_id = j / sym_count;
					int variant2_id = j % sym_count;
					NetPressureSolver::VariantSym& v1 = s.sym[variant1_id];
					NetPressureSolver::VariantSym& v2 = s.sym[variant2_id];
					
					double diff1 = v1.bid - v2.ask;
					double diff2 = v2.bid - v1.ask;
					if (diff1 - s.min_pips > 0) {
						testarbitrage.Set(row, 0, testsolver.SymbolToStr(i, 0));
						testarbitrage.Set(row, 1, testsolver.SymbolToStr(i, variant1_id));
						testarbitrage.Set(row, 2, v1.bid);
						testarbitrage.Set(row, 3, testsolver.SymbolToStr(i, variant2_id));
						testarbitrage.Set(row, 4, v2.ask);
						testarbitrage.Set(row, 5, diff1 / s.point);
						testarbitrage.Set(row, 6, pair.arbitrage_count);
						row++;
					}
					if (diff2 - s.min_pips > 0) {
						testarbitrage.Set(row, 0, testsolver.SymbolToStr(i, 0));
						testarbitrage.Set(row, 1, testsolver.SymbolToStr(i, variant2_id));
						testarbitrage.Set(row, 2, v2.bid);
						testarbitrage.Set(row, 3, testsolver.SymbolToStr(i, variant1_id));
						testarbitrage.Set(row, 4, v1.ask);
						testarbitrage.Set(row, 5, diff2 / s.point);
						testarbitrage.Set(row, 6, pair.arbitrage_count);
						row++;
					}
				}
			}
		}
		testarbitrage.SetSortColumn(5, true);
		
		for(int i = 0; i < testsolver.real_sym_count; i++) {
			testsiglist.Set(i, 0, testsolver.real_symbols[i]);
			testsiglist.Set(i, 1, testsolver.prev_position[i]);
		}
	}
	else if (tab == 1) {
		
		
		
		
	}
}


void NetOptCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	Net& net = GetNet();
	
	DrawVectorPolyline(id, sz, net.training_pts, polyline, net.opt.GetRound(), -net.straight_diff);
	
	w.DrawImage(0, 0, id);
}


}
