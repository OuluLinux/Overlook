#ifndef _DataCore_Analyzer_h_
#define _DataCore_Analyzer_h_

namespace DataCore {

class Analyzer : public Slot {
	
	
public:
	Analyzer();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void SerializeCache(Stream& s, int sym_id, int tf_id);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetName() {return "Analyzer";}
	virtual String GetShortName() const {return "analyzer";}
	virtual String GetKey() const {return "analyzer";}
	virtual String GetCtrl() const {return "analyzerctrl";}
	
};

}

#endif
