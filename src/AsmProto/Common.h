#ifndef _AsmProto_Common_h_
#define _AsmProto_Common_h_


class OnlineAverageWindow1 : Moveable<OnlineAverageWindow1> {
	Vector<double> win_a;
	double sum_a = 0.0;
	int period = 0, cursor = 0;
	int count = 0;
	
public:
	OnlineAverageWindow1() {}
	void SetPeriod(int i) {period = i; win_a.SetCount(i,0);}
	void Add(double a) {
		double& da = win_a[cursor];
		sum_a -= da;
		da = a;
		sum_a += da;
		count++;
		cursor = (cursor + 1) % period;
	}
	double Top() const {return win_a[cursor];}
	double GetMean() const {if (!count) return 0; return sum_a / min(count, period);}
	int GetPeriod() const {return period;}
	void Serialize(Stream& s) {s % win_a % sum_a % period % cursor % count;}
	const Vector<double>& GetWindow() const {return win_a;}
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

inline int PopCount32(uint64 i) {
	#ifdef flagMSC
	return __popcnt(i);
	#else
	return __builtin_popcount(i);
	#endif
}



#endif
