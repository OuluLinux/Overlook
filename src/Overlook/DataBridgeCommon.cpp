#include "Overlook.h"

namespace Overlook {

DataBridgeCommon::DataBridgeCommon() {
	inited = false;
	port = 0;
	sym_count = -1;
	cursor = 0;
}

void DataBridgeCommon::CheckInit(DataBridge* db) {
	if (!IsInited()) {
		lock.Enter();
		if (!IsInited()) {
			Init(db);
		}
		lock.Leave();
	}
}

void DataBridgeCommon::Init(DataBridge* db) {
	System& bs = db->GetSystem();
	addr = bs.addr;
	port = bs.port;
	
	if (addr.IsEmpty() || !port) throw DataExc("No address and port");
	
	MetaTrader& mt = GetMetaTrader();
	
	ASSERTEXC_(mt.GetSymbolCount() > 0, "MetaTrader must be initialized before DataBridge.");
	
	try {
		connected = mt.AccountNumber();
		account_server = mt.AccountServer();
		sym_count = mt.GetSymbolCount();
		
		tfs.Clear();
		int base_mtf = bs.GetBasePeriod() / 60;
		for(int i = 0; i < mt.GetTimeframeCount(); i++) {
			int tf = mt.GetTimeframe(i);
			if (tf >= base_mtf)
				tfs.Add(tf);
		}
		
		// Get maximum of 6 chars strings to match src symbols
		short_ids.Clear();
		data.SetCount(sym_count);
		for(int i = 0; i < sym_count; i++) {
			const String& sym = mt.GetSymbol(i).name;
			short_ids.Add( sym.GetCount() > 6 ? sym.Left(6) : sym );
		}
		
		DownloadRemoteData();
	}
	catch (...) {
		throw DataExc("DataBridge::Init: unknown error");
	}
	
	// Check that symbol names match
	for(int i = 0; i < sym_count; i++) {
		ASSERTEXC(bs.GetSymbol(i) == mt.GetSymbol(i).name);
	}
	
	// Assert that first tf matches base period
	if (bs.GetBasePeriod() != tfs[0] * 60) {
		throw DataExc("Resolver's base period differs from mt");
	}
	
	points.SetCount(sym_count, 0.0001);
	
	int tf_count = bs.GetPeriodCount();
	loaded.SetCount(sym_count * tf_count, false);
	
	inited = true;
}

void DataBridgeCommon::DownloadRemoteData() {
	LOG("DataBridgeCommon::DownloadRemoteData");
	DUMPC(tfs);
	
	const Vector<Symbol>& symbols = GetMetaTrader().GetSymbols();
	int actual = 0;
	int total = symbols.GetCount() * tfs.GetCount();
	for(int i = 0; i < symbols.GetCount(); i++) {
		actual = i * tfs.GetCount();
		for(int j = 0; j < tfs.GetCount(); j++) {
			double perc = (double)actual / total * 100;
			LOG(Format("%2!,n done", perc));
			if (DownloadHistory(symbols[i], tfs[j])) break;
			actual++;
		}
	}
	
	DownloadAskBid();
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
	String local_path = ConfigFile("askbid.bin");
	String remote_path = "MQL4\\Files\\askbid.bin";
	
	return DownloadRemoteFile(remote_path, local_path);
}

int DataBridgeCommon::DownloadRemoteFile(String remote_path, String local_path) {
	LOG("DownloadRemoteFile " << remote_path << " ----> " << local_path);
	
	MetaTrader& mt = GetMetaTrader();
	
	TimeStop ts;
	
	FileAppend out(local_path);
	if (!out.IsOpen()) return 1;
	int offset = out.GetSize();
	out.SeekEnd();
	
	// Get the MT4 remote file
	
	#define CHK(x) if (!(x)) {sock.Close(); LOG("FileRequest fail: " + String(#x)); return 1;}
	
	TcpSocket sock;
	sock.Timeout(3000);
	if (!sock.Connect(mt.GetAddr(), mt.GetPort() + 100)) {
		LOG("Can't connect file server");
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
	
	LOG("DataBridgeCommon::DownloadRemoteFile: " << out.GetSize() << " bytes for " << remote_path << " took " << ts.ToString());
	return 0;
}

void DataBridgeCommon::RefreshAskBidData() {
	lock.Enter();
	
	// 3 second update interval is enough...
	if (since_last_askbid_refresh.Elapsed() < 3000 && cursor > 0) {
		lock.Leave();
		return;
	}
		
	// Open askbid-file
	String local_askbid_file = ConfigFile("askbid.bin");
	FileIn src(local_askbid_file);
	ASSERTEXC(src.IsOpen() && src.GetSize());
	int data_size = src.GetSize();
	
	src.Seek(cursor);
	
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
			}
			else src.SeekCur(8+8);
		}
		else src.SeekCur(8+8);
		
		cursor += struct_size;
	}
	
	since_last_askbid_refresh.Reset();
	
	lock.Leave();
}

}
