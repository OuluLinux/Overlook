#include "MT4ConnectionDll.h"
#include "Common.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason,  LPVOID lpReserved)
{
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		TLOG("Process attach");
		break;
	case DLL_PROCESS_DETACH:
		TLOG("Process detach");
		break;
	case DLL_THREAD_ATTACH:
		TLOG("Thread attach");
		break;
	case DLL_THREAD_DETACH:
		TLOG("Thread detach");
		break;
	}
	return TRUE;
}

struct QueueItem : public Moveable<QueueItem> {
	TcpSocket* sock;
	String content;
	bool status;
};

Vector<QueueItem*> queue;
Mutex lock;


#define TEST(x) if (!(x)) {TLOG("Perform " #x " failed"); delete sock; return;}

void Perform(TcpSocket* sock) {
	// RpcData& rpc
	
	QueueItem item;
	int got, len;
	got = sock->Get(&len, 4);
	TEST(got == 4);
	TEST(len > 0 && len < 256*256);
	item.content = sock->Get(len);
	TEST(item.content.GetCount() == len);
	item.status = false;
	item.sock = sock;
	
	TLOG("RPC: " << item.content);
	
	lock.Enter();
	queue.Add(&item);
	lock.Leave();
	
	// Busy lock
	while (!item.status) Sleep(3);
	
	delete sock;
}

bool rpc_started;
int rpc_ret, file_ret;

void RunRpcServer(int port) {
	TcpSocket rpc;
	if(!rpc.Listen(port, 5)) {
		rpc_ret = true;
	} else {
		for(;!Thread::IsShutdownThreads();) {
			TcpSocket* http = new TcpSocket();
			http->Timeout(3000);
			if(http->Accept(rpc))
				Perform(http);
			else delete http;
		}
		rpc_ret = false;
	}
	TLOG("RunRpcServer exiting, rpc_ret=" << (int)rpc_ret);
}

String file_path;

void FileRequest(TcpSocket* sock_ptr) {
	if (!sock_ptr) return;
	
	TcpSocket& sock = *sock_ptr;
	
	int r;
	
	#define CHK(x) if (!(x)) {sock.Close(); delete sock_ptr; TLOG("FileRequest fail: " + String(#x)); return;}

	int cmd = 0;
	r = StrGet(sock, &cmd, 4);
	CHK(r == 4);
	
	// Send file
	if (cmd == 1 || cmd == 2) {
		
		int offset = 0;
		if (cmd == 2) {
			r = StrGet(sock, &offset, 4);
			CHK(r == 4);
			CHK(offset >= 0);
		}
		
		int size;
		r = StrGet(sock, &size, 4);
		CHK(r == 4);
		
		CHK(size > 0 && size < 100000);
		
		String path = sock.Get(size);
		
		CHK(path.GetCount() == size);
		
		String file = AppendFileName(file_path, path);
		
		CHK(FileExists(file));
		
		FileIn in(file);
		size = in.GetSize();
		
		CHK(offset <= size);
		in.Seek(offset);
		size -= offset;
		
		r = StrPut(sock, &size, 4);
		CHK(r == 4);
		
		int chunk = 1024*1024;
		byte* buf = (byte*)MemoryAlloc(chunk);
		
		while (size >= chunk) {
			
			r = in.Get(buf, chunk);
			CHK(r == chunk);
			
			r = StrPut(sock, buf, chunk);
			CHK(r == chunk);
			
			size -= chunk;
		}
		
		if (size > 0) {
			
			r = in.Get(buf, size);
			CHK(r == size);
			
			r = StrPut(sock, buf, size);
			CHK(r == size);
			
		}
		
		delete buf;
	}
	
	delete sock_ptr;
}

void RunFileServer(int port) {
	TcpSocket fsrv;
	if(!fsrv.Listen(port, 5)) {
		file_ret = true;
	} else {
		for(;!Thread::IsShutdownThreads();) {
			TcpSocket* freq = new TcpSocket();
			freq->Timeout(3000);
			if(freq->Accept(fsrv))
				Thread::Start(callback1(FileRequest, freq));
			else
				delete freq;
		}
		file_ret = false;
	}
	TLOG("RunFileServer exiting, file_ret=" << (int)file_ret);
}

DllExport int ConnectionInit(int port, const char* file_path_cstr) {
	if (rpc_started) return 0;
	rpc_started = true;
	
	file_path = LoadCString(file_path_cstr);
	
	Thread::Start(callback1(RunFileServer, port + 100));
	Thread::Start(callback1(RunRpcServer,  port));
	
	Sleep(1000);
	if (rpc_ret)
		return 1;
	if (file_ret)
		return 1;
	return 0;
}

DllExport void ConnectionDeinit() {
	TLOG("ConnectionDeinit starting");
	Thread::ShutdownThreads();
	TLOG("ConnectionDeinit ready");
}





#undef TEST
#define TEST(x) if (!(x)) {TLOG("Putline " #x " failed"); item->status = true; return;}

DllExport void PutLine(char* c) {
	if (!queue.GetCount()) return;
	lock.Enter();
	QueueItem* item = queue[0];
	queue.Remove(0);
	lock.Leave();
	
	String str = LoadCString(c);
	TLOG("PutLine " << str);
	TcpSocket& sock = *item->sock;
	int len = str.GetCount();
	int got;
	got = sock.Put(&len, 4);
	TEST(got == 4);
	got = sock.Put(str.Begin(), len);
	TEST(got == len);
	
	item->status = true;
}

DllExport char* GetLine() {
	for(int i = 0; i < 1000; i++) {
		if (!queue.GetCount()) Sleep(3);
		else break;
	}
	
	if (!queue.GetCount()) {
		return StoreCString("");
	}
	
	lock.Enter();
	QueueItem* item = queue[0];
	lock.Leave();
	
	TLOG("Got line: " << item->content);
	return StoreCString(item->content);
}



