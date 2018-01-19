#ifndef _Overlook_DataBridge_h_
#define _Overlook_DataBridge_h_

#include <plugin/libmt/libmt.h>

namespace Config {
extern Upp::IniBool use_internet_m1_data;
}

namespace Overlook {
using namespace libmt;

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
	
	
	void Init();
public:
	DataBridgeCommon();
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
	
	Mutex lock;
};

inline DataBridgeCommon& GetDataBridgeCommon() {return Single<DataBridgeCommon>();}

struct AskBid : Moveable<AskBid> {
	int time;
	double ask, bid;
};

struct CorrelationUnit : Moveable<CorrelationUnit> {
	int period;
	Vector<int> sym_ids;
	Vector<OnlineAverageWindow2> averages;
	Vector<Buffer> buffer;
	
	Vector<ConstBuffer*> opens, vols;
	
	void Serialize(Stream& s) {s % period % sym_ids % averages % buffer;}
};

class DataBridge : public BarData {
	
protected:
	friend class CommonForce;
	
	VectorMap<int,int> median_max_map, median_min_map;
	VectorMap<int,int> symbols;
	Vector<Vector<byte> > ext_data;
	Vector<Vector<int> > sym_group_stats, sym_groups;
	Vector<CorrelationUnit> corr;
	double spread_mean;
	int spread_count;
	int median_max, median_min;
	int max_value, min_value;
	int cursor;
	bool slow_volume, day_volume;
	bool once = true;
	
	void RefreshFromHistory(bool use_internet_data);
	void RefreshFromInternet();
	void RefreshFromAskBid(bool init_round);
	void RefreshMedian();
	void RefreshAccount();
	void RefreshCommon();
	void RefreshCorrelation();
	void ProcessCorrelation(int output);
	
public:
	typedef DataBridge CLASSNAME;
	DataBridge();
	~DataBridge();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction)
			% Out(4, 4)
			% Mem(spread_mean) % Mem(spread_count)
			% Mem(cursor)
			% Mem(median_max_map) % Mem(median_min_map)
			% Mem(symbols)
			% Mem(ext_data)
			% Mem(sym_group_stats) % Mem(sym_groups)
			% Mem(corr)
			% Mem(median_max) % Mem(median_min)
			% Mem(max_value) % Mem(min_value);
	}
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	int GetChangeStep(int shift, int steps);
	double GetMax() const {return max_value * point;}
	double GetMin() const {return min_value * point;}
	double GetMedianMax() const {return median_max * point;}
	double GetMedianMin() const {return median_min * point;}
	double GetAverageSpread() const {return spread_mean;}
	void AddSpread(double a);
	void RefreshFromFaster();
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		
		// NOTE: breaks a lot of stuff
		#if ONLY_M1_SOURCE
		if (in_sym == -1)
			return in_tf > 0 && out_tf == 0;
		return in_sym == out_sym;
		#endif
		
		
		if (in_sym == -1) {
			return in_tf == out_tf;
		}
		
		auto& sys = ::Overlook::GetSystem();
		int common_id = sys.FindCommonSymbolId(in_sym);
		if (common_id != -1)
			return common_id == in_sym && common_id != out_sym && sys.FindCommonSymbolPos(common_id, out_sym) != -1;
		
		return false;
	}
};

}

#endif
