#ifndef _DataCore_Filters_h_
#define _DataCore_Filters_h_

namespace DataCore {


// Normalization is not correct, because future open values can't be seen due the design.
// This is still better than nothing in some cases.
class NormalizedValue : public Slot {
	String src_path;
	SlotPtr src;
	OnlineVariance var;
public:
	NormalizedValue();
	virtual String GetKey() {return "normvalue";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
};

class DerivedValue : public Slot {
	String src_path;
	SlotPtr src;
public:
	DerivedValue();
	virtual String GetKey() {return "derivedvalue";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
};


}

#endif
