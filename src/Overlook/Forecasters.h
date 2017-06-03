#if 0

#ifndef _Overlook_Forecasters_h_
#define _Overlook_Forecasters_h_

namespace Overlook {

/*
	These classes are frameworks for building different datasets.
	
	Datasets can be effectively learned with the decision tree method, but binary descriptor
	matching and virtual neural network models can also be used.
	The binary feature descriptor matching is derived from image processing, where the
	similarity can be measured by calculating Hamming distance to other descriptors in
	different time-positions.
	
	These frameworks can be mixed to create new kind of classes.

*/



/*
	'Edge' recognizes changes in the trend.
	
	It predicts points of direction changes in the trend before they can be measured and it uses
	periodical nature of markets for that. It assumes that trend is relatively stable between points of
	trend changes.
	Technically it is a lagging indicator, which uses lagging symmetric average and non-lagging
	moving average, or only one moving average with two reading points. The difference between
	different time-positions is calculated in both averages and they are multiplied. >= 0 value
	means that both averages has same direction of the trend and < 0 value means that the
	symmetric average is changing direction but lagging average is not yet. This gives temporary
	vector with the oscillating graph, which shows points where the trend changes.
	To make the graph more clear, edge filtering is perfomed on it.
	
	The accuracy of the forecast is based on predictable trend changing points and stable trend
	between those points. Also, the slower trend is expected to be more powerful than faster,
	but important market events are exception to that, e.g. central bank press conferences.
*/
class Edge : public Model {
	QueryTable qt;
	
public:
	Edge();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InHigherPriority()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

/*
	'Ideal' matches past ideal orders to the current market description.
	
	It takes the ideal order sequence from zigzag indicator, which is a lagging indicator.
	Those orders are converted to constant signals of long/short and that is attached to time
	and indicators. Those values can be calculated in real-time and previously matching ideal
	order can be predicted.
	
	The accuracy of the forecast is based on experiences in the similar market description.
*/
class Ideal : public Core {
	QueryTable qt;
	
public:
	Ideal();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InHigherPriority()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

/*
	'Custom' can take traditional indicator values as inputs.
	
	Indicator values, correlation and time values are matched to changes in data. Those values
	can be calculated in real-time and the change can be predicted.
	
	The accuracy of the forecast is based on matching market descriptions in the past
*/
class Custom : public Core {
	Array<DecisionTreeNode> tree;
	QueryTable qt;
	int corr_period, max_timesteps, steps, peek;
	
public:
	typedef Custom CLASSNAME;
	Custom();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, Sym)
			% In(IndiPhase, RealChangeValue, Sym)
			% In(IndiPhase, CorrelationValue, SymTf)
			% InHigherPriority()
			//% InOptional(IndiPhase, RealIndicatorValue, SymTf)
			% Out(ForecastPhase, ForecastRealValue, SymTf, 3, 3)
			% Arg("Correlation period", corr_period);
	}
	
};

/*
	'Channel' predicts the maximum/minimum change channel and bets the center of the channel.
	
	The channel is calculated in a way, that the probability of channel break is low.
	
	The accuracy of the forecast is based on (false?) assumption that possible change has
	probability of Gaussian distribution and the center value is the most probable. The theory
	has apparent flaws, however.
*/
class Channel : public Core {
	QueryTable qt;
	
public:
	Channel();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InHigherPriority()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

/*
	'SlowTarget' takes the prediction of the slower timeframe and targets it directly.
	
	Predictions to slower periods has usually lower error rate. These slower predictions can be
	used in fixed targets in faster timeframe data. If the sum of changes is lower than
	predicted in the slow tf, then the prediction is 'long'. If the sum of changes is already
	higher than predicted in the slow tf, then the prediction is 'short'.
	
	The accuracy of the forecast is based on the fractal nature of markets, where faster
	timeframes must eventually follow slower and more probable trends.
*/
class SlowTarget : public Core {
	QueryTable qt;
	
public:
	SlowTarget();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InHigherPriority()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

/*
	'Average' is a stripped down predictor, with only time and volume as inputs.
	
	This uses only of the phase of periods as inputs. Many other classes uses them as inputs
	too, but this uses only them.
	
	The accuracy of the forecast is based on periodic nature of markets, where market week has
	fixed phases where some exchanges opens and closes, etc.
*/
class Average : public Core {
	QueryTable qt;
	
public:
	Average();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InHigherPriority()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

/*
	'Trending' predicts that will the trend continue or not.
	
	This targets the information, that is the next change the average of previous changes.
	Inputs can be e.g. custom indicators, correlation or time.
	
	The accuracy of the forecast is based on momentum-like property of trends.
*/
class Trending : public Core {
	QueryTable qt;
	
public:
	Trending();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InHigherPriority()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

/*
	'Combiner' uses all previous predictors to provide better prediction than any of those.
	
	This takes all previous predictors and time as inputs. Some combinations are more
	meaningful than others, and some predictors might be better than others sometimes.
	
	The accuracy of the forecast is based on performance comparison of other forecasters.
*/
class Combiner : public Core {
	QueryTable qt;
	
public:
	Combiner();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InHigherPriority()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

}

#endif
#endif
