#include "Overlook.h"

namespace Overlook {

ChartManager::ChartManager() {
	backtest_broker = false;
	
	SetMaximizeAll();
	
}


void ChartManager::Init() {
	
}


/*GraphGroupCtrl& ChartManager::AddChart(int id, int tf_id) {
	return AddChart(id, tf_id);
}*/

/*GraphGroupCtrl& ChartManager::AddChart(int id, int tf_id) {
	GraphGroupCtrl* chart = new Chart();
	chart->Init(this, id, tf_id);
	GraphGroupCtrl& chr = AddSubWindow<Chart>(chart);
	chr.Refresh();
	return chr;
}*/

GraphGroupCtrl& ChartManager::AddChart() {
	GraphGroupCtrl* chart = new GraphGroupCtrl();
	GraphGroupCtrl& chr = AddSubWindow<GraphGroupCtrl>(chart);
	return chr;
}

void ChartManager::RefreshBacktestWindows() {
	Panic("CLEAN");
	/*for(int i = 0; i < GetCount(); i++) {
		SubWindow& sw = SubWindows::operator[](i);
		SubWindowCtrl* swc = sw.GetSubWindowCtrl();
		
		ChartWindow* cw = dynamic_cast<ChartWindow*>(swc);
		if (cw) {
			if (cw->IsSimulation())
				cw->RefreshData();
		}
	}*/
}

void ChartManager::CloseBacktestWindows(bool exclude_indicators) {
	Panic("CLEAN");
	/*for(int i = 0; i < GetCount(); i++) {
		SubWindow& sw = SubWindows::operator[](i);
		SubWindowCtrl* swc = sw.GetSubWindowCtrl();
		
		ChartWindow* cw = dynamic_cast<ChartWindow*>(swc);
		if (cw) {
			if (cw->IsSimulation()) {
				if (exclude_indicators) {
					GraphGroupCtrl* c = dynamic_cast<GraphGroupCtrl*>(cw);
					if (!c) {
						SubWindows::CloseWindow(i);
						i--;
						continue;
					}
				} else {
					SubWindows::CloseWindow(i);
					i--;
					continue;
				}
			}
			continue;
		}
	}
	*/
}

}
