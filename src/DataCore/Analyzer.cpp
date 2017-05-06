#include "DataCore.h"

namespace DataCore {

Analyzer::Analyzer() {
	AddValue<double>("");
	
}

void Analyzer::SetArguments(const VectorMap<String, Value>& args) {
	
}

void Analyzer::SerializeCache(Stream& s, int sym_id, int tf_id) {
	
}

void Analyzer::Init() {
	AddDependency("/open", 0, 0);
	AddDependency("/spread", 0, 0);
	AddDependency("/change", 0, 0);
	AddDependency("/ma", 0, 0);
	AddDependency("/whstat_slow", 0, 0);
	AddDependency("/whdiff", 0, 0);
	AddDependency("/chp", 0, 0);
	AddDependency("/eosc", 0, 0);
	AddDependency("/lstm", 0, 0);
	AddDependency("/narx", 0, 0);
	AddDependency("/forecaster", 0, 0);
	AddDependency("/rl", 0, 0);
	AddDependency("/dqn", 0, 0);
	AddDependency("/mona", 0, 0);
	AddDependency("/metamona", 0, 0);
	AddDependency("/doublemona", 0, 0);
	SetProcessedOnce(false);
	
}

bool Analyzer::Process(const SlotProcessAttributes& attr) {
	
	
	return true;
}


}
