#ifndef _Overlook_ExpertAdvisors_h_
#define _Overlook_ExpertAdvisors_h_

namespace Overlook {
using namespace Upp;


struct AccuracyConf : Moveable<AccuracyConf> {
	
	// Distinctive attributes
	int id = 0;
	int label_id = 0;
	int period = 0;
	int ext = 0;
	int label = 0;
	int fastinput = 0;
	int labelpattern = 0;
	bool ext_dir = false;
	
	// Training stats
	RandomForestStat stat;
	double test_valuefactor = 0.0, test_valuehourfactor = 0.0, test_hourtotal = 0.0;
	double av_hour_change = 0.0;
	double largemult_count = 0.0, largemult_frac = 0.0;
	bool is_processed = false;
	
	
	AccuracyConf() {}
	AccuracyConf(const AccuracyConf& conf) {*this = conf;}
	
	uint32 GetHashValue() const {
		CombineHash ch;
		ch << id << 1 << label_id << 1 << period << 1
		   << ext << 1 << label << 1 << fastinput << 1 << labelpattern << 1 << (int)ext_dir << 1;
		return ch;
	}
	
	int GetBaseTf(int i) const {
		int tf = 0;
		if (!ext_dir)	tf = period + i;
		else			tf = period + ext-1 - i;
		ASSERT(tf >= 0 && tf < TF_COUNT);
		return tf;
	}
	
	void Print(ArrayCtrlPrinter& printer) const {
		printer.Title("Label");
		stat.Print(printer);
		
		printer.Title("Multiplier");
		printer.Add("av_hour_change", av_hour_change);
		printer.Add("largemult_count", largemult_count);
		printer.Add("largemult_frac", largemult_frac);
	}
	
	int Compare(const AccuracyConf& conf) const {
		Panic("TODO");
		return 0;
	}
	
	bool operator==(const AccuracyConf& src) const {
		return	id				== src.id &&
				label_id		== src.label_id &&
				period			== src.period &&
				ext				== src.ext &&
				label			== src.label &&
				fastinput		== src.fastinput &&
				labelpattern	== src.labelpattern &&
				ext_dir			== src.ext_dir;
	}
	
	void operator=(const AccuracyConf& src) {
		id				= src.id;
		label_id		= src.label_id;
		period			= src.period;
		ext				= src.ext;
		label			= src.label;
		fastinput		= src.fastinput;
		labelpattern	= src.labelpattern;
		ext_dir			= src.ext_dir;
		
		stat						= src.stat;
		test_valuefactor			= src.test_valuefactor;
		test_valuehourfactor		= src.test_valuehourfactor;
		test_hourtotal				= src.test_hourtotal;
		av_hour_change				= src.av_hour_change;
		largemult_count				= src.largemult_count;
		largemult_frac				= src.largemult_frac;
		is_processed				= src.is_processed;
	}
	void Serialize(Stream& s) {
		s % id
		  % label_id
		  % period
		  % ext
		  % label
		  % fastinput
		  % labelpattern
		  % ext_dir
		
		  % stat
		  % test_valuefactor
		  % test_valuehourfactor
		  % test_hourtotal
		  % av_hour_change
		  % largemult_count
		  % largemult_frac
		  % is_processed;
	}
};



class RandomForestAdvisor : public Core {
	
	struct SourceSearchCtrl : public JobCtrl {
		virtual void Paint(Draw& w);
	};
	
	struct SourceTrainingCtrl : public JobCtrl {
		virtual void Paint(Draw& w);
	};
	
	struct MainOptimizationCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	
	typedef Tuple<AccuracyConf, RandomForestMemory, VectorBool> RF;
	struct RFSorter {bool operator()(const RF& a, const RF& b) const {if (a.c.GetCount() == 0) return false; else return a.a.test_valuehourfactor > b.a.test_valuehourfactor;}};
	
	
	// Persistent
	Array<RF> rflist_pos, rflist_neg;
	BufferRandomForest rf_trainer;
	GeneticOptimizer optimizer;
	Vector<double> search_pts, training_pts, optimization_pts;
	double area_change_total[3];
	int prev_counted = 0;
	int opt_counter = 0;
	int p = 0, rflist_iter = 0;
	
	
	// Temp
	Vector<double> trial;
	VectorBool full_mask;
	One<RF> training_rf;
	ForestArea area;
	ConstBuffer* open_buf = NULL;
	double spread_point = 0.0;
	int conf_count = 0;
	int data_count = 0;
	
	
protected:
	virtual void Start();
	
	bool SourceSearchBegin();
	bool SourceSearchIterator();
	bool SourceSearchInspect();
	bool MainTrainingBegin();
	bool MainTrainingIterator();
	bool MainTrainingEnd();
	bool MainTrainingInspect();
	bool MainOptimizationBegin();
	bool MainOptimizationIterator();
	bool MainOptimizationEnd();
	bool MainOptimizationInspect();
	void RefreshMainBuffer(bool forced);
	void RunMain();
	void SetTrainingArea();
	void SetRealArea();
	void FillBufferSource(const AccuracyConf& conf, ConstBufferSource& bufs);
	void FillMainBufferSource(ConstBufferSource& bufs);
	void RefreshOutputBuffers();
	void RefreshMain();
	void RefreshAll();
	
public:
	typedef RandomForestAdvisor CLASSNAME;
	RandomForestAdvisor();
	
	virtual void Init();
	
	const int main_graphs = 1;
	const int indi_count = 3, label_count = 3;
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>();
		
		for(int i = 0; i < TF_COUNT; i++) {
			reg % In<VolatilityAverage>(&Args)
				% In<OsMA>(&Args)
				% In<StochasticOscillator>(&Args)
			
				% In<ZigZag>(&Args)
				% In<MovingAverage>(&Args)
				% In<Momentum>(&Args);
		}
		
		reg % In<LinearWeekTime>()
			% In<MinimalLabel>()
			% Out(main_graphs + LOCALPROB_DEPTH*2, main_graphs + LOCALPROB_DEPTH*2)
			% Mem(rflist_pos)
			% Mem(rflist_neg)
			% Mem(rf_trainer)
			% Mem(optimizer)
			% Mem(search_pts)
			% Mem(training_pts)
			% Mem(optimization_pts)
			% Mem(area_change_total[0]) % Mem(area_change_total[1]) % Mem(area_change_total[2])
			% Mem(prev_counted)
			% Mem(opt_counter)
			% Mem(p)
			% Mem(rflist_iter);
	}
	
	static void Args(int input, FactoryDeclaration& decl, const Vector<int>& args) {
		int pshift = (input - 1) / TF_COUNT;
		int type   = (input - 1) % TF_COUNT;
		int period = (1 << (1 + pshift));
		
		// Inspect IO: VolatilityAverage etc...
		if      (type == 0) {
			decl.AddArg(period);
		}
		else if (type == 1) {
			decl.AddArg(period);
			decl.AddArg(period*2);
			decl.AddArg(period);
		}
		else if (type == 2) {
			decl.AddArg(period);
		}
		else if (type == 3) {
			decl.AddArg(period);
			decl.AddArg(period);
			decl.AddArg(Upp::max(1, period/2));
		}
		else if (type == 4) {
			decl.AddArg(period);
			decl.AddArg(-period/2 - 1);
		}
		else if (type == 5) {
			decl.AddArg(period);
			decl.AddArg(-period/2);
		}
	}
	
	
};


}

#endif
