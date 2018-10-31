#ifndef _Agent_Manager_h_
#define _Agent_Manager_h_

namespace Agent {


struct DataRange {
	
};

struct Task : Moveable<Task> {
	
	// Temporary
	FactoryDeclaration decl;
	String task;
	String symbol;
	int actual = 0, total = 1;
	
	// Task temporary - to be freed
	Vector<CoreItem> work_queue;
	Vector<double> real_data, results, result_values, opt_result;
	Array<Looper> gen;
	DQNAgent dqn;
	Optimizer opt;
	
	
	typedef Task CLASSNAME;
	Task();
	void Run();
	void RunIndicator();
	void RunBitstreamJoin();
	void RunHistoryMatch();
	void RunOptimization();
	void RunOptimizationOnce(int i);
	bool IsCorner(int pos, double point);
	void LoadData();
	void Progress(int actual, int total) {this->actual = actual; this->total = total;}
	
};

struct Session : Moveable<Session> {
	
	// Temporary
	Array<Task> tasks;
	String symbol;
	int active_task = 0;
	
	
	Session();
	void Init();
	
	void AddIndiTask(int id, FactoryDeclaration& decl);
	void AddTask(String s);
	void AddForecastTask(const Vector<double>& real_data);
	bool RunTask();
	bool IsFinished();
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
	
	Session& GetAdd(String symbol);
	
	Session& GetSession(int i) {return sessions[i];}
	int GetSessionCount() const {return sessions.GetCount();}
};

inline Manager& GetManager() {return Single<Manager>();}

}

#endif
