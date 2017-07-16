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
	Time time, added;
	int shift;
	
	Snapshot() : shift(-1) {}
	
};

class AgentGroup;

struct TraineeBase {
	// Persistent
	ConvNet::DQNAgent dqn;
	Vector<double> seq_results;
	double peak_value;
	double best_result;
	double training_time;
	int iter;
	
	// Temp
	AgentGroup* group;
	Vector<double> thrd_equity;
	SimBroker broker;
	TimeStop ts;
	double prev_equity;
	double prev_reward;
	double begin_equity;
	int epoch_actual, epoch_total;
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG, ACT_RESETSIG,     ACTIONCOUNT};
	
	TraineeBase();
	void Init();
	void Create();
	void Action();
	void Serialize(Stream& s);
	virtual void Forward(Snapshot& snap, Brokerage& broker, Snapshot* next_snap) = 0;
	virtual void Backward(double reward) = 0;
	virtual void SetAskBid(SimBroker& sb, int pos) = 0;
	const Vector<double>& GetSequenceResults() const {return seq_results;}
	
	

};

}

#endif
