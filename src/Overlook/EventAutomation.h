#ifndef _Overlook_EventAutomation_h_
#define _Overlook_EventAutomation_h_


namespace Overlook {
using namespace libmt;


class EventAutomation : public Common {
	
protected:
	friend class EventAutomationCtrl;
	
	static const int pips_first = 10;
	
	
	// Persistency
	Vector<VectorMap<int, OnlineAverage1> > data;
	int counted = 100;
	
	
	// Temporary
	Vector<Ptr<EventCoreItem> > ci_queue;
	CoreList cl_sym;
	
	
public:
	typedef EventAutomation CLASSNAME;
	EventAutomation();
	
	virtual void Init();
	virtual void Start();
	void Process();
	
	int GetSignalWhichFirst(int sym, int pos);
	
	void LoadThis() {LoadFromFile(*this, GetOverlookFile("EventAutomation.bin"));}
	void StoreThis() {StoreToFile(*this, GetOverlookFile("EventAutomation.bin"));}
	void Serialize(Stream& s) {s % data % counted;}
	
};

inline EventAutomation& GetEventAutomation() {return GetSystem().GetCommon<EventAutomation>();}

class EventAutomationCtrl : public CommonCtrl {
	SliderCtrl slider;
	Upp::Label date;
	ArrayCtrl list;
	
public:
	typedef EventAutomationCtrl CLASSNAME;
	EventAutomationCtrl();
	
	virtual void Data();
	
};


}

#endif
