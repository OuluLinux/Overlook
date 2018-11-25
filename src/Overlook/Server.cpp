#include "Overlook.h"

namespace Overlook {

String RandomPassword(int length) {
	String s;
	for(int i = 0; i < length; i++) {
		switch (Random(3)) {
			case 0: s.Cat('0' + Random(10)); break;
			case 1: s.Cat('a' + Random(25)); break;
			case 2: s.Cat('A' + Random(25)); break;
		}
	}
	return s;
}

Server::Server() {
	
}

void Server::Init() {
	GetForeSignal().WhenEvent << THISBACK(NewEvent);
	
	db.Init();
	
	running = true;
	Thread::Start(THISBACK(Process));
}

void Server::Start() {
	
}

void Server::Deinit() {
	running = false;
	listener.Close();
}

void Server::NewEvent(int level, String e) {
	String msg;
	switch (level) {
		case 0: msg += "info "; break;
		case 1: msg += "warning "; break;
		case 2: msg += "error "; break;
	}
	msg += e;
	for(int i = 0; i < db.GetUserCount(); i++) {
		UserDatabase& db = GetDatabase(i);
		db.lock.Enter();
		db.inbox.Add().data = msg;
		db.lock.Leave();
	}
}

void Server::Process() {
	while (!Thread::IsShutdownThreads() && running) {
		try {
			if(!listener.Listen(Config::server_port, 5)) {
				throw Exc("Unable to initialize server socket!");
				SetExitCode(1);
				return;
			}
			
			TimeStop ts;
			while (listener.IsOpen() && !Thread::IsShutdownThreads() && running) {
				One<ActiveSession> ses;
				ses.Create();
				ses->server = this;
				if(ses->s.Accept(listener)) {
					ses->s.Timeout(30000);
					
					// Close blacklisted connections
					if (blacklist.Find(ses->s.GetPeerAddr()) != -1)
						ses->s.Close();
					else {
						int id = session_counter++;
						ses->sess_id = id;
						
						// Normal session
						if (sessions.GetCount() < Config::server_max_sessions) {
							lock.EnterWrite();
							sessions.Add(id, ses.Detach()).Start();
							lock.LeaveWrite();
						}
					}
				}
				
				if (ts.Elapsed() > 1000) {
					lock.EnterWrite();
					for(int i = 0; i < sessions.GetCount(); i++) {
						if (!sessions[i].s.IsOpen()) {
							sessions.Remove(i);
							i--;
						}
					}
					lock.LeaveWrite();
					ts.Reset();
				}
			}
		}
		catch (Exc e) {
			Sleep(1000);
		}
	}
	
	for(int i = 0; i < sessions.GetCount(); i++)
		sessions[i].s.Close();
	
	for(int i = 0; i < sessions.GetCount(); i++)
		while (!sessions[i].stopped)
			Sleep(100);
	
	stopped = true;
}

int64 Server::GetNewLoginId() {
	while (true) {
		int64 login_id = (int64)Random(INT_MAX) | ((int64)Random(INT_MAX) << 32);
		if (login_session_ids.Find(login_id) == -1)
			return login_id;
	}
}



ServerCtrl::ServerCtrl() {
	
}

void ServerCtrl::Data() {
	
}

}
