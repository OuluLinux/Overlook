#include "Agent.h"


namespace Agent {

Looper::Looper() {
	
}

void Looper::Init(double point, const Vector<double>& real_data) {
	this->point = point;
	
}

void Looper::Run() {
	bool is_open = false;
	int signal = 0;
	double open, volume;
	equity = 0.0;
	double prev_equity = equity;
	for(int i = 0; i < real_data->GetCount(); i++) {
		double o = (*real_data)[i];
		
		int act = 0;
		
		if ((*is_corner)[i]) {
			int match_pos = (*matches)[i];
			
			double d1 = (*real_data)[match_pos];
			double d0 = (*real_data)[match_pos+15];
			double diff = d0 - d1;
			act = diff < 0 ? -1 : 1;
		}
		
		if (!is_open && act) {
			is_open = true;
			signal = act;
			open = o;
			volume = 0.01;
		}
		else if (is_open && act) {
			if (act == signal) {
				if (signal == +1)	equity += (o / open - 1.0) * volume * 1000;
				else				equity -= (o / open - 1.0) * volume * 1000;
				if (equity < prev_equity)
					volume *= 2.0;
				open = o;
			}
			else {
				if (signal == +1)	equity += (o / (open * 1.0003) - 1.0) * volume * 1000;
				else				equity -= (o / (open * 0.9997) - 1.0) * volume * 1000;
				signal = act;
				volume = 0.01;
			}
		}
		if (prev_equity != equity) {
			LOG(equity);
			prev_equity = equity;
		}
	}
	LOG("");
}



}
