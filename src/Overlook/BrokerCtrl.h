#ifndef _Overlook_BrokerCtrl_h_
#define _Overlook_BrokerCtrl_h_

namespace Overlook {

#define LAYOUTFILE <Overlook/BrokerCtrl.lay>
#include <CtrlCore/lay.h>

struct AssetGraphDislay : public Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q,
	                   Color ink, Color paper, dword style) const {
		w.DrawRect(r, paper);
		Rect g = r;
		g.top += 2;
		g.bottom -= 2;
		double d = q;
		if (d < 0) {
			Color clr = Color(135, 22, 0);
			g.left  += (int)(g.Width() * (1.0 + d));
			w.DrawRect(g, clr);
		} else {
			Color clr = Color(0, 134, 0);
			g.right -= (int)(g.Width() * (1.0 - d));
			w.DrawRect(g, clr);
		}
	}
};

class BrokerCtrl : public WithBrokerLayout<CustomCtrl> {
	Brokerage* broker;
	
	ArrayCtrl trade, history, exposure;
	ArrayMap<int, Order> orders;
	Vector<int> open_tickets;
	
public:
	typedef BrokerCtrl CLASSNAME;
	BrokerCtrl();
	
	void Refresher();
	void Reset();
	void Close();
	void CloseAll();
	void PriceCursor();
	
	void Init();
	void Data();
	
	void SetBroker(Brokerage& broker) {this->broker = &broker;}
	void OpenOrder(int type);
	void ReadOnly();
};

}

#endif
