#ifndef _Overlook_Advisors_h_
#define _Overlook_Advisors_h_

namespace Overlook {

class ExpertAdvisor : public Core {
	
	struct TrainingCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	struct Setting : Moveable<Setting> {
		int start, stop, timeofday;
	};
	
	
	// Persistent
	Optimizer opt;
	Vector<double> training_pts;
	Vector<int> cursors;
	SimBroker sb;
	double total = 0;
	int round = 0;
	int prev_counted = 0;
	int input_size = 0;
	
	
	// Temporary
	TimeStop save_elapsed;
	Vector<double> tmp_mat;
	double point = 0.0001;
	int max_rounds = 0;
	int pos = 0;
	bool once = true;
	
protected:
	virtual void Start();
	
	bool TrainingBegin();
	bool TrainingIterator();
	bool TrainingEnd();
	bool TrainingInspect();
	void RefreshAll();
	void DumpTest();
	
public:
	typedef ExpertAdvisor CLASSNAME;
	ExpertAdvisor();
	~ExpertAdvisor() {StoreCache();}
	
	virtual void Init();
	
	virtual void InitEA() = 0;
	virtual void StartEA(int pos) = 0;
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<Avoidance>()
			% Lbl(1)
			% Mem(opt)
			% Mem(training_pts)
			% Mem(cursors)
			% Mem(sb)
			% Mem(total)
			% Mem(round)
			% Mem(prev_counted)
			% Mem(input_size)
			;
	}
	
	
public:
	String Symbol() {return GetSystem().GetSymbol(GetSymbol());}
	Time TimeCurrent() {return Now;}
	int TimeDay(const Time& t) {return t.day;}
	int TimeMonth(const Time& t) {return t.month;}
	int TimeYear(const Time& t) {return t.year;}
	void Comment(String s) {if (!avoid_refresh) ReleaseLog("COMMENT: " + s);}
	void Print(String s) {if (!avoid_refresh) ReleaseLog("PRINT: " + s);}
	void Alert(String s) {if (!avoid_refresh) ReleaseLog("ALERT: " + s);}
	int		OrderClose(int ticket, double lots, double price, int slippage) {return sb.OrderClose(ticket, lots, price, slippage);}
	double	OrderClosePrice() {return sb.OrderClosePrice();}
	int		OrderCloseTime() {return sb.OrderCloseTime();}
	String	OrderComment() {return sb.OrderComment();}
	double	OrderCommission() {return sb.OrderCommission();}
	int		OrderDelete(int ticket) {return sb.OrderDelete(ticket);}
	int		OrderExpiration() {return sb.OrderExpiration();}
	double	OrderLots() {return sb.OrderLots();}
	int		OrderMagicNumber() {return sb.OrderMagicNumber();}
	int		OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration) {return sb.OrderModify(ticket, price, stoploss, takeprofit, expiration);}
	double	OrderOpenPrice() {return sb.OrderOpenPrice();}
	int		OrderOpenTime() {return sb.OrderOpenTime();}
	double	OrderProfit() {return sb.OrderProfit();}
	int		OrderSelect(int index, int select, int pool) {return sb.OrderSelect(index, select, pool);}
	int		OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, String comment, int magic, int expiry=0) {return sb.OrderSend(symbol, cmd, volume, price, slippage, stoploss, takeprofit, magic, expiry);}
	int		OrdersHistoryTotal() {return sb.OrdersHistoryTotal();}
	double	OrderStopLoss() {return sb.OrderStopLoss();}
	int		OrdersTotal() {return sb.OrdersTotal();}
	double	OrderSwap() {return sb.OrderSwap();}
	String	OrderSymbol() {return sb.OrderSymbol();}
	double	OrderTakeProfit() {return sb.OrderTakeProfit();}
	int		OrderTicket() {return sb.OrderTicket();}
	int		OrderType() {return sb.OrderType();}
	String DoubleToStr(double d, int digits) {return Format("%" + IntStr(digits) + "!n", d);}
	double NormalizeDouble(double d, int digits) {
		int mul = 1; double div = 1.0;
		for(int i = 0; i < digits; i++) {mul *= 10; div *= 0.1;}
		int64 val = d * mul;
		return (double)val * div;
	}
	double	AccountInfoDouble(int property_id) {return sb.AccountInfoDouble(property_id);}
	int		AccountInfoInteger(int property_id) {return sb.AccountInfoInteger(property_id);}
	String	AccountInfoString(int property_id) {return sb.AccountInfoString(property_id);}
	double	AccountBalance() {return sb.AccountBalance();}
	double	AccountCredit() {return sb.AccountCredit();}
	String	AccountCompany() {return sb.AccountCompany();}
	String	AccountCurrency() {return sb.AccountCurrency();}
	double	AccountEquity() {return sb.AccountEquity();}
	double	AccountFreeMargin() {return sb.AccountFreeMargin();}
	double	AccountFreeMarginCheck(String symbol, int cmd, double volume) {return sb.AccountFreeMarginCheck(symbol, cmd, volume);}
	double	AccountFreeMarginMode() {return sb.AccountFreeMarginMode();}
	int		AccountLeverage() {return sb.AccountLeverage();}
	double	AccountMargin() {return sb.AccountMargin();}
	String	AccountName() {return sb.AccountName();}
	int		AccountNumber() {return sb.AccountNumber();}
	double	AccountProfit() {return sb.AccountProfit();}
	String	AccountServer() {return sb.AccountServer();}
	int		AccountStopoutLevel() {return sb.AccountStopoutLevel();}
	int		AccountStopoutMode() {return sb.AccountStopoutMode();}
	double	MarketInfo(String symbol, int type) {return sb.MarketInfo(symbol, type);}
	
	double Ask, Bid, Point;
	Time Now;
	
};





class HighPromises : public ExpertAdvisor {
	
	int MAGIC = 153465;
	int MAGIC2 = 253465;
	
	
	// Main params
	double Lots = 1.0;
	int StopLoss = 33;
	int TakeProfit = 8;
	
	// Buy params
	int stoch_k_period = 1;
	int stoch_d_period = 3;
	int stoch_slowing = 3;
	int stoch_buy_limit = 12;
	int cci_buy_period = 6;
	int cci_buy_limit = -60;
	int h4_limit = 60;
	int w3_limit = 0;
	
	
	// Sell params
	int stoch2_k_period = 2;
	int stoch2_d_period = 3;
	int stoch2_slowing = 3;
	int stoch_sell_limit = 90;
	int cci_sell_period = 4;
	int adx_sell_period = 16;
	int h12_limit = 10;
	int cci_sell_limit = 30;
	int adx0_shift = 0;
	int cci_shift = 0;
	
	
	// Money management params
	int use_mm = false;
	double max_risk = 0.1;
	int max_lots = 10;
	double decrease_factor = 1.0;
	
	// Management params
	bool use_ea_mgr = false;
	
	
public:
	typedef HighPromises CLASSNAME;
	HighPromises();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int Buy(int pos);
	int Sell(int pos);
	void OpenBuyOrder();
	void OpenSellOrder();
	void PrintAlert(String as_0 = "");
	bool OpenPositions();
	bool OpenPositions2();
	double LotsOptimized(int magic);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("StopLoss", StopLoss, 1, 30)
			% Arg("TakeProfit", TakeProfit, 1, 100)
			% Arg("stoch_k_period", stoch_k_period, 1, 10000)
			% Arg("stoch_d_period", stoch_d_period, 3, 10000)
			% Arg("stoch_slowing", stoch_slowing, 3, 10000)
			% Arg("stoch_buy_limit", stoch_buy_limit, 0, 100)
			% Arg("cci_buy_period", cci_buy_period, 2, 10000)
			% Arg("cci_buy_limit", cci_buy_limit, -150, +150)
			% Arg("h4_limit", h4_limit, -100, +100)
			% Arg("w3_limit", w3_limit, -100, +100)
			% Arg("stoch2_k_period", stoch2_k_period, 2, 10000)
			% Arg("stoch2_d_period", stoch2_d_period, 2, 10000)
			% Arg("stoch2_slowing", stoch2_slowing, 2, 10000)
			% Arg("stoch_sell_limit", stoch_sell_limit, 0, 100)
			% Arg("cci_sell_period", cci_sell_period, 2, 10000)
			% Arg("adx_sell_period", adx_sell_period, 2, 10000)
			% Arg("h12_limit", h12_limit, -100, +100)
			% Arg("cci_sell_limit", cci_sell_limit, -150, +150)
			% Arg("adx0_shift", adx0_shift, 0, 10000);
	}
	
};


}

#endif
