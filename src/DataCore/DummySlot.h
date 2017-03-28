#ifndef _DataCore_DummySlot_h_
#define _DataCore_DummySlot_h_

#include "Slot.h"

namespace DataCore {

class DummyValue : public Slot {
public:
	DummyValue();
	virtual String GetKey() {return "dummyvalue";}
	virtual bool Process(const SlotProcessAttributes& attr);
};

class DummyIndicator : public Slot {
	SlotPtr src;
public:
	DummyIndicator();
	virtual String GetKey() {return "dummyindicator";}
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
};




}

#endif
