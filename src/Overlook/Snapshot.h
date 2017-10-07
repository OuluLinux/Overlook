#ifndef _Overlook_Snapshot_h_
#define _Overlook_Snapshot_h_


class Snapshot : Moveable<Snapshot> {

private:
	IndicatorTuple sensors;
	ResultTuple result_bwd		[MEASURE_SIZE];
	uint8 result_cluster_id		[MEASURE_SIZE];
	uint8 indi_cluster_id;
	uint8 result_predicted				[RESULT_BYTES];
	uint8 result_predicted_targeting	[RESULT_BYTES];
	int16 result_predicted_volat		[RESULT_SIZE];
	int16 result_predicted_changed		[RESULT_SIZE];
	
	double open					[SYM_COUNT];
	double change				[SYM_COUNT];
	
	int signal_broker_symsig	[TRAINEE_COUNT];
	int amp_broker_symsig		[TRAINEE_COUNT];
	int shift;

	inline int GetSignalOutput(int i) const {
		ASSERT(i >= 0 && i < TRAINEE_COUNT);
		return signal_broker_symsig[i];
	}

	inline void SetSignalOutput(int i, int j) {
		ASSERT(i >= 0 && i < TRAINEE_COUNT);
		signal_broker_symsig[i] = j;
	}

	inline int GetAmpOutput(int i) const {
		ASSERT(i >= 0 && i < TRAINEE_COUNT);
		return amp_broker_symsig[i];
	}

	inline void SetAmpOutput(int i, int j) {
		ASSERT(i >= 0 && i < TRAINEE_COUNT);
		amp_broker_symsig[i] = j;
	}

public:
	void Reset() {
		for (int i = 0; i < TRAINEE_COUNT; i++)
			signal_broker_symsig[i] = 0;

		for (int i = 0; i < TRAINEE_COUNT; i++)
			amp_broker_symsig[i] = 0;
		
		for (int i = 0; i < MEASURE_SIZE; i++)
			result_cluster_id[i] = 0;
		
		indi_cluster_id = 0;
		
		for (int i = 0; i < RESULT_BYTES; i++)
			result_predicted[i] = 0;
		for (int i = 0; i < RESULT_BYTES; i++)
			result_predicted_targeting[i] = 0;
		for (int i = 0; i < RESULT_SIZE; i++)
			result_predicted_volat[i] = 0;
		for (int i = 0; i < RESULT_SIZE; i++)
			result_predicted_changed[i] = 0;
	}
	
	int GetResultCluster(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_cluster_id	[sym * MEASURE_PERIODCOUNT + tf];
	}
	
	void SetResultCluster(int sym, int tf, int cl) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		result_cluster_id	[sym * MEASURE_PERIODCOUNT + tf] = cl;
	}
	
	void SetResultClusterPredicted(int sym, int cl, bool b=true) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		int bit		= sym * OUTPUT_COUNT + cl;
		int byt		= bit / 8;
		bit			= bit % 8;
		if (b)
			result_predicted[byt] |= 1 << bit;
		else
			result_predicted[byt] &= ~(1 << bit);
	}
	
	bool IsResultClusterPredicted(int sym, int cl) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		int bit		= sym * OUTPUT_COUNT + cl;
		int byt		= bit / 8;
		bit			= bit % 8;
		return result_predicted[byt] & (1 << bit);
	}
	
	int GetIndicatorCluster() const {
		return indi_cluster_id;
	}
	
	void SetIndicatorCluster(int cl) {
		indi_cluster_id = cl;
	}
	
	bool IsResultClusterPredictedTarget(int sym, int cl) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		int bit		= sym * OUTPUT_COUNT + cl;
		int byt		= bit / 8;
		bit			= bit % 8;
		return result_predicted_targeting[byt] & (1 << bit);
	}
	
	void SetResultClusterPredictedTarget(int sym, int cl) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		int bit		= sym * OUTPUT_COUNT + cl;
		int byt		= bit / 8;
		bit			= bit % 8;
		result_predicted_targeting[byt] |= (1 << bit);
	}
	
	void SetResultClusterPredictedVolatiled(int sym, int cl, double volat) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		result_predicted_volat[sym * OUTPUT_COUNT + cl] = volat / VOLAT_DIV;
	}
	
	void SetResultClusterPredictedChanged(int sym, int cl, double change) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		result_predicted_changed[sym * OUTPUT_COUNT + cl] = change / CHANGE_DIV;
	}
	
	double GetResultClusterPredictedVolatiled(int sym, int cl) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		return result_predicted_volat[sym * OUTPUT_COUNT + cl] * VOLAT_DIV;
	}
	
	int GetResultClusterPredictedVolatiledInt(int sym, int cl) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		return result_predicted_volat[sym * OUTPUT_COUNT + cl];
	}
	
	double GetResultClusterPredictedChanged(int sym, int cl) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		return result_predicted_changed[sym * OUTPUT_COUNT + cl] * CHANGE_DIV;
	}
	
	int GetResultClusterPredictedChangedInt(int sym, int cl) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(cl  >= 0 && cl  < OUTPUT_COUNT);
		return result_predicted_changed[sym * OUTPUT_COUNT + cl];
	}
	
	const ResultTuple& GetResultTuple(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf];
	}
	
	const IndicatorTuple& GetIndicatorTuple() const {
		return sensors;
	}
	
	void   SetPeriodChange(int sym, int tf, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		result_bwd[sym * MEASURE_PERIODCOUNT + tf].change = d / CHANGE_DIV;
	}

	double GetPeriodChange(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf].change * CHANGE_DIV;
	}

	int    GetPeriodChangeInt(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf].change;
	}

	void   SetPeriodVolatility(int sym, int tf, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		result_bwd[sym * MEASURE_PERIODCOUNT + tf].volat = d / VOLAT_DIV;
	}

	double GetPeriodVolatility(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf].volat * VOLAT_DIV;
	}

	int    GetPeriodVolatilityInt(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf].volat;
	}
	
	inline double GetSensor(int sym, int in) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(in >= 0 && in < INPUT_SENSORS);
		return sensors.values[TIME_SENSORS + sym * INPUT_SENSORS + in] / 255.0;
	}

	inline void SetSensor(int sym, int in, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(in >= 0 && in < INPUT_SENSORS);
		sensors.values[TIME_SENSORS + sym * INPUT_SENSORS + in] = Upp::max(0, Upp::min(255, (int)(d * 255.0)));
	}

	inline double GetSensorUnsafe(int i) const {
		return sensors.values[i] / 255.0;
	}

	inline double GetTimeSensor(int i) const {
		ASSERT(i >= 0 && i < TIME_SENSORS);
		return sensors.values[i] / 255.0;
	}
	
	inline void SetTimeSensor(int i, double d) {
		ASSERT(i >= 0 && i < TIME_SENSORS);
		sensors.values[i] = Upp::max(0, Upp::min(255, (int)(d * 255.0)));
	}
	
	inline double GetOpen(int sym) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		return open[sym];
	}

	inline void   SetOpen(int sym, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		open[sym] = d;
	}

	inline double GetChange(int sym) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		return change[sym];
	}

	inline void   SetChange(int sym, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		change[sym] = d;
	}


	inline int GetSignalOutput(int group, int sym) const {
		ASSERT(group >= 0 && group < GROUP_COUNT);
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		return GetSignalOutput(group * SYM_COUNT + sym);
	}

	inline void SetSignalOutput(int group, int sym, int i) {
		ASSERT(group >= 0 && group < GROUP_COUNT);
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		SetSignalOutput(group * SYM_COUNT + sym, i);
	}

	inline int GetAmpOutput(int group, int sym) const {
		ASSERT(group >= 0 && group < GROUP_COUNT);
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		return GetAmpOutput(group * SYM_COUNT + sym);
	}

	inline void SetAmpOutput(int group, int sym, int i) {
		ASSERT(group >= 0 && group < GROUP_COUNT);
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		SetAmpOutput(group * SYM_COUNT + sym, i);
	}


	int GetShift() const {
		return shift;
	}

	void SetShift(int shift) {
		this->shift = shift;
	}

};

#endif
