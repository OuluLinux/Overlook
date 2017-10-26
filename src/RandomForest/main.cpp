#ifdef flagMAIN
#include "RandomForest.h"

#define IMAGECLASS RandomForextImg
#define IMAGEFILE <RandomForest/RandomForext.iml>
#include <Draw/iml_source.h>

namespace RandomForest {

RandomForestDemo::RandomForestDemo() {
	CtrlLayout(*this, "Random Forest Demo");
	Icon(RandomForextImg::icon());
	
	tree_count.MinMax(1, 200);
	tree_count.SetData(100);
	tree_count <<= THISBACK(Retrain);
	
	max_depth.MinMax(2, 12);
	max_depth.SetData(4);
	max_depth <<= THISBACK(Retrain);
	
	hypothesis.MinMax(1, 50);
	hypothesis.SetData(10);
	hypothesis <<= THISBACK(Retrain);
	
	toggle_type <<= THISBACK(ToggleType);
	toggle_drawing <<= THISBACK(ToggleDrawing);
	
	rfctrl.WhenItem = THISBACK(AddItem);
	
	PostCallback(THISBACK(Init));
}

void RandomForestDemo::Init() {
	N = 10;
	data.data.SetCount(N);
	data.labels.SetCount(N);
	
	data.options.type = 1;
	
	data.data[0] = Pair(-0.4326,	1.1909);
	data.data[1] = Pair(1.5,		3.0);
	data.data[2] = Pair(0.1253,		-0.0376);
	data.data[3] = Pair(0.2877,		0.3273);
	data.data[4] = Pair(-1.1465,	0.1746);
	data.data[5] = Pair(1.8133,		2.1139);
	data.data[6] = Pair(2.7258,		3.0668);
	data.data[7] = Pair(1.4117,		2.0593);
	data.data[8] = Pair(4.1832,		1.9044);
	data.data[9] = Pair(1.8636,		1.1677);
	
	data.labels[0]= 1;
	data.labels[1]= 1;
	data.labels[2]= 1;
	data.labels[3]= 1;
	data.labels[4]= 1;
	data.labels[5]= 0;
	data.labels[6]= 0;
	data.labels[7]= 0;
	data.labels[8]= 0;
	data.labels[9]= 0;
	
	rfctrl.SetData(data);
	rfctrl.SetTree(tree);
	
	Retrain();
}

void RandomForestDemo::Retrain() {
	data.options.tree_count = tree_count.GetData();
	data.options.max_depth = max_depth.GetData();
	data.options.tries_count = hypothesis.GetData();
	tree.Train(data.data, data.labels, data.options);
	dirty = true;
	rfctrl.Refresh();
}

void RandomForestDemo::AddItem(double x, double y, bool value) {
	data.data.Add(Pair(x, y));
	data.labels.Add(value);
	Retrain();
}

void RandomForestDemo::ToggleType() {
	data.options.type = !data.options.type;
	Retrain();
}

void RandomForestDemo::ToggleDrawing() {
	rfctrl.ToggleDrawing();
}

}





GUI_APP_MAIN {
	RandomForest::RandomForestDemo().Run();
}

#endif
