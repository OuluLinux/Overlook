#ifndef _Overlook_Martingale_h_
#define _Overlook_Martingale_h_

namespace Overlook {
using namespace libmt;


class Martingale : public Common {
	
protected:
	friend class MartingaleCtrl;
	
	struct Order : Moveable<Order> {
		int symbol = -1;
		int ticket;
		int type;
		double lots;
		double open;
		double stop_loss = 0.0;
		double take_profit = 0.0;
	};
	
	enum {LOTS, LOTEXP, TP, SL, PIPSTEP, TRAILSTART, TRAILSTOP, MAXTRADES, USETRAIL, ARG_COUNT};
	
	struct NetSetting : Moveable<NetSetting> {
		double lots = 0.01;
		double lot_exponent = 2;
		int take_profit = 10;
		int stop_loss = 500;
		int pip_step = 30;
		int trail_start = 10;
		int trail_stop = 10;
		int max_trades = 10;
		bool use_trailing_stop = true;
		
		double average_price;
		int slippage = 3;
		bool use_timeout = false;
		
		Vector<Order> orders;
		CoreList cl_net, cl_rsi;
		double equity, balance;
		int pos;
		int tickets = 0;
		
		void ProcessNet();
		void TrailingAlls();
		void CheckStops();
		void RefreshEquity();
		double Ask() {return cl_net.GetBuffer(0, 0, 0).Get(pos) + 0.0003;}
		double Bid() {return cl_net.GetBuffer(0, 0, 0).Get(pos);}
		int CountTrades();
		double FindLastBuyPrice();
		double FindLastSellPrice();
		double Open(int rel_pos) {return cl_net.GetBuffer(0, 0, 0).Get(Upp::max((int)0, (int)(pos - rel_pos)));}
		double Point() {return 0.0001;}
		double RSI() {return cl_rsi.GetBuffer(0, 0, 0).Get(pos);}
		double AccountEquity() {return equity;}
		double NormalizeDouble(double d, int decimal) {return d;}
		int OpenPendingOrder(int pType, double pLots, double pPrice, int pSlippage, double ad_24, int ai_32, int ai_36);
		void OrderModify(Order& o, double price, double stoploss, double takeprofit);
		void CheckPendingOrders();
		double StopLong(double ad_0, int ai_8) {if (ai_8 == 0) return (0);else return (ad_0 - ai_8 * Point());}
		double StopShort(double ad_0, int ai_8) {if (ai_8 == 0) return (0);else return (ad_0 + ai_8 * Point());}
		double TakeLong(double ad_0, int ai_8) {if (ai_8 == 0) return (0);else return (ad_0 + ai_8 * Point());}
		double TakeShort(double ad_0, int ai_8) {if (ai_8 == 0) return (0);else return (ad_0 - ai_8 * Point());}
		
	};
	
	// Persistent
	Optimizer opt;
	Vector<double> training_pts;
	
	
	// Temporary
	Vector<NetSetting> nets;
	Vector<double> points;
	Vector<Order> orders;
	CoreList cl_sym;
	double balance = 0;
	int max_rounds = 0;
	int pos;
	
	
	
	void SetSymbolLots(int sym, double lots);
	void SetRealSymbolLots(int sym, double lots);
	void CloseOrder(int i, double lots);
	void OpenOrder(int sym, int type, double lots);
	void LoadThis() {LoadFromFile(*this, ConfigFile("Martingale.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("Martingale.bin"));}
	
public:
	typedef Martingale CLASSNAME;
	Martingale();
	
	virtual void Init();
	virtual void Start();
	void Process();
	
	
	void Serialize(Stream& s) {s % opt % training_pts;}
};

inline Martingale& GetMartingale() {return GetSystem().GetCommon<Martingale>();}

class MartingaleCtrl : public CommonCtrl {
	
	struct MartCtrl : public Ctrl {
		Vector<Point> cache;
		virtual void Paint(Draw& d) {
			d.DrawRect(GetSize(), White());
			DrawVectorPolyline(d, GetSize(), GetMartingale().training_pts, cache);
		}
	};
	
	MartCtrl ctrl;
	ProgressIndicator prog;
	
public:
	typedef MartingaleCtrl CLASSNAME;
	MartingaleCtrl();
	
	virtual void Data();
};


}


#endif
