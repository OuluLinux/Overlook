#include "NarxSim.h"



NarxSim::NarxSim() {
	Sizeable();
	Icon(Imgs::icon());
	Title("NARX Simulator");
	
	narx_stage1_5 = 0;
	narx_stage1_1 = 0;
	normalize = 1;
	arch = MLP;
	M = 0;
	old_M = 0;
	N = 1;
	series_generated = false;
	
	mynarx.SetData(*this);
	mynarx.WhenLog = THISBACK(PostNarxLog);
	mynarx.WhenTrainingFinished = THISBACK(TrainingFinished);
	
	Add(tabs.HSizePos().VSizePos(0,100));
	Add(logctrl.HSizePos().BottomPos(0,70));
	Add(prev.LeftPos(4,100).BottomPos(72, 26));
	Add(next.RightPos(4,100).BottomPos(72, 26));
	
	logctrl.AddColumn();
	logctrl.NoHeader();
	
	next.SetLabel("Next");
	next <<= THISBACK(Next);
	prev.SetLabel("Previous");
	prev <<= THISBACK(Previous);
	
	CtrlLayout(train);
	CtrlLayout(seriestab);
	CtrlLayout(archtab);
	CtrlLayout(params);
	CtrlLayout(training);
	CtrlLayout(predict);
	
	train.series_type.SetData(0);
	train.predefined.SetData(0);
	train.x_start.SetData(0);
	train.x_end.SetData(1);
	train.series_len.SetData(500);
	train.base_fn.Add("sin");
	train.base_fn.Add("log");
	train.base_fn.Add("exp");
	train.base_fn.Add("sqr");
	train.base_fn.Add("hip");
	train.base_fn.Add("lin");
	train.base_fn.SetIndex(0);
	train.noise_factor.SetData(5);
	train.normalize.Set(true);
	
	seriestab.spinbox_test_percentage.SetData(5);
	
	archtab.check_exogenous.Set(true);
	archtab.spinbox_hidden_units.SetData(3);
	
	params.spinbox_dregressor.SetData(0);
	params.spinbox_xregressor.SetData(2);
	params.learning_rate.SetData(0.2);
	params.epochs.SetData(100);
	params.combo_hunits_act.Add("Sigmoid");
	params.combo_hunits_act.Add("Beta=0.1 BSigmoid");
	params.combo_hunits_act.Add("antisymmetric log");
	params.combo_hunits_act.SetIndex(0);
	
	params.combo_ounits_act.Add("linear");
	params.combo_ounits_act.SetIndex(0);
	
	training.log.AddColumn("");
	training.log.NoHeader();
	
	predict.x <<= THISBACK(Predict);
	
	tabs.Add(train, "Train source");
	tabs.Add(train);
	
	
	
	/*
	QObject::connect ( w.ui.actionAbout, SIGNAL( triggered() ), &w, SLOT( MenuAbout() ) );
	QObject::connect ( w.ui.GenerateSeries, SIGNAL( clicked() ), &w, SLOT( GenerateSeries() ) );
	QObject::connect ( w.ui.LoadSeries, SIGNAL( clicked() ), &w, SLOT( LoadSeries() ) );
	QObject::connect ( w.ui.Button_tab12, SIGNAL( clicked() ), &w, SLOT( Next1() ) );
	QObject::connect ( w.ui.Button_tab21, SIGNAL( clicked() ), &w, SLOT( Prev2() ) );
	QObject::connect ( w.ui.Button_tab23, SIGNAL( clicked() ), &w, SLOT( Next2() ) );
	QObject::connect ( w.ui.Button_tab34, SIGNAL( clicked() ), &w, SLOT( Next3() ) );
	QObject::connect ( w.ui.Button_tab32, SIGNAL( clicked() ), &w, SLOT( Prev3() ) );
	QObject::connect ( w.ui.Button_tab43, SIGNAL( clicked() ), &w, SLOT( Prev4() ) );
	QObject::connect ( w.ui.Button_tab45, SIGNAL( clicked() ), &w, SLOT( Next4() ) );
	QObject::connect ( w.ui.Button_tab54, SIGNAL( clicked() ), &w, SLOT( Prev5() ) );
	QObject::connect ( w.ui.Button_tab56, SIGNAL( clicked() ), &w, SLOT( Next5() ) );
	QObject::connect ( w.ui.Button_start_training, SIGNAL( clicked() ), &w, SLOT( StartTrain() ) );
	QObject::connect ( w.ui.button_predict, SIGNAL( clicked() ), &w, SLOT( Predict() ) );
	QObject::connect ( w.ui.button_browse, SIGNAL( clicked() ), &w, SLOT( BrowseAction() ) );
	*/
	
}


void NarxSim::ProgressInc() {
	PostCallback(THISBACK2(SetProgress, mynarx.GetEpoch(), epochs));
}
void NarxSim::Log(String s) {
	logctrl.Add(s);
	logctrl.SetCursor(logctrl.GetCount()-1);
}

void NarxSim::PostNarxLog(String s) {
	PostCallback(THISBACK1(NarxLog, s));
}

void NarxSim::NarxLog(String s) {
	training.log.Add(s);
	training.log.SetCursor(training.log.GetCount()-1);
}

void NarxSim::MenuAbout() {
	PromptOK(DeQtf("Artificial Intelligence Master Project\neng. Eugen Hristev 2012.\n"
							 "Thanks to Dr. Rene Alquezar Mancho for coordinating my project." ));
}

void NarxSim::StartTrain() {
	NormalizeF();
	mynarx.Start();
}

void NarxSim::Next() {
	int tab = tabs.Get();
	switch (tab) {
		case 0: Next1(); break;
		case 1: Next2(); break;
		case 2: Next3(); break;
		case 3: Next4(); break;
		case 4: Next5(); break;
	}
}

void NarxSim::Previous() {
	int tab = tabs.Get();
	switch (tab) {
		case 1: Prev2(); break;
		case 2: Prev3(); break;
		case 3: Prev4(); break;
		case 4: Prev5(); break;
	}
}

void NarxSim::Next1() {
	if (tabs.GetCount() == 1) {
		tabs.Add(seriestab, "Series");
		tabs.Add(seriestab);
	}
	
	if (!series_generated) {

		N = 1;
		series_start = train.x_start.GetData();
		series_end = train.x_end.GetData();
		series_len = train.series_len.GetData();
		series_func  = train.base_fn.GetIndex();
		series_noise = train.noise_factor.GetData();
		
		series.SetCount(N);
		for (int i = 0; i < N; i++)
			series[i].SetCount(series_len);
	
		double step = ( series_end - series_start ) / series_len;
		double cur = series_start;
		double prev_cury = series_start;
		double prev2_cury = series_start;
		
		// hardcoded values for Y and Z variables STARTS HERE
		double cury = 3.0;
		double curz = 1.1;
		double stepy = 0.05;
		double stepz = -0.03;
		
		// end hardcoded values for Y and Z variables
		M = 1;
		
		int predef = train.predefined.GetData();
	
		if (predef == 0) {
			seriestab.table_series.AddColumn("X (input value - exogenous)");
			seriestab.table_series.AddColumn("Y (input value - exogenous)");
			seriestab.table_series.AddColumn("Z (input value - exogenous)");
			seriestab.table_series.AddColumn("D (series value)");
			seriestab.use_split << use_options.Add().Set(true).SetLabel("X");
			seriestab.use_split << use_options.Add().Set(true).SetLabel("Y");
			seriestab.use_split << use_options.Add().Set(true).SetLabel("Z");
			M = 3;
		}
		else if (predef == 1) {
			seriestab.table_series.AddColumn("X (input value - exogenous)");
			seriestab.table_series.AddColumn("Y (input value - exogenous)");
			seriestab.table_series.AddColumn("D (series value)");
			seriestab.use_split << use_options.Add().Set(true).SetLabel("X");
			seriestab.use_split << use_options.Add().Set(true).SetLabel("Y");
			seriestab.table_series.Set(0, 0, "Use in NARX");
			M = 2;
		}
		else {
			seriestab.table_series.AddColumn("X (input value - exogenous)");
			seriestab.table_series.AddColumn("D (series value)");
			seriestab.use_split << use_options.Add().Set(true).SetLabel("X");
			/*	 exogenous_series = new double*[1];
				exogenous_series[0]=new double[series_len];
				used_exogenous = new int[1];
				used_exogenous[0]=0;*/
		}
	
		exogenous_series.SetCount(M);
		used_exogenous.SetCount(M);
	
		for (int i = 0; i < M; i++) {
			used_exogenous[i] = 0;
			exogenous_series[i].SetCount(series_len);
		}
	
		int i;
	
		for (i = 0; i < series_len; i++) {
			//seriestab.table_series.insertRow(i );
			seriestab.table_series.Set(i, 0, cur);
			exogenous_series[0][i] = cur;
			
			double val;
	
			switch (series_func) {
			case 0:
				val = sin(cur);
				break;
	
			case 1:
				val = log(cur);
				break;
	
			case 2:
				val = exp(cur);
				break;
	
			case 3:
				val = cur * cur;
				break;
	
			case 4:
				val = 1 / cur;
				break;
	
			case 5:
				val = cur;
				break;
	
			default:
				val = 1.0;
			};
	
			if (predef == 0) {
				if (i >= 3)
					val = sin(cur + cury * series[0][ i - 1 ]) * curz  + tan(series [0][ i - 2 ] - series[0] [ i - 3 ]);
				else if (i == 2)
					val = sin(cur + cury * series[0][ i - 1 ]) * curz  + tan(series[0] [ i - 2 ]);
				else if (i == 1)
					val = sin(cur + cury * series[0][ i - 1 ]) * curz  ;
				else
					val = sin(cur + cury) * curz * curz ;
	
				exogenous_series[1][i] = cury;
				exogenous_series[2][i] = curz;
				M = 3;
			}
			else if (predef == 1) {
				if (i >= 1)
					val = sin(cur - prev2_cury) * log(prev_cury + 1) +  log(fabs(series[0][i - 1]) + 1) - cury * sin(cur - prev_cury) ;
				else
					val = sin(cur - prev2_cury) * log(prev_cury +  1) - cury * sin(cur - prev_cury) ;
	
				exogenous_series[1][i] = cury;
				//exogenous_series[2][i - 1] = curz;
				M = 2;
			}
	
			if (series_noise)
				val += val * ((rand() % (2 * series_noise) - series_noise)) / 100 ;
			
			if (predef == 0) {
				seriestab.table_series.Set(i, 1, cury);
				seriestab.table_series.Set(i, 2, curz);
				seriestab.table_series.Set(i, 3, val);
			}
			else if (predef == 1) {
				seriestab.table_series.Set(i, 1, cury);
				seriestab.table_series.Set(i, 2, val);
			}
			else
				seriestab.table_series.Set(i, 1, val);
	
			series[0][i] = val;
			prev2_cury = prev_cury;
			prev_cury = cury;
			cur += step;
	
			if (predef == 0) {
				cury += stepy;
				curz += stepz;
			}
	
			else if (predef == 1) {
				cury += stepy;
				curz += stepz;
			}
		}
	
		//PromptOK("The series has been generated.");
		Log("The series has been generated.");
		normalize = train.normalize.Get();
	}
	
	tabs.Set(1);
	train.normalize.Disable();
	series_generated = 1;
}

void NarxSim::Prev2() {
	tabs.Set(0);
}

void NarxSim::Next2() {
	if (tabs.GetCount() == 2) {
		tabs.Add(archtab, "NARX Architecture select");
		tabs.Add(archtab);
	}

	if (!narx_stage1_1) {
		for (int i = 0; i < M; i++) {
			#error todo
			//if (ui.table_series->item(0, i)->checkState() == Qt::Checked) {
				used_exogenous[i] = 1;
				Log("Loaded exogenous variable.");
			//}

			//ui.table_series->item(0, i)->setFlags(ui.table_series->item(0, i)->flags() & ~Qt::ItemIsEnabled);
		}

		narx_stage1_1 = 1;
	}
	
	seriestab.spinbox_test_percentage.Disable();
	test_len = series_len * (double)seriestab.spinbox_test_percentage.GetData() / 100.0;
	train_len = series_len - test_len;
	
	tabs.Set(2);
}

void NarxSim::Prev3() {
	tabs.Set(1);
}

void NarxSim::Next3() {
	
	if (!archtab.check_del_targets.Get() && !archtab.check_del_outputs.Get())
		params.spinbox_dregressor.Disable();

	//if (!archtab.check_del_outputs.Get())
	//	ui.spinbox_yregressor->setEnabled(false);
	if (!archtab.check_exogenous.Get())
		params.spinbox_xregressor.Disable();

	if (tabs.GetCount() == 3) {
		tabs.Add(params, "Parameter Selection");
		tabs.Add(params);
	}
	archtab.check_del_targets.Disable();
	archtab.check_exogenous.Disable();
	archtab.check_del_outputs.Disable();
	archtab.spinbox_hidden_units.Disable();
	
	tabs.Set(3);
}

void NarxSim::Prev4() {
	tabs.Set(2);
}

void NarxSim::Next4() {
	// main NARX code
	if (!narx_stage1_5) {
		Unit::alfa = params.learning_rate.GetData();
		epochs = params.epochs.GetData();
		Log(Format("epochs:%d, lrate=%n, H=%d", epochs, Unit::alfa, (int)archtab.spinbox_hidden_units.GetData()));
		
		training.progress.Set(0,epochs);
		
		if (!archtab.check_del_targets.Get() && !archtab.check_del_outputs.Get() && archtab.check_exogenous.Get()
			&& (int)params.spinbox_xregressor.GetData() == 0) {
			Log("Selected architecture: MLP");
			arch = MLP;
		}
		else if (archtab.check_del_targets.Get() && !archtab.check_del_outputs.Get() && !archtab.check_exogenous.Get()) {
			Log("Selected architecture: NAR-D");
			arch = NAR_D;
		}
		else if (!archtab.check_del_targets.Get() && !archtab.check_del_outputs.Get() && archtab.check_exogenous.Get()
				 && (int)params.spinbox_xregressor.GetData() > 0) {
			Log("Selected architecture: TDNN-X");
			arch = TDNN_X;
		}
		else if (archtab.check_del_targets.Get() && !archtab.check_del_outputs.Get() && archtab.check_exogenous.Get()
				 && 1) {
			Log("Selected architecture: NARX-D");
			arch = NARX_D;
		}
		else if (!archtab.check_del_targets.Get() && archtab.check_del_outputs.Get() && archtab.check_exogenous.Get()
				) {
			Log("Selected architecture: NARX-Y");
			arch = NARX_Y;
		}
		else if (archtab.check_del_targets.Get() && archtab.check_del_outputs.Get() && archtab.check_exogenous.Get()
				 && 1) {
			Log("Selected architecture: NARX-DY");
			arch = NARX_DY;
		}
		else if (!archtab.check_del_targets.Get() && archtab.check_del_outputs.Get() && !archtab.check_exogenous.Get()
				 && 1) {
			Log("Selected architecture: NAR-Y");
			arch = NAR_Y;
		}
		else if (archtab.check_del_targets.Get() && archtab.check_del_outputs.Get() && !archtab.check_exogenous.Get()
				 && 1) {
			Log("Selected architecture: NAR-DY");
			arch = NAR_DY;
		}
		else {
			Log( "Selected no architecture: training will probably fail.");
			arch = UNKNWN;
		}

		narx_stage1_5 = 1;
		old_M = M;

		if (!archtab.check_exogenous.Get()) // we dont want to use any exogenous
			M = 0;

		mynarx.Init(arch, (int)archtab.spinbox_hidden_units.GetData(),
						  /* now the act func for hiddens */ params.combo_hunits_act.GetIndex(),
						  (int)params.spinbox_xregressor.GetData(), (int)params.spinbox_dregressor.GetData(),
						  M,
						  N,// output units
						  archtab.check_del_outputs.Get(), //feedback
						  archtab.check_del_targets.Get()
						 );
	}

	/* end of main NARX code */
	mynarx.WhenTrainingEpochFinished = THISBACK(ProgressInc);
	
	if (tabs.GetCount() == 4) {
		tabs.Add(training, "Training");
		tabs.Add(training);
	}
	
	params.spinbox_xregressor.Disable();
	params.spinbox_dregressor.Disable();
	params.epochs.Disable();
	params.learning_rate.Disable();
	params.combo_hunits_act.Disable();
	params.combo_ounits_act.Disable();
	
	// Disable next button until training is finished
	next.Disable();
	StartTrain();
	
	tabs.Set(4);
}

void NarxSim::Prev5() {
	tabs.Set(3);
}

void NarxSim::Next5() {
	if (tabs.GetCount() == 5) {
		tabs.Add(predict, "Predict");
		tabs.Add(predict);
	}
	predict.x.MinMax(0, series_len-1);
	predict.x.SetData(series_len-1);
	
	Predict();
	
	for(int i = 0; i < old_M; i++) {
		
	}
	
	/*ui.scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	ui.scrollAreaWidgetContents->setGeometry(40, 60, 201, 60 + 30 * old_M);

	if (!used_exogenous[0] || !M) ui.lineEdit_exo1->setEnabled(false);

	for (int i = 1; i < old_M; i++) {
		QLabel* label_exo = new QLabel(ui.scrollAreaWidgetContents);
		label_exo->setObjectName(String("label_exio%1").arg(i + 1));
		label_exo->setGeometry(QRect(10, 10 + 30 * (i + 0), 21, 16));
		label_exo->setText(String("%1").arg(i + 1));
		label_exo->show();
		QLineEdit* lineEdit_exo = new QLineEdit(ui.scrollAreaWidgetContents);
		lineEdit_exo->setObjectName(String("lineedit_exio%1").arg(i + 1));
		lineEdit_exo->setGeometry(QRect(40, 10 + 30 * (i + 0), 113, 20));

		//ui.scrollArea->setWidget(ui.scrollAreaWidgetContents);
		if (!used_exogenous[i] || !M)
			lineEdit_exo->setEnabled(false);

		lineEdit_exo->show();
		//ui.scrollAreaWidgetContents->repaint();
	}*/
	
	tabs.Set(5);
}


/*void NarxSim::BrowseAction() {
	String fileName = QFileDialog::getOpenFileName(this,
												   tr("Open series"), ".");
	//Log(fileName);
	FILE* series_file = fopen(fileName.toStdString().c_str(), "rt");

	if (!series_file) {
		QMessageBox::information( this, "Error", "Cannot load series." );
		Log("Error:Cannot load series.");
		return;
	}

	fscanf(series_file, "%d", &series_len);
	fscanf(series_file, "%d", &M);
	fscanf(series_file, "%d", &N);
	Log(String("Loading %1 values from file.\nLoading %2 target series").arg(series_len).arg(N));
	series = new double*[N];

	for (int i = 0; i < N; i++)
		series[i] = new double[series_len];

	exogenous_series = new double*[M];
	used_exogenous = new int[M];

	for (int i = 0; i < M; i++) {
		used_exogenous[i] = 0;
		exogenous_series[i] = new double[series_len];
	}

	for (int i = 0; i < M; i++) {
		if (i) {
			//add a new column for the exogenous variable
			ui.table_series->insertColumn(i);
			QTableWidgetItem* col1 = new QTableWidgetItem();
			col1->setText(String("%1 (input value - exogenous)").arg(i));
			ui.table_series->setHorizontalHeaderItem(i, col1);
			QTableWidgetItem* exo1 = new QTableWidgetItem();
			ui.table_series->setItem(0, i, exo1);
			exo1->setText("Use in NARX");
			exo1->setCheckState(Qt::Checked);
		}

		double aux;

		for (int j = 0; j < series_len; j++) {
			if (!i) ui.table_series->insertRow(j + 1 );

			fscanf(series_file, "%lf", &aux);
			exogenous_series[i][j] = aux;
			QTableWidgetItem* newItemY = new QTableWidgetItem(tr("%1").arg(aux, 10));
			ui. table_series->setItem(j + 1, i, newItemY);
		}
	}

	double aux;

	for (int i = 0; i < N; i++)
		for (int j = 0; j < series_len; j++) {
			//if(!i) ui.table_series->insertRow(j +1 );
			if (i) {
				ui.table_series->insertColumn(M + i);
				QTableWidgetItem* col1 = new QTableWidgetItem();
				col1->setText(String("D%1 (target values)").arg(i));
				ui.table_series->setHorizontalHeaderItem(M + i, col1);
			}

			fscanf(series_file, "%lf", &aux);
			series[i][j] = aux;
			QTableWidgetItem* newItemY = new QTableWidgetItem(tr("%1").arg(aux, 10));
			ui. table_series->setItem(j + 1, M + i, newItemY);
		}

	series_generated = 1;
	fclose(series_file);
	QMessageBox::information( this, "Proceed", "Series loaded successfully." );
}*/

void NarxSim::Predict() {
	int pos = max(0, min(series_len-1, (int)predict.x.GetData()));
	Vector<double> out;
	
	mynarx.Predict(pos, out);
	
	predict.vars.Reset();
	predict.vars.AddColumn("Pos");
	predict.vars.AddColumn("Real");
	predict.vars.AddColumn("Predicted");
	if (normalize) {
		predict.vars.AddColumn("Real normalized");
		predict.vars.AddColumn("Predicted normalized");
	}
	
	for(int i = 0; i < out.GetCount(); i++) {
		double normalized_out = out[i];
		
		if (normalize) {
			// de-normalize value
			double out = normalized_out * Nvariance[i] + N_E[i];
			predict.vars.Add(i, series[i][pos], out, Nseries[i][pos], normalized_out);
		} else {
			predict.vars.Add(i, series[i][pos], normalized_out);
		}
	}
}








void NarxSim::NormalizeF() {
	if (!normalize) {
		Nseries <<= series;
		Nexogenous_series <<= exogenous_series;
		return;
	}

	N_E.SetCount(N, 0.0);
	Nvariance.SetCount(N, 0.0);
	N_exo_E.SetCount(M, 0.0);
	N_exo_variance.SetCount(M, 0.0);

	for (int i = 0; i < N; i++) {
		N_E[i] = 0;
		Nvariance[i] = 1;
	}

	for (int i = 0; i < M; i++) {
		N_exo_E[i] = 0;
		N_exo_variance[i] = 1;
	}

	for (int j = 0; j < M; j++) {
		for (int i = 0; i < train_len; i++)
			N_exo_E[j] += exogenous_series[j][i];
		N_exo_E[j] /= train_len;
	}

	for (int j = 0; j < N; j++) {
		for (int i = 0; i < train_len; i++)
			N_E[j] += series[j][i];
		N_E[j] /= train_len;
	}

	for (int j = 0; j < M; j++) {
		if (used_exogenous[j]) {
			for (int i = 0; i < train_len; i++)
				N_exo_variance[j] += pow(exogenous_series[j][i] - N_exo_E[j], 2);
			N_exo_variance[j] /= train_len;
			Log(Format("Normalized exogenous series %d, E=%n, variance = %n", j, N_exo_E[j], N_exo_variance[j]));
		}
	}

	for (int j = 0; j < N; j++) {
		for (int i = 0; i < train_len; i++)
			Nvariance[j] += pow(series[j][i] - N_E[j], 2);
		Nvariance[j] /= train_len;
		Log(Format("Normalized target series %d, E=%n, variance = %n", j, N_E[j], Nvariance[j]));
	}

	Nseries.SetCount(N);
	for (int i = 0; i < N; i++)
		Nseries[i].SetCount(series_len, 0.0);

	Nexogenous_series.SetCount(M);
	for (int i = 0; i < M; i++)
		Nexogenous_series[i].SetCount(series_len, 0.0);

	for (int j = 0; j < N; j++) {
		for (int i = 0; i < series_len; i++) {
			Nseries[j][i] = (series[j][i] - N_E[j]) / Nvariance[j];
			//FLog(String("norm series:%1\n").arg(Nseries[j][i]).toStdString().c_str());
		}
	}

	for (int j = 0; j < M; j++) {
		for (int i = 0; i < series_len; i++) {
			Nexogenous_series[j][i] = (exogenous_series[j][i] - N_exo_E[j]) / N_exo_variance[j];
			//FLog(String("norm series:%1\n").arg(Nseries[j][i]).toStdString().c_str());
		}
	}
}


