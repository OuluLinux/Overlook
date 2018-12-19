#include "Overlook.h"

namespace Overlook {



EventAutomation::EventAutomation() {
	
}

void EventAutomation::Init() {
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	
	for(int i = 0; i < 6; i++) {
		int id = net.symbol_ids.GetKey(i);
		
		cl_sym.AddSymbol(net.symbols.GetKey(i));
		
		for(int j = 0; j < sys.EventCoreFactories().GetCount(); j++) {
			EventCore& core = *sys.EventCoreFactories()[j].c();
			
			ArgEvent args;
			core.Arg(args);
			
			if (args.mins.IsEmpty()) {
				FactoryDeclaration decl;
				decl.factory = j;
				sys.GetEventCoreQueue(ci_queue, id, decl);
			} else {
				Vector<int> arg_values;
				arg_values <<= args.mins;
				
				while (true) {
					FactoryDeclaration decl;
					decl.factory = j;
					for(int i = 0; i < arg_values.GetCount(); i++)
						decl.AddArg(arg_values[i]);
					
					sys.GetEventCoreQueue(ci_queue, id, decl);
					
					bool finish = false;
					for(int k = 0; k < arg_values.GetCount(); k++) {
						int& a = arg_values[k];
						a += args.steps[k];
						if (a > args.maxs[k]) {
							a = args.mins[k];
							if (k == arg_values.GetCount()-1) {
								finish = true;
								break;
							}
						}
						else
							break;
					}
					if (finish) break;
				}
			}
		}
	}
	
	
	cl_sym.AddTf(EventCore::fast_tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
	
	
	LoadThis();
	
	if (data.GetCount() && data.GetCount() != ci_queue.GetCount()) {
		data.Clear();
		counted = 0;
	}
	
	data.SetCount(ci_queue.GetCount());
	
}

void EventAutomation::Start() {
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	
	cl_sym.Refresh();
	
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		EventCore& c = *ci_queue[i]->core;
		VectorMap<int, OnlineAverage1>& d = data[i];
		
		int sym = net.symbol_ids.Find(c.GetSymbol());
		ConstBuffer& buf = cl_sym.GetBuffer(sym, 0, 0);
		
		int data_end = buf.GetCount() * 0.5;
		
		
		for(int j = counted; j < data_end; j++) {
			int output = 0;
			
			c.Start(j, output);
			
			if (output) {
				int first_sig = GetSignalWhichFirst(sym, j);
				if (first_sig != -1)
					d.GetAdd(output).Add(first_sig);
				
			}
		}
		
	}
	
	int count = cl_sym.GetBuffer(0, 0, 0).GetCount();
	int prev_counted = counted;
	counted = count;
	if (prev_counted != counted) {
		StoreThis();
	}
	
}

int EventAutomation::GetSignalWhichFirst(int sym, int pos) {
	ConstBuffer& buf = cl_sym.GetBuffer(sym, 0, 0);
	
	double point = cl_sym.GetDataBridge(sym)->GetPoint();
	
	double open = buf.Get(pos);
	double low_target  = open - pips_first * point;
	double high_target = open + pips_first * point;
	
	for(int i = pos + 1; i < buf.GetCount(); i++) {
		double cur = buf.Get(i);
		
		if (cur <= low_target) {
			return 1;
		}
		else if (cur >= high_target) {
			return 0;
		}
	}
	return -1;
}

void EventAutomation::Process() {
	
}



EventAutomationCtrl::EventAutomationCtrl() {
	Add(slider.TopPos(0, 30).HSizePos(0, 200));
	Add(date.TopPos(0, 30).RightPos(0, 200));
	Add(list.HSizePos().VSizePos(30));
	
	slider.MinMax(0,1);
	slider.SetData(0);
	slider <<= THISBACK(Data);
	
	list.AddColumn("Symbol");
	list.AddColumn("What");
	list.AddColumn("Signal");
	list.AddColumn("First low probability");
	list.AddColumn("First low Confidence");
}

void EventAutomationCtrl::Data() {
	EventAutomation& ea = GetEventAutomation();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	System& sys = GetSystem();
	
	const Index<Time>& idx = dbc.GetTimeIndex(EventCore::fast_tf);
	
	if (slider.GetMax() != ea.counted && ea.counted > 0)
		slider.MinMax(0, ea.counted);
	
	int pos = slider.GetData();
	
	date.SetLabel(Format("%", idx[pos]));
	
	int rows = 0;
	for(int i = 0; i < ea.ci_queue.GetCount(); i++) {
		EventCore& c = *ea.ci_queue[i]->core;
		VectorMap<int, OnlineAverage1>& d = ea.data[i];
		
		int output = 0;
		c.GetCoreList().Refresh();
		c.Start(pos, output);
		
		if (output) {
			double firstprob = d.GetAdd(output).GetMean();
			
			list.Set(rows, 0, sys.GetSymbol(c.GetSymbol()));
			list.Set(rows, 1, c.GetTitle());
			list.Set(rows, 2, output);
			list.Set(rows, 3, firstprob);
			list.Set(rows, 4, fabs(firstprob - 0.5));
			rows++;
		}
	}
	list.SetCount(rows);
	list.SetSortColumn(4, true);
}

}