#include "NarxSim.h"

#define IMAGECLASS Imgs
#define IMAGEFILE <NarxSim/NarxSim.iml>
#include <Draw/iml_source.h>

GUI_APP_MAIN {
	NarxSim().Run();
}

#if 0

#include <stdio.h>
#include <tchar.h>
#include <SDKDDKVer.h>
#include "narx2.h"
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "Unit.h"
#include "ActivationFunctions.h"
#include "InputUnit.h"
#include "OutputUnit.h"
#include "stdlib.h"
#include "time.h"
#include "NARX.h"
#include "qmath.h"

//#include <QtGui/QApplication>

NARX2* w;
FILE* outfile = 0;


NARX* mynarx = NULL;







int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	outfile = fopen("log.txt", "wt");
	NARX2 w;
	//w.ui.Frame_generate->setHidden(true);
	w.show();
	::w = &w;
	srand(time(0));
	/* QtCore.QObject.connect(self.radioButton1,QtCore.SIGNAL("toggled(bool)"),self.radio_activateInput) */
	w.ui.tabWidget->setCurrentIndex(0);
	w.ui.frame_1post->setHidden(true);
	w.ui.frame_load->setHidden(true);
	//w.ui.GenerateSeries->connect(w.ui, QtCore::SIGNAL("toggled(bool)"),
	
	w.ui.tabWidget->setTabEnabled(1, false);
	w.ui.tabWidget->setTabEnabled(2, false);
	w.ui.tabWidget->setTabEnabled(3, false);
	w.ui.tabWidget->setTabEnabled(4, false);
	w.ui.tabWidget->setTabEnabled(5, false);
	return a.exec();
}

#endif
