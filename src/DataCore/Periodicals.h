#ifndef _DataCore_Periodicals_h_
#define _DataCore_Periodicals_h_

#include "Slot.h"

namespace DataCore {
using namespace Upp;

/*
	These periodical indicators exploit the notion that the market week has fixed cycles which
	causes repeating patterns. The week starts with the opening of Asia exchanges. Europe and
	US exchanges reacts to tensions built up at weekend. The first 30mins of stock exchange might
	determine the direction of entire day, and even big indices. Most exchanges has fixed
	weekday for busiest activity. These wdayhour indicators have showed monday+thursday,
	tuesday+friday and wednesday as the most active days in different symbols.
	
	The most fixed thing in weekday/hour statistics is the distribution of possible differences
	in the value. It is reasonable to expect that the outcome will remain in the most probable
	channel. The pressure for some symbol before tight channel might be interpreted as
	weakness, and the lack of pressure for some symbol before wide channel might be interpreted
	as uncertainty. These channel statistics are sensored by Forecaster class, which has
	regression neural network. Channel statistics are joined to multi-symbol recurrent neural
	network forecasts (NARX) by regression neural network and the Forecaster is expected to give
	predictions inside the channel after training, while NARX as input source might give values
	outside of the channel.
	
	Previously, I have tested these indicators against all popular traditional indicators
	(stochastic oscillator, RSI, etc.) for trend prediction and these wdayhour indicators have
	outperformed them always after genetic optimization.
	
	EventOscillator is not weekly-periodic indicator, or any strictly fixed period, but it
	shows events, which usually occurs in monthly, quaterly or yearly period.
	
*/

class WdayHourStats : public Slot {
	
protected:
	struct SymTf : Moveable<SymTf> {
		MovingOnlineVariance total;
		Vector<MovingOnlineVariance> wdayhour, wday, hour;
	};
	Vector<SymTf> data;
	SlotPtr src;
	int var_period;
	
public:
	WdayHourStats();
	
	virtual String GetName() {return "WdayHourStats";}
	virtual String GetShortName() const {return "whstat";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

class WdayHourDiff : public Slot {
	
protected:
	
	int var_period_fast, var_period_diff;
	SlotPtr whstat_fast, whstat_slow;
	
public:
	WdayHourDiff();
	
	virtual String GetName() {return "WdayHourDiff";}
	virtual String GetShortName() const {return "whd";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

class ChannelStats : public Slot {
	SlotPtr whstat_slow;
	
public:
	ChannelStats();
	
	virtual String GetName() {return "ChannelStats";}
	virtual String GetShortName() const {return "chstat";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

class ChannelPredicter : public Slot {
	
protected:
	
	SlotPtr src, chstat;
	int length;
	
public:
	ChannelPredicter();
	
	virtual String GetName() {return "ChannelPredicter";}
	virtual String GetShortName() const {return "chp";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};















class EventOsc : public Slot {
	struct Sym : Moveable<Sym> {
		Index<String> keys;
		int counted_events;
	};
	Vector<Sym> data;
	
	SlotPtr src;
	DataBridge* db;
	EventManager* emgr;
	double mul;
	
public:
	EventOsc();
	
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetName() {return "EventOsc";}
	virtual String GetShortName() const {return SHORTNAME0("eosc");}
	virtual void SetArguments(const VectorMap<String, Value>& args);
};

}

#endif
