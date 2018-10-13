#include "AsmProto.h"



Session::Session() {
	
}

void Session::Clear() {
	
}

void Session::Load(Conf conf) {
	LOG(conf.ToString());
	
	net.Clear();
	net.Parse(conf);
}
