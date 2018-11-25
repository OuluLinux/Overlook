#ifndef _Overlook_Signal_h_
#define _Overlook_Signal_h_

namespace Overlook {
using namespace libmt;


struct PairSignal : Moveable<PairSignal> {
	int id = -1;
	int sig = 0;
	int time = -1;
	int pips = 0;
};

struct CurrencySignal : public PairSignal, Moveable<CurrencySignal> {
	int sig_count = 0;
	byte impact = 0;
	String currency;
	bool has_prev_sig = false;
};

struct Day : Moveable<Day> {
	Date date;
	Vector<int> cal;
	Vector<CurrencySignal> cursig;
	Vector<PairSignal> pairsig;
	byte calsig_counter = 0;
	bool has_high_impacts = false;
	bool has_high_impacts_upcoming = false;
};


class Signal : public Common {
	VectorMap<Date, Day> days;
	int cal_counter = 0;
	int calsig_counter = 0;
	int sig_counter = 0;
	int pip_counter = 0;
	
	void RefreshDays();
	void RefreshCalendar();
	void RefreshCalendarSignals();
	int GetSignal(const CalEvent& ev);
	int GetPrevSignal(const CalEvent& ev);
	void RefreshSignals();
	void RefreshPips();
	void GetPips(PairSignal& pairsig, int tp_pips, int sl_pips);
	
public:
	typedef Signal CLASSNAME;
	Signal();
	
	virtual void Init();
	virtual void Start();
};


class SignalCtrl : public CommonCtrl {
	Splitter split;
	ArrayCtrl days;
	ArrayCtrl signals;
	
public:
	typedef SignalCtrl CLASSNAME;
	SignalCtrl();
	
	virtual void Data();
};



}


#endif
