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
	};
	
	struct Account : Moveable<Account> {
		Array<Order> history_orders, orders;
		String url, id;
	};
	
	
	Index<String> allowed_symbols;
	Vector<Account> accounts;
	Vector<String> urls;
	bool running = false, stopped = true;
	
	Splitter splitter;
	ArrayCtrl accountlist, orderlist, historylist;
	
	void RefreshHistory();
	void RefreshOpen();
	void FixOrders();
	
	
public:
	typedef Myfxbook CLASSNAME;
	Myfxbook();
	~Myfxbook();
	
	void Add(String url, double profitability, double abs_gain) {urls.Add(url);}
	
	void Data();
	
	void Updater();
	
};

}

#endif
