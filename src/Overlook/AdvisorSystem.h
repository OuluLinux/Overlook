#ifndef _Overlook_AdvisorSystem_h_
#define _Overlook_AdvisorSystem_h_

namespace Overlook {
	
class AdvisorSystem : public Common {
	
protected:
	
	
public:
	typedef AdvisorSystem CLASSNAME;
	AdvisorSystem();
	
	virtual void Init();
	virtual void Start();
	
	
};

inline AdvisorSystem& GetAdvisorSystem() {return GetSystem().GetCommon<AdvisorSystem>();}


class AdvisorSystemCtrl : public CommonCtrl {
	ArrayCtrl list, valuelist;
	DropList symlist, modellist, mslist;
	
	/*struct Drawer : public Ctrl {
		int model = -1, msi = -1;
		Vector<Point> cache;
		virtual void Paint(Draw& d) {
			if (model == -1 || msi == -1) return;
			Size sz(GetSize());
			ImageDraw id(sz);
			id.DrawRect(sz, White());
			DrawVectorPolyline(id, sz, GetAdvisorSystem().GetModel(model).GetSetting(msi).history, cache);
			d.DrawImage(0, 0, id);
		}
	};*/
	
	//Drawer drawer;
	
public:
	typedef AdvisorSystemCtrl CLASSNAME;
	AdvisorSystemCtrl();
	
	virtual void Data();
	
};

}

#endif
