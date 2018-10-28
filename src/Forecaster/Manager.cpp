#include "Forecaster.h"
#include <plugin/bz2/bz2.h>

namespace Forecast {

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
			
			if (!ses.RunTask())
				ses_id++;
		} else {
			ses_id = 0;
		}
		
		Sleep(100);
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
	AddTask("Optimization");
	AddTask("DQN Sampling");
	AddTask("DQN Training");
	AddForecastTask(Vector<double>());
}

void Session::AddIndiTask(int id, FactoryDeclaration& decl) {
	Task& t = tasks.Add();
	t.task = "Indicator " + Format("%03d", id);
	t.decl = decl;
	t.symbol = symbol;
}

void Session::AddTask(String s) {
	Task& t = tasks.Add();
	t.task = s;
	t.symbol = symbol;
}

void Session::AddForecastTask(const Vector<double>& real_data) {
	Task& t = tasks.Add();
	t.task = "Forecast";
	t.symbol = symbol;
	t.real_data <<= real_data;
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
	else if (task == "Optimization") {
		RunOptimization();
	}
	else if (task == "DQN Sampling") {
		RunDQNSampling();
	}
	else if (task == "DQN Training") {
		RunDQNTraining();
	}
	else if (task == "Forecast") {
		RunForecast();
	}
	else Panic("Unknown task: " + task);
	
}

void Task::RunIndicator() {
	String dir = ConfigFile("ForecastSessions");
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
	String dir = ConfigFile("ForecastSessions");
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

void Task::RunOptimization() {
	String dir = ConfigFile("ForecastSessions");
	String file = AppendFileName(dir, symbol + " " + task + ".bin");
	String mid_file = AppendFileName(dir, symbol + " " + task + " unfinished.bin");
	RealizeDirectory(dir);
	if (FileExists(file)) {actual = 1; total = 1; return;}
	
	LoadData();
	
	int max_size = 4*5*1440;
	real_data.SetCount(max_size);
	double point = real_data[0] > 65 ? 0.01 : 0.0001;
	
	BitStream stream;
	String stream_file = AppendFileName(dir, symbol + " Bitstream join.bin");
	FileIn stream_fin(stream_file);
	stream_fin % stream;
	if (stream.GetColumnCount() == 0 || stream.GetCount() == 0)
		Panic("Bitsream not joined yet");
	
	
	FileIn fin(mid_file);
	fin % opt % results % result_errors;
	
	
	if (opt.GetRound() == 0) {
		opt.min_value = -1;
		opt.max_value = +1;
		opt.SetMaxGenerations(10);
		int popcount = 100;
		opt.Init(stream.GetColumnCount() * INDIPRESSURE_PARAMS + APPLYPRESSURE_PARAMS, popcount);
	}
	
	
	TimeStop ts;
	int cores = GetUsedCpuCores();
	int popcount = opt.GetPopulation();
	
	gen.SetCount(cores);
	for(int i = 0; i < gen.GetCount(); i++) {
		HeatmapLooper& g = gen[i];
		g.stream = stream;
		g.stream.SetBit(0);
		g.Init(point, real_data);
	}
	
	
	total = opt.GetMaxRounds();
	
	while (!opt.IsEnd() && !Thread::IsShutdownThreads()) {
		actual = opt.GetRound();
		
		opt.Start();
		
		
		err.SetCount(popcount, -DBL_MAX);
	
		CoWork co;
		co.SetPoolSize(cores);
		for(int i = 0; i < popcount; i++) {
			co & THISBACK1(RunOptimizationOnce, i);
		}
		co.Finish();
		
		opt.Stop(err.Begin());
		
		Sort(results, OptResult());
		if (results.GetCount() > 100) results.SetCount(100);
		
		if (ts.Elapsed() > 5*60*1000) {
			FileOut fout(mid_file);
			fout % opt % results % result_errors;
			ts.Reset();
		}
	}
	
	if (opt.IsEnd()) {
		FileOut fout(file);
		fout % results;
	}
	
	
	actual = 1;
	total = 1;
	err.Clear();
	gen.Clear();
	results.Clear();
	result_errors.Clear();
	real_data.Clear();
	//opt.Clear();
}

void Task::RunOptimizationOnce(int i) {
	while (true) {
		for(int j = 0; j < gen.GetCount(); j++) {
			HeatmapLooper& g = gen[j];
			if (g.lock.TryEnter()) {
				g.params <<= opt.GetTrialSolution(i);
				g.ResetPattern();
				g.CalculateError();
				
				ASSERT(g.err != -DBL_MAX);
				err[i] = -g.err;
				
				StringStream ss;
				ss.SetStoring();
				ss % g.image;
				ss.Seek(0);
				String heatmap = BZ2Compress(ss.Get(ss.GetSize()));
				
				
				result_lock.Enter();
				OptResult& rr = results.Add();
				rr.id = results.GetCount() - 1;
				rr.gen_id = j;
				rr.err = g.err;
				rr.heatmap = heatmap;
				rr.params <<= g.params;
				result_errors.Add(-g.err);
				result_lock.Leave();
				
				g.lock.Leave();
				return;
			}
		}
	}
}

void Task::RunDQNSampling() {
	String dir = ConfigFile("ForecastSessions");
	String file = AppendFileName(dir, symbol + " " + task + ".bin");
	RealizeDirectory(dir);
	if (FileExists(file)) {actual = 1; total = 1; return;}
	
	LoadData();
	double point = real_data[0] > 65 ? 0.01 : 0.0001;
	
	BitStream stream;
	String stream_file = AppendFileName(dir, symbol + " Bitstream join.bin");
	FileIn stream_fin(stream_file);
	stream_fin % stream;
	if (stream.GetColumnCount() == 0 || stream.GetCount() == 0)
		Panic("Bitsream not joined yet");
	
	String opt_file = AppendFileName(dir, symbol + " Optimization.bin");
	FileIn opt_fin(opt_file);
	opt_fin % results;
	Vector<Vector<double> > params;
	for(int i = 0; i < results.GetCount(); i++)
		params.Add() <<= results[i].params;
	
	
	l.Init(point, real_data, params, stream);
	
	l.WhenProgress << THISBACK(Progress);
	l.Run(true);
	
	FileOut fout(file);
	fout % l.nnsamples;
	
	l.nnsamples.Clear();
	real_data.Clear();
}

void Task::RunDQNTraining() {
	String dir = ConfigFile("ForecastSessions");
	String file = AppendFileName(dir, symbol + " " + task + ".bin");
	String file_mid = AppendFileName(dir, symbol + " " + task + " unfinished.bin");
	RealizeDirectory(dir);
	if (FileExists(file)) {actual = 1; total = 1; return;}
	
	String samp_file = AppendFileName(dir, symbol + " DQN Sampling.bin");
	FileIn samp_in(samp_file);
	samp_in % l.nnsamples;
	if (l.nnsamples.IsEmpty())
		Panic("No training samples");
	
	int iter = 0;
	FileIn mid_in(file_mid);
	mid_in % dqn % iter;
	
	if (iter == 0) {
		dqn.Init(1, NNSample::single_count * NNSample::single_size, NNSample::fwd_count);
		dqn.Reset();
	}
	
	const int max_iters = 1000000;
	
	total = max_iters;
	
	TimeStop ts;
	while (iter < max_iters && !Thread::IsShutdownThreads()) {
		actual = iter;
		
		NNSample& s = l.nnsamples[Random(l.nnsamples.GetCount())];
		dqn.Learn(&s.input[0][0], &s.output[0]);
		
		iter++;
		
		if (ts.Elapsed() >= 5*60*1000) {
			FileOut fout(file_mid);
			fout % dqn % iter;
			ts.Reset();
		}
	}
	
	if (iter >= max_iters) {
		FileOut fout(file);
		fout % dqn;
	}
	
	l.Clear();
}

void Task::RunForecast() {
	
	if (real_data.IsEmpty()) {
		LoadData();
		
		int max_size = 4*5*1440;
		int begin = real_data.GetCount() - max_size;
		real_data.Remove(0, begin);
	}
	
	
	Vector<FactoryDeclaration> decl;
	Vector<CoreItem> work_queue;
	AddDefaultDeclarations(decl);
	System::GetCoreQueue(real_data, work_queue, decl);
	
	total = work_queue.GetCount();
	for(int i = 0; i < work_queue.GetCount(); i++) {
		Core& c = *work_queue[i].core;
		actual = i;
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
	for(int i = 0; i < real_data.GetCount(); i++) {
		for(int j = 0; j < lbls.GetCount(); j++) {
			bool value = lbls[j]->signal.Get(i);
			stream.Write(value);
		}
	}
	
	
	
	String dir = ConfigFile("ForecastSessions");
	String opt_file = AppendFileName(dir, symbol + " Optimization.bin");
	FileIn opt_fin(opt_file);
	opt_fin % results;
	Vector<Vector<double> > params;
	for(int i = 0; i < results.GetCount(); i++)
		params.Add() <<= results[i].params;
	
	String dqn_file = AppendFileName(dir, symbol + " DQN Training.bin");
	FileIn dqn_fin(dqn_file);
	dqn_fin % dqn;
	
	double point = real_data[0] > 65 ? 0.01 : 0.0001;
	l.Init(point, real_data, params, stream);
	
	l.Run(false);
	
	NNSample s;
	l.GetSampleInput(s);
	
	dqn.Evaluate(&s.input[0][0], &s.output[0]);
	
	forecast.SetCount(NNSample::fwd_count, 0);
	for(int i = 0; i < NNSample::fwd_count; i++)
		forecast[i] = s.output[i];
	DUMPC(forecast);
	
	actual = 1;
	total = 1;
	
	real_data.Clear();
	l.Clear();
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
}
