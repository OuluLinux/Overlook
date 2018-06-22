#include "Overlook.h"


namespace Overlook {





void ScalperSymbol::Init() {
	
}























Scalper::Scalper() {
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

Scalper::~Scalper() {
	running = false;
	while (!stopped) Sleep(100);
}

void Scalper::Process() {
	running = true;
	stopped = false;
	
	
	if (symbols.IsEmpty()) {
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
		
		for(int i = 0; i < 10 && IsRunning(); i++)
			Sleep(1000);
	}
	
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
	
	conflist.AddColumn("Cluster");
	conflist.AddColumn("Orders");
	conflist << THISBACK(Data);
	
	sustsplit << startlist << sustlist;
	
	startlist.AddColumn("Cache");
	startlist.AddColumn("Event");
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
				//conflist.Set(i, 1, am.orders.GetCount());
			}
			conflist.SetCount(as.confs.GetCount());
			
			int cursor = conflist.GetCursor();
			if (cursor >= 0 && cursor < as.confs.GetCount()) {
				a.sel_sym = symcursor;
				a.sel_cluster = cursor;
				
				const ScalperConf& am = as.confs[cursor];
				int row = 0;
				
				/*if (tab == 0) {
					for(int i = 0; i < am.stats.GetCount(); i++) {
						int key = am.stats.GetKey(i);
						int value = am.stats[i];
						int cache = key / EVENT_COUNT;
						int event  = key % EVENT_COUNT;
						//if (event >= Scalper::SUSTAIN_ENABLE) continue;
						eventlist.Set(row, 0, a.cache[cache].title);
						eventlist.Set(row, 1, GetEventString(event));
						eventlist.Set(row, 2, (double)value * 100 / am.total);
						row++;
					}
			
					eventlist.SetSortColumn(2, true);
					eventlist.SetCount(row);
				}
				else if (tab == 1) {
					for(int i = 0; i < am.full_list.GetCount(); i++) {
						sustandlist.Set(i, 0, i);
					}
					sustandlist.SetCount(am.full_list.GetCount());
					
					int sustandcursor = sustandlist.GetCursor();
					if (sustandcursor >= 0 && sustandcursor < am.full_list.GetCount()) {
						const Vector<MatcherItem>& sust = am.full_list[sustandcursor];
						for(int i = 0; i < sust.GetCount(); i++) {
							const MatcherItem& mi = sust[i];
							sustlist.Set(row, 0, a.cache[mi.cache].title);
							sustlist.Set(row, 1, GetEventString(mi.event));
							row++;
						}
						sustlist.SetCount(sust.GetCount());
					}
				}*/
			}
		}
	}
}


}
