#ifndef _Overlook_AutoencCtrl_h_
#define _Overlook_AutoencCtrl_h_

#include <ConvNet/ConvNet.h>
#include <ConvNetCtrl/ConvNetCtrl.h>

namespace Overlook {
using namespace Upp;

class AutoencCtrl : public ParentCtrl {
	Session ses;
	SpinLock ticking_lock;
	
	
	Splitter hsplit, leftsplit, rightsplit;
	
	ParentCtrl settings;
	Label lrate, lmom, lbatch, ldecay;
	EditDouble rate, mom, decay;
	EditInt batch;
	Button apply, save_net, load_net;
	DocEdit net_edit;
	
	TrainingGraph graph;
	Label status;
	
	LayerCtrl aenc_view;
	SessionConvLayers layer_view;
	
public:
	typedef AutoencCtrl CLASSNAME;
	AutoencCtrl();
	~AutoencCtrl();
	
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
	
};

}

#endif
