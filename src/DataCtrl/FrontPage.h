#ifndef _DataCtrl_FrontPage_h_
#define _DataCtrl_FrontPage_h_

#include <CtrlLib/CtrlLib.h>
#include <RefCore/RefCore.h>

namespace DataCtrl {
using namespace Upp;
using namespace RefCore;

class FrontPage;

class FrontPageDraw : public Ctrl {
	FrontPage* fp;
	Image bg;
	
public:
	FrontPageDraw(FrontPage* fp);
	
	virtual void Paint(Draw& d);
	
	Image AdviceList(int cx, int cy);
	
	void MakeBackground();
	bool HasCacheFile(String path);
};

class FrontPage : public MetaNodeCtrl {
	int id;
	String symbol;

protected:
	friend class FrontPageDraw;
	
	FrontPageDraw draw;
	
public:
	FrontPage();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "fp";}
	static String GetKeyStatic()  {return "fp";}
	
	String GetSymbol() const {return symbol;}
	
};

}

#endif
