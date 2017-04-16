#include "DataCore.h"

namespace DataCore {

SimBroker::SimBroker() {
	
}

void SimBroker::Cycle() {
	
}


bool SimBroker::IsZeroSignal() const {
	
	
	return false;
}

int SimBroker::GetOpenOrderCount() const {
	
	
	return 0;
}

double SimBroker::GetWorkingMemoryChange() const {
	
	
	return 0;
}

double SimBroker::GetPreviousCycleChange() const {
	
	
	return 0;
}

double SimBroker::GetEquityFactor() const {
	
	
	return 0;
}


void SimBroker::SetSignal(int sym, int signal) {
	
}

void SimBroker::SetEquityFactor(double d) {
	
}

}
