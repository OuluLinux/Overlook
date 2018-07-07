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
	int Period() {return GetMinutePeriod();}
	Time TimeCurrent() {return Now;}
	int TimeMinute(const Time& t) {return t.minute;}
	int TimeHour(const Time& t) {return t.hour;}
	int TimeDay(const Time& t) {return t.day;}
	int TimeMonth(const Time& t) {return t.month;}
	int TimeYear(const Time& t) {return t.year;}
	int TimeDayOfWeek(const Time& t) {return Upp::DayOfWeek(t);}
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
	int		OrderModify(int ticket, double price, double stoploss, double takeprofit, Time expiration=Time(3000,1,1)) {return sb.OrderModify(ticket, price, stoploss, takeprofit, expiration);}
	double	OrderOpenPrice() {return sb.OrderOpenPrice();}
	Time	OrderOpenTime() {return sb.OrderOpenTime();}
	double	OrderProfit() {return sb.OrderProfit();}
	int		OrderSelect(int index, int select, int pool=libmt::MODE_TRADES) {return sb.OrderSelect(index, select, pool);}
	int		OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, String comment, int magic, int expiry=0) {return sb.OrderSend(symbol, cmd, volume, price, slippage, stoploss, takeprofit, comment, magic, expiry);}
	int		OrdersHistoryTotal() {return sb.OrdersHistoryTotal();}
	double	OrderStopLoss() {return sb.OrderStopLoss();}
	int		OrdersTotal() {return sb.OrdersTotal();}
	int		HistoryTotal() {return sb.HistoryTotal();}
	double	OrderSwap() {return sb.OrderSwap();}
	String	OrderSymbol() {return sb.OrderSymbol();}
	double	OrderTakeProfit() {return sb.OrderTakeProfit();}
	int		OrderTicket() {return sb.OrderTicket();}
	int		OrderType() {return sb.OrderType();}
	bool	IsOrderSelected() {return sb.IsOrderSelected();}
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
	double	MathFloor(double d) {return floor(d);}
	void	RefreshRates() {}
	String	GetLastError() {return sb.GetLastError();}
	bool	IsTradeContextBusy() {return false;}
	int		StringFind(const String& str, const String& match) {return str.Find(match);}
	int		StringFind(const String& str, const String& match, int begin) {return str.Find(match, begin);}
	String	StringSubstr(const String& str, int mid, int len) {return str.Mid(mid, len);}
	String	StringConcatenate(const String& a, const String& b) {return a + b;}
	int		StringLen(const String& a) {return a.GetCount();}
	void	Sleep(int ms) {}
	
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




class Snake : public ExpertAdvisor {
	
	// Args
	int Dynamic_lot = 20.0;
	int lim_step = 110;
	int lim_setup = 5;
	int Op_step = 20;
	int period = 36;
	
	int MagicNumber = 131254;
	double minlots = 0.01;
	int count = 0;

public:
	typedef Snake CLASSNAME;
	Snake();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	void CloseBuy();
	void CloseSell();
	void CloseLimit();
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("Dynamic_lot", Dynamic_lot, 1, 100)
			% Arg("lim_step", lim_step, 1, 100)
			% Arg("lim_setup", lim_setup, 1, 100)
			% Arg("Op_step", Op_step, 1, 100)
			% Arg("period", period, 1, 200);
			
	}
	
};






class ChannelScalper : public ExpertAdvisor {
	
	// Args
	int timeframe1period = 3;
	int bb_period = 20;
	int bb_deviation = 2;
	int basketpercent = false;
	int reversesignals = false;
	int hidestop = false;
	int hidetarget = true;
	int buystop = 0;
	int buytarget = 11;
	int sellstop = 0;
	int selltarget = 11;
	int trailingstart = 0;
	int trailingstop = 0;
	int trailingstep = 1;
	int breakevengain = 0;
	int breakeven = 0;
	int profit = 10.0;
	int loss = 30.0;
	
	double lots = 0.01;
	double lotdigits = 2.0;
	bool oppositeclose = true;
	int maxtrades = 100;
	int tradesperbar = 1;
	int slippage = 5;
	
	bool lotsoptimized = true;
	double risk = 1.0;
	double minlot = 0.01;
	double maxlot = 10.0;
	
	Time gt_unused_396;
	Time gt_unused_400;
	Time g_datetime_404;
	Time g_datetime_408;
	double equity_diff = 0.0;
	double g_ord_open_price_420 = 0.0;
	double g_ord_open_price_428 = 0.0;
	double g_price_436;
	double g_price_444;
	double price_point;
	double point_factor;
	double max_equity_diff;
	double min_equity_diff;
	int magic = 1234;
	int g_digits_500;
	int g_bars_504 = -1;
	int g_count_508 = 0;
	int g_ord_total_512;
	int g_ticket_516;
	int buy_count = 0;
	int sell_count = 0;
	int order_count = 0;
	int gi_unused_532 = 0;
	int gi_unused_536 = 0;

public:
	typedef ChannelScalper CLASSNAME;
	ChannelScalper();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int count(int cmd, int a_magic_4);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("timeframe1period", timeframe1period, 2, 100)
			% Arg("bb_period", bb_period, 2, 100)
			% Arg("bb_deviation", bb_deviation, 1, 3)
			% Arg("basketpercent", basketpercent, 0, 1)
			% Arg("reversesignals", reversesignals, 0, 1)
			% Arg("hidestop", hidestop, 0, 1)
			% Arg("hidetarget", hidetarget, 0, 1)
			% Arg("buystop", buystop, 0, 30)
			% Arg("buytarget", buytarget, 0, 30)
			% Arg("sellstop", sellstop, 0, 30)
			% Arg("selltarget", selltarget, 0, 30)
			% Arg("trailingstart", trailingstart, 0, 30)
			% Arg("trailingstop", trailingstop, 0, 30)
			% Arg("trailingstep", trailingstep, 0, 30)
			% Arg("breakevengain", breakevengain, 0, 30)
			% Arg("breakeven", breakeven, 0, 30)
			% Arg("profit", profit, 0, 30)
			% Arg("loss", loss, 10, 60);
	}
	
};












class RedSunrise : public ExpertAdvisor {
	
	// Args
	int     MMType = 1;
	int     TakeProfit = 10;
	int     Stoploss = 500;
	int     TrailStart = 10;
	int     TrailStop = 10;
	int     PipStep = 30;
	int     MaxTrades = 10;
	bool    UseEquityStop = false;
	int     TotalEquityRisk = 20; //loss as a percentage of equity
	bool    UseTrailingStop = false;
	int     MaxTradeOpenHours = 48;
	
	bool    UseClose = false;
	bool    UseAdd = true;
	double  LotExponent = 1.667;
	double  slip = 3;
	double  Lots = 0.1;
	double  LotsDigits = 2;
	
	int MagicNumber = 12324;
	double PriceTarget, StartEquity, BuyTarget, SellTarget;
	double AveragePrice, SellLimit, BuyLimit;
	double LastBuyPrice, LastSellPrice, ClosePrice, Spread;
	int flag;
	String EAName = "Ilan1/4";
	int NumOfTrades = 0;
	double iLots;
	int cnt = 0, total;
	double Stopper = 0;
	bool TradeNow = false, LongTrade = false, ShortTrade = false;
	int ticket;
	bool NewOrdersPlaced = false;

public:
	typedef RedSunrise CLASSNAME;
	RedSunrise();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double ND(double v);
	int ForceOrderCloseMarket(bool aCloseBuy = true, bool aCloseSell = true);
	double GetLots(int aTradeType);
	int CountTrades();
	void CloseThisSymbolAll();
	int OpenPendingOrder(int pType, double pLots, double pLevel, int sp, double pr, int sl, int tp, String pComment, int pMagic);
	double StopLong(double price, int stop);
	double StopShort(double price, int stop);
	double TakeLong(double price, int take);
	double TakeShort(double price, int take);
	double CalculateProfit();
	void TrailingAlls(int start, int stop, double AvgPrice);
	double AccountEquityHigh();
	double FindLastBuyPrice();
	double FindLastSellPrice();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("MMType", MMType, 0, 2)
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("Stoploss", Stoploss, 0, 1000)
			% Arg("TrailStart", TrailStart, 0, 100)
			% Arg("TrailStop", TrailStop, 0, 100)
			% Arg("PipStep", PipStep, 0, 100)
			% Arg("MaxTrades", MaxTrades, 0, 100)
			% Arg("TotalEquityRisk", TotalEquityRisk, 0, 100);
	}
	
};







class Eagle : public ExpertAdvisor {
	
	// Args
	int macd_fast_ema = 24;
	int macd_slow_ema = 52;
	int macd_signal_sma = 18;
	int stoch0_k_period = 3;
	int stoch0_d_period = 3;
	int stoch0_slowing = 2;
	int stoch1_k_period = 8;
	int stoch1_d_period = 3;
	int stoch1_slowing = 2;
	int sar_step = 20;
	int sar_max = 30;
	int mom_period = 14;
	int sl_factor = 900;
	int tp_factor = 3;
	int tp_limit = 9;
	int sl_limit = 1;
	int use_macd = false;
	int use_momentum = true;
	int use_sar = true;
	int use_stoch = true;
	int use_stoch2 = false;
	int stoch_hi_limit = 55.0;
	int stoch_lo_limit = 24.0;
	int mom_limit = 100.0;
	int use_trailing = true;
	
	int MagicNumber = 853953;
	double Lots = 0.1;
	bool Moneymanagement = true;
	double MaximumRisk = 0.2;
	int Slippage = 10;
	bool Fridaymode = false;
	double MaxTradesize = 100.0;
	
	bool micro_lots = false;
	double lot_divider = 1.0;
	bool use_mm_lots = true;
	int mm_lots_divider = 3;
	bool use_orders = true;
	int g_price_field_268 = 0;
	int indi_shift = 0;
	int g_timeframe_308 = 0;
	double g_imacd_312;
	double g_imacd_320;
	double g_istochastic_336;
	double g_istochastic_344;
	double g_istochastic_352;
	double g_istochastic_360;
	double g_isar_368;
	double g_isar_376;
	double g_imomentum_384;
	String gs_dummy_400;
	int gi_408;

public:
	typedef Eagle CLASSNAME;
	Eagle();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	bool CheckExitCondition(String as_unused_0);
	bool CheckEntryConditionBUY(int pos);
	bool CheckEntryConditionSELL(int pos);
	void OpenBuyOrder();
	void OpenSellOrder();
	int openPositions();
	void prtAlert(String as_0 = "");
	void CloseOrder(int a_ticket_0, double a_lots_4, double a_price_12);
	void TrailingPositions();
	void ModifyStopLoss(double a_price_0);
	int HandleOpenPositions1();
	double GetMoneyManagement();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("macd_fast_ema", macd_fast_ema, 2, 100)
			% Arg("macd_slow_ema", macd_slow_ema, 2, 100)
			% Arg("macd_signal_sma", macd_signal_sma, 2, 100)
			% Arg("stoch0_k_period", stoch0_k_period, 2, 100)
			% Arg("stoch0_d_period", stoch0_d_period, 2, 100)
			% Arg("stoch0_slowing", stoch0_slowing, 2, 100)
			% Arg("stoch1_k_period", stoch1_k_period, 2, 100)
			% Arg("stoch1_d_period", stoch1_d_period, 2, 100)
			% Arg("stoch1_slowing", stoch1_slowing, 2, 100)
			% Arg("sar_step", sar_step, 1, 100)
			% Arg("sar_max", sar_max, 1, 100)
			% Arg("mom_period", mom_period, 2, 100)
			% Arg("sl_factor", sl_factor, 1, 1000)
			% Arg("tp_factor", tp_factor, 1, 100)
			% Arg("tp_limit", tp_limit, 1, 100)
			% Arg("sl_limit", sl_limit, 1, 100)
			% Arg("use_macd", use_macd, 0, 1)
			% Arg("use_momentum", use_momentum, 0, 1)
			% Arg("use_sar", use_sar, 0, 1)
			% Arg("use_stoch", use_stoch, 0, 1)
			% Arg("use_stoch2", use_stoch2, 0, 1)
			% Arg("stoch_hi_limit", stoch_hi_limit, 0, 100)
			% Arg("stoch_lo_limit", stoch_lo_limit, 0, 100)
			% Arg("mom_limit", mom_limit, -200, 200)
			% Arg("use_trailing", use_trailing, 0, 1);
	}
	
};




class Salmon : public ExpertAdvisor {
	
	// Args
	int SL2BUY = 200;
	int SL2SELL = 200;
	int tp_factor1 = 10;
	int tp_factor2 = 10;
	int sl_factor1 = 45;
	int sl_factor2 = 55;
	
	double Lot1Size = 0.1;
	int NewYorkShift = 0;
	int Slippage = 2;
	int Magic = 2024;
	String gs_eurusd_116 = "EURUSD";
	int point_mult = 1;
	int point_mode = 4;
	int ny_hour = 15;
	int ny_hour2 = 18;
	int ny_hour3 = 15;
	int ny_hour4 = 18;
	bool use_orders = true;
	bool use_orders_lotlimit = false;
	bool use_orders_lotlimit2 = false;
	double lots0 = 5.0;
	int order_tries = 10;
	bool alert_fails = true;
	int soundid;
	bool some_fail = false;
	Time g_datetime_232;
	double g_bid_240 = 0.0;
	double g_ask_248 = 0.0;
	double g_point_256 = 0.0;
	int gi_unused_264;
	Time g_datetime_268;
	Time g_datetime_272;
	Time g_datetime_276;
	bool time_bool0 = false;
	bool time_bool1 = false;
	bool gi_unused_288 = true;
	bool gi_unused_292 = false;
	bool gi_unused_296 = false;
	bool gi_unused_324 = false;
	bool use_order_limit0 = true;
	bool gi_unused_332 = false;
	bool open_orders_arg0 = false;
	bool order_fail_value = false;
	bool gi_unused_344 = false;
	bool gi_unused_348 = false;
	bool order_type1 = false;
	bool order_type2 = false;


public:
	typedef Salmon CLASSNAME;
	Salmon();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	void vSetOrderForReal(String a_symbol_0, int a_cmd_8, double a_lots_12, double a_price_20, double a_price_28, double a_price_36, int a_magic_44);
	void vSetOrderForTest(String a_symbol_0, int a_cmd_8, double a_lots_12, double a_price_20, double a_price_28, double a_price_36, int a_magic_44);
	int iOpenPosition(String a_symbol_0, int a_cmd_8, double a_lots_12, int ai_unused_20, int ai_24, int ai_28, int a_slippage_32, int ai_36, bool ai_40, double a_price_52 = 0.0, double a_price_60 = 0.0, int a_magic_68 = 0);
	void vOpenPosition(String a_symbol_0, int a_cmd_8, double a_lots_12, int a_slippage_28, double a_price_32, double a_price_40, int a_magic_48 = 0);
	bool bExistPositions(String as_0, int a_cmd_8, int a_magic_12, Time ai_16=Time(1970,1,1));
	String sGetNameOP(int ai_0);
	int clFuncC(bool ai_0, int ai_4, int ai_8);
	int bIsCloseLastPosByStop(String a_symbol_0, int a_cmd_8, int a_magic_12);
	void vDeleteOrders(String as_0, int a_cmd_8, int a_magic_12);
	double dGetLotLastClosePos(String as_0, int a_cmd_8, int a_magic_12);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("SL2BUY", SL2BUY, 10, 400)
			% Arg("SL2SELL", SL2SELL, 10, 400)
			% Arg("tp_factor1", tp_factor1, 2, 100)
			% Arg("tp_factor2", tp_factor2, 2, 100)
			% Arg("sl_factor1", sl_factor1, 2, 100)
			% Arg("sl_factor2", sl_factor2, 2, 100)
			;
	}
	
};




class SoundOfForex : public ExpertAdvisor {
	
	// args
	int risk = 4.0;
	int frac_left_bars = 3;
	int frac_right_bars = 0;
	int macd_fast_ema = 5;
	int macd_slow_ema = 13;
	int macd_signal_sma_period = 1;
	int use_manual_sl = false;
	int use_manual_tp = false;
	int use_martingale = true;
	int use_orderclose0 = false;
	int use_macdclose0 = false;
	int slsl_factor1 = 100;
	int sl_factor0 = 300;
	int tp_factor0 = 500;
	int sl_factor1 = 300;
	int tp_factor1 = 500;
	int tp_factor2 = 10;
	int sl_factor2 = 10;
	int sl_factor3 = 1;
	int sl_factor4 = 0;
	int tp_factor4 = 0;
	
	double lots = 0.01;
	int lotdigits = 2;
	int gmtshift = 2;
	int magic = 2370;
	double g_lots_112 = 0.01;
	double max_lots = 10.0;
	int max_countsum = 100;
	int expiration_hours = 255;
	int slippage_factor = 5;
	int macd_shift = 0;
	bool use_checktime = true;
	int begin_hour = 0;
	int end_hour = 19;
	bool skip_sunday = true;
	bool skip_friday = false;
	int end_hour1 = 24;
	Time gt_unused_248;
	Time gt_unused_252;
	Time g_datetime_256;
	Time g_datetime_260;
	double gd_unused_264 = 0.0;
	double g_ord_open_price_272 = 0.0;
	double g_ord_open_price_280 = 0.0;
	double tp0;
	double sl0;
	double point0;
	double gd_312;
	double g_ord_profit_336;
	int g_pos_344;
	int g_pos_348;
	int g_digits_352;
	int g_bars_356 = -1;
	int g_count_360 = 0;
	int g_ord_total_364;
	int g_ticket_368;
	int buy_count = 0;
	int sell_count = 0;
	int total_count = 0;
	int gi_unused_384 = 0;
	int gi_unused_388 = 0;
	double mart_lots0 = 1.0;
	double mart_lots1;
	double mart_lots2 = 1.0;
	int begin_hour1;

public:
	typedef SoundOfForex CLASSNAME;
	SoundOfForex();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int count(int cmd, int a_magic_4);
	int martingalefactor();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("risk", risk, 1, 10)
			% Arg("frac_left_bars", frac_left_bars, 0, 100)
			% Arg("macd_fast_ema", macd_fast_ema, 0, 100)
			% Arg("macd_slow_ema", macd_slow_ema, 0, 100)
			% Arg("macd_signal_sma_period", macd_signal_sma_period, 0, 100)
			% Arg("use_manual_sl", use_manual_sl, 0, 1)
			% Arg("use_manual_tp", use_manual_tp, 0, 1)
			% Arg("use_martingale", use_martingale, 0, 1)
			% Arg("use_orderclose0", use_orderclose0, 0, 1)
			% Arg("use_macdclose0", use_macdclose0, 0, 1)
			% Arg("slsl_factor1", use_macdclose0, 0, 1000)
			% Arg("sl_factor0", sl_factor0, 0, 1000)
			% Arg("tp_factor0", tp_factor0, 0, 1000)
			% Arg("sl_factor1", sl_factor1, 0, 1000)
			% Arg("tp_factor1", tp_factor1, 0, 1000)
			% Arg("sl_factor2", sl_factor2, 0, 1000)
			% Arg("tp_factor2", tp_factor2, 0, 1000)
			% Arg("sl_factor3", sl_factor3, 0, 1000)
			% Arg("sl_factor4", sl_factor4, 0, 1000)
			% Arg("tp_factor4", tp_factor4, 0, 1000)
			;
	}
	
};




class Explorer : public ExpertAdvisor {
	
	// Args
	int stepslow = 5;
	int maximumslow = 5;
	int stepfast = 20;
	int maximumfast = 20;
	int barsearch = 3;
	double Lots = 0.1;
	bool RiskManagement = false;
	double RiskPercent = 10.0;
	int timecontrol = 0;
	int starttime = 7;
	int stoptime = 17;
	int BBUSize = 0;
	int TrailingStop = 0;
	int TrailingShag = 5;
	int otstup = 100;
	double Ur1 = 50.0;
	double Ur2 = 161.0;
	
	int gi_unused_176;
	int gi_unused_180;
	int g_bars_184;
	int ret0;
	int pos;
	bool ret1;
	bool ret2;

	
public:
	typedef Explorer CLASSNAME;
	Explorer();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int ScalpParabolicPattern(int pos);
	int deletelimitorder(int ai_0);
	int ChLimitOrder(int ai_0);
	double MaximumMinimum(int ai_0, int ai_4);
	double GetFiboUr(double ad_0, double ad_8, double ad_16);
	int GetTrailingStop();
	int BBU();
	int GetTimeControl(int ai_0, int ai_4);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("stepslow", stepslow, 1, 100)
			% Arg("maximumslow", maximumslow, 1, 100)
			% Arg("stepfast", stepfast, 1, 100)
			% Arg("maximumfast", maximumfast, 1, 100)
			% Arg("barsearch", barsearch, 1, 100)
			;
	}
	
};









class DoubleDecker : public ExpertAdvisor {
	
	int ch_period = 80;
	int TrailingStop=0;
	int TakeProfit=0;
	int InitialStopLoss=40;
	int BeginSession1=6;
	int EndSession1=10;
	int BeginSession2=10;
	int EndSession2=14;
	
	double Lots=1;
	double Slippage=3;
	
public:
	typedef DoubleDecker CLASSNAME;
	DoubleDecker();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int OrderSendExtended(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, String comment, int magic);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("ch_period", ch_period, 3, 200)
			% Arg("TrailingStop", TrailingStop, 0, 100)
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("InitialStopLoss", InitialStopLoss, 0, 100)
			% Arg("BeginSession1", BeginSession1, 0, 23)
			% Arg("EndSession1", EndSession1, 0, 23)
			% Arg("BeginSession2", BeginSession2, 0, 23)
			% Arg("EndSession2", EndSession2, 0, 23)
			;
	}
	
};


class Mari : public ExpertAdvisor {
	
	//Args
	int ch_period = 10;
	int OrdersCount = 10;
	int Ord_1_StopLoss = 50;
	int Ord_2_Level = 50.0;
	int Ord_2_StopLoss = 50;
	int Ord_2_TakeProfit = 50;
	int Ord_3_Level = 50.0;
	int Ord_3_StopLoss = 50;
	int Ord_3_TakeProfit = 50;
	int Ord_4_Level = 50.0;
	int Ord_4_StopLoss = 50;
	int Ord_4_TakeProfit = 50;
	int Ord_5_Level = 50.0;
	int Ord_5_StopLoss = 50;
	int Ord_5_TakeProfit = 50;
	int Ord_6_Level = 50.0;
	int Ord_6_StopLoss = 50;
	int Ord_6_TakeProfit = 50;
	int Ord_7_Level = 50.0;
	int Ord_7_StopLoss = 50;
	int Ord_7_TakeProfit = 50;
	int Ord_8_Level = 50.0;
	int Ord_8_StopLoss = 50;
	int Ord_8_TakeProfit = 50;
	int Ord_9_Level = 50.0;
	int Ord_9_StopLoss = 50;
	int Ord_9_TakeProfit = 50;
	int Ord_10_Level = 50.0;
	int Ord_10_StopLoss = 50;
	int Ord_10_TakeProfit = 50;
	
	bool CanOpenNew = true;
	int TimeFrame = 60;
	bool Send_EMail = false;
	bool Show_Alerts = true;
	int Magic_N = 555;
	bool InfoOn = true;
	double Ord_10_Lots = 9.6;
	double Ord_9_Lots = 9.6;
	double Ord_8_Lots = 4.8;
	double Ord_7_Lots = 2.4;
	double Ord_6_Lots = 1.2;
	double Ord_5_Lots = 0.6;
	double Ord_4_Lots = 0.3;
	double Ord_3_Lots = 0.1;
	double Ord_2_Lots = 0.1;
	double Ord_1_Lots = 0.1;
	int Ord_1_TakeProfit = 50;
	int Slippage = 3;
	
	int gi_436 = 16748574;
	int end_hour0 = 24;
	int point_factor0 = 50;
	bool verbose = true;
	int g_ticket_460;
	int g_error_464;
	double g_price_472;
	double g_price_480;
	double g_price_488;
	String g_comment_496;
	Time ltt;
	bool gba_508[10];
	bool gba_512[10];
	bool gba_516[10];
	bool gba_520[10];
	bool gba_524[10];
	bool gba_528[10];
	bool gba_532[10];
	bool gba_536[10];
	double gda_540[10];
	double gda_544[10];
	int gia_548[10];
	double gda_552[10];
	int gia_556[10];
	int gia_560[10];
	double min_tp;
	double min_sl;
	double max_sl;
	double max_tp;
	double alarm_ask;
	
public:
	typedef Mari CLASSNAME;
	Mari();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	void Alarm2(int ai_0, int ai_4, int ai_8, double ad_12, double ad_20);
	double ND(double d);
	int OrderIndex(String as_0);
	void AccountAndSymbolLbls();
	String MarketType(String as_0);
	double FuturesLotMargin(String as_0);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("ch_period", ch_period, 1, 100)
			% Arg("OrdersCount", OrdersCount, 2, 100)
			% Arg("Ord_1_StopLoss", Ord_1_StopLoss, 5, 100)
			% Arg("Ord_2_Level", Ord_2_Level, 5, 100)
			% Arg("Ord_2_StopLoss", Ord_2_StopLoss, 5, 100)
			% Arg("Ord_2_TakeProfit", Ord_2_TakeProfit, 5, 100)
			% Arg("Ord_3_Level", Ord_3_Level, 5, 100)
			% Arg("Ord_3_StopLoss", Ord_3_StopLoss, 5, 100)
			% Arg("Ord_3_TakeProfit", Ord_3_TakeProfit, 5, 100)
			% Arg("Ord_4_Level", Ord_4_Level, 5, 100)
			% Arg("Ord_4_StopLoss", Ord_4_StopLoss, 5, 100)
			% Arg("Ord_4_TakeProfit", Ord_4_TakeProfit, 5, 100)
			% Arg("Ord_5_Level", Ord_5_Level, 5, 100)
			% Arg("Ord_5_StopLoss", Ord_5_StopLoss, 5, 100)
			% Arg("Ord_5_TakeProfit", Ord_5_TakeProfit, 5, 100)
			% Arg("Ord_6_Level", Ord_6_Level, 5, 100)
			% Arg("Ord_6_StopLoss", Ord_6_StopLoss, 5, 100)
			% Arg("Ord_6_TakeProfit", Ord_6_TakeProfit, 5, 100)
			% Arg("Ord_7_Level", Ord_7_Level, 5, 100)
			% Arg("Ord_7_StopLoss", Ord_7_StopLoss, 5, 100)
			% Arg("Ord_7_TakeProfit", Ord_7_TakeProfit, 5, 100)
			% Arg("Ord_8_Level", Ord_8_Level, 5, 100)
			% Arg("Ord_8_StopLoss", Ord_8_StopLoss, 5, 100)
			% Arg("Ord_8_TakeProfit", Ord_8_TakeProfit, 5, 100)
			% Arg("Ord_9_Level", Ord_9_Level, 5, 100)
			% Arg("Ord_9_StopLoss", Ord_9_StopLoss, 5, 100)
			% Arg("Ord_9_TakeProfit", Ord_9_TakeProfit, 5, 100)
			% Arg("Ord_10_Level", Ord_10_Level, 5, 100)
			% Arg("Ord_10_StopLoss", Ord_10_StopLoss, 5, 100)
			% Arg("Ord_10_TakeProfit", Ord_10_TakeProfit, 5, 100);
	}
	
};




class Pyramid : public ExpertAdvisor {
	
	// Args
	int     Step = 120;
	double  FirstLot = 0.1;
	double  IncLot = 0;
	double  MinProfit = 450;
	int     Magic = 2008;
	
	double gLotSell = 0;
	double gLotBuy = 0;
	double LSP, LBP;

public:
	typedef Pyramid CLASSNAME;
	Pyramid();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int DeletePendingOrders(int Magic);
	int CloseOrders(int Magic);
	int MyOrdersTotal(int Magic);
	double GetLastBuyPrice(int Magic);
	double GetLastSellPrice(int Magic);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("Step", Step, 10, 300);
		
	}
	
};








class SuperMart : public ExpertAdvisor {
	
	
	String gs_80 = "FRX_";
	String gsa_88[7];
	bool gi_unused_92 = true;
	bool gi_96 = false;
	double gd_100 = 0.01;
	
	// Args
	int RiskPercent = 2.0;
	int ProtectionTP = 7;
	
	int TorgSloy = 3;
	int N_enable_Sloy = 7;
	double LotMultiplicator = 1.2;
	int hSETKY = 30;
	int Uvel_hSETKY = 1;
	int ShagUvel_hSETKY = 2;
	int Magic = 1230;
	bool ShowTableOnTesting = true;
	
	double g_maxlot_148 = 0.0;
	int gi_232 = 7;
	int gi_unused_236 = 1;
	double gd_unused_240 = 40.0;
	int gi_212 = 0;
	int gi_216 = 0;
	int gi_unused_184 = 30;
	
	String gs_unused_280 = "Îãð.òîðã.ïî ñâîá.ñð.ñ÷åòà";
	String gs_unused_288 = "â % îò äåïîçèòà";
	double gd_296 = 3.0;
	int gi_304 = 3;
	int g_slippage_316 = 2;
	double gd_320;
	double gd_328;
	double g_ask_344;
	double g_bid_352;
	double gd_360;
	double gd_368;
	int gia_376[7];
	int gia_unused_380[7];
	double gd_384;
	int gia_392[3][2] = {16711680, 255,
						15570276, 7504122,
						13959039, 17919
						};
	String gsa_396[3][2] = {"NextBUY_0", "NextSELL_0",
			"NextBUY_1", "NextSELL_1",
			"NextBUY_2", "NextSELL_2"
						   };
	int g_index_400;
	int gi_404 = 7;
	int gia_408[7] = {1, 0, 0, 0, 0, 0, 0};
	String gsa_412[7];
	int gia_416[7] = {10, 1, 0, 1, 1, 1, 1};
	int gia_420[7] = {5, 3, 3, 3, 3, 3, 3};
	int gia_424[7] = {20, 20, 20, 20, 20, 20, 20};
	int gia_428[7] = {7, 7, 7, 7, 7, 7, 7};
	int gia_432[7] = {111, 222, 333, 444, 555, 777, 999};
	double gda_unused_436[7];
	double gda_unused_440[7];
	double gda_unused_444[7];
	double gda_unused_448[7];
	double gda_unused_452[7];
	double gda_unused_456[7];
	double gda_unused_460[7];
	int gia_464[10] = {1, 5, 15, 30, 60, 240, 1440, 10080, 43200, 0};
	double gda_468[7][16][50];
	double gda_472[7][16][50];
	double gda_476[7][16][50];
	int gia_480[7][16][50];
	double gda_484[7][16][50];
	double gda_488[7][16][50];
	double gda_492[7];
	double gda_496[7];
	double gda_500[7][16];
	double gda_504[7][16];
	int gia_508[7];
	int gia_512[7];
	int gia_516[7];
	int gia_520[7];
	int gia_524[7];
	int gia_528[7];
	int g_pos_532;
	bool gi_536;
	int g_ord_total_540;
	int gi_552;
	int gi_556;
	int gia_568[7];
	int gia_572[7];
	int gia_576[7];
	int gia_580[7];
	int gia_584[7];
	int gia_588[7];
	int gia_592[7];
	int gia_596[7];
	int gia_600[7];
	int gia_604[7];
	int gia_608[7];
	int gia_612[7];
	int gia_616[7];
	int gia_620[7];
	int gia_unused_624[7];
	int gia_unused_628[7];
	int gia_632[7];
	int gia_636[7];
	int gia_unused_640[7];
	int gi_644;
	double gda_648[7];
	double gda_652[7];
	double gda_656[7];
	double gda_660[7];
	int gia_664[7];
	int gia_668[7];
	double gda_672[7];
	double gda_676[7];
	double gda_680[7];
	double gda_684[7];
	double gda_688[7];
	double gda_692[7];
	double gda_696[7];
	double gda_700[7];
	int gi_unused_704;
	int gi_unused_708;
	bool gi_712;
	bool gi_716;
	double g_price_736;
	double g_price_744;
	double g_price_752;
	double g_price_760;
	double gda_768[7];
	double gda_772[7];
	int gi_unused_776 = 10;

public:
	typedef SuperMart CLASSNAME;
	SuperMart();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int OpOrd(String a_symbol_0, int a_cmd_8, double a_lots_12, double a_price_20, double a_price_28, double a_price_36, int a_magic_44);
	double UB(int ai_0, int ai_4);
	double U0(int ai_0, int ai_4);
	int MIN(int ai_0, int ai_4);
	int MAX(int ai_0, int ai_4);
	int NOV(int ai_0, int ai_4, double ad_8);
	int NON(int ai_0, int ai_4, double ad_8);
	void N();
	double GetProfitForDay(int ai_0);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("RiskPercent", RiskPercent, 1, 10)
			% Arg("ProtectionTP", ProtectionTP, 1, 100);
	}
	
};






class Spiral : public ExpertAdvisor {
	
	// Args
	int mom_period = 10;
	int rsi_period = 10;
	int adx_period = 10;
	int wpr_period = 10;
	int TakeProfit = 55;
	int StopLoss = 200;
	double Lots = 1.0;
	bool UseTrailingStop = false;
	int TrailingStop = 0;
	int Slippage = 3;
	bool MM = true;
	double Risk = 0.05;
	bool NFA = false;
	int MagicNumber = 6786787;
	
	bool use_alert = false;
	bool use_bars_counter = true;
	bool use_sl = true;
	bool use_tp = true;
	bool use_manual_sltp = true;
	int g_bars_144;
	int gi_unused_148;
	bool is_recent = false;
	double price0 = 0.0;
	double price1 = 0.0;
	int g_ticket_172 = -1;

public:
	typedef Spiral CLASSNAME;
	Spiral();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double CalculateMM(double ad_0);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("mom_period", mom_period, 2, 100)
			% Arg("rsi_period", rsi_period, 2, 100)
			% Arg("adx_period", adx_period, 2, 100)
			% Arg("wpr_period", wpr_period, 2, 100)
			% Arg("TakeProfit", TakeProfit, 2, 300)
			% Arg("StopLoss", StopLoss, 2, 300);
	}
	
};











class MoneyTime : public ExpertAdvisor {
	
	// Args
	int    ProfitFactor      = 2;
	int    StopLossFactor    = 14;
	int    Distance          = 10;
	int    AccountOrders     = 5;
	int    MinProfit         = 20;
	int    Trailing          = false;
	int    TrailingStop      = 60;
	int    TrailingStep      = 20;
	int    MinProfitB        = 30;
	int    NoLossLevel       = 19;
	int    NoLoss            = true;
	int    RSIPeriod          = 15;
	int    RSIMA              = 5;
	int    RSIPeriod2         = 20;
	int    RSIPeriod3         = 20;
	int    RSIMA2             = 9;
	int    RSISellLevel       = 34;
	int    RSIBuyLevel        = 70;
	int    RSISellLevel2      = 34;
	int    RSIBuyLevel2       = 68;
	int    MAFilter           = 20;
	
	int    MagicNumber       = 89539235;
	int    Slippage          = 3;
	bool   BarsControl       = true;
	
	bool   FixedLot          = false;
	double Lots              = 0.01;
	double MaximumRisk       = 0.7;
	double DecreaseFactor    = 0;
	
	
	bool   RsiTeacher        = true;
	bool   RsiTeacher2       = true;
	int    Shift              = 0;
	bool   TrendFilter        = false;
	int    MAMode             = 0;  // 0=SMA,1=EMA,2=SSMA,3=LWMA
	int    MAPrice            = 0;  // 0=Close,1=Open,2=High,3=Low,4=Median,5=Typical,6=Weighted
	bool   TradeOnFriday      =  true;
	int    BaginTradeHour     = 0;
	int    EndTradeHour       = 0;
	
	
	// Global variables
	 
	double ShortOrderPrevProfit, ShortOrderPrevLot, ShortOrderPrevSL, ShortOrderPrevTP;
	double LongOrderPrevProfit, LongOrderPrevLot, LongOrderPrevSL, LongOrderPrevTP;
	int LongOrderPrevTicket, ShortOrderPrevTicket;
	           
	int buyOrders = 0, sellOrders = 0, allOrders = 0;
	String DivTrainStr = "; ";
	
	// Long position ticket
	int LongTicket = -1;
	
	// Short position ticket
	int ShortTicket = -1;
	int pos;
	
	int CountBuyOrders = 0, CountSellOrders = 0;
	
public:
	typedef MoneyTime CLASSNAME;
	MoneyTime();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int GetCurrentOrders(String symbol);
	double GetLotSize();
	void CheckForOpen();
	bool IsBuyLock();
	bool IsSellLock();
	bool GetBuySignal();
	bool GetSellSignal();
	void TrailPositions();
	void CreateNoLoss();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("ProfitFactor", ProfitFactor, 1, 10)
			% Arg("StopLossFactor", StopLossFactor, 1, 100)
			% Arg("Distance", Distance, 1, 100)
			% Arg("AccountOrders", AccountOrders, 1, 100)
			% Arg("MinProfit", MinProfit, 1, 100)
			% Arg("Trailing", Trailing, 0, 1)
			% Arg("TrailingStop", TrailingStop, 2, 100)
			% Arg("TrailingStep", TrailingStep, 2, 100)
			% Arg("MinProfitB", MinProfitB, 2, 100)
			% Arg("NoLossLevel", NoLossLevel, 2, 100)
			% Arg("NoLoss", NoLoss, 0, 1)
			% Arg("RSIPeriod", RSIPeriod, 2, 100)
			% Arg("RSIMA", RSIMA, 2, 100)
			% Arg("RSIPeriod2", RSIPeriod2, 2, 100)
			% Arg("RSIPeriod3", RSIPeriod3, 2, 100)
			% Arg("RSIMA2", RSIMA2, 2, 100)
			% Arg("RSISellLevel", RSISellLevel, 0, 100)
			% Arg("RSIBuyLevel", RSIBuyLevel, 0, 100)
			% Arg("RSISellLevel2", RSISellLevel2, 0, 100)
			% Arg("RSIBuyLevel2", RSIBuyLevel2, 0, 100)
			% Arg("MAFilter", MAFilter, 2, 100);
	}
	
};










class Novice : public ExpertAdvisor {
	
	// Args
	int FastEMA = 12;
	int SlowEMA = 26;
	int SignalSMA = 9;
	int TakeProfit1 = 40;
	int InitialStop1 = 0;
	int TrailingStop1 = 25;
	int ProfitIndex = 30;
	int MaxTrades = 20;
	int Pips = 15;
	
	int MAGIC = 123456;
	double Lots = 0.010000;
	double Doble = 1.500000;
	int Revers = 0;
	int ABG_Algoritm = 20;
	
	bool v54 = true;
	bool v60 = true;
	bool v6C = false;
	bool v84 = false;
	int vD8 = 10;
	double vDC = 0.010000;
	double vE4 = 2.000000;
	int vEC = 40;
	int vF0 = 0;
	int vF4 = 30;
	int v100 = 10;
	double v104 = 0.010000;
	double v10C = 2.000000;
	int v114 = 40;
	int v118 = 0;
	int v11C = 30;
	int v128 = 10;
	double v12C = 0.010000;
	double v134 = 2.000000;
	int v13C = 40;
	int v140 = 0;
	int v144 = 30;
	int v150 = 10;
	double v154 = 0.010000;
	double v15C = 2.000000;
	int v164 = 40;
	int v168 = 0;
	int v16C = 30;
	int v178 = 10;
	double v17C = 0.010000;
	double v184 = 2.000000;
	int v18C = 40;
	int v190 = 0;
	int v194 = 30;
	int v1A0 = 10;
	double v1A4 = 0.010000;
	double v1AC = 2.000000;
	int v1B4 = 20;
	int v1B8 = 0;
	int v1BC = 30;
	int v1C8 = 10;
	double v1CC = 0.010000;
	double v1D4 = 2.000000;
	int v1DC = 40;
	int v1E0 = 0;
	int v1E4 = 30;
	int v1F0 = 10;
	double v1F4 = 0.010000;
	double v1FC = 2.000000;
	int v204 = 20;
	int v208 = 0;
	int v20C = 30;
	int v218 = 10;
	double v21C = 0.010000;
	double v224 = 2.000000;
	int v22C = 20;
	int v230 = 0;
	int v234 = 30;
	int v24C = 1;
	int v250 = 0;
	int v254 = 3;
	int v258 = 2;
	bool v300 = true;
	
	int v264, v268, v26C, v270, v274, v278, v27C, v280, v284, v314, v304, v25C, v260, v308;
	double v288, v290, v298, v2A0, v2A8, v2B0, v2B8, v2C0, v2C8, v2D0, v2D8, v2E0, v2E8, v2F0, v2F8;

	
public:
	typedef Novice CLASSNAME;
	Novice();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	void InitParam(int v0);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("FastEMA", FastEMA, 2, 100)
			% Arg("SlowEMA", SlowEMA, 2, 100)
			% Arg("SignalSMA", SignalSMA, 2, 100)
			% Arg("TakeProfit1", TakeProfit1, 2, 100)
			% Arg("InitialStop1", InitialStop1, 0, 100)
			% Arg("TrailingStop1", TrailingStop1, 0, 100)
			% Arg("ProfitIndex", ProfitIndex, 0, 100)
			% Arg("MaxTrades", MaxTrades, 0, 100)
			% Arg("Pips", Pips, 2, 100);
	}
	
};

}

#endif
