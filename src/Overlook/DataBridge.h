#ifndef _Overlook_DataBridge_h_
#define _Overlook_DataBridge_h_

namespace Overlook {

class DataBridge;

class DataBridgeCommon {
	
protected:
	friend class DataBridge;
	
	Vector<int> tfs;
	Vector<bool> loaded;
	Vector<double> points;
	String account_server;
	String addr;
	int port;
	int sym_count;
	bool connected;
	bool inited;
	
public:
	DataBridgeCommon();
	void Init(DataBridge* db);
	
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
	void RefreshFromHistory();
	void RefreshFromAskBid();
public:
	typedef DataBridge CLASSNAME;
	DataBridge();
	~DataBridge();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, TimeValue, SymTf);
		reg % Out(SourcePhase, DataBridgeValue, SymTf, 4, 3);
	}
	
	virtual void Init();
	virtual void Start();
	
};

class VirtualNode : public BarData {
	
public:
	typedef VirtualNode CLASSNAME;
	VirtualNode();
	~VirtualNode();
	
	virtual void IO(ValueRegister& reg) {
		reg % InDynamic(SourcePhase, DataBridgeValue, &FilterFunction);
		reg % In(SourcePhase, TimeValue, SymTf);
		reg % Out(SourcePhase, VirtualNodeValue, SymTf, 4, 3);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf == out_tf;
		MetaTrader& mt = GetMetaTrader();
		int sym_count = mt.GetSymbolCount();
		int cur = in_sym - sym_count;
		if (cur < 0)
			return false;
		if (out_sym >= sym_count)
			return false;
		const Currency& c = mt.GetCurrency(cur);
		for(int i = 0; i < c.pairs0.GetCount(); i++)
			if (c.pairs0[i] == out_sym)
				return true;
		for(int i = 0; i < c.pairs1.GetCount(); i++)
			if (c.pairs1[i] == out_sym)
				return true;
		return false;
	}
	
	virtual void Init();
	virtual void Start();
	
};

class SymbolSource : public BarData {

public:
	typedef SymbolSource CLASSNAME;
	SymbolSource();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, DataBridgeValue, SymTf);
		reg % In(SourcePhase, VirtualNodeValue, SymTf);
		reg % In(SourcePhase, TimeValue, SymTf);
		reg % Out(SourcePhase, RealValue, SymTf, 4, 3);
	}
	
};

class BridgeAskBid : public Core {
	int cursor;
	
protected:
	friend class CostMeanProfit;
	friend class CostProfitDistribution;
	friend class SpreadStats;
	Vector<OnlineVariance> stats;
	
public:
	BridgeAskBid();
	
	
	virtual void Start();
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(SourcePhase, AskBidValue, SymTf, 2, 2)
			% Persistent(cursor);
	}
	
	const OnlineVariance& GetStat(int i) const {return stats[i];}
	
};

}

#endif
