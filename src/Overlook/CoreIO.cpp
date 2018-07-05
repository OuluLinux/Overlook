#include "Overlook.h"

namespace Overlook {


void CoreItem::SetInput(int input_id, int sym_id, int tf_id, CoreItem& core, int output_id) {
	InputDef& in = inputs[input_id];
	in.Add(HashSymTf(sym_id, tf_id), SourceDef(&core, output_id, sym_id, tf_id));
}









CoreIO::CoreIO() {
	serialized = true;
	sym_id = -1;
	tf_id = -1;
	factory = -1;
	hash = -1;
	#if defined flagDEBUG && defined flagSAFETYLIMITS
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
		inputs.Add();
	}
	else if (base.data_type == ValueBase::OUT_) {
		Output& out = outputs.Add();
		out.buffers.SetCount(base.count);
		out.visible = base.visible;
		for(int i = 0; i < out.buffers.GetCount(); i++)
			buffers.Add(&out.buffers[i]);
	}
	else if (base.data_type == ValueBase::LBL_) {
		Label& lbl = labels.Add();
		lbl.buffers.SetCount(base.count);
	}
	else if (base.data_type == ValueBase::PERS_) {
		persistents.Add(dynamic_cast<const Persistent&>(base));
	}
	else if (base.data_type == ValueBase::INT_) {
		ArgPtr& aptr = args.Add();
		aptr.ptr = (int*)base.data;
		aptr.min = base.min;
		aptr.max = base.max;
	}
}

void CoreIO::RefreshBuffers() {
	buffers.SetCount(0);
	for(int j = 0; j < outputs.GetCount(); j++)
		for(int i = 0; i < outputs[j].buffers.GetCount(); i++)
			buffers.Add(&outputs[j].buffers[i]);
}

void CoreIO::SetInput(int input_id, int sym_id, int tf_id, CoreIO& core, int output_id) {
	Input& in = inputs[input_id];
	if (core.GetOutputCount()) {
		in.Add(HashSymTf(sym_id, tf_id), Source(&core, &core.GetOutput(output_id), sym_id, tf_id));
	} else {
		in.Add(HashSymTf(sym_id, tf_id), Source(&core, NULL, sym_id, tf_id));
	}
}

ConstBuffer& CoreIO::GetInputBuffer(int input, int sym, int tf, int buffer) const {
	Output* out = inputs[input].Get(HashSymTf(sym, tf)).output;
	if (buffer < 0) buffer += out->buffers.GetCount();
	return SafetyBuffer(out->buffers[buffer]);
}

ConstLabelSignal& CoreIO::GetInputLabel(int input, int sym, int tf) const {
	return inputs[input].Get(HashSymTf(sym, tf)).core->labels[0].buffers[input];
}
	
CoreIO* CoreIO::GetInputCore(int input, int sym, int tf) const {
	return inputs[input].Get(HashSymTf(sym, tf)).core;
}

CoreIO* CoreIO::GetInputCore(int input) const {
	return inputs[input].Get(HashSymTf(this->sym_id, this->tf_id)).core;
}

const CoreIO& CoreIO::GetInput(int input, int sym, int tf) const {
	return *inputs[input].Get(HashSymTf(sym, tf)).core;
}

String CoreIO::GetCacheDirectory() {
	if (!cache_dir.IsEmpty())
		return cache_dir;
	ASSERT(factory != -1);
	int64 arghash = 0;
	Core* core = dynamic_cast<Core*>(this);
	if (core) {
		ArgChanger arg;
		arg.SetLoading();
		core->IO(arg);
		if (!arg.args.IsEmpty()) {
			CombineHash ch;
			for(int i = 0; i < arg.args.GetCount(); i++)
				ch << (int)arg.args[i] << 1;
			arghash = ch;
		}
	}
	
	String coredir = Format("%d-%d-%d-%d-", factory, sym_id, tf_id, hash) + IntStr64(arghash);
	cache_dir = AppendFileName(ConfigFile("corecache"), coredir);
	RealizeDirectory(cache_dir);
	return cache_dir;
}

void CoreIO::StoreCache() {
	if (!is_init) {
		LOG("warning: CoreIO::StoreCache not storing without init");
		return;
	}
	
	if (!serialized)
		return;
	
	if (serialization_lock.TryEnter()) {
		
		try {
			String dir = GetCacheDirectory();
			String file = AppendFileName(dir, "core.bin");
			FileOut out(file);
			if (out.IsOpen()) {
				Put(out, dir, 0);
				Core* c = dynamic_cast<Core*>(this);
				if (c) {
					for(int i = 0; i < c->subcores.GetCount(); i++)
						c->subcores[i].Put(out, dir, 1+i);
				}
			}
		}
		catch(...) {
			LOG("Serialization error");
		}
		
		serialization_lock.Leave();
	}
}

void CoreIO::Put(Stream& out, const String& dir, int subcore_id) {
	int output_count = outputs.GetCount();
	int persistent_count = persistents.GetCount();
	int job_count = jobs.GetCount();
	
	out % output_count % persistent_count % counted % bars % labels;
	
	out % job_count;
	for(int i = 0; i < jobs.GetCount(); i++)
		out % jobs[i];
	
	for(int i = 0; i < persistents.GetCount(); i++) {
		Persistent& p = persistents[i];
		p.Serialize(out);
	}
	
	for(int i = 0; i < outputs.GetCount(); i++) {
		Output& output = outputs[i];
		out % output.phase % output.type % output.visible;
		
		for(int j = 0; j < output.buffers.GetCount(); j++) {
			Buffer& buf = output.buffers[j];
			
			// Store values incrementally
			String file = AppendFileName(dir, Format("%d-%d-%d.bin", i, j, subcore_id));
			FileAppend out(file);
			if (!out.IsOpen())
				continue;
			
			int begin = buf.GetResetEarliestWrite();
			int64 add = begin != INT_MAX ? buf.value.GetCount() - begin : -1;
			if (add > 0) {
				out.Seek(begin * sizeof(double));
				out.Put(buf.value.Begin() + begin, (int)(add * sizeof(double)));
			}
		}
	}
	
}

void CoreIO::LoadCache() {
	if (!serialized)
		return;
	
	String dir = GetCacheDirectory();
	
	String file = AppendFileName(dir, "core.bin");
	FileIn in(file);
	if (!in.IsOpen())
		return;
	
	Get(in, dir, 0);
	Core* c = dynamic_cast<Core*>(this);
	if (c) {
		for(int i = 0; i < c->subcores.GetCount(); i++)
			c->subcores[i].Get(in, dir, 1+i);
	}
}

void CoreIO::Get(Stream& in, const String& dir, int subcore_id) {
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
	in % counted % bars % labels;
	
	int job_count;
	in % job_count;
	if (job_count != jobs.GetCount()) {
		LOG("CoreIO::LoadCache: error: persistent variable count mismatch");
		return;
	}
	for(int i = 0; i < jobs.GetCount(); i++)
		in % jobs[i];
	
	for(int i = 0; i < persistents.GetCount(); i++) {
		Persistent& p = persistents[i];
		p.Serialize(in);
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
			String file = AppendFileName(dir, Format("%d-%d-%d.bin", i, j, subcore_id));
			FileIn in(file);
			if (!in.IsOpen()) {
				LOG("CoreIO::LoadCache: error: file doesn't exist: " + file);
				continue;
			}
			
			int count = (int)(in.GetSize() / sizeof(double));
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
