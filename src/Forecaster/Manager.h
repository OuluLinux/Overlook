#ifndef _Forecaster_Manager_h_
#define _Forecaster_Manager_h_

namespace Forecast {

struct ForecastSession : Moveable<ForecastSession> {
	
};

struct Session : Moveable<Session> {
	
	Array<ForecastSession> forecasts;
	Regenerator regen;
	String name;
	
	
	Session();
	void LoadData();
	void LoadThis();
	void StoreThis();
	void Serialize(Stream& s) {}
};

class Manager {
	ArrayMap<String, Session> sessions;
	String active_session;
	bool stopped = true, running = false;
	
	void Process();
	
public:
	typedef Manager CLASSNAME;
	Manager();
	~Manager();
	
	void Start() {Stop(); stopped = false; running = true; Thread::Start(THISBACK(Process));}
	void Stop() {running = false; while (!stopped) Sleep(100);}
	
	void SetActiveSession(String s) {active_session = s;}
	void RefreshSessions();
	Session& GetAdd(String session_name);
	
	Session* GetActiveSession() {int i = sessions.Find(active_session); if (i == -1) return NULL; return &sessions[i];}
};

inline Manager& GetManager() {return Single<Manager>();}

}

#endif
