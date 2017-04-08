#include "Ftp.h"

#ifdef PLATFORM_WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

NAMESPACE_UPP

static bool trace_ftp;
#define LLOG(x)	do{ if(trace_ftp) RLOG(x); }while(false)

void Ftp::Trace(bool b)
{
	trace_ftp = b;
}

String Slice(const String& s, int delim1, int delim2) 
{
	int b = -1, e = -1;
	if((b = s.Find(delim1, 0)) == -1 || (e = s.Find(delim2, ++b)) == -1) 
	    return String::GetVoid();
	return s.Mid(b, e - b);
}

String Ftp::DecodePath(const String& path)
{
	int b = -1, e = -1;
	String r, d;
	if((b = path.Find('\"')) == -1 || (e = path.ReverseFind('\"')) == -1 || b == e)
		return String::GetVoid();
	d = path.Mid(b + 1, (e - 1) - b);
	for(int i = 0; i < d.GetLength(); i++) {
		if(d[i] == '\"' && d[i + 1] == '\"') {
			r.Cat(d[++i]);
			continue;
		}
		r.Cat((d[i] == '\0') ? '\n' : d[i]);
	}
	return r;
}

String Ftp::EncodePath(const String& path)
{
	String r;
	for(int i = 0; i < path.GetLength(); i++) 
		r.Cat((path[i] == '\n') ? '\0' : path[i]);
	return r;
}

int Ftp::GetRandomPort(int min, int max)
{
	int range = max - min + 1;
	return (int) Random() % range + min;
}

bool Ftp::GetSockAddr(int type, int& family, String& ip, int& port)
{
	struct sockaddr_storage ss;
	struct sockaddr_in *in = (sockaddr_in*) &ss;
	struct sockaddr_in6 *in6 = (sockaddr_in6*) &ss;
	socklen_t ss_size = sizeof(sockaddr_storage);
	memset(&ss, 0, ss_size);
	if(type == LOCAL) {
		if(getsockname(control_socket.GetSOCKET(), (sockaddr*) &ss, &ss_size) != 0)	
			return false;
	}
	else
	if(type == PEER) {
		if(getpeername(control_socket.GetSOCKET(), (sockaddr*) &ss, &ss_size) != 0)	
			return false;
	}
	family = ss.ss_family;
#ifdef PLATFORM_WIN32
	ip = inet_ntoa(in->sin_addr);
#else
	Buffer<char> dummy(64, 0), ip_buffer(16, 0);
	if(family == AF_INET6)
		memcpy(ip_buffer, &in6->sin6_addr, sizeof(in6_addr));
	else
		memcpy(ip_buffer, &in->sin_addr, sizeof(in_addr));			
	ip = inet_ntop(family, ip_buffer, dummy, 64);	
#endif
	if(ip.IsEmpty())
		return false;
	port = GetRandomPort(49152, 65535);
	return true;	
}

bool Ftp::SetError(const String& e)
{
	reply = e;
	reply_code = -1;
	if(control_socket.IsError()) {
		String socket_error = control_socket.GetErrorDesc();
		if(!socket_error.IsEmpty())
			reply << " [" << socket_error << "]";
	}
	LLOG("-- " << reply);
	return progress = false;
}

int Ftp::GetReplyCode(const String& s)
{
	if(s.IsVoid() || s.GetLength() < 3 || !IsDigit(s[0]) || !IsDigit(s[1]) || !IsDigit(s[2]))
		return -1;
	return StrInt(s.Mid(0, 3));
}

String Ftp::GetReplyAsXml()
{
	StringStream ss(reply);
	String output;

	while(!ss.IsEof()) {
		String s = ss.GetLine();
		int rc = GetReplyCode(s);
		XmlTag xml;
		xml.Tag("reply")("code", rc)("type", rc > 0 ? "protocol" : "internal");
		if(rc > 0 && s[3] == '-') {
			String ll;
			for(bool eof = false;;) {
				ll << XmlTag("line").Text(TrimBoth(rc > 0 ? s.Mid(4) : s));
				if((rc > 0 && s[3] == ' ') || eof)
					break;
				s = ss.GetLine();
				rc = GetReplyCode(s);
				eof = ss.IsEof();
			}
			output << xml(ll);
		}
		else
			output << xml(XmlTag("line").Text(TrimBoth(rc > 0 ? s.Mid(4) : s)));
	}
	return output;
}

bool Ftp::PutGet(const String& s, bool nolog)
{
	progress = true;
	// Put request.
	if(!s.IsEmpty()) {
		if(!nolog) 
			LLOG(">> " << s);
		if(!control_socket.PutAll(s + "\r\n")) { 
			return SetError("Ftp::PutGet(): Write failed.");
		}
	}
	// Get response
	reply = control_socket.GetLine();
	reply_code = GetReplyCode(reply);
	if(reply_code == -1) {
		return SetError("Ftp::PutGet(): Read failed.");
	}
	LLOG("<< " << reply);
	reply.Cat("\r\n");
	if(reply[3] && reply[3] == '-') {
		for(;;) {
			String line = control_socket.GetLine();
			if(line.IsVoid()) {
				return SetError("Ftp::PutGet(): Read failed.");
			}
			int end_code = GetReplyCode(line);
			LLOG("<< " << line);
			reply.Cat(line);
			reply.Cat("\r\n");
			if(reply_code == end_code && line[3] && line[3] == ' ')
				break;
		}
	}
	progress = false;
	return true;	
}

bool Ftp::SetDataType(int type)
{
	PutGet(type == BINARY ? "TYPE I" : "TYPE A");
	return ReplyIsSuccess();
}

bool Ftp::SetTransferMode(String& addr, int& port, int& family)
{
	bool result = false;
	switch(transfer_mode) {
		case ACTIVE: {
			if(!GetSockAddr(LOCAL, family, addr, port))
				break;
			// Try extendend command first.
			String xcmd;
			xcmd << (family == AF_INET6 ? "|2|" : "|1|") << addr << "|" << port << "|";
			PutGet("EPRT " + xcmd);
			if(ReplyIsSuccess()) {
				result = true;
				break;
			}
			else
			if(family == AF_INET6) 
				break;
			addr.Replace(".", ",");
			addr += "," + AsString((port & 0xff00) >> 8) + "," + AsString(port & 0xff);	
			PutGet("PORT " + addr);
			result = ReplyIsSuccess();
			break;				
		}
		case PASSIVE: {
			if(!GetSockAddr(PEER, family, addr, port))
				break;
			// Try extendend command first.
			PutGet("EPSV");
			if(ReplyIsSuccess()) {
				addr = control_socket.GetPeerAddr();
				int begin = reply.FindAfter("(|||");
				int end = reply.ReverseFind("|)");
				if(begin == -1 || end == -1 || addr.IsEmpty())
					break;
				port = StrInt(reply.Mid(begin, end - begin));
				result = true;
				break;
			}
			else
			if(family == AF_INET6)
				break;
			PutGet("PASV");
			if(!ReplyIsSuccess()) 
				break;
			String h1, h2, h3, h4, p1, p2;	
			if((!(addr = Slice(reply, '(', ')')).IsEmpty()  || 
		    	!(addr = Slice(reply, '=', ' ')).IsEmpty()) &&
				SplitTo(addr, ',', h1, h2, h3, h4, p1, p2)) {
			    addr = h1 + "." + h2 + "." + h3 + "." + h4;
				port = StrInt(p1) * 256 + StrInt(p2);
				result = true;
			}
			break;
		}
		default:
			NEVER();
	}
	return result;
}

bool Ftp::TransferData(int cmd, const String& request, Stream& file, int64 size, int type)
{
	if(ftps && transfer_mode == ACTIVE)
		return SetError("FTPS mode is not supported with active ftp connection.");
	String addr;
	int port, family;
	bool log = request.StartsWith("LIST");
	if(!SetTransferMode(addr, port, family))
		return false;
	TcpSocket data_socket, listener;
	data_socket.Timeout(control_socket.GetTimeout());
	data_socket.WaitStep(control_socket.GetWaitStep());
	data_socket.WhenWait = Proxy(WhenWait);
	target = &data_socket;
	switch(transfer_mode) {
		case ACTIVE: {
			if(!listener.Listen(port, 5, family == AF_INET6))
				break;
			PutGet(request);
			if(!ReplyIsWait())
				break;
			for(;;) 
				if(data_socket.Accept(listener)) {
					cmd == GET	
					? GetData(data_socket, file, size, type, log)
					: PutData(data_socket, file, type);
					break;		
				}
			break;
		}
		case PASSIVE: {
			if(!data_socket.Connect(addr, port))
				break;
			if(ftps) 
				if(!data_socket.StartSSL() || !data_socket.IsSSL()) {
					LLOG("-- Negotiation error. Couldn't put data socket into FTPS mode.");
					break;
				}
			PutGet(request);
			if(!ReplyIsWait())
				break;
			cmd == GET	
				? GetData(data_socket, file, size, type, log)
				: PutData(data_socket, file, type);
			break;
		}
		default:
			NEVER();
	}
	target = NULL;
	if(!aborted) {
		if(data_socket.IsOpen())
			data_socket.Close();
		PutGet(Null);
		return ReplyIsSuccess();
	}
	else 
		return aborted = false;
}

bool Ftp::GetData(TcpSocket& socket, Stream& out, int64 sz, int type, bool log)
{
	progress = true;
	while(!socket.IsEof()) {
		String chunk = type == ASCII ? socket.GetLine() : socket.Get(chunk_size);
		if(chunk.IsVoid())
			continue;
		if(type == ASCII) {
			if(log)
				LLOG("|| " << chunk);
#ifdef PLATFORM_WIN32
			chunk.Cat('\r');
#endif
			chunk.Cat('\n');
		}
		out.Put(chunk);
		if(WhenList(chunk) || WhenData(sz, out.GetSize())) {
			Abort();
			break;
		}
	}
	progress = false;
	return !socket.IsError() && !aborted;
}

bool Ftp::PutData(TcpSocket& socket, Stream& in, int type)
{
	// Here, it is servers' responsibility to process ascii EOL.
	progress = true;
	int64 done  = 0;
	int64 total = in.GetSize();
	while(!socket.IsEof() && !in.IsEof()) {
		String buf = in.Get(chunk_size);
		done += socket.Put(buf, buf.GetLength());
		if(WhenData(total, done)) {
			Abort();
			break;
		}
	}
	progress = false;
	return !socket.IsError() && !aborted;
}

bool Ftp::InitFtps(int size)
{
	LLOG("** Starting FTPS negotiation...");
	PutGet("AUTH TLS");
	if(ReplyIsError()) {
		if(reply_code == 504 || reply_code == 534) {
			// Try SSL if TLS is rejected.
			LLOG("!! TLS request rejected. Trying SSL3 instead...");
			PutGet("AUTH SSL");
		}
	}
	if(ReplyIsSuccess()) {
		if(control_socket.StartSSL()) {
			PutGet("PBSZ " + AsString(size));
			if(ReplyIsSuccess()) {
				PutGet("PROT P");
				if(ReplyIsSuccess()) {
					// Start login sequence.
					reply_code = 220;
					LLOG("++ FTPS negotiation is successful. Client is in secure mode.");
					return true; 
				}
			}
		}
	}
	return SetError("FTPS negotiation failed.");
}

bool Ftp::Connect(const String& host, int port)
{
	if(host.IsEmpty())
		return SetError("Hostname is not specified.");
	ftp_host = host;
	ftp_port = port;
	if(user_id.IsEmpty() || user_password.IsEmpty()) {
		static const char *default_user = "anonymous";
		static const char *default_pass = "anonymous@";
		user_id = default_user;
		user_password = default_pass;
	}
	control_socket.Clear();
	if(!control_socket.Connect(ftp_host, Nvl(ftp_port, 21)))
		return SetError(Format("Couldn't connect to %s:%d", ftp_host, ftp_port));
	LLOG(Format("++ Connected to: %s:%d", ftp_host, ftp_port));	
	// Get server greeting.
	PutGet(Null);
	if(!ReplyIsSuccess())
		return false;
	// TLS/SSL connection.
	if(ftps && !InitFtps(0))
		return false;
	// Do Login.
	bool online = false;
	String replies = reply;
	while(!online) {
		switch(reply_code) {
			case 220:
				PutGet("USER " + user_id);
				break;
			case 331:
				LLOG(">> PASS ********");
				PutGet("PASS " + user_password, true);
				break;
			case 332:
				PutGet("ACCT noaccount");
				break;
			case 202:
			case 230:
				online = true;
				continue;
			default:
				return false;
		}
		replies << reply;
	}
	reply = replies;
	return online;
}

void Ftp::Disconnect()
{
	if(control_socket.IsOpen()) {
		PutGet("QUIT");
		control_socket.Clear();
		LLOG(Format("++ Disconnected from: %s:%d", ftp_host, ftp_port));	
	}
}

String Ftp::GetDir()
{
	PutGet("PWD");
	return ReplyIsSuccess() ? DecodePath(reply) : String::GetVoid();
}

bool Ftp::SetDir(const String& path)
{
	PutGet("CWD " + EncodePath(path));
	return ReplyIsSuccess();
}

bool Ftp::DirUp()
{
	PutGet("CDUP");
	return ReplyIsSuccess();
}

bool Ftp::ListDir(const String& path, DirList& list, Gate1<String> progress)
{
	if(!SetDataType(ASCII))
		return false;
	WhenList = progress;
	StringStream ls;
	String request = "LIST" + (!path.IsEmpty() ? (" " + path) : "");
	if(!TransferData(GET, request, ls, 0, ASCII))
		return false;
	ParseFtpDirEntry(ls.GetResult(), list);
	return true;
}

bool Ftp::MakeDir(const String& path)
{
	PutGet("MKD " + EncodePath(path));
	return ReplyIsSuccess();
}

bool Ftp::RemoveDir(const String& path)
{
	PutGet("RMD " + EncodePath(path));
	return ReplyIsSuccess();
}

bool Ftp::Get(const String& path, Stream& out, Gate2<int64, int64> progress, int type)
{
	if(path.IsEmpty())
		return SetError("Nothing to download. Filename is not specified.");
	WhenData = progress;
	if(!SetDataType(type))
		return false;
	PutGet("SIZE " + EncodePath(path));
	if(!ReplyIsSuccess()) 
		return false;
	uint64 size = ScanInt64(TrimBoth(reply.Mid(3)));
	String request = "RETR " + EncodePath(path);
	return TransferData(GET, request, out, size, type);
}

bool Ftp::Put(Stream& in, const String& path, Gate2<int64, int64> progress, int type)
{
	if(path.IsEmpty())
		return SetError("Nothing to upload. Filename is not specified.");
	WhenData = progress;
	if(!SetDataType(type))
		return false;
	String request = "STOR " + EncodePath(path);
	return TransferData(PUT, request, in, 0, type);
}

bool Ftp::Info(const String& path, DirEntry& info)
{
	DirList list;
	if(!ListDir(path, list) || list.GetCount() != 1)
		return false;
	info = pick(list[0]);
	return true;
}

bool Ftp::Rename(const String& oldname, const String& newname)
{
	if(oldname.IsEmpty() || newname.IsEmpty())
		return SetError("Couldn't change file name. Old name or new name is not specified.");
	PutGet("RNFR " + EncodePath(oldname));
	if(!ReplyIsPending())
		return false;
	PutGet("RNTO " + EncodePath(newname));
	return ReplyIsSuccess();		
}

bool Ftp::Delete(const String& path)
{
	if(path.IsEmpty())
		return SetError("Nothing to delete. File name is not specified.");
	PutGet("DELE " + EncodePath(path));
	return ReplyIsSuccess();
}

bool Ftp::Noop()
{
	PutGet("NOOP");
	return ReplyIsSuccess();
}

void Ftp::Abort()
{
	if(!InProgress() || !target || !target->IsOpen()) {
		return;
	}
	// RFC 959: p. 34-35:
	// Send TELNET IAC-IP ('\377' - '\364') & IAC-DM ('\377' - '\362') succesively.
	// DM, being a data-marker, should be sent as a single byte, "urgent" data.
	// -------------------------------------------------------------
	// Note that this sequence is not working on every server,
	// due to the problematic server-side (non-)implementation of ABOR command 
	// and the out-of-band data tranfer concept. Thus, we simply close the data 
	// connection, and accept error replies (>= 500) as success too.
	target->Abort();
#ifdef PLATFORM_POSIX
	control_socket.Put("\377\364\377");
	send(control_socket.GetSOCKET(), "\362", 1, MSG_OOB);
#endif	
	control_socket.Put("ABOR\r\n");
	LLOG(">> ABOR");
	target->Close();
	aborted = true;
	PutGet(Null);
	if(ReplyIsFailure() || ReplyIsError()) {
		if(reply_code != -1)
			PutGet(Null);
	}
}

Ftp::Ftp()
{
	control_socket.WhenWait = Proxy(WhenWait);
	control_socket.Timeout(60000);
	control_socket.WaitStep(10);
	Passive();
	reply_code = 0;
	chunk_size = 65536;
	progress = false;
	aborted = false;
	ftps = false;
	target = NULL;
}

Ftp::~Ftp()
{
	Disconnect();
	ftps = false;
	target = NULL;
}

void Ftp::DirEntry::Pick(DirEntry&& e)
{
	owner = pick(e.owner);
	group = pick(e.group);
	other = pick(e.other);
    user = e.user;
    filename = e.filename;
    ownername = e.ownername;
    groupname = e.groupname;
    direntry = e.direntry;
    size = e.size;
	type = e.type;
    style = e.style;
    time = e.time;	
}

int FtpGet(const String& path, Stream& out, const String& host, int port, const String& user, 
		const String& pass, Gate2<int64, int64> progress, Callback whenwait, int type, int mode, 
		bool ssl)
{
	Ftp worker;
	worker.WhenWait = whenwait;
	if(mode == Ftp::ACTIVE)
		worker.Active();
	if(!worker.User(user, pass).SSL(ssl).Connect(host, port)) 
		return worker.GetCode();
	return worker.Get(path, out, progress, type) ? 0 : worker.GetCode();
}

int FtpPut(Stream& in, const String& path, const String& host, int port, const String& user, 
		const String& pass, Gate2<int64, int64> progress, Callback whenwait, int type, int mode,
		bool ssl)
{
	Ftp worker;
	worker.WhenWait = whenwait;
	if(mode == Ftp::ACTIVE)
		worker.Active();
	if(!worker.User(user, pass).SSL(ssl).Connect(host, port))
		return worker.GetCode();
	return worker.Put(in, path, progress, type) ? 0 : worker.GetCode();
}


bool ParseFtpDirEntry(const String& in, Ftp::DirList& out)
{
	out.Clear();
	StringStream l(in);
	while(!l.IsEof()) {
		String ls = l.GetLine();
		Vector<String> entry = Split(NormalizeSpaces(ls), ' '); 
		if(entry.GetCount() >= 9) {
			// UNIX:
			// -rw-r--r-- 1 owner group 12738 Dec  1 2013  FTP.cpp
			// drw-r--r-- 2 owner group  4096 Apr 26 12:08 src.tpp
			Ftp::DirEntry e;
			String attr = entry[0];
			// Style
			e.style = Ftp::DirEntry::UNIX;
			// Type
			if(attr[0] == '-') 	
				e.type = Ftp::DirEntry::FILE;
			else
			if(attr[0] == 'd') 
				e.type = Ftp::DirEntry::DIRECTORY;
			else
			if(attr[0] == 'l') 
				e.type = Ftp::DirEntry::LINK;
			else
				continue;	
			// Permissions.	
			e.owner.Set(0, attr[1] == 'r'); 
			e.owner.Set(1, attr[2] == 'w'); 
			e.owner.Set(2, attr[3] == 'x');	
			e.group.Set(0, attr[4] == 'r'); 
			e.group.Set(1, attr[5] == 'w'); 
			e.group.Set(2, attr[6] == 'x');  
			e.other.Set(0, attr[7] == 'r'); 
			e.other.Set(1, attr[8] == 'w'); 
			e.other.Set(2, attr[9] == 'x'); 	
			// Owner, Group, Size, Name
			e.ownername = entry[2];
			e.groupname = entry[3];
			e.size = (ScanInt64(entry[4]));
			// Check if the last modification time is <= 6 months. 
			// In that case, ls will give us the hours, not the year. 
			// U++ date/time scanning routines will automatically add the current year.
			// Format can be either "(m)onth (d)ay (y)ear", or "(m)onth (d)day [hour:minutes]"
			String datetime = entry[5] + " " + entry[6] + " " + entry[7]; 
			SetDateScan(entry[7].Find(':') != -1 ? "md" : "mdy");
			Time t;
			if(StrToTime(t, datetime) != NULL && t.IsValid())
				e.time = t;
			// Resolve link, if exists.
			e.filename = (ls.GetCount() == 10 && e.IsSymLink()) ? entry[10] : entry[8];
			e.direntry = ls;
			out.AddPick(pick(e));	 		
		}
		else
		if(entry.GetCount() == 4) {
		   	// DOS:
 			// 12-1-13		13:48AM 	12738	FTP.cpp
			// 04-26-14		12:08PM		<DIR>	src.tpp
    		Ftp::DirEntry e;
			// Style
			e.style = Ftp::DirEntry::DOS;
    		// Last modification date and time.
    		String datetime = entry[0] + " " + entry[1];
    		SetDateScan("mdy");
    		Time t;
    		if(StrToTime(t, datetime) != NULL && t.IsValid())
    	   		e.time = t;
    		// Entry type or file size.
    		if(ToUpper(entry[2]) == "<DIR>") {
    			e.type = Ftp::DirEntry::DIRECTORY;;
    		}
    		else {
    			e.size = ScanInt64(entry[2]);
    			if(GetFileExt(ToLower(entry[3])) == ".lnk")
    				e.type = Ftp::DirEntry::LINK;
    			else
    				e.type = Ftp::DirEntry::FILE;
    		}
    		// FIXME.
			e.owner.Set(0, true); 
			e.owner.Set(1, true); 
			e.owner.Set(2, true);	
			e.group.Set(0, true); 
			e.group.Set(1, true); 
			e.group.Set(2, true);  
			e.other.Set(0, true); 
			e.other.Set(1, true); 
			e.other.Set(2, true); 	
			e.filename = entry[3];
			e.direntry = ls;
			out.AddPick(pick(e));
		}
	}
	return out.GetCount() != 0;	
}
END_UPP_NAMESPACE
