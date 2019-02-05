#ifndef _Overlook_Speculation_h_
#define _Overlook_Speculation_h_

namespace Overlook {

	
struct SpecItemData : Moveable<SpecItemData> {
	double slow_sum = 0, succ_sum = 0;
	
	void Serialize(Stream& s) {s % slow_sum % succ_sum;}
};

struct SpecItem : Moveable<SpecItem> {
	VectorMap<int, double> cur_score;
	Vector<int> prio_syms;
	Vector<SpecItemData> data;
	Vector<double> succ_tf;
	Vector<double> nn, bdnn;
	VectorBool values, avvalues, succ;
	
	void Serialize(Stream& s) {s % cur_score % prio_syms % data % succ_tf % nn % bdnn % values % avvalues % succ;}
	
	static const Color PositiveColor;
	static const Color NegativeColor;
	inline Color GetColor(bool b) {return b ? NegativeColor : PositiveColor;}
	inline Color GetGrayColor() {return GrayColor(128);}
};

struct SpecBarDataItem : Moveable<SpecBarDataItem> {
	static const int tf_count = 6;
	
	VectorMap<int, bool> syms;
	double open = 0, low = 0, high = 0, volume = 0;
	double spec = 0;
	bool signal;
	
	double slow_sum = 0, succ_sum = 0;
	bool values[tf_count], avvalues[tf_count], succ[tf_count];
	
	void Serialize(Stream& s) {s % syms % open % low % high % volume % spec % signal; s % slow_sum % succ_sum; for(int i = 0; i < tf_count; i++) {s % values[i] % avvalues[i] % succ[i];}}
};

struct SpecBarData : Moveable<SpecBarData> {
	VectorMap<int, Tuple2<bool, int> > length_sigs;
	Vector<SpecBarDataItem> data;
	OnlineAverage1 spec_av;
	int counted = 0, fast_counted = 0, spec_counted = 0, spec2_counted = 0;
	bool spec_signal = false;
	
	void Serialize(Stream& s) {s % length_sigs % data % spec_av % counted % fast_counted % spec_counted % spec2_counted % spec_signal;}
};

struct BarDataOscillator : Moveable<BarDataOscillator> {
	Vector<double> scores, invs;
	VectorMap<int, int> score_stats;
	Vector<int> score_levels;
	int fast_counted = 0;
	
	void Serialize(Stream& s) {s % scores % invs % score_stats % score_levels;}
};

class Speculation {
	
protected:
	friend class SpeculationCtrl;
	friend class SpeculationMatrixCtrl;
	friend class GlobalSuccessCtrl;
	friend class CandlestickCtrl;
	friend class ActiveSession;
	
	static const int cur_count = 8;
	static const int lvl_count = 3;
	static const int prio_count = 3;
	
	
	// Persistent
	ConvNet::Session ses, bdses;
	Vector<BarDataOscillator> bardataspecosc;
	Vector<Vector<SpecBarData> > bardata;
	Vector<SpecItem> data;
	
	
	// Temporary
	Vector<Vector<CoreList> > cl;
	Index<String> sym;
	Vector<int> lengths;
	Vector<int> aiv, biv;
	Vector<int> tfs;
	Vector<int> symposv;
	Vector<bool> is_abv;
	double max_sum = 0, max_succ_sum = 0;
	double bd_max_sum = 0, bd_max_succ_sum = 0;
	double cur_slow_max = 0, cur_succ_max = 0;
	double invosc_max = 0;
	int total = 1, actual = 0;
	int bdtotal = 1, bdactual = 0;
	ThreadSignal sig;
	TimeStop ts;
	
	inline static Color GetSuccessColor(bool b) {return b ? LtYellow : GrayColor(64);}
	inline static Color GetPaper(bool b) {return b ? Color(113, 212, 255) : Color(170, 255, 150);}
	void RefreshBarDataItemTf0(int length, int prio_id, bool inv, SpecBarData& sbd);
	void RefreshBarDataItemTf(int tfi, SpecBarData& sbd, SpecBarData& sbd0);
	void RefreshBarDataSpeculation(int tfi, SpecBarData& sbd);
	SpecBarData& GetSpecBarData(int tfi, int lengthi, int prio_id, bool inv) {return bardata[tfi][(lengthi * prio_count + prio_id) * 2 + inv];}
	void RefreshBarDataSpeculation2(int lengthi, int prio_id, int inv, SpecBarData& sbd);
	void GetSpecBarDataSpecArgs(int bdspeci, int& lengthi, int& prio_id, int& inv);
	void RefreshBarDataOscillatorTf0();
	void RefreshBarDataOscillator(int tfi);
	//inline double GetMult(int tfi) {return 1.0 + (double)(tfs.GetCount()-1-tfi) / (double)tfs.GetCount() * 3;}
	inline double GetMult(int tfi) {return 1.0;}
	void LoadVolume(int pos, ConvNet::Volume& vol);
	void LoadBarDataVolume(int pos, ConvNet::Volume& vol);
	
public:
	typedef Speculation CLASSNAME;
	Speculation();
	~Speculation() {sig.Stop();}
	
	void Process();
	void Data();
	void ProcessItem(int pos, SpecItem& si);
	void RefreshCores();
	void RefreshItems();
	void RefreshNeuralItems();
	void RefreshBarData();
	void RefreshBarDataSpec();
	void RefreshBarDataOscillators();
	void RefreshBarDataNeuralItems();
	
	void Serialize(Stream& s) {s % ses % bdses % bardataspecosc % bardata % data;}
	void LoadThis() {LoadFromFile(*this, GetOverlookFile("Speculation.bin"));}
	void StoreThis() {StoreToFile(*this, GetOverlookFile("Speculation.bin"));}
	
};

inline Speculation& GetSpeculation() {return Single<Speculation>();}



class ProgressDisplayCls2 : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword s) const;
};

Display& ProgressDisplay2();

class SuccessDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword s) const;
};

Display& SuccessDisplay();

class CodeDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword s) const;
};

Display& CodeDisplay();

struct SpeculationMatrixCtrl : public Ctrl {
	int shift = 0;
	
	virtual void Paint(Draw& d);
};

struct GlobalSuccessCtrl : public Ctrl {
	int shift = 0;
	
	virtual void Paint(Draw& d);
};

struct CandlestickCtrl : public Ctrl {
	Vector<double> opens, lows, highs, specs, scores, invs;
	Vector<bool> bools;
	int shift = 0, lengthi = 0, tfi = 0, prio_id = 0, inv = 0;
	int border = 5;
	int count = 100;
	int scoremin = 50;
	
	Vector<Point> cache;
	
	typedef CandlestickCtrl CLASSNAME;
	virtual void Paint(Draw& d);
	
	void Refresh0() {Refresh();}
	void Data();
};


class SpeculationCtrl : public SubWindowCtrl {
	TabCtrl tabs;
	SliderCtrl slider;
	Upp::Label time_lbl;
	
	Splitter matrix_tab;
	SpeculationMatrixCtrl matrix;
	GlobalSuccessCtrl succ_ctrl;
	ArrayCtrl list;
	ParentCtrl listparent;
	ProgressIndicator matrix_prog, bd_prog;
	
	Splitter perf_tab;
	ParentCtrl perf_parent;
	ArrayCtrl perf_list;
	CandlestickCtrl candles;
	DropList perf_tf;
	
	
public:
	typedef SpeculationCtrl CLASSNAME;
	SpeculationCtrl();
	
	virtual void Start();
	virtual void Data();
	virtual String GetTitle();
	
};

}

#endif
