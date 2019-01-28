#ifndef _Overlook_Speculation_h_
#define _Overlook_Speculation_h_

namespace Overlook {

struct SpecItemData : Moveable<SpecItemData> {
	double slow_sum = 0, succ_sum = 0;
	
	void Serialize(Stream& s) {s % slow_sum % succ_sum;}
};

struct SpecItem : Moveable<SpecItem> {
	Vector<SpecItemData> data;
	Vector<double> succ_tf;
	Vector<bool> values, avvalues, succ;
	int tf_begin = 0, tf_end = 1, data_i = 0;
	
	void Serialize(Stream& s) {s % data % succ_tf % values % avvalues % succ % tf_begin % tf_end % data_i;}
	
	inline Color GetColor(int tf, bool b) {if (tf < tf_begin || tf >= tf_end) return b ? Color(56, 127, 255) : Color(28, 212, 0); else return b ? Color(28, 212, 255) : Color(28, 255, 0);}
	inline Color GetGrayColor(int tf) {return (tf < tf_begin || tf >= tf_end) ? GrayColor(128) : GrayColor(128+64);}
};

class Speculation {
	
protected:
	friend class SpeculationCtrl;
	friend class SpeculationMatrixCtrl;
	friend class GlobalSuccessCtrl;
	
	static const int cur_count = 8;
	
	
	// Persistent
	Vector<SpecItem> data;
	
	
	// Temporary
	Vector<Vector<CoreList> > cl;
	Index<String> sym;
	Vector<double> max_sum, max_succ_sum;
	Vector<int> aiv, biv;
	Vector<int> sizev, startv;
	Vector<int> tfs;
	ThreadSignal sig;
	
	inline static Color GetSuccessColor(bool b) {return b ? LtYellow : GrayColor(64);}
	inline static Color GetPaper(bool b) {return b ? Color(113, 212, 255) : Color(170, 255, 150);}
	
	
public:
	typedef Speculation CLASSNAME;
	Speculation();
	~Speculation() {sig.Stop();}
	
	void Process();
	void Data();
	void ProcessItem(int pos, SpecItem& si);
	void RefreshCores();
	
	void Serialize(Stream& s) {s % data;}
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

struct SpeculationMatrixCtrl : public Ctrl {
	int shift = 0;
	
	virtual void Paint(Draw& d);
};

struct GlobalSuccessCtrl : public Ctrl {
	int shift = 0;
	
	virtual void Paint(Draw& d);
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
	
	
public:
	typedef SpeculationCtrl CLASSNAME;
	SpeculationCtrl();
	
	virtual void Start();
	virtual void Data();
	virtual String GetTitle();
	
};

}

#endif
