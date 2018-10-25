#include "Forecaster.h"

namespace Forecast {

Manager::Manager() {
	
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
	while (ff.Next()) {
		String fname = ff.GetName();
		LOG(fname);
		Session& ses = sessions.GetAdd(fname);
		ses.name = fname;
		ses.LoadThis();
	}
}

void Manager::Process() {
	int mode = 0;
	while (running && !Thread::IsShutdownThreads()) {
		
		int i = sessions.Find(active_session);
		if (i != -1) {
			Session& ses = sessions[i];
			
			switch (mode) {
				case 0:
					if (!ses.regen.IsInit())
						ses.regen.Init();
					ses.regen.Iterate(10*1000);
					mode++;
					break;
				
				case 1:
					ses.regen.Forecast();
					mode++;
					break;
					
				default:
					mode = 0;
			}
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
	
	int size = 1440*5*4;
	int begin = regen.real_data.GetCount() - size;
	regen.real_data.Remove(0, begin);
	regen.real_data.SetCount(size);
}

void Session::LoadThis() {
	
}

void Session::StoreThis() {
	
}

}
