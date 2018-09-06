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
		int net = -1, src, grade, abs_grade, len;
		double cdf, mean;
		bool is_secondary_inverse;
		
		bool operator() (const Temp& a, const Temp& b) const {return a.grade < b.grade;}
	};
	Temp temp;
	
	
	
protected:
	friend class Model;
	friend class ModelSystemCtrl;
	
	Vector<Vector<Vector<StatSlot> > > stats;
	Vector<double> history;
	String secondary_pattern;
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
	
	
	int GetSignalNet() const {return temp.net;}
	int GetSignal() const {return (is_inverse ? -1 : +1) * (is_signal ? -1 : +1) * (temp.is_secondary_inverse ? -1 : +1);}
	double GetAccountGain() const {return account_gain;}
	
};

class Model {
	
protected:
	friend class ModelSystem;
	friend class ModelSetting;
	friend class ModelSystemCtrl;
	
	Array<ModelSetting> settings;
	CoreList cores;
	double cur_gain = 0;
	int cur_sig = 0, cur_sig_net = -1;
	
public:
	Model();
	
	void Init();
	
	bool Tick(ModelSystem& msys);
	
	int GetSettingCount() const {return settings.GetCount();}
	const ModelSetting& GetSetting(int i) const {return settings[i];}
	
	
};

class ModelSystem : public Common {
	
protected:
	Array<Model> models;
	
	
public:
	typedef ModelSystem CLASSNAME;
	ModelSystem();
	
	virtual void Init();
	virtual void Start();
	
	int GetModelCount() {return models.GetCount();}
	const Model& GetModel(int i) const {return models[i];}
	
	/*
	void Serialize(Stream& s) {}
	void LoadThis() {LoadFromFile(*this, ConfigFile("ModelSystem.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("ModelSystem.bin"));}*/
	
};

inline ModelSystem& GetModelSystem() {return GetSystem().GetCommon<ModelSystem>();}


class ModelSystemCtrl : public CommonCtrl {
	ArrayCtrl list, valuelist;
	DropList symlist, modellist, mslist;
	
	struct Drawer : public Ctrl {
		int model = -1, msi = -1;
		Vector<Point> cache;
		virtual void Paint(Draw& d) {
			if (model == -1 || msi == -1) return;
			Size sz(GetSize());
			ImageDraw id(sz);
			id.DrawRect(sz, White());
			DrawVectorPolyline(id, sz, GetModelSystem().GetModel(model).GetSetting(msi).history, cache);
			d.DrawImage(0, 0, id);
		}
	};
	
	Drawer drawer;
	
public:
	typedef ModelSystemCtrl CLASSNAME;
	ModelSystemCtrl();
	
	virtual void Data();
	
};

}

#endif
