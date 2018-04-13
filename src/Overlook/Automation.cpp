#include "Overlook.h"

namespace Overlook {

Automation::Automation() {
	memset(this, 0, sizeof(Automation));
	ASSERT(sym_count > 0);
	
	output_fmlevel = 0.6;
	
	if (FileExists(ConfigFile("Automation.bin")))
		LoadThis();
	
	for(int i = 0; i < sym_count; i++) {
		slow[i].sym = i;
		slow[i].tf = 4;
		slow[i].period = 60;
		slow[i].running = &running;
		for(int j = 0; j < sym_count; j++) {
			slow[i].other_open_buf[j] = slow[j].open_buf;
		}
	}
	
	not_stopped = 0;
}

Automation::~Automation() {
	
}

int Automation::GetSymGroupJobId(int symbol) const {
	for(int i = 0; i < GROUP_COUNT; i++) {
		const JobGroup& jobgroup = jobgroups[i];
		const Job& job = jobgroup.jobs[symbol];
		if (!job.is_finished)
			return i;
	}
	return GROUP_COUNT;
}

void Automation::StartJobs() {
	if (running || not_stopped > 0) return;
	
	running = true;
	int thrd_count = GetUsedCpuCores();
	not_stopped = 0;
	for(int i = 0; i < thrd_count; i++) {
		not_stopped++;
		Thread::Start(THISBACK1(JobWorker, i));
	}
}

void Automation::StopJobs() {
	
	running = false;
	while (not_stopped > 0) Sleep(100);
	
	StoreThis();
}

void Automation::Serialize(Stream& s) {
	if (s.IsLoading()) {
		s.Get(this, sizeof(Automation));
	} else {
		s.Put(this, sizeof(Automation));
	}
}

void Automation::JobWorker(int i) {
	
	while (running) {
		
		int group_id = 0;
		for (;group_id < jobgroup_count; group_id++)
			if (!jobgroups[group_id].is_finished)
				break;
		if (group_id >= jobgroup_count) {
			for(int i = 0; i < GROUP_COUNT; i++) {
				jobgroups[i].is_finished = false;
				for(int j = 0; j < sym_count; j++)
					jobgroups[i].jobs[j].is_finished = false;
			}
			continue;
		}
		
		JobGroup& jobgroup = jobgroups[group_id];
		
		workitem_lock.Enter();
		bool found = false;
		int job_id;
		for(int i = 0; i < sym_count; i++) {
			job_id = worker_cursor++;
			if (worker_cursor >= sym_count)
				worker_cursor = 0;
			Job& job = jobgroup.jobs[job_id];
			if (job.is_finished || job.is_processing)
				continue;
			job.is_processing = true;
			found = true;
			break;
		}
		
		if (!found) {
			bool all_finished = true;
			for(int i = 0; i < sym_count; i++)
				all_finished &= jobgroup.jobs[i].is_finished;
			if (all_finished) {
				jobgroup.is_finished = true;
			}
		}
		
		workitem_lock.Leave();
		
		if (!found) {
			Sleep(100);
			continue;
		}
		
		Process(group_id, job_id);
		jobgroups[group_id].jobs[job_id].is_processing = false;
		
	}
	
	
	
	not_stopped--;
}

void Automation::Process(int group_id, int job_id) {
	JobGroup& jobgroup	= jobgroups[group_id];
	Job& job			= jobgroup.jobs[job_id];
	
	if (group_id == GROUP_SOURCE) {
		if (job_id == 0)
			LoadSource();
		else
			Sleep(1000);
	}
	else if (group_id == GROUP_BITS) {
		slow[job_id].ProcessBits();
	}
	else if (group_id == GROUP_EVOLVE) {
		slow[job_id].Evolve();
	}
	
	if (running) job.is_finished = true;
}

void Automation::LoadSource() {
	System& sys = GetSystem();
	Vector<int>& used_symbols_id = sys.used_symbols_id;
	int tf = slow[0].tf;
	
	for(int i = 0; i < used_symbols_id.GetCount(); i++) {
		int sym = used_symbols_id[i];
		sys.data[sym][0].db.Start();
		sys.data[sym][tf].db.Start();
	}
	
	while (running) {
		
		// Find next smallest time
		int smallest_time = INT_MAX;
		int smallest_ids[sym_count];
		int smallest_id_count = 0;
		int largest_time = 0;
		for(int i = 0; i < sym_count; i++) {
			int sym = used_symbols_id[i];
			DataBridge& db = sys.data[sym][tf].db;
			int count = db.open.GetCount();
			int next = slow[i].loadsource_pos + 1;
			if (next >= count) continue;
			
			int time = db.time[next];
			if (time < smallest_time) {
				smallest_time = time;
				smallest_id_count = 1;
				smallest_ids[0] = i;
			}
			else if (time == smallest_time) {
				smallest_ids[smallest_id_count++] = i;
			}
			
			int prev_cursor = slow[i].loadsource_cursor - 1;
			if (prev_cursor >= 0) {
				time = slow[i].time_buf[prev_cursor];
				if (time > largest_time) {
					largest_time = time;
				}
			}
		}
		if (smallest_time == INT_MAX)
			break;
		
		
		// Increase cursor with all which has the same next time
		for(int i = 0; i < smallest_id_count; i++) {
			slow[smallest_ids[i]].loadsource_pos++;
		}
		
		bool dec_cursor = smallest_time == largest_time;
		
		for(int i = 0; i < sym_count; i++) {
			if (dec_cursor)
				slow[i].loadsource_cursor--;
			
			int sym = used_symbols_id[i];
			DataBridge& db = sys.data[sym][tf].db;
			int pos = slow[i].loadsource_pos;
			//LOG(loadsource_cursor << "\t" << i << "\t" << pos << "/" << db.open.GetCount());
			double open = db.open[pos];
			slow[i].open_buf[slow[i].loadsource_cursor] = open;
			slow[i].time_buf[slow[i].loadsource_cursor] = smallest_time;
			
			slow[i].loadsource_cursor++;
			if (slow[i].loadsource_cursor >= slow[i].maxcount)
				Panic("Reserved memory exceeded");
		}
		
	}
	
	for(int i = 0; i < sym_count; i++) {
		int sym = used_symbols_id[i];
		DataBridge& db = sys.data[sym][tf].db;
		slow[i].point = db.GetPoint();
		slow[i].spread = db.GetSpread();
		if (slow[i].point <= 0) Panic("Invalid point");
		if (slow[i].spread < slow[i].point * 2)
			slow[i].spread = slow[i].point * 2;
		ReleaseLog("Automation::LoadSource sym " + IntStr(sym) + " point " + DblStr(slow[i].point) + " spread " + DblStr(slow[i].spread));
	}
}

}
