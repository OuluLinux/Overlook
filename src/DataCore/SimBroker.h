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
	
};




}

#endif
