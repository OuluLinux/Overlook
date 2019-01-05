#ifndef _Overlook_Neural_h_
#define _Overlook_Neural_h_

namespace Overlook {


enum {
	PRICE,
	INDI,
	SPCT,
	MAS
};


class SingleChangeNeural : public ScriptCore {
	ConvNet::Session ses;
	CoreList cl_sym;
	CoreList cl_indi;
	
	int train_percent = 50;
	int input_enum = PRICE;
	int postpips_count = 5;
	int symbol = 0;
	int windowsize = 50;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Symbol", 0, GetSystem().GetNormalSymbolCount()-1, 1, symbol);
		arg.Add("Train data percent", 0, 100, 1, train_percent);
		arg.Add("Input type", PRICE, MAS, 1, input_enum);
		arg.Add("Post pips count", 2, 60, 1, postpips_count);
		arg.Add("Window size", 1, 100, 10, windowsize);
	}
	virtual void SerializeEvent(Stream& s) {s % ses;}
	virtual String GetTitle() {return "SingleChangeNeural";};
	virtual void Run();
	virtual void GetSignal(int symbol, LabelSignal& signal);
};


class MultiChangeNeural : public ScriptCore {
	ConvNet::Session ses;
	CoreList cl_sym;
	CoreList cl_indi;
	
	int train_percent = 50;
	int input_enum = PRICE;
	int postpips_count = 5;
	int extrapolation_mode = 0;
	int windowsize = 50;
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Train data percent", 0, 100, 1, train_percent);
		arg.Add("Input type", PRICE, MAS, 1, input_enum);
		arg.Add("Post pips count", 2, 60, 1, postpips_count);
		arg.Add("Unknown mode", 0, 4, 1, extrapolation_mode);
		arg.Add("Window size", 1, 100, 10, windowsize);
	}
	virtual void SerializeEvent(Stream& s) {s % ses;}
	virtual String GetTitle() {return "MultiChangeNeural";};
	virtual void Run();
	virtual void GetSignal(int symbol, LabelSignal& signal);
};


class MultinetChangeNeural : public ScriptCore {
	ConvNet::Session ses;
	CoreList cl_net;
	CoreList cl_indi;
	
	int train_percent = 50;
	int input_enum = PRICE;
	int postpips_count = 5;
	int extrapolation_mode = 0;
	int windowsize = 50;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Train data percent", 0, 100, 1, train_percent);
		arg.Add("Input type", PRICE, MAS, 1, input_enum);
		arg.Add("Post pips count", 2, 60, 1, postpips_count);
		arg.Add("Unknown mode", 0, 4, 1, extrapolation_mode);
		arg.Add("Window size", 1, 100, 10, windowsize);
	}
	virtual void SerializeEvent(Stream& s) {s % ses;}
	virtual String GetTitle() {return "MultinetChangeNeural";};
	virtual void Run();
	virtual void GetSignal(int symbol, LabelSignal& signal);
};


class SingleVolatNeural : public ScriptCore {
	ConvNet::Session ses;
	CoreList cl_sym;
	int train_percent = 50;
	int input_enum = PRICE;
	int ticks = 3;
	int symbol = 0;
	int windowsize = 50;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Symbol", 0, GetSystem().GetNormalSymbolCount()-1, 1, symbol);
		arg.Add("Train data percent", 0, 100, 1, train_percent);
		arg.Add("Input type", PRICE, MAS, 1, input_enum);
		arg.Add("Post tick count", 2, 10, 1, ticks);
		arg.Add("Window size", 1, 100, 10, windowsize);
	}
	virtual void SerializeEvent(Stream& s) {s % ses;}
	virtual String GetTitle() {return "SingleVolatNeural";};
	virtual void Run();
};


class MultiVolatNeural : public ScriptCore {
	ConvNet::Session ses;
	CoreList cl_sym;
	int train_percent = 50;
	int input_enum = PRICE;
	int ticks = 3;
	int extrapolation_mode = 0;
	int windowsize = 50;
	int postpips = 5;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Train data percent", 0, 100, 1, train_percent);
		arg.Add("Input type", PRICE, MAS, 1, input_enum);
		arg.Add("Post tick count", 2, 10, 1, ticks);
		arg.Add("Unknown mode", 0, 4, 1, extrapolation_mode);
		arg.Add("Window size", 1, 100, 10, windowsize);
		arg.Add("Signal post pip count", 2, 10, 1, postpips);
	}
	virtual void SerializeEvent(Stream& s) {s % ses;}
	virtual String GetTitle() {return "MultiVolatNeural";};
	virtual void Run();
	virtual void GetSignal(int symbol, LabelSignal& signal);
};


class MultinetVolatNeural : public ScriptCore {
	ConvNet::Session ses;
	CoreList cl_net;
	int train_percent = 50;
	int input_enum = PRICE;
	int ticks = 3;
	int extrapolation_mode = 0;
	int windowsize = 50;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Train data percent", 0, 100, 1, train_percent);
		arg.Add("Input type", PRICE, MAS, 1, input_enum);
		arg.Add("Post tick count", 2, 10, 1, ticks);
		arg.Add("Unknown mode", 0, 4, 1, extrapolation_mode);
		arg.Add("Window size", 1, 100, 10, windowsize);
	}
	virtual void SerializeEvent(Stream& s) {s % ses;}
	virtual String GetTitle() {return "MultinetVolatNeural";};
	virtual void Run();
	virtual void GetSignal(int symbol, LabelSignal& signal);
};


}


#endif
