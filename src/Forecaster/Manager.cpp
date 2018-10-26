#include "Forecaster.h"

namespace Forecast {

Manager::Manager() {
	RefreshSessions();
	
	#ifdef flagMAIN
	if (sessions.GetCount() == 0) {
		GetAdd("EURUSD0").LoadData();
		GetAdd("EURJPY0").LoadData();
		GetAdd("GBPUSD0").LoadData();
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
					if (!ses.regen.IsInit())
						ses.regen.Init();
					#ifdef flagDEBUG
					ses.regen.Iterate(3*1000);
					#else
					ses.regen.Iterate(5*60*1000);
					#endif
					mode++;
					break;
				
				case 1:
					ses.regen.Forecast();
					mode++;
					break;
				
				case 2:
					ses.StoreThis();
					mode++;
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
	FileIn fin(ConfigFile(name + ".bin"));
	
	regen.real_data.Clear();
	while (!fin.IsEof()) {
		double d;
		fin.Get(&d, sizeof(double));
		regen.real_data.Add(d);
	}
	if (regen.real_data.IsEmpty())
		Panic("No data for " + name);
	#ifdef flagDEBUG
	int size = 1440*1*1;
	#else
	int size = 1440*5*4;
	#endif
	int begin = regen.real_data.GetCount() - size;
	regen.real_data.Remove(0, begin);
	regen.real_data.SetCount(size);
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
