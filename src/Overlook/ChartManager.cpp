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

NetCtrl& ChartManager::AddNet() {
	NetCtrl* net = new NetCtrl();
	NetCtrl& netc = AddSubWindow<NetCtrl>(net);
	return netc;
}

void ChartManager::RefreshWindows() {
	for(int i = 0; i < GetCount(); i++) {
		auto* sw = Get(i).GetSubWindowCtrl();
		if (sw) sw->Start();
	}
}


}
