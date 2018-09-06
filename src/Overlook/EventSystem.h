#if 0

#ifndef _Overlook_EventSystem_h_
#define _Overlook_EventSystem_h_

namespace Overlook {



class EventSystem {
	
protected:
	VectorMap<String, double> level;
	VectorMap<int, String> cached_mail;
	CIMAPClient client;
	int email_count;
	bool email_init = false;
	
	
public:
	typedef EventSystem CLASSNAME;
	EventSystem();
	~EventSystem();
	
	void InitMail();
	int GetMailCount();
	String GetMail(int i);
	void Data();
	void ProcessAutoChartist(String s);
	bool HasLevel(String s) {return level.Find(s) != -1;}
	double GetLevel(String s) {return level.Get(s);}
	
};

inline EventSystem& GetEventSystem() {return Single<EventSystem>();}

}

#endif
#endif
