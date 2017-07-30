#ifndef _Overlook_Trainee_h_
#define _Overlook_Trainee_h_

namespace Overlook {

struct TfSymAverage {
	Vector<double> data;
	Vector<double> tf_sums;
	Vector<int> tf_periods;
	int pos;
	int bars;
	
	TfSymAverage() : pos(-1), bars(-1) {}
	
	void Reset(int bars) {
		this->bars = bars;
		pos = 0;
		data.SetCount(0);
		tf_sums.SetCount(0);
		data.SetCount(bars, 0.0);
		tf_sums.SetCount(tf_periods.GetCount(), 0.0);
	}
	void SeekNext() {
		if (pos >= bars) return;
		double value = data[pos];
		for(int j = 0; j < tf_periods.GetCount(); j++) {
			int prev = pos - tf_periods[j];
			if (prev >= 0)
				tf_sums[j] -= data[prev];
			tf_sums[j] += value;
		}
		pos++;
	}
	void Set(double value) {
		data[pos] = value;
	}
	double Get(int tf) {
		return tf_sums[tf] / tf_periods[tf];
	}
};

struct Snapshot : Moveable<Snapshot> {
	Vector<double> values;
	Vector<int> time_values;
	Vector<char> signals;
	Time time, added;
	int shift, tfs_used, id;
	uint16 tfmask;
	
	Snapshot() : shift(-1), id(-1), tfmask(0) {}
	
	bool IsActive(int tf_id) const {return tfmask & (1 << tf_id);}
	void SetActive(int tf_id) {tfmask |= (1 << tf_id);}
};

class AgentGroup;

struct TraineeBase {
	// Persistent
	Vector<double> seq_results;
	OnlineAverage1 reward_average, loss_average;
	double peak_value;
	double best_result;
	double training_time;
	double last_drawdown;
	int data_begin;
	int tf_id, tf;
	int group_id;
	int iter;
	
	// Temp
	AgentGroup* group;
	Vector<double> thrd_equity;
	SimBroker broker;
	TimeStop ts;
	double begin_equity, prev_equity;
	int epoch_actual, epoch_total;
	int main_id;
	bool at_main, save_epoch;
	bool data_looped_once;
	
	TraineeBase();
	void Init();
	void Action();
	void SeekActive();
	void Serialize(Stream& s);
	virtual void Create(int width, int height) = 0;
	virtual void Forward(Snapshot& snap, SimBroker& broker) = 0;
	virtual void Backward(double reward) = 0;
	virtual void SetAskBid(SimBroker& sb, int pos) = 0;
	const Vector<double>& GetSequenceResults() const {return seq_results;}
	
};

}

#endif
