#ifndef _Overlook_Sentiment_h_
#define _Overlook_Sentiment_h_

namespace Overlook {

struct SentimentSnapshot {
	Vector<int> net_pres, pair_pres;
	String comment;
	Time added;
	double correctness = 0.0;
	int realtf = -1;
	
	void Serialize(Stream& s) {s % net_pres % pair_pres % comment % added % correctness % realtf;}
};

class Sentiment {
	
	// Persistent
	Array<SentimentSnapshot> sents;
	
	
	// Temporary
	Index<String> symbols;
	
public:
	Sentiment();
	
	int GetSymbolCount() const {return symbols.GetCount();}
	String GetSymbol(int i) const {return symbols[i];}
	
	int GetSentimentCount() {return sents.GetCount();}
	SentimentSnapshot& GetSentiment(int sent) {return sents[sent];}
	SentimentSnapshot& AddSentiment() {return sents.Add();}
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
	ArrayCtrl historylist, tflist, eventlist, netlist, pairpreslist;
	ParentCtrl console;
	::Upp::DocEdit comment;
	Button save;
	
	Array<SentPresCtrl> pair_pres_ctrl, net_pres_ctrl;
	
	
public:
	typedef SentimentCtrl CLASSNAME;
	SentimentCtrl();
	
	
	void Data();
	void LoadTf();
	void LoadHistory();
	void Save();
	void SetPairPressures();
	void SetSignals();
	void SetEventProfile();
	void SetNetProfile();
	void SetPairProfile();
	
};

}

#endif
