#ifndef _AsmProto_Common_h_
#define _AsmProto_Common_h_


template <int max_win_size>
class OnlineAverageWindow1 {
	double win_a[max_win_size];
	double sum_a = 0.0;
	int period = 0, cursor = 0;
	int count = 0;
	
public:
	OnlineAverageWindow1() {}
	void SetPeriod(int i) PARALLEL {cursor = 0; period = i; for(int i = 0; i < max_win_size; i++) win_a[i] = 0;}
	void Add(double a) PARALLEL {
		double& da = win_a[cursor];
		sum_a -= da;
		da = a;
		sum_a += da;
		count++;
		cursor = (cursor + 1) % period;
		AMPASSERT(cursor >= 0 && cursor < period);
	}
	double Top() const PARALLEL {return win_a[cursor];}
	double GetMean() const PARALLEL { if (!count) return 0; return sum_a / (count < period ? count : period);}
	int GetPeriod() const PARALLEL {return period;}
	void Serialize(Stream& s) {s % win_a % sum_a % period % cursor % count;}
};


inline int PopCount64(uint64 i) {
	#ifdef flagMSC
	#if CPU_64
	return __popcnt64(i);
	#elif CPU_32
	return __popcnt(i) + __popcnt(i >> 32);
	#endif
	#else
	return __builtin_popcountll(i);
	#endif
}

inline int PopCount32(uint32 i) {
	#ifdef flagMSC
	#if CPU_64
	return __popcnt64(i);
	#elif CPU_32
	return __popcnt(i);
	#endif
	#else
	return __builtin_popcountl(i);
	#endif
}

#endif
