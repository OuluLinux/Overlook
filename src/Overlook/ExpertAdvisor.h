#ifndef _Overlook_ExpertAdvisor_h_
#define _Overlook_ExpertAdvisor_h_

namespace Overlook {


class ExpertAdvisor : public Core {
	
protected:
	friend class MultiExpertAdvisor;
	
	
	struct OptimizationCtrl : public Ctrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
		int type = 0;
		JobCtrl* job;
	};
	
	struct TrainingCtrl : public JobCtrl {
		TabCtrl tabs;
		OptimizationCtrl opt0, opt1, opt2, opt3;
		
		TrainingCtrl();
	};
	
	struct Setting : Moveable<Setting> {
		int start, stop, timeofday;
	};
	
	
	// Persistent
	Optimizer opt;
	Vector<double> training_pts, besteq_pts, cureq_pts, lots_pts;
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
	int digits;
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
			% Mem(besteq_pts)
			% Mem(cureq_pts)
			% Mem(lots_pts)
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
	int		OrderClose(int ticket, double lots, double price, int slippage, Color clr) {return sb.OrderClose(ticket, lots, price, slippage);}
	double	OrderClosePrice() {return sb.OrderClosePrice();}
	Time	OrderCloseTime() {return sb.OrderCloseTime();}
	String	OrderComment() {return sb.OrderComment();}
	double	OrderCommission() {return sb.OrderCommission();}
	int		OrderDelete(int ticket) {return sb.OrderDelete(ticket);}
	int		OrderExpiration() {return sb.OrderExpiration();}
	double	OrderLots() {return sb.OrderLots();}
	int		OrderMagicNumber() {return sb.OrderMagicNumber();}
	int		OrderModify(int ticket, double price, double stoploss, double takeprofit, Time expiration=Time(3000,1,1)) {return sb.OrderModify(ticket, price, stoploss, takeprofit, expiration);}
	int		OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration, Color clr) {return sb.OrderModify(ticket, price, stoploss, takeprofit, Time(3000,1,1));}
	double	OrderOpenPrice() {return sb.OrderOpenPrice();}
	Time	OrderOpenTime() {return sb.OrderOpenTime();}
	double	OrderProfit() {return sb.OrderProfit();}
	int		OrderSelect(int index, int select, int pool=libmt::MODE_TRADES) {return sb.OrderSelect(index, select, pool);}
	int		OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, String comment="", int magic=0, int expiry=0) {return sb.OrderSend(symbol, cmd, volume, price, slippage, stoploss, takeprofit, comment, magic, expiry);}
	int		OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, String comment, int magic, int expiry, Color clr) {return sb.OrderSend(symbol, cmd, volume, price, slippage, stoploss, takeprofit, comment, magic, expiry);}
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
	double	MathLog(double d) {return log(d);}
	double	MathPow(double d, double e) {return pow(d, e);}
	double	MathRound(double d) {return floor(d);}
	double	MathFloor(double d) {return floor(d);}
	double	MathCeil(double d) {return ceil(d);}
	double	MathRand() {return Random(32767);}
	void	RefreshRates() {}
	String	GetLastError() {return sb.GetLastError();}
	bool	IsTradeContextBusy() {return false;}
	int		StringFind(const String& str, const String& match) {return str.Find(match);}
	int		StringFind(const String& str, const String& match, int begin) {return str.Find(match, begin);}
	String	StringSubstr(const String& str, int mid, int len) {return str.Mid(mid, len);}
	String	StringConcatenate(const String& a, const String& b) {return a + b;}
	int		StringLen(const String& a) {return a.GetCount();}
	void	Sleep(int ms) {}
	bool	IsDemo() {return true;}
	
	double Ask, Bid, Point;
	int Digits, Bars;
	Time Now;
	
};



}

#endif
