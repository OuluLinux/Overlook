#ifndef _Overlook_ForeSignal_h_
#define _Overlook_ForeSignal_h_

namespace Overlook {
using namespace libmt;


struct SingleForeSignal : Moveable<SingleForeSignal> {
	Time time;
	String symbol;
	
	void Serialize(Stream& s) {s % time % symbol;}
};

class ForeSignal : public Common {
	
protected:
	friend class ForeSignalCtrl;
	
	// Persistent
	Vector<SingleForeSignal> signals;
	
	// Temporary
	bool email_init = false;
	int email_count = 0;
	CIMAPClient client;
	TimeStop ts;
	
	void InitMail();
	int GetMailCount();
	String GetMail(int i);
	void MailData();
	
	void LoadThis() {LoadFromFile(*this, ConfigFile("ForeSignal.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("ForeSignal.bin"));}
	
public:
	typedef Signal CLASSNAME;
	ForeSignal();
	
	virtual void Init();
	virtual void Start();
	
	void Serialize(Stream& s) {s % signals;}
	
	
	Callback2<int, String> WhenEvent;
};

inline ForeSignal& GetForeSignal() {return GetSystem().GetCommon<ForeSignal>();}


class ForeSignalCtrl : public CommonCtrl {
	ArrayCtrl list;
	
public:
	typedef ForeSignalCtrl CLASSNAME;
	ForeSignalCtrl();
	
	virtual void Data();
};



}

#endif
