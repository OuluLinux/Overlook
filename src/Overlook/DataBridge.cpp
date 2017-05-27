#include "Overlook.h"

namespace Overlook {

DataBridgeCommon::DataBridgeCommon() {
	inited = false;
	port = 0;
	sym_count = -1;
	
}

void DataBridgeCommon::Init(DataBridge* db) {
	BaseSystem& bs = db->GetBaseSystem();
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

int DataBridgeCommon::DownloadHistory(const Symbol& sym, int tf, bool force) {
	String history_dir = ConfigFile("history");
	RealizeDirectory(history_dir);
	
	String filename = sym.name + IntStr(tf) + ".hst";
	String remote_filename = filename;
	
	// At some point MT4 started to add weird 0023 text in front of the filename.
	// I don't understand this, and this hardcoded fix might be wrong for others.
	// Please contact the author if you have some other number than 0023 or some other issue.
	if (remote_filename.Left(1) == "#")
		remote_filename = "#0023" + remote_filename.Mid(1);
	
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
	//LOG("DownloadRemoteFile " << remote_path << " ----> " << local_path);
	
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


















DataBridge::DataBridge()  {
	SetSkipAllocate();
	
}

DataBridge::~DataBridge()  {
	
}

void DataBridge::Serialize(Stream& s) {
	
}

void DataBridge::Init() {
	//SetBufferCount(4, 4); // open, low, high, volume
	
}

inline int IncreaseMonthTS(int ts) {
	Time t(1970,1,1);
	int64 epoch = t.Get();
	t += ts;
	int year = t.year;
	int month = t.month;
	month++;
	if (month == 13) {year++; month=1;}
	return Time(year,month,1).Get() - epoch;
}

void DataBridge::Start() {
	
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	if (!common.IsInited()) {
		common.lock.Enter();
		if (!common.IsInited()) {
			common.Init(this);
		}
		common.lock.Leave();
	}
	
	MetaTrader& mt = GetMetaTrader();
	BaseSystem& bs = GetBaseSystem();
	
	LOG(Format("sym=%d tf=%d pos=%d", Core::GetSymbol(), GetTimeframe(), GetBars()));
	
	int sym_count = common.GetSymbolCount();
	if (GetSymbol() >= sym_count)
		return;
	
	int bars = GetBars();
	ASSERT(bars > 0);
	for(int i = 0; i < outputs.GetCount(); i++)
		for(int j = 0; j < outputs[i].buffers.GetCount(); j++)
			outputs[i].buffers[j].value.SetCount(bars, 0);
	
	
	// Open data-file
	int period = GetPeriod();
	int tf = GetTf();
	int mt_period = period * bs.GetBasePeriod() / 60;
	String symbol = bs.GetSymbol(GetSymbol());
	String history_dir = ConfigFile("history");
	String filename = symbol + IntStr(mt_period) + ".hst";
	String local_history_file = AppendFileName(history_dir, filename);
	if (!FileExists(local_history_file))
		throw DataExc();
	FileIn src(local_history_file);
	if (!src.IsOpen() || !src.GetSize())
		return;
	//ASSERTEXC(src.IsOpen() && src.GetSize());
	/*if (!src.IsOpen() || !src.GetSize()) {
		LOG("ERROR: invalid file " << local_history_file);
		throw DataExc();
	}*/
	int count = 0;
	
	
	// Init destination time vector settings
	int begin = bs.GetBeginTS(tf);
	int step = bs.GetBasePeriod() * period;
	int cur = begin;
	bool inc_month = period == 43200 && bs.GetBasePeriod() == 60;
	
	// Read the history file
	int digits;
	src.Seek(4+64+12+4);
	src.Get(&digits, 4);
	if (digits > 20)
		throw DataExc();
	double point = 1.0 / pow(10.0, digits);
	common.points[GetSymbol()] = point;
	int data_size = src.GetSize();
	const int struct_size = 8 + 4*8 + 8 + 4 + 8;
	byte row[struct_size];
	double prev_close;
	double open = 0;
	
	
	// Seek to begin of the data
	int cursor = (4+64+12+4+4+4+4 +13*4);
	cursor += count * struct_size;
	src.Seek(cursor);
	
	
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	
	
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
			common.points[GetSymbol()] = point;
			// TODO: check all data and don't rely on close
		}
		
		while (cur < time && count < bars) {
			SetSafetyLimit(count);
			open_buf.Set(count, prev_close);
			low_buf.Set(count, prev_close);
			high_buf.Set(count, prev_close);
			volume_buf.Set(count, 0);
			if (!inc_month)		cur += step;
			else				cur = IncreaseMonthTS(cur);
			count++;
		}
		
		prev_close = close;
		
		if (count < bars && time == cur) {
			SetSafetyLimit(count);
			//ASSERT(bs.GetTime(tf, count) == TimeFromTimestamp(time));
			open_buf.Set(count, open);
			low_buf.Set(count, low);
			high_buf.Set(count, high);
			volume_buf.Set(count, tick_volume);
			if (!inc_month)		cur += step;
			else				cur = IncreaseMonthTS(cur);
			count++;
		}
		
		//LOG(Format("%d: %d %f %f %f %f %d %d %d", cursor, (int)time, open, high, low, close, tick_volume, spread, real_volume));
	}
	
	while (count < bars) {
		SetSafetyLimit(count);
		open_buf.Set(count, open);
		low_buf.Set(count, open);
		high_buf.Set(count, open);
		volume_buf.Set(count, 0);
		if (!inc_month)		cur += step;
		else				cur = IncreaseMonthTS(cur);
		count++;
	}
}



















VirtualNode::VirtualNode()  {
	SetSkipAllocate();
	
}

VirtualNode::~VirtualNode()  {
	
}

void VirtualNode::Init() {
	//SetBufferCount(4, 4); // open, low, high, volume
	
}

void VirtualNode::Start() {
	Buffer& open   = GetBuffer(0);
	Buffer& low    = GetBuffer(1);
	Buffer& high   = GetBuffer(2);
	Buffer& volume = GetBuffer(3);
	
	BaseSystem& bs = GetBaseSystem();
	MetaTrader& mt = GetMetaTrader();
	int sym_count = mt.GetSymbolCount();
	int cur = GetSymbol() - sym_count;
	if (cur < 0)
		return;
	ASSERT(cur >= 0 && cur < mt.GetCurrencyCount());
	
	int bars = GetBars();
	ASSERT(bars > 0);
	for(int i = 0; i < outputs.GetCount(); i++)
		for(int j = 0; j < outputs[i].buffers.GetCount(); j++)
			outputs[i].buffers[j].value.SetCount(bars, 0);
		
	const Currency& c = mt.GetCurrency(cur);
	int tf = GetTimeframe();
	
	typedef Tuple3<ConstBuffer*,ConstBuffer*,bool> Source;
	Vector<Source> sources;
	for(int i = 0; i < c.pairs0.GetCount(); i++) {
		int id = c.pairs0[i];
		ConstBuffer& open_buf = GetInputBuffer(0,id,tf,0);
		ConstBuffer& vol_buf  = GetInputBuffer(0,id,tf,3);
		sources.Add(Source(&open_buf, &vol_buf, false));
	}
	for(int i = 0; i < c.pairs1.GetCount(); i++) {
		int id = c.pairs1[i];
		ConstBuffer& open_buf = GetInputBuffer(0,id,tf,0);
		ConstBuffer& vol_buf  = GetInputBuffer(0,id,tf,3);
		sources.Add(Source(&open_buf, &vol_buf, true));
	}
	
	int counted = GetCounted();
	if (!counted) {
		open.Set(0, 1.0);
		low.Set(0, 1.0);
		high.Set(0, 1.0);
		volume.Set(0, 0);
		counted = 1;
	}
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double change_sum = 0;
		double volume_sum = 0;
		
		for(int j = 0; j < sources.GetCount(); j++) {
			Source& s = sources[j];
			ConstBuffer& open_buf = *s.a;
			ConstBuffer& vol_buf  = *s.b;
			bool inverse = s.c;
			
			double open   = open_buf.Get(i-1);
			double close  = open_buf.Get(i);
			double vol    = vol_buf.Get(i);
			double change = open != 0.0 ? (close / open) - 1.0 : 0.0;
			if (inverse) change *= -1.0;
			
			change_sum += change;
			volume_sum += vol;
		}
		
		double prev = open.Get(i-1);
		double change = change_sum / sources.GetCount();
		double value = prev * (1.0 + change);
		double volume_av = volume_sum / sources.GetCount();
		
		open.Set(i, value);
		low.Set(i, value);
		high.Set(i, value);
		volume.Set(i, volume_av);
		
		//LOG(Format("pos=%d open=%f volume=%f", i, value, volume_av));
		
		if (i) {
			low		.Set(i-1, Upp::min(low		.Get(i-1), value));
			high	.Set(i-1, Upp::max(high		.Get(i-1), value));
		}
	}
}















BridgeAskBid::BridgeAskBid() {
	cursor = 0;
}

void BridgeAskBid::Init() {
	SetBufferColor(0, Red());
	SetBufferColor(1, Green());
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	stats.SetCount(force_d0 ? 1 : 5*24*60 / period);
}

void BridgeAskBid::Start() {
	BaseSystem& bs = GetBaseSystem();
	int id = GetSymbol();
	int tf = GetTimeframe();
	int bars = GetBars();
	int counted = GetCounted();
	String id_str = bs.GetSymbol(id).Left(6);
	ASSERTEXC(id >= 0);
	
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	// Open askbid-file
	String local_askbid_file = ConfigFile("askbid.bin");
	FileIn src(local_askbid_file);
	ASSERTEXC(src.IsOpen() && src.GetSize());
	int data_size = src.GetSize();
	
	src.Seek(cursor);
	
	int struct_size = 4 + 6 + 8 + 8;
	
	while ((cursor + struct_size) <= data_size) {
		
		int timestamp;
		double ask, bid;
		src.Get(&timestamp, 4);
		
		String askbid_id;
		for(int i = 0; i < 6; i++) {
			char c;
			src.Get(&c, 1);
			askbid_id.Cat(c);
		}
		if (id_str != askbid_id) {
			src.SeekCur(8+8);
			cursor += struct_size;
			continue;
		}
		src.Get(&ask, 8);
		src.Get(&bid, 8);
		cursor += struct_size;
		
		
		Time time = TimeFromTimestamp(timestamp);
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time) - 1;
		int dh = h + d * h_count;
		
		if (force_d0) {
			h = 0;
			d = 0;
			dh = 0;
		}
		else if (d == -1 || d == 5) {
			continue;
		}
		
		OnlineVariance& var = stats[dh];
		
		if (ask != 0.0 && bid != 0.0) {
			double spread = ask / bid - 1.0;
			if (spread != 0.0)
				var.AddResult(spread);
		}
	}
	
	
	ConstBuffer& open = GetInputBuffer(0, id, tf, 0);
	Buffer& ask = GetBuffer(0);
	Buffer& bid = GetBuffer(1);
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double spread = 0;
		
		Time t = bs.GetTimeTf(GetTf(), i);
		int h = (t.minute + t.hour * 60) / period;
		int d = DayOfWeek(t) - 1;
		int dh = h + d * h_count;
		if (force_d0) {
			h = 0;
			d = 0;
			dh = 0;
		}
		if (d == -1 || d == 5) {
			
		}
		else {
			OnlineVariance& var = stats[dh];
			if (var.GetEventCount()) {
				spread = var.GetMean();
			}
		}
		
		double ask_value = open.Get(i);
		double bid_value = ask_value / (spread + 1.0);
		
		ask.Set(i, ask_value);
		bid.Set(i, bid_value);
	}
}











SymbolSource::SymbolSource() {
	
}

void SymbolSource::Init() {
	
}

void SymbolSource::Start() {
	Buffer& open   = GetBuffer(0);
	Buffer& low    = GetBuffer(1);
	Buffer& high   = GetBuffer(2);
	Buffer& volume = GetBuffer(3);
	
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	int sym_count = common.GetSymbolCount();
	int sym = GetSymbol();
	int cur = sym - sym_count;
	int tf = GetTimeframe();
	int counted = GetCounted();
	int bars = GetBars();
	int input = cur < 0 ? 0 : 1; // DataBridgeValue or VirtualNode
	
	ConstBuffer& src_open   = GetInputBuffer(input, sym, tf, 0);
	ConstBuffer& src_low    = GetInputBuffer(input, sym, tf, 1);
	ConstBuffer& src_high   = GetInputBuffer(input, sym, tf, 2);
	ConstBuffer& src_volume = GetInputBuffer(input, sym, tf, 3);
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		open	.Set(i, src_open.Get(i));
		low		.Set(i, src_low.Get(i));
		high	.Set(i, src_high.Get(i));
		volume	.Set(i, src_volume.Get(i));
	}
}

}
