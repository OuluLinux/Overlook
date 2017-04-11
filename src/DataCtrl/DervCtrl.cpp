#include "DataCtrl.h"

namespace DataCtrl {

DervDraw::DervDraw() {
	sym = 0;
	tf = 0;
	week = 0;
	
}

void DervDraw::Paint(Draw& w) {
	TimeVector& tv = GetTimeVector();
	
	if (!src) {
		src = tv.FindLinkSlot("/open_derv");
		ASSERT(src);
	}
	
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	int tf = tv.GetTfFromSeconds(7*24*60*60); // 1 week
	int begin_pos = week * tf;
	int end_pos = Upp::min(tv.GetCount(tf), begin_pos + tf);
	
	TimeVector::Iterator it = tv.Begin();
	it.SetPosition(begin_pos);
	
	int count = end_pos - begin_pos;
	double xstep = (double)sz.cx / (count-1);
	
	
	
	// Reserve fast memory for visible values to avoid iterating slow memory twice
	int sym_count = 1;
	tmp.SetCount(sym_count); // symbols
	tmp[0].SetCount(count); // timeframe series size
	
	for(int i = 0; i < count; i++) {
		
		for(int j = 0; j < sym_count; j++) {
			
			
			
		}
		
		it++;
	}
	
	for(int i = 0; i < count; i++) {
		int x = xstep * i;
		
		
		
		it++;
	}
	
	w.DrawImage(0, 0, id);
}
















DervCtrl::DervCtrl() {
	CtrlLayout(*this);
	
	refresh.Set(true);
	
	PostCallback(THISBACK(Refresher));
}

void DervCtrl::Refresher() {
	
	draw.Refresh();
	
	if (refresh.Get())
		PostCallback(THISBACK(Refresher));
}

}
