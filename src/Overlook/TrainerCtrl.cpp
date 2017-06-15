#include "Overlook.h"

namespace Overlook {
using namespace Upp;

TrainerDraw::TrainerDraw() {
	
}

void TrainerDraw::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	int sym_count;
	int tf_count;
	int value_count;
	
	int rows = sym_count * tf_count;
	
	
	
	w.DrawImage(0,0,id);
}





TrainerCtrl::TrainerCtrl(Trainer& trainer) {
	
}

void TrainerCtrl::RefreshData() {
	
}

}
