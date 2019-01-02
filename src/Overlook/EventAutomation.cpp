#include "Overlook.h"

#if 0
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
			
			ArgScript args;
			core.Arg(args);
			
			if (args.mins.IsEmpty()) {
				FactoryDeclaration decl;
				decl.factory = j;
				sys.GetScriptCoreQueue(ci_queue, id, decl);
			} else {
				Vector<int> arg_values;
				arg_values <<= args.mins;
				
				while (true) {
					FactoryDeclaration decl;
					decl.factory = j;
					for(int i = 0; i < arg_values.GetCount(); i++)
						decl.AddArg(arg_values[i]);
					
					sys.GetScriptCoreQueue(ci_queue, id, decl);
					
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
	
	
	//LoadThis();
	
	if (data.GetCount() && data.GetCount() != ci_queue.GetCount()) {
		group_data.Clear();
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
	
	if (!is_optimized)
		Thread::Start(THISBACK(Process));
}

void EventAutomation::RefreshGroups() {
	PatternMatcher& pm = GetPatternMatcher();
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
				int group_count = 0;
				for(int k = 0; k < sym_count; k++)
					ch[k].hash = CombineHash::INIT;
				for(int k = 0; k < sym_count; k++) {
					int group = pmd.data[j * sym_count + k];
					if (group >= 0) {
						ch[group].Put(k).Put(1);
						group_count = max(group_count, group+1);
					} else {
						ch[-group-1].Put(-k-1).Put(1);
						group_count = max(group_count, -group-1+1);
					}
				}
				
				for(int k = 0; k < group_count; k++) {
					unsigned hash = ch[k];
					int l = unique_groups.Find(hash);
					if (l == -1) {
						l = unique_groups.GetCount();
						UniqueGroup& ug = unique_groups.Add(hash);
						for(int k2 = 0; k2 < sym_count; k2++) {
							int group = pmd.data[j * sym_count + k2];
							if (k == group)
								ug.symbols.Add(k2);
							else if (k == -group-1)
								ug.symbols.Add(-k2-1);
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
				}
			}
			
			counted = data_size;
		}
	}
	
	SortByValue(unique_groups, UniqueGroup());
}

void EventAutomation::GetCurrentGroups(int pos, VectorMap<GroupSettings, Vector<unsigned> >& current_groups) {
	current_groups.Clear();
	
	PatternMatcher& pm = GetPatternMatcher();
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
			
			int group_count = 0;
			for(int k = 0; k < sym_count; k++)
				ch[k].hash = CombineHash::INIT;
			for(int k = 0; k < sym_count; k++) {
				int group = pmd.data[pos * sym_count + k];
				if (group >= 0) {
					ch[group].Put(k).Put(1);
					group_count = max(group_count, group+1);
				} else {
					ch[-group-1].Put(-k-1).Put(1);
					group_count = max(group_count, -group-1+1);
				}
			}
			
			for(int k = 0; k < group_count; k++) {
				unsigned hash = ch[k];
				current_groups.GetAdd(gs).Add(hash);
			}
		}
	}
}

void EventAutomation::RefreshSimpleEvents() {
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	
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

void EventAutomation::GetCurrentSignals(int pos, Vector<byte>& current_signals) {
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	
	current_signals.SetCount(ci_queue.GetCount(), 0);
	
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		EventCore& c = *ci_queue[i]->core;
		int output = 0;
		c.Start(pos, output);
		current_signals[i] = output;
	}
}

void EventAutomation::GetCurrentComplexEvents(int pos, int min_samplecount, Vector<EventOptResult>& results) {
	VectorMap<GroupSettings, Vector<unsigned> > current_groups;
	Vector<byte> current_signals;
	
	GetCurrentGroups(pos, current_groups);
	GetCurrentSignals(pos, current_signals);
	
	VectorBool enabled, signal;
	
	
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
			
			if (ug.symbols.GetCount() <= 1)
				continue;
			
			
			Optimizer opt;
			opt.Min().SetCount(ug.symbols.GetCount(), 0.0);
			opt.Max().SetCount(ug.symbols.GetCount(), 1.0);
			opt.Init(ug.symbols.GetCount(), 100);
			
			
			enabled.SetCount(group_data.GetCount());
			
			Vector<double> best_trial;
			double best_confidence = -DBL_MAX, best_prob;
			int best_popcount = 0;
			
			while (!opt.IsEnd()) {
				
				opt.Start();
				
				const Vector<double>& trial = opt.GetTrialSolution();
				
				enabled.One();
				enabled.LimitLeft(pos);
				
				bool fail = false;
				
				for(int k = 0; k < ug.symbols.GetCount(); k++) {
					int sym = ug.symbols[k];
					if (sym < 0) sym = -sym-1;
					
					int sym_ci_count = symbol_simple_datas[sym].GetCount();
					int64 src = trial[k] * sym_ci_count;
					if (src < 0)
						continue;
					while (src >= sym_ci_count) src -= sym_ci_count;
					
					int ci_id = symbol_simple_datas[sym][(int)src];
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
				if (enabled_popcount < min_samplecount)
					fail = true;
				
				
				double prob_sum = 0;
				for(int k = 0; k < ug.symbols.GetCount() && !fail; k++) {
					int sym = ug.symbols[k];
					bool is_neg = sym < 0;
					if (is_neg) sym = -sym-1;
					
					VectorBool& neg_sym_signals = sig_whichfirst[sym];
					
					int count = min(neg_sym_signals.GetCount(), group_data.GetCount());
					
					signal = neg_sym_signals;
					signal.And(enabled);
					int neg_popcount = signal.PopCount();
					
					double prob = (double)neg_popcount / (double)enabled_popcount;
					if (!is_neg)
						prob_sum += prob;
					else
						prob_sum += 1 - prob;
				}
				
				
				
				if (fail)
					opt.Stop(-100000);
				
				else {
					double prob = prob_sum / ug.symbols.GetCount();
					double confidence = fabs(prob - 0.5) * 2.0;
					
					opt.Stop(confidence);
					
					if (confidence > best_confidence) {
						best_confidence = confidence;
						best_prob = prob;
						best_popcount = enabled_popcount;
						best_trial <<= trial;
					}
				}
			}
			
			LOG("best energy: " << opt.GetBestEnergy());
			
			if (best_trial.IsEmpty()) continue;
			
			//for (int k2 = 0; k2 < ug.symbols.GetCount() ; k2++) {
			const Vector<double>& trial = best_trial;
			EventOptResult& r = results.Add();
			r.sym = ug.symbols[0];
			r.neg_first_confidence = best_confidence;
			r.neg_first_prob = best_prob;
			r.popcount = best_popcount;
			
			if (r.sym < 0) {
				r.sym = -r.sym-1;
				r.neg_first_prob = 1.0 - r.neg_first_prob;
			}
			
			for(int k = 0; k < ug.symbols.GetCount(); k++) {
				int sym = ug.symbols[k];
				if (sym < 0) sym = -sym-1;
				
				int sym_ci_count = symbol_simple_datas[sym].GetCount();
				int64 src = trial[k] * sym_ci_count;
				if (src < 0)
					continue;
				while (src >= sym_ci_count) src -= sym_ci_count;
				
				int ci_id = symbol_simple_datas[sym][(int)src];
				int current_signal = current_signals[ci_id];
				if (!current_signal)
					continue;
				
				r.cis.Add(ci_id);
				r.signals.Add(current_signal);
			}
			//}
		}
	}
	
}

void EventAutomation::Process() {
	
	best_opt_result = -DBL_MAX;
	
	#if 0
	opt_settings.min_popcount = 10;
	opt_settings.min_confidence = 0.1;
	opt_settings.whichfirst_pips = 50;
	
	ClearCachedSignals();
	RefreshCachedSignals(opt_settings.whichfirst_pips);
	RefreshSimpleEvents();
	#else
	for(int whichfirst_pips = 30; whichfirst_pips <= 70; whichfirst_pips += 10)
		for (int min_popcount = 10; min_popcount <= 100; min_popcount += 20)
			for(double min_confidence = 0.1; min_confidence <= 0.4; min_confidence += 0.1)
				opt_total++;
		
	for(int whichfirst_pips = 30; whichfirst_pips <= 70; whichfirst_pips += 10) {
		ClearCachedSignals();
		RefreshCachedSignals(whichfirst_pips);
		RefreshSimpleEvents();
		
		for (int min_popcount = 10; min_popcount <= 100; min_popcount += 20) {
			
			for(double min_confidence = 0.1; min_confidence <= 0.4; min_confidence += 0.1) {
				
				double result = GetTestResult(24, min_popcount, min_confidence, whichfirst_pips);
				
				if (result > best_opt_result) {
					best_opt_result = result;
					opt_settings.min_popcount = min_popcount;
					opt_settings.min_confidence = min_confidence;
					opt_settings.whichfirst_pips = whichfirst_pips;
				}
				
				opt_actual++;
			}
		}
	}
	DUMP(opt_settings.min_popcount);
	DUMP(opt_settings.min_confidence);
	DUMP(opt_settings.whichfirst_pips);
	
	ClearCachedSignals();
	RefreshCachedSignals(opt_settings.whichfirst_pips);
	RefreshSimpleEvents();
	
	#endif
	
	opt_test_result = GetTestResult(1000, opt_settings.min_popcount, opt_settings.min_confidence, opt_settings.whichfirst_pips);
	
	is_optimized = true;
}

double EventAutomation::GetTestResult(int test_size, int min_popcount, double min_confidence, int whichfirst_pips) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(EventCore::fast_tf);
	
	int data_begin = idx.GetCount() - 3-1 - test_size;
	int data_end = idx.GetCount() - 3;
	
	Vector<EventOptResult> results;
	Vector<bool> sym_added;
	Vector<int> sym_sig, prev_sym_sig;
	double result = 0;
	
	sym_added.SetCount(cl_sym.GetSymbolCount());
	sym_sig.SetCount(cl_sym.GetSymbolCount());
	prev_sym_sig.SetCount(cl_sym.GetSymbolCount(), 0);
	
	test_vector.SetCount(0);
	
	for(int i = data_begin; i < data_end; i++) {
		//Time t = idx[i];
		//if (t.hour < 6 || t.hour >= 18)
		//	continue;
		
		GetCurrentComplexEvents(i, min_popcount, results);
		
		Sort(results, EventOptResult());
		
		for(int j = 0; j < sym_added.GetCount(); j++) {
			sym_added[j] = false;
			sym_sig[j] = 0;
		}
		
		#if 0
		for(int j = 0; j < results.GetCount(); j++) {
			EventOptResult& r = results[j];
			
			if (r.neg_first_confidence >= min_confidence && sym_added[r.sym] == false) {
				sym_added[r.sym] = true;
				
				bool sig = r.neg_first_prob >= 0.5;
				
				ConstBuffer& buf = cl_sym.GetBuffer(r.sym, 0, 0);
				double cur = buf.Get(i);
				double next = buf.Get(i+1);
				double ch;
				
				if (!sig) {
					cur += cl_sym.GetDataBridge(r.sym)->GetPoint() * CommonSpreads()[r.sym];
					ch = next / cur - 1.0;
				} else {
					next += cl_sym.GetDataBridge(r.sym)->GetPoint() * CommonSpreads()[r.sym];
					ch = next / cur - 1.0;
					ch *= -1;
				}
				
				result += ch;
			}
		}
		#else
		for(int j = 0; j < results.GetCount(); j++) {
			EventOptResult& r = results[j];
			if (r.neg_first_confidence >= min_confidence) {
				sym_sig[r.sym] += (r.neg_first_prob >= 0.5 ? -1 : +1);
			}
		}
		for(int j = 0; j < sym_sig.GetCount(); j++) {
			int sig = sym_sig[j];
			if (sig) sig /= abs(sig);
			
			bool is_prev_sig = prev_sym_sig[j] == sig;
			prev_sym_sig[j] = sig;
			
			if (!sig) continue;
			
			ConstBuffer& buf = cl_sym.GetBuffer(j, 0, 0);
			double cur = buf.Get(i);
			double next = buf.Get(i+1);
			double ch;
			
			if (sig == +1) {
				if (!is_prev_sig) cur += cl_sym.GetDataBridge(j)->GetPoint() * CommonSpreads()[j];
				ch = next / cur - 1.0;
			} else {
				if (!is_prev_sig) next += cl_sym.GetDataBridge(j)->GetPoint() * CommonSpreads()[j];
				ch = next / cur - 1.0;
				ch *= -1;
			}
			
			result += ch;
		}
		#endif
		
		test_vector.Add(result);
	}
	
	return result;
}

void EventAutomation::Start() {
	
	if (is_optimized) {
		DataBridgeCommon& dbc = GetDataBridgeCommon();
		const Index<Time>& idx = dbc.GetTimeIndex(EventCore::fast_tf);
		Time last_time = idx.Top();
		
		if (last_time > prev_update) {
			prev_update = last_time;
			
			cl_sym.Refresh();
			
			RefreshCachedSignals(opt_settings.whichfirst_pips);
			RefreshSimpleEvents();
			RefreshGroups();
			GetCurrentComplexEvents(idx.GetCount()-1, opt_settings.min_popcount, results);
			
			if (do_store) {
				StoreThis();
				do_store = false;
			}
		}
	}
}

void EventAutomation::ClearCachedSignals() {
	for(int i = 0; i < sig_whichfirst.GetCount(); i++)
		sig_whichfirst[i].SetCount(0);
	
	counted = 0;
	for(int i = 0; i < simple_data.GetCount(); i++)
		simple_data[i].Clear();
}

void EventAutomation::RefreshCachedSignals(int pips) {
	int sym_count = cl_sym.GetSymbolCount();
	
	sig_whichfirst.SetCount(sym_count);
	
	for(int i = 0; i < sym_count; i++) {
		VectorBool& sym_whichfirst = sig_whichfirst[i];
		ConstBuffer& buf = cl_sym.GetBuffer(i, 0, 0);
		
		int counted = sym_whichfirst.GetCount();
		sym_whichfirst.SetCount(buf.GetCount());
		
		for(int j = counted; j < buf.GetCount(); j++) {
			int sig = GetSignalWhichFirst(i, j, pips);
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
	double low_target  = open - pips * point;
	double high_target = open + pips * point;
	
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



EventAutomationCtrl::EventAutomationCtrl() {
	Add(tabs.SizePos());
	
	tabs.Add(curopt.SizePos(), "Current optimization");
	tabs.Add(curdata.SizePos(), "Current data");
	tabs.Add(uniquesplit.SizePos(), "Unique groups");
	tabs.Add(optctrl.SizePos(), "Optimization");
	tabs.Add(test.SizePos(), "Test drawer");
	
	curopt.AddColumn("Symbol");
	curopt.AddColumn("What");
	curopt.AddColumn("First low probability");
	curopt.AddColumn("First low confidence");
	curopt.AddColumn("Popcount");
	curopt.ColumnWidths("1 5 1 1 1 ");
	
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
	
	optctrl.Add(optprog.TopPos(0, 30).HSizePos());
	optctrl.Add(optlist.VSizePos(30).HSizePos());
	optlist.AddColumn("Key");
	optlist.AddColumn("Value");
}

void EventAutomationCtrl::Data() {
	EventAutomation& ea = GetEventAutomation();
	System& sys = GetSystem();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(EventCore::fast_tf);

	int tab = tabs.Get();
	
	if (tab == 0) {
		if (!ea.is_optimized)
			return;
		System::NetSetting& net = sys.GetNet(0);
		
		
		for(int i = 0; i < ea.results.GetCount(); i++) {
			EventOptResult& r = ea.results[i];
			
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
			curopt.Set(i, 4, r.popcount);
		}
		curopt.SetCount(ea.results.GetCount());
		curopt.SetSortColumn(3, true);
	}
	
	if (tab == 1) {
		
		if (slider.GetMax() != ea.counted-1 && ea.counted > 0)
			slider.MinMax(0, ea.counted-1);
		
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
				int sym = ug.symbols[j];
				if (sym < 0) sym = -sym-1;
				s << net.symbols.GetKey(sym);
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
	
	else if (tab == 3) {
		
		optprog.Set(ea.opt_actual, ea.opt_total);
		
		optlist.Set(0, 0, "Best minimum popcount");
		optlist.Set(0, 1, ea.opt_settings.min_popcount);
		optlist.Set(1, 0, "Best which-first pips");
		optlist.Set(1, 1, ea.opt_settings.whichfirst_pips);
		optlist.Set(2, 0, "Best minimum confidence");
		optlist.Set(2, 1, ea.opt_settings.min_confidence);
		optlist.Set(3, 0, "Best optimization result");
		optlist.Set(3, 1, ea.best_opt_result);
		optlist.Set(4, 0, "Best optimization test result");
		optlist.Set(4, 1, ea.opt_test_result);
	}
	
	else if (tab == 4) {
		test.Refresh();
	}
}

}
#endif
