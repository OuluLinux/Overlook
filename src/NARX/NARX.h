#pragma once

#include <Core/Core.h>
using namespace Upp;

#define MAX_INPUTS_PER_UNIT 100
#define MAX_SERIES_LEN 50000

#include "Unit.h"
#include "InputUnit.h"
#include "OutputUnit.h"
#include "EvaluationEngine.h"
#include "ActivationFunctions.h"
#include "FeedbackInfo.h"



void train_result_log(String);

void normalize_f();

enum ARCH { MLP = 1000, NAR_D, TDNN_X, NARX_D, NARX_Y, NARX_DY, NAR_Y, NAR_DY, UNKNWN};

struct NarxData : Moveable<NarxData> {
	
	Vector<Vector<double> > series;
	Vector<Vector<double> > Nseries;
	Vector<Vector<double> > exogenous_series;
	Vector<Vector<double> > Nexogenous_series;
	int series_len, train_len, test_len;
	int epochs;
	Vector<bool> used_exogenous;

};

class NARX {

protected:
	
	NarxData* data;

	int H;
	int a;
	int b;
	int M;
	int N;
	int feedback, targets;
	int hact;
	ARCH arch;
	
	Vector<Vector<double> > Y;
	Vector<Unit> hunits;
	Vector<OutputUnit> output_units;
	Vector<InputUnit> inputs;
	Vector<InputUnit> feedbacks;
	Vector<InputUnit> exogenous;
	Vector<EvaluationEngine> ee;
	Vector<EvaluationEngine> rw;

	void TrainEpoch(bool logging, int epo);
	
	void Run();

public:
	typedef NARX CLASSNAME;
	NARX();
	~NARX();

	void Init(ARCH arch, int H = 1, int hact = 2, int a = 0, int b = 0, int M = 0, int N = 1, int feedback = 0, int targets = 0);
	void Start() {Thread::Start(THISBACK(Run));}
	void Train(int epochs);
	void Test(int epo);
	ARCH GetArch();
	void Copy(NARX& n);
	void Sum(NARX& n);
	void Divide(int len);

public:
	Callback TrainingEpochFinished;
	Callback1<String> Log;
	
};

