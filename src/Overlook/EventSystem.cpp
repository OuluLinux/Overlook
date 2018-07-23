#include "Overlook.h"


namespace Overlook {

EventSystem::EventSystem() {
	
	
}

EventSystem::~EventSystem() {
	running = false;
	while (!stopped) Sleep(100);
}
	
void EventSystem::Data() {
	System& sys = GetSystem();
	for(int i = 0; i < sys.CommonFactories().GetCount(); i++) {
		sys.CommonFactories()[i].b()->Start();
	}
	
	
	if (work_queue.IsEmpty()) {
		TimeStop ts;
		/*
		symbols.Add("EURUSD");
		symbols.Add("GBPUSD");
		symbols.Add("USDCHF");
		symbols.Add("USDJPY");
		symbols.Add("USDCAD");
		symbols.Add("AUDUSD");
		symbols.Add("NZDUSD");
		symbols.Add("EURCHF");
		symbols.Add("EURJPY");
		symbols.Add("EURGBP");
		for(int i = 0; i < symbols.GetCount(); i++)
			sym_ids.Add(sys.FindSymbol(symbols[i]));
		*/
		int sym_count = sys.GetNormalSymbolCount();
		int cur_count = sys.GetCurrencyCount();
		int net_count = sys.GetNetCount();
		
		fac_ids.Add(sys.Find<DataBridge>());
		fac_ids.Add(sys.Find<SimpleHurstWindow>());
		fac_ids.Add(sys.Find<MovingAverage>());
		fac_ids.Add(sys.Find<BollingerBands>());
		fac_ids.Add(sys.Find<ParabolicSAR>());
		fac_ids.Add(sys.Find<VolatilityAverage>());
		fac_ids.Add(sys.Find<Anomaly>());
		fac_ids.Add(sys.Find<VolatilitySlots>());
		fac_ids.Add(sys.Find<PeriodicalChange>());
		fac_ids.Add(sys.Find<Calendar>());
		for(int i = 0; i < fac_ids.GetCount(); i++)
			indi_ids.Add().factory = fac_ids[i];
		for(int i = 0; i < net_count; i++)
			sym_ids.Add(sym_count + cur_count + i);
		tf_ids.Add(0);
		tf_ids.Add(1);
		tf_ids.Add(2);
		tf_ids.Add(4);
		tf_ids.Add(5);
		
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
		
		
		bufs.SetCount(BUF_COUNT);
		for(int i = 0; i < bufs.GetCount(); i++) {
			bufs[i].SetCount(sym_ids.GetCount());
			for(int j = 0; j < bufs[i].GetCount(); j++) {
				bufs[i][j].SetCount(tf_ids.GetCount());
				for(int k = 0; k < bufs[i][j].GetCount(); k++) {
					bufs[i][j][k].SetCount(1, NULL);
				}
			}
		}
		cals.SetCount(sym_ids.GetCount(), NULL);
		
		
		for(int i = 0; i < work_queue.GetCount() /*&& IsRunning()*/; i++) {
			CoreItem& ci = *work_queue[i];
			sys.Process(ci, true);
			
			Core& c = *ci.core;
			
			int faci = fac_ids.Find(c.GetFactory());
			int symi = sym_ids.Find(c.GetSymbol());
			int tfi = tf_ids.Find(c.GetTf());
			
			if (symi == -1) continue;
			if (tfi == -1) continue;
			
			auto& v = bufs[faci][symi][tfi];
			v.SetCount(c.GetBufferCount());
			for(int j = 0; j < c.GetBufferCount(); j++) {
				v[j] = &c.GetBuffer(j);
			}
				
			
			if (faci == CAL)
				cals[symi] = dynamic_cast<Calendar*>(&c);
		}
		
		
		ReleaseLog("EventSystem work queue init took " + ts.ToString());
		
		//PrintNetCode();
	}
	
	
	for(int i = 0; i < work_queue.GetCount() /*&& IsRunning()*/; i++)
		sys.Process(*work_queue[i], true);
	
	UpdateEvents();
}

void EventSystem::PrintNetCode() {
	int tfi = 2;
	
	int bars = INT_MAX;
	for(int i = 0; i < sym_ids.GetCount(); i++)
		bars = min(bars, bufs[OPEN][i][tfi][0]->GetCount());
	
	
	VectorMap<int, int> stats;
	for(int i = 1; i < bars; i++) {
		
		int code = 0;
		for(int j = 0; j < sym_ids.GetCount(); j++) {
			ConstBuffer& buf = *bufs[OPEN][j][tfi][0];
			double o0 = buf.Get(i);
			double o1 = buf.Get(i-1);
			bool b = o0 < o1;
			if (b) code |= 1 << j;
		}
		
		stats.GetAdd(code, 0)++;
	}
	
	SortByValue(stats, StdGreater<int>());
	DUMPM(stats);
	
	
	int count = 0;
	for(int i = 0; i < stats.GetCount() && i < 30; i++) {
		int code = stats.GetKey(i);
		
		String s;
		s << "AddNet(\"Net" << i << "\")";
		for(int j = 0; j < sym_ids.GetCount(); j++) {
			bool b = code & (1 << j);
			s << ".Set(\"" << symbols[j] << "\", " << (b ? "-1" : "+1") << ")";
		}
		s << ";";
		LOG(s);
		
	}
	
	LOG("");
}

void EventSystem::UpdateEvents() {
	System& sys = GetSystem();
	
	tmp_events.SetCount(0);
	
	
	for(int j = 0; j < tf_ids.GetCount(); j++) {
		int tf = tf_ids[j];
		
		{
			double hi_hurst = -DBL_MAX;
			int hi_hurst_i = -1, hi_hurst_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				ConstBuffer& hurst_buf = *bufs[HURST][i][j][0];
				double h = hurst_buf.Top();
				if (h > hi_hurst) {
					hi_hurst = h;
					hi_hurst_i = i;
					hi_hurst_sig = GetOpenSig(i, j);
				}
				//TODO: if same value, prefer longer trend
			}
			SetEvent(HIHURST, j, hi_hurst_i, hi_hurst_sig);
		}
		
		
		
		{
			double hi_ma = -DBL_MAX;
			int hi_ma_i = -1, hi_ma_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				ConstBuffer& ma_buf = *bufs[HURST][i][j][0];
				int count = ma_buf.GetCount();
				double ma0 = ma_buf.Get(count - 1);
				double ma1 = ma_buf.Get(count - 2);
				double fdiff = fabs(ma0 - ma1);
				if (fdiff > hi_ma) {
					hi_ma = fdiff;
					hi_ma_i = i;
					hi_ma_sig = ma0 < ma1 ? -1 : +1;
				}
			}
			SetEvent(HIMA, j, hi_ma_i, hi_ma_sig);
		}
		
		
		
		{
			double hi_bb = -DBL_MAX;
			int hi_bb_i = -1, hi_bb_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				ConstBuffer& lo_buf = *bufs[OPEN][i][j][1];
				ConstBuffer& hi_buf = *bufs[OPEN][i][j][2];
				ConstBuffer& ma_buf = *bufs[BB][i][j][0];
				ConstBuffer& tl_buf = *bufs[BB][i][j][1];
				ConstBuffer& bl_buf = *bufs[BB][i][j][2];
				double ma = ma_buf.Top();
				double l = lo_buf.Top();
				double h = hi_buf.Top();
				double tl = tl_buf.Top();
				double bl = bl_buf.Top();
				double tdiff = tl - ma;
				double bdiff = ma - bl;
				if (h >= tl && tdiff > hi_bb) {
					hi_bb = tdiff;
					hi_bb_i = i;
					hi_bb_sig = +1;
				}
				else if (l <= bl && bdiff > hi_bb) {
					hi_bb = bdiff;
					hi_bb_i = i;
					hi_bb_sig = -1;
				}
			}
			SetEvent(HIBB, j, hi_bb_i, hi_bb_sig);
		}
		
		
		
		{
			double hi_sar = -DBL_MAX;
			int hi_sar_i = -1, hi_sar_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				ConstBuffer& sar_buf = *bufs[PSAR][i][j][0];
				ConstBuffer& o_buf = *bufs[OPEN][i][j][0];
				int sar_count = sar_buf.GetCount();
				double sar0 = sar_buf.Get(sar_count - 1);
				double sar1 = sar_buf.Get(sar_count - 2);
				double o0 = o_buf.Get(sar_count - 1);
				double o1 = o_buf.Get(sar_count - 2);
				bool b0 = sar0 > o0;
				bool b1 = sar1 > o1;
				if (b0 != b1) {
					ConstBuffer& lo_buf = *bufs[OPEN][i][j][1];
					ConstBuffer& hi_buf = *bufs[OPEN][i][j][2];
					double h = hi_buf.Top();
					double l = lo_buf.Top();
					double diff = h - l;
					if (diff > hi_sar) {
						hi_sar = diff;
						hi_sar_i = i;
						hi_sar_sig = b0 ? -1 : +1;
					}
				}
				
			}
			SetEvent(NEWSAR, j, hi_sar_i, hi_sar_sig);
		}
		
		
		
		{
			double hi_volav = -DBL_MAX;
			int hi_volav_i = -1, hi_volav_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				ConstBuffer& volav_buf = *bufs[VA][i][j][0];
				double volav = volav_buf.Top();
				if (volav > hi_volav) {
					hi_volav = volav;
					hi_volav_i = i;
					hi_volav_sig = GetOpenSig(i, j);
				}
			}
			SetEvent(HIVOLAV, j, hi_volav_i, hi_volav_sig);
		}
		
		
		
		{
			double anom = -DBL_MAX;
			int anom_i = -1, anom_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				ConstBuffer& anom_buf = *bufs[ANOM][i][j][0];
				double a = anom_buf.Top();
				if (a >= 0.75 && a > anom) {
					anom = a;
					anom_i = i;
					anom_sig = GetOpenSig(i, j);
				}
			}
			SetEvent(ANOMALY, j, anom_i, anom_sig);
		}
		
		
		
		{
			double hich = -DBL_MAX;
			int hich_i = -1, hich_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				ConstBuffer& o_buf = *bufs[OPEN][i][j][0];
				ConstBuffer& lo_buf = *bufs[OPEN][i][j][1];
				ConstBuffer& hi_buf = *bufs[OPEN][i][j][2];
				double o = o_buf.Top();
				double h = hi_buf.Top();
				double l = lo_buf.Top();
				double pips = (h - l) / 0.0001;
				if (pips >= sym_ids.GetCount() * 3.0 && pips > hich) {
					hich = pips;
					hich_i = i;
					hich_sig = o - l > h - o ? -1 : +1;
				}
			}
			SetEvent(HICHANGE, j, hich_i, hich_sig);
		}
		
		
		
		{
			double pc = -DBL_MAX;
			int pc_i = -1, pc_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				ConstBuffer& pc_buf = *bufs[PC][i][j][0];
				double p = pc_buf.Top();
				double fp = fabs(pc);
				if (fp > pc) {
					pc = fp;
					pc_i = i;
					pc_sig = p < 0 ? -1 : +1;
				}
			}
			SetEvent(HIPERCH, j, pc_i, pc_sig);
		}
		
		
		
		{
			double vols = -DBL_MAX;
			int vols_i = -1, vols_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				ConstBuffer& vs_buf = *bufs[VS][i][j][0];
				ConstBuffer& pc_buf = *bufs[PC][i][j][0];
				double vs = vs_buf.Top();
				double p = pc_buf.Top();
				if (vs > 0.01 && vs > vols) {
					vols = vs;
					vols_i = i;
					vols_sig = p < 0 ? -1 : +1;
				}
			}
			SetEvent(HIVOLSLOT, j, vols_i, vols_sig);
		}
		
		
		
		{
			double cal = -DBL_MAX;
			int cal_i = -1, cal_sig = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				Calendar& c = *cals[i];
				double cdiff = c.GetTopDiff();
				if (cdiff > 0 && cdiff > cal) {
					cal = cdiff;
					cal_i = i;
					cal_sig = cdiff > 0 ? +1 : -1;
				}
			}
			SetEvent(HINEWS, j, cal_i, cal_sig);
		}
	}
	
	
	Swap(events, tmp_events);
	
}

void EventSystem::SetEvent(int ev, int tf, int sym, int sig) {
	if (sym < 0) return;
	System& sys = GetSystem();
	Event& e = tmp_events.Add();
	e.ev = ev;
	e.tf = tf_ids[tf];
	e.sym = sym + sys.GetNormalSymbolCount() + sys.GetCurrencyCount();
	e.sig = sig;
	
	String symstr = "Net" + IntStr(sym);
	String tfstr = sys.GetPeriodString(e.tf);
	
	NotificationQueue& n = GetNotificationQueue();
	n.SetApp("Overlook");
	n.SetSilent();
	n.SetNotificationExpiration(3);
	
	if (e.tf < 4) {
		switch (ev) {
			//case HIHURST:	n.Add(OverlookImg::hurst(), symstr + " " + tfstr + " High Hurst value"); break;
			//case HIMA:		n.Add(OverlookImg::ma(), symstr + " " + tfstr + " High Moving Average value"); break;
			case HIBB:		n.Add(OverlookImg::bb(), symstr + " " + tfstr + " High Bollinger Bands value"); break;
			case NEWSAR:	n.Add(OverlookImg::psar(), symstr + " " + tfstr + " New Parabolic SAR trend"); break;
			//case HIVOLAV:	n.Add(OverlookImg::volav(), symstr + " " + tfstr + " High volatility average"); break;
			case ANOMALY:	n.Add(OverlookImg::anom(), symstr + " " + tfstr + " Anomaly: high volatility"); break;
			case HICHANGE:	n.Add(OverlookImg::hich(), symstr + " " + tfstr + " Highest spread exceeding change"); break;
			//case HIPERCH:	n.Add(OverlookImg::pc(), symstr + " " + tfstr + " High periodical change"); break;
			case HIVOLSLOT:	n.Add(OverlookImg::vs(), symstr + " " + tfstr + " High volatility slot value"); break;
			case HINEWS:	if (e.tf == 2) n.Add(OverlookImg::news(), symstr + " " + tfstr + " News impacts"); break;
		}
	}
	else {
		switch (ev) {
			//case HIHURST:	n.Add(OverlookImg::hurst(), symstr + " " + tfstr + " High Hurst value"); break;
			//case HIMA:		n.Add(OverlookImg::ma(), symstr + " " + tfstr + " High Moving Average value"); break;
			case HIBB:		n.Add(OverlookImg::bb(), symstr + " " + tfstr + " High Bollinger Bands value"); break;
			case NEWSAR:	n.Add(OverlookImg::psar(), symstr + " " + tfstr + " New Parabolic SAR trend"); break;
			//case HIVOLAV:	n.Add(OverlookImg::volav(), symstr + " " + tfstr + " High volatility average"); break;
			case ANOMALY:	n.Add(OverlookImg::anom(), symstr + " " + tfstr + " Anomaly: high volatility"); break;
			case HICHANGE:	n.Add(OverlookImg::hich(), symstr + " " + tfstr + " Highest spread exceeding change"); break;
			//case HIPERCH:	n.Add(OverlookImg::pc(), symstr + " " + tfstr + " High periodical change"); break;
			case HIVOLSLOT:	n.Add(OverlookImg::vs(), symstr + " " + tfstr + " High volatility slot value"); break;
			//case HINEWS:	n.Add(OverlookImg::news(), symstr + " " + tfstr + " News impacts"); break;
		}
	}
}

int EventSystem::GetOpenSig(int sym, int tf) {
	ConstBuffer& open_buf = *bufs[OPEN][sym][tf][0];
	int count = open_buf.GetCount();
	double o0 = open_buf.Get(count - 1);
	double o1 = open_buf.Get(count - 2);
	if (o0 == o1) return 0;
	return o0 < o1 ? -1 : +1;
}

String EventSystem::GetEventString(int i) {
	switch (i) {
		case HIHURST:	return "High Hurst value";
		case HIMA:		return "High Moving Average value";
		case HIBB:		return "High Bollinger Bands value";
		case NEWSAR:	return "New Parabolic SAR trend";
		case HIVOLAV:	return "High volatility average";
		case ANOMALY:	return "Anomaly: high volatility";
		case HICHANGE:	return "Highest spread exceeding change";
		case HIPERCH:	return "High periodical change";
		case HIVOLSLOT:	return "High volatility slot value";
		case HINEWS:	return "News impacts";
	}
	return "Unknown";
}

}
