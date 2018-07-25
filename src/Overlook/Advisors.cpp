#include "Overlook.h"

namespace Overlook {

Eagle::Eagle() {

}

void Eagle::InitEA() {


	AddSubCore<MovingAverageConvergenceDivergence>()
		.Set("fast_ema", macd_fast_ema)
		.Set("slow_ema", macd_slow_ema)
		.Set("signal_sma", macd_signal_sma)
		;

	AddSubCore<StochasticOscillator>()
		.Set("k_period", stoch0_k_period)
		.Set("d_period", stoch0_d_period)
		.Set("slowing", stoch0_slowing);

	AddSubCore<StochasticOscillator>()
		.Set("k_period", stoch1_k_period)
		.Set("d_period", stoch1_d_period)
		.Set("slowing", stoch1_slowing);

	AddSubCore<ParabolicSAR>()
		.Set("step", sar_step)
		.Set("maximum", sar_max);

	AddSubCore<Momentum>()
		.Set("period", mom_period);

}

void Eagle::StartEA(int pos) {
	String ls_0;

	if (pos < 1)
		return;

	if (use_trailing)
		TrailingPositions();

	HandleOpenPositions1();

	gi_408 = openPositions();

	if (gi_408 > 0)
		return;

	if (AccountFreeMargin() < 1000.0 * GetMoneyManagement()) {
		Print("Not enough margin= " + DblStr(AccountFreeMargin()));
		return;
	}

	if (CheckEntryConditionBUY(pos))
		OpenBuyOrder();

	if (CheckEntryConditionSELL(pos))
		OpenSellOrder();

}

bool Eagle::CheckExitCondition(String as_unused_0) {
	bool li_ret_8 = false;
	return (li_ret_8);
}

bool Eagle::CheckEntryConditionBUY(int pos) {

	if (use_macd) {
		Core& c = At(0);
		g_imacd_312 = c.GetBuffer(0).Get(pos);
		g_imacd_320 = c.GetBuffer(1).Get(pos);

		if (g_imacd_312 >= g_imacd_320)
			return (false);
	}

	if (use_stoch) {
		Core& c = At(1);
		g_istochastic_336 = c.GetBuffer(0).Get(pos);

		if (g_istochastic_336 >= stoch_hi_limit)
			return (false);
	}

	if (use_stoch2) {
		Core& c = At(2);
		g_istochastic_336 = c.GetBuffer(0).Get(pos);
		g_istochastic_344 = c.GetBuffer(0).Get(pos - 1);
		g_istochastic_352 = c.GetBuffer(1).Get(pos);
		g_istochastic_360 = c.GetBuffer(1).Get(pos - 1);

		if (g_istochastic_336 >= stoch_hi_limit)
			return (false);

		if (g_istochastic_352 >= g_istochastic_336)
			return (false);

		if (g_istochastic_360 <= g_istochastic_344)
			return (false);
	}

	if (use_sar) {
		Core& c = At(3);
		g_isar_368 = c.GetBuffer(0).Get(pos);

		if (g_isar_368 > Ask)
			return (false);

		g_isar_376 = c.GetBuffer(0).Get(pos - 1);

		if (g_isar_376 < g_isar_368)
			return (false);
	}

	if (use_momentum) {
		Core& c = At(4);
		g_imomentum_384 = c.GetBuffer(0).Get(pos);

		if (g_imomentum_384 >= mom_limit)
			return (false);
	}

	if (Fridaymode == true && DayOfWeek() == 5)
		return (false);

	return (true);
}

bool Eagle::CheckEntryConditionSELL(int pos) {
	if (use_macd) {
		Core& c = At(0);
		g_imacd_312 = c.GetBuffer(0).Get(pos);
		g_imacd_320 = c.GetBuffer(1).Get(pos);

		if (g_imacd_312 <= g_imacd_320)
			return (false);
	}

	if (use_stoch) {
		Core& c = At(1);
		g_istochastic_336 = c.GetBuffer(0).Get(pos);

		if (g_istochastic_336 <= stoch_hi_limit)
			return (false);
	}

	if (use_stoch2) {
		Core& c = At(2);
		g_istochastic_336 = c.GetBuffer(0).Get(pos);
		g_istochastic_344 = c.GetBuffer(0).Get(pos - 1);
		g_istochastic_352 = c.GetBuffer(1).Get(pos);
		g_istochastic_360 = c.GetBuffer(1).Get(pos - 1);

		if (g_istochastic_336 <= stoch_hi_limit)
			return (false);

		if (g_istochastic_352 <= g_istochastic_336)
			return (false);

		if (g_istochastic_360 >= g_istochastic_344)
			return (false);
	}

	if (use_sar) {
		Core& c = At(3);
		g_isar_368 = c.GetBuffer(0).Get(pos);

		if (g_isar_368 < Bid)
			return (false);

		g_isar_376 = c.GetBuffer(0).Get(pos - 1);

		if (g_isar_376 >= g_isar_368)
			return (false);
	}

	if (use_momentum) {
		Core& c = At(4);
		g_imomentum_384 = c.GetBuffer(0).Get(pos);

		if (g_imomentum_384 <= mom_limit)
			return (false);
	}

	if (Fridaymode == true && DayOfWeek() == 5)
		return (false);

	return (true);
}

void Eagle::OpenBuyOrder() {
	double l_price_0 = 0;
	double l_price_8 = 0;
	l_price_0 = 0;

	if (sl_factor > 0)
		l_price_0 = Ask - sl_factor * Point;

	l_price_8 = 0;

	if (tp_factor > 0)
		l_price_8 = Ask + tp_factor * Point;

	OrderSend(Symbol(), OP_BUY, GetMoneyManagement(), Ask, Slippage, l_price_0, l_price_8, "", MagicNumber);

	prtAlert("GRIFFIN Ultimate buy @" + DoubleToStr(Ask, 4));
}

void Eagle::OpenSellOrder() {
	double l_price_0 = 0;
	double l_price_8 = 0;
	l_price_0 = 0;

	if (sl_factor > 0)
		l_price_0 = Bid + sl_factor * Point;

	l_price_8 = 0;

	if (tp_factor > 0)
		l_price_8 = Bid - tp_factor * Point;

	OrderSend(Symbol(), OP_SELL, GetMoneyManagement(), Bid, Slippage, l_price_0, l_price_8, "", MagicNumber);

	prtAlert("GRIFFIN Ultimate sell  @" + DoubleToStr(Bid, 4));
}

int Eagle::openPositions() {
	int l_count_0 = 0;

	for (int i = OrdersTotal() - 1; i >= 0; i--) {
		OrderSelect(i, SELECT_BY_POS, MODE_TRADES);

		if (OrderMagicNumber() == MagicNumber) {
			if (OrderSymbol() == Symbol()) {
				if (OrderType() == OP_BUY)
					l_count_0++;

				if (OrderType() == OP_SELL)
					l_count_0++;
			}
		}
	}

	return (l_count_0);
}

void Eagle::prtAlert(String as_0) {
	Print(Symbol() + " - " + as_0);
}

void Eagle::CloseOrder(int a_ticket_0, double a_lots_4, double a_price_12) {
	int l_error_20;
	int l_count_24 = 0;

	while (l_count_24 < 3) {
		if (OrderClose(a_ticket_0, a_lots_4, a_price_12, Slippage))
			l_count_24 = 3;
		else {
			Print("Error closing order");
			l_count_24++;
		}
	}
}

void Eagle::TrailingPositions() {
	for (int l_pos_0 = 0; l_pos_0 < OrdersTotal(); l_pos_0++) {
		if (OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES)) {
			if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY) {
					if (!use_orders || Bid - OrderOpenPrice() > tp_limit * Point)
						if (OrderStopLoss() < Bid - (tp_limit + sl_limit - 1) * Point)
							ModifyStopLoss(Bid - tp_limit * Point);
				}

				if (OrderType() == OP_SELL) {
					if (!use_orders || OrderOpenPrice() - Ask > tp_limit * Point)
						if (OrderStopLoss() > Ask + (tp_limit + sl_limit - 1) * Point || OrderStopLoss() == 0.0)
							ModifyStopLoss(Ask + tp_limit * Point);
				}
			}
		}
	}
}

void Eagle::ModifyStopLoss(double a_price_0) {
	double l_ord_open_price_12 = OrderOpenPrice();
	double l_ord_takeprofit_20 = OrderTakeProfit();
	OrderModify(OrderTicket(), l_ord_open_price_12, a_price_0, l_ord_takeprofit_20);
}

int Eagle::HandleOpenPositions1() {
	for (int l_pos_0 = OrdersTotal() - 1; l_pos_0 >= 0; l_pos_0--) {
		OrderSelect(l_pos_0, SELECT_BY_POS, MODE_TRADES);

		if (OrderSymbol() == Symbol()) {
			if (OrderMagicNumber() == MagicNumber) {
				if (OrderType() == OP_BUY)
					if (CheckExitCondition("BUY"))
						CloseOrder(OrderTicket(), OrderLots(), Bid);
			}
		}
	}

	if (OrderSelect(0, SELECT_BY_POS, MODE_TRADES))
		if (OrderType() == OP_SELL)
			if (CheckExitCondition("SELL"))
				CloseOrder(OrderTicket(), OrderLots(), Ask);

	return 0;
}

double Eagle::GetMoneyManagement() {
	double ld_ret_0 = Lots;
	int l_hist_total_8 = OrdersHistoryTotal();
	int order_count = 0;
	ld_ret_0 = NormalizeDouble(AccountFreeMargin() * MaximumRisk / 1000.0, 1);

	if (use_mm_lots == true)
		ld_ret_0 = NormalizeDouble(AccountBalance() / mm_lots_divider * MaximumRisk / 1000.0, 1);

	if (lot_divider > 0.0) {
		for (int l_pos_16 = l_hist_total_8 - 1; l_pos_16 >= 0; l_pos_16--) {
			if (OrderSelect(l_pos_16, SELECT_BY_POS, MODE_HISTORY) == false) {
				Print("Error in history data");
				break;
			}

			if (!(OrderSymbol() != Symbol() || OrderType() > OP_SELL) && !(OrderMagicNumber() != MagicNumber)) {
				if (OrderProfit() > 0.0)
					break;

				if (OrderProfit() < 0.0 && OrderMagicNumber() == MagicNumber)
					order_count++;
			}
		}

		if (order_count > 1)
			ld_ret_0 = NormalizeDouble(ld_ret_0 - ld_ret_0 * order_count / lot_divider, 1);
	}

	if (ld_ret_0 < 0.1)
		ld_ret_0 = 0.1;

	if (Moneymanagement == false)
		ld_ret_0 = Lots;

	if (micro_lots == true)
		ld_ret_0 = Lots / 10.0;

	if (ld_ret_0 > MaxTradesize)
		ld_ret_0 = MaxTradesize;

	return (ld_ret_0);
}































DqnAdvisor::DqnAdvisor() {
	

}

void DqnAdvisor::Init() {
	SetBufferColor(0, Red());
	SetCoreSeparateWindow();
	
	String tf_str = GetSystem().GetPeriodString(GetTf()) + " ";
	
	SetJobCount(1);
	
	point = GetDataBridge()->GetPoint();
	
	SetJob(0, tf_str + " Training")
		.SetBegin		(THISBACK(TrainingBegin))
		.SetIterator	(THISBACK(TrainingIterator))
		.SetEnd			(THISBACK(TrainingEnd))
		.SetInspect		(THISBACK(TrainingInspect))
		.SetCtrl		<TrainingCtrl>();
}

void DqnAdvisor::Start() {
	if (once) {
		if (prev_counted > 0) prev_counted--;
		once = false;
		//RefreshGrid(true);
		RefreshSourcesOnlyDeep();
	}
	
	if (IsJobsFinished()) {
		int bars = GetBars();
		if (prev_counted < bars) {
			LOG("DqnAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

bool DqnAdvisor::TrainingBegin() {
	#ifdef flagDEBUG
	max_rounds = 10000;
	#else
	max_rounds = 20000000;
	#endif
	training_pts.SetCount(max_rounds, 0);
	
	
	// Allow iterating
	return true;
}

void DqnAdvisor::LoadInput(int pos) {
	int matpos = 0;
	
	ConstBuffer& input_buf = GetInputBuffer(1, 0);
	for(int i = 0; i < input_length; i++) {
		int j = pos - i;
		double d = input_buf.Get(j);
		d -= 0.5;
		d *= 2 * 50;
		tmp_mat.Set(matpos++, d);
	}
	
	ASSERT(matpos == tmp_mat.GetLength());
}

bool DqnAdvisor::TrainingIterator() {
	
	// Show progress
	GetCurrentJob().SetProgress(round, max_rounds);
	
	
	// ---- Do your training work here ----
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	
	int size = input_length;
	int range = GetBars() - 1;
	range -= size;
	
	int pos = size + Random(range);
	
	LoadInput(pos);
	
	dqn.SetGamma(0);
	
	double error;
	int action = dqn.Act(tmp_mat);
	double change = open_buf.Get(pos + 1) - open_buf.Get(pos);
	if (action) change *= -1;
	double reward = change >= 0 ? +10 : -10;
	error = dqn.Learn(tmp_mat, action, reward, NULL);
	
	
	training_pts[round] = error;
	
	
	
	// Keep count of iterations
	round++;
	
	// Stop eventually
	if (round >= max_rounds) {
		SetJobFinished();
	}
	
	return true;
}

bool DqnAdvisor::TrainingEnd() {
	double max_d = -DBL_MAX;
	int max_i = 0;
	
	
	RefreshAll();
	return true;
}

bool DqnAdvisor::TrainingInspect() {
	bool success = false;
	
	INSPECT(success, "ok: this is an example");
	INSPECT(success, "warning: this is an example");
	INSPECT(success, "error: this is an example");
	
	// You can fail the inspection too
	//if (!success) return false;
	
	return true;
}

void DqnAdvisor::RefreshAll() {
	RefreshSourcesOnlyDeep();
	ConstBuffer& timebuf = GetInputBuffer(0, 4);
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	/*VectorBool& signal  = GetOutput(0).label;
	VectorBool& enabled = GetOutput(1).label;
	Buffer& out = GetBuffer(0);
	
	// ---- Do your final result work here ----
	//RefreshBits();
	//SortByValue(results, GridResult());
	
	double spread = GetDataBridge()->GetSpread();
	if (spread == 0)
		spread = GetDataBridge()->GetPoint() * 2.0;
	
	int bars = GetBars();
	signal.SetCount(bars);
	enabled.SetCount(bars);
	
	bool initial = prev_counted == 0;
	if (prev_counted < input_length) prev_counted = input_length;
	
	if (have_othersyms) {
		for(int i = 0; i < inputs[1].GetCount(); i++) {
			bars = min(bars, inputs[1][i].core->GetBuffer(0).GetCount());
		}
	}
	
	for(int i = prev_counted; i < bars; i++) {
		
		LoadInput(i);
		
		int sig, mult = 1;
		
		if (have_actionmode == ACTIONMODE_SIGN) {
			int action = dqn.Act(tmp_mat);
			sig = action ? -1 : +1;
		}
		else if (have_actionmode == ACTIONMODE_TREND) {
			int action = dqn.Act(tmp_mat);
			double change = open_buf.Get(i) - open_buf.Get(i - 1);
			bool trend_action = change < 0.0;
			if (action) trend_action = !trend_action;
			sig = trend_action ? -1 : +1;
		}
		else if (have_actionmode == ACTIONMODE_WEIGHTED) {
			int action = dqn.Act(tmp_mat);
			bool sign_action = action % 2;
			sig = sign_action ? -1 : +1;
			mult  = 1 + action / 2;
		}
		else if (have_actionmode == ACTIONMODE_HACK) {
			double output[2];
			dqn.Evaluate(tmp_mat, output, 2);
			
			//sig = output[0] < 0.5 || output[1] < 0.5 ? (output[0] > output[1] ? -1 : +1) : 0;
			sig = output[0] > output[1] ? -1 : +1;
		}
		else Panic("Action not implemented");
		
		signal. Set(i, sig <  0);
		enabled.Set(i, sig != 0);
		
		
		if (i < bars-1) {
			if (sig > 0) {
				total += (+(open_buf.Get(i+1) - open_buf.Get(i)) - spread) * mult;
			}
			else if (sig < 0) {
				total += (-(open_buf.Get(i+1) - open_buf.Get(i)) - spread) * mult;
			}
		}
		out.Set(i, total);
	}
	
	#ifndef flagDEBUG
	if (do_test && initial) {
		DumpTest();
	}
	#endif
	
	//int sig = enabled.Get(bars-1) ? (signal.Get(bars-1) ? -1 : +1) : 0;
	//GetSystem().SetSignal(GetSymbol(), sig);
	//ReleaseLog("DqnAdvisor::RefreshAll symbol " + IntStr(GetSymbol()) + " sig " + IntStr(sig));
	
	
	// Keep counted manually
	prev_counted = bars;
	*/
	
	//DUMP(main_interval);
	//DUMP(grid_interval);
}

void DqnAdvisor::TrainingCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	DqnAdvisor* ea = dynamic_cast<DqnAdvisor*>(&*job->core);
	ASSERT(ea);
	DrawVectorPolyline(id, sz, ea->training_pts, polyline);
	
	w.DrawImage(0, 0, id);
}

void DqnAdvisor::DumpTest() {
	int bars = GetBars() - 1;
	int begin = bars * 0.66;
	
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	
	double spread = GetDataBridge()->GetSpread();
	if (spread == 0)
		spread = GetDataBridge()->GetPoint() * 3;
	
	int signs_correct = 0, signs_total = 0;
	double pos_total = 0, neg_total = 0;
	double total = 0;
	
	for(int i = begin; i < bars; i++) {
		
		LoadInput(i);
		
		int sig, mult = 1;
		
		int action = dqn.Act(tmp_mat);
		sig = action ? -1 : +1;
		
		double diff;
		if (sig > 0) {
			diff = +(open_buf.Get(i+1) - open_buf.Get(i));
		}
		else if (sig < 0) {
			diff = -(open_buf.Get(i+1) - open_buf.Get(i));
		}
		double change = (diff - spread) * mult;
		total += change;
		
		if (diff > 0)
			signs_correct++;
		signs_total++;
		
		if (diff >= 0) pos_total += diff;
		else neg_total -= diff;
	}
	
	/*
	String symstr = GetSystem().GetSymbol(GetSymbol());
	String tfstr = GetSystem().GetPeriodString(GetTf());
	
	String dir = ConfigFile("test_results");
	RealizeDirectory(dir);
	String filename = Format("%d-%d-%d-%d-%d-%d-%d-%s-%s.txt",
		have_normaldata,
		have_normalma,
		have_hurst,
		have_anomaly,
		have_othersyms,
		have_actionmode,
		input_length,
		symstr,
		tfstr);
	String filepath = AppendFileName(dir, filename);
	FileOut fout(filepath);
	
	fout << "have_normaldata = " << have_normaldata << "\r\n";
	fout << "have_normalma   = " << have_normalma << "\r\n";
	fout << "have_hurst      = " << have_hurst << "\r\n";
	fout << "have_anomaly    = " << have_anomaly << "\r\n";
	fout << "have_othersyms  = " << have_othersyms << "\r\n";
	fout << "have_actionmode = " << have_actionmode << "\r\n";
	fout << "input_length    = " << input_length << "\r\n";
	fout << "symbol          = " << symstr << "\r\n";
	fout << "timeframe       = " << tfstr << "\r\n";
	fout << "\r\n";
	fout << "signs_correct   = " << DblStr((double)signs_correct / (double)signs_total) << "\r\n";
	
	double drawdown = (neg_total / (neg_total + pos_total));
	fout << "drawdown        = " << DblStr(drawdown) << "\r\n";
	fout << "total, spreads  = " << DblStr(total) << "\r\n";
	*/
}

}
