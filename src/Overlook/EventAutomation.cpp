#include "Overlook.h"

namespace Overlook {

EventAutomation::EventAutomation() {
	
	
}

EventAutomation::~EventAutomation() {
	running = false;
	while (!stopped) Sleep(100);
}
	
void EventAutomation::Data() {
	System& sys = GetSystem();
	
	if (work_queue.IsEmpty()) {
		TimeStop ts;
		
		int sym_count = sys.GetNormalSymbolCount();
		int cur_count = sys.GetCurrencyCount();
		int net_count = sys.GetNetCount();
		
		fac_ids.Add(sys.Find<DataBridge>());
		fac_ids.Add(sys.Find<BollingerBands>());
		for(int i = 0; i < fac_ids.GetCount(); i++)
			indi_ids.Add().factory = fac_ids[i];
		for(int i = 0; i < cur_count; i++) {
			int j = sym_count + i;
			sym_ids.Add(j);
			symbols.Add(sys.GetSymbol(j));
		}
		tf_ids.Add(0);
		
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
		
		
		bufs.SetCount(fac_ids.GetCount());
		for(int i = 0; i < bufs.GetCount(); i++) {
			bufs[i].SetCount(sym_ids.GetCount());
		}
		
		
		for(int i = 0; i < work_queue.GetCount() /*&& IsRunning()*/; i++) {
			CoreItem& ci = *work_queue[i];
			sys.Process(ci, true);
			
			Core& c = *ci.core;
			
			int faci = fac_ids.Find(c.GetFactory());
			int symi = sym_ids.Find(c.GetSymbol());
			int tfi = tf_ids.Find(c.GetTf());
			
			if (symi == -1) continue;
			if (tfi == -1) continue;
			
			auto& v = bufs[faci][symi];
			v.SetCount(c.GetBufferCount());
			for(int j = 0; j < c.GetBufferCount(); j++) {
				v[j] = &c.GetBuffer(j);
			}
		}
		
		
		ReleaseLog("EventAutomation work queue init took " + ts.ToString());
		
		//PrintNetCode();
	}
	
	
	for(int i = 0; i < work_queue.GetCount() /*&& IsRunning()*/; i++)
		sys.Process(*work_queue[i], true);
	
	UpdateEvents();
}


void EventAutomation::UpdateEvents() {
	System& sys = GetSystem();
	
	
	
	VectorMap<String, int> auto_sig;
	
	for(int i = 0; i < sym_ids.GetCount() - 1; i++) {
		ConstBuffer& lo_buf = *bufs[OPEN][i][1];
		ConstBuffer& hi_buf = *bufs[OPEN][i][2];
		ConstBuffer& ma_buf = *bufs[BB][i][0];
		ConstBuffer& tl_buf = *bufs[BB][i][1];
		ConstBuffer& bl_buf = *bufs[BB][i][2];
		int count = min(lo_buf.GetCount(), ma_buf.GetCount());
		int sig = 0;
		for (int j = count - 1; j >= 0; j--) {
			double ma = ma_buf.Top();
			double l = lo_buf.Top();
			double h = hi_buf.Top();
			double tl = tl_buf.Top();
			double bl = bl_buf.Top();
			if (h >= tl) {
				sig = +1;
				break;
			}
			else if (l <= bl) {
				sig = -1;
				break;
			}
		}
		double ma0 = ma_buf.Get(count - 1);
		double ma1 = ma_buf.Get(count - 2);
		if (ma0 > ma1) sig++;
		else if (ma0 < ma1) sig--;
		auto_sig.GetAdd(symbols[i], 0) += sig;
	}
	
	Swap(this->auto_sig, auto_sig);
	
}


}