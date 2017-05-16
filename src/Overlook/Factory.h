#ifndef _Overlook_Factory_h_
#define _Overlook_Factory_h_

namespace Overlook {
using namespace Upp;

struct FactoryValueRegister : public ValueRegister {
	Vector<ValueType> *in, *out, *inopt;
	
	virtual void AddIn(int phase, int type, int scale, int count) {in->Add(ValueType(phase, type, scale, count));}
	virtual void AddInOptional(int phase, int type, int scale, int count) {inopt->Add(ValueType(phase, type, scale, count));}
	virtual void AddOut(int phase, int type, int scale, int count) {out->Add(ValueType(phase, type, scale, count));}
	
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
	
	static Vector<Vector<ValueType> >& Outputs() {static Vector<Vector<ValueType> > v; return v;}
	static Vector<Vector<ValueType> >& Inputs() {static Vector<Vector<ValueType> > v; return v;}
	static Vector<Vector<ValueType> >& OptionalInputs() {static Vector<Vector<ValueType> > v; return v;}
	
	
	
public:
	
	template <class CoreT, class CtrlT> static void Register(String name) {
		AddCustomCtrl(name, &Factory::CoreFactoryFn<CoreT>, &Factory::CtrlFactoryFn<CtrlT>);
		FactoryValueRegister reg;
		reg.in = &Inputs().Add();
		reg.inopt = &OptionalInputs().Add();
		reg.out = &Outputs().Add();
		CoreT().GetIO(reg); // unfortunately one object must be created, because GetIO can't be static and virtual at the same time and it is cleaner to use virtual.
	}
	
	template <class CoreT> static CoreT& GetCore() {return *dynamic_cast<CoreT*>(CoreFactoryFn<CoreT>());}
	
	inline static const Vector<CoreCtrlFactory>& GetCtrlFactories() {return CtrlFactories();}
	static const Vector<Vector<ValueType> >& GetFactoryOutputs() {return Outputs();}
	static const Vector<Vector<ValueType> >& GetFactoryInputs() {return Inputs();}
	static const Vector<Vector<ValueType> >& GetFactoryOptionalInputs() {return OptionalInputs();}
	static int GetCtrlFactoryCount() {return GetCtrlFactories().GetCount();}
	
};


}

#endif
