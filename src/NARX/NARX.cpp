#include "NARX.h"






NARX::NARX() {
	
}


void NARX::Init(ARCH arch, int H, int hact, int a, int b, int M, int N, int feedback, int targets) {
	this->H = H;
	this->a = a;
	this->b = b;
	this->arch = arch;
	this->M = M;
	this->N = N;
	this->feedback = feedback;
	this->hact = hact;
	this->targets = targets;
	ee.SetCount(N);
	rw.SetCount(N);

	for (int i = 0; i < N; i++) {
		ee[i].Init(data->train_len);
		rw[i].Init(data->train_len);
	}

	hunits.SetCount(H);

	for (int i = 0; i < H; i++) {
		
		if (hact == 2) {
			hunits[i].set_activation_func(ActivationFunctions::AsLog);
			hunits[i].set_activation_func_derv(ActivationFunctions::AsLogDerv);
		}
		else if (hact == 1) {
			hunits[i].set_activation_func(ActivationFunctions::BSigmoid);
			hunits[i].set_activation_func_derv(ActivationFunctions::BSigmoidDerv);
		}
		else if (hact == 0) {
			hunits[i].set_activation_func(ActivationFunctions::Sigmoid);
			hunits[i].set_activation_func_derv(ActivationFunctions::SigmoidDerv);
		}
	}

	if (targets) {
		inputs.SetCount(b * N);

		for (int i = 0; i < b * N; i++) {
			inputs[i].SetInput(0);
		}

		for (int i = 0; i < H; i++)
			for (int j = 0; j < b * N; j++)
				hunits[i].AddInputUnit(inputs[j]);
	}

	if (feedback) {
		feedbacks.SetCount(N * b);

		for (int i = 0; i < N * b; i++) {
			feedbacks[i].SetInput(0);
		}

		for (int i = 0; i < H; i++)
			for (int j = 0; j < N * b; j++)
				hunits[i].AddInputUnit(feedbacks[j]);
	}

	exogenous.SetCount(M * (a + 1));
	for (int i = 0; i < M * (a + 1); i++) {
		exogenous[i].SetInput(0);
	}

	for (int i = 0; i < H; i++)
		for (int j = 0; j < M * (a + 1); j++)
			hunits[i].AddInputUnit(exogenous[j]);

	//printf("%d\n", index);
	output_units.SetCount(N);

	for (int i = 0; i < N; i++) {
		output_units[i].set_activation_func(ActivationFunctions::Identity);
		output_units[i].set_activation_func_derv(ActivationFunctions::IdentityDerv);
	}

	for (int i = 0; i < H; i++)
		for (int j = 0; j < N; j++)
			output_units[j].AddInputUnit(hunits[i]);

	//hunits[0]->
	LOG(Format("NARX: arch=%1, H=%2, hact =%3, a=%4, b=%5, M=%6, N=%7, feedback=%8, targets= %9\n", arch, H, hact, a, b, M, N, feedback, targets));
}

ARCH NARX::GetArch() {
	return arch;
}

void NARX::Train(int epochs) {
	for (int i = 0; i < epochs; i++) {
		TrainEpoch(false, i);
		TrainingEpochFinished();
	}

	//TrainEpoch(false, epochs);
	//emit TrainingEpochFinished();
}

void NARX::TrainEpoch(bool logging, int epo) {
	//	int ;
	for (int i = 0; i < N; i++) {
		ee[i].Clear();
		rw[i].Clear();
	}

	if (!feedback) {
		int feedback_index = 0;

		for (int series_index = a; series_index < data->train_len ; series_index ++) {
			if (targets) {
				for (int j = 0; j < N; j++) {
					for (int i = 1; i <= b; i++) {
						if (series_index - i >= 0 ) {
							inputs[j * b + i - 1].SetInput(data->Nseries[j][ series_index - i  ]);
							//FLOG(String("setting input:%1\n").arg(Nseries[j][ series_index - i  ]).toStdString().c_str());
						}
						else
							inputs[j * b + i - 1].SetInput(0);
					}
				}
			}

			//exogenous[0]->SetInput(series[series_index]);

			for (int i = 0; i < M; i++) {
				if (data->used_exogenous[i]) {
					for (int j = 0; j < a + 1; j++) {
						if (series_index - j >= 0)
							exogenous[j + i * a].SetInput(data->Nexogenous_series[i][series_index - j]);
						else
							exogenous[j + i * a].SetInput(0);
					}

					//_log(String("ok %1").arg(exogenous_series[i][series_index]));
					//FLOG(String("ok exo=%1\n").arg(exogenous_series[i][series_index]).toStdString().c_str());
				}
			}

			for (int i = 0; i < N; i++) {
				output_units[i].SetTarget(data->Nseries[i][series_index]);
				ee[i].Insert(data->Nseries[i][series_index], output_units[i].GetOutput());
				rw[i].Insert(data->Nseries[i][series_index], data->Nseries[i][series_index > 0 ? series_index - 1 : 0]);
			}

			//FLOG(String("input target:index %1 : %2, output narx: %3\n").arg(series_index).arg(series[series_index]).arg(output_unit->GetOutput()).toStdString().c_str());
			//output_unit
			//a += ;
			for (int i = 0; i < N; i++) {
				output_units[i].FixWeights();
				output_units[i].ComputeDelta();
				output_units[i].AdjustWeights();
			}

			//FLOG(String("ok delta=%1\n").arg(delta).toStdString().c_str());
			for (int i = 0; i < H; i++) {
				double delta = 0;

				for (int j = 0; j < N; j++)
					delta += output_units[j].GetDelta(hunits[i]);

				hunits[i].ComputeDelta(delta);
				hunits[i].AdjustWeights();
			}
		}
	}
	else {
		NARX copynarx;
		NARX originalnarx;
		copynarx.Init(arch, H, hact, a, b, M, N, feedback, targets); // tk targets
		originalnarx.Init(arch, H, hact, a, b, M, N, feedback, targets); // tk targets
		originalnarx.Copy(*this);
		;
		/* must do the feedback arch now */
		/*	 int weight_count = output_units[0]->GetInputCount();
			double *final_weights= new double[weight_count];
			for(int i=0;i<weight_count;i++)
				final_weights[i] = 0;
		*/
		Vector<Vector<double> > Y;
		Y.SetCount(N);
		for (int i = 0; i < N; i++)
			Y[i].SetCount(data->train_len, 0);
		
		Vector<FeedbackInfo> fi;
		fi.SetCount(data->train_len);
		for (int i = 0; i < data->train_len; i++)
			fi[i].Init(M * (a + 1), N * b, N * b);
		
		int t = 0;
		
		for (; t < data->train_len; t++) {
			// if (exogenous)
			for (int i = 0; i < M; i++) {
				if (data->used_exogenous[i])
					for (int j = a; j >= 0 ; j--) {
						if (t >= j)
							exogenous[i * a + j].SetInput(data->Nexogenous_series[i][t - j]);
						else
							exogenous[i * a + j].SetInput(0);

						fi[t].X[i * a + j] = exogenous[i * a + j].GetInput();
					}
			}

			if (targets)
				for (int i = 0; i < N; i++) {
					for (int j = b; j > 0; j--) {
						if (t >= j)
							inputs[i * b + j - 1].SetInput(data->Nseries[i][t - j]);
						else
							inputs[i * b + j - 1].SetInput(0);

						fi[t].D[i * b + j - 1] = inputs[i * b + j - 1].GetInput();
					}
				}

			if (feedback)
				for (int i = 0; i < N; i++) {
					for (int j = b; j > 0; j--) {
						//FLOG(String("testing%1\n").arg(fi[t].Y[i*N+j]).toStdString().c_str());
						if (t >= j)
							feedbacks[i * b + j - 1].SetInput(Y[i][t - j]);
						else
							feedbacks[i * b + j - 1].SetInput(0);

						//FLOG(String("testing%1\n").arg(fi[t].Y[i*N+j]).toStdString().c_str());
						fi[t].Y[i * b + j - 1] = feedbacks[i * b + j - 1].GetInput();
					}
				}

			for (int i = 0; i < N; i++) {
				output_units[i].SetTarget(data->Nseries[i][t]);
				Y[i][t] = output_units[i].GetOutput();
				//FLOG(String("testing%1\n").arg(Y[i][t]).toStdString().c_str());
				ee[i].Insert(data->Nseries[i][t], Y[i][t]);
				rw[i].Insert(data->Nseries[i][t], data->Nseries[i][t > 0 ? t - 1 : 0]);
			}
		}
		
		Vector<Vector<double> > previous_deltas;
		previous_deltas.SetCount(data->train_len);
		for (int i = 0; i < data->train_len; i++) {
			previous_deltas[i].SetCount(N, 0);
		}

		// backpropagating the errors
		for (t = data->train_len - 1; t >= 0; t--) {
			NARX::Copy(originalnarx);

			/* if (exogenous) */
			for (int i = 0; i < M; i++) {
				for (int j = a; j >= 0 ; j--)
					exogenous[i * a + j].SetInput(fi[t].X[i * a + j]);
			}

			if (targets)
				for (int i = 0; i < N; i++) {
					for (int j = b; j > 0; j--)
						inputs[i * b + j - 1].SetInput(fi[t].D[i * b + j - 1]);
				}

			if (feedback)
				for (int i = 0; i < N; i++) {
					for (int j = b; j > 0; j--)
						feedbacks[i * b + j - 1].SetInput(fi[t].Y[i * b + j - 1]);
				}

			for (int i = 0; i < N; i++) {
				output_units[i].FixWeights();
				output_units[i].SetTarget(data->Nseries[i][t]);

				if (t == data->train_len - 1)
					output_units[i].ComputeDelta();
				else {
					//double deltas = 0;
					//for(int j=0;j<b;j++)
					//	if(t+j<train_len)
					//		deltas += previous_deltas[t+j][i];
					//output_units[i].ComputeDelta(deltas);
					output_units[i].ComputeDelta(output_units[i].Error() + previous_deltas[t][i]);
				}

				output_units[i].AdjustWeights();
			}

			for (int i = 0; i < H; i++) {
				double delta = 0;

				for (int j = 0; j < N; j++)
					delta += output_units[j].GetDelta(hunits[i]);

				hunits[i].ComputeDelta(delta);
				hunits[i].FixWeights();
				hunits[i].AdjustWeights();
			}

			for (int k = 0; k < N; k++)
				for (int i = 0; i < b; i++) {
					double delta = 0;

					for (int j = 0; j < H; j++)
						delta += hunits[j].GetDelta(feedbacks[k * N + i]);

					if (t - i - 1 >= 0)
						previous_deltas[t - i - 1][k] += delta;
				}

			if (t == data->train_len - 1)
				copynarx.Copy(*this);
			else
				copynarx.Sum(*this);
		}

		copynarx.Divide(data->train_len);
		NARX::Copy(copynarx);
		originalnarx.Copy(*this);
	}

	if (logging)
		LOG("Epoch finished :)");

	for (int i = 0; i < N; i++) {
		LOG(Format("target %1:epoch %2: F1 = %3; F2 = %4; F3 = %5; F4 = %6; KS1= %7; KS2=%8; KS12=%9; DA = %10"
					 "; F1RW=%11; F2RW=%12; F3RW=%13; F4RW=%14; KS1=%15; KS2=%16; KS12=%17; DA=%18",
			  i,
			  epo,
			  ee[i].F1(), ee[i].F2(), ee[i].F3(), ee[i].F4(),
			  ee[i].KS1(),
			  ee[i].KS2(), ee[i].KS12(), ee[i].DA(),
			  rw[i].F1(), rw[i].F2(), rw[i].F3(), rw[i].F4(),
			  rw[i].KS1(), rw[i].KS2(), rw[i].KS12(), rw[i].DA()));
	}

	Test(epo);
}


NARX::~NARX() {
	
}

void NARX::Run() {
	Train(data->epochs);
}

void NARX::Copy(NARX& n) {
	for (int i = 0; i < M * (a + 1); i++)
		exogenous[i].Copy(n.exogenous[i]);

	if (targets)
		for (int i = 0; i < N * b; i++)
			inputs[i].Copy(n.inputs[i]);

	if (feedback)
		for (int i = 0; i < N * b; i++)
			feedbacks[i].Copy(n.feedbacks[i]);

	for (int i = 0; i < H; i++)
		hunits[i].Copy(n.hunits[i]);

	for (int i = 0; i < N; i++)
		output_units[i].Copy(n.output_units[i]);
}

void NARX::Sum(NARX& n) {
	for (int i = 0; i < M * (a + 1); i++)
		exogenous[i].Sum(n.exogenous[i]);

	if (targets)
		for (int i = 0; i < N * b; i++)
			inputs[i].Sum(n.inputs[i]);

	if (feedback)
		for (int i = 0; i < N * b; i++)
			feedbacks[i].Sum(n.feedbacks[i]);

	for (int i = 0; i < H; i++)
		hunits[i].Sum(n.hunits[i]);

	for (int i = 0; i < N; i++)
		output_units[i].Sum(n.output_units[i]);
}

void NARX::Divide(int len) {
	for (int i = 0; i < M * (a + 1); i++)
		exogenous[i].Divide(len);

	if (targets)
		for (int i = 0; i < N * b; i++)
			inputs[i].Divide(len);

	if (feedback)
		for (int i = 0; i < N * b; i++)
			feedbacks[i].Divide(len);

	for (int i = 0; i < H; i++)
		hunits[i].Divide(len);

	for (int i = 0; i < N; i++)
		output_units[i].Divide(len);
}

void NARX::Test(int epo) {
	for (int i = 0; i < N; i++) {
		ee[i].Clear();
		rw[i].Clear();
	}

	Vector<Vector<double> > Y;
	Y.SetCount(N);
	for (int i = 0; i < N; i++)
		Y[i].SetCount(data->test_len, 0.0);

	for (int series_index = data->train_len; series_index < data->series_len ; series_index ++) {
		if (targets)
			for (int j = 0; j < N; j++)
				for (int i = 1; i <= b; i++) {
					if (series_index - i >= 0)
						inputs[j * b + i - 1].SetInput(data->Nseries[j][ series_index - i ]);
					else
						inputs[j * b + i - 1].SetInput(0);
				}

		//exogenous[0]->SetInput(series[series_index]);

		if (feedback) {
			for (int j = 0; j < N; j++) {
				for (int i = 0; i < b; i++) {
					if (series_index - data->train_len - j >= 0)
						feedbacks[j * b + i].SetInput(Y[j][series_index - data->train_len - j - 1]);
					else
						feedbacks[j * b + i].SetInput(0);
				}
			}
		}
		

		for (int i = 0; i < M; i++) {
			if (data->used_exogenous[i]) {
				for (int j = 0; j < a + 1; j++) {
					if (series_index - j >= 0)
						exogenous[j + i * a].SetInput(data->Nexogenous_series[i][series_index - j]);
					else
						exogenous[j + i * a].SetInput(0);
				}

				//_log(String("ok %1").arg(exogenous_series[i][series_index]));
				//FLOG(String("ok exo=%1\n").arg(exogenous_series[i][series_index]).toStdString().c_str());
			}
		}

		for (int i = 0; i < N; i++) {
			//output_units[i].SetTarget(Nseries[i][series_index]);
			ee[i].Insert(data->Nseries[i][series_index], output_units[i].GetOutput());
			rw[i].Insert(data->Nseries[i][series_index], data->Nseries[i][series_index > 0 ? series_index - 1 : 0]);
			Y[i][series_index - data->train_len] = output_units[i].GetOutput();
		}

		//FLOG(String("input target:index %1 : %2, output narx: %3\n").arg(series_index).arg(series[series_index]).arg(output_unit->GetOutput()).toStdString().c_str());
	}

	for (int i = 0; i < N; i++) {
		LOG(Format("target %1:test %2: F1 = %3; F2 = %4; F3 = %5; F4 = %6; KS1= %7; KS2=%8; KS12=%9; DA = %10"
					 "; F1RW=%11; F2RW=%12; F3RW=%13; F4RW=%14; KS1=%15; KS2=%16; KS12=%17; DA=%18",
			  i,
			  epo,
			  ee[i].F1(), ee[i].F2(), ee[i].F3(), ee[i].F4(),
			  ee[i].KS1(),
			  ee[i].KS2(), ee[i].KS12(), ee[i].DA(),
			  rw[i].F1(), rw[i].F2(), rw[i].F3(), rw[i].F4(),
			  rw[i].KS1(), rw[i].KS2(), rw[i].KS12(), rw[i].DA()));
	}
}
