#ifndef _Overlook_AutoChartist_h_
#define _Overlook_AutoChartist_h_

namespace Overlook {

#define AUTOCHARTIST_DEBUG 1

class AutoChartistSymbol {

protected:
	friend class AutoChartist;
	friend class AutoChartistCtrl;
	
	struct DataPoint : Moveable<DataPoint> {
		int high_len = 0, low_len = 0;
		int high_angle = 0, low_angle = 0;
		int high_weight = 0, low_weight = 0;
		int highest_pos = 0, lowest_pos = 0;
		float high = 0, low = 0;
		
		void Serialize(Stream& s) {
			s % high_len % low_len % high_angle % low_angle
			  % high_weight % low_weight % highest_pos % lowest_pos
			  % high % low;}
	};
	
	
	// Persistent
	Vector<DataPoint> data;
	double point = 0.0001;
	int sym = -1, tf = -1;
	int counted = 0;
	
	
	// Temp
	Vector<Ptr<CoreItem> > work_queue;
	
public:
	typedef AutoChartistSymbol CLASSNAME;
	AutoChartistSymbol();
	
	void Data();
	
	void Serialize(Stream& s) {s % data % sym % tf % counted;}
	
};


class AutoChartist {
	
protected:
	friend class AutoChartistCtrl;
	
	// Persistent
	Array<AutoChartistSymbol> symbols;
	
	
	// Temp
	bool running = false, stopped = true;
	int total = 0, actual = 0;
	
	
public:
	typedef AutoChartist CLASSNAME;
	AutoChartist();
	
	void Data();
	void StartData();
	bool IsRunning() const {return running && !Thread::IsShutdownThreads();}
	
	void Serialize(Stream& s) {s % symbols;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("AutoChartist.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("AutoChartist.bin"));}
	
};

inline AutoChartist& GetAutoChartist() {return Single<AutoChartist>();}


class AutoChartistCtrl : public ParentCtrl {
	ParentCtrl par;
	ProgressIndicator prog;
	Splitter split;
	#if AUTOCHARTIST_DEBUG
	ArrayCtrl tflist, symlist, timelist, eventlist, featlist;
	#endif
	
	
	int prev_symcursor = -1, prev_tfcursor = -1, prev_timecursor = -1;
	
public:
	typedef AutoChartistCtrl CLASSNAME;
	AutoChartistCtrl();
	
	void Data();
	
};


}

#endif
