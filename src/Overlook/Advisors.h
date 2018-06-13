#ifndef _Overlook_Advisors_h_
#define _Overlook_Advisors_h_

namespace Overlook {

/*
	MyFxBook module shows, that top performing account there is:
		RAPIER by algo.land https://www.myfxbook.com/members/Megabot/rapier-by-algoland/1657111
		(2h-2d orders, when only signal is used with minimum lots)
	It works by expecting the price to return to the channel break price.
	
	This is scalper version from it. It expects price to return to the channel break price,
	but only when it is very close to it already.
*/

class RapierishAdvisor : public Core {
	
	struct Setting : Moveable<Setting> {
		int a, b, c, d, e;
		
		void Serialize(Stream& s) {s % a % b % c % d % e;}
	};
	
	
	struct TrainingCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	static const int sign_max = 20;
	static const int input_size = sign_max * 2;
	
	// Persistent
	Vector<double> training_pts;
	Vector<int> cursors;
	Setting best_setting;
	double total = 0;
	int round = 0;
	int prev_counted = 0;
	
	
	// Temporary
	Vector<Setting> test_settings;
	double point = 0.0001;
	int max_rounds = 0;
	bool once = true;
	bool do_test = false;
	
	
protected:
	virtual void Start();
	
	bool TrainingBegin();
	bool TrainingIterator();
	bool TrainingEnd();
	bool TrainingInspect();
	void RefreshAll();
	double TestSetting(Setting& setting, bool write_signal);
	
public:
	typedef RapierishAdvisor CLASSNAME;
	RapierishAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Out(0, 0)
			% Mem(training_pts)
			% Mem(cursors)
			% Mem(best_setting)
			% Mem(total)
			% Mem(round)
			% Mem(prev_counted);
	}
	
};




class MultiTfAdvisor : public Core {
	
protected:
	virtual void Start();
	
public:
	typedef MultiTfAdvisor CLASSNAME;
	MultiTfAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<RapierishAdvisor>(&Filter0)
			% Out(1, 1)
			% Out(0, 0);
	}
	
	static bool Filter0(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1) {
			int period = GetSystem().GetPeriod(out_tf);
			return /*period == 60 || period == 240 ||*/ period == 1;
		}
		else {
			return in_sym == out_sym;
		}
	}
};




}

#endif
