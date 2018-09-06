#ifndef _Overlook_ModelSystem_h_
#define _Overlook_ModelSystem_h_

namespace Overlook {

class ModelSystem;
class Model;

struct StatSlot : Moveable<StatSlot> {
	
	OnlineVarianceWindow av, abs_av;
	
	void SetPeriod(int i) {av.SetPeriod(i); abs_av.SetPeriod(i);}
};

class ModelSetting {
	
	struct Temp : Moveable<Temp> {
		int net = -1, src, grade, abs_grade;
		double cdf, mean;
		
		bool operator() (const Temp& a, const Temp& b) const {return a.grade < b.grade;}
	};
	Temp temp;
	
	
	
protected:
	friend class Model;
	
	Vector<Vector<Vector<StatSlot> > > stats;
	double spread_factor = 0.0003;
	double grade_div = 0.05;
	double account_gain = 1.0;
	int cursor = 0;
	int event_count = -1;
	int primary_pattern_id = -1;
	int secondary_pattern_id = -1;
	int grade_count = 2;
	int id = -1;
	bool is_inverse = false, is_signal = false;
	
	static const int primary_pattern_count = 4;
	static const int secondary_pattern_count = 4;
	
public:
	ModelSetting();
	
	void Init(Model& m);
	bool Tick(ModelSystem& msys, Model& m);
	
	
	
};

class Model {
	
protected:
	friend class ModelSystem;
	friend class ModelSetting;
	
	Array<ModelSetting> settings;
	CoreList cores;
	
public:
	Model();
	
	void Init();
	
	bool Tick(ModelSystem& msys);
	
	
};

class ModelSystem : public Common {
	
protected:
	Array<Model> models;
	
	
public:
	typedef ModelSystem CLASSNAME;
	ModelSystem();
	
	virtual void Init();
	virtual void Start();
	
	
	
	/*
	void Serialize(Stream& s) {}
	void LoadThis() {LoadFromFile(*this, ConfigFile("ModelSystem.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("ModelSystem.bin"));}*/
	
};

inline ModelSystem& GetModelSystem() {return GetSystem().GetCommon<ModelSystem>();}


class ModelSystemCtrl : public CommonCtrl {
	
public:
	typedef ModelSystemCtrl CLASSNAME;
	ModelSystemCtrl();
	
	virtual void Data();
	
};

}

#endif
