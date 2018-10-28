#include "Overlook.h"

namespace Overlook {

ChartManager::ChartManager() {
	SetMaximizeAll();
}


void ChartManager::Init() {
	
}

Chart& ChartManager::AddChart() {
	Chart* chart = new Chart();
	Chart& chr = AddSubWindow<Chart>(chart);
	return chr;
}

ForecastCtrl& ChartManager::AddForecast() {
	ForecastCtrl* net = new ForecastCtrl();
	ForecastCtrl& netc = AddSubWindow<ForecastCtrl>(net);
	return netc;
}

void ChartManager::RefreshWindows() {
	for(int i = 0; i < GetCount(); i++) {
		auto* sw = Get(i).GetSubWindowCtrl();
		if (sw) sw->Start();
	}
}


}
