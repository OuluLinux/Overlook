#include "Overlook.h"

namespace Overlook {
	
#ifdef flagDEBUG
double Buffer::Get(int i) const {
	if (check_cio) check_cio->SafetyInspect(i);
	return value[i];
}

void Buffer::Set(int i, double value) {
	if (check_cio) check_cio->SafetyInspect(i);
	this->value[i] = value;
	if (i < earliest_write) earliest_write = i;
}

void Buffer::Inc(int i, double value) {
	if (check_cio) check_cio->SafetyInspect(i);
	this->value[i] += value;
}
#endif


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

void Core::ResetSubCores() {
	subcore_factories.Clear();
	subcores.Clear();
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

void Core::AllowJobs() {
	for(int i = 0; i < jobs.GetCount(); i++)
		jobs[i].allow_processing = true;
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
	
	
	// Register jobs
	if (!jobs.IsEmpty()) {
		JobThread& thrd = sys.GetJobThread(sym_id, tf_id);
		WRITELOCK(thrd.job_lock) {
			for(int i = 0; i < jobs.GetCount(); i++) {
				Job& job = jobs[i];
				Core* core = job.core;
				if (core == NULL)
					Panic("You haven't called SetJob for all jobs or SetJobCount has invalid count.");
				thrd.jobs.Add(&job);
			}
		}
	}
	
	
	// Initialize sub-cores
	InitSubCores();
	
	
	
	is_init = true;
}

void Core::InitSubCores() {
	System& sys = GetSystem();
	FactoryRegister& src_reg = sys.regs[factory];
	for(int i = 0; i < subcores.GetCount(); i++) {
		Core& core = subcores[i];
		core.factory = subcore_factories[i];
		core.RefreshIO();
		core.SetSymbol(GetSymbol());
		core.SetTimeframe(GetTimeframe(), GetPeriod());
		
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
		core.Ready();
	}
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

void Core::RefreshSubCores() {
	for(int i = 0; i < subcores.GetCount(); i++)
		subcores[i].Refresh();
}

void Core::Refresh() {
	if (avoid_refresh)
		return;
	
	while (!ready) {Sleep(100);}
	
	if (!refresh_lock.TryEnter())
		return;
	
	RefreshSubCores();
	
	
	// Some indicators might want to set the size by themselves
	if (!skip_setcount) {
		int input_count = GetInputBuffer(0,0).GetCount();
		if (input_count == 0) {
			refresh_lock.Leave();
			return;
		}
		int count = input_count + end_offset;
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
			for (auto& l : labels) {
				for (auto& b : l.buffers) {
					b.signal.Reserve(reserve);
					b.signal.SetCount(count, 0);
					b.enabled.Reserve(reserve);
					b.enabled.SetCount(count, 1);
				}
			}
		}
	}
		
	Start();
	
	counted = next_count;
	
	refresh_lock.Leave();
	
}







DataBridge* Core::GetDataBridge() {
	for(int i = 0; i < inputs.GetCount(); i++) {
		DataBridge* bd = dynamic_cast<DataBridge*>(GetInputCore(i));
		if (bd) return bd;
	}
	return Get<DataBridge>();
}

double Core::Open ( int shift ) {
	SAFETYASSERT(shift <= read_safety_limit);
	return GetInputBuffer(db_src, GetSymbol(), GetTimeframe(), 0).Get(shift);
}

double Core::Low( int shift ) {
	SAFETYASSERT(shift < read_safety_limit);
	return GetInputBuffer(db_src, GetSymbol(), GetTimeframe(), 1).Get(shift);
}

double Core::High( int shift ) {
	SAFETYASSERT(shift < read_safety_limit);
	return GetInputBuffer(db_src, GetSymbol(), GetTimeframe(), 2).Get(shift);
}

double Core::Volume ( int shift ) {
	SAFETYASSERT(shift <= read_safety_limit);
	return GetInputBuffer(db_src, GetSymbol(), GetTimeframe(), 3).Get(shift);
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

double Core::GetAppliedValue ( int applied_value, int i ) {
	double dValue;
	
	switch ( applied_value ) {
		case 0:
			SAFETYASSERT(i <= read_safety_limit);
			dValue = Open(i);
			break;
		case 1:
			SAFETYASSERT(i < read_safety_limit);
			dValue = High(i);
			break;
		case 2:
			SAFETYASSERT(i < read_safety_limit);
			dValue = Low(i);
			break;
		case 3:
			SAFETYASSERT(i < read_safety_limit);
			dValue =
				( High(i) + Low(i) )
				/ 2.0;
			break;
		case 4:
			SAFETYASSERT(i < read_safety_limit);
			dValue =
				( High(i) + Low(i) + Open(i) )
				/ 3.0;
			break;
		case 5:
			SAFETYASSERT(i < read_safety_limit);
			dValue =
				( High(i) + Low(i) + 2 * Open(i) )
				/ 4.0;
			break;
		default:
			dValue = 0.0;
	}
	return ( dValue );
}

bool Core::IsJobsFinished() const {
	bool all_done = IsDependencyJobsFinished();
	for(int i = 0; i < jobs.GetCount(); i++)
		all_done &= jobs[i].IsFinished();
	return all_done;
}

bool Core::IsDependencyJobsFinished() const {
	bool all_done = true;
	for(int i = 0; i < inputs.GetCount(); i++) {
		const Input& input = inputs[i];
		for(int j = 0; j < input.GetCount(); j++)
			all_done &= dynamic_cast<Core*>(input[j].core)->IsJobsFinished();
	}
	return all_done;
}

Job& Core::SetJob(int i, String job_title) {
	Job& job	= jobs[i];
	job.title	= job_title;
	job.core	= this;
	
	return job;
}

Job& Core::GetJob(int i) {
	return jobs[i];
}

void Core::SetJobFinished(bool b) {
	if (!current_job)
		Panic("No job set currently");
	current_job->actual		= 1;
	current_job->total		= 1;
}

}
