#ifndef _Agent_Agent_h
#define _Agent_Agent_h

#include <CtrlLib/CtrlLib.h>
#include <ConvNet/ConvNet.h>
using namespace Upp;
using namespace ConvNet;

#include "Optimizer.h"
#include "Common.h"
#include "Core.h"
#include "System.h"
#include "Indicators.h"
#include "Looper.h"
#include "Manager.h"

namespace Agent {

struct DrawLines : public Ctrl {
	Task* t = NULL;
	double zero_line = 0.0;
	int type;
	
	enum {DQN};
	
	Vector<Point> polyline;
	
	virtual void Paint(Draw& d);
	void SetTask(Task& t) {this->t = &t;}
};


struct DqnCtrl : public DataCtrl {
	
	DrawLines draw;
	
	typedef DqnCtrl CLASSNAME;
	DqnCtrl();
	
	virtual void Data();
	void SetTask(Task& t) {draw.SetTask(t);}
};

class ManagerCtrl : public TopWindow {
	
protected:
	friend class DrawLines;
	
	
	ParentCtrl main_task;
	Splitter vsplit, hsplit;
	ArrayCtrl seslist, tasklist;
	DataCtrl* prev_ctrl = NULL;
	
	TimeCallback tc;
	
public:
	typedef ManagerCtrl CLASSNAME;
	ManagerCtrl();
	~ManagerCtrl();
	
	void Data();
	void PeriodicalRefresh();
	void SelectTask();
	void SwitchCtrl(DataCtrl* c);
	
};

}

#endif
