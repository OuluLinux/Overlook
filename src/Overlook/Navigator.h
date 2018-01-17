#ifndef _Overlook_Navigator_h_
#define _Overlook_Navigator_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

namespace Overlook {

// Tree view for features of the forex core
class Navigator : public ParentCtrl {
	
	// Vars
	TreeCtrl tree;
	int accounts, account_id, indicators, expertadvisors;
	
	// Protected main functions to prevent direct (wrong) usage
	void FillTree(TreeCtrl &tree);
	
public:
	
	// Ctors
	typedef Navigator CLASSNAME;
	Navigator();
	
	
	// Main funcs
	void Init();
	void Data();
	void SelectTree();
	
	
	// Public vars
	Callback1<int> WhenFactory;
	
};

}

#endif
