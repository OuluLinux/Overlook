#include "Overlook.h"

namespace Overlook {

CostAvoidance::CostAvoidance() {
	
	
	System& sys = GetSystem();
	sym_id_str.Add("EURUSD");
	sym_id_str.Add("GBPUSD");
	sym_id_str.Add("USDJPY");
	sym_id_str.Add("EURGBP");
	sym_id_str.Add("EURJPY");
	
	sym_id_str.Add("JPY");
	sym_id_str.Add("USD");
	sym_id_str.Add("EUR");
	//sym_id_str.Add("GBP");
	
	for(int i = 0; i < sym_id_str.GetCount(); i++)
		sym_ids.Add(sys.FindSymbol(sym_id_str[i]));
	
	tf_ids.Add(1);
	
	
	for(int i = 1; i <= 7; i++)
		for(int method = 0; method < 2; method++)
			Add<MovingAverage>(2 << i, 0, method);
	Add<MovingAverageConvergenceDivergence>(12, 26, 9);
	Add<MovingAverageConvergenceDivergence>(24, 52, 9);
	Add<MovingAverageConvergenceDivergence>(48, 104, 9);
	Add<ParabolicSAR>(20, 10);
	Add<ParabolicSAR>(40, 10);
	Add<ParabolicSAR>(20, 20);
	Add<ParabolicSAR>(40, 20);
	for(int i = 3; i < 6; i++) {
		int period = 2 << i;
		Add<StochasticOscillator>(period);
	}
	
	db_cores.SetCount(sym_ids.GetCount(), NULL);
	
	LoadThis();
	
	Thread::Start(THISBACK(Process));
}

CostAvoidance::~CostAvoidance() {
	running = false;
	while (!stopped) Sleep(100);
}

void CostAvoidance::Process() {
	running = true;
	stopped = false;
	
	
	if (symbols.IsEmpty()) {
		FillInputBooleans();
		FillChances();
		
		/*
		CoWork co;
		co.SetPoolSize(GetUsedCpuCores());
		for(int i = 0; i < symbols.GetCount(); i++) {
			co & symbols[i].InitCb();
		}
		co.Finish();
		*/
		
		
		
		if (IsRunning())
			StoreThis();
	}
	
	
	/*
	while (IsRunning()) {
		FillInputBooleans();
		
		for(int i = 0; i < 10 && IsRunning(); i++)
			Sleep(1000);
	}*/
	
	stopped = true;
}

void CostAvoidance::FillInputBooleans() {
	System& sys = GetSystem();
	
	if (work_queue.IsEmpty()) {
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	}
	
	int row = 0;
	
	for(int i = 0; i < work_queue.GetCount() && IsRunning(); i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
		
		Core& c = *ci.core;
		if (c.GetTf() != tf_ids[0])
			continue;
		
		if (c.GetFactory() == 0) {
			int l = sym_ids.Find(c.GetSymbol());
			if (l >= 0)
				db_cores[l] = dynamic_cast<DataBridge*>(&c);
		}
		
		for(int j = 0; j < c.GetLabelCount(); j++) {
			ConstLabel& l = c.GetLabel(j);
			
			for(int k = 0; k < l.buffers.GetCount(); k++) {
				ConstLabelSignal& src = l.buffers[k];
				
				if (row >= cache.GetCount())
					cache.SetCount(row+1);
				
				LabelSource& ls = cache[row++];
				
				if (ls.factory == -1) {
					ls.symbol = c.GetSymbol();
					ls.factory = c.GetFactory();
					ls.label = j;
					ls.buffer = k;
					ls.data = src;
					ls.title = sys.GetCoreFactories()[ls.factory].a + " #" + IntStr(k) + " ";
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

void CostAvoidance::FillChances() {
	System& sys = GetSystem();
	
	if (chances.IsEmpty()) {
		ASSERT(!cache.IsEmpty());
		
		for(int i = 0; i < cache.GetCount(); i++) {
			LabelSource& ls = cache[i];
			
			String symstr = sys.GetSymbol(ls.symbol);
			if (symstr.GetCount() < 6)
				continue;
			String A = symstr.Left(3);
			String B = symstr.Right(3);
			
			Chance& c1 = chances.Add();
			Chance& cA = chances.Add();
			Chance& cB = chances.Add();
			Chance& cAB = chances.Add();
			
			c1.caches.Add(i);
			c1.symbol = ls.symbol;
			c1.type = 0;
			c1.desc = ls.title;
			
			cA.desc += symstr + " ";
			cB.desc += symstr + " ";
			
			for(int j = 0; j < cache.GetCount(); j++) {
				if (i == j) continue;
				LabelSource& ls2 = cache[j];
				if (ls2.factory != ls.factory || ls2.label != ls.label || ls2.buffer != ls.buffer || ls2.title != ls.title) continue;
				
				String symstr2 = sys.GetSymbol(ls2.symbol);
				if (symstr2.GetCount() == 6) {
					String A2 = symstr2.Left(3);
					String B2 = symstr2.Right(3);
					
					if (A2 == A) {
						cA.caches.Add(j);
						cA.desc += symstr2 + " ";
					}
					else if (B2 == A) {
						cA.caches.Add(-j-1);
						cA.desc += symstr2 + " ";
					}
					
					if (A2 == B) {
						cB.caches.Add(-j-1);
						cB.desc += symstr2 + " ";
					}
					else if (B2 == B) {
						cB.caches.Add(j);
						cB.desc += symstr2 + " ";
					}
				}
				else {
					if (A == symstr2) {
						cAB.caches.Add(j);
						cAB.desc += symstr2 + " ";
					}
					else if (B == symstr2) {
						cAB.caches.Add(-j-1);
						cAB.desc += symstr2 + " ";
					}
				}
			}
			
			cA.symbol = ls.symbol;
			cA.type = 1;
			cA.desc += ls.title;
			
			cB.symbol = ls.symbol;
			cB.type = 2;
			cB.desc += ls.title;
			
			cAB.symbol = ls.symbol;
			cAB.type = 3;
			cAB.desc += ls.title;
			
		}
		
	}
	
	
	for(int i = 0; i < chances.GetCount(); i++) {
		Chance& ch = chances[i];
		
		int k = sym_ids.Find(ch.symbol);
		ConstBuffer& open_buf = db_cores[k]->GetBuffer(0);
		int pos = open_buf.GetCount()-1;
		
		int id = ch.caches[0];
		bool type;
		if (id >= 0)
			type = cache[id].data.signal.Get(pos);
		else
			type = !cache[-id-1].data.signal.Get(pos);
		ch.action = type;
		
		int j = pos;
		for (; j > 0; j--) {
			bool same = true;
			for(int k = 0; k < ch.caches.GetCount() && same; k++) {
				int id = ch.caches[k];
				if (id >= 0) {
					LabelSource& c = cache[id];
					if (c.data.signal.Get(j) != type)
						same = false;
				} else {
					LabelSource& c = cache[-id-1];
					if (c.data.signal.Get(j) == type)
						same = false;
				}
			}
			
			if (!same)
				break;
		}
		ch.len = pos - j;
		
		double point = db_cores[k]->GetPoint();
		double begin = open_buf.Get(j);
		double end = open_buf.Get(pos);
		ch.pips = (end - begin) / point;
		if (type) ch.pips *= -1;
	}
}







CostAvoidanceCtrl::CostAvoidanceCtrl() {
	Add(currentlist.SizePos());
	
	currentlist.AddColumn("Symbol");
	currentlist.AddColumn("Type");
	currentlist.AddColumn("Reason");
	currentlist.AddColumn("Length");
	currentlist.AddColumn("Change");
	currentlist.AddColumn("AbsChange");
	currentlist.ColumnWidths("2 1 4 1 1 1");
}

void CostAvoidanceCtrl::Data() {
	CostAvoidance& ca = GetCostAvoidance();
	System& sys = GetSystem();
	
	int row = 0;
	for(int i = 0; i < ca.chances.GetCount(); i++) {
		const Chance& ch = ca.chances[i];
		if (ch.len == 0 || ch.pips == 0)
			continue;
		
		String symstr = sys.GetSymbol(ch.symbol);
		currentlist.Set(row, 0, symstr);
		currentlist.Set(row, 1, ch.action ? "Sell" : "Buy");
		currentlist.Set(row, 2, ch.desc);
		currentlist.Set(row, 3, ch.len);
		currentlist.Set(row, 4, ch.pips);
		currentlist.Set(row, 5, abs(ch.pips));
		
		row++;
	}
	currentlist.SetCount(row);
	currentlist.SetSortColumn(5, true);
}

}
