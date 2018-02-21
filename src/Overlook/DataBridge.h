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
	TimeStop since_last_askbid_refresh;
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

class DataBridge {
	
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
	int sym_id = -1, tf_id = -1, period = -1;
	int counted = 0;
	bool once = true;
	
	int GetSymbol() const {return sym_id;}
	int GetTf() const {return tf_id;}
	int GetCounted() const {return counted;}
	int GetPeriod() const {ASSERT(period != -1); return period;}
	void ForceSetCounted(int i) {counted = i;}
	
	void RefreshFromHistory(bool use_internet_data);
	void RefreshFromInternet();
	void RefreshFromAskBid(bool init_round);
	void RefreshMedian();
	
public:
	typedef DataBridge CLASSNAME;
	DataBridge();
	~DataBridge();
	
	
	Vector<double> open, low, high, volume;
	Vector<int> time;
	
	/*void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction)
			% Out(5, 5)
			% Mem(open)
			% Mem(low)
			% Mem(high)
			% Mem(volume)
			% Mem(time)
			% Mem(point)
			% Mem(spread_mean) % Mem(spread_count)
			% Mem(cursor) % Mem(cursor2)
			% Mem(median_max_map) % Mem(median_min_map)
			% Mem(symbols)
			% Mem(ext_data)
			% Mem(sym_group_stats) % Mem(sym_groups)
			% Mem(median_max) % Mem(median_min)
			% Mem(max_value) % Mem(min_value);
	}*/
	
	void Init();
	void Start();
	void Assist(int cursor, VectorBool& vec);
	
	int GetChangeStep(int shift, int steps);
	double GetPoint() const {return point;}
	double GetMax() const {return max_value * point;}
	double GetMin() const {return min_value * point;}
	double GetMedianMax() const {return median_max * point;}
	double GetMedianMin() const {return median_min * point;}
	double GetAverageSpread() const {return spread_mean;}
	void AddSpread(double a);
	void RefreshFromFaster();
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf > 0 && out_tf == 0;
		return in_sym == out_sym;
	}
};

}

#endif
