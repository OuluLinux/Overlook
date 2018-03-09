#include "Overlook.h"

namespace Overlook {

bool AccountImage::LoadSources() {
	System& sys = GetSystem();
	
	if (jobs.IsEmpty()) {
		for(int i = 0; i < sys.jobs.GetCount(); i++) {
			Job& job = *sys.jobs[i];
			if (job.GetTf() == GetTf() && dynamic_cast<SourceImage*>(&job))
				jobs.Add(&job);
		}
	}
	
	ASSERT(jobs.GetCount());
	
	// Check that jobs are finished
	for(int i = 0; i < jobs.GetCount(); i++)
		if (!jobs[i]->IsFinished())
			return false;
	
	for(int i = 0; i < jobs.GetCount(); i++) {
		Job& job = *jobs[i];
		double result = job.GetBestResult();
		if (result < 1.1) {
			jobs.Remove(i);
			i--;
		}
	}
	
	if (current_state.IsEmpty()) {
		current_state.SetCount(jobs.GetCount());
		
		for(int i = 0; i < jobs.GetCount(); i++) {
			DataBridge& db = dynamic_cast<SourceImage&>(*jobs[i]).db;
			db.Start();
			current_state[i] = State(0, db.time[0], db.open[0], 0);
		}
	}
	
	if (dbs.IsEmpty()) {
		dbs.SetCount(jobs.GetCount());
		
		for(int i = 0; i < jobs.GetCount(); i++) {
			DataBridge& db = dynamic_cast<SourceImage&>(*jobs[i]).db;
			db.Start();
			dbs[i] = &db;
			ASSERT(db.GetTf() == GetTf());
		}
	}
	
	signals.row_size = jobs.GetCount() * 2;
	
	while (true) {
		int pos = this->gain.GetCount();
		int reserve_step = 100000;
		int reserve = pos - (pos % reserve_step) + reserve_step;
		signals.Reserve(reserve);
		signals.SetCount(pos+1);
		
		
		// Find next smallest time
		int smallest_time = INT_MAX;
		for(int i = 0; i < jobs.GetCount(); i++) {
			int count = dbs[i]->open.GetCount();
			int next = current_state[i].a + 1;
			if (next >= count) continue;
			int time = dbs[i]->time[next];
			if (time < smallest_time)
				smallest_time = time;
		}
		if (smallest_time == INT_MAX)
			break;
		
		// Increase cursor with all which has the same time next
		for(int i = 0; i < jobs.GetCount(); i++) {
			int count = dbs[i]->open.GetCount();
			int next = current_state[i].a + 1;
			if (next >= count) continue;
			int time = dbs[i]->time[next];
			if (time == smallest_time)
				current_state[i].a++;
		}
		
		for(int i = 0; i < jobs.GetCount(); i++) {
			Job& job = *jobs[i];
			DataBridge& db = *dbs[i];
			State& state = current_state[i];
			
			int src_pos = min(job.strand_data.GetCount() - 1, state.a);
			if (src_pos == -1) continue;
			bool signal = job.strand_data.Get(src_pos, 2);
			bool enabled = job.strand_data.Get(src_pos, 3);
			signals.Set(pos, i*2 + 0, signal);
			signals.Set(pos, i*2 + 1, enabled);
			
			int sig = enabled ? (signal ? -1 : +1) : 0;
			double open = db.open[state.a];
			
			int& prev_sig = state.d;
			
			double gain = balance;
			if (prev_sig == sig) {
				if (sig) {
					double change;
					if (sig > 0)	change = +(open / state.c - 1.0);
					else			change = -(open / state.c - 1.0);
					gain += change;
				}
			}
			else {
				if (prev_sig) {
					double change;
					if (sig > 0)	change = +(open / state.c - 1.0);
					else			change = -(open / state.c - 1.0);
					balance += change;
				}
				if (sig) {
					state.c = open;
				}
			}
			prev_sig = sig;
			ASSERT(gain > 0);
			
			this->gain.Add(gain);
		}
	}
	
	return true;
}

}
