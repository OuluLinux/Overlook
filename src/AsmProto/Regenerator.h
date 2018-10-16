#ifndef _AsmProto_Regenerator_h_
#define _AsmProto_Regenerator_h_




enum {
	SEEKER_BWD,
	SEEKER_FWD,
	SEEKER_BWDOBJ,
	SEEKER_FWDOBJ,
	SEEKER_NEWOBJ,
	SEEKER_EDITOBJ,
	SEEKER_REMOBJ,
	SEEKER_OBJITER,
	SEEKER_ITEROBJ,
	SEEKER_RANDLOC,
	SEEKER_COUNT
};

enum {
	SSENS_DIFF,
	SSENS_ISOBJ,
	SSENS_VEL,
	SSENS_OBJVEL,
	SSENS_ITER,
	SSENS_OBJITER,
	SSENS_COUNT
};

enum {
	ADJ_INCLOW,
	ADJ_DECLOW,
	ADJ_INCHIGH,
	ADJ_DECHIGH,
	ADJ_INCMIN,
	ADJ_DECMIN,
	ADJ_INCMAX,
	ADJ_DECMAX,
	ADJ_INCSIZE,
	ADJ_DECSIZE,
	ADJ_TOGGLEACTION,
	ADJ_EXIT,
	ADJ_COUNT
};

enum {
	ASENS_MIDLOW,
	ASENS_MIDHIGH,
	ASENS_MIDPRICEDIFF,
	ASENS_MIN,
	ASENS_MAX,
	ASENS_SIZE,
	ASENS_ACTION,
	ASENS_COUNT
};


struct SeekerState {
	int iter = 0, speed = 0;
	int obj_iter = 0, snap_speed = 0;
};

struct Regenerator {
	
	static const int seeker_train_count = 10;
	
	// Persistent
	Generator gen;
	ConvNet::DQNAgent seeker, adjuster;
	SeekerState seeker_state;
	int trains_total = 0;
	
	
	// Temp
	Vector<double> slist;
	Vector<double> generated;
	Vector<double>* real_data = NULL;
	
	Regenerator();
	void Train();
	double TrainAdjuster();
	void GetSeekerSensors(Vector<double>& slist);
	void GetAdjusterSensors(Vector<double>& slist);
	double GetDataError();
	void ObjIterToIter();
	void IterToObjIter();
	
};



#endif
