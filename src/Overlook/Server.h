#ifndef _Overlook_Server_h_
#define _Overlook_Server_h_

namespace Overlook {

String RandomPassword(int length);

struct LogItem : Moveable<LogItem> {
	String msg;
	Time added;
	
	void Set(const String& s) {added = GetSysTime(); msg = s;}
	void Serialize(Stream& s) {s % msg % added;}
};

struct UserSessionLog : Moveable<LogItem> {
	Vector<LogItem> log;
	Time begin = Time(1970,1,1), end = Time(1970,1,1);
	String peer_addr;
	
	
	void Serialize(Stream& s) {s % log % begin % end % peer_addr;}
};

struct InboxMessage : Moveable<InboxMessage> {
	String data;
	
	void Serialize(Stream& s) {s % data;}
};

class UserDatabase {
	
protected:
	friend class ActiveSession;
	friend class Server;
	
	// Temporary
	Mutex lock;
	String user_file, user_loc_file;
	FileAppend location;
	int user_id = -1;
	bool open = false;
	
	
public:
	UserDatabase();
	int Init(int user_id);
	void Deinit();
	bool IsOpen() const {return open;}
	
	
	// Persistent
	Vector<InboxMessage> inbox;
	ArrayMap<int64, UserSessionLog> sessions;
	unsigned passhash = 0;
	Time joined, lastlogin;
	int64 logins = 0, onlinetotal = 0, visibletotal = 0;
	Time lastupdate;
	
	void Serialize(Stream& s) {s % inbox % sessions % passhash % joined % lastlogin % logins % onlinetotal % visibletotal % lastupdate;}
	void Flush();
	
};

class ServerDatabase {
	
protected:
	friend class Server;
	friend class ActiveSession;
	
	// Persistent
	VectorMap<int, String> users;
	
	// Temporary
	String srv_file;
	
public:
	ServerDatabase();
	
	void Init();
	int GetUserCount() {return users.GetCount();}
	void AddUser(int id, String user) {users.Add(id, user);}
	void SetUser(int user_id, String name) {users.GetAdd(user_id) = name;}
	String GetUser(int i) {return users[i];}
	
	void Serialize(Stream& s) {s % users;}
	void Flush();
	
};

UserDatabase& GetDatabase(int user_id);

class ActiveSession {
	
	static const int max_set_string_len = 1000000;
	
protected:
	friend class Server;
	
	Server* server = NULL;
	TcpSocket s;
	bool stopped = false;
	
	int sess_id = -1;
	int last_user_id = -1;
	int64 last_login_id = 0;
	
	
public:
	typedef ActiveSession CLASSNAME;
	ActiveSession();
	void Print(const String& s);
	void Run();
	void Start() {Thread::Start(THISBACK(Run));}
	void Stop() {s.Close();}
	
	void Greeting(Stream& in, Stream& out);
	void Register(Stream& in, Stream& out);
	void Login(Stream& in, Stream& out);
	void Logout();
	int  LoginId(Stream& in);
	void Poll(Stream& in, Stream& out);
	void Get(Stream& in, Stream& out);
};

class Server : public Common {
	
protected:
	friend class ActiveSession;
	
	// Persistent
	int session_counter = 0;
	Vector<LogItem> log;
	Vector<double> equity_history;
	
	
	// Temporary
	ArrayMap<int, ActiveSession> sessions;
	VectorMap<int, int> user_session_ids;
	VectorMap<int64, int> login_session_ids;
	TcpSocket listener;
	Index<String> blacklist;
	ServerDatabase db;
	RWMutex lock, msglock;
	SimBroker sb;
	Time shift = Null;
	bool running = false, stopped = true;
	
	
	
public:
	typedef Server CLASSNAME;
	Server();
	
	virtual void Init();
	virtual void Start();
	virtual void Deinit();
	
	void NewEvent(int level, String e);
	void Process();
	int64 GetNewLoginId();
};


class ServerCtrl : public CommonCtrl {
	
	
public:
	typedef ServerCtrl CLASSNAME;
	ServerCtrl();
	
	virtual void Data();
};


}

#endif
