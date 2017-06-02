#include "Overlook.h"

namespace Overlook {

CoreIO::CoreIO() {
	sym_id = -1;
	tf_id = -1;
	#ifdef flagDEBUG
	read_safety_limit = 0;
	#endif
}

CoreIO::~CoreIO() {
	
}

void CoreIO::IO(const ValueBase& base) {
	if (base.data_type == ValueBase::IN_) {
		inputs.Add();
	}
	else if (base.data_type == ValueBase::INOPT_) {
		Panic("TODO: optional input signal is not received here yet");
		inputs.Add();
	}
	else if (base.data_type == ValueBase::INDYN_) {
		inputs.Add();
	}
	else if (base.data_type == ValueBase::INHIGHPRIO_) {
		inputs.Add();
	}
	else if (base.data_type == ValueBase::OUT_) {
		Output& out = outputs.Add();
		out.phase = base.phase;
		out.type = base.type;
		out.buffers.SetCount(base.count);
		out.visible = base.visible;
		for(int i = 0; i < out.buffers.GetCount(); i++)
			buffers.Add(&out.buffers[i]);
	}
	else if (base.data_type >= ValueBase::PERS_BOOL_ && base.data_type <= ValueBase::PERS_DOUBLE_) {
		persistents.Add((const Persistent&)base);
	}
}

void CoreIO::RefreshBuffers() {
	buffers.SetCount(0);
	for(int j = 0; j < outputs.GetCount(); j++)
		for(int i = 0; i < outputs[j].buffers.GetCount(); i++)
			buffers.Add(&outputs[j].buffers[i]);
}

void CoreIO::AddInput(int input_id, int sym_id, int tf_id, CoreIO& core, int output_id) {
	Input& in = inputs[input_id];
	if (core.GetOutputCount()) {
		in.sources.Add(sym_id * 100 + tf_id, Source(&core, &core.GetOutput(output_id), sym_id, tf_id));
	} else {
		in.sources.Add(sym_id * 100 + tf_id, Source(&core, NULL, sym_id, tf_id));
	}
}

String CoreIO::GetCacheDirectory() const {
	// TODO: arguments
	//CombineHash ch;
	//for(int i = 0; i < args.GetCount(); i++)
	uint32 arghash = 0;
	
	ASSERT(!unique.IsEmpty());
	String coredir = unique + Format("-%d-%d-%d", sym_id, tf_id, (int64)arghash);
	String dir = AppendFileName(ConfigFile("corecache"), coredir);
	RealizeDirectory(dir);
	return dir;
}

void CoreIO::StoreCache() {
	if (outputs.IsEmpty() || outputs.GetCount() == 1 && outputs[0].buffers.IsEmpty())
		return;
	
	String dir = GetCacheDirectory();
	
	String file = AppendFileName(dir, "core.bin");
	FileOut out(file);
	if (!out.IsOpen())
		Panic("Couldn't open file: " + file);
	
	int output_count = outputs.GetCount();
	int persistent_count = persistents.GetCount();
	out % output_count % persistent_count % counted % bars;
	
	for(int i = 0; i < persistents.GetCount(); i++) {
		Persistent& p = persistents[i];
		if (p.data_type == ValueBase::PERS_BOOL_) {
			bool v = *(bool*)p.data;
			out.Put(&v, sizeof(bool));
		}
		else if (p.data_type == ValueBase::PERS_INT_) {
			int v = *(int*)p.data;
			out.Put(&v, sizeof(int));
		}
		else if (p.data_type == ValueBase::PERS_DOUBLE_) {
			double v = *(double*)p.data;
			out.Put(&v, sizeof(double));
		}
		else if (p.data_type == ValueBase::PERS_INTMAP_) {
			VectorMap<int,int>& v = *(VectorMap<int,int>*)p.data;
			out % v;
		}
		else if (p.data_type == ValueBase::PERS_QUERYTABLE_) {
			QueryTable& v = *(QueryTable*)p.data;
			out % v;
		}
		else Panic("Invalid datatype");
	}
	
	for(int i = 0; i < outputs.GetCount(); i++) {
		Output& output = outputs[i];
		out % output.phase % output.type % output.visible;
		
		for(int j = 0; j < output.buffers.GetCount(); j++) {
			Buffer& buf = output.buffers[j];
			
			// Store values incrementally
			String file = AppendFileName(dir, Format("%d-%d.bin", i, j));
			FileAppend out(file);
			if (!out.IsOpen())
				Panic("Couldn't open file: " + file);
			
			int begin = buf.GetResetEarliestWrite();
			int64 add = begin != INT_MAX ? buf.value.GetCount() - begin : -1;
			if (add > 0) {
				out.Seek(begin * sizeof(double));
				out.Put(buf.value.Begin() + begin, add * sizeof(double));
			}
		}
	}
}

void CoreIO::LoadCache() {
	String dir = GetCacheDirectory();
	
	String file = AppendFileName(dir, "core.bin");
	FileIn in(file);
	if (!in.IsOpen())
		return;
	
	int output_count = 0;
	in % output_count;
	if (output_count != outputs.GetCount()) {
		LOG("CoreIO::LoadCache: error: output count mismatch");
		return;
	}
	
	int persistent_count = 0;
	in % persistent_count;
	if (persistent_count != persistents.GetCount()) {
		LOG("CoreIO::LoadCache: error: persistent variable count mismatch");
		return;
	}
	
	int counted, bars;
	in % counted % bars;
	
	for(int i = 0; i < persistents.GetCount(); i++) {
		Persistent& p = persistents[i];
		if (p.data_type == ValueBase::PERS_BOOL_) {
			bool* v = (bool*)p.data;
			in.Get(v, sizeof(bool));
		}
		else if (p.data_type == ValueBase::PERS_INT_) {
			int* v = (int*)p.data;
			in.Get(v, sizeof(int));
		}
		else if (p.data_type == ValueBase::PERS_DOUBLE_) {
			double* v = (double*)p.data;
			in.Get(v, sizeof(double));
		}
		else if (p.data_type == ValueBase::PERS_INTMAP_) {
			VectorMap<int,int>& v = *(VectorMap<int,int>*)p.data;
			in % v;
		}
		else if (p.data_type == ValueBase::PERS_QUERYTABLE_) {
			QueryTable& v = *(QueryTable*)p.data;
			in % v;
		}
		else Panic("Invalid datatype");
	}
	
	for(int i = 0; i < outputs.GetCount(); i++) {
		int phase, type, visible;
		in % phase % type % visible;
		
		Output& output = outputs[i];
		if (output.phase != phase || output.type != type || output.visible != visible) {
			LOG("CoreIO::LoadCache: error: output type mismatch");
			return;
		}
		
		for(int j = 0; j < output.buffers.GetCount(); j++) {
			Buffer& buf = output.buffers[j];
			
			// Store values incrementally
			String file = AppendFileName(dir, Format("%d-%d.bin", i, j));
			FileIn in(file);
			if (!in.IsOpen()) {
				LOG("CoreIO::LoadCache: error: file doesn't exist: " + file);
				continue;
			}
			
			int count = in.GetSize() / sizeof(double);
			if (count > 0) {
				buf.value.SetCount(count);
				in.Get(buf.value.Begin(), count * sizeof(double));
			}
		}
	}
	
	this->counted = counted;
	this->bars = bars;
}

}
