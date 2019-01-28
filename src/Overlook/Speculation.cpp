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
	
	data.SetCount(bars);
	
	for(int i = counted; i < bars; i++) {
		SpecItem& si = data[i];
		ProcessItem(i, si);
	}
	
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
			
			int shift_pos = shift_posv[j];
			
			ConstLabelSignal& sig = c.GetLabelSignal(0, 0, 0);
			bool b = sig.signal.Get(shift_pos);
			si.values[row] = b;
			
			double sum = 0;
			for(int k = 0; k < 21; k++) {
				bool b = sig.signal.Get(max(0, shift_pos - k));
				if (b) sum += 1.0;
			}
			sum /= 21;
			b = sum > 0.5;
			si.avvalues[row] = b;
			
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
				if (diff >= spread) {
					succ = true;
					break;
				}
			}
			si.succ[row] = succ;
			
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










SpeculationCtrl::SpeculationCtrl() {
	Add(tabs.VSizePos(0, 30).HSizePos());
	Add(slider.BottomPos(0, 30).HSizePos(0, 200));
	Add(time_lbl.BottomPos(0, 30).RightPos(0, 200));
	
	tabs.Add(matrix_tab.SizePos(), "Matrix");
	
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
	
	slider.MinMax(0, 1);
	slider.SetData(0);
	slider <<= THISBACK(Data);
}

void SpeculationCtrl::Start() {
	Data();
}

void SpeculationCtrl::Data() {
	Speculation& s = GetSpeculation();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(0);
	
	if (s.data.IsEmpty()) return;
	
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
			
			SpecItemData& d = si.data[i * s.startv.GetCount() + si.data_i];
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

}
