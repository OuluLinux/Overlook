#include "NarxSim.h"




















#if 0
#include "narx2.h"
#include "Unit.h"
#include "InputUnit.h"
#include "OutputUnit.h"
#include "NARX.h"
#include <QtGui/QMessageBox>
#include <QtCore/qmath.h>
#include <QFileDialog>
#include <QInputDialog>
#include "narx_util.h"

extern double series_start, series_end;
extern int series_len;
extern int train_len, test_len;
extern int series_func;
extern int series_noise;
extern int series_generated;
extern double **series;
extern double **exogenous_series;
extern int *used_exogenous;
extern int epochs;
extern void train_progress_inc();

extern int M;

int N;

extern int normalize;

int old_M=0;

extern NARX *mynarx;

int narx_stage1_5 = 0;
int narx_stage1_1 = 0;
//initialize NARX:
ARCH arch = MLP;

NARX2::NARX2(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	
	ui.Combo_base_function->addItem( "sin");
	ui.Combo_base_function->addItem( "log");
	ui.Combo_base_function->addItem( "exp");
	ui.Combo_base_function->addItem( "sqr");
	ui.Combo_base_function->addItem( "hip");
	ui.Combo_base_function->addItem( "lin");

	ui.combo_hunits_act->addItem("sigmoid");
	ui.combo_hunits_act->addItem("Beta=0.1 Bsigmoid");
	ui.combo_hunits_act->addItem("antisymmetric log");
	//ui.combo_hunits_act->setCurrentIndex(2);

	ui.combo_ounits_act->addItem("linear");

}

NARX2::~NARX2()
{

}

void NARX2::train_progress_inc()
{
	::train_progress_inc();
}
void NARX2::log()
{
	extern QString narx_log_str;
	//LOG(narx_log_str);
	train_result_log(narx_log_str);
}


void NARX2::Menu_about() 
{
    QMessageBox::information(this, "About", "Artificial Intelligence Master Project\neng. Eugen Hristev 2012.\n"
		"Thanks to Dr. Rene Alquezar Mancho for coordinating my project." );
}

void NARX2::Button_start_train()
{
	normalize_f();
	mynarx->start();
	ui.Button_start_training->setEnabled(false);
}

void NARX2::RadioButton_generate_series() 
{
//QMessageBox::information( this, "Information", "Just clicked Ui PushButton" ); // #include <QtGui/QMessageBox>
ui.Frame_generate->setHidden(false);
ui.frame_load->setHidden(true);
}

void NARX2::RadioButton_load_series() 
{
//QMessageBox::information( this, "Information", "Just clicked LOAD" ); // #include <QtGui/QMessageBox>
ui.Frame_generate->setHidden(true);
ui.frame_load->setHidden(false);
}

void NARX2::Button_12() 
{
	

	ui.frame1->setEnabled(false);
	ui.Frame_generate->setEnabled(false);
	ui.frame_1post->setHidden(false);

	if(series_generated) goto exit1;

	N = 1;

	series_start = ui.lineedit_series_start->text().toDouble();
	series_end = ui.lineedit_series_end->text().toDouble();
	series_len = ui.spinbox_series_len->value();
    series_func  = ui.Combo_base_function->currentIndex();
	series_noise = ui.spinbox_noise->value();

	//switch(series_f)
	series = new double*[N];
	for(int i=0;i<N;i++)
		series[i] =  new double[series_len];

	double step = ( series_end - series_start ) / series_len;

	double cur = series_start;
	double prev_cury = series_start;
	double prev2_cury = series_start;
	/* hardcoded values for Y and Z variables STARTS HERE  */
	double cury = 3.0; 
	double curz = 1.1;
	double stepy = 0.05;
	double stepz = -0.03;
	/* end hardcoded values for Y and Z variables */

	M = 1;
	

	if(ui.predefined1->isChecked())
	{
		ui.table_series->insertColumn(1);
		QTableWidgetItem * col1= new QTableWidgetItem();
		col1->setText("Y (input value - exogenous)");
		ui.table_series->setHorizontalHeaderItem(1, col1);
		QTableWidgetItem * exo1= new QTableWidgetItem();
		ui.table_series->setItem(0, 1, exo1);
		exo1->setText("Use in NARX");
		exo1->setCheckState(Qt::Checked);

		ui.table_series->insertColumn(2);
		QTableWidgetItem * col2= new QTableWidgetItem();
		col2->setText("Z (input value - exogenous)");
		ui.table_series->setHorizontalHeaderItem(2, col2);
		QTableWidgetItem * exo2= new QTableWidgetItem();
		ui.table_series->setItem(0, 2, exo2);
		exo2->setText("Use in NARX");
		exo2->setCheckState(Qt::Checked);
		

		M=3;
		
		//goto pre_exit;
	}
	else if(ui.predefined2->isChecked())
	{
		ui.table_series->insertColumn(1);
		QTableWidgetItem * col1= new QTableWidgetItem();
		col1->setText("Y (input value - exogenous)");
		ui.table_series->setHorizontalHeaderItem(1, col1);
		QTableWidgetItem * exo1= new QTableWidgetItem();
		ui.table_series->setItem(0, 1, exo1);
		exo1->setText("Use in NARX");
		exo1->setCheckState(Qt::Checked);

		
		

		M=2;
		
		//goto pre_exit;
	}
	else 
	{
		/*	exogenous_series = new double*[1];
			exogenous_series[0]=new double[series_len];
			used_exogenous = new int[1];
			used_exogenous[0]=0;*/
	}

		exogenous_series = new double*[M];
		used_exogenous = new int[M];
		for(int i=0;i<M;i++)
		{
			used_exogenous[i]= 0;
			exogenous_series[i]=new double[series_len];
		}

	int i;
	
	
	

	for(i=1; i <= series_len; i++)
	{
		
		ui.table_series->insertRow(i );
		QTableWidgetItem *newItem1 = new QTableWidgetItem(tr("%1").arg(cur));
		ui. table_series->setItem(i , 0, newItem1);

		exogenous_series[0][i - 1] = cur;

		double val;

		switch (series_func)
		{
		case 0:
			val = qSin(cur);
			break;
		case 1:
			val = qLn(cur);
			break;
		case 2:
			val = qExp(cur);
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

		
	if(ui.predefined1->isChecked())
	{
		if(i>=4)
		val = qSin(cur + cury* series[0][ i - 2 ])* curz  + qTan(series [0][ i - 3 ] - series[0] [ i - 4 ]);
		else if(i == 3)
			val = qSin(cur + cury* series[0][ i - 2 ])* curz  + qTan(series[0] [ i - 3 ]);
		else if(i==2)
			val = qSin(cur + cury* series[0][ i - 2 ])* curz  ;
		else 
			val = qSin(cur + cury)* curz * curz ;

		exogenous_series[1][i - 1] = cury;
		exogenous_series[2][i - 1] = curz;
		M=3;
	
		
	}
	else if(ui.predefined2->isChecked())
	{
		if(i>=2)
		val = qSin(cur - prev2_cury)* qLn(prev_cury + 1) +  qLn(qAbs(series[0][i - 2]) + 1) - cury*qSin(cur - prev_cury) ;
		
		else 
			val = qSin(cur - prev2_cury)* qLn(prev_cury +  1) - cury*qSin(cur - prev_cury) ;

		exogenous_series[1][i - 1] = cury;
		//exogenous_series[2][i - 1] = curz;
		M=2;
	}

		if(series_noise)
			val += val * ((rand() % (2*series_noise) - series_noise)) / 100 ;


		QTableWidgetItem *newItem2 = new QTableWidgetItem(tr("%1").arg(val));
		
		
		if(ui.predefined1->isChecked())
		{
			QTableWidgetItem *newItemY = new QTableWidgetItem(tr("%1").arg(cury));
			ui. table_series->setItem(i, 1, newItemY);
			QTableWidgetItem *newItemZ = new QTableWidgetItem(tr("%1").arg(curz));
			ui. table_series->setItem(i, 2, newItemZ);

			ui. table_series->setItem(i, 3, newItem2);
		}
		else if(ui.predefined2->isChecked())
		{
			QTableWidgetItem *newItemY = new QTableWidgetItem(tr("%1").arg(cury));
			ui. table_series->setItem(i, 1, newItemY);
			

			ui. table_series->setItem(i, 2, newItem2);
		}
		else
			ui. table_series->setItem(i, 1, newItem2);

		series [0][i - 1] = val;
		prev2_cury = prev_cury;
		prev_cury = cury;
		cur+=step;

		if(ui.predefined1->isChecked())
		{
			cury+=stepy;
			curz+=stepz;
		}
		if(ui.predefined2->isChecked())
		{
			cury+=stepy;
			curz+=stepz;
		}
	}
	//ui.table_series->removeRow(i);
pre_exit:

	QMessageBox::information( this, "Proceed", "The series has been generated." );
	LOG("The series has been generated.");

	normalize = ui.checkBox_normalize->isChecked();

	
exit1:
	//if(series_generated)
	ui.tabWidget->setCurrentIndex(1);
    ui.tabWidget->setTabEnabled(1, true);
	ui.checkBox_normalize->setEnabled(false);
	series_generated = 1;
    //ui.tabWidget->setTabEnabled(0, false);

	

}

void NARX2::Button_21() 
{
	//
	ui.tabWidget->setCurrentIndex(0);
}

void NARX2::Button_23()
{
	ui.tabWidget->setTabEnabled(2, true);
	ui.tabWidget->setCurrentIndex(2);

	if(!narx_stage1_1) {

	for(int i=0;i<M;i++)
	{
	if(ui.table_series->item(0,i)->checkState() == Qt::Checked) {
		used_exogenous[i] = 1;
		LOG("Loaded exogenous variable.");
	}
	ui.table_series->item(0,i)->setFlags(ui.table_series->item(0,i)->flags() & ~Qt::ItemIsEnabled);
	}
	narx_stage1_1 = 1;
	}
	ui.spinbox_test_percentage->setEnabled(false);
	test_len = series_len * ui.spinbox_test_percentage->value() / 100;
	train_len = series_len - test_len;
}

void NARX2::Button_32()
{
	ui.tabWidget->setTabEnabled(1, true);
	ui.tabWidget->setCurrentIndex(1);
}

void NARX2::Button_34()
{
	

	//
	

	

	

	if (!ui.check_del_targets->isChecked() && !ui.check_del_outputs->isChecked())
		ui.spinbox_dregressor->setEnabled(false);
	//if (!ui.check_del_outputs->isChecked())
	//	ui.spinbox_yregressor->setEnabled(false);
	if (!ui.check_exogenous->isChecked())
		ui.spinbox_xregressor->setEnabled(false);

	ui.tabWidget->setTabEnabled(3, true);
	ui.tabWidget->setCurrentIndex(3);
	ui.check_del_targets->setEnabled(false);
	
	ui.check_exogenous->setEnabled(false);
	ui.check_del_outputs->setEnabled(false);
	ui.spinbox_hidden_units->setEnabled(false);
}
void NARX2::Button_43()
{
	ui.tabWidget->setTabEnabled(2, true);
	ui.tabWidget->setCurrentIndex(2);
}
void NARX2::Button_45()
{

	/* main NARX code */
	if(!narx_stage1_5)
	{
		Unit::alfa = ui.lineedit_learningrate->text().toDouble();
		epochs = ui.lineedit_epochs->text().toDouble();
		LOG(QString("epochs:%1, lrate=%2, H=%3").arg(epochs).arg(Unit::alfa).arg(ui.spinbox_hidden_units->value()));
		ui.progressbar_train->setMaximum(epochs);
		
		if (!ui.check_del_targets->isChecked() && !ui.check_del_outputs->isChecked() && ui.check_exogenous->isChecked()
			&& ui.spinbox_xregressor->value()==0)
		{
			QMessageBox::information( this, "NARX", "Selected architecture: MLP" );
			LOG("Selected architecture: MLP");
			arch = MLP;
		}
		else if (ui.check_del_targets->isChecked() && !ui.check_del_outputs->isChecked() && !ui.check_exogenous->isChecked())
		{
			QMessageBox::information( this, "NARX", "Selected architecture: NAR-D" );
			LOG("Selected architecture: NAR-D");
			arch = NAR_D;
		}
		else if (!ui.check_del_targets->isChecked() && !ui.check_del_outputs->isChecked() && ui.check_exogenous->isChecked()
			&& ui.spinbox_xregressor->value()>0)
		{
			QMessageBox::information( this, "NARX", "Selected architecture: TDNN-X" );
			LOG("Selected architecture: TDNN-X");
			arch = TDNN_X;
		}
		else if (ui.check_del_targets->isChecked() && !ui.check_del_outputs->isChecked() && ui.check_exogenous->isChecked()
			&& 1)
		{
			QMessageBox::information( this, "NARX", "Selected architecture: NARX-D" );
			LOG("Selected architecture: NARX-D");
			arch = NARX_D;
		}
		else if (!ui.check_del_targets->isChecked() && ui.check_del_outputs->isChecked() && ui.check_exogenous->isChecked()
			)
		{
			QMessageBox::information( this, "NARX", "Selected architecture: NARX-Y" );
			LOG("Selected architecture: NARX-Y");
			arch = NARX_Y;
		}
		else if (ui.check_del_targets->isChecked() && ui.check_del_outputs->isChecked() && ui.check_exogenous->isChecked()
			&& 1)
		{
			QMessageBox::information( this, "NARX", "Selected architecture: NARX-DY" );
			LOG("Selected architecture: NARX-DY");
			arch = NARX_DY;
		}
		else if (!ui.check_del_targets->isChecked() && ui.check_del_outputs->isChecked() && !ui.check_exogenous->isChecked()
			&& 1)
		{
			QMessageBox::information( this, "NARX", "Selected architecture: NAR-Y" );
			LOG("Selected architecture: NAR-Y");
			arch = NAR_Y;
		}
		else if (ui.check_del_targets->isChecked() && ui.check_del_outputs->isChecked() && !ui.check_exogenous->isChecked()
			&& 1)
		{
			QMessageBox::information( this, "NARX", "Selected architecture: NAR-DY" );
			LOG("Selected architecture: NAR-DY");
			arch = NAR_DY;
		}
		else
		{
			QMessageBox::warning( this, "NARX", "Selected no architecture: training will probably fail." );
			LOG( "Selected no architecture: training will probably fail.");
			arch = UNKNWN;
		}


		narx_stage1_5 = 1;
		old_M = M;
		if(!ui.check_exogenous->isChecked()) // we dont want to use any exogenous
			M=0;
		
		mynarx = new NARX(arch, ui.spinbox_hidden_units->value(), 
			/* now the act func for hiddens */ ui.combo_hunits_act->currentIndex(),
			ui.spinbox_xregressor->value(), ui.spinbox_dregressor->value(),
		M,
		N,// output units

		ui.check_del_outputs->isChecked(), //feedback
		ui.check_del_targets->isChecked()
		
		);
	
	} 
	/* end of main NARX code */



	QObject::connect ( mynarx, SIGNAL( training_epoch_finished() ), this, SLOT( train_progress_inc() ) );
	QObject::connect ( mynarx, SIGNAL( log() ), this, SLOT( log() ), Qt::BlockingQueuedConnection );

	ui.tabWidget->setTabEnabled(4, true);
	ui.tabWidget->setCurrentIndex(4);
	ui.spinbox_xregressor->setEnabled(false);
	//ui.spinbox_yregressor->setEnabled(false);
	ui.spinbox_dregressor->setEnabled(false);
	ui.lineedit_epochs->setEnabled(false);
	ui.lineedit_learningrate->setEnabled(false);
	ui.combo_hunits_act->setEnabled(false);
	ui.combo_ounits_act->setEnabled(false);
}

void NARX2::Button_54()
{
	ui.tabWidget->setTabEnabled(3, true);
	ui.tabWidget->setCurrentIndex(3);
}

void NARX2::Button_56()
{
	ui.tabWidget->setTabEnabled(5, true);
	ui.tabWidget->setCurrentIndex(5);
	
	
	ui.scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	ui.scrollAreaWidgetContents->setGeometry(40,60,201,60 + 30*old_M);

	if(!used_exogenous[0] || !M) ui.lineEdit_exo1->setEnabled(false);

	

	for(int i =1; i<old_M;i++)
	{
		

	QLabel *label_exo = new QLabel(ui.scrollAreaWidgetContents);
        label_exo->setObjectName(QString("label_exio%1").arg(i+1));
        label_exo->setGeometry(QRect(10, 10+30*(i+0), 21, 16));
		label_exo->setText(QString("%1").arg(i+1));
		label_exo->show();
		
      QLineEdit *lineEdit_exo = new QLineEdit(ui.scrollAreaWidgetContents);
        lineEdit_exo->setObjectName(QString("lineedit_exio%1").arg(i+1));
        lineEdit_exo->setGeometry(QRect(40, 10+30*(i+0), 113, 20));
		//ui.scrollArea->setWidget(ui.scrollAreaWidgetContents);
		if(!used_exogenous[i] || !M)
			lineEdit_exo->setEnabled(false);
		lineEdit_exo->show();

		//ui.scrollAreaWidgetContents->repaint();
	}
}


void NARX2::Button_browse_action()
{
	QString fileName = QFileDialog::getOpenFileName(this,
     tr("Open series"), ".");
	//LOG(fileName);
	FILE *series_file = fopen(fileName.toStdString().c_str(),"rt");
	if(!series_file) {
		QMessageBox::information( this, "Error", "Cannot load series." );
		LOG("Error:Cannot load series.");
		return;
	}
	
	
	fscanf(series_file, "%d", &series_len);
	fscanf(series_file, "%d", &M);
	fscanf(series_file, "%d", &N);
	LOG(QString("Loading %1 values from file.\nLoading %2 target series").arg(series_len).arg(N));

	series = new double*[N];

	for(int i = 0;i<N; i++)
		series[i] = new double[series_len];

	exogenous_series = new double*[M];
	used_exogenous = new int[M];
	for(int i=0;i<M;i++)
	{
		used_exogenous[i]= 0;
		exogenous_series[i]=new double[series_len];
	}

	for(int i=0;i<M;i++)
	{
		if(i)
		{
			//add a new column for the exogenous variable
		ui.table_series->insertColumn(i);
		QTableWidgetItem * col1= new QTableWidgetItem();
		col1->setText(QString("%1 (input value - exogenous)").arg(i));
		ui.table_series->setHorizontalHeaderItem(i, col1);
		QTableWidgetItem * exo1= new QTableWidgetItem();
		ui.table_series->setItem(0, i, exo1);
		exo1->setText("Use in NARX");
		exo1->setCheckState(Qt::Checked);
		}

		double aux;

		for(int j=0;j<series_len;j++) 
		{
			if(!i) ui.table_series->insertRow(j +1 );

			fscanf(series_file, "%lf", &aux);
			exogenous_series[i][j]=aux;
			QTableWidgetItem *newItemY = new QTableWidgetItem(tr("%1").arg(aux,10));
			ui. table_series->setItem(j+1, i, newItemY);
		}
	}

	double aux;
	for(int i=0;i<N;i++)
	for(int j=0;j<series_len;j++) 
		{
			//if(!i) ui.table_series->insertRow(j +1 );
			if(i)
			{
				ui.table_series->insertColumn(M+i);
				QTableWidgetItem * col1= new QTableWidgetItem();
				col1->setText(QString("D%1 (target values)").arg(i));
				ui.table_series->setHorizontalHeaderItem(M+i, col1);
			}
			fscanf(series_file, "%lf", &aux);
			series[i][j]=aux;
			QTableWidgetItem *newItemY = new QTableWidgetItem(tr("%1").arg(aux,10));
			ui. table_series->setItem(j+1, M+i, newItemY);
		}

	series_generated = 1;
	fclose(series_file);
	QMessageBox::information( this, "Proceed", "Series loaded successfully." );

}

void NARX2::Button_predict()
{
	QInputDialog::getDouble ( this, "NARX", "Predicted value: %1.\nPlease input target if further prediction required:"		);
}
#endif
