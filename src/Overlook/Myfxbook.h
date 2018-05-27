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
		
		void Serialize(Stream& s) {s % history_orders % orders % url % id;}
	};
	
	
	VectorMap<int, Vector<int> > symbol_accounts;
	Index<String> allowed_symbols;
	Vector<Account> accounts;
	Vector<String> urls;
	
	
	Splitter splitter;
	ArrayCtrl accountlist, orderlist, historylist;
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
	void Serialize(Stream& s) {s % symbol_accounts % allowed_symbols % accounts % urls;}
};

}

#endif
