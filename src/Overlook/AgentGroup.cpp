#include "Overlook.h"

namespace Overlook {


AgentGroup::AgentGroup() {
	running = false;
	stopped = true;
}

AgentGroup::~AgentGroup() {
	Stop();
	StoreThis();
}

void AgentGroup::Init() {
	
	// Tf group multiplier
	
	
	// Reserve agents
	
	
}

void AgentGroup::Start() {
	Stop();
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Main));
}

void AgentGroup::Stop() {
	running = false;
	while (stopped != true) Sleep(100);
}

void AgentGroup::Main() {
	if (agents.size() == 0 || sym_ids.IsEmpty() || indi_ids.IsEmpty()) return;
	
	RefreshSnapshots();
	
	std::vector<Snap> snaps;
	
	
	while (running) {
		if (phase == PHASE_SEEKSNAPS) {
			
			// Seek different snapshot dataset, because GPU memory is limited.
			
			phase = PHASE_TRAINING;
		}
		else if (phase == PHASE_TRAINING) {
			array_view<Snap, 1>  snap_view(snaps.size(), snaps);
			array_view<Agent, 1> agents_view(agents.size(), agents);
			
			while (phase == PHASE_TRAINING && running) {
				parallel_for_each(agents_view.extent, [=](index<1> idx) restrict(amp)
			    {
			        Agent& agent = agents_view[idx];
			        
			        Snap& snap = snap_view[4];
			        snap.sensor[4] = 4;
			        
			        double eps = agent.GetEpsilon();
			        
			        
			        // Run something like previous Agent::Main
			        
			    });
			}
			
			agents_view.synchronize();
		}
		else if (phase == PHASE_WEIGHTS) {
			
			
			// Weight single group (21 pairs)
			
		}
		else if (phase == PHASE_FINAL) {
			
			
			// Weight group joiners to final output
			
		}
		else if (phase == PHASE_UPDATE) {
			
			RefreshSnapshots();
			
			// Updates latest snapshot signals
			
		}
		else if (phase == PHASE_REAL) {
			
			
			// Updates broker
			
		}
		else if (phase == PHASE_WAIT) {
			
			
			// Changes to PHASE_UPDATE when time to update
			
		}
		else Sleep(100);
	}
	
	stopped = true;
}

void AgentGroup::LoadThis() {
	LoadFromFile(*this,	ConfigFile(name + ".agrp"));
}

void AgentGroup::StoreThis() {
	ASSERT(!name.IsEmpty());
	Time t = GetSysTime();
	String file = ConfigFile(name + ".agrp");
	bool rem_bak = false;
	if (FileExists(file)) {
		FileMove(file, file + ".bak");
		rem_bak = true;
	}
	StoreToFile(*this,	file);
	if (rem_bak)
		DeleteFile(file + ".bak");
}

void AgentGroup::Serialize(Stream& s) {
	/*TraineeBase::Serialize(s);
	s % go % agents % tf_limit % tf_ids % sym_ids % created % name % param_str
	  % fmlevel % limit_factor
	  % group_input_width % group_input_height
	  % mode
	  % enable_training;*/
}

void AgentGroup::SetEpsilon(double d) {
	for(int i = 0; i < agents.size(); i++)
		agents[i].SetEpsilon(d);
}

void AgentGroup::SetMode(int i) {
	
}

void AgentGroup::RefreshSnapshots() {
	
}

void AgentGroup::Progress(int actual, int total, String desc) {
	a0 = actual;
	t0 = total;
	prog_desc = desc;
	WhenProgress(actual, total, desc);
	SubProgress(0, 1);
}

void AgentGroup::SubProgress(int actual, int total) {
	a1 = actual;
	t1 = total;
	WhenSubProgress(actual, total);
}



}

