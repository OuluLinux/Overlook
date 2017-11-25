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



#define DEBUG_BUFFERS 1

class DqnAdvisor : public Core {
	
	struct SourceSearchCtrl : public JobCtrl {
		virtual void Paint(Draw& w);
	};
	
	struct TrainingRFCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	struct TrainingDQNCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	
	typedef Tuple<AccuracyConf, RandomForestMemory, VectorBool> RF;
	struct RFSorter {bool operator()(const RF& a, const RF& b) const {if (a.c.GetCount() == 0) return false; else return a.a.test_valuehourfactor > b.a.test_valuehourfactor;}};
	
	enum {ACTION_LONG, ACTION_SHORT, ACTION_IDLE, ACTION_COUNT};
	typedef DQNTrainer<ACTION_COUNT, LOCALPROB_DEPTH * 2*2 + FEEDBACK_PERIOD*3, 100> DQN;
	
	
	// Persistent
	Array<RF>					rflist_pos;
	Array<RF>					rflist_neg;
	Vector<DQN::DQItem>			data;
	BufferRandomForest			rf_trainer;
	DQN							dqn_trainer;
	Vector<double>				search_pts;
	Vector<double>				training_pts;
	Vector<double>				dqntraining_pts;
	int							prev_counted	= 0;
	int							opt_counter		= 0;
	int							p				= 0;
	int							rflist_iter		= 0;
	int							dqn_round		= 0;
	int							dqn_max_rounds	= 5000000;
	int							dqn_pt_cursor	= 0;
	
	
	// Temp
	VectorBool					full_mask;
	One<RF>						training_rf;
	ForestArea					area;
	ConstBuffer*				open_buf		= NULL;
	double						spread_point	= 0.0;
	int							conf_count		= 0;
	int							data_count		= 0;
	bool						once			= true;
	
	
protected:
	virtual void Start();
	
	bool SourceSearchBegin();
	bool SourceSearchIterator();
	bool SourceSearchInspect();
	bool TrainingRFBegin();
	bool TrainingRFIterator();
	bool TrainingRFEnd();
	bool TrainingRFInspect();
	bool TrainingDQNBegin();
	bool TrainingDQNIterator();
	bool TrainingDQNEnd();
	bool TrainingDQNInspect();
	void RunMain();
	void SetTrainingArea();
	void SetRealArea();
	void FillBufferSource(const AccuracyConf& conf, ConstBufferSource& bufs);
	void RefreshOutputBuffers();
	void RefreshMain();
	void RefreshAll();
	void RefreshFeedback(int data_pos);
	
public:
	typedef DqnAdvisor CLASSNAME;
	DqnAdvisor();
	
	virtual void Init();
	
	const int main_graphs = 2;
	const int indi_count = 3, label_count = 3;
	
	#if DEBUG_BUFFERS
	const int buffer_count = main_graphs + LOCALPROB_DEPTH*2;
	#else
	const int buffer_count = main_graphs;
	#endif
	
	double GetSpreadPoint() const {return spread_point;}
	
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
			% Out(buffer_count, buffer_count)
			% Out(0, 0)
			% Mem(rflist_pos)
			% Mem(rflist_neg)
			% Mem(data)
			% Mem(rf_trainer)
			% Mem(dqn_trainer)
			% Mem(search_pts)
			% Mem(training_pts)
			% Mem(dqntraining_pts)
			% Mem(prev_counted)
			% Mem(opt_counter)
			% Mem(p)
			% Mem(rflist_iter)
			% Mem(dqn_round)
			% Mem(dqn_max_rounds)
			% Mem(dqn_pt_cursor);
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
