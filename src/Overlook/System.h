#ifndef _Overlook_System_h_
#define _Overlook_System_h_

namespace Overlook {
using namespace Upp;

struct RegisterInput : Moveable<RegisterInput> {
	int phase, type, scale, input_type;
	void* data;
	RegisterInput(int p, int t, int s, int intype, void* filter_fn) {phase=p; type=t; scale=s; input_type=intype; data=filter_fn;}
	RegisterInput(const RegisterInput& o) {phase=o.phase; type=o.type; scale=o.scale; input_type=o.input_type; data=o.data;}
	String ToString() const {return ValueType(phase, type, scale).ToString() + Format(" input_type=%d data=%X", input_type, (int64)data);}
};

enum {REGIN_NORMAL, REGIN_OPTIONAL, REGIN_DYNAMIC, REGIN_HIGHPRIO};



struct SystemValueRegister : public ValueRegister, Moveable<SystemValueRegister> {
	typedef Tuple2<String, int> ArgType;
	
	//Vector<ValueType> in, out, inopt, indyn;
	Vector<RegisterInput> in;
	Vector<ValueType> out;
	Vector<ArgType> args;
	
	virtual void IO(const ValueBase& base) {
		if (base.data_type == ValueBase::IN_) {
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_NORMAL, NULL));
		}
		else if (base.data_type == ValueBase::INOPT_) {
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_OPTIONAL, NULL));
		}
		else if (base.data_type == ValueBase::INDYN_) {
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_DYNAMIC, base.data));
		}
		else if (base.data_type == ValueBase::INHIGHPRIO_) {
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_HIGHPRIO, base.data));
		}
		else if (base.data_type == ValueBase::OUT_) {
			out.Add(ValueType(base.phase, base.type, base.scale));
		}
		else {
			args.Add(ArgType(base.s0, base.data_type - ValueBase::BOOL_));
		}
	}
	
};

class System {
	
protected:

	typedef Core*			(*CoreFactoryPtr)();
	typedef CustomCtrl*		(*CtrlFactoryPtr)();
	typedef Tuple3<String, CoreFactoryPtr, CtrlFactoryPtr> CoreCtrlSystem;
	
	static void AddCustomCtrl(const String& name, CoreFactoryPtr f, CtrlFactoryPtr c);
	template <class T> static Core*			CoreSystemFn() { return new T; }
	template <class T> static CustomCtrl*	CtrlSystemFn() { return new T; }
	inline static Vector<CoreCtrlSystem>&	CtrlFactories() {return Single<Vector<CoreCtrlSystem> >();}
	
	// These static values doesn't work between threads, unless a method returns them.
	static Vector<SystemValueRegister>& Regs() {static Vector<SystemValueRegister> v; return v;}
	
	
public:
	
	template <class CoreT, class CtrlT> static void Register(String name) {
		AddCustomCtrl(name, &System::CoreSystemFn<CoreT>, &System::CtrlSystemFn<CtrlT>);
		SystemValueRegister& reg = Regs().Add();
		CoreT().IO(reg); // unfortunately one object must be created, because IO can't be static and virtual at the same time and it is cleaner to use virtual.
	}
	
	template <class CoreT> static CoreT& GetCore() {return *dynamic_cast<CoreT*>(CoreSystemFn<CoreT>());}
	
	inline static const Vector<CoreCtrlSystem>& GetCtrlFactories() {return CtrlFactories();}
	static int GetCtrlSystemCount() {return GetCtrlFactories().GetCount();}
	static const Vector<SystemValueRegister>& GetRegs() {return Regs();}
	
	template <class CoreT> static int Find() {
		CoreFactoryPtr System_fn = &System::CoreSystemFn<CoreT>;
		const Vector<CoreCtrlSystem>& facs = CtrlFactories();
		for(int i = 0; i < facs.GetCount(); i++) {
			if (facs[i].b == System_fn)
				return i;
		}
		return -1;
	}
	
protected:
	friend class DataBridgeCommon;
	
	Vector<int>		tfbars_in_slowtf;
	Vector<int>		bars;
	Index<String>	symbols;
	Index<int>		periods;
	Vector<String>	period_strings;
	String			addr;
	int64			memory_limit;
	int				port;
	
	
protected:
	
	// Time
	Vector<Time> begin;
	Vector<int> begin_ts, end_ts;
	Time end;
	int timediff;
	int base_period;
	
	void Serialize(Stream& s) {s % begin % end % timediff % base_period % begin_ts;}
	
	
public:
	
	//int GetCount(int period) const;
	int GetCountTf(int tf_id) const;
	Time GetTimeTf(int tf, int pos) const;// {return begin + base_period * period * pos;}
	Time GetBegin(int tf) const {return begin[tf];}
	Time GetEnd() const {return end;}
	int GetBeginTS(int tf) {return begin_ts[tf];}
	int GetEndTS(int tf) {return end_ts[tf];}
	int GetBasePeriod() const {return base_period;}
	//int64 GetShift(int src_period, int dst_period, int shift);
	//int64 GetShiftFromTime(int timestamp, int period);
	int64 GetShiftTf(int src_tf, int dst_tf, int shift);
	int64 GetShiftFromTimeTf(int timestamp, int tf);
	int64 GetShiftFromTimeTf(const Time& t, int tf);
	//int GetTfFromSeconds(int period_seconds);
	
	//void SetBegin(Time t)	{begin = t; begin_ts = (int)(t.Get() - Time(1970,1,1).Get());}
	//void SetEnd(Time t)	{end = t; end_ts = (int)(t.Get() - Time(1970,1,1).Get()); timediff = (int)(end.Get() - begin.Get());}
	//void SetBasePeriod(int period)	{base_period = period;}
	
	
public:
	
	void AddPeriod(String nice_str, int period);
	void AddSymbol(String sym);
	
	int GetSymbolCount() const {return symbols.GetCount();}
	String GetSymbol(int i) const {return symbols[i];}
	
	int GetPeriod(int i) const {return periods[i];}
	String GetPeriodString(int i) const {return period_strings[i];}
	int GetPeriodCount() const {return periods.GetCount();}
	int FindPeriod(int period) const {return periods.Find(period);}
	
	
public:
	
	typedef System CLASSNAME;
	System();
	
	virtual void Arguments(ArgumentBase& args) {}
	virtual void Init();
	virtual void Start() {}
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(SourcePhase, TimeValue, All);
	}
};

}

#endif
