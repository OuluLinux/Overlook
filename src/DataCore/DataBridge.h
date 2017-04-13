#ifndef _DataCore_DataBridge_h_
#define _DataCore_DataBridge_h_

#include <plugin/libmt/libmt.h>
#include "Slot.h"

namespace DataCore {

class DataBridge : public Slot {
	MetaTrader mt;
	
	bool connected;
	String account_server;
	String addr;
	int port;
	Vector<Symbol> symbols;
	Vector<int> tfs;
	Vector<bool> loaded;
	VectorMap<int, double> points;
	bool enable_bardata;
	bool has_written;
	
	struct AskBid : Moveable<AskBid> {
		int time;
		double ask, bid;
	};
	
public:
	DataBridge();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "db";}
	virtual String GetName() {return "DataBridge";}
	
	/*virtual String GetName() {return "DataBridge";}
	virtual String GetShortName() const {return "databridge";}
	virtual int SetArguments(const VectorMap<String, Value>& args);
	virtual int Init();
	virtual int Start();*/
	
	
	void Serialize(Stream& s);
	
	void DownloadRemoteData();
	int  DownloadHistory(Symbol& sym, int tf, bool force=false);
	int  DownloadAskBid();
	int  DownloadRemoteFile(String remote_path, String local_path);
	
	MetaTrader& GetMetaTrader() {return mt;}
	const Symbol& GetSymbol(int i) const {return symbols[i];}
	int GetSymbolCount() const {return symbols.GetCount();}
	int GetTf(int i) const {return tfs[i];}
	int GetTfCount() const {return tfs.GetCount();}
	
};

}


#endif
