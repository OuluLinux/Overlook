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
	virtual ~ExpertAdvisor() {StoreCache();}
	
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
	int TimeMinute(const Time& t) {return t.minute;}
	int TimeHour(const Time& t) {return t.hour;}
	int TimeDay(const Time& t) {return t.day;}
	int TimeMonth(const Time& t) {return t.month;}
	int TimeYear(const Time& t) {return t.year;}
	int Year() {return Now.year;}
	int Month() {return Now.month;}
	int Day() {return Now.day;}
	int Hour() {return Now.hour;}
	int Minute() {return Now.minute;}
	int DayOfYear() {return Upp::DayOfYear(Now);}
	int DayOfWeek() {return Upp::DayOfWeek(Now);}
	void Comment(String s) {if (!avoid_refresh) ReleaseLog("COMMENT: " + s);}
	void Print(String s) {if (!avoid_refresh) ReleaseLog("PRINT: " + s);}
	void Alert(String s) {if (!avoid_refresh) ReleaseLog("ALERT: " + s);}
	int		OrderClose(int ticket, double lots, double price, int slippage) {return sb.OrderClose(ticket, lots, price, slippage);}
	double	OrderClosePrice() {return sb.OrderClosePrice();}
	Time	OrderCloseTime() {return sb.OrderCloseTime();}
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
	int		OrderSelect(int index, int select, int pool=libmt::MODE_TRADES) {return sb.OrderSelect(index, select, pool);}
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
	bool	IsOptimization() {return avoid_refresh;}
	bool	IsTesting() {return avoid_refresh;}
	bool	IsVisualMode() {return avoid_refresh;}
	double	MathMin(double a, double b) {return min(a, b);}
	double	MathMax(double a, double b) {return max(a, b);}
	double	MathAbs(double d) {return fabs(d);}
	double	MathPow(double d, double e) {return pow(d, e);}
	double	MathRound(double d) {return floor(d);}
	void	RefreshRates() {}
	String	GetLastError() {return sb.GetLastError();}
	bool	IsTradeContextBusy() {return false;}
	int		StringFind(const String& str, const String& match) {return str.Find(match);}
	String	StringSubstr(const String& str, int mid, int len) {return str.Mid(mid, len);}
	String	StringConcatenate(const String& a, const String& b) {return a + b;}
	
	double Ask, Bid, Point;
	int Digits, Bars;
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
	void PrintAlert(String str = "");
	bool OpenPositions1();
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






class ShockTime : public ExpertAdvisor {
	
	// Args
	int MMType = 2;
	int LotMultiplikator = 1667;
	int LotConst_or_not = true;
	int RiskPercent = 10.0;
	int Lot = 10;
	int TakeProfit = 5.0;
	int Step = 5.0;
	int MaxTrades = 30;
	int UseEquityStop = false;
	int TotalEquityRisk = 20.0;
	int ShowTableOnTesting = true;
	
	double max_lots = 0.0;
	double norm_digits = 2.0;
	double lots_mul;
	double slippage = 5.0;
	double lots;
	double tp_factor;
	double pips_factor = 0.0;
	double pips_limit0 = 10.0;
	double sl_factor0 = 10.0;
	double pip_step;
	bool use_trailing = false;
	bool no_pending_orders = false;
	int begin_hour = 2;
	int end_hour = 16;
	int Magic = 1111111;
	int magic;
	double price0;
	double price1;
	double bid_price0;
	double ask_price0;
	double last_buy_price;
	double last_sell_price;
	bool modify_order = false;
	int order_count_lots = 0;
	double lots1;
	int order_count;
	double price2 = 0.0;
	bool check_orders0 = false;
	bool check_orders1 = false;
	bool check_orders2 = false;
	int ret;
	bool orders_open = false;
	double equity1;
	double equity2;

	
public:
	typedef ShockTime CLASSNAME;
	ShockTime();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double ND(double d);
	int ForceOrderCloseMarket(bool close_buy = true, bool close_sell = true);
	double GetLots(int cmd);
	int CountTrades();
	void CloseThisSymbolAll();
	int OpenPendingOrder(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_unused_32, int ai_36, String a_comment_40, int a_magic_48, int a_datetime_52);
	double StopLong(double ad_0, int ai_8);
	double StopShort(double ad_0, int ai_8);
	double TakeLong(double ad_0, int ai_8);
	double TakeShort(double ad_0, int ai_8);
	double CalculateProfit();
	void TrailingAlls(int ai_0, int ai_4, double a_price_8);
	double AccountEquityHigh();
	double FindLastBuyPrice();
	double FindLastSellPrice();
	double GetProfitForDay(int ai_0);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("MMType", MMType, 0, 2)
			% Arg("LotMultiplikator", LotMultiplikator, 1000, 2000)
			% Arg("LotConst_or_not", LotConst_or_not, 0, 1)
			% Arg("RiskPercent", RiskPercent, 0, 100)
			% Arg("Lot", Lot, 0, 100)
			% Arg("TakeProfit", TakeProfit, 1, 100)
			% Arg("Step", Step, 1, 100)
			% Arg("MaxTrades", MaxTrades, 1, 1000)
			% Arg("UseEquityStop", UseEquityStop, 0, 1)
			% Arg("TotalEquityRisk", TotalEquityRisk, 0, 100)
			% Arg("ShowTableOnTesting", ShowTableOnTesting, 0, 1);
	}
	
};









class PipsBoss : public ExpertAdvisor {
	
	// args
	int TakeProfit = 45.0;
	int StopLoss = 55.0;
	int TrailingStop = 10.0;
	int user_lot_size = 100;
	int reversetrade = true;
	int counter_trade = true;
	int _time_shift = -7;
	int opt_active_time = 11;
	int Value_At_Risk = 1.0;
	int account_risk_control = true;
	int indicator1_check = 75.0;
	int indicator1_period = 14.0;
	int _trailing_stop_percentage = 87;
	int _mid_range_factor = 93;
	int price_deviation = 0.0;
	
	int pos = -1;
	int magic = 678914;
	int day_of_year_0;
	int day_of_year_1;
	int day_of_year_2;
	int day_of_year_3;
	int ticket;
	int sig;
	double hi;
	double lo;
	double tp_factor;
	int order_count;
	String A;
	String pair;
	bool do_trade;
	bool do_trade2;
	double g_ord_stoploss_276;
	double close_price;
	double trail_factor;
	
public:
	typedef PipsBoss CLASSNAME;
	PipsBoss();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double CalculateLotSize(double a_pips_0);
	int GetBoxSize();
	int GetInitialEntry();
	int GetReverseTrade();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("TakeProfit", TakeProfit, 1, 100)
			% Arg("StopLoss", StopLoss, 1, 100)
			% Arg("TrailingStop", TrailingStop, 1, 100)
			% Arg("user_lot_size", user_lot_size, 1, 100)
			% Arg("reversetrade", reversetrade, 0, 1)
			% Arg("counter_trade", counter_trade, 0, 1)
			% Arg("_time_shift", _time_shift, -11, 12)
			% Arg("opt_active_time", opt_active_time, -12, 12)
			% Arg("Value_At_Risk", Value_At_Risk, 0, 100)
			% Arg("account_risk_control", account_risk_control, 0, 1)
			% Arg("indicator1_check", indicator1_check, 0, 100)
			% Arg("indicator1_period", indicator1_period, 0, 100)
			% Arg("_trailing_stop_percentage", _trailing_stop_percentage, 0, 100)
			% Arg("_mid_range_factor", _mid_range_factor, 0, 100)
			% Arg("price_deviation", price_deviation, 0, 10);
	}
	
};





}

#endif
