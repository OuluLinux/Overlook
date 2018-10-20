#ifndef _AsmProto_AsmProto_h
#define _AsmProto_AsmProto_h

#include <CtrlLib/CtrlLib.h>
#include <ConvNet/ConvNet.h>
using namespace Upp;


#include "CompatAMP.h"
using namespace concurrency;

#include "Common.h"
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
	double* data0;
	double* data1;
	bool stopped = true, running = false;
	
	TimeCallback tc;
	
public:
	typedef Test1 CLASSNAME;
	Test1();
	~Test1() {running = false; while (!stopped) Sleep(100);}
	
	void Start() {stopped = false; running = true; Thread::Start(THISBACK(Train));}
	void Train();
	void Refresh0() {draw0.Refresh();}
	void PeriodicalRefresh() {tc.Set(60, THISBACK(PeriodicalRefresh)); Refresh0();}
};

#endif
