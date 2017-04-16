#ifndef _DataCore_SimBroker_h_
#define _DataCore_SimBroker_h_

#include <CoreUtils/CoreUtils.h>

namespace DataCore {
using namespace RefCore;

class SimBroker : Moveable<SimBroker> {
	double balance, equity;
	double leverage;
	String currency;
	
public:
	SimBroker();
	
	void Cycle();
	
	bool IsZeroSignal() const;
	int GetOpenOrderCount() const;
	double GetWorkingMemoryChange() const;
	double GetPreviousCycleChange() const;
	double GetEquityFactor() const;
	
	void SetSignal(int sym, int signal);
	void SetEquityFactor(double d);
};




}

#endif
