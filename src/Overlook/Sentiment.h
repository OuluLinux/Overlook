#ifndef _Overlook_Sentiment_h_
#define _Overlook_Sentiment_h_

namespace Overlook {

class SentimentSnapshot {
	
};

class Sentiment {
	
	Array<Array<SentimentSnapshot> > sents;
	
	
public:
	Sentiment();
	
	
	int GetSentimentCount(int tf_id) {return sents[tf_id].GetCount();}
	SentimentSnapshot& GetSentiment(int tf_id, int sent) {return sents[tf_id][sent];}
	
	
};

inline Sentiment& GetSentiment() {return Single<Sentiment>();}


class SentPresCtrl : public Ctrl {
	
public:
	virtual void Paint(Draw& w);

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
	
};

}

#endif
