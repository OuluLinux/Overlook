#ifndef _Overlook_Interpolation_h_
#define _Overlook_Interpolation_h_

namespace Overlook {


enum {
	PRICE,
	INDI,
	SPCT,
	MAS
};


class SingleChangeInterpolation : public ScriptCore {
	ConvNet::Session ses;
	int input_enum = PRICE;
	int postpips_count = 5;
	int symbol = 0;
	int windowsize = 20;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "SingleChangeInterpolation";};
	virtual void Run();
};


class MultiChangeInterpolation : public ScriptCore {
	ConvNet::Session ses;
	int input_enum = PRICE;
	int postpips_count = 5;
	int extrapolation_mode = 0;
	int windowsize = 20;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "MultiChangeInterpolation";};
	virtual void Run();
};


class MultinetChangeInterpolation : public ScriptCore {
	ConvNet::Session ses;
	CoreList cl_net;
	
	int input_enum = PRICE;
	int postpips_count = 5;
	int extrapolation_mode = 0;
	int windowsize = 20;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "MultinetChangeInterpolation";};
	virtual void Run();
};


class SingleVolatInterpolation : public ScriptCore {
	ConvNet::Session ses;
	int input_enum = PRICE;
	int ticks = 3;
	int symbol = 0;
	int windowsize = 20;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "SingleVolatInterpolation";};
	virtual void Run();
};


class MultiVolatInterpolation : public ScriptCore {
	ConvNet::Session ses;
	int input_enum = PRICE;
	int ticks = 3;
	int extrapolation_mode = 0;
	int windowsize = 20;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "MultiVolatInterpolation";};
	virtual void Run();
};


class MultinetVolatInterpolation : public ScriptCore {
	ConvNet::Session ses;
	CoreList cl_net;
	int input_enum = PRICE;
	int ticks = 3;
	int extrapolation_mode = 0;
	int windowsize = 20;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "MultinetVolatInterpolation";};
	virtual void Run();
};


}


#endif
