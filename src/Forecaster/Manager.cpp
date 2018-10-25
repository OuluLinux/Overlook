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
	
	while (running && !Thread::IsShutdownThreads()) {
		
		int i = sessions.Find(active_session);
		if (i != -1) {
			Session& ses = sessions[i];
			
			ses.regen.Iterate();
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
	//#ifdef flagDEBUG
	int size = 1440*6*1;
	//int begin = real_data.GetCount() * 0.3;
	int begin = regen.real_data.GetCount() - size;
	regen.real_data.Remove(0, begin);
	regen.real_data.SetCount(size);
	//#endif
}

void Session::LoadThis() {
	
}

void Session::StoreThis() {
	
}

}
