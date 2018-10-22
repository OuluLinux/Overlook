#ifndef _Overlook_CompatAMP_h_
#define _Overlook_CompatAMP_h_


#if defined flagMSC && defined flagWIN32 && !defined flagFORCE_COMPAT_AMP
	#define PARALLEL restrict(cpu,amp)
	#define HAVE_SYSTEM_AMP
	#include <amp.h>
	#include <amp_math.h>
	#include <cstdlib>

#define AMPASSERT(x)
#define AMPASSERT_(x, y)

inline float AmpTanh(float d) PARALLEL {return ::concurrency::fast_math::tanh(d);}


inline int GetAmpDeviceMemory() {
	concurrency::accelerator defdev;
	if (defdev.is_emulated)
		return 1024*1000*1000;
	return defdev.get_dedicated_memory() * 1000;
}

inline String GetAmpDevices() {
	String out;
	
	std::vector<concurrency::accelerator> accls = concurrency::accelerator::get_all();
	
	if (accls.empty()) {
		out << "No accelerators found that are compatible with C++ AMP";
		return out;
	}
	
	out << "Show all AMP Devices (";
	#if defined(_DEBUG)
	out << "DEBUG";
	#else
	out << "RELEASE";
	#endif
	out <<  " build)\n";
	
	out << "Found " << accls.size()
		<< " accelerator device(s) that are compatible with C++ AMP:\n";
	
	/*if (old_format) {
		std::for_each(accls.cbegin(), accls.cend(), [=, &n] (const concurrency::accelerator& a) {
			out << "  " << ++n << ": " << a.description
				<< ", has_display=" << (a.has_display ? "true" : "false")
				<< ", is_emulated=" << (a.is_emulated ? "true" : "false") << "\n";
		});
		out << "\n";
		return out;
	}*/
	
	for(int i = 0; i < accls.size(); i++) {
		concurrency::accelerator& a = accls[i];
		out << "  " << i << ": " << a.description.c_str() << " "
			<< "\n       device_path                       = " << a.device_path.c_str()
			<< "\n       dedicated_memory                  = " << DblStr((a.dedicated_memory) / (1024.0f * 1024.0f)) << " Gb"
			<< "\n       has_display                       = " << (a.has_display ? "true" : "false")
			<< "\n       is_debug                          = " << (a.is_debug ? "true" : "false")
			<< "\n       is_emulated                       = " << (a.is_emulated ? "true" : "false")
			<< "\n       supports_double_precision         = " << (a.supports_double_precision ? "true" : "false")
			<< "\n       supports_limited_double_precision = " << (a.supports_limited_double_precision ? "true" : "false")
			<< "\n";
	}
	out << "\n";
	return out;
}

/*inline int AmpAtomicSet(int& i, int value) PARALLEL {return concurrency::atomic_exchange(&i, value);}
inline int AmpAtomicInc(int& i) PARALLEL {return concurrency::atomic_fetch_inc(&i);}
inline int AmpAtomicDec(int& i) PARALLEL {return concurrency::atomic_fetch_dec(&i);}*/

#else
	#define PARALLEL

#define AMPASSERT(x) ASSERT(x)
#define AMPASSERT_(x, y) ASSERT_(x, y)

namespace concurrency {

template <int I> struct index {
	typedef index<I> idx;
	
	int i;
	
	index(int i) : i(i) {}
	index(const idx& src) : i(src.i) {}
	int operator[] (int i) const {AMPASSERT(i == 0); return this->i;}
};

template <int I> struct extent {
	typedef extent<I> ext;
	
	int i;
	
	extent(int i) : i(i) {AMPASSERT(I == 1);}
	extent(const ext& src) : i(src.i) {AMPASSERT(I == 1);}
	int operator[] (int i) const {AMPASSERT(i == 0); return this->i;}
	int size() const {return i;} // total size of elements
};

template <class T, int I> struct array_view {
	typedef array_view<T,I> thiscls;
	T* data;
	int count;

	array_view(int count, T* data) : data(data), count(count), extent(count) {
		
	}
	
	array_view(const thiscls& src) : extent(src.extent) {
		data = src.data;
		count = src.count;
	}
	
	T& operator[] (index<1> idx) const {
		int i = idx.i;
		ASSERT(i >= 0 && i < count);
		return data[i];
	}
	
	T& operator[] (int i) const {
		ASSERT(i >= 0 && i < count);
		return data[i];
	}
	
	int size() const {return count;}
	
	void synchronize() {}
	void discard_data() const {}
	
	extent<I> extent;
};

template <class T, int I> struct array {
	typedef array<T,I> thiscls;
	T* data;
	int count;

	array(int count, T* data) : data(data), count(count), extent(count) {
		
	}
	
	array(const thiscls& src) : extent(src.extent) {
		data = src.data;
		count = src.count;
	}
	
	T& operator[] (index<1> idx) const {
		int i = idx.i;
		ASSERT(i >= 0 && i < count);
		return data[i];
	}
	
	T& operator[] (int i) const {
		ASSERT(i >= 0 && i < count);
		return data[i];
	}
	
	int size() const {return count;}
	
	void synchronize() {}
	void discard_data() const {}
	
	extent<I> extent;
};

struct critical_section {
	Mutex m;
	void lock() {m.Enter();}
	void unlock() {m.Leave();}
};

template <class T, class CB> void parallel_for_each(T extent, CB cb) {
	CoWork co;
	co.SetPoolSize(Upp::max(1, CPU_Cores() - 2));
	for(int i = 0; i < extent.i; i++) {
		co & [=] {
			cb(index<1>(i));
		};
	}
	co.Finish();
}

inline void TestCompatAMP() {
	Vector<int> ints;
	for(int i = 0; i < 16; i++) ints.Add(1000 + i);
	
	array_view<int, 1>  ints_view(ints.GetCount(), ints.Begin());
	
	parallel_for_each(ints_view.extent, [=](index<1> idx) PARALLEL
    {
        LOG("View: " << (int)idx[0] << ",\tValue: " << ints_view[idx]);
    });
    
    ints_view.synchronize();
}

inline int GetAmpDeviceMemory() {return 1024*1000*1000;}
inline String GetAmpDevices() {return "Fake AMP";}
inline double AmpTanh(double d) PARALLEL {return tanh(d);}

inline int AmpAtomicSet(int& i, int value) {
	static Mutex l;
	l.Enter();
	int j = i;
	i = value;
	l.Leave();
	return j;
}

inline int AmpAtomicInc(int& i, int value) {
	static Mutex l;
	l.Enter();
	double j = i++;
	l.Leave();
	return j;
}

inline int AmpAtomicDec(int& i, int value) {
	static Mutex l;
	l.Enter();
	double j = i--;
	l.Leave();
	return j;
}

struct runtime_exception {
	String msg;
	
	runtime_exception(String msg) : msg(msg) {}
	String what() {return msg;}
};

}

#endif

#endif
