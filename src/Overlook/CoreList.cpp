#include "Overlook.h"

namespace Overlook {

CoreList::CoreList() {
	
}

void CoreList::Init() {
	System& sys = GetSystem();
	
	ASSERT(!symbols.IsEmpty(), "No symbols added");
	ASSERT(!tf_ids.IsEmpty(), "No timeframes added");
	ASSERT(!indi_ids.IsEmpty(), "No indicator factories added");
	ASSERT(work_queue.IsEmpty(), "Init only once");
	
	for(int i = 0; i < symbols.GetCount(); i++)
		sym_ids.Add(sys.FindSymbol(symbols[i]));
	
	for(int i = 0; i < indi_ids.GetCount(); i++) {
		const FactoryDeclaration& decl = indi_ids[i];
		String& s = indi_lbls.Add();
		s = sys.CoreFactories()[decl.factory].a;
		for(int j = 0; j < decl.arg_count; j++) {
			if (j) s << ",";
			s << " ";
			s << decl.args[j];
		}
	}
	
	sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	bufs.SetCount(sym_ids.GetCount());
	lbls.SetCount(sym_ids.GetCount());
	db.SetCount(sym_ids.GetCount(), NULL);
	db_m1.SetCount(sym_ids.GetCount(), NULL);
	for(int i = 0; i < bufs.GetCount(); i++) {
		bufs[i].SetCount(indi_ids.GetCount());
		lbls[i].SetCount(indi_ids.GetCount(), NULL);
	}
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, false, true);
		
		Core& c = *ci.core;
		
		int faci = -1;
		for(int i = 0; i < indi_ids.GetCount(); i++) {
			const FactoryDeclaration& fd = indi_ids[i];
			if (fd.factory == c.GetFactory()) {
				bool match = true;
				for(int j = 0; j < fd.arg_count && match; j++)
					if (fd.args[j] != ci.args[j])
						match = false;
				if (match) {
					faci = i;
					break;
				}
			}
		}
		int symi = sym_ids.Find(c.GetSymbol());
		int tfi = tf_ids.Find(c.GetTf());
		
		if (symi == -1) continue;
		if (c.GetTf() == 0 && c.GetFactory() == 0)
			db_m1[symi] = dynamic_cast<DataBridge*>(&c);
		if (c.GetTf() == tf_ids[0] && c.GetFactory() == 0)
			db[symi] = dynamic_cast<DataBridge*>(&c);
		if (tfi == -1) continue;
		if (faci == -1) continue;
		
		auto& v = bufs[symi][faci];
		v.SetCount(c.GetBufferCount());
		for(int j = 0; j < c.GetBufferCount(); j++) {
			v[j] = &c.GetBuffer(j);
		}
		
		if (c.GetLabelCount() && c.GetLabelBufferCount(0)) {
			lbls[symi][faci] = &c.GetLabelBuffer(0, 0);
		}
	}
	ASSERT(db_m1[0]);
	
}

void CoreList::Refresh() {
	System& sys = GetSystem();
	for(int i = 0; i < work_queue.GetCount(); i++)
		sys.Process(*work_queue[i], false);
}

String CoreList::GetIndiDescription(int i) const {
	if (i < 0 || i >= indi_lbls.GetCount())
		return "Unknown";
	return indi_lbls[i];
}

}