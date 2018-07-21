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
		
		indi_ids.Add().factory = sys.Find<DataBridge>();
		for(int i = 0; i < symbols.GetCount(); i++)
			sym_ids.Add(sys.FindSymbol(symbols[i]));
		tf_ids.Add(2);
		
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
		
		
		
		#if 0
		Vector<ConstBuffer*> open_bufs;
		for(int i = 0; i < work_queue.GetCount() /*&& IsRunning()*/; i++) {
			CoreItem& ci = *work_queue[i];
			sys.Process(ci, true);
			
			Core& c = *ci.core;
			
			if (c.GetFactory() == indi_ids[0].factory && c.GetTf() == tf_ids[0])
				open_bufs.Add(&c.GetBuffer(0));
		}
		
		
		int bars = INT_MAX;
		for(int i = 0; i < open_bufs.GetCount(); i++)
			bars = min(bars, open_bufs[i]->GetCount());
		
		
		VectorMap<int, int> stats;
		for(int i = 1; i < bars; i++) {
			
			int code = 0;
			for(int j = 0; j < open_bufs.GetCount(); j++) {
				ConstBuffer& buf = *open_bufs[j];
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
			for(int j = 0; j < open_bufs.GetCount(); j++) {
				bool b = code & (1 << j);
				s << ".Set(\"" << symbols[j] << "\", " << (b ? "-1" : "+1") << ")";
			}
			s << ";";
			LOG(s);
			
		}
		
		LOG("");
		#endif
		
		
	}
}

}
