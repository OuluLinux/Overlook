#ifndef _Overlook_Trainer_h_
#define _Overlook_Trainer_h_

namespace Overlook {
using namespace Upp;
using ConvNet::SDQNAgent;

typedef Tuple3<double, double, double> DoubleTrio;

struct ValueChanger {
	virtual ~ValueChanger() {}
	virtual void Set(double d) = 0;
	virtual double Get() const = 0;
};

template <class T>
struct TypeValueChanger : public ValueChanger {
	T* ptr;
	TypeValueChanger(T& t) {ptr = &t;}
	virtual void Set(double d) {*ptr = d;}
	virtual double Get() const {return *ptr;}
};

template <class T> inline ValueChanger* Ch(T& t) {return new TypeValueChanger<T>(t);}

struct SessionSettings {
	struct Layer : Moveable<Layer> {
		int act, neuron_count;
		bool has_dropout;
		double bias_pref, dropout_prob;
		void Serialize(Stream& s) {
			s % act % neuron_count % has_dropout % bias_pref % dropout_prob;
		}
		void GetChangers(Vector<ValueChanger*>& v) {
			v << Ch(act) << Ch(neuron_count) << Ch(has_dropout) << Ch(bias_pref) << Ch(dropout_prob);
		}
	};
	
	Vector<Layer> layers;
	double l2, lr, mom;
	int trainer_type;
	int neural_layers;
	int batch_size;
	
	void Serialize(Stream& s) {
		s % layers % l2 % lr % mom % trainer_type % neural_layers % batch_size;
	}
	void GetChangers(Vector<ValueChanger*>& v) {
		v << Ch(l2) << Ch(lr) << Ch(mom) << Ch(trainer_type) << Ch(neural_layers) << Ch(batch_size);
		for(int i = 0; i < layers.GetCount(); i++) layers[i].GetChangers(v);
	}
};

struct SessionThread {
	typedef SessionThread CLASSNAME;
	
	SessionSettings		settings;
	ConvNet::Session	ses;
	String				params;
	int					epochs, epoch_actual, epoch_total;
	int					id;
	bool				is_finished;
	
	ConvNet::Window		loss_window, reward_window, l1_loss_window, l2_loss_window, train_window, accuracy_window;
	ConvNet::Window		test_reward_window, test_window0, test_window1;
	double				total_sigchange;
	
	SimBroker			broker;
	
	void Serialize(Stream& s) {
		s % settings % ses % params % epochs % epoch_actual % epoch_total % id % is_finished
		  % loss_window % reward_window % l1_loss_window % l2_loss_window % train_window % accuracy_window
		  % test_reward_window % test_window0 % test_window1 % total_sigchange;
	}
};

struct Iterator : Moveable<Iterator> {
	Vector<Vector<Vector<DoubleTrio> > > value;
	Vector<Vector<double> > min_value, max_value;
	Vector<int> pos, tfs, periods, period_in_slower, time_values;
	Array<ConvNet::VolumeData<double> > volume_out;
	Volume volume_in;
	Time begin;
	int begin_ts;
	int value_count;
	int bars;
};

class Trainer {
	
protected:
	friend class TrainerCtrl;
	friend class TrainerDraw;
	friend class TrainerConfiguration;
	friend class RealtimeStatistics;
	friend class TrainerThreadCtrl;
	friend class StatsGraph;
	friend class TrainerResult;
	
	
	// Persistent vars
	int thrd_count;
	Array<SessionThread> sessions;
	ConvNet::Window ses_sigchanges;
	int session_count;
	
	
	// Tmp vars
	Array<ConvNet::Window> thrd_priorities, thrd_performances;
	Array<One<SessionThread> > thrds;
	Vector<Iterator> iters;
	Vector<Ptr<CoreItem> > work_queue, major_queue;
	Vector<Vector<Vector<ConstBuffer*> > > value_buffers;
	Index<int> tf_ids, sym_ids, indi_ids;
	Vector<int> data_begins;
	Vector<int> train_pos, test_pos;
	TimeStop last_store;
	System* sys;
	double evol_cont_probability, evol_scale;
	int max_sessions;
	int not_stopped;
	int test_interval;
	int input_width, input_height, input_depth, output_width;
	int training_limit;
	int session_cur;
	bool running;
	
	
	// Tmp vars from MagicNet
	double train_ratio;
	double l2_decay_min, l2_decay_max;
	double learning_rate_min, learning_rate_max;
	double momentum_min, momentum_max;
	int num_folds;
	int num_candidates;
	int num_epochs;
	int ensemble_size;
	int foldix, datapos;
	int batch_size_min, batch_size_max;
	int neurons_min, neurons_max;
	Mutex lock;
	
	
	void LoadThis();
	void ThreadHandler(int i);
	void Runner(int thrd_id);
	void ResetIterator(int thrd_id);
	bool Seek(int thrd_id, int shift);
	bool SeekCur(int thrd_id, int shift);
	void SampleCandidate(int thrd_id);
	void EvolveSettings(SessionThread& st);
	void RandomSettings(SessionThread& st);
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG, ACT_RESETSIG, ACT_INCBET, ACT_DECBET,     ACTIONCOUNT};
	
public:
	typedef Trainer CLASSNAME;
	Trainer(System& sys);
	~Trainer();
	
	void Serialize(Stream& s) {s % thrd_count % sessions % ses_sigchanges % session_count;}
	void StoreThis();
	
	void Init();
	void InitThreads();
	void Start();
	void Stop();
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ResetIterators();
	void ResetValueBuffers();
	void ShuffleTraining(Vector<int>& train_pos);
};

}

#endif
