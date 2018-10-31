#include "Agent.h"

namespace Agent {

Manager::Manager() {
	
	#ifdef flagMAIN
	if (sessions.GetCount() == 0) {
		GetAdd("EURUSD");
	}
	#endif
	
	Start();
}

Manager::~Manager() {
	
	Stop();
}

Session& Manager::GetAdd(String symbol) {
	int i = sessions.Find(symbol);
	if (i == -1) {
		Session& ses = sessions.GetAdd(symbol);
		ses.symbol = symbol;
		ses.Init();
		return ses;
	}
	else return sessions[i];
}

void Manager::Process() {
	int mode = 0;
	int ses_id = 0;
	while (running && !Thread::IsShutdownThreads()) {
		
		if (ses_id < sessions.GetCount()) {
			Session& ses = sessions[ses_id];
			
			if (!ses.RunTask()) {
				ses_id++;
				Sleep(100);
			}
		} else {
			ses_id = 0;
		}
		
		if (sessions.IsEmpty()) Sleep(100);
		
	}
	
	stopped = true;
}













Session::Session() {
	
}

void Session::Init() {
	Vector<FactoryDeclaration> decl;
	AddDefaultDeclarations(decl);
	for(int i = 0; i < decl.GetCount(); i++)
		AddIndiTask(i, decl[i]);
	
	AddTask("Bitstream join");
	AddTask("History match");
	AddTask("Optimization");
}

void Session::AddIndiTask(int id, FactoryDeclaration& decl) {
	Task& t = tasks.Add();
	t.decl = decl;
	t.symbol = symbol;
	t.task = "Indicator " + Format("%03d", id);
}

void Session::AddTask(String s) {
	Task& t = tasks.Add();
	t.symbol = symbol;
	t.task = s;
}

void Session::AddForecastTask(const Vector<double>& real_data) {
	Task& t = tasks.Add();
	t.symbol = symbol;
	t.real_data <<= real_data;
	t.task = "Forecast";
}

void Session::GetProgress(int& actual, int& total, String& state) {
	for(actual = 0; actual < tasks.GetCount(); actual++) {
		if (tasks[actual].actual < tasks[actual].total) {
			state = tasks[actual].task;
			break;
		}
	}
	total = tasks.GetCount();
}

bool Session::RunTask() {
	for(int i = 0; i < tasks.GetCount(); i++) {
		Task& t = tasks[i];
		if (t.actual < t.total) {
			t.Run();
			return true;
		}
	}
	return false;
}

bool Session::IsFinished() {
	for(int i = 0; i < tasks.GetCount(); i++) {
		Task& t = tasks[i];
		if (t.actual < t.total) {
			return false;
		}
	}
	return true;
}














Task::Task() {
	
}

void Task::Run() {
	RLOG("Task::Run " + task);
	
	if (task.Left(9) == "Indicator") {
		RunIndicator();
	}
	else if (task == "Bitstream join") {
		RunBitstreamJoin();
	}
	else if (task == "History match") {
		RunHistoryMatch();
	}
	else if (task == "Optimization") {
		RunOptimization();
	}
}

void Task::LoadData() {
	String dir = ConfigFile("m1data");
	String file = AppendFileName(dir, symbol + ".hst");
	
	FileIn src(file);
	if (!src.IsOpen() || !src.GetSize())
		Panic("Couldn't open history file for " + symbol);
	
	// Read the history file
	int digits;
	src.Seek(4+64+12+4);
	src.Get(&digits, 4);
	if (digits > 20)
		throw DataExc();
	double point = 1.0 / pow(10.0, digits);
	int data_size = (int)src.GetSize();
	int struct_size = 4 + 4*8 + 8;
	byte row[0x100];
	
	// Seek to begin of the data
	int cursor = (4+64+12+4+4+4+4 +13*4);
	int expected_count = (int)((src.GetSize() - cursor) / struct_size);
	src.Seek(cursor);
	
	real_data.Reserve(expected_count);
	
	
	while ((cursor + struct_size) <= data_size) {
		int64 time;
		double open, high, low, close;
		int64 tick_volume, real_volume;
		int spread;
		src.Get(row, struct_size);
		byte* current = row;
		
		time  = *((int32*)current);				current += 4;
		open  = *((double*)current);			current += 8;
		high  = *((double*)current);			current += 8;
		low   = *((double*)current);			current += 8;
		close = *((double*)current);			current += 8;
		tick_volume  = *((double*)current);		current += 8;
		spread       = 0;
		real_volume  = 0;
		
		cursor += struct_size;
		
		real_data.Add(open);
	}
	
	int max_size = 2*365*5/7*1440;
	
	int begin = real_data.GetCount() - max_size;
	real_data.Remove(0, begin);
	
	if (real_data.IsEmpty())
		Panic("No real data");
}


void Task::RunIndicator() {
	String dir = ConfigFile("AgentSessions");
	String file = AppendFileName(dir, symbol + " " + task + ".bin");
	RealizeDirectory(dir);
	if (FileExists(file)) {actual = 1; total = 1; return;}
	
	LoadData();
	
	Vector<FactoryDeclaration> decl;
	decl.Add(this->decl);
	System::GetCoreQueue(real_data, work_queue, decl);
	
	total = real_data.GetCount();
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		Core& c = *work_queue[i].core;
		
		for(int j = 1000; j < real_data.GetCount(); j+=1000) {
			actual = j;
			c.SetBars(j);
			c.Refresh();
		}
		
		actual = real_data.GetCount();
		c.SetBars(real_data.GetCount());
		c.Refresh();
	}
	
	Vector<ConstLabelSignal*> lbls;
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = work_queue[i];
		
		OutputCounter lc;
		ci.core->IO(lc);
		
		for(int j = 0; j < lc.lbl_counts.GetCount(); j++) {
			int count = lc.lbl_counts[j];
			for(int k = 0; k < count; k++) {
				ConstLabelSignal& buf = ci.core->GetLabelBuffer(j, k);
				lbls.Add(&buf);
			}
		}
	}
	
	BitStream stream;
	stream.SetColumnCount(lbls.GetCount());
	stream.SetCount(real_data.GetCount());
	stream.SetBit(0);
	for(int i = 0, actual = 0; i < real_data.GetCount(); i++, actual++) {
		for(int j = 0; j < lbls.GetCount(); j++) {
			bool value = lbls[j]->signal.Get(i);
			stream.Write(value);
		}
	}
	
	FileOut fout(file);
	fout % stream;
	
	work_queue.Clear();
	real_data.Clear();
}

void Task::RunBitstreamJoin() {
	String dir = ConfigFile("AgentSessions");
	String file = AppendFileName(dir, symbol + " " + task + ".bin");
	RealizeDirectory(dir);
	if (FileExists(file)) {actual = 1; total = 1; return;}
	
	Array<BitStream> streams;
	FindFile ff;
	int cols_total = 0;
	int size = 0;
	Vector<String> files;
	if (ff.Search(AppendFileName(dir, symbol + " Indicator*"))) {
		do {
			files.Add(ff.GetPath());
		}
		while (ff.Next());
	}
	Sort(files, StdLess<String>());
	
	
	for(int i = 0; i < files.GetCount(); i++) {
		BitStream& stream = streams.Add();
		FileIn fin(files[i]);
		fin % stream;
		cols_total += stream.GetColumnCount();
		stream.SetBit(0);
		if (streams.GetCount() == 1)
			size = stream.GetCount();
		else
			if (size != stream.GetCount())
				Panic("Indicator file size differs from first " + ff.GetName());
	}
	
	BitStream stream;
	stream.SetColumnCount(cols_total);
	stream.SetCount(size);
	stream.SetBit(0);
	
	total = size;
	for(int i = 0, actual = 0; i < size; i++, actual++) {
		for(int j = 0; j < streams.GetCount(); j++) {
			BitStream& src = streams[j];
			for(int k = 0; k < src.GetColumnCount(); k++) {
				bool value = src.Read();
				stream.Write(value);
			}
		}
	}
	
	FileOut fout(file);
	fout % stream;
}

void Task::RunHistoryMatch() {
	String dir = ConfigFile("AgentSessions");
	String file = AppendFileName(dir, symbol + " " + task + ".bin");
	RealizeDirectory(dir);
	if (FileExists(file)) {actual = 1; total = 1; return;}
	
	LoadData();
	
	BitStream stream;
	String stream_file = AppendFileName(dir, symbol + " Bitstream join.bin");
	FileIn stream_fin(stream_file);
	stream_fin % stream;
	if (stream.GetColumnCount() == 0 || stream.GetCount() == 0)
		Panic("Bitsream not joined yet");
	stream.SetBit(0);
	
	Vector<int> matches;
	matches.SetCount(real_data.GetCount());
	
	Vector<int64> desc;
	int desc_ints = stream.GetColumnCount() / 64;
	if (stream.GetColumnCount() % 64 != 0) desc_ints++;
	desc.SetCount(desc_ints * real_data.GetCount());
	
	int desc_id = 0;
	for(int i = 0; i < real_data.GetCount(); i++) {
		int pos = 0;
		for(int j = 0; j < desc_ints; j++) {
			int64 i64 = 0;
			for (int b = 0; b < 64 && pos < stream.GetColumnCount(); b++, pos++) {
				if (stream.Read())
					i64 |= 1 << b;
			}
			desc[desc_id++] = i64;
		}
	}
	ASSERT(desc_id == desc.GetCount());
	
	
	total = real_data.GetCount();
	for(int i = 0; i < real_data.GetCount(); i++) {
		actual = i;
		int min_dist = INT_MAX;
		int min_dist_pos = 0;
		
		for(int j = 0; j < i - 240; j++) {
			int k0 = i * desc_ints;
			int k1 = j * desc_ints;
			int dist = 0;
			for(int k = 0; k < desc_ints; k++) {
				int64 d0 = desc[k0];
				int64 d1 = desc[k1];
				dist += PopCount64(d0 ^ d1);
			}
			if (dist < min_dist) {
				min_dist = dist;
				min_dist_pos = j;
			}
		}
		matches[i] = min_dist_pos;
	}
	actual = 1; total = 1;
	
	
	FileOut fout(file);
	fout % matches;
	
	real_data.Clear();
	desc.Clear();
	matches.Clear();
}

void Task::RunOptimization() {
	String dir = ConfigFile("AgentSessions");
	String file = AppendFileName(dir, symbol + " " + task + ".bin");
	String mid_file = AppendFileName(dir, symbol + " " + task + " unfinished.bin");
	RealizeDirectory(dir);
	if (FileExists(file)) {actual = 1; total = 1; return;}
	
	LoadData();
	
	Vector<int> matches;
	String matches_file = AppendFileName(dir, symbol + " History match.bin");
	FileIn matches_fin(matches_file);
	matches_fin % matches;
	
	double point = real_data[0] > 65 ? 0.01 : 0.0001;
	
	Vector<bool> is_corner;
	is_corner.SetCount(real_data.GetCount());
	for(int i = 0; i < real_data.GetCount(); i++)
		is_corner[i] = IsCorner(i, point);
	
	FileIn mid_fin(mid_file);
	mid_fin % opt % results % result_values;
	
	
	if (opt.GetRound() == 0) {
		opt.min_value = -1;
		opt.max_value = +1;
		opt.SetMaxGenerations(10);
		int popcount = 100;
		opt.Init(1, popcount);
	}
	
	
	TimeStop ts;
	int cores = GetUsedCpuCores();
	int popcount = opt.GetPopulation();
	
	gen.SetCount(cores);
	result_values.SetCount(popcount);
	for(int i = 0; i < gen.GetCount(); i++) {
		Looper& g = gen[i];
		g.matches = &matches;
		g.real_data = &real_data;
		g.is_corner = &is_corner;
		g.Init(point, real_data);
	}
	
	
	total = opt.GetMaxRounds();
	
	while (!opt.IsEnd() && !Thread::IsShutdownThreads()) {
		actual = opt.GetRound();
		
		opt.Start();
		
		
		opt_result.SetCount(popcount, -DBL_MAX);
	
		CoWork co;
		co.SetPoolSize(cores);
		for(int i = 0; i < popcount; i++) {
			co & THISBACK1(RunOptimizationOnce, i);
		}
		co.Finish();
		
		opt.Stop(opt_result.Begin());
		/*
		Sort(results, OptResult());
		if (results.GetCount() > 100) results.SetCount(100);
		*/
		if (ts.Elapsed() > 5*60*1000) {
			FileOut fout(mid_file);
			fout % opt % results % result_values;
			ts.Reset();
		}
	}
	
	if (opt.IsEnd()) {
		FileOut fout(file);
		fout % results;
	}
	
	
	actual = 1;
	total = 1;
	opt_result.Clear();
	gen.Clear();
	results.Clear();
	result_values.Clear();
	real_data.Clear();
	//opt.Clear();
}

void Task::RunOptimizationOnce(int i) {
	
	while (true) {
		for(int j = 0; j < gen.GetCount(); j++) {
			Looper& g = gen[j];
			if (g.lock.TryEnter()) {
				g.params <<= opt.GetTrialSolution(i);
				if (i == 0) g.Run();
				result_values[i] = g.equity;
				
				g.lock.Leave();
				return;
			}
		}
	}
}

bool Task::IsCorner(int pos, double point) {
	int max_len = 15;
	int corner_steps = 4;
	
	double maxv = -DBL_MAX, minv = DBL_MAX;
	int max_pos, min_pos;
	
	for(int i = max(0, pos - max_len); i < pos; i++) {
		double d1 = real_data[i];
		if (d1 > maxv) {maxv = d1; max_pos = i;}
		if (d1 < minv) {minv = d1; min_pos = i;}
	}
	
	bool is_max_corner_right = false;
	for(int i = max_pos + 1; i <= pos; i++) {
		double d1 = real_data[i];
		if (d1 == maxv - corner_steps * point) {
			is_max_corner_right = i == pos;
			break;
		}
	}
	
	if (is_max_corner_right) {
		for(int i = max_pos - 1; i >= pos - max_len && i >= 0; i++) {
			double d1 = real_data[i];
			if (d1 == maxv - corner_steps * point) {
				return true;
			}
		}
	}
	
	bool is_min_corner_right = false;
	for(int i = min_pos + 1; i <= pos; i++) {
		double d1 = real_data[i];
		if (d1 == minv + corner_steps * point) {
			is_min_corner_right = i == pos;
			break;
		}
	}
	
	if (is_min_corner_right) {
		for(int i = min_pos - 1; i >= pos - max_len && i >= 0; i++) {
			double d1 = real_data[i];
			if (d1 == minv + corner_steps * point) {
				return true;
			}
		}
	}
	
	return false;
}

}
