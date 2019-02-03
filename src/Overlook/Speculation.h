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
	VectorBool values, avvalues, succ;
	int tf_begin = 0, tf_end = 1, data_i = 0;
	
	void Serialize(Stream& s) {s % cur_score % prio_syms % data % succ_tf % values % avvalues % succ % tf_begin % tf_end % data_i;}
	
	inline Color GetColor(int tf, bool b) {if (tf < tf_begin || tf >= tf_end) return b ? Color(56, 127, 255) : Color(28, 212, 0); else return b ? Color(28, 212, 255) : Color(28, 255, 0);}
	inline Color GetGrayColor(int tf) {return (tf < tf_begin || tf >= tf_end) ? GrayColor(128) : GrayColor(128+64);}
};

struct SpecBarDataItem : Moveable<SpecBarDataItem> {
	VectorMap<int, bool> syms;
	double open = 0, low = 0, high = 0, volume = 0;
	double spec = 0;
	bool signal;
	
	void Serialize(Stream& s) {s % syms % open % low % high % volume % spec % signal;}
};

struct SpecBarData : Moveable<SpecBarData> {
	VectorMap<int, Tuple2<bool, int> > length_sigs;
	Vector<SpecBarDataItem> data;
	OnlineAverage1 spec_av;
	int counted = 0, fast_counted = 0, spec_counted = 0;
	bool spec_signal = false;
	
	void Serialize(Stream& s) {s % length_sigs % data % spec_av % counted % fast_counted % spec_counted % spec_signal;}
};

struct SpecBarDataSpecData : Moveable<SpecBarDataSpecData> {
	static const int tf_count = 6;
	
	double slow_sum = 0, succ_sum = 0;
	bool values[tf_count], avvalues[tf_count], succ[tf_count];
	
	void Serialize(Stream& s) {s % slow_sum % succ_sum; for(int i = 0; i < tf_count; i++) {s % values[i] % avvalues[i] % succ[i];}}
};

struct SpecBarDataSpec : Moveable<SpecBarDataSpec> {
	Vector<SpecBarDataSpecData> data;
	
	void Serialize(Stream& s) {s % data;}
};

struct BarDataOscillator : Moveable<BarDataOscillator> {
	Vector<double> scores, invs;
	VectorMap<int, int> score_stats;
	Vector<int> score_levels;
	int fast_counted = 0;
	
	void Serialize(Stream& s) {s % scores % invs % score_stats % score_levels;}
};

struct TraderItemData : Moveable<TraderItemData> {
	double open = 0, low = 0, high = 0;
	int bdspeci = -1;
	
	void Serialize(Stream& s) {s % open % low % high % bdspeci;}
};

struct TraderItem : Moveable<TraderItem> {
	Vector<Vector<TraderItemData> > data;
	Vector<int> fast_counted;
	int cur_begin = 0, cur_bdspeci = -1;
	int comb, scorelvl, tfi;
	bool reqav, reqsucc;
	
	void Serialize(Stream& s) {s % data % fast_counted % cur_begin % cur_bdspeci % comb % scorelvl % tfi % reqav % reqsucc;}
};

class Speculation {
	
protected:
	friend class SpeculationCtrl;
	friend class SpeculationMatrixCtrl;
	friend class GlobalSuccessCtrl;
	friend class CandlestickCtrl;
	friend class CandlestickCtrl2;
	
	static const int cur_count = 8;
	static const int lvl_count = 3;
	
	
	// Persistent
	ConvNet::Session ses;
	Vector<BarDataOscillator> bardataspecosc;
	Vector<TraderItem> traders;
	Vector<SpecBarDataSpec> bardataspec;
	Vector<SpecBarData> bardata;
	Vector<SpecItem> data;
	
	
	// Temporary
	Vector<Vector<CoreList> > cl;
	Index<String> sym;
	Vector<double> max_sum, max_succ_sum;
	Vector<int> lengths;
	Vector<int> aiv, biv;
	Vector<int> sizev, startv;
	Vector<int> tfs;
	double bd_max_sum = 0, bd_max_succ_sum = 0;
	double cur_slow_max = 0, cur_succ_max = 0;
	double invosc_max = 0;
	ThreadSignal sig;
	TimeStop ts;
	
	inline static Color GetSuccessColor(bool b) {return b ? LtYellow : GrayColor(64);}
	inline static Color GetPaper(bool b) {return b ? Color(113, 212, 255) : Color(170, 255, 150);}
	void RefreshBarDataItemTf0(int comb, int length, int prio_id, bool inv, SpecBarData& sbd);
	void RefreshBarDataItemTf(int tfi, SpecBarData& sbd, SpecBarData& sbd0);
	void RefreshBarDataSpeculation(int tfi, SpecBarData& sbd);
	SpecBarData& GetSpecBarData(int tfi, int comb, int lengthi, int prio_id, bool inv) {return bardata[(((tfi * startv.GetCount() + comb) * lengths.GetCount() + lengthi) * 3 + prio_id) * 2 + inv];}
	void RefreshBarDataSpeculation2(int comb, int lengthi, int prio_id, int inv, SpecBarDataSpec& sbds);
	SpecBarDataSpec& GetSpecBarDataSpec(int comb, int lengthi, int prio_id, bool inv) {return bardataspec[((comb * lengths.GetCount() + lengthi) * 3 + prio_id) * 2 + inv];}
	void RefreshTrader(TraderItem& ti);
	void RefreshTrader(int tfi, TraderItem& ti);
	void GetSpecBarDataSpecArgs(int bdspeci, int& comb, int& lengthi, int& prio_id, int& inv);
	void RefreshBarDataOscillatorTf0();
	void RefreshBarDataOscillator(int tfi);
	inline double GetMult(int tfi) {return 1.0 + (double)(tfs.GetCount()-1-tfi) / (double)tfs.GetCount() * 3;}
	
public:
	typedef Speculation CLASSNAME;
	Speculation();
	~Speculation() {sig.Stop();}
	
	void Process();
	void Data();
	void ProcessItem(int pos, SpecItem& si);
	void RefreshCores();
	void RefreshItems();
	void RefreshBarData();
	void RefreshBarDataSpec();
	void RefreshBarDataOscillators();
	void RefreshTraders();
	
	void Serialize(Stream& s) {s % bardataspecosc % traders % bardataspec % bardata % data;}
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
	int shift = 0, comb = 0, lengthi = 0, tfi = 0, prio_id = 0, inv = 0;
	int border = 5;
	int count = 100;
	int scoremin = 50;
	
	Vector<Point> cache;
	
	typedef CandlestickCtrl CLASSNAME;
	virtual void Paint(Draw& d);
	
	void Refresh0() {Refresh();}
	void Data();
};

struct CandlestickCtrl2 : public Ctrl {
	Vector<double> opens, lows, highs;
	int trader = 0;
	int shift = 0;
	int border = 5;
	int count = 100;
	int tfi = 0;
	
	Vector<Point> cache;
	
	typedef CandlestickCtrl2 CLASSNAME;
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
	
	Splitter perf_tab;
	ParentCtrl perf_parent;
	ArrayCtrl perf_list;
	CandlestickCtrl candles;
	DropList perf_tf;
	
	Splitter trader_tab;
	ArrayCtrl trader_list;
	ParentCtrl trader_parent;
	CandlestickCtrl2 trader_candles;
	DropList trader_tf;
	
	
public:
	typedef SpeculationCtrl CLASSNAME;
	SpeculationCtrl();
	
	virtual void Start();
	virtual void Data();
	virtual String GetTitle();
	
};

}

#endif
