#include "Overlook.h"

namespace Overlook {

/*Core::Core() {
	
}

Core::~Core() {
	
}*/
/*
void Core::AddValue(uint16 bytes, String name, String description) {
	CoreValue& value = values.Add();
	value.bytes = bytes;
	value.name = name;
	value.description = description;
	data.Add();
}

void Core::SetReady(int sym_id, int tf_id, int pos, const CoreProcessAttributes& attr, bool ready) {
	byte* ready_slot = this->ready[sym_id][tf_id].Begin();
	ASSERT(ready_slot);
	int slot_pos = attr.slot_pos;
	int ready_bit = slot_pos % 8;
	ready_slot += slot_pos / 8;
	byte ready_mask = 1 << ready_bit;
	if (ready)
		*ready_slot |= ready_mask;
	else
		*ready_slot &= !ready_mask;
}

void Core::SetReady(int pos, const CoreProcessAttributes& attr, bool ready) {
	SetReady(GetSymbol(), GetTimeframe(), pos, attr, ready);
}

bool Core::IsReady(int pos, const CoreProcessAttributes& attr) {
	byte* ready_slot = ready[GetSymbol()][GetTimeframe()].Begin();
	if (!ready_slot)
		return false;
	int slot_pos = attr.slot_pos;
	int ready_bit = slot_pos % 8;
	ready_slot += slot_pos / 8;
	byte ready_mask = 1 << ready_bit;
	return ready_mask & *ready_slot;
}
*/


Core::Core()
{
	window_type = WINDOW_CHART;
	bars = 0;
	next_count = 0;
	counted = 0;
	has_maximum = 0;
	has_minimum = 0;
	maximum = 0;
	minimum = 0;
	skip_setcount = false;
	skip_allocate = false;
	point = 0.01;
	sym_id = -1;
	tf_id = -1;
	period = 0;
	end_offset = 0;
	
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

void Core::ClearContent() {
	bars = 0;
	counted = 0;
	for(int i = 0; i < buffers.GetCount(); i++) {
		buffers[i]->value.Clear();
	}
	for(int i = 0; i < subcores.GetCount(); i++) {
		subcores[i].ClearContent();
	}
}

void Core::InitAll() {
	
	// Clear values what can be added in the Init
	subcores.Clear();
	subcore_factories.Clear();
	
	
	// Initialize normally
	Init();
	
	// Initialize sub-cores
	const FactoryValueRegister& src_reg = Factory::GetRegs()[factory];
	for(int i = 0; i < subcores.GetCount(); i++) {
		Core& core = subcores[i];
		core.base = base;
		core.factory = subcore_factories[i];
		core.RefreshIO();
		core.SetSymbol(GetSymbol());
		core.SetTimeframe(GetTimeframe(), GetPeriod());
		
		const FactoryValueRegister& reg = Factory::GetRegs()[core.factory];
		
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
				if (rin.phase == src_rin.phase && rin.type == src_rin.type) {
					
					// Copy input
					in = src_in;
					
					found = true;
					break;
				}
			}
			ASSERT_(found, "Parent object did not have all inputs what sub-object needed");
		}
		
		core.InitAll();
	}
}

void Core::Refresh() {
	for(int i = 0; i < subcores.GetCount(); i++)
		subcores[i].Refresh();
	
	
	lock.EnterWrite();
	
	// Some indicators might want to set the size by themselves
	if (!skip_setcount) {
		int count = GetBaseSystem().GetCountTf(tf_id) + end_offset;
		bars = count;
		next_count = count;
		if (!skip_allocate) {
			for(int i = 0; i < buffers.GetCount(); i++) {
				buffers[i]->value.SetCount(count, 0);
			}
		}
	}
		
	Start();
	
	counted = next_count;
	lock.LeaveWrite();
}


void Core::SetPoint(double d) {
	point = d;
}







double Core::Open ( int shift ) {
	return inputs[0].sources[0].b->buffers[0].Get(shift);
}

double Core::Low( int shift ) {
	return inputs[0].sources[0].b->buffers[1].Get(shift);
}

double Core::High( int shift ) {
	return inputs[0].sources[0].b->buffers[2].Get(shift);
}

double Core::Volume ( int shift ) {
	return inputs[0].sources[0].b->buffers[3].Get(shift);
}

int Core::HighestHigh(int period, int shift) {
	ASSERT(period > 0);
	double highest = -DBL_MAX;
	int highest_pos = -1;
	for (int i = 0; i < period && shift >= 0; i++, shift--) {
		double high = High(shift);
		if (high > highest) {
			highest = high;
			highest_pos = shift;
		}
	}
	return highest_pos;
}

int Core::LowestLow(int period, int shift) {
	ASSERT(period > 0);
	double lowest = DBL_MAX;
	int lowest_pos = -1;
	for (int i = 0; i < period && shift >= 0; i++, shift--) {
		double low = Low(shift);
		if (low < lowest) {
			lowest = low;
			lowest_pos = shift;
		}
	}
	return lowest_pos;
}

int Core::HighestOpen(int period, int shift) {
	ASSERT(period > 0);
	double highest = -DBL_MAX;
	int highest_pos = -1;
	for (int i = 0; i < period && shift >= 0; i++, shift--) {
		double open = Open(shift);
		if (open > highest) {
			highest = open;
			highest_pos = shift;
		}
	}
	return highest_pos;
}

int Core::LowestOpen(int period, int shift) {
	ASSERT(period > 0);
	double lowest = DBL_MAX;
	int lowest_pos = -1;
	for (int i = 0; i < period && shift >= 0; i++, shift--) {
		double open = Open(shift);
		if (open < lowest) {
			lowest = open;
			lowest_pos = shift;
		}
	}
	return lowest_pos;
}

int Core::GetMinutePeriod() {
	BaseSystem& bs = GetBaseSystem();
	return bs.GetBasePeriod() * bs.GetPeriod(tf_id);
}

int Core::GetPeriod() const {
	ASSERT(period != 0);
	return period;
}

void Core::SetTimeframe(int i, int period) {
	tf_id = i;
	this->period = period;
}

double Core::GetAppliedValue ( int applied_value, int i )
{
	double dValue;
	
	switch ( applied_value )
	{

		case 0:
			dValue = Open(i);
			break;

		case 1:
			dValue = High(i);
			break;

		case 2:
			dValue = Low(i);
			break;

		case 3:
			dValue =
				( High(i) + Low(i) )
				/ 2.0;
			break;

		case 4:
			dValue =
				( High(i) + Low(i) + Open(i) )
				/ 3.0;
			break;

		case 5:
			dValue =
				( High(i) + Low(i) + 2 * Open(i) )
				/ 4.0;
			break;

		default:
			dValue = 0.0;
	}
	return ( dValue );
}

void Core::RefreshSource() {
	/*data = src.Get<BarData>();
	if (data)
		SetPoint(data->GetPoint());*/
}

}
