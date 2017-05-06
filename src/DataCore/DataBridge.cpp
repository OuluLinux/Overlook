#include "DataCore.h"


namespace DataCore {

DataBridge::DataBridge()  {
	port = 0;
	enable_bardata = true;
	has_written = false;
	running = false;
	stopped = true;
	sym_count = -1;
	
	AddValue<double>(); // open
	AddValue<double>(); // low
	AddValue<double>(); // high
	AddValue<double>(); // volume
}

DataBridge::~DataBridge()  {
	running = false;
	while (!stopped) Sleep(100);
}

void DataBridge::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("addr");
	if (i != -1)	addr = args[i];
	else			throw DataExc("No address argument");
	i = args.Find("port");
	if (i != -1)	port = args[i];
	else			throw DataExc("No port argument");
	i = args.Find("enable_bardata");
	if (i != -1)
		enable_bardata = args[i];
}

void DataBridge::Serialize(Stream& s) {
	s % connected % account_server % tfs;
}

void DataBridge::Init() {
	if (addr.IsEmpty() || !port) throw DataExc("No address and port");
	
	SetProcessing(1, 0);
	
	TimeVector& tv = GetTimeVector();
	MetaTrader& mt = GetMetaTrader();
	
	ASSERTEXC_(mt.GetSymbolCount() > 0, "MetaTrader must be initialized before DataBridge.");
	
	try {
		connected = mt.AccountNumber();
		account_server = mt.AccountServer();
		sym_count = mt.GetSymbolCount();
		
		tfs.Clear();
		int base_mtf = tv.GetBasePeriod() / 60;
		for(int i = 0; i < mt.GetTimeframeCount(); i++) {
			int tf = mt.GetTimeframe(i);
			if (tf >= base_mtf)
				tfs.Add(tf);
		}
		
		DownloadRemoteData();
	}
	catch (ConnectionError e) {
		throw DataExc("DataBridge::Init: connection failed to " + addr + ":" + IntStr(port));
	}
	catch (...) {
		throw DataExc("DataBridge::Init: unknown error");
	}
	
	// Check that symbol names match
	for(int i = 0; i < sym_count; i++) {
		ASSERTEXC(tv.GetSymbol(i) == mt.GetSymbol(i).name);
	}
	
	// Assert that first tf matches base period
	if (tv.GetBasePeriod() != tfs[0] * 60) {
		throw DataExc("Resolver's base period differs from mt");
	}
	
	points.SetCount(sym_count, 0.0001);
	
	int tf_count = tv.GetPeriodCount();
	loaded.SetCount(sym_count * tf_count, false);
	has_written = false;
	
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Run));
}

void DataBridge::Run() {
	while (running && !Thread::IsShutdownThreads()) {
		
		// Refresh ask & bid
		
		
		// Check soft takeprofit / stoploss limits
		//  - use predicted channel slots
		
		
		// Update hard (at broker) takeprofit / stoploss limits
		
		
		
		Sleep(1000);
	}
	stopped = true;
}

void DataBridge::DownloadRemoteData() {
	LOG("DataBridge::DownloadRemoteData");
	
	const Vector<Symbol>& symbols = GetMetaTrader().GetCacheSymbols();
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

int DataBridge::DownloadHistory(const Symbol& sym, int tf, bool force) {
	String history_dir = ConfigFile("history");
	RealizeDirectory(history_dir);
	
	String filename = sym.name + IntStr(tf) + ".hst";
	String local_path = AppendFileName(history_dir, filename);
	String remote_path = "history/" + account_server + "/" + filename;
	
	if (!force && FileExists(local_path)) return 0;
	
	return DownloadRemoteFile(remote_path, local_path);
}

int DataBridge::DownloadAskBid() {
	String local_path = ConfigFile("askbid.bin");
	String remote_path = "MQL4\\files\\askbid.bin";
	
	return DownloadRemoteFile(remote_path, local_path);
}

int DataBridge::DownloadRemoteFile(String remote_path, String local_path) {
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
	
	LOG("DataBridge::DownloadRemoteFile: for " << remote_path << " took " << ts.ToString());
	return 0;
}

bool DataBridge::Process(const SlotProcessAttributes& attr) {
	MetaTrader& mt = GetMetaTrader();
	TimeVector& tv = GetTimeVector();
	
	LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	if (sym_count != -1 && attr.sym_id >= sym_count) {
		if (attr.pos[0] == attr.bars[0]-1)
			return false;
		ProcessVirtualNode(attr);
		return true;
	}
	
	ASSERT(attr.GetCounted() == 0);
	
	
	// Open data-file
	int period = attr.GetPeriod();
	int mt_period = period * tv.GetBasePeriod() / 60;
	String symbol = mt.GetSymbol(attr.sym_id).name;
	String history_dir = ConfigFile("history");
	String filename = symbol + IntStr(mt_period) + ".hst";
	String local_history_file = AppendFileName(history_dir, filename);
	if (!FileExists(local_history_file))
		return false;
	FileIn src(local_history_file);
	if (!src.IsOpen() || !src.GetSize())
		return false;
	
	
	int count = 0;
	
	// Init destination time vector settings
	int bars = tv.GetCount(period);
	int begin = tv.GetBeginTS();
	int step = tv.GetBasePeriod() * period;
	int cur = begin;
	
	
	// Read the history file
	int digits;
	src.Seek(4+64+12+4);
	src.Get(&digits, 4);
	if (digits > 20)
		return false;
	double point = 1.0 / pow(10.0, digits);
	points[attr.sym_id] = point;
	int data_size = src.GetSize();
	const int struct_size = 8 + 4*8 + 8 + 4 + 8;
	byte row[struct_size];
	double prev_close;
	double open = 0;
	
	
	// Seek to begin of the data
	int cursor = (4+64+12+4+4+4+4 +13*4);
	cursor += count * struct_size;
	src.Seek(cursor);
	
	
	while ((cursor + struct_size) <= data_size && count < bars) {
		int time;
		double high, low, close;
		int64 tick_volume, real_volume;
		int spread;
		src.Get(row, struct_size);
		byte* current = row;
		
		//TODO: endian swap in big endian machines
		
		time  = *((uint64*)current);		current += 8;
		open  = *((double*)current);		current += 8;
		high  = *((double*)current);		current += 8;
		low   = *((double*)current);		current += 8;
		close = *((double*)current);		current += 8;
		tick_volume  = *((int64*)current);		current += 8;
		spread       = *((int32*)current);		current += 4;
		real_volume  = *((int64*)current);		current += 8;
		
		cursor += struct_size;
		
		// At first value
		if (count == 0) {
			prev_close = close;
			
			// Check that value is in the range of 1*point - UINT16_MAX*point
			double base_point = point;
			while (close >= (UINT16_MAX*point)) {
				// sacrifice a little bit of accuracy for optimal memory usage
				point += base_point;
			}
			points[attr.sym_id] = point;
			// TODO: check all data and don't rely on close
		}
		
		while (cur < time && count < bars) {
			*GetValuePos<double>(0, attr.sym_id, attr.tf_id, count)	= prev_close;
			*GetValuePos<double>(1, attr.sym_id, attr.tf_id, count)	= prev_close;
			*GetValuePos<double>(2, attr.sym_id, attr.tf_id, count)	= prev_close;
			*GetValuePos<double>(3, attr.sym_id, attr.tf_id, count)	= 0;
			SetReady(count, attr, true);
			cur += step;
			count++;
		}
		
		prev_close = close;
		
		if (count < bars && time == cur) {
			//ASSERT(tv.GetTime(tf, count) == TimeFromTimestamp(time));
			*GetValuePos<double>(0, attr.sym_id, attr.tf_id, count)	= open;
			*GetValuePos<double>(1, attr.sym_id, attr.tf_id, count)	= low;
			*GetValuePos<double>(2, attr.sym_id, attr.tf_id, count)	= high;
			*GetValuePos<double>(3, attr.sym_id, attr.tf_id, count)	= tick_volume;
			SetReady(count, attr, true);
			cur += step;
			count++;
		}
		
		//LOG(Format("%d: %d %f %f %f %f %d %d %d", cursor, (int)time, open, high, low, close, tick_volume, spread, real_volume));
	}
	
	while (count < bars) {
		*GetValuePos<double>(0, attr.sym_id, attr.tf_id, count)	= open;
		*GetValuePos<double>(1, attr.sym_id, attr.tf_id, count)	= open;
		*GetValuePos<double>(2, attr.sym_id, attr.tf_id, count)	= open;
		*GetValuePos<double>(3, attr.sym_id, attr.tf_id, count)	= 0;
		SetReady(count, attr, true);
		cur += step;
		count++;
	}
	
	has_written = true;
	
	return true;
}

void DataBridge::ProcessVirtualNode(const SlotProcessAttributes& attr) {
	MetaTrader& mt = GetMetaTrader();
	int cur = attr.sym_id - sym_count;
	ASSERT(cur >= 0 && cur < mt.GetCurrencyCount());
	const Currency& c = mt.GetCurrency(cur);
	
	double change = 0;
	double volume = 0;
	
	for(int j = 0; j < c.pairs0.GetCount(); j++) {
		int id = c.pairs0[j];
		double* open	= GetValue<double>(0, id, attr.tf_id,  0, attr);
		double* close	= GetValue<double>(0, id, attr.tf_id, -1, attr);
		double diff = (*close - *open) / points[id];
		change -= diff;
		double* vol	= GetValue<double>(3, id, attr.tf_id, 0, attr);
		volume += *vol;
	}
	
	for(int j = 0; j < c.pairs1.GetCount(); j++) {
		int id = c.pairs1[j];
		double* open	= GetValue<double>(0, id, attr.tf_id,  0, attr);
		double* close	= GetValue<double>(0, id, attr.tf_id, -1, attr);
		double diff = (*close - *open) / points[id];
		change -= diff;
		double* vol	= GetValue<double>(3, id, attr.tf_id, 0, attr);
		volume += *vol;
	}
	
	double* prev = GetValue<double>(0, 1, attr);
	double value = prev ? *prev * (1 + change * 0.00001) : 1.0;
	
	*GetValue<double>(0, attr) = value;
	*GetValue<double>(1, attr) = value;
	*GetValue<double>(2, attr) = value;
	*GetValue<double>(3, attr) = volume;
	
	if (prev) {
		double* prev_low = GetValue<double>(1, 1, attr);
		double* prev_high = GetValue<double>(2, 1, attr);
		*prev_low = Upp::min(*prev, value);
		*prev_high = Upp::max(*prev, value);
	}
}

}
