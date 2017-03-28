//+------------------------------------------------------------------+
//|                                                                  |
//|                                                MT4Connection.mq4 |
//|                                                                  |
//+------------------------------------------------------------------+

#include <stderror.mqh>
#include <stdlib.mqh>

#property copyright "sppp"
#property link		"mail@sppp.heliohost.org"
#property version	"1.00"
#property strict
#property script_show_inputs

input int		Port=42000;
int				pricecount = 1;

int store_tfs[8];	
int store_tf_count = 8;

string filedir;
string	symbols[512];
int		symbol_count;
double   volume_previous[512];
double   ask_previous[512];
double   bid_previous[512];

#import "MT4ConnectionDll.dll";
	int		 ConnectionInit(int port, string file_path);
	void     ConnectionDeinit();
	string	 GetLine();
	void	 PutLine(string s);
#import

#define PI 3.14159265

double Abs(double value) {
	if (value < 0) value *= -1;
	return value;
}

int LoadSymbols() {
	int handle = FileOpenHistory("symbols.raw", FILE_BIN | FILE_READ);
	if (handle == -1) {
		Alert("File: symbols.raw  Error: " + ErrorDescription(GetLastError()));
		return -1;
	}
	symbol_count = FileSize(handle) / 1936;
	Print("Symbol count: ", symbol_count);
	FileSeek(handle, 0, SEEK_SET);
	for (int i = 0; i < symbol_count; i++) {
		symbols[i] = FileReadString(handle, 12);
		string description = FileReadString(handle, 75);
		FileSeek(handle, 1849, SEEK_CUR);
		
		double bid = MarketInfo(symbols[i], MODE_BID);
		if (bid == 0) {
		   symbol_count--;
		   i--;
		   continue;
		}
		//Print(i, ": ", symbols[i]);
	}
	FileClose(handle);
	return symbol_count;
}

// Get all symbols
string GetSymbols() {
	string s;
	int count = symbol_count;
	for (int i = 0; i < count; i++) {
	   string desc = SymbolInfoString(symbols[i], SYMBOL_DESCRIPTION);
	   StringReplace(desc, ",", " ");
	   
	   string quotes, trades;
	   for (int j = 0; j < 7; j++) {
	      datetime from, to;
	      
	      SymbolInfoSessionQuote(symbols[i], j, 0, from, to);
	      quotes += "," + IntegerToString(from);
	      quotes += "," + IntegerToString(to);
	      
	      SymbolInfoSessionTrade(symbols[i], j, 0, from, to);
	      trades += "," + IntegerToString(from);
	      trades += "," + IntegerToString(to);
	      
	   }
	   
		s += 
		   symbols[i] + "," +
         DoubleToString(MarketInfo(symbols[i], MODE_LOTSIZE)) + "," +
         DoubleToString(MarketInfo(symbols[i], MODE_TRADEALLOWED)) + "," +
         DoubleToString(MarketInfo(symbols[i], MODE_PROFITCALCMODE)) + "," +
         DoubleToString(MarketInfo(symbols[i], MODE_MARGINCALCMODE)) + "," +
         DoubleToString(MarketInfo(symbols[i], MODE_MARGINHEDGED)) + "," +
         DoubleToString(MarketInfo(symbols[i], MODE_MARGINREQUIRED)) + "," +
         
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_SELECT)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_VISIBLE)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_DIGITS)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_SPREAD_FLOAT)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_SPREAD)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_TRADE_CALC_MODE)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_TRADE_MODE)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_START_TIME)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_EXPIRATION_TIME)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_TRADE_STOPS_LEVEL)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_TRADE_FREEZE_LEVEL)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_TRADE_EXEMODE)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_SWAP_MODE)) + "," +
         IntegerToString(SymbolInfoInteger(symbols[i], SYMBOL_SWAP_ROLLOVER3DAYS)) + "," +
         
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_POINT)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_TRADE_TICK_VALUE)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_TRADE_TICK_SIZE)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_TRADE_CONTRACT_SIZE)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_VOLUME_MIN)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_VOLUME_MAX)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_VOLUME_STEP)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_SWAP_LONG)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_SWAP_SHORT)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_MARGIN_INITIAL)) + "," +
         DoubleToString(SymbolInfoDouble(symbols[i], SYMBOL_MARGIN_MAINTENANCE)) + "," +
         
         SymbolInfoString(symbols[i], SYMBOL_CURRENCY_BASE) + "," +
         SymbolInfoString(symbols[i], SYMBOL_CURRENCY_PROFIT) + "," +
         SymbolInfoString(symbols[i], SYMBOL_CURRENCY_MARGIN) + "," +
         SymbolInfoString(symbols[i], SYMBOL_PATH) + "," +
         desc + quotes + trades + ";";
	}
	return s;
}

// Get ask and bid prices for all symbols
string GetAskBid() {
	RefreshRates();
	string s;
	int count = symbol_count;
	for (int i = 0; i < count; i++) {
		string sym = symbols[i];
		double ask, bid, vol, volume_now;
		
		ask = MarketInfo(sym, MODE_ASK);
		bid = MarketInfo(sym, MODE_BID);
		volume_now = iVolume(sym, 1, 0);
		
		if (volume_previous[i] > volume_now) {
			double volume_previous_max = iVolume(sym, 1, 1);
			vol = volume_previous_max - volume_previous[i] + volume_now;
		} else {
			vol = volume_now - volume_previous[i];
		}
	
		volume_previous[i] = volume_now;

		s += symbols[i] + "," + DoubleToString(ask) + "," + DoubleToString(bid) + "," + DoubleToString(vol) + ";";
	}
	return s;
}

// Collect ask and bid
int askbid_file;
void StoreAskBid() {
	RefreshRates();
	
	datetime now = TimeCurrent();
	
	int count = symbol_count;
	bool changes = false;
	for (int i = 0; i < count; i++) {
		double ask = MarketInfo(symbols[i], MODE_ASK);
		double bid = MarketInfo(symbols[i], MODE_BID);
		
		if (ask == ask_previous[i] && bid == bid_previous[i])
		   continue;
		ask_previous[i] = ask;
		bid_previous[i] = bid;

      if (askbid_file == 0) {
   	   string path = "askbid.bin";
   	   askbid_file = FileOpen(path, FILE_READ|FILE_WRITE|FILE_BIN);
   	   if (askbid_file == 0) {
   	      Print("Can't open askbid.bin");
   	      return;
   	   } else {
   	      //Print("Opened file: ", path);
   	   }
   	   FileSeek(askbid_file, 0, SEEK_END);
   	}
   			
		FileWriteInteger(askbid_file, now);
		FileWriteInteger(askbid_file, i);
		FileWriteDouble(askbid_file, ask);
		FileWriteDouble(askbid_file, bid);
		changes = true;
	}
	//if (changes) {
	   //FileFlush(askbid_file);
	if (askbid_file != 0) {
	   FileClose(askbid_file);
	   askbid_file = 0;
	   //Print("Modified askbid.bin");
	}
}



// Get all timeframe prices for all symbols
string GetPrices() {
	RefreshRates();
	string s;
	
	s += IntegerToString(store_tf_count) + ",";
	
	
	for (int tfi = 0; tfi < store_tf_count; tfi++) {
		int tf = store_tfs[tfi];
		
		s += IntegerToString(tf) + ",";
		int count = symbol_count;
		s += IntegerToString(count) + ",";
		
		for (int i = 0; i < count; i++) {
			string realsym = symbols[i];
			int count  = iBars(realsym, tf);
			if (count > pricecount) count = pricecount;
			
			s += IntegerToString(count) + ",";
			
			// From oldest to newest (0 is newest)
			for  (int j = count-1; j >= 0; j--) {
				
				int time;
				double high, low, open, close, volume;
				
				// Get common data
				time	   = iTime		(realsym, tf, j);
				high	   = iHigh		(realsym, tf, j);
				low		= iLow		(realsym, tf, j);
				open	   = iOpen		(realsym, tf, j);
				close	   = iClose	   (realsym, tf, j);
				volume	= iVolume	(realsym, tf, j);
				
				// Write common data
				s += IntegerToString(time) + ",";
				s += DoubleToString(high) + ",";
				s += DoubleToString(low) + ",";
				s += DoubleToString(open) + ",";
				s += DoubleToString(close) + ",";
				s += DoubleToString(volume) + ",";
				
			}
		}
	}
	
	// Write magic number at the end
	s += IntegerToString(1234);
	
	return s;
}

// Store all closed orders to a file
string GetOrders(int pool) {
	int c1;
	string s;
	
	if (pool == MODE_HISTORY)
	   c1 = OrdersHistoryTotal();
	else
	   c1 = OrdersTotal();
	   
	for (int i = 0; i < c1; i++) {
		if (OrderSelect(i, SELECT_BY_POS, pool) == false) {
		   //return "FAIL";
		   continue;
		}
		
		// Get order data
		int ticket = OrderTicket();
		string symbol = OrderSymbol();
		double open = OrderOpenPrice();
		double close = OrderClosePrice();
		int begin = OrderOpenTime();
		int end = OrderCloseTime();
		int type = OrderType();
		double tp = OrderTakeProfit();
		double sl = OrderStopLoss();
		double lots = OrderLots();
		double profit = OrderProfit();
		double commission = OrderCommission();
		double swap = OrderSwap();
		int expiration = OrderExpiration();
		
		// Write order data
		s += IntegerToString(ticket) + ",";
		s += symbol + ",";
		s += DoubleToString(open) + ",";
		s += DoubleToString(close) + ",";
		s += IntegerToString(begin) + ",";
		s += IntegerToString(end) + ",";
		s += IntegerToString(type) + ",";
		s += DoubleToString(tp) + ",";
		s += DoubleToString(sl) + ",";
		s += DoubleToString(lots) + ",";
		s += DoubleToString(profit) + ",";
		s += DoubleToString(commission) + ",";
		s += DoubleToString(swap) + ",";
		s += DoubleToString(expiration) + ",";
	}
	
	// Write magic number at the end
	s += IntegerToString(1234);
	
	return s;
}

string GetHistoryOrders() {
	return GetOrders(MODE_HISTORY);
}

string GetOrders() {
	return GetOrders(MODE_TRADES);
}

int FindPriceTime(int sym, int tf, datetime ts) {
   //Print(sym, ", ", tf, ", ", ts, ",", (unsigned int)ts);
   int total = iBars(symbols[sym], tf);
   for (int i = 0; i < total; i++) {
      datetime cmp_ts = iTime(symbols[sym], tf, i);
      if (cmp_ts == ts)
         return total-1-i;
   }
   return -1;
}

string GetLatestPriceTimes(bool allsym) {
   RefreshRates();
   int count = symbol_count;
   string s;
	for (int i = 0; i < count; i++) {
	   for (int tfi = 0; tfi < store_tf_count; tfi++) {
		   int tf = store_tfs[tfi];
			s += 
			IntegerToString(iTime(symbols[i], tf, 0)) + ",";
		}
		s += ";";
   }
   return s;
}

string GetEarliestPriceTimes(bool allsym) {
   RefreshRates();
   int count = symbol_count;
   string s;
	for (int i = 0; i < count; i++) {
	   for (int tfi = 0; tfi < store_tf_count; tfi++) {
		   int tf = store_tfs[tfi];
		   int count = iBars(symbols[i], tf);
			s += IntegerToString(iTime(symbols[i], tf, count-1)) + ",";
		}
		s += ";";
   }
   return s;
}

double RoundLots(double value) {
	int steps = value / 0.01;
	value = steps * 0.01;
	return  value;
}



double tmp;
string sym;
int cmd;
double lot;


void PacketReturnDbl(double d) {
	PutLine(DoubleToString(d));
}

void PacketReturnInt(int i) {
	PutLine(IntegerToString(i));
}

void PacketReturnStr(string s) {
	PutLine(s);
}

void OnStart() {
	filedir = TerminalInfoString(TERMINAL_DATA_PATH);
	
	store_tfs[0] = 1;
	store_tfs[1] = 5;
	store_tfs[2] = 15;
	store_tfs[3] = 30;
	store_tfs[4] = 60;
	store_tfs[5] = 240;
	store_tfs[6] = 1440;
	store_tfs[7] = 10080;
	
	datetime now;
	int hour, minute;
	now = TimeCurrent();
	hour = TimeHour(now);
	minute = TimeMinute(now);
	Print("Startup time is ", hour, ":", minute);
	
	now = TimeGMT();
	hour = TimeHour(now);
	minute = TimeMinute(now);
	Print("UTC time is ", hour, ":", minute);
	
	Print("Listening port: ", Port);
	
	
	if (ConnectionInit(Port, filedir)) {
		Alert("Init failed");
		return;
	}
	
	// Get all symbols
	if (LoadSymbols() == -1 || symbol_count == 0) {
		Alert("Loading symbols failed");
		return;
	}
	
	
	Print("Init ready");
	
	while (IsStopped() == false) {
		string line = GetLine();
		//Print("Received line: ", line);
		
		if (StringCompare(line, "") == 0) {
			PutLine("");
			StoreAskBid();
			Sleep(1000);
		}
		else {
			string col[];
			int col_count = StringSplit(line, ',', col);
			
			//Print("Columns: ", col_count);
			
			int packet_id = StringToInteger(col[0]);
			
			switch (packet_id) {
				case 0: PacketReturnDbl(AccountInfoDouble(StringToInteger(col[1]))); break;
				case 1: PacketReturnInt(AccountInfoInteger(StringToInteger(col[1]))); break;
				case 2: PacketReturnStr(AccountInfoString(StringToInteger(col[1]))); break;
				case 3: PacketReturnDbl(AccountBalance()); break;
				case 4: PacketReturnDbl(AccountCredit()); break;
				case 5: PacketReturnStr(AccountCompany()); break;
				case 6: PacketReturnStr(AccountCurrency()); break;
				case 7: PacketReturnDbl(AccountEquity()); break;
				case 8: PacketReturnDbl(AccountFreeMargin()); break;
				case 9: PacketReturnDbl(AccountFreeMarginCheck(col[1], StringToInteger(col[2]), StringToDouble(col[3]))); break;
				case 10: PacketReturnDbl(AccountFreeMarginMode()); break;
				case 11: PacketReturnInt(AccountLeverage()); break;
				case 12: PacketReturnDbl(AccountMargin()); break;
				case 13: PacketReturnStr(AccountName()); break;
				case 14: PacketReturnInt(AccountNumber()); break;
				case 15: PacketReturnDbl(AccountProfit()); break;
				case 16: PacketReturnStr(AccountServer()); break;
				case 17: PacketReturnInt(AccountStopoutLevel()); break;
				case 18: PacketReturnInt(AccountStopoutMode()); break;
				case 19: PacketReturnDbl(MarketInfo(col[1], StringToInteger(col[2]))); break;
				case 20: PacketReturnInt(SymbolsTotal(StringToInteger(col[1]))); break;
				case 21: PacketReturnStr(SymbolName(StringToInteger(col[1]), StringToInteger(col[2]))); break;
				case 22: PacketReturnInt(SymbolSelect(col[1], StringToInteger(col[2]))); break;
				case 23: PacketReturnDbl(SymbolInfoDouble(col[1], StringToInteger(col[2]))); break;
				case 24: PacketReturnInt(SymbolInfoInteger(col[1], StringToInteger(col[2]))); break;
				case 25: PacketReturnStr(SymbolInfoString(col[1], StringToInteger(col[2]))); break;
				case 26: PacketReturnInt(RefreshRates()); break;
				case 27: PacketReturnInt(iBars(col[1], StringToInteger(col[2]))); break;
				case 28: PacketReturnInt(iBarShift(col[1], StringToInteger(col[2]), StringToInteger(col[3]))); break;
				case 29: PacketReturnDbl(iClose(col[1], StringToInteger(col[2]), StringToInteger(col[3]))); break;
				case 30: PacketReturnDbl(iHigh(col[1], StringToInteger(col[2]), StringToInteger(col[3]))); break;
				case 31: PacketReturnDbl(iLow(col[1], StringToInteger(col[2]), StringToInteger(col[3]))); break;
				case 32: PacketReturnDbl(iOpen(col[1], StringToInteger(col[2]), StringToInteger(col[3]))); break;
				case 33: PacketReturnInt(iHighest(col[1], StringToInteger(col[2]), StringToInteger(col[3]), StringToInteger(col[4]), StringToInteger(col[5]))); break;
				case 34: PacketReturnInt(iLowest(col[1], StringToInteger(col[2]), StringToInteger(col[3]), StringToInteger(col[4]), StringToInteger(col[5]))); break;
				case 35: PacketReturnInt(iTime(col[1], StringToInteger(col[2]), StringToInteger(col[3]))); break;
				case 36: PacketReturnInt(iVolume(col[1], StringToInteger(col[2]), StringToInteger(col[3]))); break;
				case 37: PacketReturnInt(OrderClose(StringToInteger(col[1]), StringToDouble(col[2]), StringToDouble(col[3]), StringToInteger(col[4]))); break;
				case 38: PacketReturnDbl(OrderClosePrice()); break;
				case 39: PacketReturnInt(OrderCloseTime()); break;
				case 40: PacketReturnStr(OrderComment()); break;
				case 41: PacketReturnDbl(OrderCommission()); break;
				case 42: PacketReturnInt(OrderDelete(StringToInteger(col[1]))); break;
				case 43: PacketReturnInt(OrderExpiration()); break;
				case 44: PacketReturnDbl(OrderLots()); break;
				case 45: PacketReturnInt(OrderMagicNumber()); break;
				case 46: PacketReturnInt(OrderModify(StringToInteger(col[1]), StringToDouble(col[2]), StringToDouble(col[3]), StringToDouble(col[4]), StringToInteger(col[5]))); break;
				case 47: PacketReturnDbl(OrderOpenPrice()); break;
				case 48: PacketReturnInt(OrderOpenTime()); break;
				case 49: PacketReturnDbl(OrderProfit()); break;
				case 50: PacketReturnInt(OrderSelect(StringToInteger(col[1]), StringToInteger(col[2]), StringToInteger(col[3]))); break;
				case 51: PacketReturnInt(OrderSend(col[1], StringToInteger(col[2]), StringToDouble(col[3]), StringToDouble(col[4]), StringToInteger(col[5]), StringToDouble(col[6]), StringToDouble(col[7]), NULL, StringToInteger(col[8]), StringToInteger(col[9]))); break;
				case 52: PacketReturnInt(OrdersHistoryTotal()); break;
				case 53: PacketReturnDbl(OrderStopLoss()); break;
				case 54: PacketReturnInt(OrdersTotal()); break;
				case 55: PacketReturnDbl(OrderSwap()); break;
				case 56: PacketReturnStr(OrderSymbol()); break;
				case 57: PacketReturnDbl(OrderTakeProfit()); break;
				case 58: PacketReturnInt(OrderTicket()); break;
				case 59: PacketReturnInt(OrderType()); break;
				case 60: PacketReturnStr(GetSymbols()); break;
				case 61: PacketReturnStr(GetAskBid()); break;
				case 62: PacketReturnStr(GetPrices()); break;
				case 63: PacketReturnStr(GetHistoryOrders()); break;
				case 64: PacketReturnStr(GetOrders()); break;
				case 65: break;
				case 66: PacketReturnStr(ErrorDescription(GetLastError())); break;
				case 67: PacketReturnStr(GetLatestPriceTimes(StringToInteger(col[1]))); break;
				case 68: PacketReturnInt(IsDemo()); break;
				case 69: PacketReturnInt(IsConnected()); break;
				case 70: PacketReturnInt(FindPriceTime(StringToInteger(col[1]), StringToInteger(col[2]), StringToInteger(col[3]))); break;
				case 71: PacketReturnStr(GetEarliestPriceTimes(StringToInteger(col[1]))); break;
				case 100: PacketReturnInt(123456); break; // Test response
				default: PacketReturnInt(0); 
			}
			
		}
	}
	
	Print("Closing askbid.bin");
	if (askbid_file != 0)
	   FileClose(askbid_file);
	
	Print("Running ConnectionDeinit");
	ConnectionDeinit();
	
	Print("Exiting");
}


