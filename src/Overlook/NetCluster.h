#ifndef _Overlook_NetCluster_h_
#define _Overlook_NetCluster_h_

namespace Overlook {
using namespace libmt;


class NetCluster : public Common {
	CoreList cl;
	
	static const int cluster_count = 10;
	
	
public:
	typedef NetCluster CLASSNAME;
	NetCluster();
	
	virtual void Init();
	virtual void Start();
};


class NetClusterCtrl : public CommonCtrl {
	
public:
	typedef NetClusterCtrl CLASSNAME;
	NetClusterCtrl();
	
	virtual void Data();
};


}


#endif
