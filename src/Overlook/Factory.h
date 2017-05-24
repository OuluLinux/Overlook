#ifndef _Overlook_Factory_h_
#define _Overlook_Factory_h_

namespace Overlook {
using namespace Upp;

struct RegisterInput : Moveable<RegisterInput> {
	int phase, type, scale, input_type;
	void* data;
	RegisterInput(int p, int t, int s, int intype, void* filter_fn) {phase=p; type=t; scale=s; input_type=intype; data=filter_fn;}
	RegisterInput(const RegisterInput& o) {phase=o.phase; type=o.type; scale=o.scale; input_type=o.input_type; data=o.data;}
	String ToString() const {return ValueType(phase, type, scale).ToString() + Format(" input_type=%d data=%X", input_type, (int64)data);}
};

enum {REGIN_NORMAL, REGIN_OPTIONAL, REGIN_DYNAMIC};

struct FactoryValueRegister : public ValueRegister, Moveable<FactoryValueRegister> {
	typedef Tuple2<String, int> ArgType;
	
	//Vector<ValueType> in, out, inopt, indyn;
	Vector<RegisterInput> in;
	Vector<ValueType> out;
	Vector<ArgType> args;
	
	virtual void IO(const ValueBase& base) {
		if (base.data_type == ValueBase::IN_) {
			//in.Add(ValueType(base.phase, base.type, base.scale));
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_NORMAL, NULL));
		}
		else if (base.data_type == ValueBase::INOPT_) {
			//inopt.Add(ValueType(base.phase, base.type, base.scale));
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_OPTIONAL, NULL));
		}
		else if (base.data_type == ValueBase::INDYN_) {
			//indyn.Add(ValueType(base.phase, base.type, SymTf));
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_DYNAMIC, base.data));
		}
		else if (base.data_type == ValueBase::OUT_) {
			out.Add(ValueType(base.phase, base.type, base.scale));
		}
		else {
			args.Add(ArgType(base.s0, base.data_type - ValueBase::BOOL_));
		}
	}
	
};


class Factory {
	
protected:

	typedef Core*			(*CoreFactoryPtr)();
	typedef CustomCtrl*		(*CtrlFactoryPtr)();
	typedef Tuple3<String, CoreFactoryPtr, CtrlFactoryPtr> CoreCtrlFactory;
	
	static void AddCustomCtrl(const String& name, CoreFactoryPtr f, CtrlFactoryPtr c);
	template <class T> static Core*			CoreFactoryFn() { return new T; }
	template <class T> static CustomCtrl*	CtrlFactoryFn() { return new T; }
	inline static Vector<CoreCtrlFactory>&	CtrlFactories() {return Single<Vector<CoreCtrlFactory> >();}
	
	// These static values doesn't work between threads, unless a method returns them.
	static Vector<FactoryValueRegister>& Regs() {static Vector<FactoryValueRegister> v; return v;}
	
	
public:
	
	template <class CoreT, class CtrlT> static void Register(String name) {
		AddCustomCtrl(name, &Factory::CoreFactoryFn<CoreT>, &Factory::CtrlFactoryFn<CtrlT>);
		FactoryValueRegister& reg = Regs().Add();
		CoreT().IO(reg); // unfortunately one object must be created, because IO can't be static and virtual at the same time and it is cleaner to use virtual.
	}
	
	template <class CoreT> static CoreT& GetCore() {return *dynamic_cast<CoreT*>(CoreFactoryFn<CoreT>());}
	
	inline static const Vector<CoreCtrlFactory>& GetCtrlFactories() {return CtrlFactories();}
	static int GetCtrlFactoryCount() {return GetCtrlFactories().GetCount();}
	static const Vector<FactoryValueRegister>& GetRegs() {return Regs();}
	
};


}

#endif
