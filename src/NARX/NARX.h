#pragma once
#include "Unit.h"
#include "InputUnit.h"
#include "OutputUnit.h"
#include "EvaluationEngine.h"


void train_result_log(QString);

void normalize_f();

enum ARCH { MLP = 1000, NAR_D, TDNN_X, NARX_D, NARX_Y, NARX_DY, NAR_Y, NAR_DY, UNKNWN};



class NARX : public QThread
{
	Q_OBJECT

protected:
	
	int H;
	int a;
	int b;
	int M;
	int N;

	int feedback, targets;

	ARCH arch;


	int hact;

	Unit **hunits;
	OutputUnit **output_units;
	InputUnit **inputs;

	InputUnit **feedbacks;

	InputUnit **exogenous;

	double **Y;

	EvaluationEngine **ee;
	EvaluationEngine **rw;

	void trainEpoch(bool logging, int epo);
	void _log(QString str);

	void push_weights();

	void run();

public:
	NARX(ARCH arch, int H = 1, int hact = 2, int a = 0, int b = 0, int M = 0, int N = 1, int feedback = 0, int targets = 0);
	~NARX(void);

	void train(int epochs);

	void test(int epo);

	ARCH getArch();

	void copy(NARX *n);
	void sum(NARX *n);
	void divide(int len);

signals:
	void training_epoch_finished();
	void log();
};

