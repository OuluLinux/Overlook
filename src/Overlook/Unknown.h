#ifndef _Overlook_Unknown_h_
#define _Overlook_Unknown_h_

namespace Overlook {


class MultiSimple : public ScriptCore {
	int tf = 0;
	int symbol = 0;
	int src_trainpercent = 50;
	int src_windowsize = 50;
	int change_enum = PRICE;
	int change_postpips = 5;
	int volat_enum = PRICE;
	int volat_ticks = 3;
	
	ScriptList sl_change, sl_volat;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Tf", 0, GetSystem().GetPeriodCount()-1, 1, tf);
		arg.Add("Symbol", 0, GetSystem().GetNormalSymbolCount()-1, 1, symbol);
		arg.Add("Train data percent", 0, 100, 1, src_trainpercent);
		arg.Add("Window size", 1, 100, 10, src_windowsize);
		arg.Add("Change Input type", PRICE, MAS, 1, change_enum);
		arg.Add("Change Post pips count", 2, 60, 1, change_postpips);
		arg.Add("Volat Input type", PRICE, MAS, 1, volat_enum);
		arg.Add("Volat Post tick count", 2, 60, 1, volat_ticks);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "MultinetSimple";};
	virtual void Run();
};



class AllSame : public ScriptCore {
	int tf = 0;
	int symbol = 0;
	int src_trainpercent = 50;
	int src_windowsize = 50;
	int change_enum = PRICE;
	int change_postpips = 5;
	
	ScriptList sl_change_single, sl_change_multi, sl_change_net;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Tf", 0, GetSystem().GetPeriodCount()-1, 1, tf);
		arg.Add("Symbol", 0, GetSystem().GetNormalSymbolCount()-1, 1, symbol);
		arg.Add("Train data percent", 0, 100, 1, src_trainpercent);
		arg.Add("Window size", 1, 100, 10, src_windowsize);
		arg.Add("Change Input type", PRICE, MAS, 1, change_enum);
		arg.Add("Change Post pips count", 2, 60, 1, change_postpips);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "AllSame";};
	virtual void Run();
};




class AllSameMultiPips : public ScriptCore {
	int tf = 0;
	int symbol = 0;
	int src_trainpercent = 50;
	int src_windowsize = 50;
	int change_enum = PRICE;
	int change_postpips0 = 5;
	int change_postpips1 = 10;
	int change_postpips2 = 20;
	
	ScriptList sl_change_single0, sl_change_multi0, sl_change_net0;
	ScriptList sl_change_single1, sl_change_multi1, sl_change_net1;
	ScriptList sl_change_single2, sl_change_multi2, sl_change_net2;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Tf", 0, GetSystem().GetPeriodCount()-1, 1, tf);
		arg.Add("Symbol", 0, GetSystem().GetNormalSymbolCount()-1, 1, symbol);
		arg.Add("Train data percent", 0, 100, 1, src_trainpercent);
		arg.Add("Window size", 1, 100, 10, src_windowsize);
		arg.Add("Change Input type", PRICE, MAS, 1, change_enum);
		arg.Add("Change Post pips count 1.", 2, 60, 1, change_postpips0);
		arg.Add("Change Post pips count 2.", 2, 60, 1, change_postpips1);
		arg.Add("Change Post pips count 3.", 2, 60, 1, change_postpips2);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "AllSameMultiPips";};
	virtual void Run();
};





class AllSameMultiType : public ScriptCore {
	int tf = 0;
	int symbol = 0;
	int src_trainpercent = 50;
	int change_postpips = 5;
	int change_enum0 = PRICE;
	int change_enum1 = INDI;
	int change_enum2 = SPCT;
	int change_windowsize0 = 50;
	int change_windowsize1 = 5;
	int change_windowsize2 = 50;
	
	ScriptList sl_change_single0, sl_change_multi0, sl_change_net0;
	ScriptList sl_change_single1, sl_change_multi1, sl_change_net1;
	ScriptList sl_change_single2, sl_change_multi2, sl_change_net2;
	
public:
	virtual void Init();
	virtual void Arg(ArgScript& arg) {
		arg.Add("Tf", 0, GetSystem().GetPeriodCount()-1, 1, tf);
		arg.Add("Symbol", 0, GetSystem().GetNormalSymbolCount()-1, 1, symbol);
		arg.Add("Train data percent", 0, 100, 1, src_trainpercent);
		arg.Add("Change Post pips count", 2, 60, 1, change_postpips);
		arg.Add("Change Input type 1.", PRICE, MAS, 1, change_enum0);
		arg.Add("Change Input type 2.", PRICE, MAS, 1, change_enum1);
		arg.Add("Change Input type 3.", PRICE, MAS, 1, change_enum2);
		arg.Add("Window size 1.", 1, 100, 10, change_windowsize0);
		arg.Add("Window size 2.", 1, 100, 10, change_windowsize1);
		arg.Add("Window size 3.", 1, 100, 10, change_windowsize2);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return "AllSameMultiType";};
	virtual void Run();
};


}

#endif
