#ifndef _Overlook_DataBridge_h_
#define _Overlook_DataBridge_h_

namespace Overlook {

class DataBridge : public Core {
	Vector<int> tfs;
	Vector<bool> loaded;
	Vector<double> points;
	String account_server;
	String addr;
	int port;
	int sym_count;
	bool connected;
	bool enable_bardata;
	bool has_written;
	bool running, stopped;
	
	struct AskBid : Moveable<AskBid> {
		int time;
		double ask, bid;
	};
	
	
public:
	typedef DataBridge CLASSNAME;
	DataBridge();
	~DataBridge();
	
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(SourcePhase, TimeValue, SymTf, 1);
		reg.AddOut(SourcePhase, RealValue, SymTf, 1);
	}
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	
	void Run();
	void Serialize(Stream& s);
	void ProcessVirtualNode(const CoreProcessAttributes& attr);
	
	void DownloadRemoteData();
	int  DownloadHistory(const Symbol& sym, int tf, bool force=false);
	int  DownloadAskBid();
	int  DownloadRemoteFile(String remote_path, String local_path);
	
	const Symbol& GetSymbol(int i) const {return GetMetaTrader().GetSymbol(i);}
	int GetSymbolCount() const {return GetMetaTrader().GetSymbolCount();}
	int GetTf(int i) const {return tfs[i];}
	int GetTfCount() const {return tfs.GetCount();}
	
};

class BridgeAskBid : public Core {
	FloatVector ask, bid;
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
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
};

}

#endif
