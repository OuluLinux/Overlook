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
	
	AddTask("DQN Training");
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
	
	if (task == "DQN Training") {
		RunDqnAgent();
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

void Task::RunDqnAgent() {
	
	LoadData();
	
	int dqn_iters = 0;
	
	
	int input_size = 128;
	int act_count = 4;
	enum {LONG, SHORT, IDLE, STOP};
	
	if (dqn_iters == 0) {
		dqn.Init(1, input_size, act_count);
		dqn.Reset();
	}
	
	double reward_sum = 0.0;
	
	bool is_open = false, signal;
	double volume = 0.0;
	double accum_reward = 0.0;
	double open;
	
	int iter = input_size+1;
	Vector<double> input;
	input.SetCount(input_size);
	while (!Thread::IsShutdownThreads()) {
		
		
		double d0 = real_data[iter];
		for(int i = 0; i < input_size; i++) {
			double d1 = real_data[iter+1+i];
			double ch = (d0 / d1 - 1) * 1000;
			input[i] = ch;
		}
		
		int a = dqn.Act(input);
		double act_reward = 0.0;
		
		double prev_reward_sum = reward_sum;
		
		if (is_open) {
			if (signal == LONG) {
				if (a == LONG) {
					accum_reward += volume * (d0 / open - 1.0),
					open = d0;
					volume *= 2.0;
					if (volume > 10.0) volume = 10.0;
				}
				else if (a == SHORT) {
					accum_reward += volume * (d0 / (open * 1.0003) - 1.0),
					open = d0;
					reward_sum += accum_reward;
					act_reward = accum_reward;
					accum_reward = 0;
					volume = 0.01;
					signal = SHORT; is_open = true;
				}
				else if (a == STOP) {
					accum_reward += volume * (d0 / (open * 1.0003) - 1.0),
					reward_sum += accum_reward;
					act_reward = accum_reward;
					accum_reward = 0;
					is_open = false;
				}
			} else {
				if (a == SHORT) {
					accum_reward -= volume * (d0 / open - 1.0),
					open = d0;
					volume *= 2.0;
					if (volume > 10.0) volume = 10.0;
				}
				else if (a == LONG) {
					accum_reward -= volume * (d0 / (open * 0.9997) - 1.0),
					open = d0;
					reward_sum += accum_reward;
					act_reward = accum_reward;
					accum_reward = 0;
					volume = 0.01;
					signal = LONG; is_open = true;
				}
				else if (a == STOP) {
					accum_reward -= volume * (d0 / (open * 0.9997) - 1.0),
					reward_sum += accum_reward;
					act_reward = accum_reward;
					accum_reward = 0;
					is_open = false;
				}
			}
		} else {
			if (a == LONG || a == SHORT) {
				open = d0;
				volume = 0.01;
				signal = a;
				is_open = true;
			}
		}
		
		dqn.Learn(act_reward, act_reward != 0.0);
		
		if (prev_reward_sum != reward_sum) {
			//LOG(iter << "\t\t" << reward_sum);
			results.Add(reward_sum);
		}
		
		iter++;
		if (iter >= real_data.GetCount() - input_size-1)
			iter = input_size+1;
		dqn_iters++;
	}
}

}
