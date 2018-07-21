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
		
		fac_ids.Add(sys.Find<DataBridge>());
		fac_ids.Add(sys.Find<SimpleHurstWindow>());
		fac_ids.Add(sys.Find<MovingAverage>());
		fac_ids.Add(sys.Find<BollingerBands>());
		fac_ids.Add(sys.Find<VolatilityAverage>());
		fac_ids.Add(sys.Find<VolatilitySlots>());
		fac_ids.Add(sys.Find<PeriodicalChange>());
		fac_ids.Add(sys.Find<Calendar>());
		
		for(int i = 0; i < fac_ids.GetCount(); i++)
			indi_ids.Add().factory = fac_ids[i];
		for(int i = 0; i < symbols.GetCount(); i++)
			sym_ids.Add(sys.FindSymbol(symbols[i]));
		tf_ids.Add(0);
		tf_ids.Add(1);
		tf_ids.Add(2);
		tf_ids.Add(4);
		tf_ids.Add(5);
		
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
		
		
		bufs.SetCount(BUF_COUNT);
		for(int i = 0; i < bufs.GetCount(); i++) {
			bufs[i].SetCount(symbols.GetCount());
			for(int j = 0; j < bufs[i].GetCount(); j++)
				bufs[i][j].SetCount(tf_ids.GetCount(), NULL);
		}
		cals.SetCount(symbols.GetCount(), NULL);
		
		
		for(int i = 0; i < work_queue.GetCount() /*&& IsRunning()*/; i++) {
			CoreItem& ci = *work_queue[i];
			sys.Process(ci, true);
			
			Core& c = *ci.core;
			
			int faci = fac_ids.Find(c.GetFactory());
			int symi = sym_ids.Find(c.GetSymbol());
			int tfi = tf_ids.Find(c.GetTf());
			
			if (c.GetBufferCount())
				bufs[faci][symi][tfi] = &c.GetBuffer(0);
			
			if (faci == CAL)
				cals[symi] = dynamic_cast<Calendar*>(&c);
		}
		
		
	
		
		
		LOG("");
		//PrintNetCode();
	}
}

void EventSystem::PrintNetCode() {
	int tfi = 2;
	
	int bars = INT_MAX;
	for(int i = 0; i < sym_ids.GetCount(); i++)
		bars = min(bars, bufs[OPEN][i][tfi]->GetCount());
	
	
	VectorMap<int, int> stats;
	for(int i = 1; i < bars; i++) {
		
		int code = 0;
		for(int j = 0; j < sym_ids.GetCount(); j++) {
			ConstBuffer& buf = *bufs[OPEN][j][tfi];
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
		for(int j = 0; j < sym_ids.GetCount(); j++) {
			bool b = code & (1 << j);
			s << ".Set(\"" << symbols[j] << "\", " << (b ? "-1" : "+1") << ")";
		}
		s << ";";
		LOG(s);
		
	}
	
	LOG("");
}

}
