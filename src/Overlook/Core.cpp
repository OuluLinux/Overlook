#include "Overlook.h"

#if 0

namespace Overlook {
	


Core::Core()
{
	bars = 0;
	next_count = 0;
	counted = 0;
	has_maximum = 0;
	has_minimum = 0;
	maximum = 0;
	minimum = 0;
	skip_setcount = false;
	skip_allocate = false;
	period = 0;
	end_offset = 0;
	future_bars = 0;
	db_src = -1;
}

Core::~Core() {
	
}

Core& Core::Set(String key, Value value) {
	ArgChanger arg;
	arg.SetLoading();
	IO(arg);
	for(int i = 0; i < arg.keys.GetCount(); i++) {
		if (arg.keys[i] == key) {
			arg.args[i] = value;
			break;
		}
	}
	arg.SetStoring();
	IO(arg);
	return *this;
}

void Core::InitAll() {
	System& sys = GetSystem();
	
	
	// Find DataBridge input if it exists
	db_src = -1;
	for(int i = 0; i < inputs.GetCount(); i++) {
		const Input& in = inputs[i];
		if (in.IsEmpty()) continue;
		const Source& src = in[0];
		if (src.core && src.core->GetFactory() == 0) {
			db_src = i;
			break;
		}
	}
		
		
	// Clear values what can be added in the Init
	subcores.Clear();
	subcore_factories.Clear();
	
	
	// Initialize normally
	Init();
	
	
	// Initialize sub-cores
	const FactoryRegister& src_reg = sys.regs[factory];
	for(int i = 0; i < subcores.GetCount(); i++) {
		Core& core = subcores[i];
		core.factory = subcore_factories[i];
		core.RefreshIO();
		core.SetSymbol(GetSymbol());
		core.SetTimeframe(GetTimeframe(), ci.GetPeriod());
		
		const FactoryRegister& reg = sys.regs[core.factory];
		
		// Loop all inputs of the sub-core-object to be connected
		ASSERT(core.inputs.GetCount() == reg.in.GetCount());
		for(int j = 0; j < core.inputs.GetCount(); j++) {
			Input& in = core.inputs[j];
			const RegisterInput& rin = reg.in[j];
			
			// Loop all connected inputs of this parent object
			bool found = false;
			for(int k = 0; k < inputs.GetCount(); k++) {
				const Input& src_in = inputs[k];
				const RegisterInput& src_rin = src_reg.in[k];
				if (src_rin.factory != rin.factory)
					continue;
				
				// Copy input
				in <<= src_in;
				
				found = true;
				break;
			}
			ASSERT_(found, "Parent object did not have all inputs what sub-object needed");
		}
		
		core.InitAll();
	}
	
	
	is_init = true;
}

void Core::RefreshSources() {
	for(int i = 0; i < inputs.GetCount(); i++) {
		Input& in = inputs[i];
		for(int j = 0; j < in.GetCount(); j++) {
			Source& src = in[j];
			if (src.core) {
				Core* core = dynamic_cast<Core*>(src.core);
				if (core)
					core->Refresh();
			}
		}
	}
	
	Refresh();
}

void Core::RefreshSourcesOnlyDeep() {
	for(int i = 0; i < inputs.GetCount(); i++) {
		Input& in = inputs[i];
		for(int j = 0; j < in.GetCount(); j++) {
			Source& src = in[j];
			if (src.core) {
				Core* core = dynamic_cast<Core*>(src.core);
				if (core) {
					core->RefreshSourcesOnlyDeep();
					core->Refresh();
				}
			}
		}
	}
}
/*
void Core::Refresh() {
	refresh_lock.Enter();
	
	for(int i = 0; i < subcores.GetCount(); i++)
		subcores[i].Refresh();
	
	
	
	// Some indicators might want to set the size by themselves
	if (!skip_setcount) {
		int count = GetInputBuffer(0,0).GetCount() + end_offset;
		bars = count;
		next_count = count;
		if (!skip_allocate) {
			int reserve = count + 512;
			reserve -= reserve % 512;
			for (auto& o : outputs) {
				for (auto& b : o.buffers) {
					b.value.Reserve(reserve);
					b.value.SetCount(count, 0);
				}
			}
		}
	}
		
	Start();
	
	counted = next_count;
	
	refresh_lock.Leave();
	
}
*/






DataBridge* Core::GetDataBridge() {
	for(int i = 0; i < inputs.GetCount(); i++) {
		DataBridge* bd = dynamic_cast<DataBridge*>(GetInputCore(i));
		if (bd) return bd;
	}
	return Get<DataBridge>();
}

int Core::GetPeriod() {
	return GetSystem().GetPeriod(tf_id);
}

int Core::GetPeriod() const {
	ASSERT(period != 0);
	return period;
}

void Core::SetTimeframe(int i, int period) {
	CoreIO::SetTimeframe(i);
	this->period = period;
}



}

#endif
