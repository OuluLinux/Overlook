#include "Overlook.h"

namespace Overlook {
using namespace Upp;

void BooleansDraw::Paint(Draw& w) {
	System& sys = GetSystem();
	BitProcess& bp = GetBitProcessManager().Get(0, tf);
	
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	if (cursor >= 0 && cursor < bp.processbits_cursor) {
		int w = BitProcess::processbits_row_size;
		int h = USEDSYMBOL_COUNT;
		
		double xstep = (double)sz.cx / w;
		double ystep = (double)sz.cy / h;
		
		for(int i = 0; i < h; i++) {
			BitProcess& bp = GetBitProcessManager().Get(i, tf);
			int count =bp.processbits_cursor;
			if (count == 0) continue;
			
			int y0 = i * ystep;
			int y1 = (i + 1) * ystep;
			for(int j = 0; j < w; j++) {
				int x0 = j * xstep;
				int x1 = (j + 1) * xstep;
				
				bool b = bp.GetBit(cursor, j);
				if (b)
					id.DrawRect(x0, y0, x1-x0, y1-y0, Black());
			}
		}
	}
	
	w.DrawImage(0, 0, id);
}




BitProcessManagerCtrl::BitProcessManagerCtrl() {
	Add(tflist.TopPos(0,30).LeftPos(0,100));
	Add(slider.TopPos(0,30).HSizePos(100,0));
	Add(bools.HSizePos().VSizePos(30,0));
	
	slider.MinMax(0,1);
	
	tflist << THISBACK(SetTf);
	slider << THISBACK(Data);
}

void BitProcessManagerCtrl::Data() {
	System& sys = GetSystem();
	
	if (tflist.GetCount() == 0) {
		
		for(int i = 0; i < sys.GetPeriodCount(); i++)
			tflist.Add(sys.GetPeriodString(i));
		
		tflist.SetIndex(tflist.GetCount()-1);
	}
	
	bools.tf = tflist.GetIndex();
	bools.cursor = slider.GetData();
	bools.Refresh();
}

void BitProcessManagerCtrl::SetTf() {
	for(int i = 0; i < USEDSYMBOL_COUNT; i++) {
		BitProcess& bp = GetBitProcessManager().Get(i, tflist.GetIndex());
		bp.Refresh();
		
		if (i == 0) {
			int last = bp.processbits_cursor - 1;
			slider.MinMax(0, last);
			if (last < 0) return;
		}
	}
}






ExchangeSlotsCtrl::ExchangeSlotsCtrl() {
	Add(splitter.SizePos());
	splitter << slotlist << openlist;
	splitter.Horz();
	
	slotlist.AddColumn("Id");
	slotlist << THISBACK(Data);
	
	openlist.AddColumn("Open time");
	openlist.AddColumn("Close time");
	
}

void ExchangeSlotsCtrl::Data() {
	ExchangeSlots& es = GetExchangeSlots();
	
	es.Refresh();
	
	for(int i = 0; i < es.slots.GetCount(); i++) {
		Slot& slot = es.slots[i];
		
		slotlist.Set(i, 0, slot.id);
	}
	
	int cursor = slotlist.GetCursor();
	if (cursor >= 0 && cursor < es.slots.GetCount()) {
		Slot& slot = es.slots[cursor];
		
		for(int j = 0; j < slot.open_time.GetCount(); j++) {
			openlist.Set(j, 0, slot.open_time[j]);
			openlist.Set(j, 1, slot.close_time[j]);
		}
		openlist.SetCount(slot.open_time.GetCount());
	}
}






SlotSignalsCtrl::SlotSignalsCtrl() {
	Add(splitter.SizePos());
	splitter << slotlist << openlist;
	splitter.Horz();
	
	slotlist.AddColumn("Id");
	slotlist << THISBACK(Data);
	
	openlist.AddColumn("Open time");
	openlist.AddColumn("Close time");
	openlist.AddColumn("Real signal");
	openlist.AddColumn("Prediction float");
	openlist.AddColumn("Predicted signal");
	openlist.AddColumn("Is correct");
	
}

void SlotSignalsCtrl::Data() {
	SlotSignals& ss = GetSlotSignals();
	
	ss.Refresh();
	
	for(int i = 0; i < ss.slots.GetCount(); i++) {
		SlotSignal& slot = ss.slots[i];
		
		slotlist.Set(i, 0, slot.id);
	}
	
	int cursor = slotlist.GetCursor();
	if (cursor >= 0 && cursor < ss.slots.GetCount()) {
		SlotSignal& slot = ss.slots[cursor];
		
		for(int j = 0; j < slot.data.GetCount(); j++) {
			SlotSignal::Data& data = slot.data[j];
			
			openlist.Set(j, 0, data.open_time);
			openlist.Set(j, 1, data.close_time);
			openlist.Set(j, 2, data.signal);
			openlist.Set(j, 3, data.pred_value);
			openlist.Set(j, 4, data.pred_signal);
			openlist.Set(j, 5, data.signal == data.pred_signal ? "Is correct" : "");
		}
		openlist.SetCount(slot.data.GetCount());
	}
}

}
