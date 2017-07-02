#ifndef _Overlook_Trainer_h_
#define _Overlook_Trainer_h_

namespace Overlook {
using namespace Upp;
using ConvNet::SDQNAgent;

typedef Tuple3<double, double, double> DoubleTrio;

struct SessionThread {
	typedef SessionThread CLASSNAME;
	
	ConvNet::Session	ses;
	SimBroker			broker;
	
	ConvNet::Window loss_window, reward_window, l1_loss_window, l2_loss_window, train_window, accuracy_window, test_window;
	ConvNet::Window accuracy_result_window;
	
	void Serialize(Stream& s) {s % ses;}
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
	
	
	// Persistent vars
	int thrd_count;
	Array<SessionThread> thrds;
	
	
	// Tmp vars
	Iterator iter;
	Vector<Ptr<CoreItem> > work_queue, major_queue;
	Vector<Vector<Vector<ConstBuffer*> > > value_buffers;
	Index<int> tf_ids, sym_ids, indi_ids;
	System* sys;
	int not_stopped;
	int input_width, input_height, input_depth, output_width;
	bool running;
	
	void LoadThis();
	void StoreThis();
	void Runner(int i);
	
	enum {ACT_NOACT, ACT_INCSIG, ACT_DECSIG, ACT_RESETSIG, ACT_INCBET, ACT_DECBET,     ACTIONCOUNT};
	
public:
	typedef Trainer CLASSNAME;
	Trainer(System& sys);
	~Trainer() {Stop();}
	
	void Serialize(Stream& s) {s % thrd_count % thrds;}
	
	void Init();
	void InitThreads();
	void Start();
	void Stop();
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ResetIterator();
	void ResetValueBuffers();
	
	bool Seek(int shift);
	bool SeekCur(int shift);
};

}

#endif