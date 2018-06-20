#include "Overlook.h"

namespace Overlook {


void UpdateEventVectors(LabelSource& ls) {
	
	int counted = ls.eventdata[0].GetCount();
	int bars = ls.data.signal.GetCount();
	
	for(int i = 0; i < EVENT_COUNT; i++) {
		VectorBool& vb = ls.eventdata[i];
		vb.SetCount(bars);
		
		for(int j = counted; j < bars; j++) {
			bool prev_sig = ls.data.signal.Get(j-1);
			bool prev_ena = ls.data.enabled.Get(j-1);
			bool sig = ls.data.signal.Get(j);
			bool ena = ls.data.enabled.Get(j);
			
			switch (i) {
				case START_ENABLE:      if (ena && !prev_ena) vb.Set(j, true); break;
				case START_DISABLE:     if (!ena && prev_ena) vb.Set(j, true); break;
				case START_POS_LONG:    if (!sig && prev_sig) vb.Set(j, true); break;
				case START_NEG_LONG:    if (sig && !prev_sig) vb.Set(j, true); break;
				case START_POS_SHORT:   if (sig && !prev_sig) vb.Set(j, true); break;
				case START_NEG_SHORT:   if (!sig && prev_sig) vb.Set(j, true); break;
				case SUSTAIN_ENABLE:    if (ena) vb.Set(j, true); break;
				case SUSTAIN_DISABLE:   if (!ena) vb.Set(j, true); break;
				case SUSTAIN_POS_LONG:  if (!sig) vb.Set(j, true); break;
				case SUSTAIN_NEG_LONG:  if (sig) vb.Set(j, true); break;
				case SUSTAIN_POS_SHORT: if (sig) vb.Set(j, true); break;
				case SUSTAIN_NEG_SHORT: if (!sig) vb.Set(j, true); break;
				case STOP_ENABLE:       if (!ena && prev_ena) vb.Set(j, true); break;
				case STOP_DISABLE:      if (ena && !prev_ena) vb.Set(j, true); break;
				case STOP_POS_LONG:     if (sig && !prev_sig) vb.Set(j, true); break;
				case STOP_NEG_LONG:     if (!sig && prev_sig) vb.Set(j, true); break;
				case STOP_POS_SHORT:    if (!sig && prev_sig) vb.Set(j, true); break;
				case STOP_NEG_SHORT:    if (sig && !prev_sig) vb.Set(j, true); break;
			}
		}
		
	}
	
}



AnalyzerOrder::AnalyzerOrder() {
	
}






void AnalyzerSymbol::InitScalperSignal(ConstLabelSignal& full_scalper_signal, bool type, const Vector<AnalyzerOrder>& orders) {
	this->type = type;
	
	scalper_signal.data.signal	.SetCount(full_scalper_signal.signal	.GetCount());
	scalper_signal.data.enabled	.SetCount(full_scalper_signal.enabled	.GetCount());
	
	// Disable opposite signal
	for(int i = 0; i < full_scalper_signal.signal.GetCount(); i++) {
		bool ena = full_scalper_signal.enabled.Get(i);
		bool sig = full_scalper_signal.signal.Get(i);
		if (ena && sig != type) ena = false;
		scalper_signal.data.signal.Set(i, sig);
		scalper_signal.data.enabled.Set(i, ena);
	}
	
	UpdateEventVectors(scalper_signal);
	
	
	for(int i = 0; i < orders.GetCount(); i++) {
		const AnalyzerOrder& o = orders[i];
		if (o.action == type)
			this->orders.Add(o);
	}
	//Sort(orders, AnalyzerOrder());
}

void AnalyzerSymbol::Init() {
	
	InitOrderDescriptor();
	InitClusters();
	for(int i = 0; i < clusters.GetCount(); i++)
		Analyze(clusters[i]);
	InitStartMatchers();
}

void AnalyzerSymbol::InitOrderDescriptor() {
	
	for(int i = 0; i < orders.GetCount(); i++) {
		AnalyzerOrder& o = orders[i];
		
		o.descriptor		.SetCount(a->cache.GetCount() * EVENT_COUNT);
		int row = 0;
		
		for(int j = 0; j < a->cache.GetCount(); j++) {
			int e = a->GetEvent(o, a->cache[j]);
			
			for(int k = 0; k < EVENT_COUNT; k++) {
				if (e & (1 << k)) {
					o.descriptor.Set(row, true);
				}
				row++;
			}
		}
	}
	
}

void AnalyzerSymbol::InitClusters() {
	int descriptor_size = EVENT_COUNT * a->cache.GetCount();
	Vector<VectorBool> prev_av_descriptors;
	Vector<int> descriptor_sum;
	
	clusters.Clear();
	
	clusters.SetCount(CLUSTER_COUNT);
	for(int i = 0; i < clusters.GetCount(); i++) {
		AnalyzerCluster& c = clusters[i];
		c.av_descriptor = orders[Random(orders.GetCount())].descriptor;
		prev_av_descriptors.Add() = c.av_descriptor;
	}
	
	
	int cluster_iter_max = 3;
	
	for(int cluster_iter = 0; cluster_iter < cluster_iter_max && a->IsRunning(); cluster_iter++) {
		ReleaseLog("Analyzer::InitClusters clustering iteration " + IntStr(cluster_iter));
		
		for(int i = 0; i < clusters.GetCount(); i++) {
			AnalyzerCluster& c = clusters[i];
			c.orders.SetCount(0);
		}
		
		// Find closest cluster center
		for(int i = 0; i < orders.GetCount(); i++) {
			const AnalyzerOrder& o = orders[i];
			ASSERT(o.descriptor.GetCount() == descriptor_size);
			
			int lowest_distance = INT_MAX, lowest_j = -1;
			for(int j = 0; j < clusters.GetCount(); j++) {
				AnalyzerCluster& c = clusters[j];
				int distance = c.av_descriptor.Hamming(o.descriptor);
				if (distance < lowest_distance) {
					lowest_distance = distance;
					lowest_j = j;
				}
			}
			
			AnalyzerCluster& c = clusters[lowest_j];
			c.orders.Add(i);
		}
		
		
		// If some cluster is empty, add at least one
		if (cluster_iter < cluster_iter_max-1) {
			for(int i = 0; i < clusters.GetCount(); i++) {
				AnalyzerCluster& c = clusters[i];
				if (c.orders.IsEmpty()) {
					for(int j = 0; j < clusters.GetCount(); j++) {
						if (clusters[j].orders.GetCount() > 1) {
							int o = clusters[j].orders.Pop();
							c.orders.Add(o);
							break;
						}
					}
				}
			}
		}
		
		
		// Calculate new cluster centers
		for(int i = 0; i < clusters.GetCount(); i++) {
			AnalyzerCluster& c = clusters[i];
			descriptor_sum.SetCount(0);
			descriptor_sum.SetCount(descriptor_size, 0);
			for(int j = 0; j < c.orders.GetCount(); j++) {
				const AnalyzerOrder& o = orders[c.orders[j]];
				for(int k = 0; k < descriptor_size; k++) {
					if (o.descriptor.Get(k))
						descriptor_sum[k]++;
					else
						descriptor_sum[k]--;
				}
			}
			for(int k = 0; k < descriptor_size; k++) {
				c.av_descriptor.Set(k, descriptor_sum[k] > 0);
			}
		}
		
		
		// Dump debug info
		if (0) {
			String out;
			for(int i = 0; i < clusters.GetCount(); i++) {
				AnalyzerCluster& c = clusters[i];
				ReleaseLog("Cluster " + IntStr(i) + ": " + IntStr(c.orders.GetCount()) + " orders");
			}
		}
		
		
		// Break loop when no new centers have been found
		bool all_same = true;
		for(int i = 0; i < clusters.GetCount() && all_same; i++) {
			AnalyzerCluster& c = clusters[i];
			if (!c.av_descriptor.IsEqual(prev_av_descriptors[i]))
				all_same = false;
			prev_av_descriptors[i] = c.av_descriptor;
		}
		if (all_same) break;
		
		
	}
	
	
	for(int i = 0; i < clusters.GetCount(); i++) {
		AnalyzerCluster& c = clusters[i];
		if (c.orders.GetCount() <= 0) {
			clusters.Remove(i);
			i--;
		}
	}
	
	
	/*for(int i = 0; i < clusters.GetCount(); i++) {
		AnalyzerCluster& c = clusters[i];
		
		int order_type = 0;
		for(int j = 0; j < c.orders.GetCount(); j++) {
			const AnalyzerOrder& o = orders[c.orders[j]];
			if (!o.action)	order_type--;
			else			order_type++;
		}
		
		c.type = order_type > 0;
	}*/
}

void AnalyzerSymbol::Analyze(AnalyzerCluster& am) {
	
	for(int i = 0; i < am.orders.GetCount(); i++) {
		AnalyzerOrder& o = orders[am.orders[i]];
		
		am.Add();
		
		for(int j = 0; j < a->cache.GetCount(); j++) {
			int e = a->GetEvent(o, a->cache[j]);
			
			for(int k = 0; k < EVENT_COUNT; k++) {
				if (e & (1 << k)) {
					int id = j * EVENT_COUNT + k;
					am.AddEvent(id);
				}
			}
		}
		
		//am.Sort();
	}
}

void AnalyzerSymbol::InitStartMatchers() {
	#if 0
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());
	for(int i = 0; i < clusters.GetCount(); i++) {
		co & THISBACK1(InitStartMatchers, i);
	}
	co.Finish();
	#else
	for(int i = 0; i < clusters.GetCount(); i++)
		InitStartMatchers(i);
	#endif
}

void AnalyzerSymbol::InitStartMatchers(int cluster) {
	Vector<MatcherItem> list;
	list.SetCount(2);
	
	AnalyzerCluster& c = clusters[cluster];
	
	MatcherCache mcache;
	
	c.test_results.Clear();
	
	Vector<int> start_keys;
	for(int i = 0; i < c.stats.GetCount(); i++) {
		int key = c.stats.GetKey(i);
		int event = key % EVENT_COUNT;
		int type = GetEventType(event);
		if (type == EVT_START)
			start_keys.Add(key);
	}
	
	if (c.start_optimizer.GetRound() == 0) {
		c.start_optimizer.Init(start_keys.GetCount(), 30);
	}
	
	while (!c.start_optimizer.IsEnd() && a->IsRunning()) {
		
		c.start_optimizer.Start();
		
		const Vector<int>& trial = c.start_optimizer.GetTrialSolution();
		
		list.SetCount(0);
		for(int i = 0; i < trial.GetCount(); i++) {
			int key = start_keys[trial[i]];
			int event = key % EVENT_COUNT;
			int type = GetEventType(event);
			if (type != EVT_START) continue;
			MatcherItem& mi = list.Add();
			mi.cache = key / EVENT_COUNT;
			mi.event = event;
		}
		
		double score = 0;
		MatchTest test;
		if (!list.IsEmpty())
			RunMatchTest(0, list, test, mcache, c);
		score = test.true_match;// - test.false_match;
		if (score == 0) score = -100000000;
		
		if (c.start_optimizer.GetRound() % 1000 == 0)
			ReleaseLog("Analyzer::InitStartMatchers " + IntStr(cluster) + " " + IntStr(c.start_optimizer.GetRound()) + " " + IntStr(c.start_optimizer.GetRound() * 100 / c.start_optimizer.GetMaxRounds()) +
				"% trial " + IntStr(list.GetCount()) + " false_match " + IntStr(test.false_match) +
				" true_match " + IntStr(test.true_match) +
				" score " + DblStr(score));
		
		c.start_optimizer.Stop(score);
		
		
		if (test.true_match >= 95 * test.true_total / 100)
			break;
	}
}

void AnalyzerSymbol::RunMatchTest(int evtype, const Vector<MatcherItem>& list, MatchTest& t, MatcherCache& mcache, AnalyzerCluster& c) {
	ASSERT(!list.IsEmpty());
	
	t.false_match = 0;
	t.false_total = 0;
	t.true_match = 0;
	
	// Cluster order symbols
	const LabelSource& ls = scalper_signal;
	//mcache.matcher_and.SetCount(ls.data.signal.GetCount());
	mcache.matcher_or.SetCount(ls.data.signal.GetCount());
	ASSERT(ls.data.signal.GetCount() > 0);
	
	mcache.matcher_or.Zero();
	
	for(int k = 0; k < list.GetCount(); k++) {
		const MatcherItem& mi = list[k];
		const LabelSource& c = a->cache[mi.cache];
		const VectorBool& vb = c.eventdata[mi.event];
		ASSERT(!vb.IsEmpty());
		
		/*if (k % 3 == 0) {
			if (k) {
				mcache.matcher_or.Or(mcache.matcher_and);
			}
			mcache.matcher_and = vb;
		}
		else {
			mcache.matcher_and.And(vb);
		}*/
		
		mcache.matcher_or.Or(vb);
	}
	//mcache.matcher_or.Or(mcache.matcher_and);
	

	// Long/short
	if (this->type == false) {
		if (evtype == EVT_START) {
			ASSERT(ls.eventdata[START_POS_LONG].GetCount() > 0);
			t.false_match += mcache.matcher_or.PopCountNotAnd(ls.eventdata[START_POS_LONG]);
			t.true_match += mcache.matcher_or.PopCountAnd(ls.eventdata[START_POS_LONG]);
		}
		else Panic("Not implemented");
		
		t.true_total = ls.eventdata[START_POS_LONG].PopCount();
	} else {
		if (evtype == EVT_START) {
			ASSERT(ls.eventdata[START_POS_SHORT].GetCount() > 0);
			t.false_match += mcache.matcher_or.PopCountNotAnd(ls.eventdata[START_POS_SHORT]);
			t.true_match += mcache.matcher_or.PopCountAnd(ls.eventdata[START_POS_SHORT]);
		}
		else Panic("Not implemented");
		
		t.true_total = ls.eventdata[START_POS_SHORT].PopCount();
	}
}











Analyzer::Analyzer() {
	System& sys = GetSystem();
	sym_ids.Add(sys.FindSymbol("EURUSD"));
	sym_ids.Add(sys.FindSymbol("GBPUSD"));
	sym_ids.Add(sys.FindSymbol("EURJPY"));
	sym_ids.Add(sys.FindSymbol("USDJPY"));
	
	sym_ids.Add(sys.FindSymbol("EURGBP"));
	sym_ids.Add(sys.FindSymbol("GBPJPY"));
	sym_ids.Add(sys.FindSymbol("AUDUSD"));
	sym_ids.Add(sys.FindSymbol("USDCAD"));
	
	sym_ids.Add(sys.FindSymbol("EURAUD"));
	sym_ids.Add(sys.FindSymbol("AUDCAD"));
	sym_ids.Add(sys.FindSymbol("EURCHF"));
	sym_ids.Add(sys.FindSymbol("CADJPY"));
	
	sym_ids.Add(sys.FindSymbol("GBPCHF"));
	sym_ids.Add(sys.FindSymbol("USDCHF"));
	sym_ids.Add(sys.FindSymbol("NZDUSD"));
	sym_ids.Add(sys.FindSymbol("AUDJPY"));
	
	tf_ids.Add(0);
	
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

Analyzer::~Analyzer() {
	running = false;
	while (!stopped) Sleep(100);
}

void Analyzer::Process() {
	running = true;
	stopped = false;
	
	
	if (symbols.IsEmpty()) {
		FillOrdersScalper(); //FillOrdersMyfxbook();
		FillInputBooleans();
		
		CoWork co;
		co.SetPoolSize(GetUsedCpuCores());
		for(int i = 0; i < symbols.GetCount(); i++) {
			co & symbols[i].InitCb();
		}
		co.Finish();
		
		
		if (IsRunning())
			StoreThis();
	}
	
	
	
	while (IsRunning()) {
		FillInputBooleans();
		//RefreshRealtimeClusters();
		
		for(int i = 0; i < 10 && IsRunning(); i++)
			Sleep(1000);
	}
	/*
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());
	for(int i = 0; i < clusters.GetCount(); i++) {
		co & THISBACK1(Optimize, i);
	}
	co.Finish();
	*/
	stopped = true;
}

void Analyzer::FillOrdersScalper() {
	System& sys = GetSystem();
	Vector<Ptr<CoreItem> > work_queue;
	Vector<FactoryDeclaration> indi_ids;
	
	FactoryDeclaration decl;
	decl.factory = sys.Find<ScalperSignal>();
	indi_ids.Add(decl);
	
	sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	for(int i = 0; i < work_queue.GetCount() && IsRunning(); i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
		
		Core& c = *ci.core;
		if (c.GetFactory() == 0) continue;
		
		ConstLabelSignal& sig = c.GetLabelBuffer(0, 0);
		int sym = c.GetSymbol();
		
		Vector<AnalyzerOrder> orders;
		for(int j = 0; j < sig.enabled.GetCount(); j++) {
			if (sig.enabled.Get(j)) {
				bool label = sig.signal.Get(j);
				
				AnalyzerOrder& ao = orders.Add();
				ao.begin = j;
				
				for (; j < sig.enabled.GetCount(); j++) {
					if (!sig.enabled.Get(j))
						break;
				}
				ao.end = j;
				ao.symbol = sym;
				ao.action = label;
				ao.lots = 0.01;
				
			}
		}
		
		if (symbols.Find(sym) == -1) {
			AnalyzerSymbol& as_long = symbols.Add(sym);
			as_long.InitScalperSignal(sig, false, orders);
			
			AnalyzerSymbol& as_short = symbols.Add(sym);
			as_short.InitScalperSignal(sig, true, orders);
		}
	}
	
	
	RefreshSymbolPointer();
}
/*
void Analyzer::FillOrdersMyfxbook() {
	Myfxbook& m = GetMyfxbook();
	Myfxbook::Account& a = m.accounts[0];
	
	for(int i = 0; i < a.history_orders.GetCount(); i++) {
		Myfxbook::Order& o = a.history_orders[i];
		AnalyzerOrder& ao = orders.Add();
		//ao.begin = o.begin;
		//ao.end = o.end;
		Panic("TODO find shift for time");
		//ao.symbol = o.symbol;
		ao.action = o.action;
		ao.lots = o.lots;
	}
}
*/
void Analyzer::FillInputBooleans() {
	System& sys = GetSystem();
	
	if (work_queue.IsEmpty()) {
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	}
	
	int row = 0;
	
	for(int i = 0; i < work_queue.GetCount() && IsRunning(); i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
		
		Core& c = *ci.core;
		
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
		
		ReleaseLog("Analyzer::FillInputBooleans " + IntStr(i) + "/" + IntStr(work_queue.GetCount()) + "  " + IntStr(i * 100 / work_queue.GetCount()) + "%");
	}
	
	for(int i = 0; i < cache.GetCount(); i++) {
		UpdateEventVectors(cache[i]);
	}
}

int Analyzer::GetEvent(const AnalyzerOrder& o, const LabelSource& ls) {
	int e = 0;
	
	if (o.begin >= ls.data.signal.GetCount()) return e;
	
	bool label = o.action;
	e |= GetEventBegin(o.action, o.begin, ls);
	
	if (o.end + 1 >= ls.data.signal.GetCount()) return e;
	
	e |= GetEventSustain(o.action, o.begin, o.end, ls);
	e |= GetEventEnd(o.action, o.end, ls);
	
	return e;
}

int Analyzer::GetEventBegin(bool label, int begin, const LabelSource& ls) {
	int e = 0;
	
	bool pre_sig = ls.data.signal.Get(begin - 1);
	bool pre_ena = ls.data.enabled.Get(begin - 1);
	bool first_sig = ls.data.signal.Get(begin);
	bool first_ena = ls.data.enabled.Get(begin);
	
	if (pre_sig != first_sig) {
		if (label == false) {
			if (first_sig == label)	e |= 1 << START_POS_LONG;
			else					e |= 1 << START_NEG_LONG;
		} else {
			if (first_sig == label)	e |= 1 << START_POS_SHORT;
			else					e |= 1 << START_NEG_SHORT;
		}
	}
	if (pre_ena != first_ena) {
		if (first_ena == true)	e |= 1 << START_ENABLE;
		else					e |= 1 << START_DISABLE;
	}
	
	return e;
}

int Analyzer::GetEventSustain(bool label, int begin, int end, const LabelSource& ls) {
	int e = 0;
	
	bool pre_sig = ls.data.signal.Get(begin - 1);
	bool pre_ena = ls.data.enabled.Get(begin - 1);
	bool first_sig = ls.data.signal.Get(begin);
	bool first_ena = ls.data.enabled.Get(begin);
	bool last_sig = ls.data.signal.Get(end - 1);
	bool last_ena = ls.data.enabled.Get(end - 1);
	bool post_sig = ls.data.signal.Get(end);
	bool post_ena = ls.data.enabled.Get(end);
	
	bool sig_sustain = true, ena_sustain = true;
	for (int i = begin + 1; i < end; i++) {
		bool sig = ls.data.signal.Get(i);
		bool ena = ls.data.enabled.Get(i);
		if (sig != first_sig) sig_sustain = false;
		if (ena != first_ena) ena_sustain = false;
	}
	if (sig_sustain) {
		if (label == false) {
			if (first_sig == label)	e |= 1 << SUSTAIN_POS_LONG;
			else					e |= 1 << SUSTAIN_NEG_LONG;
		} else {
			if (first_sig == label)	e |= 1 << SUSTAIN_POS_SHORT;
			else					e |= 1 << SUSTAIN_NEG_SHORT;
		}
	}
	if (ena_sustain) {
		if (first_ena == true)	e |= 1 << SUSTAIN_ENABLE;
		else					e |= 1 << SUSTAIN_DISABLE;
	}
	
	return e;
}

int Analyzer::GetEventEnd(bool label, int end, const LabelSource& ls) {
	int e = 0;
	
	bool last_sig = ls.data.signal.Get(end - 1);
	bool last_ena = ls.data.enabled.Get(end - 1);
	bool post_sig = ls.data.signal.Get(end);
	bool post_ena = ls.data.enabled.Get(end);
	
	if (post_sig != last_sig) {
		if (label == false) {
			if (last_sig == label)	e |= 1 << STOP_POS_LONG;
			else					e |= 1 << STOP_NEG_LONG;
		} else {
			if (last_sig == label)	e |= 1 << STOP_POS_SHORT;
			else					e |= 1 << STOP_NEG_SHORT;
		}
	}
	if (post_ena != last_ena) {
		if (last_ena == true)	e |= 1 << STOP_ENABLE;
		else					e |= 1 << STOP_DISABLE;
	}
	
	return e;
}


String GetEventString(int i) {
	switch (i) {
		case START_ENABLE:		return "Start enable";
		case START_DISABLE:		return "Start disable";
		case START_POS_LONG:	return "Start positive long";
		case START_NEG_LONG:	return "Start negative long";
		case START_POS_SHORT:	return "Start positive short";
		case START_NEG_SHORT:	return "Start negative short";
		case SUSTAIN_ENABLE:	return "Sustain enable";
		case SUSTAIN_DISABLE:	return "Sustain disable";
		case SUSTAIN_POS_LONG:	return "Sustain positive long";
		case SUSTAIN_NEG_LONG:	return "Sustain negative long";
		case SUSTAIN_POS_SHORT:	return "Sustain positive short";
		case SUSTAIN_NEG_SHORT:	return "Sustain negative short";
		case STOP_ENABLE:		return "Stop enable";
		case STOP_DISABLE:		return "Stop disable";
		case STOP_POS_LONG:		return "Stop positive long";
		case STOP_NEG_LONG:		return "Stop negative long";
		case STOP_POS_SHORT:	return "Stop positive short";
		case STOP_NEG_SHORT:	return "Stop negative short";
		default: return "";
	}
}
/*
void Analyzer::RefreshRealtimeClusters() {
	VectorBool rt_start_descriptor;
	
	int bars = cache[0].data.signal.GetCount();
	for(int i = 1; i < cache.GetCount(); i++)
		bars = min(bars, cache[i].data.signal.GetCount());
	
	rtdata.Reserve(bars);
	
	for(int i = rtcluster_counted; i < bars; i++) {
		RtData& rt = rtdata.Add();
		
		GetRealtimeStartDescriptor(false, i, rt_start_descriptor);
		rt.cluster_long = FindClosestCluster(0, rt_start_descriptor);
		
		
		GetRealtimeStartDescriptor(true, i, rt_start_descriptor);
		rt.cluster_short = FindClosestCluster(0, rt_start_descriptor);
		
		
		
	}
	
	rtcluster_counted = bars;
}
*/
/*int Analyzer::FindClosestCluster(int type, const VectorBool& descriptor) {
	int lowest_distance = INT_MAX, lowest_j = -1;
	
	for(int j = 0; j < clusters.GetCount(); j++) {
		AnalyzerCluster& c = clusters[j];
		int distance;
		distance = c.av_descriptor.Hamming(descriptor);
		if (distance < lowest_distance) {
			lowest_distance = distance;
			lowest_j = j;
		}
	}
	
	return lowest_j;
}*/

void Analyzer::GetRealtimeStartDescriptor(bool label, int i, VectorBool& descriptor) {
	int descriptor_size = EVENT_COUNT * cache.GetCount();
	
	int row = 0;
	
	descriptor.SetCount(descriptor_size);
	descriptor.Zero();
	
	for(int j = 0; j < cache.GetCount(); j++) {
		int e = GetEventBegin(label, i, cache[j]);
		
		for(int k = 0; k < EVENT_COUNT; k++) {
			if (e & (1 << k)) {
				if (k < SUSTAIN_ENABLE)		descriptor.Set(row, true);
			}
			row++;
		}
	}
}










AnalyzerCtrl::AnalyzerCtrl() {
	Add(tabs.SizePos());
	tabs.Add(cluster);
	tabs.Add(cluster, "Clusters");
	tabs.Add(minmatchers);
	tabs.Add(minmatchers, "Mini");
	cluster.Horz();
	
	cluster << symbollist << clusterlist << eventlist;
	cluster.SetPos(1000, 0);
	cluster.SetPos(2500, 1);
	
	symbollist.AddColumn("Symbol");
	symbollist.AddColumn("Type");
	
	clusterlist.AddColumn("Cluster");
	clusterlist.AddColumn("Orders");
	clusterlist << THISBACK(Data);
	
	eventlist.AddColumn("Cache");
	eventlist.AddColumn("Event");
	eventlist.AddColumn("Probability");
	
}

void AnalyzerCtrl::Data() {
	System& sys = GetSystem();
	Analyzer& a = GetAnalyzer();
	
	int tab = tabs.Get();
	if (tab == 0) {
		
		for(int i = 0; i < a.symbols.GetCount(); i++) {
			const AnalyzerSymbol& as = a.symbols[i];
			symbollist.Set(i, 0, sys.GetSymbol(a.symbols.GetKey(i)));
			symbollist.Set(i, 1, as.type ? "Short" : "Long");
		}
		
		int symcursor = symbollist.GetCursor();
		
		if (symcursor >= 0 & symcursor < a.symbols.GetCount()) {
			const AnalyzerSymbol& as = a.symbols[symcursor];
			
			for(int i = 0; i < as.clusters.GetCount(); i++) {
				const AnalyzerCluster& am = as.clusters[i];
				
				clusterlist.Set(i, 0, i);
				clusterlist.Set(i, 1, am.orders.GetCount());
			}
			clusterlist.SetCount(as.clusters.GetCount());
			
			int cursor = clusterlist.GetCursor();
			if (cursor >= 0 && cursor < as.clusters.GetCount()) {
				const AnalyzerCluster& am = as.clusters[cursor];
				int row = 0;
				for(int i = 0; i < am.stats.GetCount(); i++) {
					int key = am.stats.GetKey(i);
					int value = am.stats[i];
					int cache = key / EVENT_COUNT;
					int event  = key % EVENT_COUNT;
					//if (event >= Analyzer::SUSTAIN_ENABLE) continue;
					eventlist.Set(row, 0, a.cache[cache].title);
					eventlist.Set(row, 1, GetEventString(event));
					eventlist.Set(row, 2, (double)value * 100 / am.total);
					row++;
				}
		
				eventlist.SetSortColumn(2, true);
				eventlist.SetCount(row);
			}
		}
	}
	else if (tab == 1) {
		/*for(int i = rtlist.GetCount(); i < a.rtdata.GetCount(); i++) {
			const RtData& rt = a.rtdata[i];
			
			rtlist.Set(i, 0, rt.cluster_long);
			rtlist.Set(i, 1, rt.cluster_short);
		}
		
		int cursor = rtlist.GetCursor();
		if (cursor >= 0 && cursor < a.rtdata.GetCount()) {
			const RtData& rt = a.rtdata[cursor];
			
		}*/
	}
	else if (tab == 2) {
		/*for(int i = 0; i < a.clusters.GetCount(); i++) {
			const AnalyzerCluster& am = a.clusters[i];
			
			testclusterlist.Set(i, 0, i);
			testclusterlist.Set(i, 1, am.test_results.GetCount());
		}
		testclusterlist.SetCount(a.clusters.GetCount());
		
		int cursor = testclusterlist.GetCursor();
		if (cursor >= 0 && cursor < a.clusters.GetCount()) {
			const AnalyzerCluster& am = a.clusters[cursor];
			int row = 0;
			for(int i = 0; i < am.test_results.GetCount(); i++) {
				const MatchTest& mt = am.test_results[i];
				
				testresultlist.Set(row, 0, mt.false_match);
				testresultlist.Set(row, 1, mt.false_total);
				testresultlist.Set(row, 2, mt.true_match);
				testresultlist.Set(row, 3, mt.true_total);
				
				row++;
			}
			
			testresultlist.SetCount(am.test_results.GetCount());
		}*/
	}
}

}


