#ifndef _AsmProto_Generator_h_
#define _AsmProto_Generator_h_



enum {
	MOM_INC,
	MOM_DEC,
	MA_INC,
	MA_DEC,
	AP_INC,
	AP_DEC,
	ARG_COUNT
};

struct AsmData : Moveable<AsmData> {
	double pres = 0;
};


struct Asm {
	static constexpr double low = 0.0;
	static constexpr double high = 2.0;
	static constexpr double step = 0.0001;
	static const int size = (high - low) / step;
	
	AsmData data[size];
	
	AsmData& Get(double d) PARALLEL {int i = (d - low) / step; return data[i];}
	
	
	Asm() {
		Reset();
	}
	void Reset() PARALLEL {
		for(int i = 0; i < size; i++)
			data[i].pres = 0;
	}
};

struct AmpPoint {
	double x, y;
};

struct Generator {
	static const int data_count = 1440*7;
	static const int test_count = 500;
	static const int pattern_count = 6;
	static const int ma_count = 12;
	
	Asm a;
	double price = 1.0;
	double step = 0.0001;
	int iter = 0;
	double data[data_count];
	int descriptors[pattern_count][data_count];
	AmpPoint pattern[pattern_count][32];
	double err = 0.0;
	
	
	// Indicators
	OnlineAverageWindow1<2 << ma_count> ma[ma_count];
	double prev_ma_mean[ma_count];
	double mom_inc = 0.1, mom_dec = 0.1;
	double ma_inc = 0.1, ma_dec = 0.1;
	double ap_inc = 3.0, ap_dec = 3.0;
	
	
	
	Generator() {
		InitMA();
		RandomizePatterns();
	}
	
	void InitMA() {
		for(int i = 0; i < ma_count; i++) {
			prev_ma_mean[i] = 1.0;
			ma[i].SetPeriod(2 << i);
		}
	}
	
	void AddRandomPressure() {
		double low  = price - (1 + Random(10)) * step;
		double high = price + (1 + Random(10)) * step;
		double min = -1 - (int)Random(10);
		double max =  1 + (int)Random(10);
		bool action = Random(2);
		
		if (low < a.low) low = a.low;
		if (low >= a.high) low = a.high - a.step;
		if (high < a.low) high = a.low;
		if (high >= a.high) high = a.high - a.step;
		
		for (double d = low; d < high; d += a.step) {
			double range = high - low;
			double diff = d - low;
			double factor = diff / range;
			if (action) factor = 1.0 - factor;
			double prange = max - min;
			double pres = max - factor * prange;
			
			AsmData& ad = a.Get(d);
			ad.pres += pres;
		}
		
	}

	void RandomizePatterns() {
		/*for(int i = 0; i < pattern_count; i++) {
			int period = 4 << i;
			String s;
			for(int j = 0; j < 32; j++) {
				pattern[i][j].x = Random(period);
				pattern[i][j].y = Random(period);
				s << (Format("pattern[%d][%d].x = %d; ", i, j, pattern[i][j].x));
				s << (Format("pattern[%d][%d].y = %d; ", i, j, pattern[i][j].y));
			}
			LOG(s);
		}*/
		pattern[0][0].x = 0; pattern[0][0].y = 1; pattern[0][1].x = 3; pattern[0][1].y = 3; pattern[0][2].x = 3; pattern[0][2].y = 3; pattern[0][3].x = 2; pattern[0][3].y = 1; pattern[0][4].x = 0; pattern[0][4].y = 0; pattern[0][5].x = 2; pattern[0][5].y = 1; pattern[0][6].x = 2; pattern[0][6].y = 2; pattern[0][7].x = 2; pattern[0][7].y = 3; pattern[0][8].x = 0; pattern[0][8].y = 0; pattern[0][9].x = 3; pattern[0][9].y = 1; pattern[0][10].x = 1; pattern[0][10].y = 1; pattern[0][11].x = 0; pattern[0][11].y = 0; pattern[0][12].x = 3; pattern[0][12].y = 3; pattern[0][13].x = 1; pattern[0][13].y = 0; pattern[0][14].x = 3; pattern[0][14].y = 3; pattern[0][15].x = 2; pattern[0][15].y = 0; pattern[0][16].x = 3; pattern[0][16].y = 2; pattern[0][17].x = 3; pattern[0][17].y = 1; pattern[0][18].x = 2; pattern[0][18].y = 1; pattern[0][19].x = 2; pattern[0][19].y = 3; pattern[0][20].x = 1; pattern[0][20].y = 3; pattern[0][21].x = 2; pattern[0][21].y = 1; pattern[0][22].x = 1; pattern[0][22].y = 2; pattern[0][23].x = 0; pattern[0][23].y = 0; pattern[0][24].x = 2; pattern[0][24].y = 3; pattern[0][25].x = 3; pattern[0][25].y = 2; pattern[0][26].x = 0; pattern[0][26].y = 2; pattern[0][27].x = 0; pattern[0][27].y = 3; pattern[0][28].x = 0; pattern[0][28].y = 2; pattern[0][29].x = 0; pattern[0][29].y = 3; pattern[0][30].x = 3; pattern[0][30].y = 3; pattern[0][31].x = 0; pattern[0][31].y = 3;
		pattern[1][0].x = 7; pattern[1][0].y = 0; pattern[1][1].x = 3; pattern[1][1].y = 2; pattern[1][2].x = 2; pattern[1][2].y = 7; pattern[1][3].x = 7; pattern[1][3].y = 3; pattern[1][4].x = 0; pattern[1][4].y = 4; pattern[1][5].x = 3; pattern[1][5].y = 1; pattern[1][6].x = 1; pattern[1][6].y = 2; pattern[1][7].x = 3; pattern[1][7].y = 0; pattern[1][8].x = 2; pattern[1][8].y = 6; pattern[1][9].x = 3; pattern[1][9].y = 5; pattern[1][10].x = 2; pattern[1][10].y = 7; pattern[1][11].x = 5; pattern[1][11].y = 0; pattern[1][12].x = 1; pattern[1][12].y = 4; pattern[1][13].x = 3; pattern[1][13].y = 0; pattern[1][14].x = 5; pattern[1][14].y = 5; pattern[1][15].x = 4; pattern[1][15].y = 4; pattern[1][16].x = 0; pattern[1][16].y = 6; pattern[1][17].x = 5; pattern[1][17].y = 4; pattern[1][18].x = 4; pattern[1][18].y = 2; pattern[1][19].x = 6; pattern[1][19].y = 1; pattern[1][20].x = 2; pattern[1][20].y = 5; pattern[1][21].x = 7; pattern[1][21].y = 2; pattern[1][22].x = 1; pattern[1][22].y = 1; pattern[1][23].x = 1; pattern[1][23].y = 1; pattern[1][24].x = 0; pattern[1][24].y = 6; pattern[1][25].x = 0; pattern[1][25].y = 1; pattern[1][26].x = 0; pattern[1][26].y = 3; pattern[1][27].x = 3; pattern[1][27].y = 0; pattern[1][28].x = 0; pattern[1][28].y = 4; pattern[1][29].x = 1; pattern[1][29].y = 6; pattern[1][30].x = 1; pattern[1][30].y = 3; pattern[1][31].x = 6; pattern[1][31].y = 1;
		pattern[2][0].x = 12; pattern[2][0].y = 14; pattern[2][1].x = 9; pattern[2][1].y = 8; pattern[2][2].x = 14; pattern[2][2].y = 7; pattern[2][3].x = 0; pattern[2][3].y = 12; pattern[2][4].x = 5; pattern[2][4].y = 6; pattern[2][5].x = 9; pattern[2][5].y = 9; pattern[2][6].x = 2; pattern[2][6].y = 11; pattern[2][7].x = 9; pattern[2][7].y = 8; pattern[2][8].x = 7; pattern[2][8].y = 11; pattern[2][9].x = 13; pattern[2][9].y = 3; pattern[2][10].x = 10; pattern[2][10].y = 2; pattern[2][11].x = 2; pattern[2][11].y = 2; pattern[2][12].x = 0; pattern[2][12].y = 9; pattern[2][13].x = 3; pattern[2][13].y = 4; pattern[2][14].x = 11; pattern[2][14].y = 10; pattern[2][15].x = 8; pattern[2][15].y = 12; pattern[2][16].x = 5; pattern[2][16].y = 3; pattern[2][17].x = 13; pattern[2][17].y = 5; pattern[2][18].x = 1; pattern[2][18].y = 3; pattern[2][19].x = 6; pattern[2][19].y = 3; pattern[2][20].x = 11; pattern[2][20].y = 14; pattern[2][21].x = 7; pattern[2][21].y = 2; pattern[2][22].x = 12; pattern[2][22].y = 5; pattern[2][23].x = 3; pattern[2][23].y = 3; pattern[2][24].x = 14; pattern[2][24].y = 15; pattern[2][25].x = 2; pattern[2][25].y = 6; pattern[2][26].x = 3; pattern[2][26].y = 3; pattern[2][27].x = 6; pattern[2][27].y = 8; pattern[2][28].x = 15; pattern[2][28].y = 6; pattern[2][29].x = 7; pattern[2][29].y = 6; pattern[2][30].x = 9; pattern[2][30].y = 9; pattern[2][31].x = 0; pattern[2][31].y = 13;
		pattern[3][0].x = 7; pattern[3][0].y = 10; pattern[3][1].x = 30; pattern[3][1].y = 23; pattern[3][2].x = 6; pattern[3][2].y = 26; pattern[3][3].x = 24; pattern[3][3].y = 6; pattern[3][4].x = 23; pattern[3][4].y = 4; pattern[3][5].x = 0; pattern[3][5].y = 22; pattern[3][6].x = 12; pattern[3][6].y = 14; pattern[3][7].x = 16; pattern[3][7].y = 1; pattern[3][8].x = 16; pattern[3][8].y = 23; pattern[3][9].x = 23; pattern[3][9].y = 13; pattern[3][10].x = 20; pattern[3][10].y = 18; pattern[3][11].x = 20; pattern[3][11].y = 28; pattern[3][12].x = 18; pattern[3][12].y = 24; pattern[3][13].x = 20; pattern[3][13].y = 24; pattern[3][14].x = 14; pattern[3][14].y = 20; pattern[3][15].x = 9; pattern[3][15].y = 24; pattern[3][16].x = 24; pattern[3][16].y = 31; pattern[3][17].x = 29; pattern[3][17].y = 26; pattern[3][18].x = 4; pattern[3][18].y = 16; pattern[3][19].x = 8; pattern[3][19].y = 15; pattern[3][20].x = 21; pattern[3][20].y = 31; pattern[3][21].x = 28; pattern[3][21].y = 18; pattern[3][22].x = 29; pattern[3][22].y = 25; pattern[3][23].x = 31; pattern[3][23].y = 20; pattern[3][24].x = 26; pattern[3][24].y = 13; pattern[3][25].x = 20; pattern[3][25].y = 30; pattern[3][26].x = 8; pattern[3][26].y = 10; pattern[3][27].x = 12; pattern[3][27].y = 28; pattern[3][28].x = 26; pattern[3][28].y = 18; pattern[3][29].x = 25; pattern[3][29].y = 31; pattern[3][30].x = 12; pattern[3][30].y = 21; pattern[3][31].x = 21; pattern[3][31].y = 25;
		pattern[4][0].x = 30; pattern[4][0].y = 37; pattern[4][1].x = 29; pattern[4][1].y = 30; pattern[4][2].x = 8; pattern[4][2].y = 20; pattern[4][3].x = 20; pattern[4][3].y = 23; pattern[4][4].x = 49; pattern[4][4].y = 8; pattern[4][5].x = 47; pattern[4][5].y = 37; pattern[4][6].x = 26; pattern[4][6].y = 55; pattern[4][7].x = 28; pattern[4][7].y = 62; pattern[4][8].x = 30; pattern[4][8].y = 47; pattern[4][9].x = 54; pattern[4][9].y = 48; pattern[4][10].x = 33; pattern[4][10].y = 6; pattern[4][11].x = 0; pattern[4][11].y = 11; pattern[4][12].x = 26; pattern[4][12].y = 32; pattern[4][13].x = 26; pattern[4][13].y = 54; pattern[4][14].x = 37; pattern[4][14].y = 25; pattern[4][15].x = 43; pattern[4][15].y = 18; pattern[4][16].x = 27; pattern[4][16].y = 12; pattern[4][17].x = 44; pattern[4][17].y = 54; pattern[4][18].x = 4; pattern[4][18].y = 24; pattern[4][19].x = 7; pattern[4][19].y = 58; pattern[4][20].x = 13; pattern[4][20].y = 39; pattern[4][21].x = 8; pattern[4][21].y = 51; pattern[4][22].x = 6; pattern[4][22].y = 1; pattern[4][23].x = 22; pattern[4][23].y = 22; pattern[4][24].x = 56; pattern[4][24].y = 45; pattern[4][25].x = 24; pattern[4][25].y = 40; pattern[4][26].x = 34; pattern[4][26].y = 9; pattern[4][27].x = 45; pattern[4][27].y = 41; pattern[4][28].x = 8; pattern[4][28].y = 43; pattern[4][29].x = 29; pattern[4][29].y = 8; pattern[4][30].x = 41; pattern[4][30].y = 33; pattern[4][31].x = 17; pattern[4][31].y = 49;
		pattern[5][0].x = 34; pattern[5][0].y = 61; pattern[5][1].x = 87; pattern[5][1].y = 58; pattern[5][2].x = 77; pattern[5][2].y = 70; pattern[5][3].x = 112; pattern[5][3].y = 34; pattern[5][4].x = 71; pattern[5][4].y = 46; pattern[5][5].x = 124; pattern[5][5].y = 24; pattern[5][6].x = 29; pattern[5][6].y = 39; pattern[5][7].x = 12; pattern[5][7].y = 74; pattern[5][8].x = 6; pattern[5][8].y = 16; pattern[5][9].x = 109; pattern[5][9].y = 103; pattern[5][10].x = 29; pattern[5][10].y = 49; pattern[5][11].x = 52; pattern[5][11].y = 67; pattern[5][12].x = 53; pattern[5][12].y = 65; pattern[5][13].x = 111; pattern[5][13].y = 25; pattern[5][14].x = 80; pattern[5][14].y = 38; pattern[5][15].x = 42; pattern[5][15].y = 51; pattern[5][16].x = 111; pattern[5][16].y = 47; pattern[5][17].x = 96; pattern[5][17].y = 72; pattern[5][18].x = 103; pattern[5][18].y = 8; pattern[5][19].x = 55; pattern[5][19].y = 1; pattern[5][20].x = 82; pattern[5][20].y = 83; pattern[5][21].x = 100; pattern[5][21].y = 25; pattern[5][22].x = 114; pattern[5][22].y = 5; pattern[5][23].x = 19; pattern[5][23].y = 125; pattern[5][24].x = 59; pattern[5][24].y = 81; pattern[5][25].x = 44; pattern[5][25].y = 49; pattern[5][26].x = 83; pattern[5][26].y = 51; pattern[5][27].x = 87; pattern[5][27].y = 86; pattern[5][28].x = 119; pattern[5][28].y = 56; pattern[5][29].x = 17; pattern[5][29].y = 106; pattern[5][30].x = 104; pattern[5][30].y = 92; pattern[5][31].x = 89; pattern[5][31].y = 2;
		STATIC_ASSERT(pattern_count == 6);
}
	
	void AddMomentumPressure() PARALLEL {
		for(int i = 0; i < ma_count; i++) {
			int period = ma[i].GetPeriod();
			double prev = ma[i].Top();
			double cur = price;
			double diff = cur - prev;
			if (diff == 0.0) continue;
			
			int abs_diff_step = diff / step;
			if (abs_diff_step < 0) abs_diff_step = -abs_diff_step;
			int steps = abs_diff_step * period;
			double step = this->step * (diff < 0 ? -1.0 : +1.0);
			double d0 = price;
			double d1 = price;
			for (int j = 0; j < steps; j++) {
				AsmData& ad0 = a.Get(d0);
				ad0.pres -= mom_inc * (1.0 / period);
				d0 += step;
				if (d0 < a.low) break;
				if (d0 >= a.high) break;
				
				AsmData& ad1 = a.Get(d1);
				ad1.pres += mom_dec * (1.0 / period);
				d1 -= step;
				if (d1 < a.low) break;
				if (d1 >= a.high) break;
			}
		}
	}
	
	void AddMaPressure() PARALLEL {
		for(int i = 0; i < ma_count; i++) {
			int period = ma[i].GetPeriod();
			double prev = prev_ma_mean[i];
			double cur = ma[i].GetMean();
			double diff = cur - prev;
			if (diff == 0.0) continue;
			
			int abs_diff_step = diff / step;
			if (abs_diff_step < 0) abs_diff_step = -abs_diff_step;
			int steps = abs_diff_step * period;
			double step = this->step * (diff < 0 ? -1.0 : +1.0);
			double d0 = price;
			double d1 = price;
			for (int j = 0; j < steps; j++) {
				AsmData& ad0 = a.Get(d0);
				ad0.pres -= ma_inc * (1.0 / period);
				d0 += step;
				if (d0 < a.low) break;
				if (d0 >= a.high) break;
				
				AsmData& ad1 = a.Get(d1);
				ad1.pres += ma_dec * (1.0 / period);
				d1 -= step;
				if (d1 < a.low) break;
				if (d1 >= a.high) break;
			}
		}
	}
	
	void AddAntiPatternPressure() PARALLEL {
		int last_pos = iter - 1;
		int32 last[pattern_count];
		int closest[pattern_count];
		int min_dist[pattern_count];
		for(int i = 0; i < pattern_count; i++) {
			last[i] = descriptors[i][iter - 1];
			closest[i] = -1;
			min_dist[i] = INT_MAX;
		}
		for(int i = 0; i < last_pos; i++) {
			for(int j = 0; j < pattern_count; j++) {
				int32 cur = descriptors[j][i];
				#ifdef flagFORCE_COMPAT_AMP
				int dist = PopCount32(last[j] ^ cur);
				#else
				int dist = 0;
				int distbits = last[j] ^ cur;
				for(int i = 0; i < 32; i++)
					if (distbits & (1 << i))
						dist++;
				#endif
				if (dist < min_dist[j]) {
					min_dist[j] = dist;
					closest[j] = i;
				}
			}
		}
		for(int i = 0; i < pattern_count; i++) {
			int period = 4 << i;
			int prev_pos = closest[i];
			int cur_pos = prev_pos + period;
			if (cur_pos > last_pos) cur_pos = last_pos;
			if (prev_pos < 0) continue;
			
			double cur = data[cur_pos];
			double prev = data[prev_pos];
			double diff = cur - prev;
			if (diff == 0.0) continue;
			
			int abs_diff_step = diff / step;
			if (abs_diff_step < 0) abs_diff_step = -abs_diff_step;
			int steps = abs_diff_step * period;
			double step = this->step * (diff < 0 ? -1.0 : +1.0);
			double d0 = price;
			double d1 = price;
			for (int j = 0; j < steps; j++) {
				AsmData& ad0 = a.Get(d0);
				ad0.pres -= ap_inc * (1.0 / period);
				d0 += step;
				if (d0 < a.low) break;
				if (d0 >= a.high) break;
				
				AsmData& ad1 = a.Get(d1);
				ad1.pres += ap_dec * (1.0 / period);
				d1 -= step;
				if (d1 < a.low) break;
				if (d1 >= a.high) break;
			}
		}
	}
	
	void ResetGenerate() {
		a.Reset();
		price = 1.0;
		err = 0;
		iter = 0;
		
	}
	
	void GenerateData(double* real_data=NULL, int real_data_count=0) PARALLEL {
		int count = data_count;
		bool add_random = real_data == NULL;
		int max_random_count = count - test_count;
		
		
		/*if (add_random) {
			for(int i = 0; i < 3; i++)
				AddRandomPressure();
		}*/
		
		
		int call_iters = 0;
		for(; iter < count && call_iters < 30000; iter++, call_iters++) {
			
			if (iter > 0) {
				ApplyPressureChanges();
				
				/*if (add_random && iter < max_random_count)
					if (Random(10) == 0)
						AddRandomPressure();*/
				
				AddMomentumPressure();
				AddMaPressure();
				AddAntiPatternPressure();
			}
			
			double max_pres = -DBL_MAX;
			double prev_price = this->price;
			for (int i = -3; i <= 3; i++) {
				double price = prev_price + i * step;
				double pres = a.Get(price).pres;
				if (pres > max_pres) {
					this->price = price;
					max_pres = pres;
				}
				/*if (iter == 500) {
					LOG(i << "\t" << price << "\t" << pres);
				}*/
			}
			
			// Price is known
			
			if (iter < real_data_count) {
				if (iter > 0 && real_data) {
					double prev = real_data[iter - 1];
					double cur = real_data[iter];
					double real_diff = cur - prev;
					double gen_diff = price - prev_price;
					double abs_err = real_diff - gen_diff;
					if (abs_err < 0) abs_err = -abs_err;
					err += abs_err;
				}
				
				if (real_data) {
					price = real_data[iter];
				}
			} else {
				
			}
			
			
			
			for(int i = 0; i < ma_count; i++) {
				double mean = ma[i].GetMean();
				if (mean == 0.0) mean = 1.0;
				prev_ma_mean[i] = mean;
				ma[i].Add(price);
			}
			data[iter] = price;
			
			for(int i = 0; i < pattern_count; i++)
				descriptors[i][iter] = GetDescriptor(iter, i);
			
			
		}
		
	}
	
	int32 GetDescriptor(int pos, int pattern_id) PARALLEL {
		int period = 4 << pattern_id;
		int data_begin = pos - period + 1;
		int32 out = 0;
		for(int i = 0; i < 32; i++) {
			const AmpPoint& pt = pattern[pattern_id][i];
			int pos_a = data_begin + pt.x;
			int pos_b = data_begin + pt.y;
			if (pos_a < 0) pos_a = 0;
			if (pos_b < 0) pos_b = 0;
			AMPASSERT(pos_a <= pos && pos_b <= pos);
			double a = data[pos_a];
			double b = data[pos_b];
			bool value = a < b;
			if (value)		out |= 1 << i;
		}
		return out;
	}
	
	void ApplyPressureChanges() PARALLEL {
		if (iter > 0) {
			a.Get(price).pres += -0.4;
			a.Get(price + step).pres += -0.2;
			a.Get(price - step).pres += -0.2;
		}
	}
	
	
};





#endif
