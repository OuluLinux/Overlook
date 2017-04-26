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
	
	NOTE: it is mathematically incorrect to use normal distrbution, because the probability is
	nearly 0 for the price-change to be 0 during open market. Better solution would be to use
	two separate gamma distributions for positive and negative, and Bernoulli distribution for
	the price-change to be positive or negative. It would require a statistician to analyse the
	correct distribution, but for this use case, it is not necessary, because we are only
	interested about mean average and some kind of maximum/minimum channel, not how probable
	some value-change is or something like that.
*/

class WdayHourStats : public Slot {
	
protected:
	struct SymTf : Moveable<SymTf> {
		MovingStepDistribution total;
		Vector<MovingStepDistribution> wdayhour, wday, hour;
	};
	Vector<SymTf> data;
	int var_period;
	
public:
	WdayHourStats();
	
	virtual String GetName() {return "WdayHourStats";}
	virtual String GetShortName() const {return "whstat";}
	virtual String GetKey() const {return "whstat";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

class WdayHourDiff : public Slot {
	
protected:
	
	int var_period_fast, var_period_diff;
	
public:
	WdayHourDiff();
	
	virtual String GetName() {return "WdayHourDiff";}
	virtual String GetShortName() const {return "whdiff";}
	virtual String GetKey() const {return "whdiff";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

class ChannelPredicter : public Slot {
	
protected:
	
	int length;
	
public:
	ChannelPredicter();
	
	virtual String GetName() {return "ChannelPredicter";}
	virtual String GetShortName() const {return "chp";}
	virtual String GetKey() const {return "chp";}
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
	
	DataBridge* db;
	EventManager* emgr;
	double mul;
	
public:
	EventOsc();
	
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetName() {return "EventOsc";}
	virtual String GetShortName() const {return SHORTNAME0("eosc");}
	virtual String GetKey() const {return SHORTNAME0("eosc");}
	virtual void SetArguments(const VectorMap<String, Value>& args);
};

}

#endif
