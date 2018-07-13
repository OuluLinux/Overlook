#ifndef _Overlook_Advisors_h_
#define _Overlook_Advisors_h_

#undef MIN
#undef MAX

namespace Overlook {


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





class HurstAdvisor : public ExpertAdvisor {
	
	struct Hurst {
		bool types[4], dirs[4];
		
		void Add(bool type, bool dir) {
			for(int i = 1; i < 4; i++) {
				types[i] = types[i-1];
				dirs[i] = dirs[i-1];
			}
			types[0] = type;
			dirs[0] = dir;
		}
	};
	
	int prev_pos;
	int take_profit, stop_loss;
	
public:
	typedef HurstAdvisor CLASSNAME;
	HurstAdvisor();
	
	
	virtual void InitEA();
	virtual void StartEA(int pos);
	
	virtual void IO(ValueRegister& reg) {
		ExpertAdvisor::IO(reg);
		
		reg
			% Arg("take_profit", take_profit, 0, 100)
			% Arg("stop_loss", stop_loss, 0, 1000)
			;
	}
	
};

}

#endif
