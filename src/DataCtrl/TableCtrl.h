#ifndef _DataCtrl_TableCtrl_h_
#define _DataCtrl_TableCtrl_h_

namespace DataCtrl {
using namespace DataCore;

class TableCtrl : public MetaNodeCtrl {
	ArrayCtrl ctrl;
	MetaVar data;
	
public:
	TableCtrl();
	
	virtual String GetKey() const {return "table";}
	static String GetKeyStatic()  {return "table";}
	
	void SetTable(MetaVar src);
	
	void RefreshData();
	
};

}


#endif
