#ifndef _Overlook_Net_h_
#define _Overlook_Net_h_

namespace Overlook {

class NetPressureSolver {
	
protected:
	friend class NetCtrl;
	
	Vector<double> pressures, solved_pressures;
	double scale = 1;
	
	
	static const int MAX_ALLSYMBOLS = 380;
	static const int MAX_VARIANTSYMBOLS = 74;
	static const int MAX_REALSYMBOLS = 380;
	static const int MAX_VARIANTPAIRS = 5402;
	
	struct VariantPair : Moveable<VariantPair> {
		int arbitrage_count = 0;
		bool xtrade = 0;
		double xposition = 0;
		
		VariantPair() {}
		VariantPair(const VariantPair& s) {*this = s;}
		void operator=(const VariantPair& s) {
			arbitrage_count = s.arbitrage_count;
			xtrade = s.xtrade;
			xposition = s.xposition;
		}
	};
	
	struct VariantSym : Moveable<VariantSym> {
		double bid = 0, ask = 0;
		int math = 0, all_sym = 0;
		
		VariantSym() {}
		VariantSym(const VariantSym& s) {*this = s;}
		void operator=(const VariantSym& s) {
			bid = s.bid;
			ask = s.ask;
			math = s.math;
			all_sym = s.all_sym;
		}
	};
	
	struct Symbol : Moveable<Symbol> {
		Vector<VariantPair> pair;
		Vector<VariantSym> sym;
		
		double point = 0.0;
		double min_pips = 0.0;
		
		Symbol() {}
		Symbol(const Symbol& s) {*this = s;}
		void operator=(const Symbol& s) {
			pair <<= s.pair;
			sym <<= s.sym;
			point = s.point;
			min_pips = s.min_pips;
		}
	};
	
	VectorMap<String, int> spreads;
	Vector<Symbol> symbols;
	Vector<String> currencies, real_symbols;
	String str_out;
	double min_pips = -3;
	double bids_real[MAX_REALSYMBOLS];
	double asks_real[MAX_REALSYMBOLS];
	double position[MAX_REALSYMBOLS];
	double prev_position[MAX_REALSYMBOLS];
	double lots = 1;
	double ALPHA = 0.001;
	int max_arbitrage_count = 0;
	int real_sym_count = 0;
	bool running = false, stopped = true;
	
	
public:
	typedef NetPressureSolver CLASSNAME;
	NetPressureSolver();
	
	void SetPressure(int i, double d) {pressures[i] = d;}
	void SetScale(double d) {scale = fabs(d);}
	double GetPressure(int i) const {return pressures[i];}
	double GetSolvedPressure(int i) const {return solved_pressures[i];}
	int GetSolvedPressureCount() const {return solved_pressures.GetCount();}
	int GetScale() const {return scale;}
	
	void Solve();
	
	void Init();
	void RandomPressure();
	void Process();
	void GetRealSymbols();
	bool IsRealSymbol(const String& s);
	int  GetNumIsRealSymbol(const String& s) {return GetSystem().FindSymbol(s);}
	void GetAllSymbols();
	void GetPipsD();
	void PrintBeginInfo();
	void TradeArbitrage();
	void OpenArbitragePosition(int sym_id, int variant1_id, int variant2_id, double Vol);
	String SymbolToStr(int i, int j);
	String SymbolToFile(int i, int j);
	String ArbitragePositions();
	void RefreshPositions();
	void MonitoringArbitrage(int sym_id, int variant1_id, int variant2_id);
	void OpenSymbolPosition(int sym_id, int Variant, int Type, double Vol);
	void SymbolDone(double Vol, int Symb);
	void GetBidAsk(int i, int j);
	void WriteStatistic(String FileName);
	void GetRealBidAsk();
	
};

class Net {
	
	
public:
	typedef Net CLASSNAME;
	Net();
	
	
};

inline Net& GetNet() {return Single<Net>();}


class NetCircle : public Ctrl {
	enum {MODE_PRES, MODE_SOLVEDPRES, MODE_DIFF, MODE_COUNT};
	
	int mode = 0;
	
public:
	NetCircle();
	
	virtual void Paint(Draw& w);
	virtual void LeftDown(Point p, dword keyflags);

	
	NetPressureSolver* solver = NULL;
};

class NetCtrl : public SubWindowCtrl {
	TabCtrl tabs;
	
	// Testing mode
	ParentCtrl testctrl;
	NetCircle testcircle;
	SliderCtrl testslider;
	DropList testsymbol;
	ArrayCtrl testarbitrage, testsiglist;
	NetPressureSolver testsolver;
	Option testrandompres;
	
public:
	typedef NetCtrl CLASSNAME;
	NetCtrl();
	
	virtual void Start() {Data();}
	virtual void Data();
	
	void SetSymbol();
	void SetPressure();
	
};

}

#endif
