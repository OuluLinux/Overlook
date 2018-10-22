#ifndef _Forecaster_System_h_
#define _Forecaster_System_h_

namespace Forecast {

class System {
	
public:

	typedef Core* (*CoreFactoryPtr)();
	typedef Tuple<String, CoreFactoryPtr, CoreFactoryPtr> CoreSystem;
	
	static void								AddCustomCore(const String& name, CoreFactoryPtr f, CoreFactoryPtr singlef);
	template <class T> static Core*			CoreSystemFn() { return new T; }
	template <class T> static Core*			CoreSystemSingleFn() { return &Single<T>(); }
	inline static Vector<CoreSystem>&		CoreFactories() {static Vector<CoreSystem> list; return list;}
	inline static Vector<int>&				Indicators() {static Vector<int> list; return list;}
	
public:
	
	template <class CoreT> static void		Register(String name) {
		int id = CoreFactories().GetCount();
		Indicators().Add(id);
		AddCustomCore(name, &System::CoreSystemFn<CoreT>, &System::CoreSystemSingleFn<CoreT>);
	}
	
	template <class CoreT> static CoreT&	GetCore() {return *dynamic_cast<CoreT*>(CoreSystemFn<CoreT>());}
	
	inline static const Vector<CoreSystem>&	GetCoreFactories() {return CoreFactories();}
	
	template <class CoreT> static int		Find() {
		CoreFactoryPtr System_fn = &System::CoreSystemFn<CoreT>;
		const Vector<CoreSystem>& facs = CoreFactories();
		for(int i = 0; i < facs.GetCount(); i++) {
			if (facs[i].b == System_fn)
				return i;
		}
		return -1;
	}
	
	
public:
	
	static void GetCoreQueue(const Vector<double>& real_data, Vector<CoreItem>& queue, const Vector<FactoryDeclaration>& decl);
	static void GetBitStream(Vector<CoreItem>& work_queue, BitStream& stream);
	static void GetLabels(Vector<CoreItem>& work_queue, Vector<ConstLabelSignal*>& lbls);
	
};

}

#endif
