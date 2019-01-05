#ifndef _Overlook_Scripts_h_
#define _Overlook_Scripts_h_

namespace Overlook {

void InitSessionDefault(ConvNet::Session& ses, int input_depth, int output_count);
void LoadSymbol(CoreList& cl_sym, int symbol, int tf);
void LoadSymbolIndicators(CoreList& cl_indi, int symbol, int tf);
void LoadNetSymbols(CoreList& cl_sym, int tf);
void LoadNetIndicators(CoreList& cl_indi, int tf);
void LoadNets(CoreList& cl_net, int tf);
void LoadNetsIndicators(CoreList& cl_indi, int tf);
void LoadDataPriceInput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int windowsize);
void LoadVolumePriceInput(CoreList& cl_net, int pos, ConvNet::Volume& in, int windowsize);
void LoadDataIndiInput(ConvNet::Session& ses, CoreList& cl_indi, int begin, int count, int windowsize);
void LoadVolumeIndicatorsInput(CoreList& cl_indi, int pos, ConvNet::Volume& in, int windowsize);
void LoadDataPipOutput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int postpips_count);
void LoadDataVolatOutput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int postpips_count);
void TrainSession(ConvNet::Session& ses, int iterations, int& actual);
String TestPriceInPipOut(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int windowsize, int postpips_count);
String TestIndicatorsInPipOut(ConvNet::Session& ses, CoreList& cl_net, CoreList& cl_indi, int begin, int count, int windowsize, int postpips_count);
String TestPriceInVolatOut(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int windowsize, int ticks);
String TestTrade(int symbol, int postpips, LabelSignal& signal);



class ScriptList {
	
	Vector<FactoryDeclaration> script_ids;
	Vector<Ptr<ScriptCoreItem> > script_queue;
public:
	
	FactoryDeclaration& AddFactory(int factory) {return script_ids.Add().Set(factory);}
	ScriptCore& GetScript(int i) {return *script_queue[i]->core;}
	
	void Init();
	
};

class ScriptCommon : public Common {
	
protected:
	friend class ScriptCtrl;
	friend class ScriptList;
	
	// Persistent
	
	
	// Temporary
	Vector<Ptr<ScriptCoreItem> > process_queue;
	ThreadSignal sig;
	int queue_cursor = 0;
	
	
public:
	typedef ScriptCommon CLASSNAME;
	ScriptCommon();
	~ScriptCommon() {sig.Stop();}
	
	virtual void Init();
	virtual void Start();
	virtual void Deinit();
	void Process();
};

inline ScriptCommon& GetScriptCommon() {return GetSystem().GetCommon<ScriptCommon>();}

class ScriptCtrl : public CommonCtrl {
	Splitter hsplit;
	ArrayCtrl scriptlist;
	RichTextView result;
	String prev_result;
	
public:
	typedef ScriptCtrl CLASSNAME;
	ScriptCtrl();
	
	virtual void Data();
};
}

#endif
