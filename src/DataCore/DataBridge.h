#ifndef _DataCore_DataBridge_h_
#define _DataCore_DataBridge_h_

#include <plugin/libmt/libmt.h>
#include "Slot.h"
#include "SimBroker.h"

namespace DataCore {

class DataBridge : public Slot {
	//MetaTrader mt;
	//Vector<Symbol> symbols;
	Vector<int> tfs;
	Vector<bool> loaded;
	VectorMap<int, double> points;
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
	
	void Run();
	void Serialize(Stream& s);
	void ProcessVirtualNode(const SlotProcessAttributes& attr);
	
	void DownloadRemoteData();
	int  DownloadHistory(const Symbol& sym, int tf, bool force=false);
	int  DownloadAskBid();
	int  DownloadRemoteFile(String remote_path, String local_path);
	
	const Symbol& GetSymbol(int i) const {return GetMetaTrader().GetSymbol(i);}
	int GetSymbolCount() const {return GetMetaTrader().GetSymbolCount();}
	int GetTf(int i) const {return tfs[i];}
	int GetTfCount() const {return tfs.GetCount();}
	virtual String GetCtrl() const {return "dbctrl";}
	virtual int GetCtrlType() const {return SLOT_ONCE;}
	virtual int GetType() const {return SLOT_SYMTF;}
	
	SimBroker demo;
};



}


#endif
