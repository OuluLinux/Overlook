#ifndef _Overlook_CoreList_h_
#define _Overlook_CoreList_h_

namespace Overlook {

class CoreList : Moveable<CoreList> {
	
	
	// Temporary
	Index<String> symbols;
	Vector<String> indi_lbls;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<Vector<Vector<ConstBuffer*> > > bufs;
	Vector<Vector<Vector<ConstLabelSignal*> > > lbls;
	Vector<DataBridge*> db, db_m1;
	Index<int> sym_ids, tf_ids;
	
	
public:
	typedef CoreList CLASSNAME;
	CoreList();
	
	void AddSymbol(String s) {symbols.Add(s);}
	void AddTf(int i) {tf_ids.Add(i);}
	FactoryDeclaration& AddIndi(int factory) {return indi_ids.Add().Set(factory);}
	
	int GetSymbolCount() const {return symbols.GetCount();}
	int GetIndiCount() const {return indi_ids.GetCount();}
	int GetTfCount() const {return tf_ids.GetCount();}
	String GetSymbol(int i) const {return symbols[i];}
	String GetIndiDescription(int i) const;
	int GetTf(int i) const {return tf_ids[i];}
	ConstBuffer& GetBuffer(int sym, int src, int buf) const {return *bufs[sym][src][buf];}
	ConstLabelSignal& GetLabelSignal(int sym, int src, int buf) const {return *lbls[sym][src][buf];}
	int GetLabelSignalCount(int sym, int src) const {return lbls[sym][src].GetCount();}
	bool IsEmpty() {return bufs.IsEmpty();}
	DataBridge* GetDataBridge(int i) {return db[i];}
	DataBridge* GetDataBridgeM1(int i) {return db_m1[i];}
	
	void Init();
	void Refresh();
	
};

}

#endif
