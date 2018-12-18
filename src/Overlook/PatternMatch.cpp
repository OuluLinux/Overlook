#include "Overlook.h"

namespace Overlook {

PatternMatcher::PatternMatcher() {
	
}

void PatternMatcher::Init() {
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(0);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
	
	
}

void PatternMatcher::Start() {
	
}

PatternMatcherData& PatternMatcher::RefreshData(int group_step, int period) {
	group_step = max(0, min(100, group_step));
	period = max(5, min(1440, period));
	
	int code = period * 100 + group_step;
	PatternMatcherData& d = data.GetAdd(code);
	
	if (d.pattern.IsEmpty()) {
		for(int i = 0; i < 64; i++) {
			int a = Random(period);
			int b;
			do {
				b = Random(period);
			}
			while (a == b);
			d.pattern.Add(Point(a, b));
		}
	}
	
	int count = INT_MAX;
	for(int i = 0; i < cl_sym.GetSymbolCount(); i++) {
		count = min(count, cl_sym.GetBuffer(i, 0, 0).GetCount());
	}
	count = min(count, 2000);
	this->count = count;
	
	
	d.data.SetCount(count);
	
	Vector<uint64> descriptors;
	descriptors.SetCount(cl_sym.GetSymbolCount());
	Vector<PatternDistance> distances;
	for(int i = d.counted; i < count; i++) {
		
		for(int j = 0; j < cl_sym.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_sym.GetBuffer(j, 0, 0);
			uint64 desc = 0;
			for(int k = 0; k < d.pattern.GetCount(); k++) {
				const Point& p = d.pattern[k];
				double a = buf.Get(max(0, i - p.x));
				double b = buf.Get(max(0, i - p.y));
				if (a < b)
					desc |= 1 << k;
			}
			descriptors[j] = desc;
		}
		
		distances.SetCount(0);
		for(int j0 = 0; j0 < cl_sym.GetSymbolCount(); j0++) {
			for(int j1 = j0+1; j1 < cl_sym.GetSymbolCount(); j1++) {
				uint64 a = descriptors[j0];
				uint64 b = descriptors[j1];
				int dist = PopCount64(a ^ b);
				PatternDistance& d = distances.Add();
				d.j0 = j0;
				d.j1 = j1;
				d.dist = dist * 100 / 64;
				d.absdist = d.dist >= 50 ? 100 - (d.dist - 50) * 2 : d.dist * 2;
			}
		}
		Sort(distances, PatternDistance());
		
		Index<int> added_symbols;
		Vector<PatternGroup>& groups = d.data[i];
		for(int j = 0; j < distances.GetCount(); j++) {
			PatternDistance& d = distances[j];
			
			if (d.absdist > group_step) break;
			
			if (added_symbols.Find(d.j0) != -1 && added_symbols.Find(d.j1) != -1)
				continue;
			
			bool added = false;
			for(int k = 0; k < groups.GetCount(); k++) {
				PatternGroup& pg = groups[k];
				for(int l = 0; l < pg.symbols.GetCount(); l++) {
					int sym = pg.symbols[l];
					if (d.j0 == sym && added_symbols.Find(d.j1) == -1) {
						pg.symbols.Add(d.j1);
						added_symbols.Add(d.j1);
						added = true;
						break;
					}
					else if (d.j1 == sym && added_symbols.Find(d.j0) == -1) {
						pg.symbols.Add(d.j0);
						added_symbols.Add(d.j0);
						added = true;
						break;
					}
				}
			}
			
			if (!added) {
				PatternGroup& pg = groups.Add();
				pg.symbols.Add(d.j0);
				pg.symbols.Add(d.j1);
				added_symbols.Add(d.j0);
				added_symbols.Add(d.j1);
			}
		}
		
		
		for(int j = 0; j < cl_sym.GetSymbolCount(); j++) {
			if (added_symbols.Find(j) != -1) continue;
			
			PatternGroup& pg = groups.Add();
			pg.symbols.Add(j);
		}
	}
	
	
	d.counted = count;
	
	return d;
}



PatternMatcherCtrl::PatternMatcherCtrl() {
	Add(period.TopPos(0, 30).LeftPos(0, 100));
	Add(group_step.TopPos(0, 30).LeftPos(100, 100));
	Add(slider.TopPos(0, 30).HSizePos(200, 200));
	Add(date.TopPos(0, 30).RightPos(0, 200));
	Add(list.HSizePos().VSizePos(30));
	
	period.MinMax(0, 100);
	period.SetData(20);
	period <<= THISBACK(Data);
	group_step.MinMax(0, 1440);
	group_step.SetData(20);
	group_step <<= THISBACK(Data);
	slider.MinMax(0,1);
	slider.SetData(0);
	slider <<= THISBACK(Data);
	
	list.AddColumn("#");
	list.AddColumn("Symbols");
}
	
void PatternMatcherCtrl::Data() {
	PatternMatcher& pm = GetPatternMatcher();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	
	const Index<Time>& idx = dbc.GetTimeIndex(0);
	
	if (slider.GetMax() != pm.count && pm.count > 0)
		slider.MinMax(0, pm.count);
	
	int pos = slider.GetData();
	
	date.SetLabel(Format("%", idx[pos]));
	
	PatternMatcherData& data = pm.RefreshData(group_step.GetData(), period.GetData());
	
	if (pos >= data.data.GetCount())
		return;
	
	Vector<PatternGroup>& pgv = data.data[pos];
	for(int i = 0; i < pgv.GetCount(); i++) {
		PatternGroup& pg = pgv[i];
		list.Set(i, 0, i);
		String s;
		for(int j = 0; j < pg.symbols.GetCount(); j++) {
			String sym = net.symbols.GetKey(pg.symbols[j]);
			if (j) s += ", ";
			s += sym;
		}
		list.Set(i, 1, s);
	}
	list.SetCount(pgv.GetCount());
	
}

}
