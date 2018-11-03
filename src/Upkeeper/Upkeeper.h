#ifndef _Upkeeper_Upkeeper_h
#define _Upkeeper_Upkeeper_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <Upkeeper/Upkeeper.lay>
#include <CtrlCore/lay.h>

class Upkeeper : public WithUpkeeperLayout<TopWindow> {
	
	
public:
	typedef Upkeeper CLASSNAME;
	Upkeeper();
	
	void Process();
	void Add(String what) {list.Add(GetSysTime(), what); list.ScrollEnd();}
	
};

#endif
