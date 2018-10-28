#ifndef _Forecaster_Manager_h_
#define _Forecaster_Manager_h_

namespace Forecast {


struct DataRange {
	
};

struct Task : Moveable<Task> {
	
	// Temporary
	FactoryDeclaration decl;
	String task;
	String symbol;
	int actual = 0, total = 1;
	
	// Task temporary - to be freed
	Vector<OptResult> results;
	Vector<double> result_errors;
	Vector<double> real_data;
	Vector<double> err;
	Vector<double> forecast;
	Array<HeatmapLooper> gen;
	DQNAgent dqn;
	MultiHeatmapLooper l;
	Optimizer opt;
	Mutex result_lock;
	
	
	typedef Task CLASSNAME;
	Task();
	void Run();
	void RunIndicator();
	void RunBitstreamJoin();
	void RunOptimization();
	void RunDQNSampling();
	void RunDQNTraining();
	void RunForecast();
	void RunOptimizationOnce(int i);
	void LoadData();
};

struct Session : Moveable<Session> {
	
	// Temporary
	Vector<Task> tasks;
	String symbol;
	int active_task = 0;
	
	
	Session();
	void Init();
	
	void AddIndiTask(int factory, int arg=0);
	void AddTask(String s);
	void AddForecastTask(const Vector<double>& real_data);
	bool RunTask();
	
	void GetProgress(int& actual, int& total, String& state);
};

class Manager {
	
protected:
	friend class ManagerCtrl;
	
	ArrayMap<String, Session> sessions;
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
	Session& GetAdd(String symbol);
	
	Session& GetSession(int i) {return sessions[i];}
	int GetSessionCount() const {return sessions.GetCount();}
};

inline Manager& GetManager() {return Single<Manager>();}

}

#endif
