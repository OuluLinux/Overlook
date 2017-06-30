#ifndef _Overlook_AICtrl_h_
#define _Overlook_AICtrl_h_

#include <ConvNet/ConvNet.h>
#include <ConvNetCtrl/ConvNetCtrl.h>

namespace Overlook {
using namespace Upp;


class TrainCtrl : public ParentCtrl {
	
public:
	typedef TrainCtrl CLASSNAME;
	TrainCtrl();
	
	Session ses;
	SpinLock ticking_lock;
	
	Splitter tasksplit, leftsplit, rightsplit;
	
	ArrayCtrl threadlist, tasklist;
	ParentCtrl settings;
	Label lrate, lmom, lbatch, ldecay;
	EditDouble rate, mom, decay;
	EditInt batch;
	Button apply, save_net, load_net;
	
	TrainingGraph graph;
	Label status;
	
	LayerCtrl aenc_view;
	SessionConvLayers layer_view;
};


class TrainerThread {
	TrainCtrl train;
	
public:
	typedef TrainerThread CLASSNAME;
	TrainerThread();
	
	
	
};


class LevelCtrl {
	Array<TrainerThread> thrds;
	
public:
	typedef LevelCtrl CLASSNAME;
	LevelCtrl();
	virtual ~LevelCtrl();
	
	
	
};


class CurrencyLevel : public LevelCtrl {
	
	
public:
	typedef CurrencyLevel CLASSNAME;
	CurrencyLevel();
	
	
};


class AICtrl : public ParentCtrl {
	Splitter hsplit;
	Array<LevelCtrl> levels;
	
public:
	typedef AICtrl CLASSNAME;
	AICtrl();
	~AICtrl();
	
	Session& GetSession() {return ses;}
	
	void Refresher();
	void ApplySettings();
	void OpenFile();
	void SaveFile();
	void Reload();
	void RefreshStatus();
	void UpdateNetParamDisplay();
	void ResetAll();
	void PostReload() {PostCallback(THISBACK(Reload));}
	void StepInterval(int steps);
	void LoadData();
};


}

#endif
