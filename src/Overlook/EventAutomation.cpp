#include "Overlook.h"

namespace Overlook {



EventAutomation::EventAutomation() {
	prev_update = Time(1970,1,1);
	
}

void EventAutomation::Init() {
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	
	for(int i = 0; i < net.symbols.GetCount(); i++) {
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
	
	
	int sym_count = cl_sym.GetSymbolCount();
	symbol_simple_datas.SetCount(sym_count);
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		EventCore& ec = *ci_queue[i]->core;
		int sym_id = ec.GetSymbol();
		int sym = net.symbol_ids.Find(sym_id);;
		symbol_simple_datas[sym].Add(i);
	}
	
	
	LoadThis();
	
	if (data.GetCount() && data.GetCount() != ci_queue.GetCount()) {
		group_data.Clear();
		current_groups.Clear();
		unique_groups.Clear();
		data.Clear();
		simple_data.Clear();
		group_counted.Clear();
		counted = 100;
	}
	
	data.SetCount(ci_queue.GetCount());
	simple_data.SetCount(ci_queue.GetCount());
	
	
	RefreshGroups();
	StoreThis();
}

void EventAutomation::RefreshGroups() {
	PatternMatcher& pm = GetPatternMatcher();
	Vector<int> periods;
	Array<CombineHash> ch;
	int sym_count = pm.GetSymbolCount();
	ch.SetCount(sym_count);
	for(int i = 3; i < 13; i++) {
		int period = 1 << i;
		for(int group_step = 20; group_step <= 60; group_step+=20) {
			GroupSettings gs;
			gs.period = period;
			gs.group_step = group_step;
			gs.average_period = period;
			
			PatternMatcherData& pmd = pm.RefreshData(group_step, period, period);
			int data_size = pmd.data.GetCount() / sym_count;
			
			VectorMap<unsigned, VectorBool>& group_data = this->group_data.Add(gs);
			
			int& counted = group_counted.GetAdd(gs, 0);
			
			for(int j = counted; j < data_size; j++) {
				bool current_groups = j == data_size-1;
				if (current_groups)
					this->current_groups.Clear();
				
				int group_count = 0;
				for(int k = 0; k < sym_count; k++)
					ch[k].hash = CombineHash::INIT;
				for(int k = 0; k < sym_count; k++) {
					int group = pmd.data[j * sym_count + k];
					ch[group].Put(k).Put(1);
					group_count = max(group_count, group+1);
				}
				
				for(int k = 0; k < group_count; k++) {
					unsigned hash = ch[k];
					int l = unique_groups.Find(hash);
					if (l == -1) {
						l = unique_groups.GetCount();
						UniqueGroup& ug = unique_groups.Add(hash);
						for(int k2 = 0; k2 < sym_count; k2++) {
							int group = pmd.data[j * sym_count + k2];
							if (group == k)
								ug.symbols.Add(k2);
						}
					}
					UniqueGroup& ug = unique_groups[l];
					if (ug.settings.Find(gs) == -1)
						ug.settings.Add(gs);
					
					l = group_data.Find(hash);
					if (l == -1) {
						l = group_data.GetCount();
						VectorBool& vb = group_data.Add(hash);
						vb.SetCount(data_size);
					}
					VectorBool& vb = group_data[l];
					vb.Set(j, true);
					
					if (current_groups)
						this->current_groups.GetAdd(gs).Add(hash);
				}
			}
			
			counted = data_size;
		}
	}
	
	SortByValue(unique_groups, UniqueGroup());
}

void EventAutomation::RefreshSimpleEvents() {
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	
	cl_sym.Refresh();
	
	current_signals.SetCount(ci_queue.GetCount(), 0);
	
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		EventCore& c = *ci_queue[i]->core;
		VectorMap<int, OnlineAverage1>& d = data[i];
		
		int sym = net.symbol_ids.Find(c.GetSymbol());
		ConstBuffer& buf = cl_sym.GetBuffer(sym, 0, 0);
		
		int data_end = buf.GetCount();
		
		VectorMap<int, VectorBool>& data_vectors = simple_data[i];
		
		for(int j = counted; j < data_end; j++) {
			int output = 0;
			
			c.Start(j, output);
			
			if (output) {
				VectorBool& sigv = sig_whichfirst[sym];
				
				if (j < sigv.GetCount()) {
					int first_sig = sigv.Get(j);
					d.GetAdd(output).Add(first_sig);
				}
				
				int k = data_vectors.Find(output);
				if (k == -1) {
					k = data_vectors.GetCount();
					VectorBool& vb = data_vectors.Add(output);
					vb.SetCount(data_end);
				}
				VectorBool& vb = data_vectors[k];
				if (vb.GetCount() < data_end) vb.SetCount(data_end);
				vb.Set(j, true);
			}
			
			if (j == data_end-1)
				current_signals[i] = output;
		}
		
		for(int j = 0; j < data_vectors.GetCount(); j++)
			data_vectors[j].SetCount(data_end);
	}
	
	int count = cl_sym.GetBuffer(0, 0, 0).GetCount();
	int prev_counted = counted;
	counted = count;
	if (prev_counted != counted) {
		do_store = true;
	}
}

void EventAutomation::RefreshCurrentComplexEvents() {
	VectorBool enabled, signal;
	
	Vector<EventOptResult> results;
	
	ASSERT(!current_groups.IsEmpty());
	for(int i = 0; i < current_groups.GetCount(); i++) {
		GroupSettings gs = current_groups.GetKey(i);
		const Vector<unsigned>& group_hashes = current_groups[i];
		
		for(int j = 0; j < group_hashes.GetCount(); j++) {
			unsigned group_hash = group_hashes[j];
			VectorBool& group_data = this->group_data.GetAdd(gs).GetAdd(group_hash);
			UniqueGroup& ug = unique_groups.Get(group_hash);
			
			if (group_data.IsEmpty())
				continue;
			
			for(int k = 0; k < ug.symbols.GetCount(); k++) {
				VectorBool& neg_sym_signals = sig_whichfirst[k];
				
				int count = min(neg_sym_signals.GetCount(), group_data.GetCount());
				
				Optimizer opt;
				opt.Min().SetCount(ug.symbols.GetCount(), 0.0);
				opt.Max().SetCount(ug.symbols.GetCount(), 1.0);
				opt.Init(ug.symbols.GetCount(), 100);
				
				
				enabled.SetCount(count);
				
				double best_confidence = -DBL_MAX, best_prob;
				
				while (!opt.IsEnd()) {
					
					opt.Start();
					
					const Vector<double>& trial = opt.GetTrialSolution();
					
					enabled.One();
					
					bool fail = false;
					
					for(int k = 0; k < ug.symbols.GetCount(); k++) {
						int sym_ci_count = symbol_simple_datas[k].GetCount();
						int64 src = trial[k] * sym_ci_count;
						if (src < 0)
							continue;
						while (src >= sym_ci_count) src -= sym_ci_count;
						
						int ci_id = symbol_simple_datas[k][(int)src];
						int current_signal = current_signals[ci_id];
						if (!current_signal)
							continue;
						
						int l = simple_data[ci_id].Find(current_signal);
						if (l == -1) {
							fail = true;
							break;
						}
						VectorBool& data = simple_data[ci_id][l];
						
						enabled.And(data);
					}
					
					int enabled_popcount = enabled.PopCount();
					
					if (enabled_popcount < 10 || fail)
						opt.Stop(-100000);
					
					else {
						signal = neg_sym_signals;
						signal.And(enabled);
						int neg_popcount = signal.PopCount();
						
						double prob = (double)neg_popcount / (double)enabled_popcount;
						double confidence = fabs(prob - 0.5) * 2.0;
						
						opt.Stop(confidence);
						
						if (confidence > best_confidence) {
							best_confidence = confidence;
							best_prob = prob;
						}
					}
				}
				
				LOG("best energy: " << opt.GetBestEnergy());
				
				
				const Vector<double>& trial = opt.GetBestSolution();
				EventOptResult& r = results.Add();
				r.sym = k;
				r.neg_first_confidence = best_confidence;
				r.neg_first_prob = best_prob;
				
				for(int k = 0; k < ug.symbols.GetCount(); k++) {
					int sym_ci_count = symbol_simple_datas[k].GetCount();
					int64 src = trial[k] * sym_ci_count;
					if (src < 0)
						continue;
					while (src >= sym_ci_count) src -= sym_ci_count;
					
					int ci_id = symbol_simple_datas[k][(int)src];
					int current_signal = current_signals[ci_id];
					if (!current_signal)
						continue;
					
					r.cis.Add(ci_id);
					r.signals.Add(current_signal);
				}
			}
		}
	}
	
	Swap(results, opt_results);
}

void EventAutomation::Start() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(EventCore::fast_tf);
	Time last_time = idx.Top();
	
	if (last_time > prev_update) {
		prev_update = last_time;
		
		RefreshCachedSignals();
		RefreshSimpleEvents();
		RefreshGroups();
		RefreshCurrentComplexEvents();
		
		if (do_store) {
			StoreThis();
			do_store = false;
		}
	}
}

void EventAutomation::RefreshCachedSignals() {
	int sym_count = cl_sym.GetSymbolCount();
	
	sig_whichfirst.SetCount(sym_count);
	
	for(int i = 0; i < sym_count; i++) {
		VectorBool& sym_whichfirst = sig_whichfirst[i];
		ConstBuffer& buf = cl_sym.GetBuffer(i, 0, 0);
		
		int counted = sym_whichfirst.GetCount();
		sym_whichfirst.SetCount(buf.GetCount());
		
		for(int j = counted; j < buf.GetCount(); j++) {
			int sig = GetSignalWhichFirst(i, j, 30);
			if (sig == -1) {
				sym_whichfirst.SetCount(j);
				break;
			}
			sym_whichfirst.Set(j, sig);
		}
		
	}
}

int EventAutomation::GetSignalWhichFirst(int sym, int pos, int pips) {
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
	Add(tabs.SizePos());
	
	tabs.Add(curopt.SizePos(), "Current optimization");
	tabs.Add(curdata.SizePos(), "Current data");
	tabs.Add(uniquesplit.SizePos(), "Unique groups");
	
	curopt.AddColumn("Symbol");
	curopt.AddColumn("What");
	curopt.AddColumn("First low probability");
	curopt.AddColumn("First low confidence");
	curopt.ColumnWidths("1 5 1 1 ");
	
	curdata.Add(slider.TopPos(0, 30).HSizePos(0, 200));
	curdata.Add(date.TopPos(0, 30).RightPos(0, 200));
	curdata.Add(list.HSizePos().VSizePos(30));
	
	slider.MinMax(0,1);
	slider.SetData(0);
	slider <<= THISBACK(Data);
	
	list.AddColumn("Symbol");
	list.AddColumn("What");
	list.AddColumn("Signal");
	list.AddColumn("First low probability");
	list.AddColumn("First low Confidence");
	
	uniquesplit << uniquegroups << uniquesettings;
	uniquegroups.AddColumn("Symbols");
	uniquesettings.AddColumn("Pattern size");
	uniquesettings.AddColumn("Group step");
	uniquesettings.AddColumn("Average period");
	uniquegroups <<= THISBACK(Data);
}

void EventAutomationCtrl::Data() {
	EventAutomation& ea = GetEventAutomation();
	System& sys = GetSystem();
	
	int tab = tabs.Get();
	
	if (tab == 0) {
		System::NetSetting& net = sys.GetNet(0);
		for(int i = 0; i < ea.opt_results.GetCount(); i++) {
			EventOptResult& r = ea.opt_results[i];
			
			String what;
			for(int j = 0; j < r.cis.GetCount(); j++) {
				EventCore& c = *ea.ci_queue[r.cis[j]]->core;
				
				if (j) what << ", ";
				what << sys.GetSymbol(c.GetSymbol()) << " " << c.GetTitle();
			}
			curopt.Set(i, 0, net.symbols.GetKey(r.sym));
			curopt.Set(i, 1, what);
			curopt.Set(i, 2, r.neg_first_prob);
			curopt.Set(i, 3, r.neg_first_confidence);
		}
		curopt.SetCount(ea.opt_results.GetCount());
		curopt.SetSortColumn(3, true);
	}
	
	if (tab == 1) {
		DataBridgeCommon& dbc = GetDataBridgeCommon();
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
				list.Set(rows, 4, fabs(firstprob - 0.5) * 2.0);
				rows++;
			}
		}
		list.SetCount(rows);
		list.SetSortColumn(4, true);
	}
	
	else if (tab == 2) {
		System::NetSetting& net = sys.GetNet(0);
		
		for(int i = 0; i < ea.unique_groups.GetCount(); i++) {
			UniqueGroup& ug = ea.unique_groups[i];
			
			String s;
			for(int j = 0; j < ug.symbols.GetCount(); j++) {
				if (j) s << ", ";
				s << net.symbols.GetKey(ug.symbols[j]);
			}
			uniquegroups.Set(i, 0, s);
		}
		
		if (uniquegroups.IsCursor()) {
			UniqueGroup& ug = ea.unique_groups[uniquegroups.GetCursor()];
			
			for(int i = 0; i < ug.settings.GetCount(); i++) {
				const GroupSettings& gs = ug.settings[i];
				uniquesettings.Set(i, 0, gs.period);
				uniquesettings.Set(i, 1, gs.group_step);
				uniquesettings.Set(i, 2, gs.average_period);
			}
			uniquesettings.SetCount(ug.settings.GetCount());
		}
	}
}

}
