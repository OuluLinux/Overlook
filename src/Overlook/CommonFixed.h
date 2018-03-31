#ifndef _Overlook_CommonFixed_h_
#define _Overlook_CommonFixed_h_

template <int I>
class FixedOnlineAverageWindow1 : Moveable<FixedOnlineAverageWindow1<I> > {
	static const int period = I;
	
	double win_a[I];
	double sum_a = 0.0;
	int cursor = 0;
	
public:
	FixedOnlineAverageWindow1() {}
	void Clear() {cursor = 0; sum_a = 0.0; for(int i = 0; i < I; i++) win_a[i] = 0.0;}
	void SetPeriod(int i) {period = i; win_a.SetCount(i,0);}
	void Add(double a) {
		double& da = win_a[cursor];
		sum_a -= da;
		da = a;
		sum_a += da;
		cursor = (cursor + 1) % period;
	}
	double GetMean() const {return sum_a / period;}
	void Serialize(Stream& s) {
		if (s.IsLoading())
			s.Get(this, sizeof(FixedOnlineAverageWindow1<I>));
		else
			s.Put(this, sizeof(FixedOnlineAverageWindow1<I>));
	}
};


template <int I>
struct FixedExtremumCache : Moveable<FixedExtremumCache<I> > {
	static const int size = I;
	
	double max[size], min[size];
	double max_value = -DBL_MAX, min_value = DBL_MAX;
	int pos = -1, max_left = 0, min_left = 0;
	
	FixedExtremumCache(int size=0) {
		for(int i = 0; i < I; i++) {
			max[i] = -DBL_MAX;
			min[i] = +DBL_MAX;
		}
	}
	
	void Serialize(Stream& s) {
		if (s.IsLoading())
			s.Get(this, sizeof(FixedExtremumCache<I>));
		else
			s.Put(this, sizeof(FixedExtremumCache<I>));
	}
	
	void Add(double low, double high) {
		pos++;
		int write_pos = pos % size;
		
		max_left--;
		max[write_pos] = high;
		if (max_left <= 0) {
			max_value = -DBL_MAX;
			for(int i = 0; i < size; i++) {
				double d = max[i];
				if (d > max_value) {
					if (i <= write_pos)	max_left = size - (write_pos - i);
					else				max_left = i - write_pos;
					max_value = d;
				}
			}
		}
		else if (high >= max_value) {
			max_left = size;
			max_value = high;
		}
		
		min_left--;
		min[write_pos] = low;
		if (min_left <= 0) {
			min_value = DBL_MAX;
			for(int i = 0; i < size; i++) {
				double d = min[i];
				if (d < min_value) {
					if (i <= write_pos)	min_left = size - (write_pos - i);
					else				min_left = i - write_pos;
					min_value = d;
				}
			}
		}
		else if (low <= min_value) {
			min_left = size;
			min_value = low;
		}
	}
	
	int GetHighest() {
		return pos - (size - max_left);
	}
	
	int GetLowest() {
		return pos - (size - min_left);
	}
};


#endif
