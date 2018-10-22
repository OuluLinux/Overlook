#ifndef _Forecaster_Core_h_
#define _Forecaster_Core_h_

namespace Forecast {

// Classes for IO arguments

struct Out : public ValueBase {
	Out(int count, int visible) {this->count = count; this->visible = visible; data_type = OUT_;}
};

struct Lbl : public ValueBase {
	Lbl(int count) {this->count = count; data_type = LBL_;}
};

struct Arg : public ValueBase {
	Arg(const char* key, int& value, int min, int max=10000) {s0 = key; data = &value; data_type = INT_; this->min = min; this->max = max;}
};

struct Persistent : public ValueBase {
	Callback1<Stream&> serialize;
	
protected:
	Persistent() {data_type = PERS_;}
	
public:
	Persistent(Callback1<Stream&> serialize) : serialize(serialize) {data_type = PERS_;}
	Persistent(const Persistent& src) {*this = src;}
	virtual ~Persistent() {}
	
	void operator = (const Persistent& base) {
		serialize = base.serialize;
		ValueBase::operator=(base);
	}
	
	virtual void Serialize(Stream& s) {serialize(s);}
};

struct StreamSerializer {
	typedef StreamSerializer CLASSNAME;
	template <class T> void ItemSerialize(Stream& s, T* obj) {s % *obj;}
	template <class T> Callback1<Stream&> GetSerializer(T& obj) {
		return THISBACK1(ItemSerialize<T>, &obj);
	}
};

inline StreamSerializer& GetStreamSerializer() {return Single<StreamSerializer>();}

template <class T> inline Persistent Mem(T& t) {
	return Persistent(GetStreamSerializer().GetSerializer(t));
}




struct ArgChanger : public ValueRegister {
	ArgChanger() : cursor(0), storing(0) {}
	
	virtual void IO(const ValueBase& base) {
		if (!storing) {
			keys.SetCount(cursor+1);
			keys[cursor] = base.s0;
			args.SetCount(cursor+1);
			if (base.data_type == ValueBase::INT_)
				args[cursor++] = *(int*)base.data;
		} else {
			if (base.data_type == ValueBase::INT_)
				*(int*)base.data = args[cursor++];
		}
	}
	void SetLoading() {storing = false; cursor = 0;}
	void SetStoring() {storing = true;  cursor = 0;}
	
	Vector<Value> args;
	Vector<String> keys;
	int cursor;
	bool storing;
};

struct OutputCounter : public ValueRegister {
	OutputCounter() {}
	
	virtual void IO(const ValueBase& base) {
		if (base.data_type == ValueBase::LBL_) {
			lbl_counts.Add(base.count);
			lbl_sum += base.count;
		}
		else if (base.data_type == ValueBase::OUT_) {
			out_counts.Add(base.count);
			out_sum += base.count;
		}
	}
	
	Vector<int> lbl_counts, out_counts;
	int lbl_sum = 0, out_sum = 0;
};



class Core {
	
protected:
	friend class System;
	
	
	int bars = 0;
	int counted = 0;
	Vector<Buffer> buffers;
	Vector<Label> labels;
	const Vector<double>* input = NULL;
	
public:
	Core() {}
	virtual ~Core() {}
	
	virtual void Init() {}
	virtual void Start() {}
	virtual void IO(ValueRegister& reg) {}
	
	void Refresh(bool run_start=true);
	void SetupBuffers();
	
	void SetBars(int i) {bars = i;}
	Buffer& GetBuffer(int buffer) {return buffers[buffer];}
	ConstLabelSignal& GetLabelBuffer(int lbl, int buf) const {return labels[lbl].buffers[buf];}
	LabelSignal& GetLabelBuffer(int lbl, int buf) {return labels[lbl].buffers[buf];}
	int GetBars() {return bars;}
	int GetCounted() {return counted;}
	
	double Open(int i) {return (*input)[i];}
	double Low(int i) {
		if (i == 0) return (*input)[i];
		return min((*input)[i], (*input)[i-1]);
	}
	double High(int i) {
		if (i == 0) return (*input)[i];
		return max((*input)[i], (*input)[i-1]);
	}
};



struct CoreItem : Moveable<CoreItem> {
	Core* core = NULL;
	
	CoreItem() {}
	~CoreItem() {if (core) {delete core; core = NULL;}}
	
	
};

}

#endif
