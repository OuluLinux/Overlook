#include "Overlook.h"

namespace Overlook {

Speculation::Speculation() {
	
	Thread::Start(THISBACK(Process));
}

void Speculation::RefreshCores() {
	for(int i = 0; i < cl.GetCount(); i++)
		for(int j = 0; j < cl[i].GetCount(); j++)
			cl[i][j].Refresh();
}

void Speculation::Process() {
	sig.Start();
	
	System& sys = GetSystem();
	
	sym.Add("EUR2");
	sym.Add("USD2");
	sym.Add("GBP2");
	sym.Add("JPY2");
	sym.Add("CHF2");
	sym.Add("CAD2");
	sym.Add("AUD2");
	sym.Add("NZD2");
	sym.Add("EURUSD" + sys.GetPostFix());
	sym.Add("EURGBP" + sys.GetPostFix());
	sym.Add("EURJPY" + sys.GetPostFix());
	sym.Add("EURCHF" + sys.GetPostFix());
	sym.Add("EURCAD" + sys.GetPostFix());
	sym.Add("EURAUD" + sys.GetPostFix());
	sym.Add("EURNZD" + sys.GetPostFix());
	sym.Add("GBPUSD" + sys.GetPostFix());
	sym.Add("USDJPY" + sys.GetPostFix());
	sym.Add("USDCHF" + sys.GetPostFix());
	sym.Add("USDCAD" + sys.GetPostFix());
	sym.Add("AUDUSD" + sys.GetPostFix());
	sym.Add("NZDUSD" + sys.GetPostFix());
	sym.Add("GBPJPY" + sys.GetPostFix());
	sym.Add("GBPCHF" + sys.GetPostFix());
	sym.Add("GBPCAD" + sys.GetPostFix());
	sym.Add("GBPAUD" + sys.GetPostFix());
	sym.Add("GBPNZD" + sys.GetPostFix());
	sym.Add("CHFJPY" + sys.GetPostFix());
	sym.Add("CADJPY" + sys.GetPostFix());
	sym.Add("AUDJPY" + sys.GetPostFix());
	sym.Add("NZDJPY" + sys.GetPostFix());
	sym.Add("CADCHF" + sys.GetPostFix());
	sym.Add("AUDCHF" + sys.GetPostFix());
	sym.Add("NZDCHF" + sys.GetPostFix());
	sym.Add("AUDCAD" + sys.GetPostFix());
	sym.Add("NZDCAD" + sys.GetPostFix());
	sym.Add("AUDNZD" + sys.GetPostFix());
	
	tfs.Add(0);
	tfs.Add(2);
	tfs.Add(4);
	tfs.Add(5);
	tfs.Add(6);
	
	cl.SetCount(sym.GetCount());
	for(int i = 0; i < cl.GetCount(); i++) {
		cl[i].SetCount(tfs.GetCount());
		for(int j = 0; j < cl[i].GetCount(); j++) {
			CoreList& c = cl[i][j];
			
			c.AddSymbol(sym[i]);
			c.AddTf(tfs[j]);
			c.AddIndi(System::Find<SpeculationOscillator>());
			c.Init();
		}
	}
	
	aiv.SetCount(sym.GetCount(), -1);
	biv.SetCount(sym.GetCount(), -1);
	for(int i = 0; i < sym.GetCount(); i++) {
		String s = sym[i];
		String a = s.Left(3);
		String b = s.Mid(3,3);
		int ai = sym.Find(a + "2");
		int bi = sym.Find(b + "2");
		aiv[i] = ai;
		biv[i] = bi;
	}
	
	for(int size = 1; size <= tfs.GetCount(); size++) {
		int start_count = tfs.GetCount() - size + 1;
		for(int k = 0; k < start_count; k++) {
			startv.Add(k);
			sizev.Add(size);
			double max_sum = 0, max_succ_sum = 0;
			for (int l = 0; l < size; l++) {
				int tfi = k + l;
				double mult = 1.0 + (double)tfi / (double)tfs.GetCount();
				max_sum += mult + mult * 2 + mult + mult * 2;
				max_succ_sum += mult + mult * 2;
			}
			this->max_sum.Add(max_sum);
			this->max_succ_sum.Add(max_succ_sum);
		}
	}
	
	for(int i = 0; i < tfs.GetCount(); i++) {
		double mult = 1.0 + (double)i / (double)tfs.GetCount();
		bd_max_sum += mult * 2;
		bd_max_succ_sum += mult;
	}
	
	lengths.Add(0);
	lengths.Add(1);
	lengths.Add(2);
	lengths.Add(4);
	
	bardata.SetCount(tfs.GetCount() * startv.GetCount() * lengths.GetCount());
	bardataspec.SetCount(startv.GetCount() * lengths.GetCount());
	
	LoadThis();
	
	while (sig.IsRunning() && !Thread::IsShutdownThreads()) {
		RefreshCores();
		Data();
		
		Sleep(100);
	}
	
	sig.SetStopped();
}

void Speculation::Data() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(0);
	
	int counted = data.GetCount();
	int bars = idx.GetCount();
	for(int i = 0; i < sym.GetCount(); i++)
		bars = min(bars, cl[i][0].GetBuffer(0,0,0).GetCount());
	bars--;
	
	data.SetCount(bars);
	
	for(int i = counted; i < bars; i++) {
		SpecItem& si = data[i];
		ProcessItem(i, si);
	}
	
	RefreshBarData();
	
	RefreshBarDataSpec();
	
	RefreshTraders();
	
	if (counted < bars)
		StoreThis();
}


void Speculation::ProcessItem(int pos, SpecItem& si) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	
	Time t = idx0[pos];
	
	Vector<int> shift_posv;
	for(int i = 0; i < tfs.GetCount(); i++)
		shift_posv.Add(dbc.GetTimeIndex(tfs[i]).Find(SyncTime(tfs[i], t)));
	
	int symtf_count = sym.GetCount() * tfs.GetCount();
	int symstart_count = sym.GetCount() * startv.GetCount();
	si.values.SetCount(symtf_count);
	si.avvalues.SetCount(symtf_count);
	si.succ.SetCount(symtf_count);
	si.data.SetCount(symstart_count);
	si.succ_tf.SetCount(tfs.GetCount(), 0);
	
	int row = 0;
	for(int i = 0; i < sym.GetCount(); i++) {
		CoreList& c0 = cl[i][0];
		DataBridge& dbm1 = *c0.GetDataBridgeM1(0);
		double spread = dbm1.GetPoint();
		int spread_id = CommonSpreads().Find(sym[i]);
		if (spread_id != -1)
			spread *= CommonSpreads()[spread_id];
		else
			spread *= CommonSpreads().Top();
		ConstBuffer& open_m1 = dbm1.GetBuffer(0);
		double close = open_m1.Get(min(open_m1.GetCount()-1, shift_posv[0]));
		
		
		for(int j = 0; j < tfs.GetCount(); j++) {
			CoreList& c = cl[i][j];
			DataBridge& db = *c.GetDataBridge(0);
			ConstBuffer& open = db.GetBuffer(0);
			
			ConstLabelSignal& sig = c.GetLabelSignal(0, 0, 0);
			int shift_pos = min(sig.signal.GetCount()-1, shift_posv[j]);
			
			bool b = sig.signal.Get(shift_pos);
			si.values.Set(row, b);
			
			double sum = 0;
			for(int k = 0; k < 21; k++) {
				bool b = sig.signal.Get(max(0, shift_pos - k));
				if (b) sum += 1.0;
			}
			sum /= 21;
			b = sum > 0.5;
			si.avvalues.Set(row, b);
			
			int steps = 0;
			switch (tfs[j]) {
				case 0: steps = 21; break;
				case 2: steps = 4; break;
				case 4: steps = 2; break;
				case 5: steps = 2; break;
				case 6: steps = 1; break;
			}
			bool prev_sig;
			bool succ = false;
			for(int k = 0; k < steps; k++) {
				int pos = max(0, min(open.GetCount()-1, shift_pos - k));
				bool cur_sig = sig.signal.Get(pos);
				if (k && cur_sig != prev_sig) break;
				prev_sig = cur_sig;
				double cur_open = open.Get(pos);
				double diff = close - cur_open;
				if (cur_sig) diff *= -1;
				if (diff > spread) {
					succ = true;
					break;
				}
			}
			si.succ.Set(row, succ);
			
			row++;
		}
		
	}
	
	
	
	for(int i = cur_count; i < sym.GetCount(); i++) {
		int ai = aiv[i];
		int bi = biv[i];
				
		for(int j = 0; j < tfs.GetCount(); j++) {
			int ij = i * tfs.GetCount() + j;
			bool v = si.values[ij];
			bool avv = si.avvalues[ij];
			bool sv = si.succ[ij];
			int aij = ai * tfs.GetCount() + j;
			int bij = bi * tfs.GetCount() + j;
			bool av = si.values[aij];
			bool bv = si.values[bij];
			bool aav = si.avvalues[aij];
			bool abv = si.avvalues[bij];
			bool sav = si.succ[aij];
			bool sbv = si.succ[bij];
			double slow_mult = 1.0 + (double)j / (double)tfs.GetCount();
			double symtf_sum = 0;
			symtf_sum += (v ? -1 : +1) * slow_mult;
			symtf_sum += (avv ? -1 : +1) * slow_mult;
			int ab = (av ? -1 : +1) + (bv ? +1 : -1);
			int aab = (aav ? -1 : +1) + (abv ? +1 : -1);
			if (ab) symtf_sum += (ab < 0 ? -2 : +2) * slow_mult;
			if (aab) symtf_sum += (aab < 0 ? -2 : +2) * slow_mult;
			
			for(int k = 0; k < startv.GetCount(); k++) {
				SpecItemData& d = si.data[i * startv.GetCount() + k];
				int start = startv[k];
				int stop = start + sizev[k];
				if (j >= start && j < stop) {
					d.slow_sum += symtf_sum;
					d.succ_sum += (sv + sav + sbv) * slow_mult;
				}
			}
			
			si.succ_tf[j] += (sv + sav + sbv) / ((sym.GetCount() - cur_count) * 3.0);
		}
	}
	
	int tf_begin = -1, tf_end = -1, data_i = 0;
	double max_tf_fac = -DBL_MAX;
	for(int i = 0; i < startv.GetCount(); i++) {
		double fac_sum = 0;
		for(int j = cur_count; j < sym.GetCount(); j++) {
			int k = j * startv.GetCount() + i;
			double fac0 = fabs(si.data[k].slow_sum) / max_sum[i];
			double fac1 = si.data[k].succ_sum / max_succ_sum[i];
			fac_sum += fac0 + fac1;
		}
		if (fac_sum > max_tf_fac) {
			tf_begin = startv[i];
			tf_end = tf_begin + sizev[i];
			max_tf_fac = fac_sum;
			data_i = i;
		}
	}
	
	si.tf_begin = tf_begin;
	si.tf_end = tf_end;
	si.data_i = data_i;
}

void Speculation::RefreshBarData() {
	int row = 0;
	bool do_print = bardata[0].spec_counted = 0;
	for(int j = 0; j < tfs.GetCount(); j++) {
		for(int k = 0; k < startv.GetCount(); k++) {
			for (int l = 0; l < lengths.GetCount(); l++) {
				SpecBarData& sbd = GetSpecBarData(j, k, l);
				if (j == 0)
					RefreshBarDataItemTf0(k, l, sbd);
				else
					RefreshBarDataItemTf(j, k, l, sbd, GetSpecBarData(0, k, l));
				
				RefreshBarDataSpeculation(j, sbd);
				
				if (do_print) {
					ReleaseLog(Format("RefreshBarData %d / %d = %n", row, bardata.GetCount(), (double)row / bardata.GetCount() * 100));
					row++;
				}
			}
		}
	}
}

void Speculation::RefreshBarDataItemTf0(int comb, int lengthi, SpecBarData& sbd) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(0);
	
	int counted = sbd.counted;
	int bars = min(data.GetCount(), idx.GetCount());
	for(int i = 0; i < sym.GetCount(); i++)
		bars = min(bars, cl[i][0].GetBuffer(0,0,0).GetCount());
	
	sbd.data.SetCount(bars);
	
	
	
	for(int i = counted; i < bars; i++) {
		const SpecItem& si = data[i];
		SpecBarDataItem& sbdi = sbd.data[i];
		
		if (lengthi) {
			for(int j = 0; j < sbd.length_sigs.GetCount(); j++) {
				int sym = sbd.length_sigs.GetKey(j);
				Tuple2<bool, int>& t = sbd.length_sigs[j];
				int begin = t.b;
				int length = i - begin;
				if (length >= lengths[lengthi] * 60) {
					sbd.length_sigs.Remove(j);
					j--;
				} else {
					sbdi.syms.GetAdd(sym) = t.a;
				}
			}
		}
		
		for(int j = cur_count; j < this->sym.GetCount(); j++) {
			const SpecItemData& sid = si.data[j * startv.GetCount() + comb];
			double slow_sum_fac = fabs(sid.slow_sum) / max_sum[comb];
			double succ_sum_fac = sid.succ_sum / max_succ_sum[comb];
			if (slow_sum_fac >= 0.5 && succ_sum_fac >= 0.5) {
				bool sig = sid.slow_sum < 0;
				sbdi.syms.GetAdd(j) = sig;
				if (lengthi) {
					Tuple2<bool, int>& t = sbd.length_sigs.GetAdd(j);
					t.a = sig;
					t.b = i;
				}
			}
		}
		
		if (i == 0) {
			sbdi.open = 1.0;
			sbdi.low = 1.0;
			sbdi.high = 1.0;
			sbdi.volume = 0.0;
		}
		else {
			SpecBarDataItem& sbdi1 = sbd.data[i-1];
			SpecBarDataItem* sbdi2 = NULL;
			if (i >= 2)
				sbdi2 = &sbd.data[i-2];
			
			double change_sum = 0, low_change_sum = 0, high_change_sum = 0, vol_sum = 0;
			for(int j = 0; j < sbdi1.syms.GetCount(); j++) {
				int sym = sbdi1.syms.GetKey(j);
				bool sig = sbdi1.syms[j];
				
				CoreList& c = cl[sym][0];
				DataBridge& db = *c.GetDataBridge(0);
				ConstBuffer& open = db.GetBuffer(0);
				ConstBuffer& low = db.GetBuffer(1);
				ConstBuffer& high = db.GetBuffer(2);
				ConstBuffer& vol = db.GetBuffer(3);
				double o0 = open.Get(i);
				double o1 = open.Get(i-1);
				double l = low.Get(i-1);
				double h = high.Get(i-1);
				double v = vol.Get(i-1);
				double change = o0 / o1 - 1.0;
				if (!sig)			change_sum += change;
				else				change_sum -= change;
				double low_change = l / o0 - 1.0;
				if (!sig)			low_change_sum += low_change;
				else				low_change_sum -= low_change;
				double high_change = h / o0 - 1.0;
				if (!sig)			high_change_sum += high_change;
				else				high_change_sum -= high_change;
				vol_sum += v;
				
				bool pay_spreads = false;
				if (sbdi2 == NULL) {
					pay_spreads = true;
				} else {
					int k = sbdi2->syms.Find(sym);
					if (k != -1) {
						bool prev_sig = sbdi2->syms[k];
						if (prev_sig != sig)
							pay_spreads = true;
					}
					else
						pay_spreads = true;
				}
				
				if (pay_spreads) {
					double spread = db.GetPoint();
					int spread_id = CommonSpreads().Find(this->sym[sym]);
					if (spread_id != -1)
						spread *= CommonSpreads()[spread_id];
					else
						spread *= CommonSpreads().Top();
					double so0 = o1 + spread;
					double so1 = o1;
					double change = so0 / so1 - 1.0;
					change_sum -= change;
					low_change_sum -= change;
					high_change_sum -= change;
				}
			}
			
			if (!sbdi1.syms.IsEmpty()) {
				change_sum /= sbdi1.syms.GetCount();
				low_change_sum /= sbdi1.syms.GetCount();
				high_change_sum /= sbdi1.syms.GetCount();
				vol_sum /= sbdi1.syms.GetCount();
			}
			
			double d = sbdi1.open;
			d *= 1.0 + change_sum;
			double l = sbdi1.open * (1.0 + low_change_sum);
			double h = sbdi1.open * (1.0 + high_change_sum);
			if (l < sbdi1.low)		sbdi1.low = l;
			if (h > sbdi1.high)		sbdi1.high = h;
			ASSERT(d > 0.0);
			sbdi1.volume = vol_sum;
			
			sbdi.open = d;
			sbdi.low = d;
			sbdi.high = d;
		}
	}
	
	sbd.counted = bars;
}

void Speculation::RefreshBarDataItemTf(int tfi, int comb, int length, SpecBarData& sbd, SpecBarData& sbd0) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	const Index<Time>& idx = dbc.GetTimeIndex(tfs[tfi]);
	
	int counted = sbd.counted;
	int bars = idx.GetCount();
	int fast_counted = sbd.fast_counted;
	int fast_bars = min(sbd0.data.GetCount(), idx0.GetCount());
	
	sbd.data.SetCount(bars);
	
	for(int i = fast_counted; i < fast_bars; i++) {
		Time t0 = idx0[i];
		Time t = SyncTime(tfs[tfi], t0);
		int pos = idx.Find(t);
		if (pos == -1) continue;
		
		const SpecBarDataItem& sbdi0 = sbd0.data[i];
		SpecBarDataItem& sbdi = sbd.data[pos];
		
		if (sbdi.open == 0) {
			sbdi.open = sbdi0.open;
			sbdi.low = sbdi0.low;
			sbdi.high = sbdi0.high;
			sbdi.volume = sbdi0.volume;
		} else {
			sbdi.low = min(sbdi.low, sbdi0.low);
			sbdi.high = max(sbdi.high, sbdi0.high);
			sbdi.volume += sbdi0.volume;
		}
	}
	
	sbd.fast_counted = fast_bars;
	sbd.counted = bars;
}

void Speculation::RefreshBarDataSpeculation(int tfi, SpecBarData& sbd) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(tfs[tfi]);
	
	int counted = sbd.spec_counted;
	int bars = min(sbd.data.GetCount(), idx.GetCount());
	
	if (!counted) counted++;
	else counted--;
	
	for(int i = counted; i < bars; i++) {
		SpecBarDataItem& sbdi0 = sbd.data[i];
		const SpecBarDataItem& sbdi1 = sbd.data[i-1];
		
		double vol = sbdi1.volume;
		Time t = idx[i];
		int wday = DayOfWeek(t);
		
		double open = sbdi1.open;
		double close = sbdi0.open;
		double absch = fabs(open / close - 1);
		double volch = vol != 0.0 ? absch / vol : 0.0;
		
		if (vol >= 0 && wday != 0 && i > counted && IsFin(volch)) {
			sbd.spec_av.Add(volch);
		}
		
		double mean = sbd.spec_av.GetMean();
		double relch = mean != 0.0 ? volch / mean - 1.0: 0;
		sbdi0.spec = relch;
		if (relch > 0) {
			sbd.spec_signal = close <= open;
		}
		else if (open == close)
			sbd.spec_signal = true;
		sbdi0.signal = sbd.spec_signal;
	}
	
	sbd.spec_counted = bars;
}

void Speculation::RefreshBarDataSpec() {
	for(int i = 0; i < startv.GetCount(); i++) {
		for(int j = 0; j < lengths.GetCount(); j++) {
			SpecBarDataSpec& sbds = GetSpecBarDataSpec(i, j);
			RefreshBarDataSpeculation2(i, j, sbds);
		}
	}
	
	const SpecBarData& spd0 = GetSpecBarData(0, 0, 0);
	int counted = bardataspecscore.GetCount();
	int bars = spd0.data.GetCount();
	bardataspecscore.SetCount(bars);
	
	for(int i = counted; i < bars; i++) {
		
		double sum = 0;
		for(int j = 0; j < bardataspec.GetCount(); j++) {
			const SpecBarDataSpec& sbds = bardataspec[j];
			const SpecBarDataSpecData& spdsd = sbds.data[i];
			
			double score = (max(0.0, spdsd.slow_sum) / bd_max_sum * 2 + spdsd.succ_sum / bd_max_succ_sum) / 3.0;
			sum += score;
		}
		
		sum /= bardataspec.GetCount();
		sum *= 100;
		bardataspecscore[i] = sum;
		
		bardataspecscore_stats.GetAdd((int)sum, 0)++;
	}
	SortByKey(bardataspecscore_stats, StdLess<int>());
	DUMPM(bardataspecscore_stats);
	
	
	bardataspecscore_levels.SetCount(lvl_count);
	for(int i = 0; i < lvl_count; i++) {
		int limit = bars * i / lvl_count;
		
		int sum = 0;
		int level = 100;
		for(int j = 0; j < bardataspecscore_stats.GetCount(); j++) {
			sum += bardataspecscore_stats[j];
			if (sum >= limit) {
				level = bardataspecscore_stats.GetKey(j);
				break;
			}
		}
		bardataspecscore_levels[i] = level;
	}
	DUMPC(bardataspecscore_levels);
	
}

void Speculation::RefreshBarDataSpeculation2(int comb, int lengthi, SpecBarDataSpec& sbds) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	
	const SpecBarData& spd0 = GetSpecBarData(0, comb, lengthi);
	
	int counted = sbds.data.GetCount();
	int bars = spd0.data.GetCount();
	
	sbds.data.SetCount(bars);
	
	for(int i = counted; i < bars; i++) {
		SpecBarDataSpecData& spdsd = sbds.data[i];
		const SpecBarDataItem& spdi0 = spd0.data[i];
		double close = spdi0.open;
		
		Time t = idx0[i];
		
		for(int j = 0; j < tfs.GetCount(); j++) {
			int shift_pos = j == 0 ? i : dbc.GetTimeIndex(tfs[j]).Find(SyncTime(tfs[j], t));
			SpecBarData& spd = GetSpecBarData(j, comb, lengthi);
			SpecBarDataItem& spdi = spd.data[shift_pos];
			
			bool b = spdi.signal;
			spdsd.values[j] = b;
			
			double sum = 0;
			for(int k = 0; k < 21; k++) {
				SpecBarDataItem& spdi = spd.data[max(0, shift_pos - k)];
				if (spdi.signal) sum += 1.0;
			}
			sum /= 21;
			b = sum > 0.5;
			spdsd.avvalues[j] = b;
			
			int steps = 0;
			switch (tfs[j]) {
				case 0: steps = 21; break;
				case 2: steps = 4; break;
				case 4: steps = 2; break;
				case 5: steps = 2; break;
				case 6: steps = 1; break;
			}
			bool prev_sig = false;
			bool succ = false;
			for(int k = 0; k < steps; k++) {
				int pos = max(0, min(spd.data.GetCount()-1, shift_pos - k));
				const SpecBarDataItem& spdi = spd.data[pos];
				bool cur_sig = spdi.signal;
				if (cur_sig != prev_sig) break;
				prev_sig = cur_sig;
				double cur_open = spdi.open;
				double diff = close - cur_open;
				if (diff > 0) {
					succ = true;
					break;
				}
			}
			spdsd.succ[j] = succ;
			
			double slow_mult = 1.0 + (double)j / (double)tfs.GetCount();
			spdsd.slow_sum += (!spdsd.values[j]) * slow_mult;
			spdsd.slow_sum += (!spdsd.avvalues[j]) * slow_mult;
			spdsd.succ_sum += spdsd.succ[j] * slow_mult;
		}
	}
}

void Speculation::RefreshTraders() {
	
	if (traders.IsEmpty()) {
		
		for(int comb = 0; comb < startv.GetCount(); comb++) {
			
			for(int reqav = 0; reqav < 2; reqav++) {
				
				for(int scorelvl = 0; scorelvl < lvl_count; scorelvl++) {
					
					for(int reqsucc = 0; reqsucc < 2; reqsucc++) {
						
						for(int tfi = 0; tfi < 3; tfi++) {
							TraderItem& ti = traders.Add();
							ti.comb = comb;
							ti.reqav = reqav;
							ti.scorelvl = scorelvl;
							ti.reqsucc = reqsucc;
							ti.tfi = tfi;
							
							RefreshTrader(ti);
							
							/*if (ti.data.Top().open < 1.0) {
								traders.Pop();
							}*/
						}
					}
				}
			}
		}
	}
	else {
		for(int i = 0; i < traders.GetCount(); i++) {
			RefreshTrader(traders[i]);
		}
	}
	
	for(int i = 0; i < traders.GetCount(); i++) {
		for(int j = 1; j < tfs.GetCount(); j++) {
			RefreshTrader(j, traders[i]);
		}
	}
	
}

void Speculation::RefreshTrader(TraderItem& ti) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	
	SpecBarDataSpec& sbds0 = bardataspec[0];
	
	int counted = ti.data.GetCount();
	int bars = sbds0.data.GetCount();
	
	ti.data.SetCount(tfs.GetCount());
	Vector<TraderItemData>& data = ti.data[0];
	data.SetCount(bars);
	
	int start = startv[ti.comb];
	int stop = start + sizev[ti.comb];
	int scoremin = bardataspecscore_levels[ti.scorelvl];
	int continue_length = GetSystem().GetPeriod(tfs[ti.tfi]);
	
	for(int i = counted; i < bars; i++) {
		TraderItemData& tid = data[i];
		
		bool do_continue = ti.cur_bdspeci >= 0;
		if (do_continue) {
			int length = i - ti.cur_begin;
			if (length <= continue_length) {
				tid.bdspeci = ti.cur_bdspeci;
			} else {
				do_continue = false;
				ti.cur_bdspeci = -1;
			}
		}
		
		int start_spec = -1;
		bool do_start = true;
		
		int score = bardataspecscore[i];
		if (score < scoremin)
			do_start = false;
		
		if (do_start || do_continue) {
			VectorMap<int, double> scoremap;
			for(int j = 0; j < bardataspec.GetCount(); j++) {
				const SpecBarDataSpec& sbds = bardataspec[j];
				const SpecBarDataSpecData& sbdsd = sbds.data[i];
				
				bool is_sig = true;
				for(int k = start; k < stop; k++) {
					if (sbdsd.values[k]) {
						is_sig = false;
						break;
					}
					if (ti.reqav) {
						if (sbdsd.avvalues[k]) {
							is_sig = false;
							break;
						}
					}
					if (ti.reqsucc) {
						if (!sbdsd.succ[k]) {
							is_sig = false;
							break;
						}
					}
				}
				
				if (is_sig) {
					double score = (2 * sbdsd.slow_sum / bd_max_sum + sbdsd.succ_sum / bd_max_succ_sum) / 3.0;
					scoremap.Add(j, score);
				}
			}
			if (!scoremap.IsEmpty()) {
				if (do_continue) {
					int j = scoremap.Find(ti.cur_bdspeci);
					if (j >= 0) {
						ti.cur_begin = i;
					}
				} else {
					SortByValue(scoremap, StdGreater<double>());
					ti.cur_begin = i;
					ti.cur_bdspeci = scoremap.GetKey(0);
					tid.bdspeci = ti.cur_bdspeci;
				}
			}
		}
		
		if (i == 0) {
			tid.open = 1.0;
			tid.low = 1.0;
			tid.high = 1.0;
		} else {
			TraderItemData& tid1 = data[i-1];
			
			double change_sum = 0, low_change_sum = 0, high_change_sum = 0;
			if (tid1.bdspeci >= 0) {
				int comb1 = tid1.bdspeci / lengths.GetCount();
				int lengthi1 = tid1.bdspeci % lengths.GetCount();
				const SpecBarData& sbd1 = GetSpecBarData(0, comb1, lengthi1);
				const SpecBarDataItem& sbdi1 = sbd1.data[i-1];
				const SpecBarDataItem* sbdi2 = NULL;
				if (i >= 2) {
					TraderItemData& tid2 = ti.data[0][i-2];
					if (tid2.bdspeci >= 0) {
						int comb2 = tid2.bdspeci / lengths.GetCount();
						int lengthi2 = tid2.bdspeci % lengths.GetCount();
						const SpecBarData& sbd2 = GetSpecBarData(0, comb2, lengthi2);
						sbdi2 = &sbd2.data[i-2];
					}
				}
				
				for(int j = 0; j < sbdi1.syms.GetCount(); j++) {
					int sym = sbdi1.syms.GetKey(j);
					bool sig = sbdi1.syms[j];
						
					CoreList& c = cl[sym][0];
					DataBridge& db = *c.GetDataBridge(0);
					ConstBuffer& open = db.GetBuffer(0);
					ConstBuffer& low = db.GetBuffer(1);
					ConstBuffer& high = db.GetBuffer(2);
					double o0 = open.Get(i);
					double o1 = open.Get(i-1);
					double l = low.Get(i-1);
					double h = high.Get(i-1);
					double change = o0 / o1 - 1.0;
					if (!sig)			change_sum += change;
					else				change_sum -= change;
					double low_change = l / o0 - 1.0;
					if (!sig)			low_change_sum += low_change;
					else				low_change_sum -= low_change;
					double high_change = h / o0 - 1.0;
					if (!sig)			high_change_sum += high_change;
					else				high_change_sum -= high_change;
					
					bool pay_spreads = false;
					if (sbdi2 == NULL) {
						pay_spreads = true;
					} else {
						int k = sbdi2->syms.Find(sym);
						if (k != -1) {
							bool prev_sig = sbdi2->syms[k];
							if (prev_sig != sig)
								pay_spreads = true;
						}
						else
							pay_spreads = true;
					}
					
					if (pay_spreads) {
						double spread = db.GetPoint();
						int spread_id = CommonSpreads().Find(this->sym[sym]);
						if (spread_id != -1)
							spread *= CommonSpreads()[spread_id];
						else
							spread *= CommonSpreads().Top();
						double so0 = o1 + spread;
						double so1 = o1;
						double change = so0 / so1 - 1.0;
						change_sum -= change;
						low_change_sum -= change;
						high_change_sum -= change;
					}
				}
			}
			
			double d = tid1.open;
			d *= 1.0 + change_sum;
			double l = tid1.open * (1.0 + low_change_sum);
			double h = tid1.open * (1.0 + high_change_sum);
			if (l < tid1.low)		tid1.low = l;
			if (h > tid1.high)		tid1.high = h;
			ASSERT(d > 0.0);
			
			tid.open = d;
			tid.low = d;
			tid.high = d;
		}
	}
}

void Speculation::RefreshTrader(int tfi, TraderItem& ti) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	const Index<Time>& idx = dbc.GetTimeIndex(tfs[tfi]);
	
	Vector<TraderItemData>& data0 = ti.data[0];
	Vector<TraderItemData>& data = ti.data[tfi];
	
	ti.fast_counted.SetCount(tfs.GetCount(), 0);
	int fast_counted = ti.fast_counted[tfi];
	int fast_bars = min(data0.GetCount(), idx0.GetCount());
	int bars = idx.GetCount();
	
	data.SetCount(bars);
	
	for(int i = fast_counted; i < fast_bars; i++) {
		Time t0 = idx0[i];
		Time t = SyncTime(tfs[tfi], t0);
		int pos = idx.Find(t);
		if (pos == -1) continue;
		
		const TraderItemData& tid0 = data0[i];
		TraderItemData& tid = data[pos];
		
		if (tid.open == 0) {
			tid.open = tid0.open;
			tid.low = tid0.low;
			tid.high = tid0.high;
		} else {
			tid.low = min(tid.low, tid0.low);
			tid.high = max(tid.high, tid0.high);
		}
	}
	
	ti.fast_counted[tfi] = fast_bars;
}



















SpeculationCtrl::SpeculationCtrl() {
	Add(tabs.VSizePos(0, 30).HSizePos());
	Add(slider.BottomPos(0, 30).HSizePos(0, 200));
	Add(time_lbl.BottomPos(0, 30).RightPos(0, 200));
	
	slider.MinMax(0, 1);
	slider.SetData(0);
	slider <<= THISBACK(Data);
	
	tabs.Add(matrix_tab.SizePos(), "Matrix");
	tabs.Add(perf_tab.SizePos(), "Performance");
	tabs.Add(trader_tab.SizePos(), "Traders");
	
	matrix_tab << matrix << listparent;
	matrix_tab.Horz();
	matrix_tab.SetPos(8000);
	
	listparent.Add(succ_ctrl.TopPos(0, 15).HSizePos());
	listparent.Add(list.VSizePos(15).HSizePos());
	list.AddColumn("Symbol");
	list.AddColumn("Sum");
	list.AddColumn("Success");
	list.AddColumn("Sig");
	list.AddColumn("Value");
	list.ColumnWidths("6 5 5 3 1");
	
	perf_tab << perf_list << perf_parent;
	perf_tab.Horz();
	perf_tab.SetPos(4000);
	
	perf_list.AddColumn("Start");
	perf_list.AddColumn("Size");
	perf_list.AddColumn("Length");
	perf_list.AddColumn("Signals");
	perf_list.AddColumn("Sum");
	perf_list.AddColumn("Success");
	perf_list.AddColumn("Score");
	perf_list.AddIndex();
	perf_list.AddIndex();
	perf_list.ColumnWidths("2 2 2 8 3 3 1");
	perf_list.SetLineCy(30);
	perf_list.WhenAction << THISBACK(Data);
	
	perf_parent.Add(perf_tf.TopPos(0, 30).LeftPos(0, 200));
	perf_parent.Add(candles.VSizePos(30).HSizePos());
	
	trader_tab << trader_list << trader_parent;
	trader_tab.Horz();
	trader_tab.SetPos(3500);
	
	trader_list.AddColumn("Start");
	trader_list.AddColumn("Size");
	trader_list.AddColumn("Score");
	trader_list.AddColumn("Tfi");
	trader_list.AddColumn("Req av");
	trader_list.AddColumn("Req succ");
	trader_list.AddColumn("Latest value");
	trader_list.AddIndex();
	trader_list.WhenAction << THISBACK(Data);
	
	trader_parent.Add(trader_tf.TopPos(0, 30).LeftPos(0, 200));
	trader_parent.Add(trader_candles.VSizePos(30).HSizePos());
}

void SpeculationCtrl::Start() {
	Data();
}

void SpeculationCtrl::Data() {
	Speculation& s = GetSpeculation();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	
	if (perf_tf.GetCount() == 0) {
		for(int i = 0; i < s.tfs.GetCount(); i++)
			perf_tf.Add(GetSystem().GetPeriodString(s.tfs[i]));
		perf_tf.SetIndex(0);
		perf_tf <<= THISBACK(Data);
	}
	if (trader_tf.GetCount() == 0) {
		for(int i = 0; i < s.tfs.GetCount(); i++)
			trader_tf.Add(GetSystem().GetPeriodString(s.tfs[i]));
		trader_tf.SetIndex(0);
		trader_tf <<= THISBACK(Data);
	}
	
	if (s.data.IsEmpty()) return;
	
	bool is_last = slider.GetData() == slider.GetMax();
	slider.MinMax(0, s.data.GetCount()-1);
	if (is_last)
		slider.SetData(slider.GetMax());
	
	matrix.shift = slider.GetData();
	succ_ctrl.shift = matrix.shift;
	trader_candles.shift = matrix.shift;
	
	
	time_lbl.SetLabel(Format("%", idx0[matrix.shift]));
	
	int tab = tabs.Get();
	if (tab == 0) {
		matrix.Refresh();
		succ_ctrl.Refresh();
		
		SpecItem& si = s.data[matrix.shift];
		
		int cursor = 0;
		if (list.IsCursor()) cursor = list.GetCursor();
		
		for(int i = s.cur_count; i < s.sym.GetCount(); i++) {
			String sym = s.sym[i];
			
			int pos = i * s.startv.GetCount() + si.data_i;
			if (pos < 0 || pos >= si.data.GetCount())
				continue;
			SpecItemData& d = si.data[pos];
			double sum = d.slow_sum;
			int new_value = fabs(sum) / s.max_sum[si.data_i] * 1000;
			
			double succ = d.succ_sum;
			int new_succ_value = succ / s.max_succ_sum[si.data_i] * 1000;
			
			int row = i-s.cur_count;
			/*if (ses.HasOrders(full)) {
				bool sig = sum < 0;
				bool real_sig = ses.GetOrderSig(full);
				list.Set(row, 0, AttrText(s).Paper(real_sig ? sell_paper : buy_paper));
				list.Set(row, 3, AttrText(!sig ? "Buy" : "Sell").Paper(sig == real_sig ? White() : LtRed()));
			}
			else {*/
				list.Set(row, 0, sym);
				list.Set(row, 3, sum > 0 ? "Buy" : "Sell");
			//}
			list.Set(row, 1, new_value);
			list.SetDisplay(row, 1, ProgressDisplay2());
			list.Set(row, 2, new_succ_value);
			list.SetDisplay(row, 2, SuccessDisplay());
			list.Set(row, 4, (new_value + new_succ_value) / 20);
		}
		list.SetSortColumn(4, true);
		list.SetCursor(cursor);
	}
	else if (tab == 1) {
		if (matrix.shift < 0 || matrix.shift >= s.data.GetCount())
			return;
		SpecItem& si = s.data[matrix.shift];
		
		int cursor = 0;
		if (perf_list.IsCursor()) cursor = perf_list.GetCursor();
		int scroll = perf_list.GetScroll();
		
		int comb = 0, lengthi = 0;
		if (perf_list.GetCount()) {
			comb = max(0, min(s.startv.GetCount()-1, (int)perf_list.Get(cursor, 7)));
			lengthi = max(0, min(s.lengths.GetCount()-1, (int)perf_list.Get(cursor, 8)));
			int tfi = perf_tf.GetIndex();
			int shift = matrix.shift;
			if (tfi)
				shift = dbc.GetTimeIndex(s.tfs[tfi]).Find(SyncTime(s.tfs[tfi], idx0[shift]));
			if (comb != candles.comb || lengthi != candles.lengthi || shift != candles.shift || tfi != candles.tfi) {
				candles.comb = comb;
				candles.lengthi = lengthi;
				candles.shift = shift;
				candles.tfi = tfi;
				candles.Data();
				candles.Refresh();
			}
		}
		
		int row = 0;
		for(int i = 0; i < s.startv.GetCount(); i++) {
			for(int j = 0; j < s.lengths.GetCount(); j++) {
				const SpecBarDataSpec& sbds = s.GetSpecBarDataSpec(i, j);
				if (matrix.shift < 0 || matrix.shift >= sbds.data.GetCount())
					 continue;
				
				perf_list.Set(row, 0, s.startv[i]);
				perf_list.Set(row, 1, s.sizev[i]);
				perf_list.Set(row, 2, s.lengths[j]);
				
				const SpecBarDataSpecData& sbdsd = sbds.data[matrix.shift];
				int code = 0;
				int bit = 0;
				for(int k = 0; k < 5; k++) {
					if (sbdsd.values[k]) code |= 1 << bit;
					bit++;
					if (sbdsd.avvalues[k]) code |= 1 << bit;
					bit++;
					if (sbdsd.succ[k]) code |= 1 << bit;
					bit++;
				}
				perf_list.Set(row, 3, code);
				perf_list.SetDisplay(row, 3, CodeDisplay());
				
				ASSERT(s.bd_max_sum != 0 && s.bd_max_succ_sum != 0);
				int new_value = max(0.0, sbdsd.slow_sum) * 1000 / s.bd_max_sum;
				int new_succ_value = sbdsd.succ_sum * 1000 / s.bd_max_succ_sum;
				perf_list.Set(row, 4, new_value);
				perf_list.SetDisplay(row, 4, ProgressDisplay2());
				perf_list.Set(row, 5, new_succ_value);
				perf_list.SetDisplay(row, 5, SuccessDisplay());
				perf_list.Set(row, 6, (new_value * 2 + new_succ_value) / 30);
				perf_list.Set(row, 7, i);
				perf_list.Set(row, 8, j);
				
				row++;
			}
		}
		
		perf_list.SetSortColumn(6, true);
		
		perf_list.WhenAction.Clear();
		for(int i = 0; i < perf_list.GetCount(); i++) {
			int comb2 = perf_list.Get(i, 7);
			int lengthi2 = perf_list.Get(i, 8);
			if (comb2 == comb && lengthi2 == lengthi) {
				perf_list.SetCursor(i);
				break;
			}
		}
		perf_list.ScrollTo(scroll);
		perf_list.WhenAction << THISBACK(Data);
	}
	else if (tab == 2) {
		int cursor = -1;
		if (trader_list.IsCursor()) {
			cursor = trader_list.GetCursor();
			trader_candles.trader = trader_list.Get(cursor, 7);
		}
		int tfi = trader_tf.GetIndex();
		int shift = matrix.shift;
		if (tfi)
			shift = dbc.GetTimeIndex(s.tfs[tfi]).Find(SyncTime(s.tfs[tfi], idx0[shift]));
		trader_candles.tfi = tfi;
		trader_candles.shift = shift;
		trader_candles.Data();
		trader_candles.Refresh();
		
		for(int i = 0; i < s.traders.GetCount(); i++) {
			const TraderItem& ti = s.traders[i];
			
			trader_list.Set(i, 0, s.startv[ti.comb]);
			trader_list.Set(i, 1, s.sizev[ti.comb]);
			trader_list.Set(i, 2, ti.scorelvl);
			trader_list.Set(i, 3, ti.tfi);
			trader_list.Set(i, 4, ti.reqav);
			trader_list.Set(i, 5, ti.reqsucc);
			
			double last_value = ti.data[0].Top().open;
			trader_list.Set(i, 6, last_value);
			trader_list.Set(i, 7, i);
			
		}
		trader_list.SetSortColumn(6, true);
		
		if (cursor != -1) {
			trader_list.WhenAction.Clear();
			trader_list.SetCursor(cursor);
			trader_list.WhenAction << THISBACK(Data);
		}
	}
}

String SpeculationCtrl::GetTitle() {
	return "Speculation";
}










void GlobalSuccessCtrl::Paint(Draw& d) {
	Speculation& s = GetSpeculation();
	if (shift < 0 || shift >= s.data.GetCount()) return;
	SpecItem& si = s.data[shift];
	if (si.values.IsEmpty()) return;
	
	Rect r(GetSize());
	
	double col = r.GetWidth() / s.tfs.GetCount();
	
	for(int i = 0; i < s.tfs.GetCount(); i++) {
		int blend = si.succ_tf[i] * 255;
		Color c = Blend(Black, LtYellow, blend);
		d.DrawRect(i * col, 0, col + 1, r.Height(), c);
	}
	for(int i = 1; i < s.tfs.GetCount(); i++) {
		d.DrawLine(i * col, 0, i * col, r.Height(), 1, GrayColor());
	}
}

void SpeculationMatrixCtrl::Paint(Draw& d) {
	System& sys = GetSystem();
	Speculation& s = GetSpeculation();
	int cur_count = Speculation::cur_count;
	
	int tf_count = s.tfs.GetCount();
	
	Size sz(GetSize());
	Rect r(sz);
	d.DrawRect(sz, White());
	
	if (shift < 0 || shift >= s.data.GetCount()) return;
	SpecItem& si = s.data[shift];
	if (si.values.IsEmpty()) return;
	
	int grid_count = cur_count+1;
	double row = r.GetHeight() / (double)grid_count;
	double col = r.GetWidth() / (double)grid_count;
	double subrow = row / 2;
	double subsubrow = subrow / 3;
	double subsubsubrow = subsubrow / 2;
	double subcol = col / tf_count;
	Font fnt = Arial(subrow * 0.6);
	
	for(int i = 0; i < cur_count; i++) {
		String a = s.sym[i].Left(3);
		
		int x = 0;
		int y = (1 + i) * row;
		Size a_size = GetTextSize(a, fnt);
		d.DrawText(x + (col - a_size.cx) / 2, y + (subrow - a_size.cy) / 2, a, fnt);
		
		for(int j = 0; j < tf_count; j++) {
			bool b = si.values[i * tf_count + j];
			bool ab = si.avvalues[i * tf_count + j];
			bool su = si.succ[i * tf_count + j];
			x = j * subcol;
			y = (1 + i) * row + subrow;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, si.GetColor(j, b));
			y = (1 + i) * row + subrow + subsubrow;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, si.GetColor(j, ab));
			y = (1 + i) * row + subrow + subsubrow * 2;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, s.GetSuccessColor(su));
		}
		
		x = (1 + i) * col;
		y = 0;
		d.DrawText(x + (col - a_size.cx) / 2, y + (subrow - a_size.cy) / 2, a, fnt);
		for(int j = 0; j < tf_count; j++) {
			bool b = si.values[i * tf_count + j];
			bool ab = si.avvalues[i * tf_count + j];
			bool su = si.succ[i * tf_count + j];
			x = (1 + i) * col + j * subcol;
			y = subrow;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, si.GetColor(j, b));
			y = subrow + subsubrow;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, si.GetColor(j, ab));
			y = subrow + subsubrow * 2;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, s.GetSuccessColor(su));
		}
		
		
		for(int j = 0; j < cur_count; j++) {
			if (i == j) continue;
			String b = s.sym[j].Left(3);
			String ab = a + b + sys.GetPostFix();
			String ba = b + a + sys.GetPostFix();
			String sym;
			bool is_ab = s.sym.Find(ab) != -1;
			if (is_ab)
				sym = ab;
			else
				sym = ba;
			int sympos = s.sym.Find(sym);
			
			x = (1 + i) * col;
			y = (1 + j) * row;
			
			/*int mtsym_id = ses.FindSymbolLeft(sym);
			ASSERT(mtsym_id != -1);
			String mtsym = ses.GetSymbol(mtsym_id);
			bool has_orders = ses.HasOrders(mtsym);
			if (has_orders) {
				bool sig = ses.GetOrderSig(mtsym);
				d.DrawRect(x, y, col + 1, row + 1, s.GetPaper(sig));
			}*/
			
			Size s_size = GetTextSize(sym, fnt);
			d.DrawText(x + (col - s_size.cx) / 2, y + (subrow - s_size.cy) / 2, sym, fnt);
			for(int k = 0; k < tf_count; k++) {
				x = (1 + i) * col + k * subcol;
				y = (1 + j) * row + subrow;
				
				bool b = si.values[sympos * tf_count + k];
				bool ab = si.avvalues[sympos * tf_count + k];
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, si.GetColor(k, b));
				y = (1 + j) * row + subrow + subsubsubrow;
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, si.GetColor(k, ab));
				
				y = (1 + j) * row + subrow + subsubrow;
				bool av = si.values[i * tf_count + k];
				bool bv = si.values[j * tf_count + k];
				int sum = 0;
				if (is_ab) {
					sum += av ? -1 : +1;
					sum += bv ? +1 : -1;
				} else {
					sum += av ? +1 : -1;
					sum += bv ? -1 : +1;
				}
				Color c;
				if (sum > 0)		c = si.GetColor(k, 0);
				else if (sum < 0)	c = si.GetColor(k, 1);
				else				c = si.GetGrayColor(k);
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, c);
				
				y = (1 + j) * row + subrow + subsubrow + subsubsubrow;
				bool aav = si.avvalues[i * tf_count + k];
				bool abv = si.avvalues[j * tf_count + k];
				sum = 0;
				if (is_ab) {
					sum += aav ? -1 : +1;
					sum += abv ? +1 : -1;
				} else {
					sum += aav ? +1 : -1;
					sum += abv ? -1 : +1;
				}
				if (sum > 0)		c = si.GetColor(k, 0);
				else if (sum < 0)	c = si.GetColor(k, 1);
				else				c = si.GetGrayColor(k);
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, c);
				
				y = (1 + j) * row + subrow + subsubrow * 2;
				bool su = si.succ[sympos * tf_count + k];
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, s.GetSuccessColor(su));
				
				y = (1 + j) * row + subrow + subsubrow * 2 + subsubsubrow;
				bool as = si.succ[i * tf_count + k];
				bool bs = si.succ[j * tf_count + k];
				sum = as * 1 + bs * 1;
				c = Blend(s.GetSuccessColor(0), s.GetSuccessColor(1), sum * 255 / 2);
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, c);
			}
		}
	}
	
	for(int i = 0; i < cur_count; i++) {
		int y = (1 + i) * row;
		int x = (1 + i) * col;
		int y2 = (1 + i) * row + subrow;
		int y3 = (1 + i) * row + subrow + subsubrow;
		int y4 = (1 + i + 1) * row;
		int y5 = (1 + i) * row + subrow + subsubsubrow;
		int y6 = (1 + i) * row + subrow + subsubrow + subsubsubrow;
		int y7 = (1 + i) * row + subrow + subsubrow * 2;
		int y8 = (1 + i) * row + subrow + subsubrow * 2 + subsubsubrow;
		
		for(int j = 0; j < cur_count; j++) {
			if (j == i) continue;
			for(int k = 1; k < tf_count; k++) {
				int x = (1 + j) * col + k * subcol;
				d.DrawLine(x, y2, x, y4, 1, GrayColor());
			}
		}
		
		for(int j = 1; j < tf_count; j++) {
			int x = j * subcol;
			d.DrawLine(x, y2, x, y4, 1, GrayColor());
			x = (1 + i) * col + j * subcol;
			d.DrawLine(x, subrow, x, row, 1, GrayColor());
		}
		
		d.DrawLine(0, y2, r.GetWidth(), y2, 1, GrayColor());
		d.DrawLine(0, y3, r.GetWidth(), y3, 1, GrayColor());
		d.DrawLine(col, y5, r.GetWidth(), y5, 1, GrayColor());
		d.DrawLine(col, y6, r.GetWidth(), y6, 1, GrayColor());
		d.DrawLine(col, y7, r.GetWidth(), y7, 1, GrayColor());
		d.DrawLine(col, y8, r.GetWidth(), y8, 1, GrayColor());
		d.DrawLine(0, y, r.GetWidth(), y, 1, Black());
		d.DrawLine(x, 0, x, r.GetHeight(), 1, Black());
	}
	d.DrawLine(col, subrow, r.GetWidth(), subrow, 1, GrayColor());
	d.DrawLine(col, subrow + subsubrow, r.GetWidth(), subrow + subsubrow, 1, GrayColor());
}




void ProgressDisplayCls2::Paint(Draw& w, const Rect& _r, const Value& q,
                               Color ink, Color paper, dword s) const
{
	Rect r = _r;
	r.top += 1;
	r.bottom -= 1;
	DrawBorder(w, r, InsetBorder);
	r.Deflate(2);
	int pos = minmax(int((double)q * r.Width() / 1000), 0, r.Width());
	Color c = (double)q >= 500.0 ? Green : LtBlue;
	if(pos) {
		w.DrawRect(r.left, r.top, 1, r.Height(), SColorLight);
		w.DrawRect(r.left + 1, r.top, pos - 1, 1, SColorLight);
		w.DrawRect(r.left + 1, r.top + 1, pos - 1, r.Height() - 2, c);
		w.DrawRect(r.left + 1, r.top + r.Height() - 1, pos - 1, 1, SColorLight);
		w.DrawRect(r.left + pos - 1, r.top + 1, 1, r.Height() - 1, SColorLight);
	}
	w.DrawRect(r.left + pos, r.top, r.Width() - pos, r.Height(), SColorPaper);
};

Display& ProgressDisplay2()
{
	return Single<ProgressDisplayCls2>();
}

void SuccessDisplayCls::Paint(Draw& w, const Rect& _r, const Value& q,
                               Color ink, Color paper, dword s) const
{
	Rect r = _r;
	r.top += 1;
	r.bottom -= 1;
	DrawBorder(w, r, InsetBorder);
	r.Deflate(2);
	int pos = minmax(int((double)q * r.Width() / 1000), 0, r.Width());
	Color c = (double)q >= 500.0 ? Color(160, 26, 0) : Color(163, 162, 0);
	if(pos) {
		w.DrawRect(r.left, r.top, 1, r.Height(), SColorLight);
		w.DrawRect(r.left + 1, r.top, pos - 1, 1, SColorLight);
		w.DrawRect(r.left + 1, r.top + 1, pos - 1, r.Height() - 2, c);
		w.DrawRect(r.left + 1, r.top + r.Height() - 1, pos - 1, 1, SColorLight);
		w.DrawRect(r.left + pos - 1, r.top + 1, 1, r.Height() - 1, SColorLight);
	}
	w.DrawRect(r.left + pos, r.top, r.Width() - pos, r.Height(), SColorPaper);
};

Display& SuccessDisplay()
{
	return Single<SuccessDisplayCls>();
}


void CodeDisplayCls::Paint(Draw& w, const Rect& r, const Value& q,
                               Color ink, Color paper, dword s) const
{
	double col = r.Width() / 5.0;
	double row = r.Height() / 3.0;
	
	int code = q;
	int bit = 0;
	for(int i = 0; i < 5; i++) {
		int x = r.left + i * col, y;
		
		bool value = code & (1 << bit);
		bit++;
		bool avvalue = code & (1 << bit);
		bit++;
		bool succ = code & (1 << bit);
		bit++;
		
		y = r.top + 0 * row;
		w.DrawRect(x, y, col+1, row+1, value ? LtBlue : LtGreen);
		y = r.top + 1 * row;
		w.DrawRect(x, y, col+1, row+1, avvalue ? LtBlue : LtGreen);
		y = r.top + 2 * row;
		w.DrawRect(x, y, col+1, row+1, succ ? LtYellow : Black);
	}
	
	for(int i = 1; i < 5; i++) {
		int x = r.left + i * col, y;
		w.DrawLine(x, r.top, x, r.bottom, 1, GrayColor());
	}
	for(int i = 1; i < 3; i++) {
		int y = r.top + i * row;
		w.DrawLine(r.left, y, r.right, y, 1, GrayColor());
	}
};

Display& CodeDisplay()
{
	return Single<CodeDisplayCls>();
}



#define DATAUP Color(56, 212, 150)
#define DATADOWN Color(28, 85, 150)
#define DATAUP_DARK Color(0, 138, 78)
#define DATADOWN_DARK Color(23, 58, 99)

void CandlestickCtrl::Paint(Draw& d) {
	int f, pos, x, y, h, c, w;
	double diff;
    Rect r(GetSize());
	d.DrawRect(r, White());
	
	double hi = -DBL_MAX, lo = +DBL_MAX;
	for(double d : lows)
		if (d < lo) lo = d;
	for(double d : highs)
		if (d > hi) hi = d;
	
	int div = 8;
	count = r.GetWidth() / div;
	
    f = 2;
    x = border - (div - r.GetWidth() % div);
	y = r.top;
    h = r.GetHeight();
    w = r.GetWidth();
	c = opens.GetCount();
	diff = hi - lo;
	
	//d.DrawText(0, 0, GetSystem().GetSymbol(sym), Arial(28));
	
	int cs_h = h * 0.7;
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O, H, L, C;
        pos = c - (count - i);
        if (pos >= opens.GetCount() || pos < 0) continue;
        
        double open  = opens[pos];
        double low   = lows[pos];
        double high  = highs[pos];
		double close =
			pos+1 < c ?
				opens[pos+1] :
				open;
        
        O = (1 - (open  - lo) / diff) * cs_h;
        H = (1 - (high  - lo) / diff) * cs_h;
        L = (1 - (low   - lo) / diff) * cs_h;
        C = (1 - (close - lo) / diff) * cs_h;
		
		P <<
			Point((int)(x+i*div+f),		(int)(y+O)) <<
			Point((int)(x+(i+1)*div-f),	(int)(y+O)) <<
			Point((int)(x+(i+1)*div-f),	(int)(y+C)) <<
			Point((int)(x+i*div+f),		(int)(y+C)) <<
			Point((int)(x+i*div+f),		(int)(y+O));
        
        {
	        Color c, c2;
	        if (C < O) {c = DATAUP; c2 = DATAUP_DARK;}
	        else {c = DATADOWN; c2 = DATADOWN_DARK;}
	        
	        d.DrawLine(
				(int)(x+(i+0.5)*div), (int)(y+H),
				(int)(x+(i+0.5)*div), (int)(y+L),
				2, c2);
	        d.DrawPolygon(P, c, 1, c2);
        }
    }
    
    int indi_h = h - cs_h;
    y += cs_h;
    hi = -DBL_MAX;
    lo = +DBL_MAX;
	for(double d : specs) {
		if (d < lo) lo = d;
		if (d > hi) hi = d;
	}
	diff = hi - lo;
	
	cache.SetCount(0);
	
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O;
        pos = c - (count - i);
        if (pos >= specs.GetCount() || pos < 0) continue;
        
        double spec  = specs[pos];
        bool b = bools[pos];
        
        O = (1 - (spec  - lo) / diff) * indi_h;
		
		cache <<
			Point((int)(x+(i+0.5)*div),		(int)(y+O));
		
		int rx = x + i * div + 1;
		d.DrawRect(rx, y, div-2, div-2, b ? Color(28, 127, 255) : Color(28, 212, 0));
    }
    if (!cache.IsEmpty())
		d.DrawPolyline(cache, 1, Green());
    
    double O = (1 - (0  - lo) / diff) * indi_h;
    if (IsFin(O)) {
	    y += O;
	    d.DrawLine(0, y, w, y, 1, Black());
    }
}

void CandlestickCtrl::Data() {
	Speculation& s = GetSpeculation();
	const SpecBarData& sbd = s.GetSpecBarData(tfi, comb, lengthi);
	if (shift < 0 || shift >= sbd.data.GetCount()) return;
	
	int count = min(shift+1, this->count);
	Vector<double> opens, lows, highs;
	Vector<bool> bools;
	opens.SetCount(count);
	lows.SetCount(count);
	highs.SetCount(count);
	specs.SetCount(count);
	bools.SetCount(count);
	for(int i = 0; i < count; i++) {
		int pos = shift+1 - count + i;
		const SpecBarDataItem& sbdi = sbd.data[pos];
		
		opens[i] = sbdi.open;
		lows[i] = sbdi.low;
		highs[i] = sbdi.high;
		specs[i] = sbdi.spec;
		bools[i] = sbdi.signal;
	}
	Swap(this->highs, highs);
	Swap(this->lows, lows);
	Swap(this->opens, opens);
	Swap(this->specs, specs);
	Swap(this->bools, bools);
	
	
}



void CandlestickCtrl2::Paint(Draw& d) {
	int f, pos, x, y, h, c, w;
	double diff;
    Rect r(GetSize());
	d.DrawRect(r, White());
	
	double hi = -DBL_MAX, lo = +DBL_MAX;
	for(double d : lows)
		if (d < lo) lo = d;
	for(double d : highs)
		if (d > hi) hi = d;
	
	int div = 8;
	count = r.GetWidth() / div;
	
    f = 2;
    x = border - (div - r.GetWidth() % div);
	y = r.top;
    h = r.GetHeight();
    w = r.GetWidth();
	c = opens.GetCount();
	diff = hi - lo;
	
	//d.DrawText(0, 0, GetSystem().GetSymbol(sym), Arial(28));
	
	int cs_h = h;
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O, H, L, C;
        pos = c - (count - i);
        if (pos >= opens.GetCount() || pos < 0) continue;
        
        double open  = opens[pos];
        double low   = lows[pos];
        double high  = highs[pos];
		double close =
			pos+1 < c ?
				opens[pos+1] :
				open;
        
        O = (1 - (open  - lo) / diff) * cs_h;
        H = (1 - (high  - lo) / diff) * cs_h;
        L = (1 - (low   - lo) / diff) * cs_h;
        C = (1 - (close - lo) / diff) * cs_h;
		
		P <<
			Point((int)(x+i*div+f),		(int)(y+O)) <<
			Point((int)(x+(i+1)*div-f),	(int)(y+O)) <<
			Point((int)(x+(i+1)*div-f),	(int)(y+C)) <<
			Point((int)(x+i*div+f),		(int)(y+C)) <<
			Point((int)(x+i*div+f),		(int)(y+O));
        
        {
	        Color c, c2;
	        if (C < O) {c = DATAUP; c2 = DATAUP_DARK;}
	        else {c = DATADOWN; c2 = DATADOWN_DARK;}
	        
	        d.DrawLine(
				(int)(x+(i+0.5)*div), (int)(y+H),
				(int)(x+(i+0.5)*div), (int)(y+L),
				2, c2);
	        d.DrawPolygon(P, c, 1, c2);
        }
    }
}

void CandlestickCtrl2::Data() {
	Speculation& s = GetSpeculation();
	if (trader < 0 || trader >= s.traders.GetCount()) return;
	const TraderItem& ti = s.traders[trader];
	if (shift < 0 || shift >= ti.data[tfi].GetCount()) return;
	
	/*const TraderItemData& tid0 = ti.data[shift];
	if (tid0.bdspeci >= 0) {
		int comb1 = tid0.bdspeci / s.lengths.GetCount();
		int lengthi1 = tid0.bdspeci % s.lengths.GetCount();
		ReleaseLog(Format("CandlestickCtrl2::Data trader=%d bdspeci=%d comb=%d lengthi=%d", trader, tid0.bdspeci, comb1, lengthi1));
		const SpecBarData& sbd = s.GetSpecBarData(0, comb1, lengthi1);
		const SpecBarDataItem& sbdi = sbd.data[shift];
		for(int i = 0; i < sbdi.syms.GetCount(); i++) {
			ReleaseLog(Format("CandlestickCtrl2::Data %d %d %d", i, sbdi.syms.GetKey(i), (int)sbdi.syms[i]));
		}
	}*/
	
	int count = min(shift+1, this->count);
	Vector<double> opens, lows, highs;
	opens.SetCount(count);
	lows.SetCount(count);
	highs.SetCount(count);
	for(int i = 0; i < count; i++) {
		int pos = shift+1 - count + i;
		const TraderItemData& tid = ti.data[tfi][pos];
		
		opens[i] = tid.open;
		lows[i] = tid.low;
		highs[i] = tid.high;
	}
	Swap(this->highs, highs);
	Swap(this->lows, lows);
	Swap(this->opens, opens);
}

}
