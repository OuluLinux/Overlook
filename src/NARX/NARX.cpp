#include "NARX.h"

namespace Narx {

NARX::NARX() {
	
}


void NARX::Init(ARCH arch, int H, int hact, int a, int b, int input_count, int output_count, int feedback, int targets) {
	ASSERT(data);
	this->H = H;
	this->a = a;
	this->b = b;
	this->arch = arch;
	this->input_count = input_count;
	this->output_count = output_count;
	this->feedback = feedback;
	this->hact = hact;
	this->targets = targets;
	ee.SetCount(output_count);
	rw.SetCount(output_count);
	
	// this was fatal error in the original...
	// the last exogenous position was never visited :S
	// ...author must gone crazy because of it xD
	a1 = a + 1;
	
	epoch = 0;
	
	for (int i = 0; i < output_count; i++) {
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
		inputs.SetCount(output_count);

		for (int i = 0; i < output_count; i++) {
			inputs[i].SetCount(b);
			for(int j = 0; j < b; j++) {
				inputs[i][j].SetInput(0);
			}
		}

		for (int i = 0; i < H; i++) {
			for (int j = 0; j < output_count; j++) {
				for(int k = 0; k < b; k++) {
					hunits[i].AddInputUnit(inputs[j][k]);
				}
			}
		}
	}
	

	if (feedback) {
		feedbacks.SetCount(output_count);

		for (int i = 0; i < output_count; i++) {
			feedbacks[i].SetCount(b);
			for(int j = 0; j < b; j++) {
				feedbacks[i][j].SetInput(0);
			}
		}

		for (int i = 0; i < H; i++) {
			for (int j = 0; j < output_count; j++) {
				for(int k = 0; k < b; k++) {
					hunits[i].AddInputUnit(feedbacks[j][k]);
				}
			}
		}
	}

	exogenous.SetCount(input_count);
	for (int i = 0; i < input_count; i++) {
		exogenous[i].SetCount(a1);
		for(int j = 0; j < a1; j++) {
			exogenous[i][j].SetInput(0);
		}
	}

	for (int i = 0; i < H; i++) {
		for (int j = 0; j < input_count; j++) {
			for(int k = 0; k < a1; k++) {
				hunits[i].AddInputUnit(exogenous[j][k]);
			}
		}
	}

	//printf("%d\n", index);
	output_units.SetCount(output_count);

	for (int i = 0; i < output_count; i++) {
		output_units[i].set_activation_func(ActivationFunctions::Identity);
		output_units[i].set_activation_func_derv(ActivationFunctions::IdentityDerv);
	}

	for (int i = 0; i < H; i++)
		for (int j = 0; j < output_count; j++)
			output_units[j].AddInputUnit(hunits[i]);

	//hunits[0]->
	WhenLog(Format("NARX: arch=%d, H=%d, hact =%d, a=%d, b=%d, M=%d, N=%d, feedback=%d, targets=%d",
		arch, H, hact, a, b, input_count, output_count, feedback, targets));
}

ARCH NARX::GetArch() {
	return arch;
}

void NARX::Train(int epochs) {
	for (epoch = 0; epoch < epochs; epoch++) {
		TrainEpoch(false, epoch);
		WhenTrainingEpochFinished();
	}

	//TrainEpoch(false, epochs);
	WhenTrainingFinished();
}

void NARX::TrainEpoch(bool logging, int epo) {
	//	int ;
	for (int i = 0; i < output_count; i++) {
		ee[i].Clear();
		rw[i].Clear();
	}

	if (!feedback) {
		int feedback_index = 0;

		for (int series_index = a; series_index < data->train_len ; series_index ++) {
			if (targets) {
				for (int j = 0; j < output_count; j++) {
					for (int i = 0; i < b; i++) {
						if (series_index - i - 1 >= 0 ) {
							inputs[j][i].SetInput(data->Nseries[j][ series_index - i - 1 ]);
							//FWhenLog(String("setting input:%1\n").arg(Nseries[j][ series_index - i  ]).toStdString().c_str());
						}
						else
							inputs[j][i].SetInput(0);
					}
				}
			}
			
			for (int i = 0; i < input_count; i++) {
				if (data->used_exogenous[i]) {
					for (int j = 0; j < a1; j++) {
						if (series_index - j >= 0)
							exogenous[i][j].SetInput(data->Nexogenous_series[i][series_index - j]);
						else
							exogenous[i][j].SetInput(0);
					}
					//_log(String("ok %1").arg(exogenous_series[i][series_index]));
					//FWhenLog(String("ok exo=%1\n").arg(exogenous_series[i][series_index]).toStdString().c_str());
				}
			}

			for (int i = 0; i < output_count; i++) {
				output_units[i].SetTarget(data->Nseries[i][series_index]);
				ee[i].Insert(data->Nseries[i][series_index], output_units[i].GetOutput());
				rw[i].Insert(data->Nseries[i][series_index], data->Nseries[i][series_index > 0 ? series_index - 1 : 0]);
			}

			//FWhenLog(String("input target:index %1 : %2, output narx: %3\n").arg(series_index).arg(series[series_index]).arg(output_unit->GetOutput()).toStdString().c_str());
			//output_unit
			//a += ;
			for (int i = 0; i < output_count; i++) {
				output_units[i].FixWeights();
				output_units[i].ComputeDelta();
				output_units[i].AdjustWeights();
			}

			//FWhenLog(String("ok delta=%1\n").arg(delta).toStdString().c_str());
			for (int i = 0; i < H; i++) {
				double delta = 0;

				for (int j = 0; j < output_count; j++)
					delta += output_units[j].GetDelta(hunits[i]);

				hunits[i].ComputeDelta(delta);
				hunits[i].AdjustWeights();
			}
		}
	}
	else {
		NARX copynarx;
		NARX originalnarx;
		copynarx.SetData(*(NarxData*)this);
		copynarx.Init(arch, H, hact, a, b, input_count, output_count, feedback, targets); // tk targets
		originalnarx.SetData(*(NarxData*)this);
		originalnarx.Init(arch, H, hact, a, b, input_count, output_count, feedback, targets); // tk targets
		originalnarx.Copy(*this);
		
		/* must do the feedback arch now */
		/*	 int weight_count = output_units[0]->GetInputCount();
			double *final_weights= new double[weight_count];
			for(int i=0;i<weight_count;i++)
				final_weights[i] = 0;
		*/
		Vector<Vector<double> > Y;
		Y.SetCount(output_count);
		for (int i = 0; i < output_count; i++)
			Y[i].SetCount(data->train_len, 0);
		
		Vector<FeedbackInfo> fi;
		fi.SetCount(data->train_len);
		for (int i = 0; i < data->train_len; i++)
			fi[i].Init(input_count, a1, output_count, b, output_count, b);
		
		int t = 0;
		
		for (; t < data->train_len; t++) {
			// if (exogenous)
			for (int i = 0; i < input_count; i++) {
				if (data->used_exogenous[i])
					for (int j = a; j >= 0 ; j--) {
						if (t >= j)
							exogenous[i][j].SetInput(data->Nexogenous_series[i][t - j]);
						else
							exogenous[i][j].SetInput(0);

						fi[t].X[i][j] = exogenous[i][j].GetInput();
					}
			}

			if (targets) {
				for (int i = 0; i < output_count; i++) {
					for (int j = b; j > 0; j--) {
						if (t >= j)
							inputs[i][j - 1].SetInput(data->Nseries[i][t - j]);
						else
							inputs[i][j - 1].SetInput(0);

						fi[t].D[i][j - 1] = inputs[i][j - 1].GetInput();
					}
				}
			}

			if (feedback) {
				for (int i = 0; i < output_count; i++) {
					for (int j = b; j > 0; j--) {
						//FWhenLog(String("testing%1\n").arg(fi[t].Y[i*N+j]).toStdString().c_str());
						if (t >= j)
							feedbacks[i][j - 1].SetInput(Y[i][t - j]);
						else
							feedbacks[i][j - 1].SetInput(0);

						//FWhenLog(String("testing%1\n").arg(fi[t].Y[i*N+j]).toStdString().c_str());
						fi[t].Y[i][j - 1] = feedbacks[i][j - 1].GetInput();
					}
				}
			}

			for (int i = 0; i < output_count; i++) {
				output_units[i].SetTarget(data->Nseries[i][t]);
				Y[i][t] = output_units[i].GetOutput();
				//FWhenLog(String("testing%1\n").arg(Y[i][t]).toStdString().c_str());
				ee[i].Insert(data->Nseries[i][t], Y[i][t]);
				rw[i].Insert(data->Nseries[i][t], data->Nseries[i][t > 0 ? t - 1 : 0]);
			}
		}
		
		Vector<Vector<double> > previous_deltas;
		previous_deltas.SetCount(data->train_len);
		for (int i = 0; i < data->train_len; i++) {
			previous_deltas[i].SetCount(output_count, 0);
		}

		// backpropagating the errors
		for (t = data->train_len - 1; t >= 0; t--) {
			NARX::Copy(originalnarx);

			/* if (exogenous) */
			for (int i = 0; i < input_count; i++) {
				for (int j = a; j >= 0 ; j--)
					exogenous[i][j].SetInput(fi[t].X[i][j]);
			}

			if (targets) {
				for (int i = 0; i < output_count; i++) {
					for (int j = b; j > 0; j--)
						inputs[i][j - 1].SetInput(fi[t].D[i][j - 1]);
				}
			}

			if (feedback) {
				for (int i = 0; i < output_count; i++) {
					for (int j = b; j > 0; j--)
						feedbacks[i][j - 1].SetInput(fi[t].Y[i][j - 1]);
				}
			}

			for (int i = 0; i < output_count; i++) {
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

				for (int j = 0; j < output_count; j++)
					delta += output_units[j].GetDelta(hunits[i]);

				hunits[i].ComputeDelta(delta);
				hunits[i].FixWeights();
				hunits[i].AdjustWeights();
			}

			for (int k = 0; k < output_count; k++)
				for (int i = 0; i < b; i++) {
					double delta = 0;

					for (int j = 0; j < H; j++)
						delta += hunits[j].GetDelta(feedbacks[k][i]);

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
		WhenLog("Epoch finished :)");

	for (int i = 0; i < output_count; i++) {
		WhenLog(Format("target %d:epoch %d: F1 = %n; F2 = %n; F3 = %n; F4 = %n; KS1= %n; KS2=%n; KS12=%n; DA = %n"
					 "; F1RW=%n; F2RW=%n; F3RW=%n; F4RW=%n; KS1=%n; KS2=%n; KS12=%n; DA=%n",
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
	for (int i = 0; i < input_count; i++) {
		for(int j = 0; j < a1; j++) {
			exogenous[i][j].Copy(n.exogenous[i][j]);
		}
	}
	
	if (targets) {
		for (int i = 0; i < output_count; i++) {
			for(int j = 0; j < b; j++) {
				inputs[i][j].Copy(n.inputs[i][j]);
			}
		}
	}
	
	if (feedback) {
		for (int i = 0; i < output_count; i++) {
			for(int j = 0; j < b; j++) {
				feedbacks[i][j].Copy(n.feedbacks[i][j]);
			}
		}
	}
	
	for (int i = 0; i < H; i++)
		hunits[i].Copy(n.hunits[i]);

	for (int i = 0; i < output_count; i++)
		output_units[i].Copy(n.output_units[i]);
}

void NARX::Sum(NARX& n) {
	for (int i = 0; i < input_count; i++) {
		for(int j = 0; j < a1; j++) {
			exogenous[i][j].Sum(n.exogenous[i][j]);
		}
	}
	
	if (targets) {
		for (int i = 0; i < output_count; i++) {
			for(int j = 0; j < b; j++) {
				inputs[i][j].Sum(n.inputs[i][j]);
			}
		}
	}
	

	if (feedback) {
		for (int i = 0; i < output_count; i++) {
			for(int j = 0; j < b; j++) {
				feedbacks[i][j].Sum(n.feedbacks[i][j]);
			}
		}
	}
	

	for (int i = 0; i < H; i++)
		hunits[i].Sum(n.hunits[i]);

	for (int i = 0; i < output_count; i++)
		output_units[i].Sum(n.output_units[i]);
}

void NARX::Divide(int len) {
	for (int i = 0; i < input_count; i++) {
		for(int j = 0; j < a1; j++) {
			exogenous[i][j].Divide(len);
		}
	}
	

	if (targets) {
		for (int i = 0; i < output_count; i++) {
			for(int j = 0; j < b; j++) {
				inputs[i][j].Divide(len);
			}
		}
	}

	if (feedback) {
		for (int i = 0; i < output_count; i++) {
			for(int j = 0; j < b; j++) {
				feedbacks[i][j].Divide(len);
			}
		}
	}
	

	for (int i = 0; i < H; i++)
		hunits[i].Divide(len);

	for (int i = 0; i < output_count; i++)
		output_units[i].Divide(len);
}

void NARX::Test(int epo) {
	for (int i = 0; i < output_count; i++) {
		ee[i].Clear();
		rw[i].Clear();
	}

	Vector<Vector<double> > Y;
	Y.SetCount(output_count);
	for (int i = 0; i < output_count; i++)
		Y[i].SetCount(data->test_len, 0.0);

	for (int series_index = data->train_len; series_index < data->series_len ; series_index ++) {
		if (targets) {
			for (int j = 0; j < output_count; j++) {
				for (int i = 1; i <= b; i++) {
					if (series_index - i >= 0)
						inputs[j][i - 1].SetInput(data->Nseries[j][ series_index - i ]);
					else
						inputs[j][i - 1].SetInput(0);
				}
			}
		}
		
		if (feedback) {
			for (int j = 0; j < output_count; j++) {
				for (int i = 0; i < b; i++) {
					if (series_index - data->train_len - i - 1 >= 0)
						feedbacks[j][i].SetInput(Y[j][series_index - data->train_len - i - 1]);
					else
						feedbacks[j][i].SetInput(0);
				}
			}
		}
		

		for (int i = 0; i < input_count; i++) {
			if (data->used_exogenous[i]) {
				for (int j = 0; j < a1; j++) {
					if (series_index - j >= 0)
						exogenous[i][j].SetInput(data->Nexogenous_series[i][series_index - j]);
					else
						exogenous[i][j].SetInput(0);
				}

				//_log(String("ok %1").arg(exogenous_series[i][series_index]));
				//FWhenLog(String("ok exo=%1\n").arg(exogenous_series[i][series_index]).toStdString().c_str());
			}
		}

		for (int i = 0; i < output_count; i++) {
			//output_units[i].SetTarget(Nseries[i][series_index]);
			double in = data->Nseries[i][series_index];
			double out = output_units[i].GetOutput();
			ee[i].Insert(in, out);
			rw[i].Insert(in, data->Nseries[i][series_index > 0 ? series_index - 1 : 0]);
			Y[i][series_index - data->train_len] = out;
		}

		//FWhenLog(String("input target:index %1 : %2, output narx: %3\n").arg(series_index).arg(series[series_index]).arg(output_unit->GetOutput()).toStdString().c_str());
	}

	for (int i = 0; i < output_count; i++) {
		WhenLog(Format("target %d:test %d: F1 = %n; F2 = %n; F3 = %n; F4 = %n; KS1= %n; KS2=%n; KS12=%n; DA = %n"
					 "; F1RW=%n; F2RW=%n; F3RW=%n; F4RW=%n; KS1=%n; KS2=%n; KS12=%n; DA=%n",
			  i,
			  epo,
			  ee[i].F1(), ee[i].F2(), ee[i].F3(), ee[i].F4(),
			  ee[i].KS1(),
			  ee[i].KS2(), ee[i].KS12(), ee[i].DA(),
			  rw[i].F1(), rw[i].F2(), rw[i].F3(), rw[i].F4(),
			  rw[i].KS1(), rw[i].KS2(), rw[i].KS12(), rw[i].DA()));
	}
}

void NARX::Predict(int series_index_, Vector<double>& out) {
	out.SetCount(output_count);
	
	Vector<Vector<double> > Y;
	Y.SetCount(output_count);
	for (int i = 0; i < output_count; i++)
		Y[i].SetCount(data->series_len, 0.0);
	
	// feedback requires buffer from beginning
	int begin = feedback ? 0 : series_index_;
	
	for (int series_index = begin; series_index <= series_index_ ; series_index ++) {
		if (targets) {
			for (int j = 0; j < output_count; j++) {
				for (int i = 1; i <= b; i++) {
					if (series_index - i >= 0)
						inputs[j][i - 1].SetInput(data->Nseries[j][ series_index - i ]);
					else
						inputs[j][i - 1].SetInput(0);
				}
			}
		}
		
		if (feedback) {
			for (int j = 0; j < output_count; j++) {
				for (int i = 0; i < b; i++) {
					if (series_index - data->train_len - i - 1 >= 0)
						feedbacks[j][i].SetInput(Y[j][series_index - i - 1]);
					else
						feedbacks[j][i].SetInput(0);
				}
			}
		}
		
	
		for (int i = 0; i < input_count; i++) {
			if (data->used_exogenous[i]) {
				for (int j = 0; j < a1; j++) {
					if (series_index - j >= 0)
						exogenous[i][j].SetInput(data->Nexogenous_series[i][series_index - j]);
					else
						exogenous[i][j].SetInput(0);
				}
	
				//_log(String("ok %1").arg(exogenous_series[i][series_index]));
				//FWhenLog(String("ok exo=%1\n").arg(exogenous_series[i][series_index]).toStdString().c_str());
			}
		}
		
		for (int i = 0; i < output_count; i++) {
			double out = output_units[i].GetOutput();
			Y[i][series_index] = out;
		}
	}
	
	for (int i = 0; i < output_count; i++) {
		out[i] = output_units[i].GetOutput();
	}
}

}
