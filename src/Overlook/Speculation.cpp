#include "Overlook.h"

namespace Overlook {

const Color SpecItem::PositiveColor = Color(28, 212, 0);
const Color SpecItem::NegativeColor = Color(56, 127, 255);

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
	tfs.Add(1);
	tfs.Add(2);
	tfs.Add(4);
	tfs.Add(5);
	tfs.Add(6);
	if (SpecBarDataItem::tf_count != tfs.GetCount()) Panic("Fatal error");
	
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
	
	
	for(int i = 0; i < tfs.GetCount(); i++) {
		double mult = GetMult(i);
		max_sum += mult + mult * 2 + mult + mult * 2;
		max_succ_sum += mult + mult * 2;
		bd_max_sum += mult * 2;
		bd_max_succ_sum += mult;
		cur_slow_max += mult * 2;
		cur_succ_max += mult;
	}
	
	lengths.Add(1);
	lengths.Add(2);
	lengths.Add(4);
	lengths.Add(8);
	
	bardata.SetCount(tfs.GetCount());
	for(int i = 0; i < bardata.GetCount(); i++)
		bardata[i].SetCount(lengths.GetCount() * prio_count * 2);
	bardataspecosc.SetCount(tfs.GetCount());
	
	
	for(int i = 0; i < cur_count; i++) {
		String a = sym[i].Left(3);
		for(int j = 0; j < cur_count; j++) {
			String b = sym[j].Left(3);
			String ab = a + b + GetSystem().GetPostFix();
			String ba = b + a + GetSystem().GetPostFix();
			String sym;
			bool is_ab = this->sym.Find(ab) != -1;
			bool invert = !is_ab;
			if (is_ab)
				sym = ab;
			else
				sym = ba;
			int sympos = this->sym.Find(sym);
			is_abv.Add(is_ab);
			symposv.Add(sympos);
		}
	}
	
	
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
	
	RefreshItems();
	
	RefreshNeuralItems();
	
	RefreshBarData();
	
	RefreshBarDataSpec();
	
	RefreshBarDataOscillators();
	
	RefreshBarDataNeuralItems();
	
	if (counted < bars && ts.Elapsed() > 5*60*1000) {
		StoreThis();
		ts.Reset();
	}
}

void Speculation::RefreshItems() {
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
}

void Speculation::ProcessItem(int pos, SpecItem& si) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	
	Time t = idx0[pos];
	
	Vector<int> shift_posv;
	for(int i = 0; i < tfs.GetCount(); i++)
		shift_posv.Add(dbc.GetTimeIndex(tfs[i]).Find(SyncTime(tfs[i], t)));
	
	int symtf_count = sym.GetCount() * tfs.GetCount();
	int symstart_count = sym.GetCount();
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
				default:
						steps = 2; break;
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
	
	
	si.cur_score.Clear();
	bool expect_1 = false;
	for(int i = 0; i < cur_count; i++) {
		double slow_sum = 0, succ_sum = 0;
		bool expect_plus1 = true, expect_minus1 = true;
		for(int j = 0; j < tfs.GetCount(); j++) {
			int ij = i * tfs.GetCount() + j;
			bool v = si.values[ij];
			bool avv = si.avvalues[ij];
			bool sv = si.succ[ij];
			double slow_mult = GetMult(j);
			
			slow_sum += (v ? -1 : +1) * slow_mult;
			slow_sum += (avv ? -1 : +1) * slow_mult;
			succ_sum += (v ? -1 : +1) * sv * slow_mult;
			
			if (!v || !avv || !sv) expect_plus1 = false;
			if (v || avv || !sv) expect_minus1 = false;
		}
		double pos_score = (+2 * slow_sum / cur_slow_max + succ_sum / cur_succ_max) / 3.0;
		si.cur_score.Add(i, pos_score);
	}
	SortByValue(si.cur_score, StdGreater<double>());
	
	
	VectorMap<int, int> prio_syms;
	for(int i = cur_count; i < sym.GetCount(); i++) {
		int ai = aiv[i];
		int bi = biv[i];
		int ai_scorepos = si.cur_score.Find(ai);
		int bi_scorepos = si.cur_score.Find(bi);
		bool a_is_neg = ai_scorepos >= cur_count/2;
		bool b_is_neg = bi_scorepos >= cur_count/2;
		bool ai_is_prio = !a_is_neg ? ai_scorepos < cur_count/4 : ai_scorepos >= cur_count*3/4;
		bool bi_is_prio = !b_is_neg ? bi_scorepos < cur_count/4 : bi_scorepos >= cur_count*3/4;
		int ai_prio = !a_is_neg ? -ai_scorepos : +ai_scorepos;
		int bi_prio = !b_is_neg ? -bi_scorepos : +bi_scorepos;
		if (ai_is_prio && bi_is_prio && a_is_neg != b_is_neg)
			prio_syms.Add(i,  ai_prio + bi_prio);
				
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
			double slow_mult = GetMult(j);
			double symtf_sum = 0;
			symtf_sum += (v ? -1 : +1) * slow_mult;
			symtf_sum += (avv ? -1 : +1) * slow_mult;
			int ab = (av ? -1 : +1) + (bv ? +1 : -1);
			int aab = (aav ? -1 : +1) + (abv ? +1 : -1);
			if (ab) symtf_sum += (ab < 0 ? -2 : +2) * slow_mult;
			if (aab) symtf_sum += (aab < 0 ? -2 : +2) * slow_mult;
			
			SpecItemData& d = si.data[i];
			d.slow_sum += symtf_sum;
			d.succ_sum += ((v ? -1 : +1) * sv + (av ? -1 : +1) * sav + (bv ? +1 : -1) * sbv) * slow_mult;
			
			si.succ_tf[j] += (sv + sav + sbv) / ((sym.GetCount() - cur_count) * 3.0);
		}
	}
	SortByValue(prio_syms, StdGreater<int>());
	si.prio_syms.Clear();
	for(int i = 0; i < prio_syms.GetCount(); i++)
		si.prio_syms.Add(prio_syms.GetKey(i));
	
}

void Speculation::RefreshNeuralItems() {
	
	if (ses.GetStepCount() == 0) {
		System& sys = GetSystem();
		SpecItem& si0 = data[0];
		int data_count = data.GetCount() - 61;
		int tf_count = tfs.GetCount();
		
		int width = (cur_count+1)*tfs.GetCount();
		int height = (cur_count+1)*6;
		int depth = 1;
		int output_size = sym.GetCount();
		
		ConvNet::SessionData& d = ses.GetData();
		d.BeginDataResult(output_size, data_count, width, height, depth, 0);
		
		for(int pos = 0; pos < data_count; pos++) {
			const SpecItem& si = data[pos];
			
			int result_row = 0;
			for(int i = 0; i < cur_count; i++) {
				int sym_id_a = si.cur_score.GetKey(i);
				CoreList& c0 = cl[sym_id_a][0];
				DataBridge& dbm1 = *c0.GetDataBridgeM1(0);
				ConstBuffer& open_m1 = dbm1.GetBuffer(0);
				double o0 = open_m1.Get(pos);
				double o1 = open_m1.Get(pos + 60);
				double change = (o1 / o0 - 1.0) * 100;
				d.SetResult(pos, result_row++, change);
			}
			
			for(int i = 0; i < cur_count; i++) {
				int sym_id_a = si.cur_score.GetKey(i);
				
				for(int j = 0; j < tf_count; j++) {
					bool b = si.values		[sym_id_a * tf_count + j];
					bool ab = si.avvalues	[sym_id_a * tf_count + j];
					bool su = si.succ		[sym_id_a * tf_count + j];
					
					d.SetData(pos, j, (1+i)*6 + 0, 0, b  * 1.0);
					d.SetData(pos, j, (1+i)*6 + 1, 0, ab * 1.0);
					d.SetData(pos, j, (1+i)*6 + 2, 0, b  * 1.0);
					d.SetData(pos, j, (1+i)*6 + 3, 0, ab * 1.0);
					d.SetData(pos, j, (1+i)*6 + 4, 0, su * 1.0);
					d.SetData(pos, j, (1+i)*6 + 5, 0, su * 1.0);
					
					d.SetData(pos, (1+cur_count-1-i)*tf_count + j, 0, 0, b  * 1.0);
					d.SetData(pos, (1+cur_count-1-i)*tf_count + j, 1, 0, ab * 1.0);
					d.SetData(pos, (1+cur_count-1-i)*tf_count + j, 2, 0, b  * 1.0);
					d.SetData(pos, (1+cur_count-1-i)*tf_count + j, 3, 0, ab * 1.0);
					d.SetData(pos, (1+cur_count-1-i)*tf_count + j, 4, 0, su * 1.0);
					d.SetData(pos, (1+cur_count-1-i)*tf_count + j, 5, 0, su * 1.0);
				}
				
				for(int j = 0; j < cur_count; j++) {
					if (i == j) {
						for(int k = 0; k < tf_count; k++)
							for (int l = 0; l < 6; l++)
								d.SetData(pos, (1+cur_count-1-j)*tf_count+k, (1+i)*6+l, 0, 0);
						continue;
					}
					
					int sym_id_b = si.cur_score.GetKey(j);
					
					bool is_ab = is_abv[sym_id_a * cur_count + sym_id_b];
					bool invert = !is_ab;
					int sympos = symposv[sym_id_a * cur_count + sym_id_b];
					
					for(int k = 0; k < tf_count; k++) {
						int x = (1+cur_count-1-j)*tf_count+k;
						int y = (1+i)*6;
						
						bool b = si.values[sympos * tf_count + k];
						bool ab = si.avvalues[sympos * tf_count + k];
						if (invert) {
							b = !b;
							ab = !ab;
						}
						d.SetData(pos, x, y+0, 0, b*1.0);
						d.SetData(pos, x, y+1, 0, ab*1.0);
						
						bool av = si.values[sym_id_a * tf_count + k];
						bool bv = si.values[sym_id_b * tf_count + k];
						
						double sum = 0;
						sum += av ? 0.5 : 0;
						sum += bv ? 0 : 0.5;
						d.SetData(pos, x, y+2, 0, sum);
						
						bool aav = si.avvalues[sym_id_a * tf_count + k];
						bool abv = si.avvalues[sym_id_b * tf_count + k];
						
						sum = 0;
						sum += aav ? 0.5 : 0;
						sum += abv ? 0 : 0.5;
						d.SetData(pos, x, y+3, 0, sum);
						
						bool su = si.succ[sympos * tf_count + k];
						d.SetData(pos, x, y+4, 0, su*1.0);
						
						bool as = si.succ[sym_id_a * tf_count + k];
						bool bs = si.succ[sym_id_b * tf_count + k];
						sum = as * 0.5 + bs * 0.5;
						d.SetData(pos, x, y+5, 0, sum);
					}
					
					if (i < j) {
						CoreList& c0 = cl[sympos][0];
						DataBridge& dbm1 = *c0.GetDataBridgeM1(0);
						ConstBuffer& open_m1 = dbm1.GetBuffer(0);
						
						double o0 = open_m1.Get(pos);
						double o1 = open_m1.Get(pos + 60);
						double change = (o1 / o0 - 1.0) * 100;
						if (!is_ab) change *= 1.0;
						
						d.SetResult(pos, result_row++, change);
					}
				}
			}
			
			for(int i = 0; i < tfs.GetCount(); i++)
				for(int j = 0; j < 6; j++)
					d.SetData(pos, i, j, 0, si.succ_tf[i]);
		}
		
		
		InitSessionDefault(ses, width, height, depth, output_size);
	}
	
	total = 200000;
	TrainSession(ses, total, actual);
	
	ses.GetData().ClearData();
	
	ConvNet::Volume vol;
	for (int i = data.GetCount()-1; i >= 0; i--) {
		SpecItem& si = data[i];
		
		if (si.nn.GetCount()) break;
		
		si.nn.SetCount(sym.GetCount(), 0);
		
		LoadVolume(i, vol);
		
		ConvNet::Net& net = ses.GetNetwork();
		ConvNet::Volume& out = net.Forward(vol);
		
		int result_row = 0;
		for(int j = 0; j < cur_count; j++)
			si.nn[si.cur_score.GetKey(j)] = out.Get(result_row++);
		for(int j = 0; j < cur_count; j++) {
			int sym_id_a = si.cur_score.GetKey(j);
			for(int k = 0; k < cur_count; k++) {
				int sym_id_b = si.cur_score.GetKey(k);
				if (j < k) {
					int sympos = symposv[sym_id_a * cur_count + sym_id_b];
					bool is_ab = is_abv[sym_id_a * cur_count + sym_id_b];
					double change = out.Get(result_row++);
					if (!is_ab) change *= -1;
					si.nn[sympos] = change;
				}
			}
		}
	}
	
}

void Speculation::LoadVolume(int pos, ConvNet::Volume& vol) {
	int tf_count = tfs.GetCount();
	int width = (cur_count+1)*tfs.GetCount();
	int height = (cur_count+1)*6;
	int depth = 1;
	int output_size = sym.GetCount();
	
	vol.Init(width, height, depth, 0);
	
	const SpecItem& si = data[pos];
	
	for(int i = 0; i < cur_count; i++) {
		int sym_id_a = si.cur_score.GetKey(i);
		
		for(int j = 0; j < tf_count; j++) {
			bool b = si.values		[sym_id_a * tf_count + j];
			bool ab = si.avvalues	[sym_id_a * tf_count + j];
			bool su = si.succ		[sym_id_a * tf_count + j];
			
			vol.Set(j, (1+i)*6 + 0, 0, b  * 1.0);
			vol.Set(j, (1+i)*6 + 1, 0, ab * 1.0);
			vol.Set(j, (1+i)*6 + 2, 0, b  * 1.0);
			vol.Set(j, (1+i)*6 + 3, 0, ab * 1.0);
			vol.Set(j, (1+i)*6 + 4, 0, su * 1.0);
			vol.Set(j, (1+i)*6 + 5, 0, su * 1.0);
			
			vol.Set((1+cur_count-1-i)*tf_count + j, 0, 0, b  * 1.0);
			vol.Set((1+cur_count-1-i)*tf_count + j, 1, 0, ab * 1.0);
			vol.Set((1+cur_count-1-i)*tf_count + j, 2, 0, b  * 1.0);
			vol.Set((1+cur_count-1-i)*tf_count + j, 3, 0, ab * 1.0);
			vol.Set((1+cur_count-1-i)*tf_count + j, 4, 0, su * 1.0);
			vol.Set((1+cur_count-1-i)*tf_count + j, 5, 0, su * 1.0);
		}
		
		for(int j = 0; j < cur_count; j++) {
			if (i == j) {
				for(int k = 0; k < tf_count; k++)
					for (int l = 0; l < 6; l++)
						vol.Set((1+cur_count-1-j)*tf_count+k, (1+i)*6+l, 0, 0);
				continue;
			}
			
			int sym_id_b = si.cur_score.GetKey(j);
			
			bool is_ab = is_abv[sym_id_a * cur_count + sym_id_b];
			bool invert = !is_ab;
			int sympos = symposv[sym_id_a * cur_count + sym_id_b];
			
			for(int k = 0; k < tf_count; k++) {
				int x = (1+cur_count-1-j)*tf_count+k;
				int y = (1+i)*6;
				
				bool b = si.values[sympos * tf_count + k];
				bool ab = si.avvalues[sympos * tf_count + k];
				if (invert) {
					b = !b;
					ab = !ab;
				}
				vol.Set(x, y+0, 0, b*1.0);
				vol.Set(x, y+1, 0, ab*1.0);
				
				bool av = si.values[sym_id_a * tf_count + k];
				bool bv = si.values[sym_id_b * tf_count + k];
				
				double sum = 0;
				sum += av ? 0.5 : 0;
				sum += bv ? 0 : 0.5;
				vol.Set(x, y+2, 0, sum);
				
				bool aav = si.avvalues[sym_id_a * tf_count + k];
				bool abv = si.avvalues[sym_id_b * tf_count + k];
				
				sum = 0;
				sum += aav ? 0.5 : 0;
				sum += abv ? 0 : 0.5;
				vol.Set(x, y+3, 0, sum);
				
				bool su = si.succ[sympos * tf_count + k];
				vol.Set(x, y+4, 0, su*1.0);
				
				bool as = si.succ[sym_id_a * tf_count + k];
				bool bs = si.succ[sym_id_b * tf_count + k];
				sum = as * 0.5 + bs * 0.5;
				vol.Set(x, y+5, 0, sum);
			}
		}
	}
	
	for(int i = 0; i < tfs.GetCount(); i++)
		for(int j = 0; j < 6; j++)
			vol.Set(i, j, 0, si.succ_tf[i]);
	
}

void Speculation::RefreshBarData() {
	int row = 0;
	bool do_print = bardata[0][0].spec_counted = 0;
	for(int j = 0; j < tfs.GetCount(); j++) {
		for (int l = 0; l < lengths.GetCount(); l++) {
			for (int prio_id = 0; prio_id < prio_count; prio_id++) {
				for (int inv = 0; inv < 2; inv++) {
					SpecBarData& sbd = GetSpecBarData(j, l, prio_id, inv);
					if (j == 0)
						RefreshBarDataItemTf0(l, prio_id, inv, sbd);
					else
						RefreshBarDataItemTf(j, sbd, GetSpecBarData(0, l, prio_id, inv));
					
					RefreshBarDataSpeculation(j, sbd);
					
					if (do_print) {
						ReleaseLog(Format("RefreshBarData %d / %d = %n", row, bardata.GetCount(), (double)row / bardata.GetCount() * 100));
						row++;
					}
				}
			}
		}
	}
}

void Speculation::RefreshBarDataItemTf0(int lengthi, int prio_id, bool inv, SpecBarData& sbd) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(0);
	
	int counted = sbd.counted;
	int bars = min(data.GetCount(), idx.GetCount());
	for(int i = 0; i < sym.GetCount(); i++)
		bars = min(bars, cl[i][0].GetBuffer(0,0,0).GetCount());
	
	sbd.data.SetCount(bars);
	sbd.counted = bars;
	int prio_count = 1 << prio_id;
	int length_mins = lengths[lengthi] * 60;
	
	for(int i = counted; i < bars; i++) {
		const SpecItem& si = data[i];
		SpecBarDataItem& sbdi = sbd.data[i];
		
		if (length_mins) {
			for(int j = 0; j < sbd.length_sigs.GetCount(); j++) {
				int sym = sbd.length_sigs.GetKey(j);
				Tuple2<bool, int>& t = sbd.length_sigs[j];
				int begin = t.b;
				int length = i - begin;
				if (length >= length_mins) {
					sbd.length_sigs.Remove(j);
					j--;
				} else {
					sbdi.syms.GetAdd(sym) = t.a;
				}
			}
		}
		
		for(int j = 0; j < si.prio_syms.GetCount() && j < prio_count; j++) {
			int sym_id = si.prio_syms[j];
			const SpecItemData& sid = si.data[sym_id];
			
			int ai = aiv[sym_id];
			int bi = biv[sym_id];
			double ainn = si.nn[ai];
			double binn = si.nn[bi];
			double nn = si.nn[sym_id];
			bool sig = sid.slow_sum < 0;
			bool succ_sig = sid.succ_sum < 0;
			if ((!inv && !sig && !succ_sig && ainn > 0 && binn < 0 && nn > 0) ||
				(!inv &&  sig &&  succ_sig && ainn < 0 && binn > 0 && nn < 0) ||
				( inv &&  sig &&  succ_sig && ainn > 0 && binn < 0 && nn > 0) ||
				( inv && !sig && !succ_sig && ainn < 0 && binn > 0 && nn < 0)) {
				sbdi.syms.GetAdd(sym_id) = sig;
				if (length_mins) {
					Tuple2<bool, int>& t = sbd.length_sigs.GetAdd(sym_id);
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
}

void Speculation::RefreshBarDataItemTf(int tfi, SpecBarData& sbd, SpecBarData& sbd0) {
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
	for(int j = 0; j < lengths.GetCount(); j++) {
		for (int prio_id = 0; prio_id < prio_count; prio_id++) {
			for (int inv = 0; inv < 2; inv++) {
				SpecBarData& sbd = GetSpecBarData(0, j, prio_id, inv);
				RefreshBarDataSpeculation2(j, prio_id, inv, sbd);
			}
		}
	}
}

void Speculation::RefreshBarDataOscillators() {
	if (invosc_max == 0.0) {
		for(int i = 0; i < bardata[0].GetCount(); i++) {
			double mult = ((double)i / (bardata[0].GetCount() - 1)) * 2.0 - 1.0;
			invosc_max += fabs(mult);
		}
	}
	
	RefreshBarDataOscillatorTf0();
	
	for(int i = 1; i < tfs.GetCount(); i++) {
		RefreshBarDataOscillator(i);
	}
	
}

void Speculation::RefreshBarDataOscillatorTf0() {
	BarDataOscillator& bdo = bardataspecosc[0];
	
	const SpecBarData& spd0 = GetSpecBarData(0, 0, 0, 0);
	int counted = bdo.scores.GetCount();
	int bars = spd0.data.GetCount();
	bdo.scores.SetCount(bars, 0);
	bdo.invs.SetCount(bars, 0);
	
	VectorMap<int, double> score_list;
	
	for(int i = counted; i < bars; i++) {
		
		score_list.Clear();
		double sum = 0;
		for(int j = 0; j < bardata[0].GetCount(); j++) {
			const SpecBarData& sbd = bardata[0][j];
			const SpecBarDataItem& spdi = sbd.data[i];
			
			double score = (max(0.0, spdi.slow_sum) / bd_max_sum * 2 + max(0.0, spdi.succ_sum) / bd_max_succ_sum) / 3.0;
			sum += score;
			
			score_list.Add(j, score);
		}
		
		sum /= bardata[0].GetCount();
		sum *= 100;
		bdo.scores[i] = sum;
		
		bdo.score_stats.GetAdd((int)sum, 0)++;
		
		
		double inv_sum = 0;
		SortByValue(score_list, StdGreater<double>());
		for(int j = 0; j < score_list.GetCount(); j++) {
			double mult = ((double)j / (bardata[0].GetCount() - 1)) * -2.0 + 1.0;
			int k = score_list.GetKey(j);
			bool inv = k % 2; //GetSpecBarDataSpecArgs(k, NULL, NULL, inv)
			double d = mult * (inv ? -1 : +1);
			inv_sum += d;
		}
		inv_sum /= invosc_max;
		
		
		bdo.invs[i] = inv_sum;
	}
	SortByKey(bdo.score_stats, StdLess<int>());
	DUMPM(bdo.score_stats);
	
	
	bdo.score_levels.SetCount(lvl_count);
	for(int i = 0; i < lvl_count; i++) {
		int limit = bars * i / lvl_count;
		
		int sum = 0;
		int level = 100;
		for(int j = 0; j < bdo.score_stats.GetCount(); j++) {
			sum += bdo.score_stats[j];
			if (sum >= limit) {
				level = bdo.score_stats.GetKey(j);
				break;
			}
		}
		bdo.score_levels[i] = level;
	}
	DUMPC(bdo.score_levels);
	
	
	
}

void Speculation::RefreshBarDataOscillator(int tfi) {
	BarDataOscillator& bdo = bardataspecosc[tfi];
	BarDataOscillator& bdo0 = bardataspecosc[0];
	
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	const Index<Time>& idx = dbc.GetTimeIndex(tfs[tfi]);
	
	int bars = idx.GetCount();
	int fast_counted = bdo.fast_counted;
	int fast_bars = min(bdo0.scores.GetCount(), idx0.GetCount());
	
	bdo.scores.SetCount(bars, 0);
	bdo.invs.SetCount(bars, 0);
	
	for(int i = fast_counted; i < fast_bars; i++) {
		Time t0 = idx0[i];
		Time t = SyncTime(tfs[tfi], t0);
		int pos = idx.Find(t);
		if (pos == -1) continue;
		
		double score0 = bdo0.scores[i];
		double inv0 = bdo0.invs[i];
		double& score = bdo.scores[pos];
		double& inv = bdo.invs[pos];
		
		if (score == 0) {
			score = score0;
			inv = inv0;
			
			bdo.score_stats.GetAdd((int)score0, 0)++;
		}
	}
	
	bdo.fast_counted = fast_bars;
	
	
	SortByKey(bdo.score_stats, StdLess<int>());
	bdo.score_levels.SetCount(lvl_count);
	for(int i = 0; i < lvl_count; i++) {
		int limit = bars * i / lvl_count;
		
		int sum = 0;
		int level = 100;
		for(int j = 0; j < bdo.score_stats.GetCount(); j++) {
			sum += bdo.score_stats[j];
			if (sum >= limit) {
				level = bdo.score_stats.GetKey(j);
				break;
			}
		}
		bdo.score_levels[i] = level;
	}
	DUMPC(bdo.score_levels);
}

void Speculation::RefreshBarDataSpeculation2(int lengthi, int prio_id, int inv, SpecBarData& sbd) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	
	int counted = sbd.spec2_counted;
	sbd.spec2_counted = sbd.data.GetCount();
	
	for(int i = counted; i < sbd.data.GetCount(); i++) {
		SpecBarDataItem& spdi0 = sbd.data[i];
		double close = spdi0.open;
		
		Time t = idx0[i];
		
		for(int j = 0; j < tfs.GetCount(); j++) {
			int shift_pos = j == 0 ? i : dbc.GetTimeIndex(tfs[j]).Find(SyncTime(tfs[j], t));
			if (shift_pos == -1) continue;
			SpecBarData& spd = GetSpecBarData(j, lengthi, prio_id, inv);
			SpecBarDataItem& spdi = spd.data[shift_pos];
			
			bool b = spdi.signal;
			spdi0.values[j] = b;
			
			double sum = 0;
			for(int k = 0; k < 21; k++) {
				SpecBarDataItem& spdi = spd.data[max(0, shift_pos - k)];
				if (spdi.signal) sum += 1.0;
			}
			sum /= 21;
			b = sum > 0.5;
			spdi0.avvalues[j] = b;
			
			int steps = 0;
			switch (tfs[j]) {
				case 0: steps = 21; break;
				case 2: steps = 4; break;
				default: steps = 2; break;
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
			spdi0.succ[j] = succ;
			
			double slow_mult = GetMult(j);
			spdi0.slow_sum += (!spdi0.values[j]) * slow_mult;
			spdi0.slow_sum += (!spdi0.avvalues[j]) * slow_mult;
			spdi0.succ_sum += (!spdi0.values[j]) * spdi0.succ[j] * slow_mult;
		}
	}
}

void Speculation::GetSpecBarDataSpecArgs(int bdspeci, int& lengthi, int& prio_id, int& inv) {
	inv = bdspeci % 2;
	bdspeci /= 2;
	prio_id = bdspeci % prio_count;
	bdspeci /= prio_count;
	lengthi = bdspeci;
}

void Speculation::RefreshBarDataNeuralItems() {
	int bd_count = bardata[0].GetCount();
	
	if (bdses.GetStepCount() == 0) {
		System& sys = GetSystem();
		const BarDataOscillator& bdo = bardataspecosc[0];
		
		int begin = bd_count * 5;
		int data_count = data.GetCount() - 61 - begin;
		int tf_count = tfs.GetCount();
		
		int width = 3*tfs.GetCount() + 2 + 2;
		int height = bd_count;
		int depth = 1;
		int output_size = bd_count;
		
		ConvNet::SessionData& d = bdses.GetData();
		d.BeginDataResult(output_size, data_count, width, height, depth, 0);
		
		for(int pos = begin; pos < data_count; pos++) {
			for(int i = 0; i < bd_count; i++) {
				const SpecBarData& sbd = bardata[0][i];
				const SpecBarDataItem& sbdi = sbd.data[pos];
				const SpecBarDataItem& sbdi1 = sbd.data[pos + 60];
				
				for(int j = 0; j < tf_count; j++) {
					bool v = sbdi.values[j];
					bool av = sbdi.avvalues[j];
					bool s = sbdi.succ[j];
					
					d.SetData(pos, 0*tf_count+j, i, 0, v  * 1.0);
					d.SetData(pos, 1*tf_count+j, i, 0, av * 1.0);
					d.SetData(pos, 2*tf_count+j, i, 0, s  * 1.0);
					
					double max_sum = fabs(sbdi.slow_sum) / bd_max_sum;
					double succ_sum = fabs(sbdi.succ_sum) / bd_max_succ_sum;
					d.SetData(pos, 3*tf_count+0, i, 0, max_sum);
					d.SetData(pos, 3*tf_count+1, i, 0, succ_sum);
				}
				
				int begin = pos - (i+1) * 5;
				int end = pos - i * 5;
				OnlineAverage1 inv_av, score_av;
				for(int j = begin; j < end; j++) {
					inv_av.Add(bdo.invs[j]);
					score_av.Add(bdo.scores[j]);
				}
				d.SetData(pos, 3*tf_count+2, i, 0, score_av.GetMean());
				d.SetData(pos, 3*tf_count+3, i, 0, inv_av.GetMean());
				
				double o0 = sbdi.open;
				double o1 = sbdi1.open;
				double change = (o1 / o0 - 1.0) * 100;
				d.SetResult(pos, i, change);
			}
		}
		
		InitSessionDefault(bdses, width, height, depth, output_size);
	}
	
	bdtotal = 200000;
	TrainSession(bdses, bdtotal, bdactual);
	
	bdses.GetData().ClearData();
	
	
	ConvNet::Volume vol;
	for (int i = data.GetCount()-1; i >= 0; i--) {
		SpecItem& si = data[i];
		
		if (si.bdnn.GetCount()) break;
		
		si.bdnn.SetCount(sym.GetCount(), 0);
		
		LoadBarDataVolume(i, vol);
		
		ConvNet::Net& net = bdses.GetNetwork();
		ConvNet::Volume& out = net.Forward(vol);
		
		for(int j = 0; j < bd_count; j++) {
			const SpecBarData& sbd = bardata[0][j];
			const SpecBarDataItem& sbdi = sbd.data[i];
			
			double change = out.Get(j);
			
			if (change > 0) {
				for(int k = 0; k < sbdi.syms.GetCount(); k++) {
					int sym_id = sbdi.syms.GetKey(k);
					bool sig = sbdi.syms[k];
					double sym_change = change;
					if (sig) sym_change *= -1.0;
					si.bdnn[sym_id] += sym_change;
				}
			}
		}
	}
}

void Speculation::LoadBarDataVolume(int pos, ConvNet::Volume& vol) {
	System& sys = GetSystem();
	const BarDataOscillator& bdo = bardataspecosc[0];
	
	int bd_count = bardata[0].GetCount();
	int begin = bd_count * 5;
	int data_count = data.GetCount() - 61 - begin;
	int tf_count = tfs.GetCount();
	
	int width = 3*tfs.GetCount() + 2 + 2;
	int height = bd_count;
	int depth = 1;
	
	vol.Init(width, height, depth, 0);
	
	for(int i = 0; i < bd_count; i++) {
		const SpecBarData& sbd = bardata[0][i];
		const SpecBarDataItem& sbdi = sbd.data[pos];
		
		for(int j = 0; j < tf_count; j++) {
			bool v = sbdi.values[j];
			bool av = sbdi.avvalues[j];
			bool s = sbdi.succ[j];
			
			vol.Set(0*tf_count+j, i, 0, v  * 1.0);
			vol.Set(1*tf_count+j, i, 0, av * 1.0);
			vol.Set(2*tf_count+j, i, 0, s  * 1.0);
			
			double max_sum = fabs(sbdi.slow_sum) / bd_max_sum;
			double succ_sum = fabs(sbdi.succ_sum) / bd_max_succ_sum;
			vol.Set(3*tf_count+0, i, 0, max_sum);
			vol.Set(3*tf_count+1, i, 0, succ_sum);
		}
		
		int begin = pos - (i+1) * 5;
		int end = pos - i * 5;
		OnlineAverage1 inv_av, score_av;
		for(int j = begin; j < end; j++) {
			inv_av.Add(bdo.invs[j]);
			score_av.Add(bdo.scores[j]);
		}
		vol.Set(3*tf_count+2, i, 0, score_av.GetMean());
		vol.Set(3*tf_count+3, i, 0, inv_av.GetMean());
	}
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
	
	matrix_tab << matrix << listparent;
	matrix_tab.Horz();
	matrix_tab.SetPos(8000);
	
	listparent.Add(succ_ctrl.TopPos(0, 15).HSizePos());
	listparent.Add(list.VSizePos(15, 30).HSizePos());
	listparent.Add(matrix_prog.BottomPos(15, 15).HSizePos());
	listparent.Add(bd_prog.BottomPos(0, 15).HSizePos());
	list.AddColumn("Symbol");
	list.AddColumn("Sum");
	list.AddColumn("Success");
	list.AddColumn("Sig");
	list.AddColumn("Value");
	list.ColumnWidths("6 5 5 3 1");
	matrix_prog.Set(0, 1);
	bd_prog.Set(0, 1);
	
	perf_tab << perf_list << perf_parent;
	perf_tab.Horz();
	perf_tab.SetPos(4000);
	
	perf_list.AddColumn("Length");
	perf_list.AddColumn("Priority-count");
	perf_list.AddColumn("Inv");
	perf_list.AddColumn("Signals");
	perf_list.AddColumn("Sum");
	perf_list.AddColumn("Success");
	perf_list.AddColumn("Score");
	perf_list.AddIndex();
	perf_list.AddIndex();
	perf_list.AddIndex();
	perf_list.ColumnWidths("2 2 2 8 3 3 1");
	perf_list.SetLineCy(30);
	perf_list.WhenAction << THISBACK(Data);
	
	perf_parent.Add(perf_tf.TopPos(0, 30).LeftPos(0, 200));
	perf_parent.Add(candles.VSizePos(30).HSizePos());
	
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
	
	if (s.data.GetCount() < 2) return;
	
	bool is_last = slider.GetData() == slider.GetMax();
	slider.MinMax(0, s.data.GetCount()-1);
	if (is_last)
		slider.SetData(slider.GetMax());
	
	matrix.shift = slider.GetData();
	succ_ctrl.shift = matrix.shift;
	
	
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
			
			if (i < 0 || i >= si.data.GetCount())
				continue;
			SpecItemData& d = si.data[i];
			
			int new_value = fabs(d.slow_sum) / s.max_sum * 1000;
			int new_succ_value = fabs(d.succ_sum) / s.max_succ_sum * 1000;
			
			int row = i-s.cur_count;
			/*if (ses.HasOrders(full)) {
				bool sig = sum < 0;
				bool real_sig = ses.GetOrderSig(full);
				list.Set(row, 0, AttrText(s).Paper(real_sig ? sell_paper : buy_paper));
				list.Set(row, 3, AttrText(!sig ? "Buy" : "Sell").Paper(sig == real_sig ? White() : LtRed()));
			}
			else {*/
				list.Set(row, 0, sym);
				list.Set(row, 3, d.slow_sum > 0 ? "Buy" : "Sell");
			//}
			list.Set(row, 1, new_value);
			list.SetDisplay(row, 1, ProgressDisplay2());
			list.Set(row, 2, new_succ_value);
			list.SetDisplay(row, 2, SuccessDisplay());
			list.Set(row, 4, (new_value + new_succ_value) / 20);
		}
		list.SetSortColumn(4, true);
		list.SetCursor(cursor);
		
		matrix_prog.Set(s.actual, s.total);
		bd_prog.Set(s.bdactual, s.bdtotal);
	}
	else if (tab == 1) {
		if (matrix.shift < 0 || matrix.shift >= s.data.GetCount())
			return;
		SpecItem& si = s.data[matrix.shift];
		
		int cursor = 0;
		if (perf_list.IsCursor()) cursor = perf_list.GetCursor();
		int scroll = perf_list.GetScroll();
		
		int lengthi = 0, prio_id = 0, inv = 0;
		if (perf_list.GetCount()) {
			lengthi = max(0, min(s.lengths.GetCount()-1, (int)perf_list.Get(cursor, 7)));
			prio_id = max(0, min(2, (int)perf_list.Get(cursor, 8)));
			inv = max(0, min(1, (int)perf_list.Get(cursor, 9)));
			int tfi = perf_tf.GetIndex();
			int shift = matrix.shift;
			if (tfi)
				shift = dbc.GetTimeIndex(s.tfs[tfi]).Find(SyncTime(s.tfs[tfi], idx0[shift]));
			if (lengthi != candles.lengthi || shift != candles.shift || tfi != candles.tfi || prio_id != candles.prio_id || inv != candles.inv) {
				candles.lengthi = lengthi;
				candles.prio_id = prio_id;
				candles.inv = inv;
				candles.shift = shift;
				candles.tfi = tfi;
				candles.Data();
				candles.Refresh();
			}
		}
		
		int row = 0;
		for(int j = 0; j < s.lengths.GetCount(); j++) {
			for (int prio_id = 0; prio_id < s.prio_count; prio_id++) {
				for (int inv = 0; inv < 2; inv++) {
					const SpecBarData& sbd = s.GetSpecBarData(0, j, prio_id, inv);
					if (matrix.shift < 0 || matrix.shift >= sbd.data.GetCount())
						 continue;
					
					perf_list.Set(row, 0, s.lengths[j]);
					perf_list.Set(row, 1, 1 << prio_id);
					perf_list.Set(row, 2, inv);
					
					const SpecBarDataItem& sbdi = sbd.data[matrix.shift];
					int code = 0;
					int bit = 0;
					for(int k = 0; k < SpecBarDataItem::tf_count; k++) {
						if (sbdi.values[k]) code |= 1 << bit;
						bit++;
						if (sbdi.avvalues[k]) code |= 1 << bit;
						bit++;
						if (sbdi.succ[k]) code |= 1 << bit;
						bit++;
					}
					perf_list.Set(row, 3, code);
					perf_list.SetDisplay(row, 3, CodeDisplay());
					
					ASSERT(s.bd_max_sum != 0 && s.bd_max_succ_sum != 0);
					int new_value = max(0.0, sbdi.slow_sum) * 1000 / s.bd_max_sum;
					int new_succ_value = max(0.0, sbdi.succ_sum) * 1000 / s.bd_max_succ_sum;
					perf_list.Set(row, 4, new_value);
					perf_list.SetDisplay(row, 4, ProgressDisplay2());
					perf_list.Set(row, 5, new_succ_value);
					perf_list.SetDisplay(row, 5, SuccessDisplay());
					perf_list.Set(row, 6, (new_value * 2 + new_succ_value) / 30);
					perf_list.Set(row, 7, j);
					perf_list.Set(row, 8, prio_id);
					perf_list.Set(row, 9, inv);
					
					row++;
				}
			}
		}
		
		perf_list.SetSortColumn(6, true);
		
		perf_list.WhenAction.Clear();
		for(int i = 0; i < perf_list.GetCount(); i++) {
			int lengthi2 = perf_list.Get(i, 7);
			int prio_id2 = perf_list.Get(i, 8);
			int inv2 = perf_list.Get(i, 9);
			if (lengthi2 == lengthi && prio_id2 == prio_id && inv2 == inv) {
				perf_list.SetCursor(i);
				break;
			}
		}
		perf_list.ScrollTo(scroll);
		perf_list.WhenAction << THISBACK(Data);
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
	double subrow = row / 3;
	double subsubrow = subrow * 2 / 4;
	double subsubsubrow = subsubrow / 2;
	double subcol = col / tf_count;
	Font fnt = Arial(subrow * 0.6);
	const double nn_max = 0.025;
	const double bdnn_max = 0.1;
	
	for(int i = 0; i < cur_count; i++) {
		int sym_id_a = si.cur_score.GetKey(i);
		
		String a = s.sym[sym_id_a].Left(3);
		
		int x = 0;
		int y = (1 + i) * row;
		Size a_size = GetTextSize(a, fnt);
		d.DrawText(x + (col - a_size.cx) / 2, y + (subrow - a_size.cy) / 2, a, fnt);
		
		for(int j = 0; j < tf_count; j++) {
			bool b = si.values[sym_id_a * tf_count + j];
			bool ab = si.avvalues[sym_id_a * tf_count + j];
			bool su = si.succ[sym_id_a * tf_count + j];
			
			x = j * subcol;
			y = (1 + i) * row + subrow;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, si.GetColor(b));
			y = (1 + i) * row + subrow + subsubrow;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, si.GetColor(ab));
			y = (1 + i) * row + subrow + subsubrow * 2;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, s.GetSuccessColor(su));
		}
		if (!si.nn.IsEmpty()) {
			double nn = si.nn[sym_id_a];
			x = 0;
			y = (1 + i) * row + subrow + subsubrow * 3;
			if (nn >= 0) {
				int w = min(col / 2.0, nn / nn_max * col / 2.0);
				d.DrawRect(x + col / 2.0 + 1, y, w, subsubrow + 1, SpecItem::PositiveColor);
			} else {
				int w = min(col / 2.0, -nn / nn_max * col / 2.0);
				d.DrawRect(x + col / 2.0 - w + 1, y, w, subsubrow + 1, SpecItem::NegativeColor);
			}
		}
		
		
		x = (1 + cur_count - 1 - i) * col;
		y = 0;
		d.DrawText(x + (col - a_size.cx) / 2, y + (subrow - a_size.cy) / 2, a, fnt);
		for(int j = 0; j < tf_count; j++) {
			bool b = si.values[sym_id_a * tf_count + j];
			bool ab = si.avvalues[sym_id_a * tf_count + j];
			bool su = si.succ[sym_id_a * tf_count + j];
			
			x = (1 + cur_count - 1 - i) * col + j * subcol;
			y = subrow;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, si.GetColor(b));
			y = subrow + subsubrow;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, si.GetColor(ab));
			y = subrow + subsubrow * 2;
			d.DrawRect(x, y, subcol + 1, subsubrow + 1, s.GetSuccessColor(su));
		}
		if (!si.nn.IsEmpty()) {
			double nn = si.nn[sym_id_a];
			x = (1 + cur_count - 1 - i) * col;
			y = subrow + subsubrow * 3;
			if (nn >= 0) {
				int w = min(col / 2.0, nn / nn_max * col / 2.0);
				d.DrawRect(x + col / 2.0 + 1, y, w, subsubrow + 1, SpecItem::PositiveColor);
			} else {
				int w = min(col / 2.0, -nn / nn_max * col / 2.0);
				d.DrawRect(x + col / 2.0 - w + 1, y, w, subsubrow + 1, SpecItem::NegativeColor);
			}
		}
		
		
		for(int j = 0; j < cur_count; j++) {
			if (i == j) continue;
			int sym_id_b = si.cur_score.GetKey(j);
			
			String b = s.sym[sym_id_b].Left(3);
			String ab = a + b + sys.GetPostFix();
			String ba = b + a + sys.GetPostFix();
			String sym;
			bool is_ab = s.sym.Find(ab) != -1;
			bool invert = !is_ab;
			if (is_ab)
				sym = ab;
			else
				sym = ba;
			int sympos = s.sym.Find(sym);
			
			x = (1 + cur_count - 1 - j) * col;
			y = (1 + i) * row;
			
			if (!si.nn.IsEmpty() && !si.bdnn.IsEmpty()) {
				bool a_sig = si.nn[sym_id_a] < 0;
				bool b_sig = si.nn[sym_id_b] < 0;
				double nn = si.nn[sympos];
				double bdnn = si.bdnn[sympos];
				bool ab_nn_sig = nn < 0;
				bool ab_bdnn_sig = bdnn < 0;
				if (invert) {
					ab_nn_sig = !ab_nn_sig;
					ab_bdnn_sig = !ab_bdnn_sig;
				}
				bool is_opportunity = !a_sig && b_sig && !ab_nn_sig && !ab_bdnn_sig && nn != 0.0 && bdnn != 0.0;
				if (is_opportunity)
					d.DrawRect(x, y, col + 1, row + 1, Color(255, 255, 189));
			}
			
			/*int mtsym_id = ses.FindSymbolLeft(sym);
			ASSERT(mtsym_id != -1);
			String mtsym = ses.GetSymbol(mtsym_id);
			bool has_orders = ses.HasOrders(mtsym);
			if (has_orders) {
				bool sig = ses.GetOrderSig(mtsym);
				d.DrawRect(x, y, col + 1, row + 1, s.GetPaper(sig));
			}
			if (invert) {
			 todo
			}
			*/
			
			Size s_size = GetTextSize(ab, fnt);
			d.DrawText(x + (col - s_size.cx) / 2, y + (subrow - s_size.cy) / 2, ab, fnt);
			for(int k = 0; k < tf_count; k++) {
				x = (1 + cur_count - 1 - j) * col + k * subcol;
				y = (1 + i) * row + subrow;
				
				bool b = si.values[sympos * tf_count + k];
				bool ab = si.avvalues[sympos * tf_count + k];
				if (invert) {
					b = !b;
					ab = !ab;
				}
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, si.GetColor(b));
				y = (1 + i) * row + subrow + subsubsubrow;
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, si.GetColor(ab));
				
				y = (1 + i) * row + subrow + subsubrow;
				bool av = si.values[sym_id_a * tf_count + k];
				bool bv = si.values[sym_id_b * tf_count + k];
				
				int sum = 0;
				sum += av ? -1 : +1;
				sum += bv ? +1 : -1;
				
				Color c;
				if (sum > 0)		c = si.GetColor(0);
				else if (sum < 0)	c = si.GetColor(1);
				else				c = si.GetGrayColor();
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, c);
				
				y = (1 + i) * row + subrow + subsubrow + subsubsubrow;
				bool aav = si.avvalues[sym_id_a * tf_count + k];
				bool abv = si.avvalues[sym_id_b * tf_count + k];
				
				sum = 0;
				sum += aav ? -1 : +1;
				sum += abv ? +1 : -1;
				
				if (sum > 0)		c = si.GetColor(0);
				else if (sum < 0)	c = si.GetColor(1);
				else				c = si.GetGrayColor();
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, c);
				
				y = (1 + i) * row + subrow + subsubrow * 2;
				bool su = si.succ[sympos * tf_count + k];
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, s.GetSuccessColor(su));
				
				y = (1 + i) * row + subrow + subsubrow * 2 + subsubsubrow;
				bool as = si.succ[sym_id_a * tf_count + k];
				bool bs = si.succ[sym_id_b * tf_count + k];
				sum = as * 1 + bs * 1;
				c = Blend(s.GetSuccessColor(0), s.GetSuccessColor(1), sum * 255 / 2);
				d.DrawRect(x, y, subcol + 1, subsubsubrow + 1, c);
			}
			
			if (!si.nn.IsEmpty()) {
				x = (1 + cur_count - 1 - j) * col;
				y = (1 + i) * row + subrow + subsubrow * 3;
				double nn = si.nn[sympos];
				if (invert) nn *= -1.0;
				if (nn >= 0) {
					int w = min(col / 2.0, nn / nn_max * col / 2.0);
					d.DrawRect(x + col / 2.0 + 1, y, w, subsubsubrow + 1, SpecItem::PositiveColor);
				} else {
					int w = min(col / 2.0, -nn / nn_max * col / 2.0);
					d.DrawRect(x + col / 2.0 - w + 1, y, w, subsubsubrow + 1, SpecItem::NegativeColor);
				}
			}
			
			if (!si.bdnn.IsEmpty()) {
				x = (1 + cur_count - 1 - j) * col;
				y = (1 + i) * row + subrow + subsubrow * 3 + subsubsubrow;
				double bdnn = si.bdnn[sympos];
				if (invert) bdnn *= -1.0;
				if (bdnn >= 0) {
					int w = min(col / 2.0, bdnn / bdnn_max * col / 2.0);
					d.DrawRect(x + col / 2.0 + 1, y, w, subsubsubrow + 1, SpecItem::PositiveColor);
				} else {
					int w = min(col / 2.0, -bdnn / bdnn_max * col / 2.0);
					d.DrawRect(x + col / 2.0 - w + 1, y, w, subsubsubrow + 1, SpecItem::NegativeColor);
				}
			}
		}
	}
	
	for(int i = 0; i < cur_count; i++) {
		int y = (1 + i) * row;
		int x = (1 + i) * col;
		int y2 = (1 + i) * row + subrow;
		int y3 = (1 + i) * row + subrow + subsubrow;
		int y4 = (1 + i + 1) * row - subsubrow;
		int y5 = (1 + i) * row + subrow + subsubsubrow;
		int y6 = (1 + i) * row + subrow + subsubrow + subsubsubrow;
		int y7 = (1 + i) * row + subrow + subsubrow * 2;
		int y8 = (1 + i) * row + subrow + subsubrow * 2 + subsubsubrow;
		
		for(int j = 0; j < cur_count; j++) {
			if (cur_count - 1 - j == i) continue;
			for(int k = 1; k < tf_count; k++) {
				int x = (1 + j) * col + k * subcol;
				d.DrawLine(x, y2, x, y4, 1, GrayColor());
			}
		}
		
		for(int j = 1; j < tf_count; j++) {
			int x = j * subcol;
			d.DrawLine(x, y2, x, y4, 1, GrayColor());
			x = (1 + i) * col + j * subcol;
			d.DrawLine(x, subrow, x, row - subsubrow, 1, GrayColor());
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
	double col = r.Width() / (double)SpecBarDataItem::tf_count;
	double row = r.Height() / 3.0;
	
	int code = q;
	int bit = 0;
	for(int i = 0; i < SpecBarDataItem::tf_count; i++) {
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
	
	for(int i = 1; i < SpecBarDataItem::tf_count; i++) {
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
	
	int cs_h = h * 0.5;
	int indi_h = h / 6.0;
	
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
    if (IsFin(O) && O >= 0 && O <= indi_h) {
	    d.DrawLine(0, y + O, w, y + O, 1, Black());
    }
    
    
    y += indi_h;
    hi = -DBL_MAX;
    lo = +DBL_MAX;
	for(double d : scores) {
		if (d < lo) lo = d;
		if (d > hi) hi = d;
	}
	diff = hi - lo;
	
	cache.SetCount(0);
	
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O;
        pos = c - (count - i);
        if (pos >= scores.GetCount() || pos < 0) continue;
        
        double score  = scores[pos];
        bool b = score < scoremin;
        
        O = (1 - (score  - lo) / diff) * indi_h;
		
		cache <<
			Point((int)(x+(i+0.5)*div),		(int)(y+O));
		
		int rx = x + i * div + 1;
		d.DrawRect(rx, y, div-2, div-2, b ? Color(28, 127, 255) : Color(28, 212, 0));
    }
    if (!cache.IsEmpty())
		d.DrawPolyline(cache, 1, Green());
    
    O = (1 - (scoremin - lo) / diff) * indi_h;
    if (IsFin(O) && O >= 0 && O <= indi_h) {
	    d.DrawLine(0, y + O, w, y + O, 1, Black());
    }
    
    
    y += indi_h;
    hi = 1;
    lo = -1;
	diff = hi - lo;
	
	cache.SetCount(0);
	
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O;
        pos = c - (count - i);
        if (pos >= invs.GetCount() || pos < 0) continue;
        
        double inv  = invs[pos];
        bool b = inv < 0;
        
        O = (1 - (inv  - lo) / diff) * indi_h;
		
		cache <<
			Point((int)(x+(i+0.5)*div),		(int)(y+O));
		
		int rx = x + i * div + 1;
		d.DrawRect(rx, y, div-2, div-2, b ? Color(28, 127, 255) : Color(28, 212, 0));
    }
    if (!cache.IsEmpty())
		d.DrawPolyline(cache, 1, Green());
    
    O = (1 - (0 - lo) / diff) * indi_h;
    if (IsFin(O) && O >= 0 && O <= indi_h) {
	    d.DrawLine(0, y + O, w, y + O, 1, Black());
    }
}

void CandlestickCtrl::Data() {
	Speculation& s = GetSpeculation();
	const SpecBarData& sbd = s.GetSpecBarData(tfi, lengthi, prio_id, inv);
	const BarDataOscillator& bdo = s.bardataspecosc[tfi];
	if (shift < 0 || shift >= sbd.data.GetCount()) return;
	if (shift < 0 || shift >= bdo.scores.GetCount()) return;
	if (shift < 0 || shift >= bdo.invs.GetCount()) return;
	if (bdo.score_levels.IsEmpty()) return;
	
	scoremin = bdo.score_levels.Top();
	
	int count = min(shift+1, this->count);
	Vector<double> opens, lows, highs, scores, invs;
	Vector<bool> bools;
	opens.SetCount(count);
	lows.SetCount(count);
	highs.SetCount(count);
	specs.SetCount(count);
	bools.SetCount(count);
	scores.SetCount(count);
	invs.SetCount(count);
	for(int i = 0; i < count; i++) {
		int pos = shift+1 - count + i;
		const SpecBarDataItem& sbdi = sbd.data[pos];
		
		opens[i] = sbdi.open;
		lows[i] = sbdi.low;
		highs[i] = sbdi.high;
		specs[i] = sbdi.spec;
		bools[i] = sbdi.signal;
		
		scores[i] = bdo.scores[pos];
		invs[i] = bdo.invs[pos];
	}
	Swap(this->highs, highs);
	Swap(this->lows, lows);
	Swap(this->opens, opens);
	Swap(this->specs, specs);
	Swap(this->bools, bools);
	Swap(this->scores, scores);
	Swap(this->invs, invs);
	
	
}



}
