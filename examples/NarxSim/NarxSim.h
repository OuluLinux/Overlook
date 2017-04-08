#ifndef _NarxSim_NarxSim_h
#define _NarxSim_NarxSim_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGEFILE <NarxSim/NarxSim.iml>
#include <Draw/iml_header.h>

class NarxSim : public TopWindow {
public:
	typedef NarxSim CLASSNAME;
	NarxSim();
};


#if 0
class NARX2 : public QMainWindow
{
	Q_OBJECT

public:
	NARX2(QWidget *parent = 0, Qt::WFlags flags = 0);
	~NARX2();

public slots:
	void RadioButton_generate_series();
	void RadioButton_load_series();
	void Button_12();
	void Button_21();
	void Button_23();
	void Button_34();
	void Button_32();
	void Button_43();
	void Button_45();
	void Button_54();
	void Button_56();
	void Menu_about();

	void Button_browse_action();

	void Button_start_train();

	void Button_predict();

	void train_progress_inc();
	void log();

public:
	Ui::NARX2Class ui;
};
#endif


#endif
