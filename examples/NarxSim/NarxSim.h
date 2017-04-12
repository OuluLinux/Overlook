#ifndef _NarxSim_NarxSim_h
#define _NarxSim_NarxSim_h

#include <CtrlLib/CtrlLib.h>
#include <NARX/NARX.h>
using namespace Upp;
using namespace Narx;


#define IMAGECLASS Imgs
#define IMAGEFILE <NarxSim/NarxSim.iml>
#include <Draw/iml_header.h>



#define LAYOUTFILE <NarxSim/NarxSim.lay>
#include <CtrlCore/lay.h>

void NormalizeF();

class NarxSim : public TopWindow, public NarxData {
	NARX mynarx;
	
	TabCtrl tabs;
	ArrayCtrl logctrl;
	Button next, prev;
	
	WithTrainSourceLayout<ParentCtrl>	train;
	WithSeriesLayout<ParentCtrl>		seriestab;
	WithArchLayout<ParentCtrl>			archtab;
	WithParamLayout<ParentCtrl>			params;
	WithTrainingLayout<ParentCtrl>		training;
	WithPredictLayout<ParentCtrl>		predict;
	
	Array<Option> use_options;
	
	
	double series_start, series_end;
	ARCH arch;
	int old_input_count;
	int series_func;
	int series_noise;
	int narx_stage1_5;
	int narx_stage1_1;
	int normalize;
	bool series_generated;
	
	Vector<double> Nvariance ;
	Vector<double> N_exo_variance;
	Vector<double> N_E ;
	Vector<double> N_exo_E;
	
	int output_count;
	int input_count;
	

public:
	typedef NarxSim CLASSNAME;
	NarxSim();
	
	void Next();
	void Previous();
	void Next1();
	void Prev2();
	void Next2();
	void Next3();
	void Prev3();
	void Prev4();
	void Next4();
	void Prev5();
	void Next5();
	void MenuAbout();
	void BrowseAction();
	void StartTrain();
	void Predict();
	void ProgressInc();
	void Log(String s);
	void PostNarxLog(String s);
	void NarxLog(String s);
	void NormalizeF();
	void TrainingFinished() {next.Enable();}
	void SetProgress(int i, int total) {
		training.progress.Set(i, total);
		training.perc.SetLabel(IntStr(i * 100 / total) + "%");
	}
	
};




#endif
