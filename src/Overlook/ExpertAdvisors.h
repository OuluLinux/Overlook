#ifndef _Overlook_ExpertAdvisors_h_
#define _Overlook_ExpertAdvisors_h_

namespace Overlook {
using namespace Upp;


struct AccuracyConf : Moveable<AccuracyConf> {
	
	// Distinctive attributes
	int id = 0;
	int symbol = 0;
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
		ch << id << 1 << symbol << 1 << label_id << 1 << period << 1
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
	
	bool operator==(const AccuracyConf& src) const {
		return	id				== src.id &&
				symbol			== src.symbol &&
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
		symbol			= src.symbol;
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
		  % symbol
		  % label_id
		  % period
		  % ext
		  % label
		  % fastinput
		  % labelpattern
		  % ext_dir;
		
		s % stat
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
	
	typedef Tuple<AccuracyConf, RandomForestMemory, double> RF;
	enum {RF_IDLE, RF_OPTIMIZING, RF_IDLEREAL, RF_TRAINREAL, RF_REAL};
	
	Array<RF> rflist_pos, rflist_neg;
	BufferRandomForest rf_trainer;
	int opt_counter = 0;
	int phase = RF_IDLE;
	
protected:
	virtual void Start();
	
public:
	typedef RandomForestAdvisor CLASSNAME;
	RandomForestAdvisor();
	
	virtual void Init();
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(LOCALPROB_DEPTH*2+1, LOCALPROB_DEPTH*2+1);
	}
	
	void Optimize();
	void TrainReal();
	
	
};


}

#endif
