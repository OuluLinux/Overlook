#ifndef _AsmProto_AsmProto_h
#define _AsmProto_AsmProto_h

#include <CtrlLib/CtrlLib.h>
#include <ConvNet/ConvNet.h>
using namespace Upp;


#include "Optimizer.h"
#include "Generator.h"
#include "Regenerator.h"



void DrawVectorPolyline(Draw& d, Size sz, const Vector<double>& pts, Vector<Point>& cache, int max_count=0, double zero_line=0.0);


class Test1 : public TopWindow {
	
	struct DrawLines : public Ctrl {
		Test1* t;
		Vector<Point> polyline;
		virtual void Paint(Draw& d);
	};
	
	DrawLines draw0;
	
	Generator gen;
	Regenerator regen;
	Vector<double> data0;
	Vector<double>* data1;
	bool stopped = true, running = false;
	
public:
	typedef Test1 CLASSNAME;
	Test1();
	~Test1() {running = false; while (!stopped) Sleep(100);}
	
	void Start() {stopped = false; running = true; Thread::Start(THISBACK(Train));}
	void Train();
	void Refresh0() {draw0.Refresh();}
	
};

#endif
