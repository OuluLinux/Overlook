#ifndef _Overlook_QueryTable_h_
#define _Overlook_QueryTable_h_

// Theory: http://www.saedsayad.com/decision_tree.htm

namespace Overlook {


class QueryTable {
	struct Column : Moveable<Column> {
		Column() : bit_begin(0), bit_end(0) {}
		int bit_begin, bit_end, size;
		String name;
	};
	Vector<Vector<byte> > data;
	Vector<double> gains;
	Vector<Column> columns;
	String target_name;
	int bytes, bits;
	int target_count;
	
public:
	
	QueryTable();
	
	int Get(int row, int col) const {
		int out = 0;
		const Vector<byte>& vec = data[row];
		const Column& pred = columns[col];
		int byt = pred.bit_begin / 8;
		int bit = pred.bit_begin % 8;
		typedef const byte ConstByte;
		ConstByte* v = vec.Begin() + byt;
		for(int i = 0; i < pred.size; i++) {
			bool b = *v & (1 << bit);
			if (b)
				out |= 1 << i;
			bit++;
			if (bit == 8) {
				bit = 0;
				v++;
			}
		}
		return out;
	}
	
	void Set(int row, int col, int value) {
		Vector<byte>& vec = data[row];
		const Column& pred = columns[col];
		int byt = pred.bit_begin / 8;
		int bit = pred.bit_begin % 8;
		byte* v = vec.Begin() + byt;
		for(int i = 0; i < pred.size; i++) {
			bool b = value & (1 << i);
			if (!b)		*v &= ~(1 << bit);
			else		*v |=  (1 << bit);
			bit++;
			if (bit == 8) {
				bit = 0;
				v++;
			}
		}
	}
	
	void SetCount(int i) {
		int old_count = data.GetCount();
		data.SetCount(i);
		for(int i = old_count; i < data.GetCount(); i++) data[i].SetCount(bytes, 0);
	}
	
	Column& AddColumn(const String& name, int bits) {
		Column& p = columns.Add();
		p.name = name;
		p.bit_begin = this->bits;
		p.bit_end = p.bit_begin + bits;
		p.size = bits;
		this->bits += bits;
		this->bytes = this->bits / 8;
		if (this->bits % 8 != 0) this->bits++;
		return p;
	}
	
	//void AddValue(int i, const T& value) {columns[i].Add(value);}
	//void AddTargetValue(const T& value) {target.Add(value);}
	
	const Vector<double>& GetInfoGains() const {return gains;}
	int GetLargestInfoGainPredictor(int target_id);
	void EndTargets() {target_count = columns.GetCount();}
};


class QueryTableForecaster : public Core {
	QueryTable qt;
	
public:
	typedef QueryTableForecaster CLASSNAME;
	QueryTableForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf)
			//% InOptional(IndiPhase, RealIndicatorValue, SymTf)
			% Out(ForecastPhase, ForecastChangeValue, SymTf);
	}
};


class QueryTableHugeForecaster : public Core {
	
public:
	typedef QueryTableHugeForecaster CLASSNAME;
	QueryTableHugeForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, All)
			% Out(ForecastPhase, ForecastChangeValue, SymTf);
	}
};


class QueryTableMetaForecaster : public Core {
	
public:
	typedef QueryTableMetaForecaster CLASSNAME;
	QueryTableMetaForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf)
			% In(ForecastPhase, ForecastChangeValue, Sym)
			% In(IndiPhase, ForecastChannelValue, Sym)
			% InOptional(ForecastPhase, ForecastChangeValue, Sym)
			% InOptional(IndiPhase, IndicatorValue, Sym)
			% Out(ForecastCombPhase, ForecastChangeValue, SymTf);
	}
};


class QueryTableAgent : public Core {
	
public:
	typedef QueryTableAgent CLASSNAME;
	QueryTableAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf);
		reg % In(ForecastCombPhase, ForecastChangeValue, SymTf);
		reg % In(IndiPhase, ForecastChannelValue, SymTf);
		reg % In(IndiPhase, IdealOrderSignal, SymTf);
		reg % Out(AgentPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableHugeAgent : public Core {
	
public:
	typedef QueryTableHugeAgent CLASSNAME;
	QueryTableHugeAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, All);
		reg % In(ForecastCombPhase, ForecastChangeValue, All);
		reg % In(IndiPhase, ForecastChannelValue, All);
		reg % In(IndiPhase, IdealOrderSignal, All);
		reg % Out(AgentPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableMetaAgent : public Core {
	
public:
	typedef QueryTableMetaAgent CLASSNAME;
	QueryTableMetaAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, Sym);
		///reg % In(AgentPhase, ForecastOrderSignal, Sym);
		//reg % InOptional(AgentPhase, ForecastOrderSignal, Sym);
		reg % Out(AgentCombPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableDoubleAgent : public Core {
	
public:
	typedef QueryTableDoubleAgent CLASSNAME;
	QueryTableDoubleAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, All);
		reg % In(AgentCombPhase, ForecastOrderSignal, All);
		reg % Out(AgentCombPhase, ForecastOrderSignal, All);
	}
};


}


#endif
