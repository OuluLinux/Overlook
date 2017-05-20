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

class DataBridge : public Core {
	
public:
	typedef DataBridge CLASSNAME;
	DataBridge();
	~DataBridge();
	
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(SourcePhase, TimeValue, SymTf);
		reg.AddOut(SourcePhase, DataBridgeValue, SymTf, 4, 0);
	}
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	void Serialize(Stream& s);
};

class VirtualNode : public Core {
	
public:
	typedef VirtualNode CLASSNAME;
	VirtualNode();
	~VirtualNode();
	
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(SourcePhase, DataBridgeValue, Sym);
		reg.AddOut(SourcePhase, VirtualNodeValue, SymTf, 4, 0);
	}
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
};

class SymbolSource : public Core {

public:
	typedef SymbolSource CLASSNAME;
	SymbolSource();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(SourcePhase, DataBridgeValue, SymTf);
		reg.AddIn(SourcePhase, VirtualNodeValue, SymTf);
		reg.AddOut(SourcePhase, RealValue, SymTf, 4, 0);
	}
	
};

class BridgeAskBid : public Core {
	Vector<double> ask, bid;
	int cursor;
	
protected:
	friend class CostMeanProfit;
	friend class CostProfitDistribution;
	Vector<OnlineVariance> stats;
	
	
public:
	BridgeAskBid();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % ask % bid /*% ask_high % bid_low*/ % cursor;}
	
	virtual void Start();
	virtual void Init();
	virtual void Arguments(ArgumentBase& args);
	
};

}

#endif
