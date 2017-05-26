#ifndef _Overlook_SpreadUtils_h_
#define _Overlook_SpreadUtils_h_


namespace Overlook {


class SpreadStats : public Core {
	
public:
	SpreadStats();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, TimeValue, SymTf);
		reg % In(SourcePhase, AskBidValue, SymTf);
		reg % Out(SourcePhase, SpreadValue, SymTf, 1, 1);
	}
};


class SpreadMeanProfit : public Core {
	BridgeAskBid* askbid;
	
public:
	SpreadMeanProfit();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, TimeValue, SymTf);
		reg % In(SourcePhase, RealValue, SymTf);
		reg % In(SourcePhase, AskBidValue, SymTf);
		reg % Out(SourcePhase, SpreadOvercomeValue, SymTf, 1, 1);
	}
};


class ValueChange : public Core {
	bool has_proxy;
	int proxy_id, proxy_factor;
	DataBridge* db;
	
public:
	ValueChange();
	
	virtual void IO(ValueRegister& reg) {
		reg % InDynamic(SourcePhase, RealValue, &FilterFunction);
		reg % InDynamic(SourcePhase, SpreadValue, &FilterFunction);
		reg % In(SourcePhase, TimeValue, SymTf);
		reg % Out(IndiPhase, RealChangeValue, SymTf, 7, 7);
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



class IdealOrders : public Core {
	SimBroker broker;
	
public:
	IdealOrders();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf);
		reg % Out(IndiPhase, IdealOrderSignal, SymTf);
	}
	
	virtual void Init();
	virtual void Start();
	
};

}

#endif
