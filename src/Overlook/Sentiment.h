#ifndef _Overlook_Sentiment_h_
#define _Overlook_Sentiment_h_

namespace Overlook {

struct SentimentSnapshot {
	Vector<int> cur_pres, pair_pres;
	String comment;
	Time added;
	double fmlevel = 0.0, tplimit = 0.1;
	
	void Serialize(Stream& s) {s % cur_pres % pair_pres % comment % added % fmlevel % tplimit;}
	
	bool IsPairEqual(const SentimentSnapshot& s) {
		if (s.pair_pres.GetCount() != pair_pres.GetCount())
			return false;
		for(int i = 0; i < pair_pres.GetCount(); i++)
			if (pair_pres[i] != s.pair_pres[i])
				return false;
		return true;
	}
};

class Sentiment : public Common {
	
protected:
	friend class Automation;
	friend class EventSystem;
	
	// Persistent
	Array<SentimentSnapshot> sents;
	
	
	// Temporary
	Index<String> symbols;
	VectorMap<String, double> prev_levels;
	bool enable_takeprofit = false;
	
public:
	Sentiment();
	
	virtual void Init();
	virtual void Start();
	void SetSignals();
	int GetSymbolCount() const {return symbols.GetCount();}
	String GetSymbol(int i) const {return symbols[i];}
	
	int GetSentimentCount() {return sents.GetCount();}
	SentimentSnapshot& GetSentiment(int sent) {return sents[sent];}
	SentimentSnapshot& AddSentiment() {return sents.Add();}
	bool IsTakeProfit() {if (sents.IsEmpty()) return false; return sents.Top().comment == "Auto take-profit";}
	
	void Serialize(Stream& s) {s % sents;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("Sentiment.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("Sentiment.bin"));}
	
};

inline Sentiment& GetSentiment() {return GetSystem().GetCommon<Sentiment>();}


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

class SentimentCtrl : public CommonCtrl {
	Splitter split;
	ArrayCtrl historylist, curpreslist, pairpreslist, errlist;
	EditDouble fmlevel;
	ParentCtrl console;
	::Upp::DocEdit comment;
	Button save;
	
	Array<SentPresCtrl> pair_pres_ctrl, cur_pres_ctrl, net_pres_ctrl;
	
	
public:
	typedef SentimentCtrl CLASSNAME;
	SentimentCtrl();
	
	
	void Data();
	void LoadHistory();
	void Save();
	void SetNetPairPressures();
	void SetCurPairPressures();
	void SetCurProfile();
	void SetPairProfile();
	
};

}

#endif
