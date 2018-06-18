#include "Overlook.h"

namespace Overlook {


AnalyzerOrder::AnalyzerOrder() {
	
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
	
	
	if (orders.IsEmpty())
		FillOrdersScalper(); //FillOrdersMyfxbook();
	if (cache.IsEmpty()) {
		FillInputBooleans();
		InitOrderDescriptor();
	}
	if (clusters.IsEmpty())
		InitClusters();
	for(int i = 0; i < clusters.GetCount(); i++)
		Analyze(clusters[i]);
	RefreshRealtimeClusters();
	
	if (IsRunning())
		StoreThis();
	
	
	while (IsRunning()) {
		FillInputBooleans();
		RefreshRealtimeClusters();
		
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
		
		scalper_signal.GetAdd(sym) = sig;
	}
	
	
	Sort(orders, AnalyzerOrder());
}

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
	
}

void Analyzer::InitOrderDescriptor() {
	
	for(int i = 0; i < orders.GetCount(); i++) {
		AnalyzerOrder& o = orders[i];
		
		o.descriptor		.SetCount(cache.GetCount() * EVENT_COUNT);
		o.start_descriptor	.SetCount(cache.GetCount() * EVENT_COUNT);
		o.sust_descriptor	.SetCount(cache.GetCount() * EVENT_COUNT);
		o.stop_descriptor	.SetCount(cache.GetCount() * EVENT_COUNT);
		int row = 0;
		
		for(int j = 0; j < cache.GetCount(); j++) {
			int e = GetEvent(o, cache[j]);
			
			for(int k = 0; k < EVENT_COUNT; k++) {
				if (e & (1 << k)) {
					o.descriptor.Set(row, true);
					if (k < SUSTAIN_ENABLE)		o.start_descriptor.Set(row, true);
					else if (k < STOP_ENABLE)	o.sust_descriptor.Set(row, true);
					else						o.stop_descriptor.Set(row, true);
				}
				row++;
			}
		}
	}
}

void Analyzer::Analyze(AnalyzerCluster& am) {
	
	for(int i = 0; i < am.orders.GetCount(); i++) {
		AnalyzerOrder& o = orders[am.orders[i]];
		
		am.Add();
		
		for(int j = 0; j < cache.GetCount(); j++) {
			int e = GetEvent(o, cache[j]);
			
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

void Analyzer::InitClusters() {
	int descriptor_size = EVENT_COUNT * cache.GetCount();
	Vector<VectorBool> prev_av_start_descriptors;
	Vector<int> descriptor_sum;
	
	clusters.Clear();
	
	clusters.SetCount(CLUSTER_COUNT);
	for(int i = 0; i < clusters.GetCount(); i++) {
		AnalyzerCluster& c = clusters[i];
		c.av_start_descriptor = orders[Random(orders.GetCount())].start_descriptor;
		prev_av_start_descriptors.Add() = c.av_start_descriptor;
	}
	
	
	for(int cluster_iter = 0; cluster_iter < 10 && IsRunning(); cluster_iter++) {
		ReleaseLog("Analyzer::InitClusters clustering iteration " + IntStr(cluster_iter));
		
		for(int i = 0; i < clusters.GetCount(); i++) {
			AnalyzerCluster& c = clusters[i];
			c.orders.SetCount(0);
		}
		
		// Find closest cluster center
		for(int i = 0; i < orders.GetCount(); i++) {
			const AnalyzerOrder& o = orders[i];
			ASSERT(o.start_descriptor.GetCount() == descriptor_size);
			
			int lowest_distance = INT_MAX, lowest_j = -1;
			for(int j = 0; j < clusters.GetCount(); j++) {
				AnalyzerCluster& c = clusters[j];
				int distance = c.av_start_descriptor.Hamming(o.start_descriptor);
				if (distance < lowest_distance) {
					lowest_distance = distance;
					lowest_j = j;
				}
			}
			
			AnalyzerCluster& c = clusters[lowest_j];
			c.orders.Add(i);
		}
		
		
		// If some cluster is empty, add at least one
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
		
		
		// Calculate new cluster centers
		for(int i = 0; i < clusters.GetCount(); i++) {
			AnalyzerCluster& c = clusters[i];
			descriptor_sum.SetCount(0);
			descriptor_sum.SetCount(descriptor_size, 0);
			for(int j = 0; j < c.orders.GetCount(); j++) {
				const AnalyzerOrder& o = orders[c.orders[j]];
				for(int k = 0; k < descriptor_size; k++) {
					if (o.start_descriptor.Get(k))
						descriptor_sum[k]++;
					else
						descriptor_sum[k]--;
				}
			}
			for(int k = 0; k < descriptor_size; k++) {
				c.av_start_descriptor.Set(k, descriptor_sum[k] > 0);
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
			if (!c.av_start_descriptor.IsEqual(prev_av_start_descriptors[i]))
				all_same = false;
			prev_av_start_descriptors[i] = c.av_start_descriptor;
		}
		if (all_same) break;
		
		
	}
	
	
	for(int i = 0; i < clusters.GetCount(); i++) {
		AnalyzerCluster& c = clusters[i];
		if (c.orders.GetCount() <= 100) {
			clusters.Remove(i);
			i--;
		}
	}
	
}

int Analyzer::GetEvent(const AnalyzerOrder& o, const LabelSource& ls) {
	int e = 0;
	
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
		if (first_sig == label)	e |= 1 << START_POS;
		else					e |= 1 << START_NEG;
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
		if (first_sig == label)	e |= 1 << SUSTAIN_POS;
		else					e |= 1 << SUSTAIN_NEG;
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
		if (last_sig == label)	e |= 1 << STOP_POS;
		else					e |= 1 << STOP_NEG;
	}
	if (post_ena != last_ena) {
		if (last_ena == true)	e |= 1 << STOP_ENABLE;
		else					e |= 1 << STOP_DISABLE;
	}
	
	return e;
}


String Analyzer::GetEventString(int i) {
	switch (i) {
		case START_ENABLE:		return "Start enable";
		case START_DISABLE:		return "Start disable";
		case START_POS:			return "Start positive";
		case START_NEG:			return "Start negative";
		case SUSTAIN_ENABLE:	return "Sustain enable";
		case SUSTAIN_DISABLE:	return "Sustain disable";
		case SUSTAIN_POS:		return "Sustain positive";
		case SUSTAIN_NEG:		return "Sustain negative";
		case STOP_ENABLE:		return "Stop enable";
		case STOP_DISABLE:		return "Stop disable";
		case STOP_POS:			return "Stop positive";
		case STOP_NEG:			return "Stop negative";
		default: return "";
	}
}

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

int Analyzer::FindClosestCluster(int type, const VectorBool& descriptor) {
	int lowest_distance = INT_MAX, lowest_j = -1;
	
	for(int j = 0; j < clusters.GetCount(); j++) {
		AnalyzerCluster& c = clusters[j];
		int distance;
		distance = c.av_start_descriptor.Hamming(descriptor);
		if (distance < lowest_distance) {
			lowest_distance = distance;
			lowest_j = j;
		}
	}
	
	return lowest_j;
}

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
	tabs.Add(realtime);
	tabs.Add(realtime, "Realtime");
	cluster.Horz();
	
	cluster << clusterlist << eventlist;
	cluster.SetPos(2500);
	
	realtime << rtlist << rtdata;
	realtime.SetPos(2500);
	
	clusterlist.AddColumn("Cluster");
	clusterlist.AddColumn("Orders");
	clusterlist << THISBACK(Data);
	
	eventlist.AddColumn("Cache");
	eventlist.AddColumn("Event");
	eventlist.AddColumn("Probability");
	
	rtlist.AddColumn("Long cluster");
	rtlist.AddColumn("Short cluster");
	rtdata.AddColumn("");
	rtdata.AddColumn("");
	rtdata.AddColumn("");
}

void AnalyzerCtrl::Data() {
	System& sys = GetSystem();
	Analyzer& a = GetAnalyzer();
	
	int tab = tabs.Get();
	if (tab == 0) {
		for(int i = 0; i < a.clusters.GetCount(); i++) {
			const AnalyzerCluster& am = a.clusters[i];
			
			clusterlist.Set(i, 0, i);
			clusterlist.Set(i, 1, am.orders.GetCount());
		}
		clusterlist.SetCount(a.clusters.GetCount());
		
		int cursor = clusterlist.GetCursor();
		if (cursor >= 0 && cursor < a.clusters.GetCount()) {
			const AnalyzerCluster& am = a.clusters[cursor];
			int row = 0;
			for(int i = 0; i < am.stats.GetCount(); i++) {
				int key = am.stats.GetKey(i);
				int value = am.stats[i];
				int cache = key / Analyzer::EVENT_COUNT;
				int event  = key % Analyzer::EVENT_COUNT;
				if (event >= Analyzer::SUSTAIN_ENABLE) continue;
				eventlist.Set(row, 0, a.cache[cache].title);
				eventlist.Set(row, 1, a.GetEventString(event));
				eventlist.Set(row, 2, (double)value * 100 / am.total);
				row++;
			}
	
			eventlist.SetSortColumn(2, true);
			eventlist.SetCount(row);
		}
	}
	else if (tab == 1) {
		for(int i = rtlist.GetCount(); i < a.rtdata.GetCount(); i++) {
			const RtData& rt = a.rtdata[i];
			
			rtlist.Set(i, 0, rt.cluster_long);
			rtlist.Set(i, 1, rt.cluster_short);
		}
		
		int cursor = rtlist.GetCursor();
		if (cursor >= 0 && cursor < a.rtdata.GetCount()) {
			const RtData& rt = a.rtdata[cursor];
			
		}
	}
}

}





































#if 0
void Analyzer::Optimize(int cluster) {
	AnalyzerCluster& am = clusters[cluster];
	
	int max_gens = 100;
	for(int i = 0; i < max_gens; i++) {
		am.solutions.SetCount(i+1);
		am.solutions[i].SetCount(0);
		
		if (i == 0) {
			
			// Generate minimal nodes
			
			Vector<int> starts, stops, sustains;
			
			
			for(int i = 0; i < am.stats.GetCount(); i++) {
				int key = am.stats.GetKey(i);
				int value = am.stats[i];
				int cache = key / Analyzer::EVENT_COUNT;
				int event  = key % Analyzer::EVENT_COUNT;
				
				if (event < SUSTAIN_ENABLE)		starts.Add(key);
				else if (event < STOP_ENABLE)	stops.Add(key);
				else							sustains.Add(key);
				
				
			}
			
			int count = min(starts.GetCount(), min(stops.GetCount(), sustains.GetCount()));
			for(int mul = 0; mul < 1; mul++) {
				
				int sust_i = Random(count);
				int start_i = Random(count);
				int stop_i = Random(count);
				
				for(int j = 0; j < count; j++) {
					int start   = starts[start_i];
					int stop    = stops[stop_i];
					int sustain = sustains[sust_i];
					
					AnalyzerSolution& as = am.solutions[0].Add();
					as.symbol = am.symbol;
					as.required.SetCount(1);
					struct Match& startm = as.required[0];
					//struct Match& stopm  = as.required[1];
					//struct Match& sustm  = as.required[2];
					startm.cache = start / Analyzer::EVENT_COUNT;
					startm.event = start % Analyzer::EVENT_COUNT;
					startm.row   = start;
					/*stopm.cache = stop / Analyzer::EVENT_COUNT;
					stopm.event = stop % Analyzer::EVENT_COUNT;
					stopm.row   = stop;
					sustm.cache = sustain / Analyzer::EVENT_COUNT;
					sustm.event = sustain % Analyzer::EVENT_COUNT;
					sustm.row   = sustain;*/
					
					Test(as);
					ReleaseLog("Analyzer::Optimize first gen " + IntStr(cluster) + " " + IntStr(j) + "/" + IntStr(count) + " " + DblStr(as.result));
					
					sust_i	= (sust_i  + 1) % count;
					start_i	= (start_i + 1) % count;
					stop_i	= (stop_i  + 1) % count;
				}
			}
			
			Sort(am.solutions[0], AnalyzerSolution());
		} else {
			
			// Combine best nodes of previous generations
			
			Vector<AnalyzerSolution>& cur  = am.solutions[i];
			Vector<AnalyzerSolution>& prev = am.solutions[i-1];
			
			double gen_max = -DBL_MAX;
			
			for(int j = 1; j < prev.GetCount(); j+=2) {
				AnalyzerSolution& a = prev[j];
				AnalyzerSolution& b = prev[j-1];
				AnalyzerSolution& c = cur.Add();
				c.required.Append(a.required);
				c.required.Append(b.required);
				c.symbol = a.symbol;
			
				Test(c);
				
				if (c.result > gen_max) gen_max = c.result;
				
				ReleaseLog("Analyzer::Optimize gen " + IntStr(i) + " " + IntStr(cluster) + " " + IntStr(j) + "/" + IntStr(prev.GetCount()) + " " + DblStr(c.result) + " (gen max " + DblStr(gen_max) + ")");
				
			}
			
			Sort(cur, AnalyzerSolution());
		}
		
	}
}


void Analyzer::Test(AnalyzerSolution& am) {
	ConstLabelSignal& ls = scalper_signal.Get(am.symbol);
	int begin = 0;
	int end = min(cache[0].data.signal.GetCount(), ls.signal.GetCount());
	double score = 0;
	
	bool prev_sig = false, prev_ena = false;
	for(int i = begin; i < end; i++) {
		bool sig = ls.signal.Get(i);
		bool ena = ls.enabled.Get(i);
		
		bool is_start = (ena && !prev_ena) || (ena && prev_ena && prev_sig != sig);
		bool is_stop = !ena && prev_ena;
		
		bool cluster_start = true;
		bool cluster_stop = true;
		bool cluster_ena = true;
		
		bool has_label = false;
		bool label = false;
		
		for(int j = 0; j < am.required.GetCount(); j++) {
			const struct Match& m = am.required[j];
			
			const LabelSignal& ls = cache[m.cache].data;
			bool prev_sig = ls.signal.Get(i-1);
			bool prev_ena = ls.enabled.Get(i-1);
			bool sig = ls.signal.Get(i);
			bool ena = ls.enabled.Get(i);
			
			
			switch (m.event) {
				case START_ENABLE:
					if (!(!prev_ena && ena))
						cluster_start = false;
					break;
				case START_DISABLE:
					if (!(prev_ena && !ena))
						cluster_start = false;
					break;
				case START_POS:
					if (!has_label) {label = sig; has_label = true;}
					if (!(prev_sig != sig && sig == label))
						cluster_start = false;
					break;
				case START_NEG:
					if (!has_label) {label = !sig; has_label = true;}
					if (!(prev_sig != sig && sig != label))
						cluster_start = false;
					break;
				case SUSTAIN_ENABLE:
					if (!(ena))
						cluster_ena = false;
					break;
				case SUSTAIN_DISABLE:
					if (!(!ena))
						cluster_ena = false;
					break;
				case SUSTAIN_POS:
					if (!has_label) {label = sig; has_label = true;}
					if (!(sig == label))
						cluster_ena = false;
					break;
				case SUSTAIN_NEG:
					if (!has_label) {label = !sig; has_label = true;}
					if (!(sig != label))
						cluster_ena = false;
					break;
				case STOP_ENABLE:
					if (!(prev_ena && !ena))
						cluster_stop = false;
					break;
				case STOP_DISABLE:
					if (!(!prev_ena && ena))
						cluster_stop = false;
					break;
				case STOP_POS:
					if (!has_label) {label = sig; has_label = true;}
					if (!(prev_sig != sig && sig != label))
						cluster_stop = false;
					break;
				case STOP_NEG:
					if (!has_label) {label = !sig; has_label = true;}
					if (!(prev_sig != sig && sig == label))
						cluster_stop = false;
					break;
			}
			
			if (!cluster_start && !cluster_ena && !cluster_stop) break;
		}
		
		
		if (cluster_start) {
			if (is_start)		score += 5;
			else				score -= 5;
		}
		/*if (cluster_stop) {
			if (is_stop)		score += 5;
			else				score -= 5;
		}
		if (cluster_ena) {
			if (ena)			score += 1;
			else				score -= 1;
		}*/
		
		
		prev_sig = sig;
		prev_ena = ena;
	}
	
	if (score == 0.0)
		score = -1000000;
	
	am.result = score;
}

bool Analyzer::Match(const AnalyzerMatcher& am, const AnalyzerOrder& o) {
	if (am.symbol != o.symbol)
		return false;
	
	
	return true;
}
#endif