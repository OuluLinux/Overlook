#ifndef _Overlook_Automation_h_
#define _Overlook_Automation_h_


namespace Overlook {
using namespace libmt;


struct AutomationSignal : Moveable<AutomationSignal> {
	String symbol;
	Time from, till;
	bool sig = 0;
	double open, tp, sl;
	
	void Serialize(Stream& s) {s % symbol % from % till % sig % open % tp % sl;}
};

class Automation : public Common {
	
protected:
	friend class AutomationCtrl;
	
	// Persistent
	Vector<AutomationSignal> signals;
	
	// Temporary
	Index<String> symbols;
	Time last_update;
	int mode = 0;
	
	enum {NO_PENDING, PENDING, WAITING};
	
	void LoadThis() {LoadFromFile(*this, ConfigFile("Automation.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("Automation.bin"));}
	
	void RemoveOrder(const Order& o);
public:
	typedef Automation CLASSNAME;
	Automation();
	
	virtual void Init();
	virtual void Start();
	
	void Serialize(Stream& s) {s % signals;}
};


inline Automation& GetAutomation() {return GetSystem().GetCommon<Automation>();}

class AutomationCtrl : public CommonCtrl {
	Splitter split;
	ArrayCtrl list;
	WithAutomationCtrl<ParentCtrl> parent;
	Vector<String> symbols;
	
public:
	typedef AutomationCtrl CLASSNAME;
	AutomationCtrl();
	
	void Save();
	virtual void Data();
};



}

#endif
