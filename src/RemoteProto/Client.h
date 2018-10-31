#ifndef _Client_Client_h
#define _Client_Client_h

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#define LAYOUTFILE <RemoteProto/Client.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS Images
#define IMAGEFILE <RemoteProto/Client.iml>
#include <Draw/iml_header.h>


void Print(const String& s);


class Client : public TopWindow {
	
	static const bool continuous = false;
	
	// Persistent
	int user_id = -1;
	String pass;
	bool is_registered = false;
	
	// Session
	String user_name;
	String addr = "127.0.0.1";
	One<TcpSocket> s;
	int64 login_id = 0;
	int port = 17000;
	int age = 0;
	bool gender = 0;
	bool is_logged_in = false;
	bool running = false, stopped = true;
	Mutex call_lock, lock;
	
	
	MenuBar menu;
	Splitter split;
	ArrayCtrl nearestlist;
	
public:
	typedef Client CLASSNAME;
	Client();
	~Client();
	
	int GetUserId() const {return user_id;}
	String GetPassword() const {return pass;}
	
	void Serialize(Stream& s) {s % user_id % pass % is_registered;}
	void StoreThis() {StoreToFile(*this, ConfigFile("Client" + IntStr64(GetServerHash()) + ".bin"));}
	void LoadThis() {LoadFromFile(*this, ConfigFile("Client" + IntStr64(GetServerHash()) + ".bin"));}
	unsigned GetServerHash() {CombineHash h; h << addr << port; return h;}
	
	void MainMenu(Bar& bar);
	bool Connect();
	void Disconnect();
	void CloseConnection() {if (!s.IsEmpty()) s->Close(); is_logged_in = false;}
	bool LoginScript();
	bool RegisterScript();
	void HandleConnection();
	void Start() {if (!stopped) return; stopped = false; running = true; Thread::Start(THISBACK(HandleConnection));}
	void Call(Stream& out, Stream& in);
	void SetAddress(String a, int p) {addr = a; port = p;}
	
	void Register();
	void Login();
	bool Set(const String& key, const String& value);
	void Get(const String& key, String& value);
	void Poll();
	
	void RefreshGui();
	void RefreshGuiChannel();
	void RefreshNearest();
	void Command(String cmd);
	
	bool IsConnected() {return is_logged_in;}
	
};

class PreviewImage : public ImageCtrl {
	void SetData(const Value& val) {
		String path = val;
		if(IsNull(path.IsEmpty()))
			SetImage(Null);
		else
			SetImage(StreamRaster::LoadFileAny(~path));
	}
};

class ServerDialog;


class RegisterDialog : public WithRegisterDialog<TopWindow> {
	ServerDialog& sd;
	
public:
	typedef RegisterDialog CLASSNAME;
	RegisterDialog(ServerDialog& sd);
	~RegisterDialog();
	
	void Register();
	void TryRegister();
};

class ServerDialog : public WithServerDialog<TopWindow> {
	
protected:
	friend class RegisterDialog;
	
	// Persistent
	String srv_addr;
	int srv_port;
	bool autoconnect = false;
	
	// Temporary
	Client& cl;
	bool connecting = false;
	RegisterDialog rd;
	
public:
	typedef ServerDialog CLASSNAME;
	ServerDialog(Client& c);
	
	void StartTryConnect() {Enable(false); Thread::Start(THISBACK(TryConnect));}
	void StartStopConnect() {Thread::Start(THISBACK(StopConnect));}
	void TryConnect();
	void StopConnect();
	void Close0() {Close();}
	void SetError(String e) {error.SetLabel(e);}
	void Enable(bool b);
	bool Connect(bool do_login);
	void Register();
	
	bool IsAutoConnect() const {return autoconnect;}
	
	void StoreThis() {StoreToFile(*this, ConfigFile("ClientServer.bin"));}
	void LoadThis() {LoadFromFile(*this, ConfigFile("ClientServer.bin"));}
	void Serialize(Stream& s) {s % srv_addr % srv_port % autoconnect;}
	
};


#endif
