#include "Overlook.h"

namespace Overlook {

NetCluster::NetCluster() {
	
}

void NetCluster::Init() {
	System& sys = GetSystem();
	cl.AddSymbol("AUDCAD");
	cl.AddSymbol("AUDJPY");
	cl.AddSymbol("AUDNZD");
	cl.AddSymbol("AUDUSD");
	cl.AddSymbol("CADJPY");
	cl.AddSymbol("CHFJPY");
	cl.AddSymbol("EURAUD");
	cl.AddSymbol("EURCAD");
	cl.AddSymbol("EURCHF");
	cl.AddSymbol("EURGBP");
	cl.AddSymbol("EURJPY");
	cl.AddSymbol("EURUSD");
	cl.AddSymbol("GBPCHF");
	cl.AddSymbol("GBPJPY");
	cl.AddSymbol("GBPUSD");
	cl.AddSymbol("NZDUSD");
	cl.AddSymbol("USDCAD");
	cl.AddSymbol("USDCHF");
	cl.AddSymbol("USDJPY");
	cl.AddTf(0);
	cl.AddIndi(0);
	cl.Init();
}

void NetCluster::Start() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	
	const Index<Time>& ti = dbc.GetTimeIndex();
	
	int data_count = ti.GetCount();
	
	cl.Refresh();
	
	
	VectorMap<int, int> counts;
	
	#if 0
	for(int i = data_count * 0.8; i < data_count; i++) {
		
		int code = 0;
		for(int j = 0; j < cl.GetSymbolCount(); j++) {
			
			double cur = cl.GetBuffer(j, 0, 0).Get(i);
			double prev = cl.GetBuffer(j, 0, 0).Get(i-1);
			
			if (cur < prev)
				code |= 1 << j;
		}
		if (!code) continue;
		
		counts.GetAdd(code, 0)++;
	}
	#else
	for(int i = 0; i < cluster_count; i++) {
		int code = 0;
		for(int j = 0; j < cl.GetSymbolCount(); j++) {
			if (Random(2))
				code |= 1 << j;
		}
		counts.GetAdd(code, 0)++;
	}
	
	#endif
	
	SortByValue(counts, StdGreater<int>());
	
	for(int i = 0; i < cluster_count; i++) {
		String s;
		
		int code = counts.GetKey(i);
		
		s << "AddNet(\"Net" << IntStr(i) << "\")";
		
		for(int j = 0; j < cl.GetSymbolCount(); j++) {
			bool b = code & (1 << j);
			s << ".Set(\"" << cl.GetSymbol(j) << "\", " << (b ? "-1" : "+1") << ")";
		}
		s << ";";
		
		LOG(s);
	}
	LOG("");
}

NetClusterCtrl::NetClusterCtrl() {
	
}

void NetClusterCtrl::Data() {
	
}


}
