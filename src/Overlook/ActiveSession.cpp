#include "Overlook.h"

namespace Overlook {

ActiveSession::ActiveSession() {
	
}

void ActiveSession::Print(const String& s) {
	if (last_user_id < 0 || last_login_id == 0) return;
	UserDatabase& db = GetDatabase(last_user_id);
	db.lock.Enter();
	UserSessionLog& ses = db.sessions.GetAdd(last_login_id);
	db.lock.Leave();
	if (ses.begin == Time(1970,1,1)) ses.begin = GetSysTime();
	ses.log.Add().Set(s);
}

void ActiveSession::Run() {
	Print("Session " + IntStr(sess_id) + " started handling socket from " + s.GetPeerAddr());
	int count = 0;
	
	struct NoPrintExc : public String {NoPrintExc(String s) : String(s) {}};
	
	String hex("0123456789ABCDEF");
	try {
		
		while (s.IsOpen()) {
			int r;
			int in_size = 0;
			r = s.Get(&in_size, sizeof(in_size));
			if (r != sizeof(in_size) || in_size < 0 || in_size >= 10000000) throw NoPrintExc("Received invalid size " + IntStr(in_size));
			
			String in_data = s.Get(in_size);
			if (in_data.GetCount() != in_size) throw Exc("Received invalid data");
			
			/*String hexdump;
			for(int i = 0; i < in_data.GetCount(); i++) {
				byte b = in_data[i];
				hexdump.Cat(hex[b >> 4]);
				hexdump.Cat(hex[b & 0xF]);
			}
			LOG("in: " + hexdump);*/
			
			MemReadStream in(in_data.Begin(), in_data.GetCount());
			StringStream out;
			
			int code;
			int test = SwapEndian32(-939524096);
			r = in.Get(&code, sizeof(code));
			if (r != sizeof(code)) throw Exc("Received invalid code");
			
			
			switch (code) {
				case 0:			Greeting(in, out); break;
				case 10:		Register(in, out); break;
				case 20:		Login(in, out); break;
				case 30:		Set(in, out); break;
				case 40:		Get(in, out); break;
				case 50:		Poll(in, out); break;
				
				default:
					throw Exc("Received invalid code " + IntStr(code));
			}
			
			out.Seek(0);
			String out_str = out.Get(out.GetSize());
			int out_size = out_str.GetCount();
			
			/*hexdump = "";
			for(int i = 0; i < out_str.GetCount(); i++) {
				byte b = out_str[i];
				hexdump.Cat(hex[b >> 4]);
				hexdump.Cat(hex[b & 0xF]);
			}
			LOG("out " + IntStr(out_str.GetCount()) + ":" + hexdump);*/
			
			
			r = s.Put(&out_size, sizeof(out_size));
			if (r != sizeof(out_size)) throw Exc("Data sending failed");
			r = s.Put(out_str.Begin(), out_str.GetCount());
			if (r != out_str.GetCount()) throw Exc("Data sending failed");
			
			count++;
		}
	}
	catch (NoPrintExc e) {
		
	}
	catch (Exc e) {
		Print("Error processing client from: " + s.GetPeerAddr() + " Reason: " + e);
	}
	catch (const char* e) {
		Print("Error processing client from: " + s.GetPeerAddr() + " Reason: " + e);
	}
	catch (...) {
		Print("Error processing client from: " + s.GetPeerAddr());
	}
	
	Logout();
	
	s.Close();
	
	stopped = true;
}

void ActiveSession::Greeting(Stream& in, Stream& out) {
	String title = Config::server_title;
	int i = title.GetCount();
	out.Put(&i, sizeof(int));
	out.Put(title.Begin(), title.GetCount());
	i = server->sessions.GetCount();
	out.Put(&i, sizeof(int));
	i = Config::server_max_sessions;
	out.Put(&i, sizeof(int));
}

void ActiveSession::Register(Stream& in, Stream& out) {
	
	server->lock.EnterWrite();
	
	int user_id = server->db.GetUserCount();
	String name = "User" + IntStr(user_id);
	
	String pass = RandomPassword(8);
	int64 passhash = pass.GetHashValue();
	Time now = GetUtcTime();
	
	UserDatabase& db = GetDatabase(user_id);
	
	server->db.AddUser(user_id, name);
	server->db.Flush();
	
	db.name = name;
	db.passhash = passhash;
	db.joined = now;
	db.lastlogin = Time(1970,1,1);
	db.logins = 0;
	db.onlinetotal = 0;
	db.visibletotal = 0;
	db.longitude = 0;
	db.latitude = 0;
	db.elevation = 0;
	db.lastupdate = Time(1970,1,1);
	db.Flush();
	
	last_user_id = user_id;
	
	server->lock.LeaveWrite();
	
	out.Put(&user_id, sizeof(user_id));
	out.Put(pass.Begin(), 8);
	
	Print("Registered " + IntStr(user_id) + " " + pass + " (hash " + IntStr64(passhash) + ")");
}

void ActiveSession::Login(Stream& in, Stream& out) {
	try {
		int user_id;
		int r = in.Get(&user_id, sizeof(user_id));
		if (r != sizeof(user_id)) throw Exc("Invalid login id");
		
		if (user_id < 0 || user_id >= server->db.GetUserCount()) throw Exc("Invalid login id");
		UserDatabase& db = GetDatabase(user_id);
		last_user_id = user_id;
		
		String pass = in.Get(8);
		if (pass.GetCount() != 8) throw Exc("Invalid login password");
		int64 passhash = pass.GetHashValue();
		
		int64 correct_passhash = db.passhash;
		if (passhash != correct_passhash || !correct_passhash) throw Exc("Invalid login password");
		
		server->lock.EnterWrite();
		server->user_session_ids.GetAdd(user_id, sess_id) = sess_id;
		int64 login_id = server->GetNewLoginId();
		server->login_session_ids.GetAdd(login_id, user_id);
		server->lock.LeaveWrite();
			
		db.logins++;
		db.Flush();
		
		out.Put32(0);
		
		out.Put64(login_id);
		
		out.Put32(db.name.GetCount());
		out.Put(db.name.Begin(), db.name.GetCount());
		out.Put32(db.age);
		out.Put32(db.gender);
		
		Print("Logged in");
	}
	 catch (Exc e) {
	    out.Put32(1);
	    Print("Login failed");
	 }
}

void ActiveSession::Logout() {
	if (last_user_id < 0)
		return;
	
	server->lock.EnterWrite();
	server->user_session_ids.RemoveKey(last_user_id);
	server->lock.LeaveWrite();
	
	if (last_login_id != 0) {
		UserDatabase& db = GetDatabase(last_user_id);
		UserSessionLog& ses = db.sessions.GetAdd(last_login_id);
		ses.end = GetSysTime();
	}
}

int ActiveSession::LoginId(Stream& in) {
	int64 login_id;
	int r = in.Get(&login_id, sizeof(login_id));
	if (r != sizeof(login_id)) throw Exc("Invalid login id");
	
	server->lock.EnterRead();
	int i = server->login_session_ids.Find(login_id);
	int user_id = -1;
	if (i >= 0) user_id = server->login_session_ids[i];
	server->lock.LeaveRead();
	if (user_id < 0)
		throw Exc("Invalid login id");
	
	last_login_id = login_id;
	last_user_id = user_id;
	
	return user_id;
}

void ActiveSession::Set(Stream& in, Stream& out) {
	int user_id = LoginId(in);
	
	UserDatabase& db = GetDatabase(user_id);
	int r;
	int key_len;
	String key;
	r = in.Get(&key_len, sizeof(key_len));
	if (r != sizeof(key_len) || key_len < 0 || key_len > 200) throw Exc("Invalid key argument");
	key = in.Get(key_len);
	if (key.GetCount() != key_len) throw Exc("Invalid key received");
	
	int value_len;
	String value;
	r = in.Get(&value_len, sizeof(value_len));
	if (r != sizeof(value_len) || value_len < 0 || value_len > max_set_string_len) throw Exc("Invalid value argument");
	value = in.Get(value_len);
	if (value.GetCount() != value_len) throw Exc("Invalid value received");
	
	int ret = 0;
	/*if (key == "name") {
		Print("Set name " + value);
		server->lock.EnterWrite();
		server->db.SetUser(user_id, value);
		server->db.Flush();
		server->lock.LeaveWrite();
		db.name = value;
		db.Flush();
		
		server->lock.EnterRead();
		Index<int> userlist;
		server->GetUserlist(userlist, user_id);
		userlist.RemoveKey(user_id);
		server->SendMessage(user_id, "name " + IntStr(user_id) + " " + value, userlist);
		server->lock.LeaveRead();
	}
	else if (key == "age") {
		Print("Set age " + value);
		db.age = ScanInt(value);
		db.Flush();
	}
	else if (key == "gender") {
		Print("Set gender " + value);
		db.gender = ScanInt(value);
		db.Flush();
	}
	else if (key == "profile_image") {
		Print("Set profile image");
		
		if (value.GetCount() > Config::max_image_size) throw Exc("Invalid image received");
		db.profile_img = value;
		db.profile_img_hash = ImageHash(db.profile_img);
		db.Flush();
		
		StoreImageCache(db.profile_img_hash, db.profile_img);
		
		server->SendToAll(user_id, "profile " + IntStr(user_id) + " " + value);
	}*/
	out.Put32(ret);
}

void ActiveSession::Get(Stream& in, Stream& out) {
	int user_id = LoginId(in);
	
	int r;
	int key_len;
	String key;
	r = in.Get(&key_len, sizeof(key_len));
	if (r != sizeof(key_len) || key_len < 0 || key_len > 200) throw Exc("Invalid key argument");
	key = in.Get(key_len);
	if (key.GetCount() != key_len) throw Exc("Invalid key received");
	
	const UserDatabase& db = GetDatabase(user_id);
	
	int64 size_pos = out.GetPos();
	out.SeekCur(sizeof(int));
	
	int i = key.Find(" ");
	Vector<String> args;
	if (i != -1) {
		args = Split(key.Mid(i+1), " ");
		key = key.Left(i);
	}
	
	/*if (key == "who") {
		int user_id = StrInt(args[0]);
		server->lock.EnterRead();
		Who(user_id, out);
		server->lock.LeaveRead();
	}*/
	
	out.Seek(size_pos);
	out.Put32(out.GetSize() - size_pos - 4);
	out.SeekEnd();
	out.Put32(0);
}

void ActiveSession::Poll(Stream& in, Stream& out) {
	int user_id = LoginId(in);
	
	UserDatabase& db = GetDatabase(user_id);
	
	Vector<InboxMessage> tmp;
	db.lock.Enter();
	Swap(db.inbox, tmp);
	db.lock.Leave();
	
	int count = tmp.GetCount();
	out.Put32(count);
	for(int i = 0; i < count; i++) {
		const InboxMessage& im = tmp[i];
		
		int data_len = im.data.GetCount();
		out.Put32(data_len);
		if (data_len > 0)
			out.Put(im.data.Begin(), data_len);
	}
}

}
