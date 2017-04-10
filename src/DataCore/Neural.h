#ifndef _DataCore_Neural_h_
#define _DataCore_Neural_h_

#include "Slot.h"

namespace DataCore {

class Recurrent : public Slot {
	enum {PHASE_GATHERDATA, PHASE_TRAINING};
	
	Array<Array<ConvNet::RecurrentSession> > ses;
	//VectorMap<int, int> letterToIndex;
	//VectorMap<int, double> indexToLetter;
	Vector<double> vocab;
	Vector<double> temperatures;
	Vector<int> sequence;
	OnlineVariance var;
	SlotPtr src;
	String model_str;
	double min, max, step, diff;
	int batch;
	int shifts;
	int phase;
	int value_count;
	int iter;
	int tick_time;
	bool is_training_loop;
	SpinLock lock;
	
	int ToChar(double diff);
	double FromChar(int i);
	
	void StoreThis();
	void LoadThis();
	void InitSessions();
public:
	Recurrent();
	virtual String GetKey() {return "rnn";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
	void Reset();
	void Reload();
	
	int GetIter() const {return iter;}
	int GetTickTime() const {return tick_time;}
	int GetSampleTemperature(int i) const {return temperatures[i];}
	double GetPerplexity() const;
	String GetModel() const {return model_str;}
	
	void SetPreset(int i);
	void SetLearningRate(double rate);
	void SetSampleTemperature(int i, double t) {temperatures[i] = t;}
	void SetModel(String model_str) {this->model_str = model_str;}
	void Pause();
	void Resume();
	void Refresher();
	void InitVocab();
	void PredictSentence(const SlotProcessAttributes& attr, bool samplei=false, double temperature=1.0);
	void Tick(const SlotProcessAttributes& attr);
	
	ConvNet::RecurrentSession& GetSession(int sym, int tf) {return ses[sym][tf];}
	
	
	
	void Serialize(Stream& s) {
		s % vocab % temperatures % var % model_str %
			min % max % step % diff % batch % shifts % phase % value_count % iter %
			tick_time % is_training_loop;
		if (s.IsLoading()) {
			InitSessions();
			for(int i = 0; i < ses.GetCount(); i++) {
				for(int j = 0; j < ses[i].GetCount(); j++) {
					ValueMap map;
					s % map;
					ses[i][j].Load(map);
				}
			}
		} else {
			for(int i = 0; i < ses.GetCount(); i++) {
				for(int j = 0; j < ses[i].GetCount(); j++) {
					ValueMap map;
					ses[i][j].Store(map);
					s % map;
				}
			}
		}
	}
};

}

#endif
