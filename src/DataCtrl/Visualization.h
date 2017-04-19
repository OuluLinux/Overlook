#ifndef _DataCtrl_Visualization_h_
#define _DataCtrl_Visualization_h_

#include "Container.h"

namespace DataCtrl {
using namespace DataCore;

class VisualizationCtrl;

class VisualizationDraw : public Ctrl {
	/*VisualizationCtrl* runner;
	Vector<Vector<double> > tmp;
	Vector<Point> pts;
	Vector<double> xsteps;
	One<ImageDraw> tmp_draw;
	SlotPtr src;
	int sym, tf;
	int week;
	int mode;*/
	
public:
	typedef VisualizationDraw CLASSNAME;
	VisualizationDraw();
	void Init(int mode, VisualizationCtrl* r);
	void RefreshDraw();
	
	virtual void Paint(Draw& w);
	
};

class Visualization : public MetaNodeCtrl {
	
protected:
	/*friend class VisualizationDraw;
	Vector<SlotProcessAttributes> attrs;
	int last_total_duration, last_total, last_total_ready;
	bool running, stopped;
	
	void Run();*/
	
public:
	typedef Visualization CLASSNAME;
	Visualization();
	~Visualization();
	
	void Refresher();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "visualization";}
	static String GetKeyStatic()  {return "visualization";}
	
};

}


#endif
