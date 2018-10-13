#ifndef _AsmProto_Session_h_
#define _AsmProto_Session_h_


class Session  {
	Net net;
	
public:
	typedef Session CLASSNAME;
	Session();
	void Load(Conf conf);
	void Clear();
	
	Net& GetNet() {return net;}
	
};



#endif
