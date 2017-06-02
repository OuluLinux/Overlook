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

enum {REGIN_NORMAL, REGIN_OPTIONAL, REGIN_DYNAMIC, REGIN_HIGHPRIO};

struct ValueBase {
	int phase, type, scale, count, visible, data_type;
	const char* s0;
	void* data;
	ValueBase() {phase=-1; type=-1; scale=-1; count=0; visible=0; s0=0; data=0; data_type = -1;}
	enum {IN_, INOPT_, INDYN_, INHIGHPRIO_, OUT_, BOOL_, INT_, DOUBLE_, TIME_, STRING_, PERS_BOOL_, PERS_INT_, PERS_DOUBLE_, PERS_INTMAP_, PERS_QUERYTABLE_};
};

struct ValueRegister {
	ValueRegister() {}
	
	virtual void IO(const ValueBase& base) = 0;
	virtual ValueRegister& operator % (const ValueBase& base) {IO(base); return *this;}
};

struct FactoryValueRegister : public ValueRegister, Moveable<FactoryValueRegister> {
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

class Core;
class CustomCtrl;

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
	
	template <class CoreT> static int Find() {
		CoreFactoryPtr factory_fn = &Factory::CoreFactoryFn<CoreT>;
		const Vector<CoreCtrlFactory>& facs = CtrlFactories();
		for(int i = 0; i < facs.GetCount(); i++) {
			if (facs[i].b == factory_fn)
				return i;
		}
		return -1;
	}
};


}

#endif
