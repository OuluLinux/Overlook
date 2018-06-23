#include "Overlook.h"


namespace Overlook {





void ScalperSymbol::Init() {
	confs.SetMax(10000);
	
}

void ScalperSymbol::Start() {
	confs.SetNoDuplicates(false);
	
	TimeStop ts;
	
	while (ts.Elapsed() < 5000) {
		Evolve();
	}
	
	if (confs.GetCount()) {
		signal.Zero();
		
		for(int i = 0; i < confs.GetCount(); i++) {
			ScalperConf& s = confs[i];
			if (s.profit > 0.0)
				Evaluate(s, true);
		}
		
		int sig = signal.Top();
		if (type) sig *= -1;
		GetSystem().SetSignal(symbol, sig);
	}
}

void ScalperSymbol::Evolve() {
	
	double prob = Randomf();
	
	// Generate totally random
	if (prob < 0.3) {
		ScalperConf sc;
		Randomize(sc);
		Evaluate(sc, false);
		confs.Add(conf_counter++, sc);
	}
	// Evolve locally
	else /*if (prob < 0.9)*/ {
		ScalperConf sc;
		Evolve(sc);
		Evaluate(sc, false);
		confs.Add(conf_counter++, sc);
	}
	// Evolve globally (not thread safe)
	/*else {
		ScalperConf sc;
		s->symbols[Random(s->symbols.GetCount())].Evolve(sc);
		Evaluate(sc, false);
		confs.Add(conf_counter++, sc);
	}*/
	
}

void ScalperSymbol::Randomize(ScalperConf& sc) {
	int start_size = 1 + Random(9);
	int sust_size = 1 + Random(9);
	
	sc.start_list.SetCount(start_size);
	for(int i = 0; i < sc.start_list.GetCount(); i++) {
		Vector<MatcherItem>& v = sc.start_list[i];
		int or_size = 5 + Random(10);
		v.SetCount(or_size);
		for(int j = 0; j < v.GetCount(); j++) {
			MatcherItem& mi = v[j];
			mi.cache = Random(s->cache.GetCount());
			mi.event = Random(EVENT_COUNT);
			mi.key = mi.cache * EVENT_COUNT + mi.event;
		}
	}
	
	
	sc.sust_list.SetCount(start_size);
	for(int i = 0; i < sc.sust_list.GetCount(); i++) {
		Vector<MatcherItem>& v = sc.sust_list[i];
		int or_size = 5 + Random(10);
		v.SetCount(or_size);
		for(int j = 0; j < v.GetCount(); j++) {
			MatcherItem& mi = v[j];
			mi.cache = Random(s->cache.GetCount());
			mi.event = Random(EVENT_COUNT);
			mi.key = mi.cache * EVENT_COUNT + mi.event;
		}
	}
	
}

void ScalperSymbol::Evolve(ScalperConf& sc) {
	if (confs.GetCount() < 4) {Randomize(sc); return;}
	
	int start_size = 1 + Random(9);
	int sust_size = 1 + Random(9);
	
	sc.start_list.SetCount(start_size);
	sc.sust_list.SetCount(start_size);
	
	
	// Best1Exp
	int r0 = 1 + Random(confs.GetCount()-1);
	int r1, r2;
	do {r1 = 1 + Random(confs.GetCount()-1);} while (r1 == r0);
	do {r2 = 1 + Random(confs.GetCount()-1);} while (r1 == r0 || r2 == r1);
	const ScalperConf& best = confs[0];
	const ScalperConf& s0 = confs[r0];
	const ScalperConf& s1 = confs[r1];
	const ScalperConf& s2 = confs[r2];
	
	sc = s0;
	
	int best_start_n = Random(best.start_list.GetCount());
	int sc_start_n = Random(sc.start_list.GetCount());
	int s1_start_n = Random(s1.start_list.GetCount());
	int s2_start_n = Random(s2.start_list.GetCount());
	for(int i = 0; i < sc.start_list.GetCount() && Randomf() < 0.9; i++) {
		Vector<MatcherItem>& v = sc.start_list[sc_start_n];
		switch (Random(4)) {
			case 0:	v <<= best.start_list[best_start_n]; break;
			case 1:	v <<= s1.start_list[s1_start_n]; break;
			case 2:	v <<= s2.start_list[s2_start_n]; break;
			case 3:
				int or_size = 5 + Random(10);
				v.SetCount(or_size);
				for(int j = 0; j < v.GetCount(); j++) {
					MatcherItem& mi = v[j];
					mi.cache = Random(s->cache.GetCount());
					mi.event = Random(EVENT_COUNT);
					mi.key = mi.cache * EVENT_COUNT + mi.event;
				}
				break;
		}
		
		best_start_n = (best_start_n + 1) % best.start_list.GetCount();
		sc_start_n = (sc_start_n + 1) % sc.start_list.GetCount();
		s1_start_n = (s1_start_n + 1) % s1.start_list.GetCount();
		s2_start_n = (s2_start_n + 1) % s2.start_list.GetCount();
	}
	
	int best_sust_n = Random(best.sust_list.GetCount());
	int sc_sust_n = Random(sc.sust_list.GetCount());
	int s1_sust_n = Random(s1.sust_list.GetCount());
	int s2_sust_n = Random(s2.sust_list.GetCount());
	for(int i = 0; i < sc.sust_list.GetCount() && Randomf() < 0.9; i++) {
		#if 0
		switch (Random(3)) {
			case 0:	sc.sust_list[sc_sust_n] <<= best.sust_list[best_sust_n]; break;
			case 1:	sc.sust_list[sc_sust_n] <<= s1.sust_list[s1_sust_n]; break;
			case 2:	sc.sust_list[sc_sust_n] <<= s2.sust_list[s2_sust_n]; break;
		}
		#else
		int r = Random(3);
		auto& dst = sc.sust_list[sc_sust_n];
		const auto& src = r == 0 ? best.sust_list[best_sust_n] : (r == 1 ? s1.sust_list[s1_sust_n] : s2.sust_list[s2_sust_n]);
		int dst_n = Random(dst.GetCount());
		int src_n = Random(src.GetCount());
		for(int j = 0; j < dst.GetCount() && Randomf() < 0.9; j++) {
			dst[dst_n] = src[src_n];
			dst_n = (dst_n + 1) % dst.GetCount();
			src_n = (src_n + 1) % src.GetCount();
		}
		#endif
		
		best_sust_n = (best_sust_n + 1) % best.sust_list.GetCount();
		sc_sust_n = (sc_sust_n + 1) % sc.sust_list.GetCount();
		s1_sust_n = (s1_sust_n + 1) % s1.sust_list.GetCount();
		s2_sust_n = (s2_sust_n + 1) % s2.sust_list.GetCount();
	}
	
}

void ScalperSymbol::Evaluate(ScalperConf& sc, bool write_signal) {
	DataBridge& db = *s->dbs.Get(symbol);
	ConstBuffer& open_buf = db.GetBuffer(0);
	int bars = open_buf.GetCount();
	double point = db.GetPoint();
	double spread = point * 3;
	
	
	
	// Start
	start_cache.matcher_and.SetCount(bars);
	start_cache.matcher_or.SetCount(bars);
	ASSERT(bars > 0);
	
	start_cache.matcher_and.One();
	
	for(int k = 0; k < sc.start_list.GetCount(); k++) {
		const Vector<MatcherItem>& sublist = sc.start_list[k];
		
		start_cache.matcher_or.Zero();
		
		for(int k = 0; k < sublist.GetCount(); k++) {
			const MatcherItem& mi = sublist[k];
			const LabelSource& c = s->cache[mi.cache];
			const VectorBool& vb = c.eventdata[mi.event];
			ASSERT(!vb.IsEmpty());
			
			start_cache.matcher_or.Or(vb);
			bars = min(vb.GetCount(), bars);
		}
		
		start_cache.matcher_and.And(start_cache.matcher_or);
	}
	
	
	// Sustain
	sust_cache.matcher_and.SetCount(bars);
	sust_cache.matcher_or.SetCount(bars);
	ASSERT(bars > 0);
	
	sust_cache.matcher_and.One();
	
	for(int k = 0; k < sc.sust_list.GetCount(); k++) {
		const Vector<MatcherItem>& sublist = sc.sust_list[k];
		
		sust_cache.matcher_or.Zero();
		
		for(int k = 0; k < sublist.GetCount(); k++) {
			const MatcherItem& mi = sublist[k];
			const LabelSource& c = s->cache[mi.cache];
			const VectorBool& vb = c.eventdata[mi.event];
			ASSERT(!vb.IsEmpty());
			
			sust_cache.matcher_or.Or(vb);
			bars = min(vb.GetCount(), bars);
		}
		
		sust_cache.matcher_and.And(sust_cache.matcher_or);
	}
	
	
	// Start requires sustain
	start_cache.matcher_and.And(sust_cache.matcher_and);
	
	
	signal.SetCount(bars);
	
	bool enabled = false;
	double open, profit = 0;
	int count = 0;
	int max_neg = 1, neg_count = 0;
	ASSERT(bars > 0);
	for(int i = 100; i < bars; i++) {
		if (!enabled && (i % 64) == 0 && *(start_cache.matcher_and.Begin() + (i / 64)) == 0) {
			i += 63;
			continue;
		}
		
		if (!enabled) {
			if (start_cache.matcher_and.Get(i)) {
				enabled = true;
				open = open_buf.Get(i);
			}
		} else {
			double o0 = open_buf.Get(i);
			double o1 = open_buf.Get(i-1);

			double diff = o0 - open;
			if (this->type) diff *= -1;
			diff -= spread;

			bool prev_type = o0 < o1;
			if (prev_type != this->type && o0 != o1) {
				neg_count++;
			}
			if (neg_count >= max_neg || diff > 0.0) {
				profit += diff / point;
				enabled = false;
				neg_count = 0;
				count++;
			}
		}
		
		if (write_signal && enabled)
			signal.Set(i, enabled);
	}
	
	
	if (count > 0)	sc.profit = profit * 10;
	else			sc.profit = INT_MIN;
	
	sc.test_profit = 0;
}






















Scalper::Scalper() {
	System& sys = GetSystem();
	sym_ids.Add(sys.FindSymbol("EURUSD"));
	sym_ids.Add(sys.FindSymbol("GBPUSD"));
	sym_ids.Add(sys.FindSymbol("USDCHF"));
	sym_ids.Add(sys.FindSymbol("USDJPY"));
	
	sym_ids.Add(sys.FindSymbol("USDCAD"));
	sym_ids.Add(sys.FindSymbol("AUDUSD"));
	sym_ids.Add(sys.FindSymbol("NZDUSD"));
	sym_ids.Add(sys.FindSymbol("EURCHF"));
	
	sym_ids.Add(sys.FindSymbol("EURGBP"));
	sym_ids.Add(sys.FindSymbol("EURJPY"));
	
	
	tf_ids.Add(1);
	
	Add<SimpleHurstWindow>(3);
	Add<SimpleHurstWindow>(6);
	for(int i = 1; i <= 7; i++)
		for(int method = 0; method < 2; method++)
			Add<MovingAverage>(2 << i, 0, method);
	Add<MovingAverageConvergenceDivergence>(12, 26, 9);
	Add<MovingAverageConvergenceDivergence>(24, 52, 9);
	Add<MovingAverageConvergenceDivergence>(48, 104, 9);
	for(int i = 10; i <= 30; i += 10)
		for(int j = 10; j <= 30; j += 10)
			Add<BollingerBands>(i, 0, j);
	Add<ParabolicSAR>(20, 10);
	Add<ParabolicSAR>(40, 10);
	Add<ParabolicSAR>(20, 20);
	Add<ParabolicSAR>(40, 20);
	for(int i = 3; i < 6; i++) {
		int period = 2 << i;
		Add<BullsPower>(period);
		Add<BearsPower>(period);
		Add<CommodityChannelIndex>(period);
		Add<DeMarker>(period);
		Add<Momentum>(period);
		Add<RelativeStrengthIndex>(period);
		Add<RelativeVigorIndex>(period);
		Add<StochasticOscillator>(period);
		Add<WilliamsPercentRange>(period);
		Add<MoneyFlowIndex>(period);
	}
	Add<AcceleratorOscillator>(0);
	Add<AwesomeOscillator>(0);
	for(int i = 2; i < 6; i++) {
		int period = 2 << i;
		Add<FractalOsc>(period);
		Add<SupportResistanceOscillator>(period);
		Add<Psychological>(period);
		Add<VolatilityAverage>(period);
		Add<ChannelOscillator>(period);
		Add<ScissorChannelOscillator>(period);
	}
	Add<Anomaly>(0);
	for(int i = 0; i < 6; i++) {
		int period = 1 << i;
		Add<VariantDifference>(period);
	}
	
	LoadThis();
	
	Thread::Start(THISBACK(Process));
}

Scalper::~Scalper() {
	running = false;
	while (!stopped) Sleep(100);
}

void Scalper::Process() {
	System& sys = GetSystem();
	running = true;
	stopped = false;
	
	FillInputBooleans();
	
	if (symbols.IsEmpty()) {
		
		for(int i = 0; i < sym_ids.GetCount(); i++) {
			int sym = sym_ids[i];
			
			ScalperSymbol& lng = symbols.Add(sym);
			lng.type = false;
			lng.symbol = sym;
			
			ScalperSymbol& shr = symbols.Add(-sym-1);
			shr.type = true;
			shr.symbol = sym;
		}
		
		RefreshSymbolPointer();
		
		CoWork co;
		co.SetPoolSize(GetUsedCpuCores());
		for(int i = 0; i < symbols.GetCount(); i++) {
			co & symbols[i].InitCb();
		}
		co.Finish();
		
		
		if (IsRunning())
			StoreThis();
	}
	
	
	TimeStop save_ts;
	while (IsRunning()) {
		Time now = GetUtcTime();
		int wday = DayOfWeek(now);
		bool update = wday > 0 && wday < 6;
		
		if (update) FillInputBooleans();
		
		TimeStop ts;
		
		CoWork co;
		co.SetPoolSize(GetUsedCpuCores());
		for(int i = 0; i < symbols.GetCount(); i++) {
			co & symbols[i].StartCb();
		}
		co.Finish();
		
		if (update) sys.RefreshReal();
		
		/*int sec = ts.Elapsed() / 1000;
		for(int i = sec; i < 10 && IsRunning(); i++)
			Sleep(1000);*/
		
		
		if (save_ts.Elapsed() > 5*60*1000) {
			save_ts.Reset();
			StoreThis();
		}
	}
	
	StoreThis();
	
	stopped = true;
}

void Scalper::FillInputBooleans() {
	System& sys = GetSystem();
	
	if (work_queue.IsEmpty()) {
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	}
	
	int row = 0;
	
	for(int i = 0; i < work_queue.GetCount() && IsRunning(); i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
		
		Core& c = *ci.core;
		
		if (c.GetFactory() == 0) {
			DataBridge& db = dynamic_cast<DataBridge&>(c);
			dbs.GetAdd(db.GetSymbol()) = &db;
		}
		
		for(int j = 0; j < c.GetLabelCount(); j++) {
			ConstLabel& l = c.GetLabel(j);
			
			for(int k = 0; k < l.buffers.GetCount(); k++) {
				ConstLabelSignal& src = l.buffers[k];
				
				if (row >= cache.GetCount())
					cache.SetCount(row+1);
				
				LabelSource& ls = cache[row++];
				
				if (ls.factory == -1) {
					ls.factory = c.GetFactory();
					ls.label = j;
					ls.buffer = k;
					ls.data = src;
					ls.title = sys.GetSymbol(c.GetSymbol()) + " "
						+ sys.GetCoreFactories()[ls.factory].a + " #" + IntStr(k) + " ";
					for(int l = 0; l < ci.args.GetCount(); l++) {
						if (l) ls.title += ",";
						ls.title += " " + IntStr(ci.args[l]);
					}
				} else {
					// Extend data
					int l = ls.data.signal.GetCount();
					ls.data.signal.SetCount(src.signal.GetCount());
					ls.data.enabled.SetCount(src.enabled.GetCount());
					for (; l < src.signal.GetCount(); l++) {
						ls.data.signal.Set(l, src.signal.Get(l));
						ls.data.enabled.Set(l, src.enabled.Get(l));
					}
				}
			}
		}
		
		
		//if (c.GetFactory() != 0)
		//	ci.core.Clear();
		
		ReleaseLog("Scalper::FillInputBooleans " + IntStr(i) + "/" + IntStr(work_queue.GetCount()) + "  " + IntStr(i * 100 / work_queue.GetCount()) + "%");
	}
	
	for(int i = 0; i < cache.GetCount(); i++) {
		UpdateEventVectors(cache[i]);
	}
}









ScalperCtrl::ScalperCtrl() {
	
	Add(symbollist.LeftPos(0,200).VSizePos());
	Add(conflist.LeftPos(200,200).VSizePos());
	Add(tabs.HSizePos(400).VSizePos());
	
	tabs.Add(sustsplit);
	tabs.Add(sustsplit, "Confs");
	tabs.WhenSet << THISBACK(Data);
	
	symbollist.AddColumn("Symbol");
	symbollist.AddColumn("Type");
	symbollist << THISBACK(Data);
	
	conflist.AddColumn("Conf");
	conflist.AddColumn("Profit");
	conflist << THISBACK(Data);
	
	sustsplit << startlist << sustlist;
	
	startlist.AddColumn("Or-list");
	startlist.AddColumn("Cache");
	startlist.AddColumn("Event");
	sustlist.AddColumn("Or-list");
	sustlist.AddColumn("Cache");
	sustlist.AddColumn("Event");
	
	
}

void ScalperCtrl::Data() {
	System& sys = GetSystem();
	Scalper& a = GetScalper();
	
	int tab = tabs.Get();
	if (tab < 4) {
		
		for(int i = 0; i < a.symbols.GetCount(); i++) {
			const ScalperSymbol& as = a.symbols[i];
			int s = a.symbols.GetKey(i);
			if (s < 0) s = -s-1;
			symbollist.Set(i, 0, sys.GetSymbol(s));
			symbollist.Set(i, 1, as.type ? "Short" : "Long");
		}
		
		int symcursor = symbollist.GetCursor();
		
		if (symcursor >= 0 & symcursor < a.symbols.GetCount()) {
			const ScalperSymbol& as = a.symbols[symcursor];
			
			for(int i = 0; i < as.confs.GetCount(); i++) {
				const ScalperConf& am = as.confs[i];
				
				conflist.Set(i, 0, i);
				conflist.Set(i, 1, am.profit);
			}
			conflist.SetCount(as.confs.GetCount());
			
			int cursor = conflist.GetCursor();
			if (cursor >= 0 && cursor < as.confs.GetCount()) {
				a.sel_sym = symcursor;
				a.sel_conf = cursor;
				
				const ScalperConf& am = as.confs[cursor];
				int row = 0;
				
				for(int i = 0; i < am.start_list.GetCount(); i++) {
					const Vector<MatcherItem>& v = am.start_list[i];
					for(int j = 0; j < v.GetCount(); j++) {
						const MatcherItem& mi = v[j];
						startlist.Set(row, 0, i);
						startlist.Set(row, 1, a.cache[mi.cache].title);
						startlist.Set(row, 2, GetEventString(mi.event));
						row++;
					}
				}
				startlist.SetCount(row);
				
				row = 0;
				for(int i = 0; i < am.sust_list.GetCount(); i++) {
					const Vector<MatcherItem>& v = am.sust_list[i];
					for(int j = 0; j < v.GetCount(); j++) {
						const MatcherItem& mi = v[j];
						sustlist.Set(row, 0, i);
						sustlist.Set(row, 1, a.cache[mi.cache].title);
						sustlist.Set(row, 2, GetEventString(mi.event));
						row++;
					}
				}
				sustlist.SetCount(row);
				
			}
		}
	}
}


}
