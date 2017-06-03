#ifndef _Overlook_DataBridge_h_
#define _Overlook_DataBridge_h_

namespace Overlook {

class DataBridge;

class DataBridgeCommon {
	
protected:
	friend class DataBridge;
	
	typedef Tuple3<Time, double, double> AskBid;
	Vector<Vector<AskBid> > data;
	Vector<int> tfs;
	Vector<bool> loaded;
	Vector<double> points;
	Index<String> short_ids;
	String account_server;
	String addr;
	TimeStop since_last_askbid_refresh;
	int port;
	int sym_count;
	int cursor;
	bool connected;
	bool inited;
	
	void RefreshAskBidData();
	void Init(DataBridge* db);
public:
	DataBridgeCommon();
	void CheckInit(DataBridge* db);
	
	const Symbol& GetSymbol(int i) const {return GetMetaTrader().GetSymbol(i);}
	int GetSymbolCount() const {return GetMetaTrader().GetSymbolCount();}
	int GetTf(int i) const {return tfs[i];}
	int GetTfCount() const {return tfs.GetCount();}
	void DownloadRemoteData();
	int  DownloadHistory(const Symbol& sym, int tf, bool force=false);
	int  DownloadAskBid();
	int  DownloadRemoteFile(String remote_path, String local_path);
	bool IsInited() const {return inited;}
	
	Mutex lock;
};

struct AskBid : Moveable<AskBid> {
	int time;
	double ask, bid;
};

class DataBridge : public BarData {
	QueryTable spread_qt, volume_qt;
	VectorMap<int,int> median_max_map, median_min_map;
	double point;
	int median_max, median_min;
	int max_value, min_value;
	int cursor, buffer_cursor;
	bool slow_volume, day_volume;
	
	void RefreshFromHistory();
	void RefreshFromAskBid(bool init_round);
	void RefreshVirtualNode();
	void RefreshMedian();
public:
	typedef DataBridge CLASSNAME;
	DataBridge();
	~DataBridge();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, TimeValue, SymTf)
			% InHigherPriority(&FilterFunction)
			% Out(SourcePhase, RealValue, SymTf, 5, 3)
			% Persistent(cursor) % Persistent(buffer_cursor)
			% Persistent(spread_qt) % Persistent(volume_qt)
			% Persistent(max_value) % Persistent(min_value)
			% Persistent(median_max) % Persistent(median_min) % Persistent(median_max_map) % Persistent(median_min_map);
	}
	
	virtual void Init();
	virtual void Start();
	
	int GetChangeStep(int shift, int steps);
	double GetMax() const {return max_value * point;}
	double GetMin() const {return min_value * point;}
	double GetMedianMax() const {return median_max * point;}
	double GetMedianMin() const {return median_min * point;}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		MetaTrader& mt = GetMetaTrader();
		int sym_count = mt.GetSymbolCount();
		
		// Never add timeframes
		if (in_sym == -1)
			return false;
		
		// Never for regular symbols
		if (in_sym < sym_count)
			return false;
		
		// Never for irregular input symbols
		if (out_sym >= sym_count)
			return false;
		
		// Get virtual symbol
		int cur = in_sym - sym_count;
		const Currency& c = mt.GetCurrency(cur);
		if (c.pairs0.Find(out_sym) != -1 || c.pairs1.Find(out_sym) != -1)
			return true; // add pair sources add inputs
		
		return false;
	}
};


class ValueChange : public Core {
	bool has_proxy;
	int proxy_id, proxy_factor;
	DataBridge* db;
	
public:
	ValueChange();
	
	virtual void IO(ValueRegister& reg) {
		reg % InDynamic(SourcePhase, RealValue, &FilterFunction)
			% Out(IndiPhase, RealChangeValue, SymTf, 7, 7);
	}
	
	virtual void Init();
	virtual void Start();
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		// Match SymTf
		if (in_sym == -1)
			return in_tf == out_tf;
		if (in_sym == out_sym)
			return true;
		
		// Enable proxy, if it exists
		MetaTrader& mt = GetMetaTrader();
		if (in_sym < mt.GetSymbolCount()) {
			const Symbol& sym = mt.GetSymbol(in_sym);
			return out_sym == sym.proxy_id;
		}
		return false;
	}
};

}

#endif
