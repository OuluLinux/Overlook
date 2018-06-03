#ifndef _Overlook_Myfxbook_h_
#define _Overlook_Myfxbook_h_


namespace Overlook {

class Myfxbook : public ParentCtrl {
	
	struct Order : Moveable<Order> {
		Time begin, end;
		String symbol, open, profit;
		bool action = 0;
		double lots = 0.0;
		
		bool operator() (const Order& a, const Order& b) const {
			if (a.begin == b.begin)
				return a.end < b.end;
			return a.begin < b.begin;
		}
		
		void Serialize(Stream& s) {s % begin % end % symbol % open % profit % action % lots;}
	};
	
	struct Account : Moveable<Account> {
		Array<Order> history_orders, orders;
		String url, id;
		double profitability = 0, pips = 0, gain = 0, av_gain = 0;
		
		bool operator() (const Account& a, const Account& b) const {
			return a.av_gain > b.av_gain;
		}
		
		void Serialize(Stream& s) {s % history_orders % orders % url % id % profitability % pips % gain % av_gain;}
	};
	
	struct SymbolStats : Moveable<SymbolStats> {
		String symbol;
		double lots_mult = 0.0;
		int id = -1;
		int signal = 0;
		
		bool wait = true;
		
		bool operator() (const SymbolStats& a, const SymbolStats& b) const {
			return a.lots_mult < b.lots_mult;
		}
		
		String ToString() const {return symbol + ": " + DblStr(lots_mult);}
		void Serialize(Stream& s) {s % symbol % lots_mult % id % signal;}
	};
	
	VectorMap<String, SymbolStats > symbols;
	Vector<Account> accounts;
	Vector<String> urls;
	Time latest_update;
	
	
	Splitter splitter;
	ArrayCtrl valuelist, accountlist, orderlist, historylist;
	bool running = false, stopped = true;
	
	
	void RefreshHistory();
	void RefreshOpen();
	void FixOrders();
	void SolveSources();
	
public:
	typedef Myfxbook CLASSNAME;
	Myfxbook();
	~Myfxbook();
	
	void Add(String url, double profitability, double abs_gain) {urls.Add(url);}
	
	void Data();
	
	void Updater();
	
	void StoreThis() {StoreToFile(*this, ConfigFile("myfxbook.bin"));}
	void LoadThis() {LoadFromFile(*this, ConfigFile("myfxbook.bin"));}
	void Serialize(Stream& s) {s % symbols % accounts % urls % latest_update;}
};

}

#endif
