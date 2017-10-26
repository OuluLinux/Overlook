#ifndef _RandomForest_RandomForest_h
#define _RandomForest_RandomForest_h

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include "Forest.h"

namespace RandomForest {

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
	
};



#ifdef flagMAIN

#define LAYOUTFILE <RandomForest/RandomForest.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS RandomForextImg
#define IMAGEFILE <RandomForest/RandomForext.iml>
#include <Draw/iml_header.h>


class RandomForestDemo : public WithRandomForestLayout<TopWindow> {
	
public:
	RandomForest tree;
	Data data;
	int N = 10; // number of data points
	bool dirty = true;
	
public:
	typedef RandomForestDemo CLASSNAME;
	RandomForestDemo();
	
	void Init();
	void Retrain();
	void AddItem(double x, double y, bool value);
	void ToggleType();
	void ToggleDrawing();
};

#endif

}

#endif
