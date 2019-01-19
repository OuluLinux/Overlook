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
	CoreList cl_sym, cl_sym0;
	CoreList cl_indi;
	
	int tf = 6;
	int train_percent = 50;
	int input_enum = PRICE;
	int postpips_count = 5;
	int symbol = 0;
	int windowsize = 50;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Tf", 0, GetSystem().GetPeriodCount()-1, 1, tf);
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
	CoreList cl_sym, cl_sym0;
	CoreList cl_indi;
	
	int tf = 6;
	int train_percent = 50;
	int input_enum = PRICE;
	int postpips_count = 5;
	int windowsize = 50;
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Tf", 0, GetSystem().GetPeriodCount()-1, 1, tf);
		arg.Add("Train data percent", 0, 100, 1, train_percent);
		arg.Add("Input type", PRICE, MAS, 1, input_enum);
		arg.Add("Post pips count", 2, 60, 1, postpips_count);
		arg.Add("Window size", 1, 100, 10, windowsize);
	}
	virtual void SerializeEvent(Stream& s) {s % ses;}
	virtual String GetTitle() {return "MultiChangeNeural";};
	virtual void Run();
	virtual void GetSignal(int symbol, LabelSignal& signal);
};




class Change2Tf : public ScriptCore {
	ScriptList sl0, sl1;
	CoreList cl_sym;
	CoreList cl_indi;
	
	int tf0 = 6;
	int tf1 = 8;
	int symbol = 0;
	int train_percent = 50;
	int input_enum = PRICE;
	int postpips_count = 5;
	int windowsize = 50;
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Tf0", 0, GetSystem().GetPeriodCount()-1, 1, tf0);
		arg.Add("Tf1", 0, GetSystem().GetPeriodCount()-1, 1, tf1);
		arg.Add("Symbol", 0, GetSystem().GetNormalSymbolCount()-1, 1, symbol);
		arg.Add("Train data percent", 0, 100, 1, train_percent);
		arg.Add("Input type", PRICE, MAS, 1, input_enum);
		arg.Add("Post pips count", 2, 60, 1, postpips_count);
		arg.Add("Window size", 1, 100, 10, windowsize);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "Change2Tf";};
	virtual void Run();
	virtual void GetSignal(int symbol, LabelSignal& signal);
};



class MultinetChangeNeural : public ScriptCore {
	ConvNet::Session ses;
	CoreList cl_net, cl_net0;
	CoreList cl_indi;

	int tf = 6;
	int train_percent = 50;
	int input_enum = PRICE;
	int postpips_count = 5;
	int windowsize = 50;

public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Tf", 0, GetSystem().GetPeriodCount()-1, 1, tf);
		arg.Add("Train data percent", 0, 100, 1, train_percent);
		arg.Add("Input type", PRICE, MAS, 1, input_enum);
		arg.Add("Post pips count", 2, 60, 1, postpips_count);
		arg.Add("Window size", 1, 100, 10, windowsize);
	}
	virtual void SerializeEvent(Stream& s) {s % ses;}
	virtual String GetTitle() {return "MultinetChangeNeural";};
	virtual void Run();
	virtual void GetSignal(int symbol, LabelSignal& signal);
};




}


#endif
