#ifndef _Overlook_DqnAutomation_h_
#define _Overlook_DqnAutomation_h_

namespace Overlook {
using namespace libmt;


class DqnAutomation : public Common {
	
protected:
	friend class DqnAutomationCtrl;
	
	struct SymStat : Moveable<SymStat> {
		double open = 0;
		double lots = 0;
		int type = 0;
	};
	
	// Persistent
	ConvNet::DQNAgent dqn;
	Vector<double> training_pts;
	int iter = 0;
	
	// Temporary
	Vector<SymStat> symstats;
	Vector<double> points;
	Vector<double> slist;
	CoreList cl_net, cl_sym;
	int max_rounds = 0;
	double equity = 0, balance = 0;
	
	
	static const int sym_count = 10;
	static const int input_length = 1;
	static const int input_count = sym_count * input_length;
	static const int output_count = sym_count*2+1;
	
	
	void ResetOrders(int pos);
	void RefreshEquity(int pos);
	void RefreshInput(int pos);
	void Signal(int pos, int op, int net);
	void LoadThis() {LoadFromFile(*this, GetOverlookFile("DqnAutomation.bin"));}
	void StoreThis() {StoreToFile(*this, GetOverlookFile("DqnAutomation.bin"));}
	
public:
	typedef DqnAutomation CLASSNAME;
	DqnAutomation();
	
	virtual void Init();
	virtual void Start();
	void Process();
	
	void Serialize(Stream& s) {s % dqn % training_pts % iter;}
};

inline DqnAutomation& GetDqnAutomation() {return GetSystem().GetCommon<DqnAutomation>();}

class DqnAutomationCtrl : public CommonCtrl {
	
	struct DqnCtrl : public Ctrl {
		Vector<Point> cache;
		virtual void Paint(Draw& d) {
			d.DrawRect(GetSize(), White());
			DrawVectorPolyline(d, GetSize(), GetDqnAutomation().training_pts, cache);
		}
	};
	
	DqnCtrl ctrl;
	
public:
	typedef DqnAutomationCtrl CLASSNAME;
	DqnAutomationCtrl();
	
	virtual void Data();
};


}

#endif
