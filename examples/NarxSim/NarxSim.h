#ifndef _NarxSim_NarxSim_h
#define _NarxSim_NarxSim_h

#include <CtrlLib/CtrlLib.h>
#include <NARX/NARX.h>
using namespace Upp;

#define IMAGECLASS Imgs
#define IMAGEFILE <NarxSim/NarxSim.iml>
#include <Draw/iml_header.h>



#define LAYOUTFILE <NarxSim/NarxSim.lay>
#include <CtrlCore/lay.h>

void NormalizeF();

class NarxSim : public TopWindow {
	NARX mynarx;
	
	TabCtrl tabs;
	DocEdit logctrl;
	Button next, prev;
	
	WithTrainSourceLayout<ParentCtrl>	train;
	WithSeriesLayout<ParentCtrl>		seriestab;
	WithArchLayout<ParentCtrl>			archtab;
	WithParamLayout<ParentCtrl>			params;
	WithTrainingLayout<ParentCtrl>		training;
	WithPredictLayout<ParentCtrl>		predict;
	
	
	
	double series_start, series_end;
	ARCH arch;
	int series_generated;
	int old_M;
	int series_len;
	int test_len;
	int train_len;
	int series_func;
	int series_noise;
	int narx_stage1_5;
	int narx_stage1_1;
	int normalize;
	Vector<Vector<double> > series;
	Vector<Vector<double> > exogenous_series;
	Vector<Vector<double> > Nexogenous_series;
	Vector<Vector<double> > Nseries;
	
	Vector<double> Nvariance ;
	Vector<double> N_exo_variance;
	
	Vector<double> N_E ;
	Vector<double> N_exo_E;
	
	int N;
	
	
	Vector<bool> used_exogenous;
	
	int epochs = 100;
	
	int M = 0;
	

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
	void NormalizeF();
};


#if 0
class NARX2 : public QMainWindow {
	Q_OBJECT

public:
	NARX2(QWidget* parent = 0, Qt::WFlags flags = 0);
	~NARX2();

public slots:
	

public:
	Ui::NARX2Class ui;
};
#endif


#endif
