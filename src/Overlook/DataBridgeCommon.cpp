#include "Overlook.h"

namespace Overlook {

DataBridgeCommon::DataBridgeCommon() {
	inited = false;
	sym_count = -1;
	cursor = 0;
	
	LoadThis();
}

void DataBridgeCommon::InspectInit() {
	if (!IsInited()) {
		LOCK(lock) {
			if (!IsInited()) {
				Init();
			}
		}
	}
}

void DataBridgeCommon::Init() {
	System& sys = GetSystem();
	
	if (String(Config::arg_addr).IsEmpty() || !Config::arg_port) throw DataExc("No address nor port defined");
	
	MetaTrader& mt = GetMetaTrader();
	
	ASSERTEXC_(mt.GetSymbolCount() > 0, "MetaTrader must be initialized before DataBridge.");
	
	try {
		connected = mt.AccountNumber();
		account_server = mt.AccountServer();
		sym_count = mt.GetSymbolCount();
		
		tfs.Clear();
		for(int i = 0; i < mt.GetTimeframeCount(); i++)
			tfs.Add(mt.GetTimeframe(i));
		
		// Get maximum of 6 chars strings to match src symbols
		short_ids.Clear();
		data.SetCount(sym_count);
		for(int i = 0; i < sym_count; i++) {
			const String& sym = mt.GetSymbol(i).name;
			short_ids.Add( sym.GetCount() > 6 ? sym.Left(6) : sym );
		}
		
	}
	catch (...) {
		throw DataExc("DataBridge::Init: unknown error");
	}
	
	// Inspect that symbol names match
	for(int i = 0; i < sym_count; i++) {
		ASSERTEXC(sys.GetSymbol(i) == mt.GetSymbol(i).name);
	}
	
	points.SetCount(sym_count, 0.0001);
	
	int tf_count = sys.GetPeriodCount();
	loaded.SetCount(sym_count * tf_count, false);
	
	RefreshTimeBuffers();
	StoreThis();
	
	inited = true;
}

void DataBridgeCommon::Start() {
	GetMetaTrader().Data();
	InspectInit();
	lock.Enter();
	DownloadAskBid();
	lock.Leave();
	RefreshAskBidData(true);
	RefreshTimeBuffers();
}

void DataBridgeCommon::DownloadRemoteData() {
	const Vector<Symbol>& symbols = GetMetaTrader().GetSymbols();
	int actual = 0;
	int total = symbols.GetCount() * tfs.GetCount();
	for(int i = 0; i < symbols.GetCount(); i++) {
		actual = i * tfs.GetCount();
		for(int j = 0; j < tfs.GetCount(); j++) {
			double perc = (double)actual / total * 100;
			if (DownloadHistory(symbols[i], tfs[j])) break;
			actual++;
		}
	}
	
	DownloadAskBid();
}

int DataBridgeCommon::DownloadHistory(int sym, int tf, bool force) {
	const Vector<Symbol>& symbols = GetMetaTrader().GetSymbols();
	return DownloadHistory(symbols[sym], tfs[tf], force);
}

int DataBridgeCommon::DownloadHistory(const Symbol& sym, int tf, bool force) {
	String history_dir = ConfigFile("history");
	RealizeDirectory(history_dir);
	
	String filename = sym.name + IntStr(tf) + ".hst";
	String remote_filename = filename;
	String local_path = AppendFileName(history_dir, filename);
	String remote_path = "history/" + account_server + "/" + remote_filename;
	
	if (!force && FileExists(local_path)) return 0;
	
	return DownloadRemoteFile(remote_path, local_path);
}

int DataBridgeCommon::DownloadAskBid() {
	ReleaseLog("DownloadAskBid");
	
	String local_path = ConfigFile("askbid.bin");
	String remote_path = "MQL4\\Files\\askbid.bin";
	
	return DownloadRemoteFile(remote_path, local_path);
}

int DataBridgeCommon::DownloadRemoteFile(String remote_path, String local_path) {
	ReleaseLog("DownloadRemoteFile " << remote_path << " ----> " << local_path);
	
	MetaTrader& mt = GetMetaTrader();
	
	TimeStop ts;
	
	FileAppend out(local_path);
	if (!out.IsOpen()) return 1;
	out.SeekEnd();
	int offset = (int)out.GetPos(); // No >2Gt files expected
	ReleaseLog("Existing size " + IntStr(offset));
	
	// Get the MT4 remote file
	
	#define CHK(x) if (!(x)) {sock.Close(); ReleaseLog("FileRequest fail: " + String(#x)); return 1;}
	
	TcpSocket sock;
	sock.Timeout(3000);
	if (!sock.Connect(mt.GetAddr(), mt.GetPort() + 100)) {
		ReleaseLog("Can't connect file server");
		return 1;
	}
	
	int r;
	int cmd = offset > 0 ? 2 : 1;
	r = StrPut(sock, &cmd, sizeof(int));
	CHK(r == sizeof(int));
	
	if (cmd == 2) {
		r = StrPut(sock, &offset, sizeof(int));
		CHK(r ==  sizeof(int));
	}
	
	int size = remote_path.GetCount();
	StrPut(sock, &size, sizeof(int));
	r = sock.Put(remote_path, size);
	CHK(r == size);
	
	r = StrGet(sock, &size, sizeof(int));
	CHK(r ==   sizeof(int));
	CHK(size > 0);
	int data_size = size;
	
	struct Block {
		byte *b;
		Block(int64 size) {b = (byte*)MemoryAlloc(size); ASSERT(b);}
		~Block() {MemoryFree(b);}
	};
	
	int chunk = 1024*1024;
	Block b(size);
	byte* buf = b.b;
	
	while (size >= chunk) {
		r = StrGet(sock, buf, chunk);
		CHK(r == chunk);
		
		out.Put(buf, chunk);
		
		size -= chunk;
		buf += chunk;
	}
	
	if (size > 0) {
		r = StrGet(sock, buf, size);
		CHK(r == size);
		
		out.Put(buf, size);
	}
	
	out.Flush();
	out.Close();
	
	ReleaseLog("DataBridgeCommon::DownloadRemoteFile: " + IntStr(out.GetSize()) + " bytes for " + remote_path + " took " + ts.ToString());
	return 0;
}

void DataBridgeCommon::RefreshAskBidData(bool forced) {
	InspectInit();
	
	lock.Enter();
	
	// 3 second update interval is enough...
	if (!forced && since_last_askbid_refresh.Elapsed() < 500 && cursor > 0) {
		lock.Leave();
		return;
	}
		
	// Open askbid-file
	String local_askbid_file = ConfigFile("askbid.bin");
	if (!FileExists(local_askbid_file) || cursor == 0)
		DownloadAskBid();
	FileIn src(local_askbid_file);
	ASSERTEXC(src.IsOpen() && src.GetSize());
	int data_size = (int)src.GetSize(); // No >2Gt files expected
	
	src.Seek(cursor);
	
	int added = 0;
	int struct_size = 4 + 6 + 8 + 8;
	String src_id;
	while ((cursor + struct_size) <= data_size) {
		int timestamp;
		src.Get(&timestamp, 4);
		
		src_id = "";
		for(int i = 0; i < 6; i++) {
			char c;
			src.Get(&c, 1);
			if (!c) {
				src.SeekCur(6-1-i);
				break;
			}
			src_id.Cat(c);
		}
		if (!src_id.IsEmpty()) {
			int i = short_ids.Find(src_id);
			if (i != -1) {
				Vector<AskBid>& data = this->data[i];
				AskBid& ab = data.Add();
				ab.a = TimeFromTimestamp(timestamp);
				src.Get(&ab.b, 8); // ask
				src.Get(&ab.c, 8); // bid
				added++;
			}
			else src.SeekCur(8+8);
		}
		else src.SeekCur(8+8);
		
		cursor += struct_size;
	}
	
	ReleaseLog("RefreshAskBidData added " + IntStr(added));
	
	since_last_askbid_refresh.Reset();
	
	lock.Leave();
}

void DataBridgeCommon::RefreshTimeBuffers() {
	System& sys = GetSystem();
	int tf_count = sys.GetPeriodCount();
	
	time_bufs.SetCount(tf_count);
	int64 now = GetUtcTime().Get() - Time(1970,1,1).Get();
	
	idx.SetCount(tf_count);
	
	for(int i = 0; i < tf_count; i++) {
		Buffer& time_buf = time_bufs[i];
		
		int64 time = time_buf.GetCount() ? time_buf.Top() : 0;
		int shift = time_buf.GetCount() - 1;
		int minperiod = sys.GetPeriod(i);
		
		while (time < now) {
			
			time += minperiod * 60;
			
			if (!SyncData(i, time, shift))
				continue;
			
			if (shift == time_buf.GetCount()) {
				int res = shift + 10000;
				res -= res % 10000;
				res += 10000;
				
				time_buf.Reserve(res);
				time_buf.SetCount(shift+1);
			}
		
			time_buf.Set(shift, time);
			
			ASSERT(idx[i].GetCount() == shift);
			idx[i].Add(Time(1970,1,1) + time);
		}
	}
	
}

bool DataBridgeCommon::SyncData(int tf, int64 time, int& shift) {
	System& sys = GetSystem();
	
	Time utc_time = Time(1970,1,1) + time;
	#ifndef flagSECONDS
	ASSERT(utc_time.second == 0);
	#endif
	int wday = DayOfWeek(utc_time);
	if (wday == 6 || (wday == 0 && utc_time.hour < 21) || (wday == 5 && utc_time.hour >= 22))
		return false;
	
	int minperiod = sys.GetPeriod(tf);
	bool is_vtf = tf == VTF;
	
	Buffer& time_buf = time_bufs[tf];
	
	Time t;
	if (shift < 0) {
		shift = -1;
		#ifndef flagSECONDS
		t = Time(1970,1,1) + Config::start_time - 60 * minperiod;
		#else
		t = Time(1970,1,1) + Config::start_time - 1 * minperiod;
		#endif
	} else {
		if (time_buf.IsEmpty())
			return false;
		shift = time_buf.GetCount() - 1;
		t = Time(1970,1,1) + time_buf.Top();
		#ifndef flagSECONDS
		ASSERT(t.second == 0);
		#endif
	}
	if (utc_time < t)
		return false;
	
	if (t < utc_time) {
		shift++;
		#ifdef flagSECONDS
		t += 1 * minperiod;
		#else
		t += 60 * minperiod;
		#endif
		while (t < utc_time) {
			int wday = DayOfWeek(t);
			if ((!is_vtf && !(wday == 6 || (wday == 0 && t.hour < 22) || (wday == 5 && t.hour >= 21))) || (is_vtf && IsVtfTime(wday, t))) {
				int time = t.Get() - Time(1970,1,1).Get();
				
				int res = shift + 100000;
				res -= res % 100000;
				res += 100000;
				
				time_buf.Reserve(res);
				time_buf.SetCount(shift+1);
				time_buf.Set(shift, time);
				
				shift++;
			}
			#ifdef flagSECONDS
			t += 1 * minperiod;
			#else
			t += 60 * minperiod;
			#endif
		}
	}
	else if (shift == -1) {
		shift++;
	}
	
	if (is_vtf && !IsVtfTime(wday, t)) {
		if (shift == 0) shift = -1;
		return false;
	}
	
	return true;
}

bool DataBridgeCommon::IsVtfTime(int wday, const Time& t) {
	#define IS(h, m) {if (t.hour == h && t.minute == m) {return true;}}
	
	// NOTE: update GetVtfWeekbars if changed
	if (wday == 0) {
		IS(22,00);
		IS(22,59);
	}
	// Monday
	else if (wday == 1) {
		IS(2,00);
		IS(4,10);
		IS(4,16);
		IS(4,54);
		IS(7,00); // EU ses
		IS(8,30);
		IS(10,00);
		IS(11,00);
		IS(12,00); // US ses
		IS(12,30); // first US M30 has gone
		IS(14,00);
		IS(15,00);
		IS(17,59);
		IS(18,57);
		IS(21,40);
		IS(23,37);
	}
	if (wday == 2) {
		IS(1,00);
		IS(2,00);
		IS(4,00);
		IS(5,30);
		IS(7,00); // EU ses
		IS(8,30);
		IS(10,00); // Strong
		IS(12,00); // US ses
		IS(12,30);
		IS(14,00);
		IS(14,57);
		IS(16,30);
		IS(18,00);
		IS(19,30);
		IS(20,42);
		IS(22,30);
		IS(22,59);
		IS(23,59);
	}
	else if (wday == 3) {
		IS(0,59);
		IS(1,30);
		IS(2,00);
		IS(2,46);
		IS(3,30);
		IS(4,00);
		IS(4,30);
		IS(5,30);
		IS(7,00); //EU ses
		IS(8,30);
		IS(10,00);
		IS(11,00);
		IS(11,29);
		IS(12,00); //US ses
		IS(12,30);
		IS(13,30);
		IS(15,00);
		IS(16,30);
		IS(17,00);
		IS(18,00);
		IS(20,45);
		IS(22,00);
	}
	else if (wday == 4) {
		IS(0,59);
		IS(2,00);
		IS(3,05);
		IS(4,00);
		IS(5,30);
		IS(7,00);
		IS(8,30);
		IS(10,00);
		IS(11,00);
		IS(12,00);
		IS(12,30);
		IS(15,00);
		IS(16,30);
		IS(17,15);
		IS(18,00);
		IS(19,00);
		IS(20,00);
		IS(20,49);
		IS(21,35);
		IS(22,49);
		IS(23,59);
	}
	else if (wday == 5) {
		IS(0,59);
		IS(2,00);
		IS(3,00);
		IS(4,10);
		IS(4,54);
		IS(6,35);
		IS(7,00);
		IS(8,12);
		IS(9,00);
		IS(10,00);
		IS(11,00);
		IS(12,00);
		IS(12,30);
		IS(14,05);
		IS(15,29);
		IS(16,30);
		IS(18,00);
		IS(18,57);
		IS(19,59);
	}
	#undef IS
	return false;
}

}
