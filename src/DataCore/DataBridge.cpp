#include "DataCore.h"


namespace DataCore {

DataBridge::DataBridge()  {
	port = 0;
	enable_bardata = true;
	has_written = false;
	running = false;
	stopped = true;
	vnode_begin = -1;
	
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
	s % mt % connected % account_server % symbols % tfs;
}

void DataBridge::Init() {
	if (addr.IsEmpty() || !port) throw DataExc("No address and port");
	
	//PathResolver& res = GetResolver();
	TimeVector& tv = GetTimeVector();
	
	try {
		mt.Init(addr, port);
		connected = mt.AccountNumber();
		account_server = mt.AccountServer();
		
		symbols <<= mt.GetSymbols();
		
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
	
	// Find currencies
	VectorMap<String, int> currencies;
	for(int i = 0; i < symbols.GetCount(); i++) {
		Symbol& s = symbols[i];
		tv.AddSymbol(s.name);
		if (s.IsForex() && s.name.GetCount() == 6) {
			String a = s.name.Left(3);
			String b = s.name.Right(3);
			currencies.GetAdd(a, 0)++;
			currencies.GetAdd(b, 0)++;
		}
	}
	SortByValue(currencies, StdGreater<int>()); // sort by must pairs having currency
	this->currencies.Clear();
	for(int i = 0; i < currencies.GetCount(); i++) {
		this->currencies.Add(currencies.GetKey(i));
	}
	
	
	// Assert that first tf matches base period
	if (tv.GetBasePeriod() != tfs[0] * 60) {
		throw DataExc("Resolver's base period differs from mt");
	}
	
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	loaded.SetCount(sym_count * tf_count, false);
	has_written = false;
	
	// Add BridgeBarData
	/*int pair_count = 0;
	Index<String> node_ids;
	for(int i = 0; i < symbols.GetCount(); i++) {
		
		// Link frontpage
		Symbol& sym = symbols[i];
		bool is_pair = sym.IsForex();
		String frontpage_link = "/symbols/" + sym.name;
		String frontpage_path = "/fp?id=" + IntStr(i) + "&symbol=\"" + sym.name + "\"";
		frontpage_link.Replace("#", "");
		res.ResolvePath(frontpage_path);
		res.LinkPath(frontpage_link, frontpage_path);
		if (is_pair) {
			String a = sym.name.Left(3);
			String b = sym.name.Right(3);
			if (node_ids.Find(a) == -1) node_ids.Add(a);
			if (node_ids.Find(b) == -1) node_ids.Add(b);
			String pair_link = "/pairs/" + sym.name;
			res.LinkPath(pair_link, frontpage_path);
			pair_count++;
		}
		
		int id = res.GetNewId();
		ASSERT(id == i); // mt id must match resolver id
		
		
		if (enable_bardata) {
			for(int j = 0; j < tfs.GetCount(); j++) {
				int tf = tfs[j] / tfs[0];
				String path = Format("/bbd?id=%d&symbol=\"%s\"&period=%d", i, sym.name, tf);
				if (is_pair) {
					path += "&key0=\"" + sym.name.Left(3) + "\"&key1=\"" + sym.name.Right(3) + "\"";
				}
				else if (!sym.currency_base.IsEmpty()) {
					path += "&key0=\"" + sym.currency_base + "\"";
				}
				String link1 = "/symbols/" + sym.name + "/tf" + IntStr(tf);
				String link2 = "/id/id" + IntStr(i) + "/tf" + IntStr(tf);
				String link3 = "/rev_id/id" + IntStr(i) + "/tf" + IntStr(tf);
				String path2 = "/revbd?id=" + IntStr(i) + "&period=" + IntStr(tf);
				link1.Replace("#", "");
				DataVar dv = res.ResolvePath(path);
				if (!dv.Is()) return 1;
				res.LinkPath(link1, path);
				res.LinkPath(link2, path);
				res.LinkPath(link3, path2);
				dv->SetId(i);
				if (is_pair) {
					String link3 = "/pairs/" + sym.name + "/tf" + IntStr(tf);
					res.LinkPath(link3, path);
				}
			}
		}
	}
	
	for(int i = 0; i < node_ids.GetCount(); i++) {
		const String& key = node_ids[i];
		int id = res.GetNewId();
		
		String frontpage_link = "/nodes/" + key;
		String frontpage_path = "/fp?id=" + IntStr(id) + "&symbol=\"" + key + "\"";
		res.LinkPath(frontpage_link, frontpage_path);
		
		if (enable_bardata) {
			for(int j = 0; j < tfs.GetCount(); j++) {
				String period_str = IntStr(tfs[j] / tfs[0]);
				String path = "/bmc?symbol=\"" + key + "\"&period=" + period_str;
				DataVar dv = res.ResolvePath(path);
				if (!dv.Is()) return 1;
				dv->SetId(id);
				String link1 = "/nodes/" + key + "/tf" + period_str;
				String link2 = "/id/id" + IntStr(id) + "/tf" + period_str;
				res.LinkPath(link1, path);
				res.LinkPath(link2, path);
			}
		}
	}
	*/
	


	Panic("TODO vnode_begin");
	// set vnode_begin
	
	demo.Init(mt);
	
	
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

int DataBridge::DownloadHistory(Symbol& sym, int tf, bool force) {
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
	TimeVector& tv = GetTimeVector();
	
	LOG(Format("sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	//if (attr.GetCounted() < attr.GetBars()-1)
	//	return has_written;
	
	ASSERT(attr.GetCounted() == 0);
	
	if (vnode_begin != -1 && attr.sym_id >= vnode_begin) {
		//ProcessVirtualNode(attr);
		Panic("TODO");
	}
	
	
	// Open data-file
	int period = attr.GetPeriod();
	int mt_period = period * tv.GetBasePeriod() / 60;
	String symbol = symbols[attr.sym_id].name;
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
	/*open.SetCount(bars);
	low.SetCount(bars);
	high.SetCount(bars);
	vol.SetCount(bars);*/
	
	
	// Read the history file
	int digits;
	src.Seek(4+64+12+4);
	src.Get(&digits, 4);
	if (digits > 20)
		return false;
	double point = 1.0 / pow(10.0, digits);
	points.GetAdd(attr.sym_id) = point;
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
			points.GetAdd(attr.sym_id) = point;
			// TODO: check all data and don't rely on close
		}
		
		while (cur < time && count < bars) {
			*GetValuePos<double>(0, attr.sym_id, attr.tf_id, count, attr)	= prev_close;
			*GetValuePos<double>(1, attr.sym_id, attr.tf_id, count, attr)	= prev_close;
			*GetValuePos<double>(2, attr.sym_id, attr.tf_id, count, attr)	= prev_close;
			*GetValuePos<double>(3, attr.sym_id, attr.tf_id, count, attr)	= 0;
			SetReady(count, attr, true);
			cur += step;
			count++;
		}
		
		prev_close = close;
		
		if (count < bars && time == cur) {
			//ASSERT(GetTime().GetTime(tf, count) == TimeFromTimestamp(time));
			*GetValuePos<double>(0, attr.sym_id, attr.tf_id, count, attr)	= open;
			*GetValuePos<double>(1, attr.sym_id, attr.tf_id, count, attr)	= low;
			*GetValuePos<double>(2, attr.sym_id, attr.tf_id, count, attr)	= high;
			*GetValuePos<double>(3, attr.sym_id, attr.tf_id, count, attr)	= tick_volume;
			SetReady(count, attr, true);
			cur += step;
			count++;
		}
		
		//LOG(Format("%d: %d %f %f %f %f %d %d %d", cursor, (int)time, open, high, low, close, tick_volume, spread, real_volume));
	}
	
	while (count < bars) {
		*GetValuePos<double>(0, attr.sym_id, attr.tf_id, count, attr)	= open;
		*GetValuePos<double>(1, attr.sym_id, attr.tf_id, count, attr)	= open;
		*GetValuePos<double>(2, attr.sym_id, attr.tf_id, count, attr)	= open;
		*GetValuePos<double>(3, attr.sym_id, attr.tf_id, count, attr)	= 0;
		SetReady(count, attr, true);
		cur += step;
		count++;
	}
	
	has_written = true;
	
	return true;
}
















VirtualNode::VirtualNode() {
	
}

void VirtualNode::SetArguments(const VectorMap<String, Value>& args) {
	/*
	int i = args.Find("symbol");
	if (i == -1) return 1;
	symbol = args[i];
	
	i = args.Find("period");
	if (i == -1) return 1;
	period = args[i];
	
	return 0;
	*/
}

void VirtualNode::Init() {
	/*
	BarData::Init();
	
	if (period == -1) return 1;
	SetPeriod(period);
	
	SetSkipSetCount();
	
	PathResolver& res = GetResolver();
	String this_path = GetPath();
	
	// Find all pairs
	String pairpath = "/pairs";
	PathLink* pairs = res.FindLinkPath(pairpath);
	if (!pairs) return 1;
	int count = pairs->keys.GetCount();
	if (!count) return 1;
	
	for(int i = 0; i < count; i++) {
		const String& key = pairs->keys.GetKey(i);
		
		String k0 = key.Left(3);
		String k1 = key.Right(3);
		if (k0 != symbol && k1 != symbol) continue;
		
		String period_str = IntStr(GetPeriod());
		
		// Add the PairData
		String path = pairpath + "/" + key + "/tf" + period_str;
		PathLink* link = res.FindLinkPath(path);
		if (!link) {
			LOG("MeshNodeConverter::Init: Not a linked path: " << path);
			return 1;
		}
		
		BarData* bd = link->link.Get<BarData>();
		if (!bd) {
			LOG("MeshNodeConverter::Init: Not a BarData: " << pairpath << "/" << key);
			return 1;
		}
		
		if (k0 == symbol) {
			PairData& p = pairs0.Add();
			p.bardata = link->link;
			bd0.Add(bd);
		} else {
			PairData& p = pairs1.Add();
			p.bardata = link->link;
			bd1.Add(bd);
		}
		
	}
	
	
	return 0;
	*/
}

bool VirtualNode::Process(const SlotProcessAttributes& attr) {
	/*
	int bars = GetTime().GetCount(period);
	
	int count0 = pairs0.GetCount();
	int count1 = pairs1.GetCount();
	pos.SetCount(count0 + count1, 0);
	
	for(int i = 0; i < count0; i++)
		bd0[i]->Refresh();
	for(int i = 0; i < count1; i++)
		bd1[i]->Refresh();
	
	int counted = GetCounted();
	int begin = max(counted-1, 1);
	
	SetPeriod(period);
	open.SetCount(bars);
	low.SetCount(bars);
	high.SetCount(bars);
	vol.SetCount(bars);
	
	double prev = 1.0;
	if (begin > 1) {
		prev = open.Get(begin-1);
	} else {
		open	.Set(0, 1.0);
		low		.Set(0, 1.0);
		high	.Set(0, 1.0);
		vol		.Set(0, 0.0);
	}
	
	// Loop time
	for(int i = begin; i < bars; i++) {
		
		double change = 0;
		double volume = 0;
		
		for(int j = 0; j < count0; j++) {
			const Data32f& open = bd0[j]->GetOpen();
			double diff = open.Get(i) - open.Get(i-1);
			change -= diff;
			volume += bd0[j]->GetVolume().Get(i);
		}
		
		for(int j = 0; j < count1; j++) {
			const Data32f& open = bd1[j]->GetOpen();
			double diff = open.Get(i) - open.Get(i-1);
			change += diff;
			volume += bd1[j]->GetVolume().Get(i);
		}
		
		double value = prev * (1 + change * 0.01);
		
		open	.Set(i,   value);
		high	.Set(i-1, max(prev, value));
		low		.Set(i-1, min(prev,  value));
		vol		.Set(i,   volume);
		
		prev = value;
	}
	
	high.Set(bars-1, open.Get(bars-1));
	low.Set(bars-1, open.Get(bars-1));
	
	
	ForceSetCounted(bars);
	
	return 0;
	*/
}

}
