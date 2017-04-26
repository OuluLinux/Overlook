#ifndef _DataCore_DummySlot_h_
#define _DataCore_DummySlot_h_

#include "Slot.h"

namespace DataCore {

class DummyValue : public Slot {
public:
	DummyValue();
	virtual String GetKey() const {return "dummyvalue";}
	virtual bool Process(const SlotProcessAttributes& attr);
};

class DummyIndicator : public Slot {
	
public:
	DummyIndicator();
	virtual String GetKey() const {return "dummyindicator";}
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
};

class DummyOscillator : public Slot {
public:
	DummyOscillator();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "dummyoscillator";}
};

class DummyTrainer : public Slot {
public:
	DummyTrainer();
	virtual String GetKey() const {return "dummytrainer";}
	virtual bool Process(const SlotProcessAttributes& attr);
};

class TestValue : public Slot {
	int series_noise;
public:
	TestValue();
	virtual String GetKey() const {return "testvalue";}
	virtual bool Process(const SlotProcessAttributes& attr);
};




}

#endif
