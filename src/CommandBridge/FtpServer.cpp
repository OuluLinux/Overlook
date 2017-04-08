#include "FtpServer.h"

#define MAX_BUFF 4096

FTPServer::FTPServer() {
	#ifdef flagWIN32
	port = 21;
	#else
	port = 10021;
	#endif
	running = false;
	stopped = true;
	client_counter = 0;
}

FTPServer::~FTPServer() {
	Stop();
}

void FTPServer::Start() {
	Stop();
	Thread::Start(THISBACK(Listener));
}

void FTPServer::Listener() {
	running = true;
	stopped = false;
	
	TcpSocket server;
	if(!server.Listen(port, 5)) {
		LOG("FTPServer: Unable to initialize server socket!");
		return;
	}
	
	LOG("FTPServer: listening port " << port);
	
	while (running) {
		int id = client_counter++;
		
		lock.Enter();
		TcpSocket& sock = sock_list.Add(id);
		lock.Leave();
		
		if (sock.Accept(server)) {
			lock.Enter();
			client_list.Add(id);
			thrd_list.Add(id).Run(THISBACK1(ServeClient, id));
			lock.Leave();
		}
		else {
			int pos = sock_list.Find(id);
			sock_list.Remove(pos);
		}
		
		// Clean finished clients
		lock.Enter();
		for(int i = 0; i < rm_queue.GetCount(); i++) {
			id = rm_queue[i];
			int pos = sock_list.Find(id);
			sock_list.Remove(pos);
			client_list.Remove(pos);
			sock_list.Remove(pos);
		}
		lock.Leave();
	}
	stopped = true;
}

void FTPServer::Stop() {
	running = false;
	while (!stopped) Sleep(100);
	
	for(int i = 0; i < client_list.GetCount(); i++) {
		client_list[i].Stop();
	}
	
}

void FTPServer::ServeClient(int id) {
	TcpSocket& sock = sock_list.Get(id);
	FTPClientConnection& cl = client_list.Get(id);
	
	cl.Process(sock);
	
	lock.Enter();
	rm_queue.Add(id);
	lock.Leave();
}








FTPClientConnection::FTPClientConnection() {
	running = true;
	stopped = false;
	passive = false;
	#ifdef flagWIN32
	cwd = "/C/";
	#else
	cwd = GetHomeDirectory();
	#endif
}

void FTPClientConnection::Response(String s) {
	LOG("--> " << s);
	sock->Put(s + "\n");
}

String FTPClientConnection::Scan() {
	String s;
	while (!sock->IsEof()) {
		char c;
		int got = sock->Get(&c, 1);
		if (got != 1) break;
		if (c == ' ' || c == '\n' || c == 13) break;
		s.Cat(c);
	}
	return s;
}

String FTPClientConnection::GetSysDir(String cwd) {
	Vector<String> parts = Split(cwd, "/");
	#ifdef flagWIN32
	if (parts.IsEmpty()) return "C:" DIR_SEPS;
	String path = parts[0] + ":";
	if (parts.GetCount() == 1) path << DIR_SEPS;
	#else
	if (parts.IsEmpty()) return DIR_SEPS;
	String path = DIR_SEPS + parts[0];
	#endif
	for (int i = 1; i < parts.GetCount(); i++) {
		path << DIR_SEPS << parts[i];
	}
	return path;
}

String FTPClientConnection::AppendCwd(String s) {
	Vector<String> parts = Split(cwd, "/");
	String new_cwd;
	if (s == "..") {
		if (!parts.IsEmpty()) {
			parts.Remove(parts.GetCount()-1);
		}
		new_cwd = "/" + Join(parts, "/");
	} else {
		Vector<String> new_parts = Split(s, "/");
		parts.Append(new_parts);
		new_cwd = "/" + Join(parts, "/");
	}
	return new_cwd;
}

String FTPClientConnection::AppendCwdSys(String s) {
	return GetSysDir(AppendCwd(s));
}

void FTPClientConnection::Stop() {
	running = false;
	while (!stopped) Sleep(100);
}

String FTPClientConnection::ChangeDirectory(String s) {
	String new_cwd;
	if (s.Left(1) == "/")
		new_cwd = s;
	else
		new_cwd = AppendCwd(arg);
	return new_cwd;
}

String FTPClientConnection::ChangeDirectorySys(String s) {
	return GetSysDir(ChangeDirectory(s));
}

void FTPClientConnection::CheckPassive() {
	if (passive) {
		TcpSocket* sock = new TcpSocket();
		sock->Accept(*data_socket);
		data_socket.Clear();
		data_socket = sock;
	}
}

void FTPClientConnection::Process(TcpSocket& sock) {
	this->sock = &sock;
	
	Response("220 " + sock.GetHostName() + " FTP server (Version 6.00LS) ready.");
	
	while (running && sock.IsOpen()) {
		
		command = Scan();
		if (command.IsEmpty()) {
			Sleep(10);
			continue;
		}
		
		LOG("<-- " << command);
		
		if (command == "USER") {
			arg = Scan();
			LOG("(USER): " << arg);
			if (arg == "funk") {
				Response("331 User name ok, need password.");
			}
			else {
				Response("332 Need account for login.");
			}
		}
		
		else if (command == "PASS") {
			arg = Scan();
			LOG("(PASS): " << arg);
			if (arg == "123123") {
				Response("230 User logged in, proceed.");
			}
			else {
				Response("530 Not logged in.");
			}
		}
		
		else if (command == "SYST" || command == "syst") {
			LOG("(SYST):");
			//Response("215 UNIX Type: L8.");
			Response("215 UNIX Type: L8 Version: BSD-199506");
		}
		
		else if (command == "PWD") {
			LOG("(PWD):SHOW");
			Response("257 \"" + cwd + "\"");
		}
		
		else if (command == "CWD") {
			arg = Scan();
			LOG("(CWD): " << arg);
			String new_cwd = ChangeDirectory(arg);
			String sysdir = GetSysDir(new_cwd);
			if (DirectoryExists(sysdir)) {
				cwd = new_cwd;
				Response("250 Directory successfully changed.");
			}
			else {
				Response("550 Failed to change directory.");
			}
		}
		
		else if (command == "RNFR") {
			arg = Scan();
			LOG("(RNFR): " << arg);
			Response("350 Requested file action pending further information.");
		}
		
		else if (command == "RNTO") {
			arg2 = Scan();
			LOG("(RNTO): " << arg2);
			
			String path1 = ChangeDirectorySys(arg);
			String path2 = ChangeDirectorySys(arg2);
			if (!FileExists(path1) || FileExists(path2)) {
				Response("550 Failed to rename file.");
			} else {
				if (rename(path1, path2) >= 0) {
					Response("250 File successfully renamed.");
				} else {
					Response("550 Failed to rename file.");
				}
			}
		}
		
		else if (command == "DELE") {
			arg = Scan();
			LOG("(DELE): " << arg);
			
			String path = ChangeDirectorySys(arg);
			if (!FileExists(path) || !FileDelete(path)) {
				Response("550 Failed to remove file.");
			} else {
				Response("250 File successfully removed.");
			}
		}
		
		else if (command == "MKD") {
			arg = Scan();
			LOG("(MKD): " << arg);
			
			String path = ChangeDirectorySys(arg);
			RealizeDirectory(path);
			if (DirectoryExists(path)) {
				Response("550 Failed to create directory.");
			} else {
				Response("257 Directory successfully created.");
			}
		}
		
		else if (command == "RMD") {
			arg = Scan();
			LOG("(RMD): " << arg);
			
			String path = ChangeDirectorySys(arg);
			#ifdef flagWIN32
			if (RemoveDirectory(path))
			#else
			if (unlink(path))
			#endif
			{
				Response("550 Failed to remove directory.");
			} else {
				Response("250 Directory successfully removed.");
			}
		}
		
		else if (command == "LIST") {
			LOG("(LIST): SHOW");
			Response("125 Data connection already open; transfer starting");
			
			CheckPassive();
			
			String path = GetSysDir(cwd);
			FindFile ff(AppendFileName(path, "*"));
			while (ff) {
				char line[1024];
				
				const char* permstr = ff.IsDirectory() ? "drwxr-xr-x+" : "-rwxr-xr-x+";
				const char* username = "funk";
				int length = ff.GetLength();
				String filename = ff.GetName();
				
				sprintf(line, "%s   1 %-10s %-10s %10lu Jan  1  1980\t%s\r\n",
				    permstr, username, username,
				    length,
				    filename.Begin());
				
				data_socket->Put(line);
				ff.Next();
			}
			
			Response("250 Closing data connection. Requested file action successful.");
			data_socket->Close();
		}
		
		else if (command == "SIZE") {
			arg = Scan();
			LOG("(SIZE): " << arg);
			
			String path = ChangeDirectorySys(arg);
			FindFile ff(path);
			String size = ff ? IntStr(ff.GetLength()) : "0";
			
			Response("213 " + size);
		}
		
		else if (command == "MDTM") {
			arg = Scan();
			LOG("(MDTM): " << arg);
			
			String path = ChangeDirectorySys(arg);
			FindFile ff(path);
			Time t = ff ? ff.GetLastWriteTime() : Time(1970,1,1);
			
			// YYYYMMDDhhmmss
			String time = Format("%04d%02d%02d%02d%02d%02d",
				(int)t.year, (int)t.month, (int)t.day,
				(int)t.hour, (int)t.minute, (int)t.second);
			
			Response("213 " + time);
		}
		else if (command == "TYPE") {
			arg = Scan();
			LOG("(TYPE): " << arg);
			
			if (arg == "A") {
				Response("200 Switching to ASCII mode.");
			}
			else if (arg == "I") {
				Response("200 Switching to Binary mode.");
			}
			else if (arg == "L") {
				Response("200 Switching to Tenex mode.");
			}
			else {
				Response("501 Syntax error in parameters or arguments.");
			}
		}
		
		else if (command == "PORT") {
			arg = Scan();
			LOG("(PORT): " << arg);
			
			passive = false;
			
			Vector<String> parts = Split(arg, ",");
			if (parts.GetCount() == 6) {
				String addr = parts[0] << "." << parts[1] << "." << parts[2] << "." << parts[3];
				unsigned int port[2];
				port[0] = StrInt(parts[4]);
				port[1] = StrInt(parts[5]);
				uint16 port_v = port[0] << 8 | port[1];
				data_socket.Clear();
				data_socket.Create();
				if (!data_socket->Connect(addr, port_v)) {
					LOG("Couldn't connect " << addr << ":" << port_v);
				}
				Response("200 Okey");
			}
		}
		
		else if (command == "PASV") {
			passive = true;
			data_socket.Clear();
			data_socket.Create();
			
			int port = -1;
			for(int i = 0; i < 100; i++) {
				port = 10020 + i;
				if (data_socket->Listen(port, 5)) {
					break;
				}
			}
			// TODO: resolve listening address
			Response("227 Entering Passive Mode " +
				Format("(127,0,0,1,%d,%d)", port >> 8, port & 0xff));
		}
		
		else if (command == "RETR") {
			arg = Scan();
			LOG("(RETR): " << arg);
			
			String path = ChangeDirectorySys(arg);
			FileIn in(path);
			if (!in.Open(path)) {
				Response("450 Requested file action not taken. File unavaible.");
			}
			else {
				Response("150 File status okay; about to open data connection.");
				
				CheckPassive();
				
				byte buf[MAX_BUFF];
				while (!in.IsEof()) {
					int got = in.Get(buf, MAX_BUFF);
					data_socket->Put(buf, got);
					if (got != MAX_BUFF) break;
				}
				
				Response("226 Closing data connection. Requested file action successful.");
				data_socket->Close();
			}
		}
		
		else if (command == "STOR") {
			arg = Scan();
			LOG("(STOR): " << arg);
			
			String path = ChangeDirectorySys(arg);
			FileOut out(path);
			if (!out.IsOpen()) {
				Response("450 Requested file action not taken. File unavaible.");
			}
			else {
				Response("150 File status okay; about to open data connection.");
				
				CheckPassive();
				
				byte buf[MAX_BUFF];
				while (!data_socket->IsEof()) {
					int got = data_socket->Get(buf, MAX_BUFF);
					out.Put(buf, got);
					if (got != MAX_BUFF) break;
				}
				
				Response("226 Closing data connection. Requested file action successful.");
				data_socket->Close();
			}
		}
		
		else if (command == "QUIT") {
			Response("221 Service closing control connection");
			Stop();
		}
		
		// Extensions
		
		else if (command == "opts") {
			arg = Scan();
			arg2 = Scan();
			LOG("(opts): " << arg << " " << arg2);
			
			Response("500 OPTS " + arg + " " + arg2 + ": command not understood.");
		}
		
		else if (command == "site") {
			arg = Scan();
			LOG("(opts): " << arg);
			
			Response("214 Direct comments to ftp-bugs@desk.");
		}
		
		else {
			Response("502 Command not implemented.");
			LOG("Invalid command: " << command);
		}
	}
	
	sock.Close();
	running = false;
	stopped = true;
}
