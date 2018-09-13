#ifndef _Overlook_AdvisorSystem_h_
#define _Overlook_AdvisorSystem_h_

namespace Overlook {

class Advisor {
	
	
protected:
	friend class AdvisorSystem;
	
	static const int ARG_MAX_COUNT = 10;
	
	SimBroker broker;
	int active_net = -1;
	bool active_signal;
	
	int arg[ARG_MAX_COUNT], arg_min[ARG_MAX_COUNT], arg_max[ARG_MAX_COUNT];
	int arg_count = 0;
public:
	
	Advisor();
	void SetArgCount(int i) {arg_count = i;}
	void SetArg(int i, int min, int max) {arg_min[i] = min; arg_max[i] = max;}
	void SetSignal(int net, bool signal) {active_net = net; active_signal = signal;}
	
	int GetArgCount() const {return arg_count;}
	
	virtual void Serialize(Stream& s) {};
	virtual void Tick() = 0;
	void CycleBroker();
	
};

class AdvisorInput : Moveable<AdvisorInput> {
	
	struct StatCollectTemp : Moveable<StatCollectTemp> {
		int end, var;
		double open;
		bool signal;
		
		void Serialize(Stream& s) {s % end % var % open % signal;}
	};
	
	static constexpr double grade_div = 0.05;
	static const int grade_count = 2;
	static const int event_count = 5;
	
	// Persistent
	Vector<StatCollectTemp> temp;
	Vector<OnlineVariance> anomaly_var;
	Vector<OnlineVarianceWindow> av;
	OnlineStdDevWindow stdav;
	int type = -1, tf_min = -1, arg0 = -1, arg1 = -1;
	int prev_var = -1;
	double prev = 0.0;
	double mean = 0.0;
	bool prev_enabled = false;
	bool signal = false;
	bool is_straight_trigger = false, is_inverse_trigger = false;
	
	
	
	
public:
	
	void Serialize(Stream& s) {
		s % temp % anomaly_var % av % stdav % type % tf_min % arg0 % arg1 % prev_var % prev % mean % prev_enabled % signal % is_straight_trigger % is_inverse_trigger;
	}
	
	void Set(int type, int tf_min, int arg0=0, int arg1=0) {this->type = type; this->tf_min = tf_min; this->arg0 = arg0; this->arg1 = arg1; Init();}
	
	void Init();
	void Tick(int cursor, double open, const Time& t, int wday);
	
	bool IsTriggered() const {return is_straight_trigger;}
	bool IsInverseTriggered() const {return is_inverse_trigger;}
	bool GetSignal() const {return signal;}
	bool GetInverseSignal() const {return !signal;}
	double GetMean() const {return mean;}
};

class AdvisorSystem : public Common {
	
public:
	
	typedef Advisor* (*AdvisorFactoryPtr)();
	typedef Tuple<String, AdvisorFactoryPtr> AdvisorSource;
	
	template <class T> static Advisor*		AdvisorSourceFn() { return new T; }
	inline static Vector<AdvisorSource>&	AdvisorFactories() {static Vector<AdvisorSource> list; return list;}
	
	template <class AdvisorT> static void	Register(String name) {
		AdvisorFactories().Add(AdvisorSource(name, &AdvisorSystem::AdvisorSourceFn<AdvisorT>));
	}
	
protected:
	friend class Advisor;
	
	
	// Persistent
	Array<Advisor> advisors;
	Vector<Vector<Vector<AdvisorInput> > > inputs;
	int cursor = 0;
	
	// Temporary
	CoreList cores, pricecores;
	
	
public:
	typedef AdvisorSystem CLASSNAME;
	AdvisorSystem();
	
	virtual void Init();
	virtual void Start();
	
	bool Tick();
	
	Vector<Vector<Vector<AdvisorInput> > >& GetInputs() {return inputs;}
	int GetCursor() const {return cursor;}
	
	void	LoadThis() {LoadFromFile(*this, ConfigFile("AdvisorSystem.bin"));}
	void	StoreThis() {StoreToFile(*this, ConfigFile("AdvisorSystem.bin"));}
	void	Serialize(Stream& s) {
		s % inputs % cursor;
		for(int i = 0; i < advisors.GetCount(); i++)
			s % advisors[i];
	}
	
};

inline AdvisorSystem& GetAdvisorSystem() {return GetSystem().GetCommon<AdvisorSystem>();}


class AdvisorSystemCtrl : public CommonCtrl {
	ArrayCtrl list, valuelist;
	DropList symlist, modellist, mslist;
	
	/*struct Drawer : public Ctrl {
		int model = -1, msi = -1;
		Vector<Point> cache;
		virtual void Paint(Draw& d) {
			if (model == -1 || msi == -1) return;
			Size sz(GetSize());
			ImageDraw id(sz);
			id.DrawRect(sz, White());
			DrawVectorPolyline(id, sz, GetAdvisorSystem().GetModel(model).GetSetting(msi).history, cache);
			d.DrawImage(0, 0, id);
		}
	};*/
	
	//Drawer drawer;
	
public:
	typedef AdvisorSystemCtrl CLASSNAME;
	AdvisorSystemCtrl();
	
	virtual void Data();
	
};

}

#endif
