#ifndef _Overlook_Factory_h_
#define _Overlook_Factory_h_

namespace Overlook {
using namespace Upp;

class Factory {
	
protected:

	typedef Core*			(*CoreFactoryPtr)();
	typedef Pipe*			(*PipeFactoryPtr)();
	typedef CustomCtrl*		(*CtrlFactoryPtr)();
	typedef Tuple3<String, CoreFactoryPtr, CtrlFactoryPtr> CoreCtrlFactory;
	typedef Tuple3<String, PipeFactoryPtr, CtrlFactoryPtr> PipeCtrlFactory;
	
	static void AddCustomCtrl(const String& name, CoreFactoryPtr f, CtrlFactoryPtr c);
	static void AddCustomPipe(const String& name, PipeFactoryPtr f, CtrlFactoryPtr c);
	template <class T> static Core*			CoreFactoryFn() { static One<T> t; if (t.IsEmpty()) t.Create(); return &*t; }
	template <class T> static Pipe*			PipeFactoryFn() { return new T; }
	template <class T> static CustomCtrl*	CtrlFactoryFn() { static One<T> t; if (t.IsEmpty()) t.Create(); return &*t; }
	inline static Vector<CoreCtrlFactory>&	CtrlFactories() {return Single<Vector<CoreCtrlFactory> >();}
	inline static Vector<PipeCtrlFactory>&	PipeFactories() {return Single<Vector<PipeCtrlFactory> >();}
	
	static Vector<Vector<ValueType> >& Outputs() {static Vector<Vector<ValueType> > v; return v;}
	static Vector<Vector<ValueType> >& Inputs() {static Vector<Vector<ValueType> > v; return v;}
	static Vector<Vector<ValueType> >& OptionalInputs() {static Vector<Vector<ValueType> > v; return v;}
	
	
	
public:
	static void Init();
	static void Deinit();
	
	template <class CoreT, class CtrlT> static void Register(String name) {
		AddCustomCtrl(name, &Factory::CoreFactoryFn<CoreT>, &Factory::CtrlFactoryFn<CtrlT>);
	}
	
	template <class PipeT, class CtrlT> static void RegisterPipe(String name) {
		AddCustomPipe(name, &Factory::PipeFactoryFn<PipeT>, &Factory::CtrlFactoryFn<CtrlT>);
		PipeT::GetIO(Inputs().Add(), OptionalInputs().Add(), Outputs().Add());
	}
	
	template <class CoreT> static CoreT& GetCore() {return *dynamic_cast<CoreT*>(CoreFactoryFn<CoreT>());}
	
	inline static const Vector<CoreCtrlFactory>& GetCtrlFactories() {return CtrlFactories();}
	inline static const Vector<PipeCtrlFactory>& GetPipeFactories() {return PipeFactories();}
	static const Vector<Vector<ValueType> >& GetFactoryOutputs() {return Outputs();}
	static const Vector<Vector<ValueType> >& GetFactoryInputs() {return Inputs();}
	static const Vector<Vector<ValueType> >& GetFactoryOptionalInputs() {return OptionalInputs();}
	static int GetCtrlFactoryCount() {return GetCtrlFactories().GetCount();}
	static int GetPipeFactoryCount() {return GetPipeFactories().GetCount();}
	
};


}

#endif
