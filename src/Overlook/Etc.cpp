#if 0






IdealOrders::IdealOrders() {
	
}

void IdealOrders::Init() {
	//AddDependency("/open", 1, 0);
	
}

void IdealOrders::Start() {
	
	// Search ideal order sequence with A* search in SimBroker
	Panic("TODO");
	
}

























#define PREV_STEPS 8
#define PREV_LEN 8
#define MAXTIMESTEP 6

QueryTableForecaster::QueryTableForecaster() {
	
}

void QueryTableForecaster::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-1);
	SetCoreMaximum(+1);
	SetBufferColor(0, Green);
	SetBufferLineWidth(0, 2);
	
	
	// Add targets
	qt.AddColumn("Next change", PREV_STEPS);
	qt.EndTargets();
	
	// Add previous values
	for(int i = 1; i <= PREV_LEN; i++) {
		qt.AddColumn("Cur diff -" + IntStr(i), PREV_STEPS);
		if (i > 1) qt.AddColumn("Change -" + IntStr(i), PREV_STEPS);
	}
	
	// Add constants columns
	//  Note: adding month and day causes underfitting
	qt.AddColumn("Wday",			7);
	qt.AddColumn("Hour",			24);
	qt.AddColumn("5-min",			12);
	
	// Add columns from inputs
	for(int i = 0; i < optional_inputs.GetCount(); i++) {
		Input& indi_input = optional_inputs[i];
		if (indi_input.IsEmpty()) continue;
		ASSERT(indi_input.GetCount() == 1);
		
		Panic("TODO");
	}
}

void QueryTableForecaster::Start() {
	int bars = GetBars();
	int counted = GetCounted();
	if (counted == bars)
		return;
	
	int tf = GetTf();
	System& bs = GetSystem();
	ValueChange& bc = *Get<ValueChange>();
	double max_change = bc.GetMedianMax() * 2;
	double min_change = bc.GetMedianMin() * 2;
	double diff = max_change - min_change;
	Panic("TODO: change -> diff");
	bars -= 1;
	
	qt.Reserve(bars);
	
	TimeStop ts;
	VectorMap<int, int> in_qt;
	in_qt.Reserve(bars - counted);
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+1);
		
		// Only add valid data to avoid underfitting
		double open = Open(i);
		double close = Open(i+1);
		if (open == close) continue;
		
		// Add row
		int row = qt.GetCount();
		in_qt.Add(i, row);
		qt.SetCount(row+1);
		
		// Get some time values in binary format (starts from 0)
		Time t = bs.GetTimeTf(tf, i);
		int month = t.month-1;
		int day = t.day-1;
		int hour = t.hour;
		int minute = t.minute;
		int dow = DayOfWeek(t);
		int wdayhour = dow*24 + t.hour;
		int pos = 0;
		
		// Add target value after one timestep
		double change = (open != 0.0 ? close / open - 1.0 : 0.0);
		const int div = PREV_STEPS;
		double step = diff / div;
		int v = (change - min_change) / step;
		if (v < 0)
			v = 0;
		if (v >= div)
			v = div -1;
		qt.Set(row, pos++, v);
		
		// Add previous changes in value
		double prev = open;
		for(int j = 1; j <= PREV_LEN; j++) {
			int k = Upp::max(0, i - j);
			double cur = Open(k);
			
			double change1 = cur != 0.0 ? open / cur - 1.0 : 0.0;
			int v = (change1 - min_change) / step;
			if (v < 0) v = 0;
			if (v >= div) v = div -1;
			qt.Set(row, pos++, v);
			
			if (j > 1) {
				double change2 = cur != 0.0 ? prev / cur - 1.0 : 0.0;
				v = (change2 - min_change) / step;
				if (v < 0) v = 0;
				if (v >= div) v = div -1;
				qt.Set(row, pos++, v);
			}
			
			prev = cur;
		}
		
		// Add constant time values
		qt.Set(row, pos++, dow);
		qt.Set(row, pos++, hour);
		qt.Set(row, pos++, minute / 5);
		
		
		// TODO: add indicators
		
	}
	
	// Create decision tree
	DecisionTreeNode tree;
	qt.GetDecisionTree(0, tree, qt.GetCount());
	
	// Draw error oscillator
	Buffer& buf = GetBuffer(0);
	for (int i = counted; i < bars; i++) {
		
		// Skip invalid values, which weren't added to the query-table
		int j = in_qt.Find(i);
		if (j == -1) {
			buf.Set(i, -1);
			continue;
		}
		int row = in_qt[j];
		
		// Get prediction and correct value
		double predicted = qt.Predict(tree, row, 0);
		double correct = qt.Get(row, 0);
		double diff = fabs(predicted - correct) / PREV_STEPS;
		buf.Set(i, diff);
	}
}















QueryTableHugeForecaster::QueryTableHugeForecaster() {
	corr_period = 4;
}

void QueryTableHugeForecaster::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-1);
	SetCoreMaximum(+1);
	for(int i = 0; i < 6; i++) {
		SetBufferColor(i, RainbowColor(i / 6.0));
		SetBufferLineWidth(i, 6-i);
	}
	
	// Add targets
	for(int i = 0; i < MAXTIMESTEP; i++) {
		int len = pow(2, i);
		qt.AddColumn("Change +" + IntStr(len), PREV_STEPS);
	}
	qt.EndTargets();
	
	System& bs = GetSystem();
	int sym_count = bs.GetSymbolCount();
	int tf_count = bs.GetPeriodCount() - GetTimeframe();
	
	// Add previous values
	for (int sym = 0; sym < sym_count; sym++) {
		if (sym == GetSymbol()) continue; // skip this
		for (int tf = 0; tf < tf_count; tf++) {
			qt.AddColumn("Cur diff -1 (" + IntStr(sym) + ":" + IntStr(tf) + ")", PREV_STEPS);
			qt.AddColumn("Correlation (" + IntStr(sym) + ":" + IntStr(tf) + ")", PREV_STEPS);
		}
	}
	
	// Add constants columns
	//  Note: adding month and day causes underfitting
	qt.AddColumn("Wday",			7);
	qt.AddColumn("Hour",			24);
	qt.AddColumn("5-min",			12);
	
	// Add columns from inputs
	/*for (int sym = 0; sym < sym_count; sym++) {
		if (sym == GetSymbol()) continue; // skip this
		for (int tf = 0; tf < tf_count; tf++) {
			for(int i = 0; i < optional_inputs.GetCount(); i++) {
				Input& indi_input = optional_inputs[i];
				if (indi_input.sources.IsEmpty()) continue;
				ASSERT(indi_input.sources.GetCount() == 1);
				
				Panic("TODO");
			}
		}
	}*/
}

void QueryTableHugeForecaster::Start() {
	int id = GetSymbol();
	int thistf = GetTf();
	int bars = GetBars();
	int counted = GetCounted();
	if (bars == counted)
		return;
	
	
	System& bs = GetSystem();
	int sym_count = bs.GetSymbolCount();
	int tf_count = bs.GetPeriodCount() - GetTimeframe();
	
	// Get some useful values for all syms and tfs
	Vector<ConstBuffer*> bufs;
	Vector<double> max_changes, min_changes, diffs;
	bufs.SetCount(sym_count * tf_count, NULL);
	max_changes.SetCount(sym_count * tf_count, 0);
	min_changes.SetCount(sym_count * tf_count, 0);
	diffs.SetCount(sym_count * tf_count, 0);
	
	Vector<int> time_pos;
	time_pos.SetCount(tf_count, 0);
	
	for (int sym = 0; sym < sym_count; sym++) {
		for (int tf = 0; tf < tf_count; tf++) {
			int i = sym * tf_count + tf;
			bufs[i] = &GetInputBuffer(0, sym, tf + thistf, 0);
			ValueChange& bc = *dynamic_cast<ValueChange*>(GetInputCore(1, sym, tf + thistf));
			double max_change = bc.GetMedianMax() * 2;
			double min_change = bc.GetMedianMin() * 2;
			double diff = max_change - min_change;
			Panic("TODO: change -> diff");
			//LOG(Format("i=%d sym=%d tf=%d max=%f min=%f diff=%f", i, sym, tf, max_change, min_change, diff));
			max_changes[i] = max_change;
			min_changes[i] = min_change;
			diffs[i] = diff;
		}
	}
	
	int ii = id * tf_count;
	double max_change = max_changes[ii];
	double min_change = min_changes[ii];
	double diff       = diffs[ii];
	//LOG(Format("i=%d sym=%d tf=%d max=%f min=%f diff=%f", ii, id, 0, max_change, min_change, diff));
	
	int peek = pow(2, MAXTIMESTEP-1);
	bars -= peek;
	
	qt.Reserve(bars);
	
	TimeStop ts;
	VectorMap<int, int> in_qt;
	in_qt.Reserve(bars - counted);
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+peek);
		
		// Only add valid data to avoid underfitting
		double open = Open(i);
		double close = Open(i+1);
		if (open == close) continue;
		
		// Add row
		int row = qt.GetCount();
		in_qt.Add(i, row);
		qt.SetCount(row+1);
		
		// Get some time values in binary format (starts from 0)
		Time t = bs.GetTimeTf(thistf, i);
		int month = t.month-1;
		int day = t.day-1;
		int hour = t.hour;
		int minute = t.minute;
		int dow = DayOfWeek(t);
		int wdayhour = dow*24 + t.hour;
		int pos = 0;
		time_pos[0] = i;
		for(int j = 1; j < tf_count; j++)
			time_pos[j] = bs.GetShiftTf(thistf, thistf + j, i);
		
		// Add target value after timestep
		for(int j = 0; j < MAXTIMESTEP; j++) {
			int len = pow(2, j);
			int k = i + len;
			double next = Open(k);
			double change = open != 0.0 ? next / open - 1.0 : 0.0;
			const int div = PREV_STEPS;
			double step = diff / div;
			int v = (change - min_change) / step;
			if (v < 0)
				v = 0;
			if (v >= div)
				v = div -1;
			qt.Set(row, pos++, v);
		}
		
		// Add previous changes in value
		for (int sym = 0, csym = 0; sym < sym_count; sym++) {
			if (sym == GetSymbol()) continue; // skip this
			for (int tf = 0; tf < tf_count; tf++) {
				int j = sym * tf_count + tf;
				ConstBuffer& buf = *bufs[j];
				int k = time_pos[tf];
				double open = k > 0 ? buf.GetUnsafe(k-1) : 0.0;
				double cur = buf.GetUnsafe(k);
				double min_change = min_changes[j];
				double diff = diffs[j];
				const int div = PREV_STEPS;
				double step = diff / div;
				
				// Difference to previous time-position
				double change = open != 0.0 ? cur / open - 1.0 : 0.0;
				int v = (change - min_change) / step;
				if (v < 0) v = 0;
				if (v >= div) v = div -1;
				qt.Set(row, pos++, v);
				
				// Correlation to the main symbol
				ConstBuffer& cbuf = GetInputBuffer(2, GetSymbol(), thistf + tf, csym);
				double corr = cbuf.GetUnsafe(k);
				v = (corr + 1.0) / 2.0 * PREV_STEPS;
				if (v == PREV_STEPS) v = PREV_STEPS - 1; // equal to 1.0 doesn't need own range
				ASSERT(v >= 0 && v < PREV_STEPS);
				qt.Set(row, pos++, v);
			}
			csym++;
		}
		
		// Add constant time values
		qt.Set(row, pos++, dow);
		qt.Set(row, pos++, hour);
		qt.Set(row, pos++, minute / 5);
		
		
		// TODO: add indicators
		
	}
	
	// Create decision trees
	tree.Clear();
	tree.SetCount(MAXTIMESTEP);
	for(int i = 0; i < tree.GetCount(); i++)
		qt.GetDecisionTree(i, tree[i], qt.GetCount());
	
	// Draw error oscillator
	for (int i = counted; i < bars; i++) {
		
		// Skip invalid values, which weren't added to the query-table
		int j = in_qt.Find(i);
		if (j == -1) {
			for(int j = 0; j < MAXTIMESTEP; j++)
				GetBuffer(j).Set(i, -1);
			continue;
		}
		int row = in_qt[j];
		
		// Get prediction and correct value
		for(int j = 0; j < MAXTIMESTEP; j++) {
			double predicted = qt.Predict(tree[j], row, j);
			double correct = qt.Get(row, j);
			double diff = fabs(predicted - correct) / PREV_STEPS;
			GetBuffer(j).Set(i, diff);
		}
	}
}


QueryTableMetaForecaster::QueryTableMetaForecaster() {
	
}

void QueryTableMetaForecaster::Init() {
	
}

void QueryTableMetaForecaster::Start() {
	
	Panic("TODO");
}


QueryTableAgent::QueryTableAgent() {
	
}

void QueryTableAgent::Init() {
	
}

void QueryTableAgent::Start() {
	
	Panic("TODO");
}


QueryTableHugeAgent::QueryTableHugeAgent() {
	
}

void QueryTableHugeAgent::Init() {
	
}

void QueryTableHugeAgent::Start() {
	
	Panic("TODO");
}


QueryTableMetaAgent::QueryTableMetaAgent() {
	
}

void QueryTableMetaAgent::Init() {
	
}

void QueryTableMetaAgent::Start() {
	
	Panic("TODO");
}


QueryTableDoubleAgent::QueryTableDoubleAgent() {
	
}

void QueryTableDoubleAgent::Init() {
	
}

void QueryTableDoubleAgent::Start() {
	
	Panic("TODO");
}









#endif