#include "Client.h"
#include <plugin/jpg/jpg.h>

namespace Config {

INI_STRING(master_addr, "overlook.zzz.fi", "Master server's address");
INI_INT(master_port, 17123, "Master server's port");

};

void Print(const String& s) {
	static Mutex lock;
	lock.Enter();
	Cout() << s;
	Cout().PutEol();
	LOG(s);
	lock.Leave();
}

double DegreesToRadians(double degrees) {
  return degrees * M_PI / 180.0;
}

double CoordinateDistanceKM(Pointf a, Pointf b) {
	double earth_radius_km = 6371.0;
	
	double dLat = DegreesToRadians(b.y - a.y);
	double dLon = DegreesToRadians(b.x - a.x);
	
	a.y = DegreesToRadians(a.y);
	b.y = DegreesToRadians(b.y);
	
	double d = sin(dLat/2) * sin(dLat/2) +
		sin(dLon/2) * sin(dLon/2) * cos(a.y) * cos(b.y);
	double c = 2 * atan2(sqrt(d), sqrt(1-d));
	return earth_radius_km * c;
}


Client::Client() {
	Icon(Images::icon());
	Title("F2F Client program");
	Sizeable().MaximizeBox().MinimizeBox();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(split.SizePos());
	split.Horz();
	
	
}

Client::~Client() {
	running = false;
	if (!s.IsEmpty()) s->Close();
	while (!stopped) Sleep(100);
}

void Client::MainMenu(Bar& bar) {
	bar.Sub("File", [=](Bar& bar) {
		
	});
	bar.Sub("Help", [=](Bar& bar) {
		
	});
	
}

bool Client::Connect() {
	if (s.IsEmpty() || !s->IsOpen()) {
		if (!s.IsEmpty()) s.Clear();
		s.Create();
		
		if(!s->Connect(addr, port)) {
			Print("Client " + IntStr(user_id) + " Unable to connect to server!");
			return false;
		}
	}
	return true;
}

void Client::Disconnect() {
	s.Clear();
}

bool Client::RegisterScript() {
	if (!is_registered) {
		try {
			Register();
			is_registered = true;
			StoreThis();
		}
		catch (Exc e) {
			return false;
		}
	}
	return true;
}

bool Client::LoginScript() {
	if (!is_logged_in) {
		try {
			Login();
			is_logged_in = true;
			PostCallback(THISBACK(RefreshGui));
		}
		catch (Exc e) {
			return false;
		}
	}
	return true;
}

void Client::HandleConnection() {
	Print("Client " + IntStr(user_id) + " Running");
	
	int count = 0;
	
	
	while (!Thread::IsShutdownThreads() && running) {
		
		if (continuous)
			Connect();
		
		try {
			while (!Thread::IsShutdownThreads() && running) {
				RegisterScript();
				LoginScript();
				Poll();
				
				Sleep(1000);
				count++;
			}
		}
		catch (Exc e) {
			Print("Client " + IntStr(user_id) + " Error: " + e);
			count = min(count, 1);
		}
		catch (const char* e) {
			Print("Client " + IntStr(user_id) + " Error: " + e);
			count = min(count, 1);
		}
		catch (...) {
			Print("Client " + IntStr(user_id) + " Unexpected error");
			break;
		}
		
		is_logged_in = false;
		
		if (continuous)
			Disconnect();
	}
	
	Print("Client " + IntStr(user_id) + " Stopping");
	stopped = true;
}

void Client::Call(Stream& out, Stream& in) {
	int r;
	
	out.Seek(0);
	String out_str = out.Get(out.GetSize());
	int out_size = out_str.GetCount();
	
	call_lock.Enter();
	
	if (!continuous) {
		s.Clear();
		Connect();
	}
	
	r = s->Put(&out_size, sizeof(out_size));
	if (r != sizeof(out_size)) {if (!continuous) Disconnect(); call_lock.Leave(); throw Exc("Data sending failed");}
	r = s->Put(out_str.Begin(), out_str.GetCount());
	if (r != out_str.GetCount()) {if (!continuous) Disconnect(); call_lock.Leave(); call_lock.Leave(); throw Exc("Data sending failed");}
	
	s->Timeout(30000);
	int in_size;
	r = s->Get(&in_size, sizeof(in_size));
	if (r != sizeof(in_size) || in_size < 0 || in_size >= 10000000) {if (!continuous) Disconnect(); call_lock.Leave(); call_lock.Leave(); throw Exc("Received invalid size");}
	
	String in_data = s->Get(in_size);
	if (in_data.GetCount() != in_size) {if (!continuous) Disconnect(); call_lock.Leave(); call_lock.Leave(); throw Exc("Received invalid data");}
	
	if (!continuous)
		Disconnect();
	
	call_lock.Leave();
	
	int64 pos = in.GetPos();
	in << in_data;
	in.Seek(pos);
}

void Client::Register() {
	StringStream out, in;
	
	out.Put32(10);
	
	Call(out, in);
	
	in.Get(&user_id, sizeof(int));
	pass = in.Get(8);
	if (pass.GetCount() != 8) throw Exc("Invalid password");
	
	Print("Client " + IntStr(user_id) + " registered (pass " + pass + ")");
}

void Client::Login() {
	StringStream out, in;
	
	out.Put32(20);
	out.Put32(user_id);
	out.Put(pass.Begin(), pass.GetCount());
	
	Call(out, in);
	
	int ret = in.Get32();
	if (ret != 0) throw Exc("Login failed");
	
	login_id = in.Get64();
	
	int name_len = in.Get32();
	if (name_len <= 0) throw Exc("Login failed");
	user_name = in.Get(name_len);
	
	age = in.Get32();
	gender = in.Get32();
	
	Print("Client " + IntStr(user_id) + " logged in (" + IntStr(user_id) + ", " + pass + ") nick: " + user_name);
}

bool Client::Set(const String& key, const String& value) {
	StringStream out, in;
	
	out.Put32(30);
	
	out.Put64(login_id);
	
	out.Put32(key.GetCount());
	out.Put(key.Begin(), key.GetCount());
	out.Put32(value.GetCount());
	out.Put(value.Begin(), value.GetCount());
	
	Call(out, in);
	
	int ret = in.Get32();
	if (ret == 1) {
		Print("Client " + IntStr(user_id) + " set " + key + " FAILED");
		return false;
	}
	else if (ret != 0) throw Exc("Setting value failed");
	
	Print("Client " + IntStr(user_id) + " set " + key);
	return true;
}

void Client::Get(const String& key, String& value) {
	StringStream out, in;
	
	out.Put32(40);
	
	out.Put64(login_id);
	
	out.Put32(key.GetCount());
	out.Put(key.Begin(), key.GetCount());
	
	Call(out, in);
	
	int value_len = in.Get32();
	value = in.Get(value_len);
	if (value.GetCount() != value_len) throw Exc("Getting value failed");
	
	int ret = in.Get32();
	if (ret != 0) throw Exc("Getting value failed");
	
	Print("Client " + IntStr(user_id) + " get " + key);
}

void Client::Poll() {
	StringStream out, in;
	
	out.Put32(80);
	
	out.Put64(login_id);
	
	Call(out, in);
	
	lock.Enter();
	
	int count = in.Get32();
	if (count < 0 || count >= 10000) {lock.Leave(); throw Exc("Polling failed");}
	for(int i = 0; i < count; i++) {
		int sender_id = in.Get32();
		int msg_len = in.Get32();
		String message;
		if (msg_len > 0)
			message = in.Get(msg_len);
		if (message.GetCount() != msg_len) {lock.Leave(); throw Exc("Polling failed");}
		Print("Client " + IntStr(user_id) + " received from " + IntStr(sender_id) + ": " + IntStr(message.GetCount()));
		
		int j = message.Find(" ");
		if (j == -1) continue;
		String key = message.Left(j);
		message = message.Mid(j + 1);
		
		/*if (key == "msg") {
			String ch_name = "user" + IntStr(sender_id);
			ASSERT(sender_id != this->user_id);
			User& u = users.GetAdd(sender_id);
			my_channels.FindAdd(ch_name);
			Channel& ch = channels.GetAdd(ch_name);
			ch.userlist.FindAdd(sender_id);
			ch.Post(sender_id, u.name, message);
			PostCallback(THISBACK(RefreshGui));
		}*/
	}
	
	
	lock.Leave();
}


void Client::RefreshGui() {
	lock.Enter();
	
	lock.Leave();
}

void Client::Command(String cmd) {
	if (cmd.IsEmpty()) return;
	
	if (cmd[0] == '/') {
		Vector<String> args = Split(cmd.Mid(1), " ");
		if (args.IsEmpty()) return;
		String key = args[0];
		/*if (key == "join") {
			if (args.GetCount() < 2) return;
			String ch_name = args[1];
			Join(ch_name);
			irc.SetActiveChannel(ch_name);
			try {
				RefreshUserlist();
			}
			catch (Exc e) {
			
			}
			RefreshGuiChannel();
		}*/
	}
}



























int PasswordFilter(int c) {
	return '*';
}

ServerDialog::ServerDialog(Client& c) : cl(c), rd(*this) {
	CtrlLayout(*this, "F2F select server");
	
	password.SetFilter(PasswordFilter);
	
	LoadThis();
	
	addr.SetData(srv_addr);
	port.SetData(srv_port);
	auto_connect.Set(autoconnect);
	
	reg <<= THISBACK(Register);
	
	addr.SetFocus();
	Enable(true);
}

void ServerDialog::Enable(bool b) {
	reg.Enable(b);
	username.Enable(b);
	password.Enable(b);
	addr.Enable(b);
	port.Enable(b);
	auto_connect.Enable(b);
	connect.Enable(b);
	if (b) {
		connect.WhenAction = THISBACK(StartTryConnect);
		connect.SetLabel(t_("Connect"));
	} else {
		connect.WhenAction = THISBACK(StartStopConnect);
		connect.SetLabel(t_("Stop"));
	}
}

void ServerDialog::TryConnect() {
	autoconnect = auto_connect.Get();
	srv_addr = addr.GetData();
	srv_port = port.GetData();
	StoreThis();
	
	if (srv_addr.IsEmpty())
		PostCallback(THISBACK1(SetError, "Server address is empty"));
	else if (Connect(true))
		PostCallback(THISBACK(Close0));
	else
		PostCallback(THISBACK1(SetError, "Connecting server failed"));
	PostCallback(THISBACK1(Enable, true));
}

bool ServerDialog::Connect(bool do_login) {
	srv_addr = addr.GetData();
	srv_port = port.GetData();
	cl.SetAddress(srv_addr, srv_port);
	cl.LoadThis();
	cl.CloseConnection();
	return cl.Connect() && cl.RegisterScript() && (!do_login || cl.LoginScript());
}

void ServerDialog::StopConnect() {
	cl.CloseConnection();
}

void ServerDialog::Register() {
	rd.Register();
	if (rd.Execute() == IDOK) {
		username.SetData(rd.username.GetData());
		password.SetData(rd.password.GetData());
		StoreThis();
	}
}






















RegisterDialog::RegisterDialog(ServerDialog& sd) : sd(sd) {
	CtrlLayoutOKCancel(*this, "Register to server");
}

RegisterDialog::~RegisterDialog() {
	
}
	
void RegisterDialog::Register() {
	Thread::Start(THISBACK(TryRegister));
}

void RegisterDialog::TryRegister() {
	if (sd.Connect(false)) {
		GuiLock __;
		username.SetData(sd.cl.GetUserId());
		password.SetData(sd.cl.GetPassword());
	}
}





