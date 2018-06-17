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
	
	#ifndef flagDEBUG
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
	#endif
	
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
	if (cache.IsEmpty())
		FillInputBooleans();
	if (matchers.IsEmpty())
		InitMatchers();
	for(int i = 0; i < matchers.GetCount(); i++)
		Analyze(matchers[i]);
	
	if (running)
		StoreThis();
	
	
	
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());
	for(int i = 0; i < matchers.GetCount(); i++) {
		co & THISBACK1(Optimize, i);
	}
	co.Finish();
	
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
	
	for(int i = 0; i < work_queue.GetCount() && running; i++) {
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
	
	Vector<Ptr<CoreItem> > work_queue;
	
	cache.Clear();
	work_queue.Clear();
	sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	for(int i = 0; i < work_queue.GetCount() && running; i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
		
		Core& c = *ci.core;
		
		for(int j = 0; j < c.GetLabelCount(); j++) {
			ConstLabel& l = c.GetLabel(j);
			
			for(int k = 0; k < l.buffers.GetCount(); k++) {
				ConstLabelSignal& src = l.buffers[k];
				
				LabelSource& ls = cache.Add();
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
			}
		}
		
		
		if (c.GetFactory() != 0)
			ci.core.Clear();
		
		ReleaseLog("Analyzer::FillInputBooleans " + IntStr(i) + "/" + IntStr(work_queue.GetCount()) + "  " + IntStr(i * 100 / work_queue.GetCount()) + "%");
	}
	
}

void Analyzer::InitMatchers() {
	matchers.Clear();
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		AnalyzerMatcher& am = matchers.Add();
		
		am.symbol = sym_ids[i];
	}
	
}

void Analyzer::Analyze(AnalyzerMatcher& am) {
	
	
	
	for(int i = 0; i < orders.GetCount(); i++) {
		AnalyzerOrder& o = orders[i];
		
		if (!Match(am, o)) continue;
		
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
		
		am.Sort();
	}
	
	
	
}

void Analyzer::Optimize(int matcher) {
	AnalyzerMatcher& am = matchers[matcher];
	
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
					as.required.SetCount(3);
					struct Match& startm = as.required[0];
					struct Match& stopm  = as.required[1];
					struct Match& sustm  = as.required[2];
					startm.cache = start / Analyzer::EVENT_COUNT;
					startm.event = start % Analyzer::EVENT_COUNT;
					startm.row   = start;
					stopm.cache = stop / Analyzer::EVENT_COUNT;
					stopm.event = stop % Analyzer::EVENT_COUNT;
					stopm.row   = stop;
					sustm.cache = sustain / Analyzer::EVENT_COUNT;
					sustm.event = sustain % Analyzer::EVENT_COUNT;
					sustm.row   = sustain;
					
					Test(as);
					ReleaseLog("Analyzer::Optimize first gen " + IntStr(matcher) + " " + IntStr(j) + "/" + IntStr(count) + " " + DblStr(as.result));
					
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
				
				ReleaseLog("Analyzer::Optimize gen " + IntStr(i) + " " + IntStr(matcher) + " " + IntStr(j) + "/" + IntStr(prev.GetCount()) + " " + DblStr(c.result) + " (gen max " + DblStr(gen_max) + ")");
				
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
		
		bool matcher_start = true;
		bool matcher_stop = true;
		bool matcher_ena = true;
		
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
						matcher_start = false;
					break;
				case START_DISABLE:
					if (!(prev_ena && !ena))
						matcher_start = false;
					break;
				case START_POS:
					if (!has_label) {label = sig; has_label = true;}
					if (!(prev_sig != sig && sig == label))
						matcher_start = false;
					break;
				case START_NEG:
					if (!has_label) {label = !sig; has_label = true;}
					if (!(prev_sig != sig && sig != label))
						matcher_start = false;
					break;
				case SUSTAIN_ENABLE:
					if (!(ena))
						matcher_ena = false;
					break;
				case SUSTAIN_DISABLE:
					if (!(!ena))
						matcher_ena = false;
					break;
				case SUSTAIN_POS:
					if (!has_label) {label = sig; has_label = true;}
					if (!(sig == label))
						matcher_ena = false;
					break;
				case SUSTAIN_NEG:
					if (!has_label) {label = !sig; has_label = true;}
					if (!(sig != label))
						matcher_ena = false;
					break;
				case STOP_ENABLE:
					if (!(prev_ena && !ena))
						matcher_stop = false;
					break;
				case STOP_DISABLE:
					if (!(!prev_ena && ena))
						matcher_stop = false;
					break;
				case STOP_POS:
					if (!has_label) {label = sig; has_label = true;}
					if (!(prev_sig != sig && sig != label))
						matcher_stop = false;
					break;
				case STOP_NEG:
					if (!has_label) {label = !sig; has_label = true;}
					if (!(prev_sig != sig && sig == label))
						matcher_stop = false;
					break;
			}
			
			if (!matcher_start && !matcher_ena && !matcher_stop) break;
		}
		
		
		if (matcher_start) {
			if (is_start)		score += 5;
			else				score -= 5;
		}
		if (matcher_stop) {
			if (is_stop)		score += 5;
			else				score -= 5;
		}
		if (matcher_ena) {
			if (ena)			score += 1;
			else				score -= 1;
		}
		
		
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

int Analyzer::GetEvent(const AnalyzerOrder& o, const LabelSource& ls) {
	int e = 0;
	
	if (o.end + 1 >= ls.data.signal.GetCount()) return e;
	
	bool label = o.action;
	bool pre_sig = ls.data.signal.Get(o.begin - 1);
	bool pre_ena = ls.data.enabled.Get(o.begin - 1);
	bool first_sig = ls.data.signal.Get(o.begin);
	bool first_ena = ls.data.enabled.Get(o.begin);
	bool last_sig = ls.data.signal.Get(o.end - 1);
	bool last_ena = ls.data.enabled.Get(o.end - 1);
	bool post_sig = ls.data.signal.Get(o.end);
	bool post_ena = ls.data.enabled.Get(o.end);
	
	if (pre_sig != first_sig) {
		if (first_sig == label)	e |= 1 << START_POS;
		else					e |= 1 << START_NEG;
	}
	if (pre_ena != first_ena) {
		if (first_ena == true)	e |= 1 << START_ENABLE;
		else					e |= 1 << START_DISABLE;
	}
	
	bool sig_sustain = true, ena_sustain = true;
	for (int i = o.begin + 1; i < o.end; i++) {
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







AnalyzerCtrl::AnalyzerCtrl() {
	Add(split.SizePos());
	split.Horz();
	
	split << matcherlist << eventlist;
	split.SetPos(2500);
	
	matcherlist.AddColumn("Symbol");
	matcherlist.AddColumn("Required");
	matcherlist << THISBACK(Data);
	
	eventlist.AddColumn("Cache");
	eventlist.AddColumn("Event");
	eventlist.AddColumn("Probability");
	
}

void AnalyzerCtrl::Data() {
	System& sys = GetSystem();
	Analyzer& a = GetAnalyzer();
	
	for(int i = 0; i < a.matchers.GetCount(); i++) {
		const AnalyzerMatcher& am = a.matchers[i];
		
		matcherlist.Set(i, 0, sys.GetSymbol(am.symbol));
		matcherlist.Set(i, 1, am.GetRequiredString());
	}
	
	int cursor = matcherlist.GetCursor();
	if (cursor >= 0 && cursor < a.matchers.GetCount()) {
		const AnalyzerMatcher& am = a.matchers[cursor];
		
		for(int i = 0; i < am.stats.GetCount(); i++) {
			int key = am.stats.GetKey(i);
			int value = am.stats[i];
			int cache = key / Analyzer::EVENT_COUNT;
			int event  = key % Analyzer::EVENT_COUNT;
			eventlist.Set(i, 0, a.cache[cache].title);
			eventlist.Set(i, 1, a.GetEventString(event));
			eventlist.Set(i, 2, (double)value * 100 / am.total);
		}

		eventlist.SetSortColumn(2, true);
		eventlist.SetCount(am.stats.GetCount());
	}
}

}
