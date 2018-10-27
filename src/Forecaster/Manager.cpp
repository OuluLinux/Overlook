#include "Forecaster.h"

namespace Forecast {

Manager::Manager() {
	RefreshSessions();
	
	#ifdef flagMAIN
	if (sessions.GetCount() == 0) {
		GetAdd("EURUSD");
		//GetAdd("EURJPY0");
		//GetAdd("GBPUSD0");
	}
	#endif
	
	Start();
}

Manager::~Manager() {
	
	Stop();
}

Session& Manager::GetAdd(String session_name) {
	Session& ses = sessions.GetAdd(session_name);
	ses.name = session_name;
	return ses;
}

void Manager::RefreshSessions() {
	String dir = ConfigFile("ForecastSessions");
	RealizeDirectory(dir);
	FindFile ff(AppendFileName(dir, "*.bin"));
	do {
		String fname = ff.GetName();
		if (fname == "") continue;
		Session& ses = sessions.GetAdd(fname);
		ses.name = GetFileTitle(fname);
		ses.LoadThis();
		if (ses.regen.real_data.IsEmpty())
			sessions.RemoveKey(fname);
	}
	while (ff.Next());
}

void Manager::Process() {
	int mode = 0;
	int ses_id = 0;
	while (running && !Thread::IsShutdownThreads()) {
		
		if (ses_id < sessions.GetCount()) {
			Session& ses = sessions[ses_id];
			active_session = ses_id;
			
			switch (mode) {
				case 0:
					if (!ses.regen.IsInit()) {
						ses.LoadData();
						ses.regen.Init();
					}
					#ifdef flagDEBUG
					ses.regen.Iterate(3*1000);
					#else
					ses.regen.Iterate(5*60*1000);
					#endif
					mode++;
					break;
				
				case 1:
					ses.StoreThis();
					mode++;
					break;
					
				case 2:
					if (ses.regen.opt.IsEnd())
						mode++;
					else
						mode = 0;
					break;
					
				case 3:
					if (ses.regen.nnsamples.IsEmpty())
						ses.regen.RefreshNNSamples(); // 1-shot, no fail allowed
					mode++;
					break;
					
				case 4:
					ses.StoreThis();
					mode++;
					break;
				
				case 5:
					#ifdef flagDEBUG
					ses.regen.IterateNN(3*1000);
					#else
					ses.regen.IterateNN(5*60*1000);
					#endif
					mode++;
					break;
					
				case 6:
					ses.StoreThis();
					mode++;
					break;
					
				case 7:
					if (ses.regen.dqn_iters < Regenerator::max_dqniters)
						mode = 5;
					else
						mode++;
					break;
				
				case 8:
					ses.regen.Forecast();
					break;
				
				default:
					ses_id++;
					if (ses_id >= sessions.GetCount())
						ses_id = 0;
					mode = 0;
			}
		} else {
			ses_id = 0;
		}
		
		Sleep(100);
	}
	
	stopped = true;
}













Session::Session() {
	
}

void Session::LoadData() {
	String dir = ConfigFile("m1data");
	String file = AppendFileName(dir, name + ".hst");
	
	FileIn src(file);
	if (!src.IsOpen() || !src.GetSize())
		Panic("Couldn't open history file for " + name);
	
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
	
	regen.real_data.Reserve(expected_count);
	
	
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
		
		regen.real_data.Add(open);
	}
	
	#ifdef flagDEBUG
	int max_size = 1440;
	#else
	//int max_size = 2*365*5/7*1440;
	int max_size = 4*5*1440;
	#endif
	
	int begin = regen.real_data.GetCount() - max_size;
	regen.real_data.Remove(0, begin);
}

void Session::LoadThis() {
	String dir = ConfigFile("ForecastSessions");
	String file = AppendFileName(dir, name + ".bin");
	RealizeDirectory(dir);
	LoadFromFile(*this, file);
}

void Session::StoreThis() {
	String dir = ConfigFile("ForecastSessions");
	String file = AppendFileName(dir, name + ".bin");
	RealizeDirectory(dir);
	StoreToFile(*this, file);
}

}
