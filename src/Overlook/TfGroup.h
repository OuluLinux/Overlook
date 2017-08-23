#ifndef _Overlook_TfGroup_h_
#define _Overlook_TfGroup_h_

namespace Overlook {

class TfGroup {
	
public:
	
	// Persistent
	Array<Agent> agents;
	double tf_limit;
	int input_width, input_height;
	int tf_id;
	
	
	// Temp
	Vector<Vector<Vector<ConstBuffer*> > > value_buffers;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Vector<Core*> > databridge_cores;
	Array<Snapshot> snaps;
	int tf, tf_minperiod, tf_period, tf_type;
	int data_begin;
	
	
	
	typedef TfGroup CLASSNAME;
	TfGroup();
	
	
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ProcessDataBridgeQueue();
	
	
};

}

#endif
