#ifndef _Overlook_DataBridge_h_
#define _Overlook_DataBridge_h_

#include <plugin/libmt/libmt.h>



namespace Overlook {
using namespace libmt;

class DataBridge;

class DataBridgeCommon : public Common {
	
protected:
	friend class DataBridge;
	
	// Persistent
	Vector<Buffer> time_bufs;
	Vector<Index<Time> > idx;
	
	// Temp
	typedef Tuple3<Time, double, double> AskBid;
	Vector<Vector<AskBid> > data;
	Vector<int> tfs;
	Vector<bool> loaded;
	Vector<double> points;
	Index<String> short_ids;
	String account_server;
	TimeStop since_last_askbid_refresh;
	int sym_count;
	int cursor;
	bool connected;
	bool inited;
	
	
	
public:
	DataBridgeCommon();
	
	virtual void Init();
	virtual void Start();
	
	void InspectInit();
	
	const Symbol& GetSymbol(int i) const {return GetMetaTrader().GetSymbol(i);}
	int GetSymbolCount() const {return GetMetaTrader().GetSymbolCount();}
	int GetTf(int i) const {return tfs[i];}
	int GetTfCount() const {return tfs.GetCount();}
	void DownloadRemoteData();
	int  DownloadHistory(const Symbol& sym, int tf, bool force=false);
	int  DownloadHistory(int sym, int tf, bool force=false);
	int  DownloadAskBid();
	int  DownloadRemoteFile(String remote_path, String local_path);
	bool IsInited() const {return inited;}
	void RefreshAskBidData(bool forced=false);
	void RefreshTimeBuffers();
	bool SyncData(int tf, int64 time, int& shift);
	bool IsVtfTime(int wday, const Time& t);
	ConstBuffer& GetTimeBuffer(int tf) const {return time_bufs[tf];}
	const Index<Time>& GetTimeIndex(int i) const {return idx[i];}
	int GetTimeBufferCount() const {return time_bufs.GetCount();}
	void Serialize(Stream& s) {s % time_bufs % idx;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("DataBridgeCommon.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("DataBridgeCommon.bin"));}
	
	Mutex lock;
};

inline DataBridgeCommon& GetDataBridgeCommon() {return GetSystem().GetCommon<DataBridgeCommon>();}

struct AskBid : Moveable<AskBid> {
	int time;
	double ask, bid;
};

class DataBridge : public Core {
	
protected:
	friend class CommonForce;
	
	VectorMap<int,int> median_max_map, median_min_map;
	VectorMap<int,int> symbols;
	Vector<Vector<byte> > ext_data;
	Vector<Vector<int> > sym_group_stats, sym_groups;
	double point = 0.01;
	double spread_mean;
	int spread_count;
	int median_max, median_min;
	int max_value, min_value;
	int cursor = 0, cursor2 = 0;
	bool slow_volume, day_volume;
	bool once = true;
	
	void RefreshFromHistory(bool use_internet_data);
	void RefreshFromInternet();
	void RefreshFromAskBid(bool init_round);
	void RefreshMedian();
	bool SyncData(int64 time, int& shift, double ask);
	
public:
	typedef DataBridge CLASSNAME;
	DataBridge();
	~DataBridge();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction)
			% Out(5, 5)
			% Mem(point)
			% Mem(spread_mean) % Mem(spread_count)
			% Mem(cursor) % Mem(cursor2)
			% Mem(median_max_map) % Mem(median_min_map)
			% Mem(symbols)
			% Mem(ext_data)
			% Mem(sym_group_stats) % Mem(sym_groups)
			% Mem(median_max) % Mem(median_min)
			% Mem(max_value) % Mem(min_value);
	}
	
	virtual void Init();
	virtual void Start();
	
	
	int GetChangeStep(int shift, int steps);
	double GetPoint() const {return point;}
	double GetDigits() const;
	double GetSpread() const {return spread_mean * point;}
	double GetMax() const {return max_value * point;}
	double GetMin() const {return min_value * point;}
	double GetMedianMax() const {return median_max * point;}
	double GetMedianMin() const {return median_min * point;}
	double GetAverageSpread() const {return spread_mean;}
	void AddSpread(double a);
	void RefreshFromFasterTime();
	void RefreshFromFasterChange();
	void RefreshCurrency();
	void RefreshNet();
	
	static bool FilterFunction(void* basesystem, bool match_tf, int in_sym, int in_tf, int out_sym, int out_tf) {
		System& sys = GetSystem();
		
		if (in_tf == VTF) {
			if (match_tf)
				return out_tf == 0;
			return in_sym == out_sym;
		}
		
		if (sys.IsNormalSymbol(in_sym)) {
			#if REFRESH_FROM_FASTER
			if (match_tf)
				return in_tf > 0 && out_tf == 0;
			return in_sym == out_sym;
			#else
			return false;
			#endif
		}
		
		if (sys.IsNetSymbol(in_sym)) {
			#if REFRESH_FROM_FASTER
			if (match_tf)
				return out_tf == 0;
			if (in_tf == 0) {
				const System::NetSetting& net = sys.GetSymbolNet(in_sym);
				return net.symbol_ids.Find(out_sym) != -1;
			}
			else
				return in_sym == out_sym;
			#else
			if (match_tf)
				return out_tf == in_tf;
			const System::NetSetting& net = sys.GetSymbolNet(in_sym);
			return net.symbol_ids.Find(out_sym) != -1;
			#endif
		}
		
		return false;
	}
};



class DataBridgeCtrl : public CommonCtrl {
	
public:
	DataBridgeCtrl() {}
	virtual void Data() {}
	
};

}

#endif
