#ifndef _DataCtrl_DervCtrl_h_
#define _DataCtrl_DervCtrl_h_

#include "Container.h"


namespace DataCtrl {
using namespace DataCore;

class DervDraw : public Ctrl {
	Vector<Vector<double> > tmp;
	SlotPtr src;
	int sym, tf;
	int week;
	
public:
	typedef DervDraw CLASSNAME;
	DervDraw();
	
	virtual void Paint(Draw& w);
	
};

#define LAYOUTFILE <DataCtrl/DervCtrl.lay>
#include <CtrlCore/lay.h>

class DervCtrl : public WithDervCtrlLayout<MetaNodeCtrl> {
	
public:
	typedef DervCtrl CLASSNAME;
	DervCtrl();
	
	void Refresher();
	
	virtual String GetKey() const {return "derv";}
	static String GetKeyStatic()  {return "derv";}
	
};

}

#endif
