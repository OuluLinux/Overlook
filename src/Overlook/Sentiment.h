#ifndef _Overlook_Sentiment_h_
#define _Overlook_Sentiment_h_

namespace Overlook {

struct SentimentSnapshot {
	Index<int> cur_events, pair_events;
	Vector<int> cur_pres, pair_pres;
	String comment;
	Time added;
	double correctness = 0.0;
	int realtf = -1;
	
	void Serialize(Stream& s) {s % cur_events % pair_events % cur_pres % pair_pres % comment % added % correctness % realtf;}
};

class Sentiment {
	
	Array<Array<SentimentSnapshot> > sents;
	
	
public:
	Sentiment();
	
	
	int GetSentimentCount(int tf_id) {return sents[tf_id].GetCount();}
	SentimentSnapshot& GetSentiment(int tf_id, int sent) {return sents[tf_id][sent];}
	SentimentSnapshot& AddSentiment(int tf_id) {return sents[tf_id].Add();}
	SentimentSnapshot* FindReal();
	
	void Serialize(Stream& s) {s % sents;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("Sentiment.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("Sentiment.bin"));}
	
};

inline Sentiment& GetSentiment() {return Single<Sentiment>();}


class SentPresCtrl : public Ctrl {
	int i = 0;
	
public:
	virtual void Paint(Draw& w);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);
	virtual void MouseMove(Point p, dword keyflags);

	
	virtual Value GetData() const {return i;}
	virtual void SetData(const Value& v) {i = v;}
};

class SentimentCtrl : public ParentCtrl {
	Splitter split;
	ArrayCtrl tflist, historylist, cureventlist, paireventlist, curpreslist, pairpreslist;
	ParentCtrl console, curevent, pairevent;
	DropList cureventsym, paireventsym, realtf;
	::Upp::DocEdit comment;
	Button save;
	
	Array<SentPresCtrl> pair_pres_ctrl, cur_pres_ctrl;
	Array<Option> pair_ena_ctrl, cur_ena_ctrl;
	
	
public:
	typedef SentimentCtrl CLASSNAME;
	SentimentCtrl();
	
	
	void Data();
	void LoadTf();
	void LoadHistory();
	void Save();
	void SetPairPressures();
	void SetSignals();
	void SetCurrencyProfile();
	void SetPairProfile();
	
};

}

#endif
