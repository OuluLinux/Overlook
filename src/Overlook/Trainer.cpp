#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Trainer::Trainer(System& sys) : sys(&sys) {
	
}

void Trainer::Init() {
	int sym, tf, indi;
	
	tf = sys->FindPeriod(10080);
	ASSERT(tf != -1);
	tf_ids.Add(tf);
	tf = sys->FindPeriod(1440);
	ASSERT(tf != -1);
	tf_ids.Add(tf);
	tf = sys->FindPeriod(240);
	ASSERT(tf != -1);
	tf_ids.Add(tf);
	/*tf = sys->FindPeriod(30);
	ASSERT(tf != -1);
	tf_ids.Add(tf);*/
	
	
	MetaTrader& mt = GetMetaTrader();
	if (mt.AccountCurrency() != "USD")
		Panic("Only USD is allowed currency");
	
	for(int i = 0; i < mt.GetSymbolCount(); i++) {
		const Symbol& sym = mt.GetSymbol(i);
		// Skip symbols with proxy
		if (sym.proxy_id != -1) continue;
		sym_ids.Add(i);
	}
	
	int basket_begin = mt.GetSymbolCount() + mt.GetCurrencyCount();
	for(int i = basket_begin; i < sys->GetTotalSymbolCount(); i++) {
		sym_ids.Add(i);
	}
	
	indi = sys->Find<MovingAverageConvergenceDivergence>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	indi = sys->Find<StandardDeviation>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	indi = sys->Find<CommodityChannelIndex>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	indi = sys->Find<WilliamsPercentRange>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
}

void Trainer::RefreshWorkQueue() {
	sys->GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
}

void Trainer::ProcessWorkQueue() {
	for(int i = 0; i < work_queue.GetCount(); i++) {
		LOG(i << "/" << work_queue.GetCount());
		sys->WhenProgress(i, work_queue.GetCount());
		sys->Process(*work_queue[i]);
	}
}

}
