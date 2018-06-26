#include "Overlook.h"

namespace Overlook {

ArbitrageCtrl::ArbitrageCtrl() {
	Add(split.SizePos());
	split.Horz();
	
	split << arblist << siglist;
	split.SetPos(8000);
	
	arblist.AddColumn("Real");
	arblist.AddColumn("A");
	arblist.AddColumn("Bid");
	arblist.AddColumn("B");
	arblist.AddColumn("Ask");
	arblist.AddColumn("Diff");
	arblist.AddColumn("Count");
	arblist.ColumnWidths("1 3 1 3 1 1 1");
	
	siglist.AddColumn("Symbol");
	siglist.AddColumn("Signal");
}
	
void ArbitrageCtrl::Data() {
	
	Arbitrage& a = GetArbitrage();
	
	int row = 0;
	for(int i = 0; i < a.symbols.GetCount(); i++) {
		Arbitrage::Symbol& s = a.symbols[i];
		
		int sym_count = s.sym.GetCount();
		for(int j = 0; j < s.pair.GetCount(); j++) {
			Arbitrage::VariantPair& pair = s.pair[j];
			if (pair.arbitrage_count > 0) {
				int variant1_id = j / sym_count;
				int variant2_id = j % sym_count;
				Arbitrage::VariantSym& v1 = s.sym[variant1_id];
				Arbitrage::VariantSym& v2 = s.sym[variant2_id];
				
				double diff1 = v1.bid - v2.ask;
				double diff2 = v2.bid - v1.ask;
				if (diff1 - s.min_pips > 0) {
					arblist.Set(row, 0, a.SymbolToStr(i, 0));
					arblist.Set(row, 1, a.SymbolToStr(i, variant1_id));
					arblist.Set(row, 2, v1.bid);
					arblist.Set(row, 3, a.SymbolToStr(i, variant2_id));
					arblist.Set(row, 4, v2.ask);
					arblist.Set(row, 5, diff1 / s.point);
					arblist.Set(row, 6, pair.arbitrage_count);
					row++;
				}
				if (diff2 - s.min_pips > 0) {
					arblist.Set(row, 0, a.SymbolToStr(i, 0));
					arblist.Set(row, 1, a.SymbolToStr(i, variant2_id));
					arblist.Set(row, 2, v2.bid);
					arblist.Set(row, 3, a.SymbolToStr(i, variant1_id));
					arblist.Set(row, 4, v1.ask);
					arblist.Set(row, 5, diff2 / s.point);
					arblist.Set(row, 6, pair.arbitrage_count);
					row++;
				}
			}
		}
	}
	arblist.SetSortColumn(5, true);
	
	
	for(int i = 0; i < a.real_sym_count; i++) {
		siglist.Set(i, 0, a.real_symbols[i]);
		siglist.Set(i, 1, a.prev_position[i]);
	}
}
	
	
	
	
	
	
	
	
	
	
	
	
Arbitrage::Arbitrage() {
	currencies.Add("AUD");
	currencies.Add("EUR");
	currencies.Add("USD");
	currencies.Add("CHF");
	currencies.Add("JPY");
	currencies.Add("NZD");
	currencies.Add("GBP");
	currencies.Add("CAD");
	/*
	currencies.Add("SGD");
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
	currencies.Add("TRY");
	*/
	Init();
}

Arbitrage::~Arbitrage() {
	running = false;
	while (!stopped) Sleep(100);
}

void Arbitrage::Init() {

	LOG("Arbitrage: min_pips = " + DblStr(min_pips)) ;
	
	GetRealSymbols();
	GetAllSymbols();
	GetPipsD();
	
	max_arbitrage_count = 0;
	
	for (int i = 0; i < real_sym_count; i++)
		position[i] = 0;
		
	GetXTrade("Trade-Arbitrage.txt");
	
	PrintBeginInfo();
	
	
	Thread::Start(THISBACK(Process));
}

void Arbitrage::Process() {
	running = true;
	stopped = false;
	
	while (running) {
		
		TradeArbitrage();
		
		for(int i = 0; i < 10*1 && running; i++)
			Sleep(100);
	}
	
	stopped = true;
}

void Arbitrage::GetRealSymbols() {
	real_sym_count = 0;
	
	MetaTrader& mt = GetMetaTrader();
	const Vector<libmt::Symbol>& symbols = mt.GetSymbols();
	for(int i = 0; i < symbols.GetCount(); i++) {
		real_symbols.Add(symbols[i].name);
		real_sym_count++;
	}
}

void Arbitrage::GetRealBidAsk() {
	MetaTrader& mt = GetMetaTrader();
	const Vector<Price>& prices = mt.GetAskBid();
	
	for (int i = 0; i < prices.GetCount(); i++) {
		const Price& p = prices[i];
		
		#if 1
		bids_real[i] = p.bid;
		asks_real[i] = p.ask;
		#else
		System& sys = GetSystem();
		Vector<Ptr<CoreItem> > work_queue;
		Vector<FactoryDeclaration> indi_ids;
		Index<int> tf_ids, sym_ids;
		FactoryDeclaration decl;
		decl.factory = System::Find<MovingAverage>();
		decl.AddArg(15);
		indi_ids.Add(decl);
		tf_ids.Add(0);
		sym_ids.Add(i);
		work_queue.Clear();
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
		for (int j = 0; j < work_queue.GetCount(); j++)
			sys.Process(*work_queue[j], true);
		MovingAverage& ma = dynamic_cast<MovingAverage&>(*work_queue.Top()->core);
		ConstBuffer& ma_buf = ma.GetBuffer(0);
		int bars = ma_buf.GetCount();
		double ma0 = ma_buf.Get(bars-1);
		double ma1 = ma_buf.Get(bars-2);
		double diff = ma0 - ma1;
		double change = diff * 1440;
		
		bids_real[i] = p.bid + change;
		asks_real[i] = p.ask + change;
		#endif
	}
}

bool Arbitrage::IsRealSymbol(const String& s) {
	return GetSystem().FindSymbol(s) != -1;
}

void Arbitrage::GetAllSymbols() {
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

void Arbitrage::GetPipsD() {
	GetRealBidAsk();
	
	for (int i = 0; i < symbols.GetCount(); i++) {
		Symbol& s = symbols[i];
		
		GetBidAsk(i, 0);
		
		if (s.sym[0].bid > 10) {
			s.point = 0.01;
			s.min_pips = min_pips * 0.01;
		}
		
		else {
			s.point = 0.0001;
			s.min_pips = min_pips * 0.0001;
		}
	}
}

void Arbitrage::GetXTrade(String FileName) {
	/*int handle;
	int AmountStrings = 0;
	String Str1, Str2, Str3, Str[MAX_VARIANTPAIRS];
	
	handle = FileOpen(FileName, FILE_READ);
	
	while (!FileIsEnding(handle)) {
		Str[AmountStrings] = FileReadString(handle);
		AmountStrings++;
	}
	
	FileClose(handle);
	
	for (int i = 0; i < symbols.GetCount(); i++) {
		Pos = 0;
		
		for (int j = 0; j < count[i] - 1; j++) {
			Str1 = SymbolToStr(i, j);
			Pos += j + 1;
			
			for (int k = j + 1; k < count[i]; k++) {
				Str2 = Str1 + " && " + SymbolToStr(i, k);
				Str3 = SymbolToStr(i, k) + " && " + Str1;
				
				for (int m = 0; m < AmountStrings; m++)
					if ((Str[m] == Str2) || (Str[m] == Str3))
						break;
						
				if (m == AmountStrings)
					XTrade[i][Pos] = FALSE;
				else
					XTrade[i][Pos] = TRUE;
					
				xposition[i][Pos] = 0;
				
				arbitrage_count[i][Pos] = 0;
				
				Pos++;
			}
		}
	}
	
	return;*/
}


void Arbitrage::PrintBeginInfo() {
	
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

void Arbitrage::TradeArbitrage() {
	
	GetRealBidAsk();
	
	for (int i = 0; i < real_sym_count; i++)
		position[i] = 0;
	
	
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

void Arbitrage::OpenArbitragePosition(int sym_id, int variant1_id, int variant2_id, double vol) {
	int V;
	double xpos;
	
	Symbol& s = symbols[sym_id];
	int count = s.sym.GetCount();
	
	if (variant1_id < variant2_id) {
		V = variant1_id * count + variant2_id;
		VariantPair& vp = s.pair[V];
		
		xpos = vp.xposition;
		
		if (xpos < -ALPHA)
			return;
			
		xpos += vol;
		
		vp.xposition = -vol;
	}
	
	else {
		V = variant2_id * count + variant1_id;
		VariantPair& vp = s.pair[V];
		
		xpos = vp.xposition;
		
		if (xpos > ALPHA)
			return;
			
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
		
		LOG("variant1_id = " + SymbolToStr(sym_id, variant1_id) + " (Bid = " + DblStr(v1.bid) +
			  "), variant2_id = " + SymbolToStr(sym_id, variant2_id) + " (Ask = " + DblStr(v2.ask) +
			  "), Difference = " + DblStr((v1.bid - v2.ask) / s.point) + " pips");
		LOG(str_out);
		
		str_out = "";
	}
}

String Arbitrage::SymbolToStr(int i, int j) {
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

void Arbitrage::RefreshPositions() {
	bool Flag = false;
	double vol;
	
	for (int i = 0; i < real_sym_count; i++) {
		//position[i] = NormalizeDouble(position[i], DigitsLot);
		position[i] = ((int)(position[i] / 0.01)) * 0.01;
		
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
	
	if (Flag)
		LOG(ArbitragePositions());
		
	//CloseLock();
}

String Arbitrage::ArbitragePositions() {
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


void Arbitrage::MonitoringArbitrage(int sym_id, int variant1_id, int variant2_id) {
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

String Arbitrage::SymbolToFile( int i, int j )
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

void Arbitrage::OpenSymbolPosition(int sym_id, int V, int Type, double Vol) {
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

void Arbitrage::SymbolDone(double Vol, int Symb) {
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

void Arbitrage::GetBidAsk(int i, int j) {
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

void Arbitrage::WriteStatistic(String FileName) {
	/*String Str = "Arbitrage: min_pips = " + DblStr(min_pips);
	int i, j, k, Pos;
	int V1, V2;
	int handle, Cnt;
	int BenchTime = GetTickCount();
	
	Cnt = max_arbitrage_count;
	
	while (Cnt >= 2) {
		for (i = 0; i < symbols.GetCount(); i++) {
			Pos = 0;
			
			for (j = 0; j < count[i] - 1; j++) {
				Pos += j + 1;
				
				for (k = j + 1; k < count[i]; k++) {
					if (arbitrage_count[i][Pos] == Cnt)
						Str = Str + "\n" + arbitrage_count[i][Pos] + ": " + SymbolToStr(i, j) + " && " + SymbolToStr(i, k);
						
					Pos++;
				}
			}
		}
		
		Cnt--;
	}
	
	handle = FileOpen(FileName, FILE_WRITE, "\t");
	
	FileWrite(handle, Str);
	FileClose(handle);
	
	Print("max_arbitrage_count = " + max_arbitrage_count + ", Write Time Statistic = " +
		  DblStr((GetTickCount() - BenchTime) / 1000, 0) + " s.");
	      
	return;*/
}

}
