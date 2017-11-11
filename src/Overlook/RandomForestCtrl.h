#if 0

#ifndef _Overlook_RandomForestCtrl_h_
#define _Overlook_RandomForestCtrl_h_

namespace Overlook {

class RandomForestCtrl : public Ctrl {
	RandomForest* tree = NULL;
	Data* data = NULL;
	double ss = 50.0;
	double density = 10; // density of drawing. Decrease for higher resolution (but slower)
	bool drawSoft = true;
	
public:
	typedef RandomForestCtrl CLASSNAME;
	RandomForestCtrl();
	
	void ToggleDrawing() {drawSoft = !drawSoft; Refresh();}
	
	void SetData(Data& data) {this->data = &data;}
	void SetTree(RandomForest& tree) {this->tree = &tree;}
	
	virtual void Paint(Draw& w);
	virtual void LeftDown(Point p, dword keyflags);
	
	Callback3<double, double, bool> WhenItem;
	
	
	double d0=0, d1=0, d2=0;
};

#define LAYOUTFILE <Overlook/RandomForestCtrl.lay>
#include <CtrlCore/lay.h>

class RandomForestTester : public WithRandomForestLayout<TopWindow> {
	
public:
	VectorBool full_mask;
	RandomForest tree;
	Data data;
	int N = 10; // number of data points
	bool dirty = true;
	
public:
	typedef RandomForestTester CLASSNAME;
	RandomForestTester();
	
	void Init();
	void Retrain();
	void AddItem(double x, double y, bool value);
	void ToggleType();
	void ToggleDrawing();
	void DoRefresh();
};

}

#endif
#endif
