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


class DqnAdvisor : public Core {
	
	struct TrainingCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	static const int other_syms = 8;
	
	
	
	static const int input_length = 30;
	static const int input_size = input_length * 1;
	static const int output_size = 2;
	typedef DQNTrainer<output_size, input_size> DQN;
	
	
	// Persistent
	DQN dqn;
	Vector<double> training_pts;
	Vector<int> cursors;
	double total = 0;
	int round = 0;
	int prev_counted = 0;
	
	
	// Temporary
	DQN::MatType tmp_mat;
	double point = 0.0001;
	int max_rounds = 0;
	bool once = true;
	
	void LoadInput(int pos);
protected:
	virtual void Start();
	
	bool TrainingBegin();
	bool TrainingIterator();
	bool TrainingEnd();
	bool TrainingInspect();
	void RefreshAll();
	void DumpTest();
	
public:
	typedef DqnAdvisor CLASSNAME;
	DqnAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&Filter)
			% Out(1, 1)
			% Out(0, 0)
			% Mem(dqn)
			% Mem(training_pts)
			% Mem(cursors)
			% Mem(total)
			% Mem(round)
			% Mem(prev_counted);
	}
	
	
	static bool Filter(void* basesystem, bool match_tf, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (match_tf)
			return in_tf == out_tf;
		else {
			if (in_sym == out_sym)
				return true;
			
			String sym = GetSystem().GetSymbol(out_sym);
			return
				sym == "EURJPY" || sym == "EURUSD" || sym == "GBPUSD" || sym == "USDCAD" ||
				sym == "USDJPY" || sym == "USDCHF" || sym == "AUDUSD" || sym == "EURAUD";
		}
	}
};

}

#endif
