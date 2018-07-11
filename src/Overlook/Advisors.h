#ifndef _Overlook_Advisors_h_
#define _Overlook_Advisors_h_

#undef MIN
#undef MAX

namespace Overlook {

/*
	Start:	1136
							EURJPY
							M5		H1
	Snake							55700
	ChannelScalper					3102
	Eagle							10164
	Salmon							1403
	Explorer						1616
	Mari							55085
	Pyramid							2203
	SuperMart		(Slow)			2514
	Spiral							4004
	MoneyTime						9075
	StochPower						34577
	Gainer							53907
	Cashier							8708
	ModestTry		(Slow)			44000
	Gaia							10376
	Squirter						10124
	Julia							5547
	Outsider						7e7
	Foster							16436
	
*/

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

















class StochPower : public ExpertAdvisor {
	
	int    ma_period = 14;
	int risk = 1;           // risk to calculate the lots size (only if mm is enabled)
	int OrderstoProtect = 3; // Number of orders to enable the account protection
	int    TakeProfit = 60;  // Profit Goal for the latest order opened
	int Pips = 90;            // Distance in Pips from one order to another
	int    StopLoss = 500;  // StopLoss
	
	bool ExitWithSTOCH = false;
	int pos = 0;
	double Lots = 0.01;       // We start with this lots number
	bool MyMoneyProfitTarget = false;
	double My_Money_Profit_Target = 50;
	double multiply = 3.0;
	int MaxTrades = 3;       // Maximum number of orders to open
	int    TrailingStop = 0;// Pips to trail the StopLoss
	int mm = 1;              // if one the lots size will increase based on account size
	int AccountType = 1;  // 0 if Normal Lots, 1 for mini lots, 2 for micro lots
	int MagicNumber = 312133;  // Magic number for the orders placed
	bool SecureProfitProtection = false;
	int SecureProfit = 2000000000;   // If profit made is bigger than SecureProfit we close the orders
	bool AllSymbolsProtect = false; // if one will check profit from all symbols, if cero only this symbol
	bool EquityProtection = false;  // if true, then the expert will protect the account equity to the percent specified
	int AccountEquityPercentProtection = 90; // percent of the account to protect on a set of trades
	bool AccountMoneyProtection = false;
	double AccountMoneyProtectionValue = 3000.00;
	bool UseHourTrade = false;
	int FromHourTrade = 0;
	int ToHourTrade = 23;
	
	String  TradingTime = "--trading time setting--";
	bool    UseTradingHours = false;
	bool    TradeAsianMarket = true;
	int     StartHour1 = 0;       // Start trades after time
	int     StopHour1 = 3;      // Stop trading after time
	bool    TradeEuropeanMarket = true;
	int     StartHour2 = 9;       // Start trades after time
	int     StopHour2 = 11;      // Stop trading after time
	bool    TradeNewYorkMarket = true;
	int     StartHour3 = 15;       // Start trades after time
	int     StopHour3 = 17;      // Stop trading after time
	bool TradeOnFriday = true;
	String  OtherSetting = "--Others Setting--";
	int OrdersTimeAlive = 0;  // in seconds
	String  reverse = "if one the desition to go long/short will be reversed";
	bool ReverseCondition = false;  // if one the desition to go long/short will be reversed
	String  limitorder = "if true, instead open market orders it will open limit orders ";
	bool SetLimitOrders = false; // if true, instead open market orders it will open limit orders
	int Manual = 0;          // If set to one then it will not open trades automatically
	
	
	int OpenOrdersBasedOn = 16; // Method to decide if we start long or short
	bool ContinueOpening = true;
	
	int  OpenOrders = 0, cnt = 0;
	int MarketOpenOrders = 0, LimitOpenOrders = 0;
	int  slippage = 5;
	double sl = 0, tp = 0;
	double BuyPrice = 0, SellPrice = 0;
	double lotsi = 0, mylotsi = 0;
	int mode = 0, myOrderType = 0, myOrderTypetmp = 0;
	
	double LastPrice = 0;
	int  PreviousOpenOrders = 0;
	double Profit = 0;
	int LastTicket = 0, LastType = 0;
	double LastClosePrice = 0, LastLots = 0;
	double Pivot = 0;
	double PipValue = 0;
	bool Reversed = false;
	double tmp = 0;
	double iTmpH = 0;
	double iTmpL = 0;
	Time NonTradingTime[2];
	bool FileReaded = false;
	String dateLimit = "2030.01.12 23:00";
	int CurTimeOpeningFlag = 0;
	Time LastOrderOpenTime;
	bool YesStop;


public:
	typedef StochPower CLASSNAME;
	StochPower();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	void OpenMarketOrders();
	void OpenLimitOrders();
	void DeleteAllObjects();
	int OpenOrdersBasedOnSTOCH();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("ma_period", ma_period, 2, 100)
			% Arg("risk", risk, 1, 4)
			% Arg("OrderstoProtect", OrderstoProtect, 2, 100)
			% Arg("TakeProfit", TakeProfit, 2, 100)
			% Arg("Pips", Pips, 2, 150)
			% Arg("StopLoss", StopLoss, 10, 1000)
			;
	}
	
};











class Gainer : public ExpertAdvisor {

	//int gi_76 = 3027389;
	String gs_80 = "FRX_";
	String gsa_88[7];
	int gi_unused_92 = true;
	int gi_96 = false;
	double gd_100 = 0.01;
	int RiskPercent = 20.0;
	int TorgSloy = 3;
	int N_enable_Sloy = 7;
	double g_maxlot_148 = 0.0;
	double LotMultiplicator = 1.2;
	int hSETKY = 30;
	int gi_unused_184 = 30;
	int Uvel_hSETKY = 1; // 1, 2
	int ShagUvel_hSETKY = 2;
	int gi_212 = 0;
	int gi_216 = 0;
	int ProtectionTP = 7;
	int gi_232 = 7;
	int gi_unused_236 = 1;
	double gd_unused_240 = 40.0;
	int Magic = 1230;
	bool ShowTableOnTesting = true;
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("gi_96", gi_96, 0, 1)
			% Arg("RiskPercent", RiskPercent, 0, 100)
			% Arg("TorgSloy", TorgSloy, 0, 10)
			% Arg("N_enable_Sloy", N_enable_Sloy, 0, 10)
			% Arg("hSETKY", hSETKY, 0, 100)
			% Arg("Uvel_hSETKY", hSETKY, 1, 2)
			% Arg("ProtectionTP", ProtectionTP, 1, 100)
			% Arg("gi_232", gi_232, 1, 100)
			;
	}
	
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
	   13959039, 17919};
	String gsa_396[3][2] = {"NextBUY_0", "NextSELL_0",
	   "NextBUY_1", "NextSELL_1",
	   "NextBUY_2", "NextSELL_2"};
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
	int gi_536;
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
	typedef Gainer CLASSNAME;
	Gainer();
	
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
	
};








class Cashier : public ExpertAdvisor {
	
	
	int g_period_312 = 5.0;
	int g_period_320 = 10.0;
	int g_period_328 = 22.0;
	int g_period_336 = 20.0;
	int g_period_344 = 26.0;
	int g_period_352 = 13.0;
	int g_period_360 = 15.0;
	int g_period_368 = 10.0;
	int Percent_Over_Balance = 10.0;
	int MaximumRisk = 5.0;
	int TakeProfit = 25.0;
	int StopLoss = 500.0;
	int DecreaseFactor = 3.0;
	int Hedge = false;
	int HedgeProfit = 25.0;
	int HedgeStopLoss = 100.0;
	int PipDistance = 50;
	int HighRisk = false;
	int HR_StopLoss = 500.0;
	
	String Header = "Auto Cash Generator";
	String Creator = "Oewee, Apr 2009";
	int MagicNo = 4399;
	String Style = "=== Trading Management ===";
	int StartTime_Hr = 0;
	int EndTime_Hr = 23;
	bool TradeOnFriday = true;
	bool ProtectBalance = false;
	bool ProtectProfit = false;
	bool UseAutoLot = false;
	double MaxLots = 0.1;
	double MinLots = 0.1;
	int MaxOrder = 1;
	double StopTime = 0.0;
	double TrailingStop = 0.0;
	bool TrailingProfit = false;
	int TrailingPips = 1;
	String Header1 = "=== Hedge setting ===";
	int Hedge_Trigger = 100;
	double HedgeLotSize = 2.0;
	double LtSize = 2.0;
	String Wish_U = "=== Good Luck ===";
	
	int g_period_284 = 10;
	double gd_288 = 10.0;
	double gd_296 = 3.0;
	String gs_unused_304 = "=== Indicators setting ===";
	int g_period_376 = 100;
	int g_slippage_380 = 5;
	double g_price_384;
	double gd_392;
	double g_point_400;
	int pos;

public:
	typedef Cashier CLASSNAME;
	Cashier();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double LotsOptimized();
	double Predict();
	double Predict2();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("g_period_312", g_period_312, 2, 100)
			% Arg("g_period_320", g_period_320, 2, 100)
			% Arg("g_period_328", g_period_328, 2, 100)
			% Arg("g_period_336", g_period_336, 2, 100)
			% Arg("g_period_344", g_period_344, 2, 100)
			% Arg("g_period_352", g_period_352, 2, 100)
			% Arg("g_period_360", g_period_360, 2, 100)
			% Arg("g_period_368", g_period_368, 2, 100)
			% Arg("Percent_Over_Balance", Percent_Over_Balance, 2, 100)
			% Arg("MaximumRisk", MaximumRisk, 2, 10)
			% Arg("TakeProfit", TakeProfit, 2, 100)
			% Arg("StopLoss", StopLoss, 2, 1000)
			% Arg("DecreaseFactor", DecreaseFactor, 1, 10)
			% Arg("Hedge", Hedge, 0, 1)
			% Arg("HedgeProfit", HedgeProfit, 0, 100)
			% Arg("HedgeStopLoss", HedgeStopLoss, 0, 200)
			% Arg("PipDistance", PipDistance, 0, 200)
			% Arg("HighRisk", HighRisk, 0, 1)
			% Arg("HR_StopLoss", HR_StopLoss, 0, 1000)
			;
	}
	
};













class ModestTry : public ExpertAdvisor {
		
	int StopLoss = 80;
	int TakeProfit = 0;
	int AddToLevel = 14;
	int TryMinutes = 15;
	int TrailingStop = 28;
	int TrailingStep = 10;
	int TrailingKeep = 13;
	int MaxOrdersCount = 1;
	
	int TimeFrame = 0;
	double Lots = 0.1;
	int Magic = 123;
	int gi_124;
	int gi_128 = 1;
	Time g_datetime_132;
	Time g_datetime_136;
	Time g_datetime_140;
	Time g_datetime_144;
	Time g_datetime_148;
	Time g_datetime_152;
	Time g_datetime_156;
	Time g_datetime_160;
	Time g_datetime_164;
	
public:
	typedef ModestTry CLASSNAME;
	ModestTry();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int fDeletePendig(int a_ticket_0);
	double ND(double ad_0);
	int fOrderSetBuyStop(double ad_0, int a_datetime_8 = 0);
	int fOrderSetSellStop(double ad_0, int a_datetime_8 = 0);
	double fGetLotsSimple(int a_cmd_0);
	String fMyErDesc(int ai_0);
	int fTrailingWithStart();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("StopLoss", StopLoss, 2, 150)
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("AddToLevel", AddToLevel, 0, 100)
			% Arg("TryMinutes", TryMinutes, 0, 200)
			% Arg("TrailingStop", TrailingStop, 2, 100)
			% Arg("TrailingStep", TrailingStep, 2, 100)
			% Arg("TrailingKeep", TrailingKeep, 2, 100)
			% Arg("MaxOrdersCount", MaxOrdersCount, 1, 100)
			;
	}
	
};





class Gaia : public ExpertAdvisor {
	
	int g_period_200 = 25;
	int g_period_184 = 57;
	int gd_168 = 20.0;
	
	double gd_160 = 44.0;
	bool Acc5Digits = true;
	double Lots = 1.0;
	int gi_96 = 141;
	int gi_100 = 200;
	int g_slippage_104 = 3;
	int g_magic_108 = 20070729;
	bool EnableMM = false;
	double LotBalancePcnt = 10.0;
	double MinLot = 0.1;
	double MaxLot = 5.0;
	int LotPrec = 1;
	
	int gi_188 = 79;
	int g_ma_method_192 = MODE_EMA;
	int g_applied_price_196 = PRICE_CLOSE;
	int gi_204 = 80;
	int g_ma_method_208 = MODE_EMA;
	int g_applied_price_212 = PRICE_CLOSE;
	int gi_216 = 1;
	
public:
	typedef Gaia CLASSNAME;
	Gaia();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double GetLots();
	int CloseOrders(int a_cmd_0);
	int OrdersCount0();
	int Buy(String a_symbol_0, double a_lots_8, double a_price_16, double a_price_24, double a_price_32, int a_magic_40, String a_comment_44 = "");
	int Sell(String a_symbol_0, double a_lots_8, double a_price_16, double a_price_24, double a_price_32, int a_magic_40, String a_comment_44 = "");
	int CloseOrder(int a_ticket_0, double a_lots_4, double a_price_12);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % In<VolumeSlots>()
			% Arg("g_period_200", g_period_200, 2, 100)
			% Arg("g_period_184", g_period_184, 2, 100)
			% Arg("gd_168", gd_168, 2, 100)
			;
		////reg % Arg("", , );
	}
	
};



class Squirter : public ExpertAdvisor {
	
	
	int mm = false;
	int risk = 1.0;
	int maxtrades = 1;
	int tradesperbar = 1;
	int stoploss = 75.0;
	int takeprofit = 10.0;
	int gd_348 = 15.0;
	int gd_356 = 15.0;
	int gd_364 = 1.0;
	int gd_372 = 0.0;
	int gd_380 = 0.0;
	int g_period_256 = 14;
	int g_period_260 = 7;
	int g_slowing_264 = 9;
	int gi_268 = 30;
	int gi_272 = 70;
	int g_period_236 = 7;
	int g_period_216 = 7;
	int gi_240 = 27;
	int gi_244 = 69;
	
	int magic = 7777;
	bool ecn = false;
	double minlot = 0.01;
	double maxlot = 50000.0;
	double lots = 0.1;
	int gi_152 = 1;
	double gd_156 = 5.0;
	double gd_164 = 10.0;
	double gd_172 = 10.0;
	bool gi_180 = true;
	bool gi_184 = false;
	bool gi_188 = false;
	bool gi_192 = false;
	int gi_196 = 20;
	bool gi_200 = false;
	bool gi_204 = false;
	bool gi_208 = false;
	int g_timeframe_212;
	int g_ma_method_220 = MODE_SMMA;
	int gi_224 = 8;
	bool gi_228 = true;
	int g_timeframe_232 = 0;
	bool gi_248 = true;
	int g_timeframe_252 = 0;
	int gi_276 = 0;
	int gi_280 = 2;
	int gi_284 = 1;
	bool gi_288 = false;
	int gi_292 = 12;
	int gi_296 = 0;
	bool gi_300 = false;
	int gi_304 = 0;
	int gi_308 = 1;
	int gi_312 = 21;
	int gi_316 = 0;
	bool gi_320 = true;
	bool gi_324 = true;
	int gi_328 = 0;
	int gi_332 = 0;
	int gi_336 = 2;
	bool gi_340 = false;
	bool gi_344 = false;
	bool i_unused_0;
	int g_str2time_396;
	int g_str2time_400;
	int g_str2time_404;
	int g_str2time_408;
	Time g_datetime_412;
	Time g_datetime_416;
	int g_pos_424;
	int g_bars_428 = -1;
	int g_count_432;
	int g_count_436;
	int g_count_440;
	int gi_444 = 100;
	int gi_460;
	int gi_464;
	int gi_468;
	int gi_472;
	int gi_476;
	int g_count_480;
	int gi_484;
	int gi_488;
	String gs_496;
	String gs_504;
	String gs_512;
	String gs_520;
	String gs_528;
	String gs_536;
	String gs_544;
	String gs_552;
	double g_price_568;
	double g_price_576;
	double g_lots_584;
	double g_ord_open_price_640;
	String ts_unused_0;
	double g_ord_open_price_648;
	bool gi_unused_680 = false;
	bool gi_684 = true;
	bool gi_688 = true;
	double gd_692;
	double gd_700;
	int g_digits_708;
	int gi_712 = 1304208000;

public:
	typedef Squirter CLASSNAME;
	Squirter();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int open(int a_cmd_0, double ad_4, double ad_12, double ad_20, double ad_28, Time a_datetime_36);
	double lotsoptimized();
	bool checktime();
	int count(int a_cmd_0);
	void closebuy();
	void closesell();
	void hideclosebuy();
	void hideclosesell();
	void movebreakeven(double ad_0, double ad_8);;
	void movetrailingstop(double ad_0, double ad_8);
	void createlstoploss(double ad_0);
	void createsstoploss(double ad_0);
	void createltakeprofit(double ad_0);
	void createstakeprofit(double ad_0);
	void Delete(int a_cmd_0);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("mm", mm, 0, 1)
			% Arg("risk", risk, 1, 5)
			% Arg("maxtrades", maxtrades, 1, 100)
			% Arg("tradesperbar", tradesperbar, 1, 5)
			% Arg("stoploss", stoploss, 1, 200)
			% Arg("takeprofit", takeprofit, 1, 200)
			% Arg("gd_348", gd_348, 1, 100)
			% Arg("gd_356", gd_356, 1, 100)
			% Arg("gd_364", gd_364, 1, 100)
			% Arg("gd_372", gd_372, 1, 100)
			% Arg("gd_380", gd_380, 1, 100)
			% Arg("g_period_256", g_period_256, 2, 100)
			% Arg("g_period_260", g_period_260, 2, 100)
			% Arg("g_slowing_264", g_slowing_264, 2, 100)
			% Arg("gi_268", gi_268, 0, 100)
			% Arg("gi_272", gi_272, 0, 100)
			% Arg("g_period_236", g_period_236, 2, 100)
			% Arg("g_period_216", g_period_216, 2, 100)
			% Arg("gi_240", gi_240, 0, 100)
			% Arg("gi_244", gi_244, 0, 100)
			;
	}
	
};










class Julia : public ExpertAdvisor {
	
	int NLot = 3;
	int Rate = 1.0;
	int MMoney = false;
	int Martin = false;
	int idle_bars_avoid = 10;
	
	double BLot = 0.1;
	bool gi_104;
	bool gi_108;
	bool gi_112;
	bool gi_116;
	bool gi_120;
	bool gi_124;
	bool gi_128;
	double g_price_132;
	double g_price_140;
	double g_price_148;
	double g_price_156;
	double g_price_164;
	double g_price_172;
	double g_price_180;
	double g_price_188;
	double g_price_196;
	int g_ticket_204;
	int g_ticket_208;
	int g_ticket_212;
	int g_ticket_216;
	int g_ticket_220;
	int g_ticket_224;
	int g_ticket_228;
	int g_ticket_232;
	int g_count_236;
	int last_open_pos = 0;
	
public:
	typedef Julia CLASSNAME;
	Julia();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("NLot", NLot, 0, 10)
			% Arg("Rate", Rate, 1, 5)
			//% Arg("MMoney", MMoney, 0, 1)
			% Arg("Martin", Martin, 0, 1)
			% Arg("idle_bars_avoid", idle_bars_avoid, 2, 100)
			;
	}
	
};




class Outsider : public ExpertAdvisor {
	
	int ma_period = 10;
	int gi_164 = 20;
	int gi_200 = 4;
	int Percent_Risk = 3;
	int Trailing_Stop = 100;
	
	String gs_76 = "V2_6";
	int gi_84 = 10;
	double gd_88 = 4.0;
	double gd_96 = 25.0;
	String gs_104 = "OrderReliable fname unset";
	int gi_unused_112 = 0/* NO_ERROR */;
	String gs_unused_116 = "===USE THIS TO ADJUST FIP SETTING===";
	int gi_124 = 15;
	int gi_128 = 3;
	int gi_unused_132 = 0;
	String Trade_Setting = "===SET YOUR TRADE SETTING HERE===";
	int TradeMagic1 = 978541;
	int TradeMagic2 = 978542;
	int TradeMagic3 = 978543;
	int gi_156 = 4;
	bool gi_160 = false;
	int gi_168 = 0;
	bool gi_172 = true;
	String MM_Setting = "===SET YOUR MONEY MANAGEMENT SETTING HERE===";
	double Lots = 0.0;
	bool gi_196 = false;
	int gi_204 = 200;
	int Slippage = 3;
	int Limit1 = 100;
	int Limit2 = 200;
	String Trailing = "===SET TRAILING FOR 3RD TRADE===";
	bool Use_Trailing = true;
	int g_bars_236;
	int gi_240;
	double gd_244;
	double g_lotstep_252;
	double g_minlot_260;
	double g_maxlot_268;
	double g_lotsize_276;
	int pos;
	
public:
	typedef Outsider CLASSNAME;
	Outsider();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double Line(int p);
	int OrderSendReliable(String a_symbol_0, int a_cmd_8, double a_lots_12, double a_price_20, int a_slippage_28, double a_price_32, double a_price_40, String a_comment_48, int a_magic_56);
	int OrderModifyReliable(int a_ticket_0, double a_price_4, double a_price_12, double a_price_20);
	void OrderModifyReliableSymbol(String a_symbol_0, int ai_8, double ad_12, double ad_20, double ad_28, int ai_36, int ai_40 = -1);
	int OrderCloseReliable(int a_ticket_0, double a_lots_4, double a_price_12, int a_slippage_20);
	String OrderReliableErrTxt(int ai_0);
	void OrderReliablePrint(String as_0);
	String OrderReliable_CommandString(int ai_0);
	void OrderReliable_EnsureValidStop(String a_symbol_0, double ad_8, double &ad_16);
	void OrderReliable_SleepRandomTime(double ad_0, double ad_8);
	double LotsOptimized(int ai_0);
	bool IsBarEnd();
	int IsFIPUpCrossXCandle(int ai_0);
	int IsFIPDownCrossXCandle(int ai_0);
	int MyOrdersTotal();
	int MyOrdersTotalMagic(int a_magic_0);
	bool MyOrderBuy(int a_magic_0);
	bool MyOrderSell(int a_magic_0);
	double NormalizeTP(String as_0, int ai_8);
	int GetSLPips(int ai_0);
	double NormalizeSL(String as_0, int ai_8);
	void TrailOrder(int a_magic_0, int ai_unused_4);
	int CloseOrder(String as_0, int a_magic_8);
	double LowestCandles(int ai_0);
	double HighestCandles(int ai_0);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("ma_period", ma_period, 2, 100)
			% Arg("gi_164", gi_164, 2, 100)
			% Arg("gi_200", gi_200, 2, 10)
			% Arg("Percent_Risk", Percent_Risk, 2, 10)
			% Arg("Trailing_Stop", Trailing_Stop, 10, 200)
			;
	}
	
};







class Foster : public ExpertAdvisor {
	
	
	int g_period_104 = 14;
	int TP = 90;
	int SL = 110;
	int gi_92 = 80;
	
	//int g_period_108 = 16;
	int gi_120 = 70;
	int gi_124 = 10;
	double MaxLots = 1.0;
	int gi_144 = 15;
	int g_slippage_156 = 3;
	int Magic = 753494;
	bool gi_164 = false;
	int g_bars_168;
	double g_lots_172 = 1.0;
	double g_point_180;

public:
	typedef Foster CLASSNAME;
	Foster();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg % Arg("g_period_104", g_period_104, 2, 100)
			% Arg("TP", TP, 10, 150)
			% Arg("SL", SL, 10, 200)
			% Arg("gi_92", gi_92, 20, 200)
		;
	}
	
};






class Licker : public ExpertAdvisor {
	
	int gd_120 = 2;
	int gd_128 = 2;
	int gd_136 = 5;
	int gd_144 = 5;
	int gi_152 = 3;
	int gi_116 = 5; // sltp
	int gi_156 = 100; // sltp
	
	double lot = 0.01;
	bool RiskManagement = true;
	double RiskPercent = 1.0;
	int timecontrol = 1;
	int starttime = 18;
	int stoptime = 24;
	int TrailingStop = 0;
	int gi_112 = 0;
	double gd_160 = 50.0;
	double gd_168 = 161.0;
	double g_point_176;
	int gi_unused_184;
	int gi_unused_188;
	int g_bars_192;
	int gi_196;
	bool gi_200;
	bool gi_204;
	
	int MG = 23452;


public:
	typedef Licker CLASSNAME;
	Licker();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int ScalpingPattern();
	int deletelimitorder(int ai_0);
	int ChLimitOrder(int ai_0);
	double MaximumMinimum(int ai_0, int ai_4);
	double GetFiboUr(double ad_0, double ad_8, double ad_16);
	int GetTrailingStop();
	int BBU();
	int gettimecontrol(int ai_0, int ai_4);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("gd_120", gd_120, 2, 100)
			% Arg("gd_128", gd_128, 2, 100)
			% Arg("gd_136", gd_136, 2, 100)
			% Arg("gd_144", gd_144, 2, 100)
			% Arg("gi_152", gi_152, 2, 100)
			% Arg("gi_116", gi_116, 2, 100)
			% Arg("gi_156", gi_156, 2, 200)
			;
	}
	
};








class Spectrum : public ExpertAdvisor {
	
	
	int g_period_132 = 210;
	int adx_period = 10;
	int rsi_period = 10;
	int Level = 100;
	int TakeProfit = 30;
	int StopLoss = 150;
	
	bool OpenBuy = true;
	bool OpenSell = true;
	double StartLot = 0.1;
	double LotsDouble = 10.0;
	int Frame = 60;
	bool Timetrade = false;
	int StartTime = 16;
	int EndTime = 20;
	int MagicNumber = 20100;
	
	int g_ticket_136;
	int g_ticket_140;
	int g_ticket_144;
	int g_ticket_148;
	int g_ticket_152;
	int pos = 0;
	
public:
	typedef Spectrum CLASSNAME;
	Spectrum();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	void OpenBUYOrder(double a_lots_0, int a_magic_8);
	void OpenSELLOrder(double a_lots_0, int a_magic_8);
	int Procces();
	int MyRealOrdersTotal(int a_magic_0);
	int MyPendingOrdersTotal(int a_magic_0);
	int DeletePendingOrders(int a_magic_0);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("g_period_132", g_period_132, 2, 300)
			% Arg("adx_period", adx_period, 2, 100)
			% Arg("rsi_period", rsi_period, 2, 100)
			% Arg("Level", Level, 0, 200)
			% Arg("TakeProfit", TakeProfit, 0, 200)
			% Arg("StopLoss", StopLoss, 0, 200)
			;
	}
	
};










class Mega : public ExpertAdvisor {
	
	String EA_Name = "Forex-Giga_V2";
	String Creator = "Copyright © 2009, forex-giga.com";
	int MagicNo = 337733;
	String Part1 = "=== Time Management ===";
	bool Use_Time_Mgmt = false;
	bool Trade_On_Friday = true;
	String Part1_1 = "= Start-End Time =";
	int TradeStartHour = 0;
	int TradeStartMinutes = 0;
	int TradeEndHour = 12;
	int TradeEndMinutes = 0;
	String Part1_2 = "= Start-End Time 2 =";
	int TradeStartHour2 = 12;
	int TradeStartMinutes2 = 0;
	int TradeEndHour2 = 23;
	int TradeEndMinutes2 = 0;
	String Part2 = "=== Trading Management ===";
	bool MM = true;
	double RiskPercent = 5.0;
	double LotSize = 0.01;
	double Max_Lot_Size = 100.0;
	int MaxOrder = 5;
	double StopTime = 1.0;
	double TakeProfit = 25.0;
	double Min_Auto_TP = 10.0;
	double StopLoss = 350.0;
	double Slippage = 3.0;
	double TrailingStop = 15.0;
	bool TrailingProfit = true;
	int TrailingPips = 1;
	bool Profit_Protection = false;
	double Percent_Over_Balance = 5.0;
	bool BreakEven = false;
	int BreakEvenPips = 5;
	bool HedgeAllowed = false;
	String Wish_U_Have = "=== Good Luck ===";
	
	int g_period_288 = 10;
	double gd_unused_292 = 5.0;
	double gd_unused_300 = 10.0;
	double gd_unused_308 = 22.0;
	double gd_unused_316 = 20.0;
	double gd_unused_324 = 26.0;
	double gd_unused_332 = 13.0;
	double gd_unused_340 = 15.0;
	double gd_unused_348 = 10.0;
	double gd_356;
	double g_minlot_364;
	double g_lotstep_372;
	double gd_380;
	int gi_388;
	double g_price_392;
	double gd_400;
	double g_point_408;
	int g_slippage_416;
	bool gi_unused_420 = false;
	int gi_424 = 23;
	int gi_428 = 30;
	int gi_432 = 100;
	int gi_436 = 0;
	int gi_440 = 0;
	int gi_444 = 0;
	String gs_448;
	bool gi_460 = true;


public:
	typedef Mega CLASSNAME;
	Mega();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int Crossed(double ad_0, double ad_8);
	double Predict();
	double Predict2();
	double LotsOptimized();
	bool isTradeTime();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		////reg % Arg("", , );
	}
	
};
















class Maverick : public ExpertAdvisor {
	
	int TakeProfit = 275;
	int StopLoss = 0;
	int cci_period0 = 10;
	int cci_period1 = 10;
	int cci_period2 = 10;
	
	int Interval = 1;
	double Lots = 0.1;
	double g_open_96 = 0.0;
	double g_open_104 = 0.0;
	int g_count_112 = 0;

public:
	typedef Maverick CLASSNAME;
	Maverick();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TakeProfit", TakeProfit, 0, 300)
			% Arg("StopLoss", StopLoss, 0, 300)
			% Arg("cci_period0", cci_period0, 2, 200)
			% Arg("cci_period1", cci_period1, 2, 200)
			% Arg("cci_period2", cci_period2, 2, 200)
		;
	}
	
};


class Sherlock : public ExpertAdvisor {
	
	
	String gs_unused_76 = "version 1.0";
	double Grid_space_pips = 12.0;
	bool Close_open_trades = false;
	bool Close_pending_trades = false;
	bool Use_trail = true;
	double trail_in_pips = 60.0;
	double trail_space = 20.0;
	double size_of_lot = 0.1;
	int SL = 40;
	bool Use_trend_change = false;
	double pips_over_trend = 10.0;
	double nr_total_orders = 200.0;
	
	int gi_unused_152 = 0;
	double g_lots_156;
	int gi_unused_164 = 2000;
	bool gi_168 = true;
	bool gi_172 = true;
	bool gi_176 = true;
	bool gi_180 = true;
	bool gi_184 = true;
	bool gi_188 = true;
	bool gi_unused_192 = true;
	bool gi_unused_196 = true;
	bool gi_unused_200 = true;
	bool gi_unused_204 = true;
	bool gi_unused_208 = true;
	bool gi_unused_212 = true;
	double gd_216;
	int g_pos_224;
	double g_price_232;
	double g_price_240;
	double g_price_248;
	double g_price_256;
	double g_price_264;
	double g_price_272;
	double g_price_280;
	double g_price_288;
	double g_price_296;
	double g_price_304;
	double g_price_312;
	double g_price_320;
	double g_price_328;
	double g_price_336;
	int gi_344;
	int g_pos_348;
	int g_count_352;
	int g_pos_356;
	int gi_360;
	int g_pos_364;
	int gi_368;
	int g_pos_372;
	int g_count_376;
	int g_pos_380;
	int g_count_384;
	int g_pos_388;
	int gi_392 = 0;
	int gi_396;
	String g_comment_400 = "no comment";
	bool gi_408 = false;
	int gi_unused_420 = 0;
	int gi_unused_424 = 1;
	int gi_unused_428 = 13;
	bool gi_unused_436 = false;
	bool gi_unused_440 = false;
	bool gi_444 = false;
	bool gi_unused_448 = true;
	double gd_452;
	double gd_460;
	double gd_468;
	double gd_476;
	double gd_484;
	double gd_492;
	double gd_500;
	double gd_508;
	double gd_516;
	double gd_524;
	double gd_unused_532;
	int gi_540;
	double gd_544;
	double gd_552;
	int g_pos_576;
	int g_pos_580;
	bool gi_584 = false;
	bool gi_588 = false;
	bool gi_unused_592 = false;
	bool gi_unused_596 = false;
	bool gi_unused_600 = false;
	double gd_604 = 0.0;
	double gd_612 = 100.0;
	double gd_620 = 10000.0;
	int g_ord_total_628 = 0;


public:
	typedef Sherlock CLASSNAME;
	Sherlock();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		////reg % Arg("", , );
	}
	
};



class President : public ExpertAdvisor {
	
	int Period_Filtring = 3;
	int Aggr_level = 1;
	int SL_bs = 122;
	
	double Lots = 0.1;
	bool MM = false;
	bool In_BUY = true;
	bool In_SELL = true;
	double f1 = 0.18;
	double f2 = 0.18;
	bool RecoveryMode = false;
	bool TradeOnFridays = false;
	
	String password = "12345"; //MD5 hashed ( account number + IsDemo() + 1000 )
	String gs_164 = "";
	int gi_152 = 0;
	int gi_156 = 2;
	int gi_160 = 1;
	int g_ticket_176 = 0;
	int g_magic_180 = 144;
	int g_magic_184 = 233;
	double g_lots_188;
	bool gi_196;
	int pos = -1;
	
public:
	typedef President CLASSNAME;
	President();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	void LotHandle();
	int MA_Signal();
	void Trade_BUY();
	void Trade_SELL();
	double Out(int a_period_0, double ad_4);
	double lot(int ai_0);
	double LastClosedProfit();
	int LastClosedTicket();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("Period_Filtring", Period_Filtring, 2, 100)
			% Arg("Aggr_level", Aggr_level, 1, 100)
			% Arg("SL_bs", SL_bs, 0, 200)
		;
	}
	
};


class Gatherer : public ExpertAdvisor {
	
	int lag_gamma = 60;
	int cci_period = 60;
	int ma_period = 60;
	int Hours = 0;
	int Ìinutes = 0;
	bool RazVSutki = true;
	
	double gd_88 = 0.1;
	double gd_96 = 0.1;
	double gd_104 = 3.0;
	double g_pips_112 = 10.0;
	double g_pips_120 = 12.0;
	double g_period_128 = 120.0;
	double g_pips_136 = 60.0;
	double g_spread_144;


public:
	typedef Gatherer CLASSNAME;
	Gatherer();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double LotsOptimized();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("lag_gamma", lag_gamma, 0, 100)
			% Arg("cci_period", cci_period, 0, 100)
			% Arg("ma_period", ma_period, 0, 100)
			;
	}
	
};


class Musk : public ExpertAdvisor {
	
	double LotExponent = 1.59;
	double slip = 3.0;
	int gi_unused_88;
	double Lots = 0.01;
	int lotdecimal = 2;
	bool UseEquityStop = false;
	double TotalEquityRisk = 20.0;
	double MaxTradeOpenHours = 48.0;
	
	int g_magic_176 = 12324;
	double g_price_180;
	double gd_188;
	double gd_unused_196;
	double gd_unused_204;
	double g_price_212;
	double g_bid_220;
	double g_ask_228;
	double gd_236;
	double gd_244;
	double gd_260;
	bool gi_268;
	Time gi_284;
	int gi_288 = 0;
	double gd_292;
	int g_pos_300 = 0;
	int gi_304;
	double gd_308 = 0.0;
	bool gi_316 = false;
	bool gi_320 = false;
	bool gi_324 = false;
	int gi_328;
	bool gi_332 = false;
	double gd_336;
	double gd_344;

	int TakeProfit = 10.0;
	int Stoploss = 500.0;
	int TrailStart = 10.0;
	int TrailStop = 10.0;
	int PipStep = 30.0;
	int MaxTrades = 10;
	int UseTrailingStop = false;
	
public:
	typedef Musk CLASSNAME;
	Musk();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int CountTrades();
	void CloseThisSymbolAll();
	int OpenPendingOrder(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_32, int ai_36, String a_comment_40, int a_magic_48);
	double StopLong(double ad_0, int ai_8);
	double StopShort(double ad_0, int ai_8);
	double TakeLong(double ad_0, int ai_8);
	double TakeShort(double ad_0, int ai_8);
	double CalculateProfit();
	void TrailingAlls(int ai_0, int ai_4, double a_price_8);
	double AccountEquityHigh();
	double FindLastBuyPrice();
	double FindLastSellPrice();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TakeProfit", TakeProfit, 3, 100)
			% Arg("Stoploss", Stoploss, 0, 1000)
			% Arg("TrailStart", TrailStart, 0, 100)
			% Arg("TrailStop", TrailStop, 0, 100)
			% Arg("PipStep", PipStep, 0, 100)
			% Arg("MaxTrades", MaxTrades, 0, 100)
			% Arg("UseTrailingStop", UseTrailingStop, 0, 1)
			;
	}
	
};


class Puma : public ExpertAdvisor {
	
	int  Slippage =  2;
	int  MagicNumber =  123;
	double LotSize      =  0.01;
	int  MA_Shift =  0;
	
	bool UseHourTrade = false;
	int  GMTOffSet   =  1;
	int FromHourTrade = 6;
	int ToHourTrade = 18;
	String  LatOrderStoppedOut = " --Delay time--";
	bool     UseDelay = true;
	int     MinutesToDelay = 5;
	String  AddComment     =  "";
	
	int BL_ticket;
	int SL_ticket;
	int totalOrders;
	int HasBuyLimitOrder = 0;
	int HasSellLimitOrder = 0;
	int HasBuyOrder = 0;
	int HasSellOrder = 0;
	double AveragePrice;
	double stoploss;
	bool StoppedOut = false;
	Time StopTime;
	
	double LimitEntry;
	double MA_distance;
	double MinStopLoss;
	double   stoplossReal;
	double BL_openprice;
	double SL_openprice;
	double MA_currentbar;
	double spread;
	double lots;
	int OrderAttemps = 3;
	bool     BuyLimitOrderEntered = false;
	double   BuyLiMitOpenprice = 0.0;
	bool     SellLimitOrderEntered = true;
	double   SellLiMitOpenprice = 0.0;

	int TakeProfit = 20;
	int StopLoss = 290;
	int MA_Period = 16;
	int UseMoneyManagement = true;
	int Risk = 1;
	int MA_PipsAway = 20;
	
public:
	typedef Puma CLASSNAME;
	Puma();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int CloseOrder(int ticket, double numLots, double price);
	void Close_B(int ticket, double lot);
	void Close_S(int ticket, double lot);
	void Modify_order();
	void OpenLimitOrder();
	double GetLots();
	void OpenBuyOrder();
	void OpenSellOrder();
	void OpenMarketOrder();
	bool LastTradeStoppedOut();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("StopLoss", StopLoss, 0, 300)
			% Arg("MA_Period", MA_Period, 0, 100)
			% Arg("UseMoneyManagement", UseMoneyManagement, 0, 1)
			% Arg("Risk", Risk, 1, 10)
			% Arg("MA_PipsAway", MA_PipsAway, 1, 100)
			;
	}
	
};



class Rose : public ExpertAdvisor {
	
	String INFO1 = "Apply On EURUSD H1 Only";
	String INFO2 = "Account Balance From $250 - $250000";
	String INFO3 = "Select Appropriate Money Management Type by Inserting 1";
	int gi_112 = 0;
	int gi_116 = 0;
	String INFO4 = "If ECN broker select true, otherwise select false";
	bool ECN = false;
	bool microlot = true;
	int gi_unused_136 = 10;
	bool gi_unused_140 = true;
	int gi_unused_144 = 5;
	int g_magic_148 = 33301;
	int g_magic_152 = 33302;
	int g_magic_156 = 33303;
	int g_magic_160 = 33304;
	int g_magic_164 = 33305;
	int g_magic_168 = 33306;
	int gi_172 = 1;
	int gi_176 = 0;
	int gi_180 = 0;
	int gi_184 = 0;
	int gi_188 = 0;
	int gi_192 = 1;
	int gi_196 = 0;
	int gi_200 = 0;
	int gi_204 = 0;
	int gi_208 = 0;
	bool gi_212 = true;
	double gd_216 = 1.42;
	double gd_224 = 0.01;
	double gd_288 = 0.01;
	double gd_296 = 70.0;
	double gd_304 = 70.0;
	int g_slippage_344 = 8;
	double gd_348 = 246.0;
	bool gi_356 = true;
	int gi_unused_360 = 50;
	double gd_364 = 2.72;
	double gd_372 = 0.0;
	double gd_380 = 0.0;
	int gi_unused_388 = 10;
	int gi_unused_392 = 10;
	int g_ticket_396 = 0;
	int gi_unused_400 = 0;
	int gi_404 = 10;
	double gd_408 = 100.0;
	Time gi_420;
	double gd_424 = 0.0;
	int g_ticket_432 = 0;
	double gd_436 = 0.0;
	int gi_444 = 0;
	int gi_448 = 10;
	int gi_452 = 10;
	int gi_456 = 10;
	int gi_460 = 10;
	int gi_464 = 10;
	int gi_unused_468 = 10;
	int gi_472 = 10;
	int gi_476 = 10;
	int gi_480 = 10;
	int gi_484 = 10;
	int gi_488 = 10;
	int gi_unused_492 = 10;
	int gi_496 = 0;
	int gi_500 = 0;
	double gd_504 = 0.0;
	double gd_512 = 0.0;
	double gd_520 = 0.0;
	double gd_528 = 0.0;
	double g_ord_open_price_536 = 0.0;
	double g_ord_open_price_544 = 0.0;
	double g_ord_open_price_552 = 0.0;
	double g_ord_open_price_560 = 0.0;
	bool gi_unused_568 = true;
	bool gi_unused_572 = true;
	int gi_unused_576 = 0;
	int gi_unused_580 = 10;
	int gi_unused_584 = 10;
	double gd_588 = 0.0;
	double g_close_596 = 0.0;
	double gd_604 = 0.0;
	double gd_612 = 0.0;
	double gd_628 = 0.01;
	double gd_636 = 100.0;
	int gi_644 = 2;
	int gi_648 = 1;
	double g_lots_652 = 0.1;
	double gd_660 = 0.0;
	double gd_668 = 0.0;
	Time g_time_676;
	Time g_time_680;
	int gi_unused_684 = 0;
	double gd_688 = 3.0;
	double gd_696 = 1.5;
	double gd_704 = 10.0;
	double gd_712 = 10.0;
	double gd_720 = 10.0;
	double gd_728 = 10.0;
	int g_bars_736 = 10;
	int g_bars_740 = 10;
	int gi_unused_744 = 10;
	int gi_unused_748 = 10;
	int g_bars_752 = 0;
	int g_bars_756 = 0;
	int gi_unused_760 = 0;
	int g_bars_764 = 0;
	int g_bars_768 = 0;
	bool gi_unused_772 = false;
	bool gi_unused_776 = false;
	int gi_unused_780 = 0;
	int gi_unused_784 = 0;
	int gi_unused_788 = 0;
	int gi_unused_792 = 0;
	int gi_unused_796 = 0;
	int gi_unused_800 = 0;
	int gi_unused_804 = 0;
	int gi_808 = 10;
	int gi_812 = 10;
	int gi_816 = 10;
	int gi_820 = 10;
	
	int CONSERVATIVE = 0;
	int MODERATE = 0;
	int AGGRESSIVE = 0;
	int g_pips_252 = 36.0;
	int g_pips_260 = 1000.0;
	int g_pips_268 = 56.0;
	int g_pips_276 = 86.0;
	int gd_236 = 1000.0;
	int gd_244 = 90.0;
	int gi_620 = 30;
	int gi_624 = 10;
	int gi_416 = 100;
	int gi_232 = 5;
	int gi_284 = 126;
	int g_pips_312 = 46.0;
	int g_pips_320 = 1000.0;
	int g_pips_328 = 46.0;
	int g_pips_336 = 46.0;
	
public:
	typedef Rose CLASSNAME;
	Rose();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int CountOpenPositions();
	int Minus();
	int Loss();
	double RightLots(double lots);
	double ND(double price);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("CONSERVATIVE", CONSERVATIVE, 0, 1)
			% Arg("MODERATE", MODERATE, 0, 1)
			% Arg("AGGRESSIVE", AGGRESSIVE, 0, 1)
			% Arg("g_pips_252", g_pips_252, 0, 100)
			% Arg("g_pips_260", g_pips_260, 500, 1500)
			% Arg("g_pips_268", g_pips_268, 0, 100)
			% Arg("g_pips_276", g_pips_276, 0, 150)
			% Arg("gd_236", gd_236, 500, 1500)
			% Arg("gd_244", gd_244, 50, 150)
			% Arg("gi_620", gi_620, 0, 100)
			% Arg("gi_624", gi_624, 0, 100)
			% Arg("gi_416", gi_416, 0, 200)
			% Arg("gi_232", gi_232, 0, 100)
			% Arg("gi_284", gi_284, 0, 200)
			% Arg("g_pips_312", g_pips_312, 0, 100)
			% Arg("g_pips_320", g_pips_320, 500, 1500)
			% Arg("g_pips_328", g_pips_328, 0, 100)
			% Arg("g_pips_336", g_pips_336, 100)
			;
	}
	
};




class Thief : public ExpertAdvisor {
	
	
	int gi_unused_76 = 0;
	int Type = 0;
	double MaxLossPercent = 100.0;
	double RiskPercent = 300.0;
	double LotIncrement = 1.6;
	int OrderToIncLots = 1;
	int Magic = 770;
	
	int g_slippage_140 = 0;
	double gd_144;
	int gi_unused_152 = 0;
	int gi_unused_156 = 0;
	int gi_unused_160 = 0;
	int gi_unused_164 = 0;
	int g_count_168 = 0;
	double g_lots_172;
	String gs_180 = "lblfinPER_";

	int TakeProfit = 10;
	int StopLoss = 2500;
	int MinProfit = 5;
	int TrailingStop = 0;
	int UseTrail = false;
	int BEPoints = 30;
	
public:
	typedef Thief CLASSNAME;
	Thief();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int TotalBuy();
	int TotalSell();
	int BuyStopsTotal();
	int SellStopsTotal();
	void SetSellStop(double a_price_0, int ai_8, int ai_12, double a_lots_16);
	void SetBuyStop(double a_price_0, int ai_8, double a_pips_12, double a_lots_20);
	double ND(double ad_0);
	void TrailSellStop();
	void TrailBuyStop();
	void OpenSell();
	void OpenBuy();
	void GetBuyParameters(double &a_ord_open_price_0, double &a_ord_takeprofit_8, double &a_ord_stoploss_16, double &a_ord_lots_24);
	void GetSellParameters(double &a_ord_open_price_0, double &a_ord_takeprofit_8, double &a_ord_stoploss_16, double &a_ord_lots_24);
	void GetSellStopParameters(double &a_ord_open_price_0, double &a_ord_takeprofit_8, double &a_ord_stoploss_16, double &a_ord_lots_24);
	void GetBuyStopParameters(double &a_ord_open_price_0, double &a_ord_takeprofit_8, double &a_ord_stoploss_16, double &a_ord_lots_24);
	void DeleteAllSellStops();
	void DeleteAllBuyStops();
	void DoTrail();
	int GetLastClosedOrderType();
	double LotsOptimized();
	double GetProfitForDay(int ai_0);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("StopLoss", StopLoss, 0, 3000)
			% Arg("MinProfit", MinProfit, 0, 100)
			% Arg("TrailingStop", TrailingStop, 0, 100)
			% Arg("UseTrail", UseTrail, 0, 1)
			% Arg("BEPoints", BEPoints, 0, 100)
			;
	}
	
};






class Roulette : public ExpertAdvisor {
	
	double gd_108;
	double SizeLot = 0.1;
	bool OpenPosition = true;
	int gi_unused_144 = 1;
	int gi_unused_148 = 6;
	int gi_152 = 50;
	double gd_156 = 2.0;
	double gd_164 = 3.0;
	double gd_172 = 3.0;
	double gd_180 = 2.0;
	double gd_188 = 5.0;
	double gd_196 = 5.0;
	int gi_204 = 2003;
	int gi_208 = 1;
	int gi_212 = 2050;
	int gi_216 = 12;
	int gi_unused_220 = 22;
	int gi_unused_224 = 30;
	int gi_228 = 0;
	int gi_232 = 12;
	int gi_236 = 0;
	int g_count_240 = 0;
	int g_pos_244 = 0;
	int g_slippage_248 = 5;
	double g_price_252 = 0.0;
	double g_price_260 = 0.0;
	double g_ask_268 = 0.0;
	double g_bid_276 = 0.0;
	double gd_284 = 0.0;
	double g_lots_292 = 0.0;
	int g_cmd_300 = OP_BUY;
	int gi_304 = 0;
	int gi_unused_308 = 0;
	bool gi_312 = true;
	double g_ord_open_price_316 = 0.0;
	int gi_unused_324 = 0;
	double gd_unused_328 = 0.0;
	int gi_unused_336 = 0;
	int gi_unused_340 = 0;
	double gd_unused_344 = 0.0;
	double gd_unused_352 = 0.0;
	double gd_unused_360 = 0.0;
	double gd_368 = 0.0;
	String gs_376 = "";
	String gs_unused_384 = "";
	int gi_unused_392 = 16711680;
	double gda_396[2][6];
	
	int TakeProfit = 15.0;
	int StopLoss = 0.0;
	int MaxTrades = 20;
	int Pips = 30;
	
public:
	typedef Roulette CLASSNAME;
	Roulette();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("StopLoss", StopLoss, 0, 150)
			% Arg("MaxTrades", MaxTrades, 0, 100)
			% Arg("Pips", Pips, 0, 100)
			;
	}
	
};





class Sculptor : public ExpertAdvisor {
	
	
	double Lots = 0.01;
	double MaxLots = 2.0;
	double timestart1 = 0.0;
	double timestop1 = 24.0;
	double timestart2 = 0.0;
	double timestop2 = 24.0;
	bool gi_140 = false;
	bool Closeorders = false;
	bool Sunday = true;
	bool Monday = true;
	bool Tuesday = true;
	bool Wednesday = true;
	bool Thursday = true;
	bool Friday = true;
	bool Saturday = true;
	int magic = 7043;
	bool gi_188 = false;
	bool gi_192 = false;
	bool gi_196 = false;
	double g_lots_200 = 0.0;
	
	int level2 = 22.0;
	int profit2 = 20.0;
	int level3 = 22.0;
	int profit3 = 20.0;
	int level4 = 22.0;
	int profit4 = 20.0;
	int level5 = 22.0;
	int profit5 = 20.0;
	int level6 = 22.0;
	int profit6 = 20.0;
	int level7 = 22.0;
	int profit7 = 20.0;
	int level8 = 22.0;
	int profit8 = 20.0;
	int level9 = 22.0;
	int profit9 = 20.0;
	int level10 = 22.0;
	int profit10 = 20.0;
	int level11 = 22.0;
	int profit11 = 22.0;
	//bool gi_368 = false;

	int Profit = 35.0;
	int Stop_Los = 1000.0;
	int Closeprocent = 10.0;
	
public:
	typedef Sculptor CLASSNAME;
	Sculptor();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int sum_buy();
	int sum_sell();
	double getDayProfit(int ai_0, int ai_4);
	double getDayProfitall(int ai_0, int ai_4);
	int LevelMM(double ad_0);
	int ProfitMM(double ad_0);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("Profit", Profit, 0, 150)
			% Arg("Stop_Los", Stop_Los, 100, 1000)
			% Arg("Closeprocent", Closeprocent, 0, 100)
			% Arg( "level2",  level2, 0, 100)
			% Arg("profit2", profit2, 0, 100)
			% Arg( "level3",  level3, 0, 100)
			% Arg("profit3", profit3, 0, 100)
			% Arg( "level4",  level4, 0, 100)
			% Arg("profit4", profit4, 0, 100)
			% Arg( "level5",  level5, 0, 100)
			% Arg("profit5", profit5, 0, 100)
			% Arg( "level6",  level6, 0, 100)
			% Arg("profit6", profit6, 0, 100)
			% Arg( "level7",  level7, 0, 100)
			% Arg("profit7", profit7, 0, 100)
			% Arg( "level8",  level8, 0, 100)
			% Arg("profit8", profit8, 0, 100)
			% Arg( "level9",  level9, 0, 100)
			% Arg("profit9", profit9, 0, 100)
			% Arg( "level10",  level10, 0, 100)
			% Arg("profit10", profit10, 0, 100)
			% Arg( "level11",  level11, 0, 100)
			% Arg("profit11", profit11, 0, 100)
			;
	}
	
};







class Starter : public ExpertAdvisor {
	//int gi_76 = 3005838;
	double Multiplier = 2.0;
	double RiskPercent = 5.0;
	int Distance = 15;
	int TakeProfit = 20;
	int StopLoss = 20;
	int TrailingStep = 1;
	int TrailingStop = 15;
	int Magic = 605;
	bool ShowTableOnTesting = true;
	double gd_124 = 0.1;
	int g_slippage_132 = 0;
	double gd_136;
	int gi_unused_144 = 0;
	int gi_unused_148 = 0;
	String gs_152 = "lblfin7_";

	
public:
	typedef Starter CLASSNAME;
	Starter();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int TotalBuy();
	int TotalSell();
	int BuyStopsTotal();
	int SellStopsTotal();
	void DeleteAllPending();
	void SetSellStop(double a_price_0, double a_lots_8);
	void SetBuyStop(double a_price_0, double a_lots_8);
	double ND(double ad_0);
	void GetLastOrderParameters(double &a_ord_lots_0, double &a_ord_profit_8);
	void DoTrail();
	double GetProfitForDay(int ai_0);
	double LotsOptimized();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("StopLoss", StopLoss, 0, 100)
			% Arg("TrailingStep", TrailingStep, 1, 10)
			% Arg("TrailingStop", TrailingStop, 0, 100)
			;
	}
	
};






class Turtle : public ExpertAdvisor {
	
	String gs_80 = "FRX_";
	String gsa_88[7];
	bool gi_unused_92 = true;
	bool gi_96 = false;
	double gd_100 = 0.01;
	String _ = "Ïàðàìåòð ðèñêà (ëîò îò Áàëàíñà)";
	double RiskPercent = 2.0;
	String __ = "Êîëè÷åñòâî òîðã.ñëî¸â (<4)";
	int TorgSloy = 3;
	String ___ = "Êîëè÷åñòâî îðäåðîâ ïðåäûä.ñëîÿ äî âêë.ñëåä.";
	double g_maxlot_148 = 0.0;
	String ____ = "ìíîæèòåëü ñëåä ëîòà";
	double LotMultiplicator = 1.2;
	String _____ = "Ðàññòîÿíèå äî ñëåä. îðäåðà";
	int gi_unused_184 = 30;
	String ______ = "hSETKY (0-const 1-óâ. 2-óì.)";
	int Uvel_hSETKY = 1;
	String _______ = "øàã óâåëè÷.ñåòêè";
	int ShagUvel_hSETKY = 2;
	int gi_212 = 0;
	int gi_216 = 0;
	String ________ = "Ïðèáûëü îò çàêðûòèÿ ãðóïïû";
	int gi_unused_236 = 1;
	double gd_unused_240 = 40.0;
	String __________ = "Èäåíòèôèêàòîð îðäåðîâ";
	int Magic = 1230;
	String ____________ = "ëîãîòèï è âûâîä äàííûõ";
	bool ShowTableOnTesting = true;
	String _____________ = "(true-âêë.,false-âûêë.)";
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
	   13959039, 17919};
	String gsa_396[3][2] = {"NextBUY_0", "NextSELL_0",
	   "NextBUY_1", "NextSELL_1",
	   "NextBUY_2", "NextSELL_2"};
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

	int N_enable_Sloy = 7;
	int hSETKY = 30;
	int ProtectionTP = 7;
	int gi_232 = 7;
	
public:
	typedef Turtle CLASSNAME;
	Turtle();
	
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
		
		reg
			% Arg("N_enable_Sloy", N_enable_Sloy, 0, 100)
			% Arg("hSETKY", hSETKY, 0, 100)
			% Arg("ProtectionTP", ProtectionTP, 0, 100)
			% Arg("gi_232", gi_232, 0, 100)
			;
	}
	
};






class Unreal : public ExpertAdvisor {
	
	
	int Magic_number = 197125;
	bool info = false;
	bool DoubleOne = false;
	bool SecondSide = true;
	bool Abs0 = false;
	double Vremya = 0.0;
	bool gi_104 = false;
	bool QQE = true;
	double Lot = 0.1;
	int DigitsAfterDot = 2;
	double Multiplier = 2.0;
	double MultiplierSS = 0.0;
	bool gi_156 = false;
	bool gi_160 = false;
	bool gi_unused_164 = false;
	bool gi_168 = false;
	bool gi_172 = false;
	bool gi_176 = false;
	double gd_180 = 0.0;
	double gd_188 = 0.0;
	double gd_196 = 0.0;
	double gd_204 = 0.0;
	double gd_212 = 0.0;
	double gd_220 = 0.0;
	double gd_228 = 0.0;
	double gd_236 = 0.0;
	bool gi_244;
	bool gi_248;
	int gi_unused_252 = 0;
	int g_ticket_256;
	int g_count_260;
	int g_pos_264;
	Time gi_268;
	int g_count_272;
	String gs_276;
	double gd_296;
	double gd_308;
	double g_lots_316;
	int gi_332;

	int TP = 10;
	int MinStep = 50;
	int MaxTrades = 7;
	int SF = 5;
	
public:
	typedef Unreal CLASSNAME;
	Unreal();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double FindLastBuyPrice();
	double FindLastSellPrice();
	double FindLastOrder(String as_0, String as_8);
	int OpenBuy();
	int OpenSell();
	int OpenALL();
	double Balance(String as_0, String as_8);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TP", TP, 0, 100)
			% Arg("MinStep", MinStep, 0, 100)
			% Arg("MaxTrades", MaxTrades, 1, 100)
			% Arg("SF", SF, 0, 100)
			;
	}
	
};






class Hippie : public ExpertAdvisor {
	
	double Lots = 1.0;
	double Risk = 5.0;
	double Slippage = 2.0;
	double MagicNumber = 12225.0;
	double g_point_148;
	bool gi_156;
	
	int TakeProfit = 25.0;
	int Stoploss = 150.0;
	int TrendPower = 40.0;
	int TrendLevel = 300.0;
	int Sensitivity = 41.0;
	int osma_period = 10;
	int macd_period = 10;
	
	
public:
	typedef Hippie CLASSNAME;
	Hippie();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("Stoploss", Stoploss, 0, 200)
			% Arg("TrendPower", TrendPower, 0, 100)
			% Arg("TrendLevel", TrendLevel, 0, 500)
			% Arg("Sensitivity", Sensitivity, 2, 100)
			% Arg("osma_period", osma_period, 2, 100)
			% Arg("macd_period", macd_period, 2, 100)
			;
	}
	
};






class ProfitChance : public ExpertAdvisor {
	
	double RiskPercent = 25.0;
	double LotMultiplicator = 1.4;
	int TakeProfit = 30;
	int ProtectionTP = 10;
	int Magic = 615;
	int Slippage = 1;
	bool ShowTableOnTesting = true;
	int gd_116 = 100.0;
	int gi_124 = 0;
	double gd_128;
	int gi_136;
	String gs_140 = "lblfin_";
	int gi_148 = 1;
	int gi_152 = 1;
	int gi_156 = 1;
	double g_minlot_160 = 0.1;
	bool gi_168;
	bool gi_172;
	bool gi_176;
	bool gi_180;
	
public:
	typedef ProfitChance CLASSNAME;
	ProfitChance();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int TotalBuy(int a_magic_0);
	int TotalSell(int a_magic_0);
	int BuyStopsTotal(int a_magic_0);
	int SellStopsTotal(int a_magic_0);
	int BuyLimitsTotal(int a_magic_0);
	int SellLimitsTotal(int a_magic_0);
	void DeleteAllPending(int a_magic_0);
	void SetSellStop(double a_price_0, double a_lots_8, int ai_16, int ai_20, int a_magic_24);
	void SetBuyStop(double a_price_0, double a_lots_8, int ai_16, int ai_20, int a_magic_24);
	void SetSellLimit(double a_price_0, double a_lots_8, int ai_16, int ai_20, int a_magic_24);
	void SetBuyLimit(double a_price_0, double a_lots_8, int ai_16, int ai_20, int a_magic_24);
	void OpenBuy(double a_lots_0, int ai_8, int a_magic_12);
	void OpenSell(double a_lots_0, int ai_8, int a_magic_12);
	void TrailBuyTp(double a_price_0, int a_magic_8);
	void TrailBuySl(double a_price_0, int a_magic_8);
	void TrailSellTp(double a_price_0, int a_magic_8);
	void TrailSellSl(double a_price_0, int a_magic_8);
	double GetWeightedBELevel(int ai_0);
	double GetWeightedPrice(int a_magic_0);
	double GetSellLotsSum(int a_magic_0);
	double GetBuyLotsSum(int a_magic_0);
	void GetHighestBuyParameters(double &a_ord_takeprofit_0, double &a_ord_stoploss_8, double &a_ord_open_price_16, double &a_ord_lots_24, int a_magic_32);
	void GetLowestSellParameters(double &a_ord_takeprofit_0, double &a_ord_stoploss_8, double &a_ord_open_price_16, double &a_ord_lots_24, int a_magic_32);
	double GetProfitForDay(int ai_0);
	double LotsOptimized();
	void CloseAllSell();
	void CloseAllBuy();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("ProtectionTP", ProtectionTP, 0, 100)
			% Arg("gd_116", gd_116, 0, 100)
			;
	}
	
};






class Rabbit : public ExpertAdvisor {
	
	String Comment_MultiLotsFactor = "Êîýôô. óâåëè÷åíèÿ ëîòà äëÿ âûâîäà â áåç óáûòîê";
	double MultiLotsFactor = 1.6;
	double Lots = 0.01;
	String Comment_SafeEquity = "Çàïðåòèòü îòêðûòèå îðäåðîâ ïðè Equity ìåíüøå SafeEquityRisk %";
	bool SafeEquity = false;
	double SafeEquityRisk = 20.0;
	String Comment_slippage = "ðàçðåøåííîå ïðîñêàëüçûâàíèå öåíû â ïèïñàõ";
	double slippage = 3.0;
	int MagicNumber = 13579;
	bool gi_220 = false;
	double gd_224 = 48.0;
	double g_pips_232 = 500.0;
	double gd_240 = 0.0;
	bool gi_248 = true;
	bool gi_252 = false;
	int gi_256 = 1;
	double g_price_260;
	double gd_268;
	double gd_unused_276;
	double gd_unused_284;
	double g_price_292;
	double g_bid_300;
	double g_ask_308;
	double gd_316;
	double gd_324;
	double gd_332;
	bool gi_340;
	String gs_344 = "WULF_AUTO";
	int g_time_352 = 0;
	Time gi_356;
	int gi_360 = 0;
	double gd_364;
	int g_pos_372 = 0;
	int gi_376;
	double gd_380 = 0.0;
	bool gi_388 = false;
	bool gi_392 = false;
	bool gi_396 = false;
	int gi_400;
	bool gi_404 = false;
	Time g_datetime_408;
	Time g_datetime_412;
	double gd_416;
	double gd_424;

	int TakeProfit = 19.0;
	int StepLots = 5.0;
	int UseTrailing = false;
	int TrailStart = 35.0;
	int TrailStop = 49.0;
	int MaxCountOrders = 10;
	
public:
	typedef Rabbit CLASSNAME;
	Rabbit();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double ND(double ad_0);
	int fOrderCloseMarket(bool ai_0 = true, bool ai_4 = true);
	double fGetLots(int a_cmd_0);
	int CountTrades();
	void CloseThisSymbolAll();
	int OpenPendingOrder(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_32, int ai_36, String a_comment_40, int a_magic_48);
	double StopLong(double ad_0, int ai_8);
	double StopShort(double ad_0, int ai_8);
	double TakeLong(double ad_0, int ai_8);
	double TakeShort(double ad_0, int ai_8);
	double CalculateProfit();
	void TrailingAlls(int ai_0, int ai_4, double a_price_8);
	double AccountEquityHigh();
	double FindLastBuyPrice();
	double FindLastSellPrice();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TakeProfit", TakeProfit, 0, 100)
			% Arg("StepLots", StepLots, 1, 100)
			% Arg("UseTrailing", UseTrailing, 0, 1)
			% Arg("TrailStart", TrailStart, 0, 100)
			% Arg("TrailStop", TrailStop, 0, 100)
			% Arg("MaxCountOrders", MaxCountOrders, 0, 100)
			;
	}
	
};








class Midday : public ExpertAdvisor {
	
		
	double timestart1 = 0.0;
	double timestop1 = 24.0;
	double timestart2 = 0.0;
	double timestop2 = 24.0;
	double g_lots_232 = 0.0;
	double Lots = 0.01;
	bool gi_172 = false;
	bool Closeorders = false;
	double Closeprocent = 10.0;
	bool Sunday = true;
	bool Monday = true;
	bool Tuesday = true;
	bool Wednesday = true;
	bool Thursday = true;
	bool Friday = true;
	bool Saturday = true;
	int magic = 7043;
	bool gi_220 = false;
	bool gi_224 = false;
	bool gi_228 = false;
	bool gi_400 = false;

	int MaxLots = 2.0;
	int Profit = 35.0;
	int Stop_Los = 1000.0;
	int level2 = 22.0;
	int profit2 = 20.0;
	int level3 = 22.0;
	int profit3 = 20.0;
	int level4 = 22.0;
	int profit4 = 20.0;
	int level5 = 22.0;
	int profit5 = 20.0;
	int level6 = 22.0;
	int profit6 = 20.0;
	int level7 = 22.0;
	int profit7 = 20.0;
	int level8 = 22.0;
	int profit8 = 20.0;
	int level9 = 22.0;
	int profit9 = 20.0;
	int level10 = 22.0;
	int profit10 = 20.0;
	int level11 = 22.0;
	int profit11 = 22.0;
	
public:
	typedef Midday CLASSNAME;
	Midday();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int sum_buy();
	int sum_sell();
	double getDayProfit(int ai_0, int ai_4);
	double getDayProfitall(int ai_0, int ai_4);
	int LevelMM(double ad_0);
	int ProfitMM(double ad_0);
	bool myCheckAllowWorking();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("Profit", Profit, 0, 100)
			% Arg("Stop_Los", Stop_Los, 0, 1000)
			% Arg( "level2",  level2, 0, 100)
			% Arg("profit2", profit2, 0, 100)
			% Arg( "level3",  level3, 0, 100)
			% Arg("profit3", profit3, 0, 100)
			% Arg( "level4",  level4, 0, 100)
			% Arg("profit4", profit4, 0, 100)
			% Arg( "level5",  level5, 0, 100)
			% Arg("profit5", profit5, 0, 100)
			% Arg( "level6",  level6, 0, 100)
			% Arg("profit6", profit6, 0, 100)
			% Arg( "level7",  level7, 0, 100)
			% Arg("profit7", profit7, 0, 100)
			% Arg( "level8",  level8, 0, 100)
			% Arg("profit8", profit8, 0, 100)
			% Arg( "level9",  level9, 0, 100)
			% Arg("profit9", profit9, 0, 100)
			% Arg( "level10",  level10, 0, 100)
			% Arg("profit10", profit10, 0, 100)
			% Arg( "level11",  level11, 0, 100)
			% Arg("profit11", profit11, 0, 100)
			;
	}
	
};








class FxOne : public ExpertAdvisor {
	
	
	double BaseLot = 0.01;
	double Multiplier = 2.0;
	int MaxTrades = 7;
	String Serial = "demo";
	int MagicNumber = 10777;
	double gd_108;
	int gi_116 = 2;
	double gd_136 = 3.0;
	double g_price_144;
	double gd_152;
	double gd_160;
	double g_price_168;
	double gd_176;
	double gd_184;
	double gd_192;
	bool gi_200;
	String gs_fxdiler_204 = "fxdiler";
	int g_time_212 = 0;
	int gi_216 = 0;
	double gd_220;
	double g_lotstep_228;
	int g_pos_236 = 0;
	int gi_240;
	bool gi_244 = false;
	bool gi_248 = false;
	bool gi_252 = false;
	int gi_256;
	bool gi_260 = false;
	double g_ihigh_264;
	double g_ilow_272;
	bool gi_280 = false;
	bool gi_284 = false;
	int gi_288 = 5;
	String gs_eurusd_292 = "EURUSD";
	bool gi_340 = false;
	bool gi_344 = false;
	int gi_348 = 900;
	Time g_datetime_352;
	int gi_356 = 0;
	
	int rsi_period = 10;
	int g_pips_120 = 10.0;
	int g_pips_128 = 30.0;
	
public:
	typedef FxOne CLASSNAME;
	FxOne();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int f0_0();
	int f0_1(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_32, int ai_36, String a_comment_40, int a_magic_48, int a_datetime_52);
	double f0_2(double ad_0, int ai_8);
	double f0_3(double ad_0, int ai_8);
	double f0_4(double ad_0, int ai_8);
	double f0_5(double ad_0, int ai_8);
	double f0_6();
	double f0_7();
	double f0_8();
	String f0_9(String as_0);
	void f0_10();
	int f0_11(String as_0);
	int f0_12(String a_name_0, int a_corner_8, int a_x_12, int a_y_16, String a_text_20, int a_fontsize_28);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("rsi_period", rsi_period, 2, 100)
			% Arg("g_pips_120", g_pips_120, 0, 100)
			% Arg("g_pips_128", g_pips_128, 0, 100)
			;
	}
	
};






class Sunrise : public ExpertAdvisor {
	
	//ÂÍÈÌÀÍÈÅ!!! ÌÀÐÒÈÍÃÅÉË!!!
	String Attention = "ÂÍÈÌÀÍÈÅ!!! ÌÀÐÒÈÍÃÅÉË!!!";
	String HeadLine = "Âåðñèÿ ñîâåòíèêà îò 17.03.2009";
	String BuyBlock = "-------Ïàðàìåòðû äëÿ ïîêóïêè-------";
	
	double Buy_PipStepExponent = 1.1; //åñëè íóæåí ïðîãðåññèâíûé øàã
	double Buy_LotExponent = 1.69;  //ïðîãðåññèâíûé ëîò
	double Buy_LotSize = 0.01;      //ðàçìåð ëîòà
	
	String SellBlock = "-------Ïàðàìåòðû äëÿ ïðîäàæè-------";
	
	double Sell_PipStepExponent = 1.5;
	double Sell_LotExponent = 1.69;
	double Sell_LotSize = 0.01;
	
	//String IndBlock = "-------Ïàðàìåòðû èíäèêàòîðîâ-------";
	
	//bool UseInd = false;
	//...
	
	String GeneralBlock = "-------Îáùèå ïàðàìåòðû-------";
	
	int OpenNewTF = 1;          //òàéìôðåì äëÿ îòêðûòèÿ íîâûõ îðäåðîâ
	int OpenNextTF = 15;        //òàéìôðåéì äëÿ îòêðûòèÿ óñðåäíÿþùèõ îðäåðîâ
	bool BackBar = false;       //èñïîëüçîâàòü èëè íåò ðàçâîðîòíûé áàð äëÿ óñðåäíåíèÿ
	int BackBarTF = 15;         //òàéìôðåéì íà êîòîðîì, ïðè âêëþ÷ííîì BackBar, áóäåò ïðîâåðÿòüñÿ ðàçâîðîòíûé áàð.
	bool FixLot = true;         //ôèêñèðîâàííûé èëè íåò ëîò.
	int LotStep = 1000;        //øàã óâåëè÷åíèÿ ëîòà. ò.å. ñêîëüêî â äåïîçèòå LotStep âîñòîëüêî óâåëè÷èòñÿ LotSize. åñëè äåïî 2000 òî ëîò 0.01, åñëè ñòàíåò 4000 òî ëîò 0.02
	
	String OfficialVariable = "-------Ñëóæåáíûå ïåðåìåíûå-------";
	String MaxAttemptsString = "MaxAttempts= Êîëè÷åñòâî ïîïûòîê îòêðûòèÿ îðäåðîâ";
	int MaxAttempts = 3; //êîë-âî ïîïûòîê îòêðûòü îðäåð íà ñëó÷àé îøèáîê è ïð.
	String InformationOnChartString = "InformationOnChart - îòêëþ÷èòü äëÿ òåñòåðà (çàìåäëÿåò)";
	bool InformationOnChart = true; //âûâîäèòü ëè èíôîðìàöèþ íà ãðàôèê, îòêëþ÷èòü äëÿ òåñòåðà (çàìåäëÿåò)
	int MagicNumber = 123456789; //ïî ýòîìó íîìåðó ïðîèñõîäèò èäåíòèôèêàöèÿ îðäåðîâ íà ïðåíàäëåæíîñòü ñîâåòíèêó
	
	Time timeprevMIN;
	Time timeprevMAX;
	int LotDecimal = 0;
	int total, ticket, CountTrades;
	bool LongTradeNew, ShortTradeNew;
	double Buy_NextLot, Sell_NextLot, Buy_NewLot, Sell_NewLot, Buy_LastLot, Sell_LastLot;
	String CommentTrades;
	
	int Buy_PipStep = 30;           //øàã ìåæäó äîëèâêàìè. êàæäûé øàã óâèëè÷èâàåòñÿ â PipStepExponent ðàç.
	int Buy_TP = 10;                //òýéêïðîôèò
	int Buy_MaxTrades = 5;         //ìàêñèìàëüíîå êîë-âî êîëåí
	int Sell_PipStep = 10;
	int Sell_TP = 10;
	int Sell_MaxTrades = 4;
	
	int pos;

public:
	typedef Sunrise CLASSNAME;
	Sunrise();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	void Norrect(String OrdType);
	double NewLot(String OrdType);
	double NextLot(String OrdType);
	bool NextOrder(String OrdType);
	double FindLastOrder(String OrdType, String inf);
	int GetCountTrades(String OrdType);
	double GetAveragePrice(String OrdType);
	double Balance(String OrdType, String inf);
	double GetLotDecimal();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("Buy_PipStep", Buy_PipStep, 0, 100)
			% Arg("Buy_TP", Buy_TP, 0, 100)
			% Arg("Buy_MaxTrades", Buy_MaxTrades, 0, 100)
			% Arg("Sell_PipStep", Sell_PipStep, 0, 100)
			% Arg("Sell_TP", Sell_TP, 0, 100)
			% Arg("Sell_MaxTrades", Sell_MaxTrades, 0, 100)
			;
	}
	
};






class YetAnother : public ExpertAdvisor {
	
	String gs_unused_76 = "---------------- Settings";
	int gi_84 = 3;
	int gi_88 = 2;
	String gs_unused_92 = "---------------- Settings";
	String gs_unused_108 = "Buy when between these and rising";
	int gi_116 = 50;
	int gi_120 = 56;
	String gs_unused_124 = "Sell when between these and decreasing";
	int gi_132 = 43;
	int gi_136 = 32;
	String gs_unused_140 = "---------------- 2nD Settings";
	String gs_unused_148 = "---------Show the trend";
	int g_period_156 = 10;
	String gs_unused_160 = "If Under-Over allow to trade";
	int gi_168 = 32;
	int gi_172 = 71;
	String gs_unused_180 = "If Under-Over allow to trade";
	int g_period_196 = 9;
	int gi_200 = 1;
	int gi_204 = -1;
	String S2 = "---------------- Money Management";
	double Lots = 0.1;
	bool RiskMM = false;
	double RiskPercent = 1.0;
	String S3 = "---------------- Order Management";
	bool HideSL = false;
	bool HideTP = false;
	int TrailingStop = 0;
	int TrailingStep = 0;
	int BreakEven = 0;
	int MaxOrders = 100;
	int Slippage = 3;
	int Magic = 170109;
	String S6 = "---------------- Extras";
	bool Hedge = false;
	bool ReverseSystem = false;
	Time g_time_300;
	Time g_time_304;
	double gd_unused_324 = 0.0;
	
	int g_period_104 = 9;
	int g_period_188 = 17;
	int g_period_192 = 26;
	int StopLoss = 0;
	int TakeProfit = 0;
	
	
public:
	typedef YetAnother CLASSNAME;
	YetAnother();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int CloseBuyOrders(int a_magic_0);
	int CloseSellOrders(int a_magic_0);
	void MoveTrailingStop();
	void MoveBreakEven();
	int NewBarBuy();
	int NewBarSell();
	void CalculateMM();
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("g_period_104", g_period_104, 2, 100)
			% Arg("g_period_188", g_period_188, 2, 100)
			% Arg("g_period_192", g_period_192, 2, 100)
			% Arg("StopLoss", StopLoss, 0, 100)
			% Arg("TakeProfit", TakeProfit, 0, 100)
			;
	}
	
};






class Hornet : public ExpertAdvisor {
	String Program = "TOPGUN FX";
	String Rev = "201";
	double Lots = 0.1;
	bool UseAutoLot = true;
	double MaxLots = 100;
	double MinLots = 0.01;
	double MaximumRisk = 3.0;
	int Slippage = 3;
	bool Aggressive = true;
	int Nanpin1 = 5;
	int Nanpin2 = 50;
	int FTLOrderWaitingTime = 10;
	int BuyLossPoint = 240;
	int SellLossPoint = 240;
	int LowToHigh = 400;
	int MAGIC1 = 321031;
	int MAGIC2 = 321032;
	int MAGIC3 = 321033;
	String gs_232 = "TOPGUN FX";
	int gi_unused_248;
	int gi_252 = 1;
	int gi_256 = 1;
	int gia_260[6] = {16711680, 255, 16711680, 255, 16711680, 255};
	
	int bb_period = 10;
	int bb_deviation = 10;
	
public:
	typedef Hornet CLASSNAME;
	Hornet();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double LotsOptimized();
	double FTLOrderOpenPrice(int a_magic_0);
	double FTLCurrentOrders(int ai_0, int a_magic_4);
	int FTLOrderSend(int a_cmd_0, double a_lots_4, double a_price_12, int a_slippage_20, double a_price_24, double a_price_32, String a_comment_40, int a_magic_48);
	int FTLOrderClose(int a_slippage_0, int a_magic_4);
	void FTLOrderSendSL(int ai_0, double ad_4, double ad_12, int ai_20, int ai_24, int ai_28, String as_32, int ai_40);
	
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("bb_period", bb_period, 2, 100)
			% Arg("bb_deviation", bb_deviation, 2, 100)
			% Arg("BuyLossPoint", BuyLossPoint, 2, 300)
			% Arg("SellLossPoint", SellLossPoint, 2, 300)
			;
	}
	
};






class Constellation : public ExpertAdvisor {
	
	String gs_80 = "TurboMax v.1.1";
	double gd_88 = 2.0;
	String gs_96 = "TurboMax_";
	String __1__ = " ÌÀÐÒÈÍ 1 - âêë. 2 - âûêë.";
	int MMType = 1;
	bool gi_116 = true;
	String __2__ = "ìíîæèòåëü ñëåä. ëîòà";
	double LotMultiplikator = 1.667;
	double gd_136;
	double g_slippage_144 = 5.0;
	String __3__ = "íà÷àëüíûé ëîò";
	double Lots = 0.01;
	String __4__ = "ïðèáûëü â ïóíêòàõ - ÒÐ";
	double TakeProfit = 5.0;
	double gd_184;
	double g_pips_192 = 0.0;
	String __5__ = "ðàññòîÿíèå ì/ó îðäåðàìè";
	double Step = 5.0;
	double gd_232;
	String __6__ = "ÌÀX êîë-âî îðäåðîâ";
	int MaxTrades = 30;
	String __7__ = "Îãðàíè÷åíèå ïîòåðü";
	bool UseEquityStop = false;
	double TotalEquityRisk = 20.0;
	bool gi_272 = false;
	bool gi_276 = false;
	bool gi_280 = false;
	double gd_284 = 48.0;
	bool gi_292 = false;
	int gi_296 = 2;
	int gi_300 = 16;
	String __8__ = "Èäåíòèôèêàòîð îðäåðà";
	int Magic = 1111111;
	int gi_316;
	String __9__ = "ëîãîòèï è âûâîä äàííûõ";
	bool ShowTableOnTesting = true;
	String _ = "(true-âêë.,false-âûêë.)";
	double g_price_340;
	double gd_348;
	double gd_unused_356;
	double gd_unused_364;
	double g_price_372;
	double g_bid_380;
	double g_ask_388;
	double gd_396;
	double gd_404;
	double gd_412;
	bool gi_420;
	int g_time_424 = 0;
	Time gi_428;
	int gi_432 = 0;
	double gd_436;
	int g_pos_444 = 0;
	int gi_448;
	double gd_452 = 0.0;
	bool gi_460 = false;
	bool gi_464 = false;
	bool gi_468 = false;
	int gi_472;
	bool gi_476 = false;
	Time g_datetime_480;
	Time g_datetime_484;
	double gd_488;
	double gd_496;

	int gd_200 = 10.0;
	int gd_208 = 10.0;
	
public:
	typedef Constellation CLASSNAME;
	Constellation();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	double f0_0(double ad_0);
	int f0_1(bool ai_0 = true, bool ai_4 = true);
	double f0_2(int a_cmd_0);
	int f0_3();
	void f0_4();
	int f0_5(int ai_0, double a_lots_4, double a_price_12, int a_slippage_20, double ad_24, int ai_unused_32, int ai_36, String a_comment_40, int a_magic_48, int a_datetime_52);
	double f0_6(double ad_0, int ai_8);
	double f0_7(double ad_0, int ai_8);
	double f0_8(double ad_0, int ai_8);
	double f0_9(double ad_0, int ai_8);
	double f0_10();
	void f0_11(int ai_0, int ai_4, double a_price_8);
	double f0_12();
	double f0_13();
	double f0_14();
	void f0_15();
	void f0_16();
	void f0_17();
	double f0_18(int ai_0);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("gd_200", gd_200, 0, 100)
			% Arg("gd_208", gd_208, 0, 100)
			;
	}
	
};




class Concorde : public ExpertAdvisor {
	
	String S0="Òåéê ïðîôèò â ïóíêòàõ.";
	int TP = 5;
	String S1="Ëîò(0- íå èñîëüçîâàòü).";
	double Lots = 0;
	String S2="Ëîò â % îò áàëàíñà(0- íå èñîëüçîâàòü).";
	double LotsProc=1;
	String S3="Ìèíèìàëüíîå ðàññòîÿíèå ìåæäó îðäåðàìè.";
	int Step=40;
	String S4="Òðåéëèíã ñòîï.";
	int TS=30;
	String S5="Êîýôôèöèåíò óâåëè÷åíèÿ ëîòà(0- óâåëè÷åíèå íà íà÷àëüíûé ëîò).";
	double Kef = 0;
	String S6="Ìàãè÷åñêîå ÷èñëî.";
	int Magic=777;
	String S7="Çíà÷åíèå ìàêñèìàëüíîãî ïðîñêàëüçûâàíèÿ.(Slippage)";
	int Slippage=3;

	int D = 1;
	
public:
	typedef Concorde CLASSNAME;
	Concorde();
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	int ModBuy();
	int ModSell();
	int Tral(int Ticket);
	int OpenOrder(String symbol, int cmd, double volume, double price, double stoploss, double takeprofit, String comment, int magic);
	int ModifyOrder(int ticket, double price, double stoploss, double takeprofit);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("TP", TP, 0, 100)
			% Arg("Step", Step, 0, 100)
			% Arg("TS", TS, 0, 100)
			;
	}
	
};







/*
	Skipped, too hard:
		- tengri
		- giga
		- fx equity builder
		- fx no loss
		- fx enforcer
		- fx voodoo
		- grizzly
		- large investor
		- tfp
		- utf8 steiniz
		- 10pipsmulti
		- indorun
		- dozer
*/
}

#endif
