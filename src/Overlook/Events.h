#ifndef _Overlook_Events_h_
#define _Overlook_Events_h_

namespace Overlook {


class MovingAverageEvent : public EventCore {
	
	// Temporary
	int sym_count;
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("MA tf=%d period=%d", tf, period);}
	
};



class MACDEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("MACD tf=%d period=%d", tf, period);}
	
};




class BBEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 40, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("BB tf=%d period=%d", tf, period);}
	
};



class PSAREvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("PSAR tf=%d period=%d", tf, period);}
	
};



class CCIEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("CCI tf=%d period=%d", tf, period);}
	
};



class DeMarkerEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("DeMarker tf=%d period=%d", tf, period);}
	
};



class MomentumEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("Momentum tf=%d period=%d", tf, period);}
	
};


class RSIEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 50, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("RSI tf=%d period=%d", tf, period);}
	
};


class RVIEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 50, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("RVI tf=%d period=%d", tf, period);}
	
};


class StochasticEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("Stochastic tf=%d period=%d", tf, period);}
	
};


class AcceleratorEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("Accelerator tf=%d period=%d", tf, period);}
	
};


class ChannelEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("Channel tf=%d period=%d", tf, period);}
	
};



class ScissorsChannelEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("ScissorsChannel tf=%d period=%d", tf, period);}
	
};




class OnlineMinimalLabelEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(0, 10, 5, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("OnlineMinimalLabel tf=%d period=%d", tf, period);}
	
};




class LaguerreEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 80, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("Laguerre tf=%d period=%d", tf, period);}
	
};




class QQEEvent : public EventCore {
	
	// Temporary
	
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("QQE tf=%d period=%d", tf, period);}
	
};




class TickBalanceEvent : public EventCore {
	
	// Temporary
	int period = 13;
	int tf = -1;
	
public:
	virtual void Init();
	virtual void Start(int pos, int& output);
	virtual void Arg(ArgEvent& args) {
		args.Add(fast_tf, 4, 1, tf);
		args.Add(10, 100, 10, period);
	}
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() {return Format("QQE tf=%d period=%d", tf, period);}
	
};


}

#endif
