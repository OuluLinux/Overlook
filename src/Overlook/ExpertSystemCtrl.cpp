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
	Add(symlist.LeftPos(0,100).TopPos(0,30));
	Add(find_current.LeftPos(100,100).TopPos(0,30));
	Add(refresh.LeftPos(200,100).TopPos(0,30));
	Add(splitter.HSizePos().VSizePos(30));
	
	symlist << THISBACK(Data);
	find_current.SetLabel("Find current");
	refresh.SetLabel("Refresh");
	find_current << THISBACK(FindCurrent);
	refresh << THISBACK(RefreshSource);
	
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

void SlotSignalsCtrl::RefreshSource() {
	SlotSignals& ss = GetSlotSignals();
	ss.Refresh();
}

void SlotSignalsCtrl::FindCurrent() {
	SlotSignals& ss = GetSlotSignals();
	Time now = GetUtcTime();
	
	Time max_time(1970,1,1);
	int max_i = 0;
	
	for(int i = 0; i < ss.slots.GetCount(); i++) {
		SlotSignal& slot = ss.slots[i];
		
		for(int j = slot.data.GetCount() - 1; j >= 0; j--) {
			SlotSignal::Data& data = slot.data[j];
			if (data.open_time > now)
				continue;
			
			if (data.open_time > max_time) {
				max_time = data.open_time;
				max_i = i;
			}
			break;
		}
	}
	
	slotlist.SetCursor(max_i);
	openlist.SetCursor(openlist.GetCount() - 1);
}

void SlotSignalsCtrl::Data() {
	System& sys = GetSystem();
	SlotSignals& ss = GetSlotSignals();
	
	if (symlist.GetCount() == 0) {
		for(int i = 0; i < sys.used_symbols.GetCount(); i++)
			symlist.Add(sys.used_symbols[i]);
		symlist.SetIndex(0);
	}
	
	for(int i = 0; i < ss.slots.GetCount(); i++) {
		SlotSignal& slot = ss.slots[i];
		
		slotlist.Set(i, 0, slot.id);
	}
	
	int sym = symlist.GetIndex();
	int cursor = slotlist.GetCursor();
	if (cursor >= 0 && cursor < ss.slots.GetCount()) {
		SlotSignal& slot = ss.slots[cursor];
		
		for(int j = 0; j < slot.data.GetCount(); j++) {
			SlotSignal::Data& data = slot.data[j];
			
			openlist.Set(j, 0, data.open_time);
			openlist.Set(j, 1, data.close_time);
			openlist.Set(j, 2, data.signal[sym]);
			openlist.Set(j, 3, data.pred_value[sym]);
			openlist.Set(j, 4, data.pred_signal[sym]);
			openlist.Set(j, 5, data.signal[sym] == data.pred_signal[sym] ? "Is correct" : "");
		}
		openlist.SetCount(slot.data.GetCount());
	}
}

}
