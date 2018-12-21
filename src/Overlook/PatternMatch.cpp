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
	cl_sym.AddTf(EventCore::fast_tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
	
	
}

void PatternMatcher::Start() {
	
}

PatternMatcherData& PatternMatcher::RefreshData(int group_step, int period, int average_period) {
	group_step = max(0, min(100, group_step));
	period = max(5, min(1440, period));
	
	int sym_count = cl_sym.GetSymbolCount();
	
	CombineHash ch;
	ch << group_step << 1 << period << 1 << average_period;
	int code = ch;
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
		
		int count = 0;
		for(int j0 = 0; j0 < sym_count; j0++)
			for(int j1 = j0+1; j1 < sym_count; j1++)
				count++;
		d.distance_averages.SetCount(count);
		for(int i = 0; i < count; i++)
			d.distance_averages[i].SetPeriod(average_period);
	}
	
	int count = INT_MAX;
	for(int i = 0; i < sym_count; i++) {
		count = min(count, cl_sym.GetBuffer(i, 0, 0).GetCount());
	}
	//count = min(count, 2000);
	this->count = count;
	
	
	d.data.SetCount(count * sym_count, 0);
	
	Vector<PatternDistance> distances;
	Vector<uint64> descriptors;
	Vector<bool> symbol_added;
	Vector<int> symbol_group;
	
	descriptors.SetCount(sym_count);
	symbol_added.SetCount(sym_count);
	symbol_group.SetCount(sym_count);
	
	for(int i = d.counted; i < count; i++) {
		
		for(int j = 0; j < sym_count; j++) {
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
		for(int j0 = 0; j0 < sym_count; j0++) {
			for(int j1 = j0+1; j1 < sym_count; j1++) {
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
		for(int i = 0; i < distances.GetCount(); i++) {
			OnlineAverageWindow1& av = d.distance_averages[i];
			PatternDistance& d = distances[i];
			av.Add(d.absdist);
			d.absdist = av.GetMean();
		}
		Sort(distances, PatternDistance());
		
		for(int i = 0; i < sym_count; i++) {
			symbol_added[i] = false;
			symbol_group[i] = -1;
		}
		int group_count = 0;
		
		for(int j = 0; j < distances.GetCount(); j++) {
			PatternDistance& d = distances[j];
			
			if (d.absdist > group_step) break;
			
			if (symbol_added[d.j0] && symbol_added[d.j1])
				continue;
			
			bool added = false;
			for(int k = 0; k < group_count; k++) {
				for(int sym = 0; sym < sym_count; sym++) {
					if (symbol_group[sym] != k)
						continue;
					
					if (d.j0 == sym && symbol_added[d.j1] == false) {
						symbol_group[d.j1] = k;
						symbol_added[d.j1] = true;
						added = true;
						break;
					}
					else if (d.j1 == sym && symbol_added[d.j0] == false) {
						symbol_group[d.j0] = k;
						symbol_added[d.j0] = true;
						added = true;
						break;
					}
				}
			}
			
			if (!added) {
				int group_id = group_count++;
				symbol_group[d.j0] = group_id;
				symbol_group[d.j1] = group_id;
				symbol_added[d.j0] = true;
				symbol_added[d.j1] = true;
			}
		}
		
		
		for(int j = 0; j < sym_count; j++) {
			if (!symbol_added[j]) {
				int group_id = group_count++;
				symbol_group[j] = group_id;
			}
			d.data[i * sym_count + j] = symbol_group[j];
		}
	}
	
	
	d.counted = count;
	
	return d;
}



PatternMatcherCtrl::PatternMatcherCtrl() {
	Add(period_lbl.TopPos(0, 30).LeftPos(0, 100));
	Add(period.TopPos(0, 30).LeftPos(100, 100));
	Add(group_step_lbl.TopPos(0, 30).LeftPos(200, 100));
	Add(group_step.TopPos(0, 30).LeftPos(300, 100));
	Add(average_period_lbl.TopPos(0, 30).LeftPos(400, 100));
	Add(average_period.TopPos(0, 30).LeftPos(500, 100));
	Add(slider.TopPos(0, 30).HSizePos(600, 200));
	Add(date.TopPos(0, 30).RightPos(0, 200));
	Add(list.HSizePos().VSizePos(30));
	
	period_lbl.SetLabel("Pattern size:");
	group_step_lbl.SetLabel("Group limit:");
	average_period_lbl.SetLabel("Average:");
	period.MinMax(0, 1440*5);
	period.SetData(20);
	period <<= THISBACK(Data);
	group_step.MinMax(0, 100);
	group_step.SetData(20);
	group_step <<= THISBACK(Data);
	average_period.MinMax(0, 1440*5);
	average_period.SetData(20);
	average_period <<= THISBACK(Data);
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
	int sym_count = pm.cl_sym.GetSymbolCount();
	
	const Index<Time>& idx = dbc.GetTimeIndex(EventCore::fast_tf);
	
	if (slider.GetMax() != pm.count && pm.count > 0)
		slider.MinMax(0, pm.count);
	
	int pos = slider.GetData();
	
	date.SetLabel(Format("%", idx[pos]));
	
	PatternMatcherData& data = pm.RefreshData(group_step.GetData(), period.GetData(), average_period.GetData());
	
	int data_begin = pos * sym_count;
	if (data_begin >= data.data.GetCount())
		return;
	
	VectorMap<int, Vector<int> > groups;
	for(int i = 0; i < sym_count; i++) {
		groups.GetAdd(data.data[data_begin + i]).Add(i);
	}
	SortByKey(groups, StdLess<int>());
	
	for(int i = 0; i < groups.GetCount(); i++) {
		Vector<int>& vec = groups[i];
		list.Set(i, 0, i);
		String s;
		for(int j = 0; j < vec.GetCount(); j++) {
			String sym = net.symbols.GetKey(vec[j]);
			if (j) s += ", ";
			s += sym;
		}
		list.Set(i, 1, s);
	}
	list.SetCount(groups.GetCount());
	
}

}
