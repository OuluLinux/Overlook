#ifndef _Forecaster_Manager_h_
#define _Forecaster_Manager_h_

namespace Forecast {


struct Session : Moveable<Session> {
	
	// Persistent
	Regenerator regen;
	
	// Temporary
	String name;
	
	
	Session();
	void LoadData();
	void LoadThis();
	void StoreThis();
	bool HasData() {return !regen.real_data.IsEmpty();}
	void Serialize(Stream& s) {s % regen;}
};

class Manager {
	
protected:
	friend class ManagerCtrl;
	
	ArrayMap<String, Session> sessions;
	int active_session = -1, selected_session = 0;
	TimeCallback tc;
	TimeStop ts;
	bool stopped = true, running = false;
	
	void Process();
	
public:
	typedef Manager CLASSNAME;
	Manager();
	~Manager();
	
	void Start() {Stop(); stopped = false; running = true; Thread::Start(THISBACK(Process));}
	void Stop() {running = false; while (!stopped) Sleep(100);}
	
	void RefreshSessions();
	Session& GetAdd(String session_name);
	void Select(int i) {selected_session = i;}
	
	int GetSessionCount() const {return sessions.GetCount();}
	Session* GetActiveSession() {if (active_session < 0 || active_session >= sessions.GetCount()) return NULL; return &sessions[active_session];}
	Session* GetSelectedSession() {if (selected_session < 0 || selected_session >= sessions.GetCount()) return NULL; return &sessions[selected_session];}
};

inline Manager& GetManager() {return Single<Manager>();}

}

#endif
