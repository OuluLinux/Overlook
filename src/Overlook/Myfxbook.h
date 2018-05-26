#ifndef _Overlook_Myfxbook_h_
#define _Overlook_Myfxbook_h_


namespace Overlook {

class Myfxbook : public ParentCtrl {
	
	struct Order : Moveable<Order> {
		String time, symbol, action, units, open, profit;
		double lots = 0.0;
	};
	
	struct Account : Moveable<Account> {
		Vector<Order> history_orders, orders;
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
