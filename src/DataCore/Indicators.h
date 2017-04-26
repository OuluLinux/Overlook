#ifndef _DataCore_Indicators_h_
#define _DataCore_Indicators_h_

#include "Slot.h"

namespace DataCore {
using namespace Upp;


enum {MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA};
enum {MODE_SIMPLE, MODE_EXPONENTIAL, MODE_SMOOTHED, MODE_LINWEIGHT};


class MovingAverage : public Slot {
	int ma_period;
	int ma_shift;
	int ma_method;
	
protected:
	
	bool Simple(const SlotProcessAttributes& attr);
	bool Exponential(const SlotProcessAttributes& attr);
	bool Smoothed(const SlotProcessAttributes& attr);
	bool LinearlyWeighted(const SlotProcessAttributes& attr);
	
public:
	MovingAverage();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetKey() const {return "ma";}
	virtual String GetName() {return "MovingAverage";}
	virtual String GetShortName() const {return SHORTNAME3("ma", ma_period, ma_shift, ma_method);}
};

}

#endif
