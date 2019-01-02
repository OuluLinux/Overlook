#include "Overlook.h"

namespace Overlook {

void LoadSymbol(CoreList& cl_sym, int symbol, int tf) {
	System& sys = GetSystem();
	
	System::NetSetting& net = sys.GetNet(0);
	cl_sym.AddSymbol(sys.GetSymbol(symbol));
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
}

void LoadNetSymbols(CoreList& cl_sym, int tf) {
	System& sys = GetSystem();
	
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
}


void LoadNets(CoreList& cl_net, int tf) {
	System& sys = GetSystem();
	
	for(int i = 0; i < sys.GetNetCount(); i++) {
		cl_net.AddSymbol("Net" + IntStr(i));
	}
	cl_net.AddTf(tf);
	cl_net.AddIndi(0);
	cl_net.Init();
	cl_net.Refresh();
}

void LoadDataPriceInput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int windowsize) {
	ConvNet::SessionData& d = ses.GetData();
	
	for(int i = 0; i < count; i++) {
		int pos = begin + i;
		
		int col = 0;
		for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			double next = buf.Get(pos);
			for(int j = 0; j < windowsize; j++) {
				int pos2 = max(0, pos - j - 1);
				double cur = buf.Get(pos2);
				double ch = ((next / cur) - 1.0) * 1000;
				d.SetData(i, col++, ch);
				next = cur;
			}
		}
	}
}

void LoadDataPipOutput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int postpips_count) {
	ConvNet::SessionData& d = ses.GetData();
	
	for(int i = 0; i < count; i++) {
		int pos = begin + i;
		
		for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			ConstBuffer& lowbuf = cl_net.GetBuffer(j, 0, 1);
			ConstBuffer& highbuf = cl_net.GetBuffer(j, 0, 2);
			double point = cl_net.GetDataBridge(j)->GetPoint();
			double open = buf.Get(pos);
			double lo = open - postpips_count * point;
			double hi = open + postpips_count * point;
			
			bool result = false;
			
			for(int k = pos+1; k < buf.GetCount(); k++) {
				double low  = lowbuf.Get(k-1);
				double high = highbuf.Get(k-1);
				if (low <= lo) {
					result = true;
					break;
				}
				else if (high >= hi) {
					result = false;
					break;
				}
			}
			
			d.SetResult(i, j, result ? 1.0 : 0.0);
		}
	}
}

void LoadDataVolatOutput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int ticks) {
	ConvNet::SessionData& d = ses.GetData();
	
	for(int i = 0; i < count; i++) {
		int pos = begin + i;
		
		for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			
			double open = buf.Get(pos);
			double close = buf.Get(min(buf.GetCount()-1, pos + ticks));
			double ch = ((close / open) - 1.0) * 1000;
			ch = fabs(ch);
			
			d.SetResult(i, j, ch);
		}
	}
}

void TrainSession(ConvNet::Session& ses, int iterations) {
	if (ses.GetStepCount() < iterations) {
		ses.StartTraining();
		while (ses.GetStepCount() < iterations) Sleep(100);
		ses.StopTraining();
	}
}

}
