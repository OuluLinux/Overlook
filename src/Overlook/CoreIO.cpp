#include "Overlook.h"

namespace Overlook {


void CoreItem::SetInput(int input_id, int sym_id, int tf_id, CoreItem& core, int output_id) {
	InputDef& in = inputs[input_id];
	in.Add(sym_id * 1000 + tf_id, SourceDef(&core, output_id, sym_id, tf_id));
}









CoreIO::CoreIO() {
	serialized = true;
	sym_id = -1;
	tf_id = -1;
	factory = -1;
	hash = -1;
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
		inputs.Add();
	}
	else if (base.data_type == ValueBase::OUT_) {
		Output& out = outputs.Add();
		out.buffers.SetCount(base.count);
		out.visible = base.visible;
		for(int i = 0; i < out.buffers.GetCount(); i++)
			buffers.Add(&out.buffers[i]);
	}
	else if (base.data_type == ValueBase::PERS_) {
		persistents.Add(dynamic_cast<const Persistent&>(base));
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
		in.Add(sym_id * 1000 + tf_id, Source(&core, &core.GetOutput(output_id), sym_id, tf_id));
	} else {
		in.Add(sym_id * 1000 + tf_id, Source(&core, NULL, sym_id, tf_id));
	}
}

CoreIO* CoreIO::GetInputCore(int input, int sym, int tf) const {
	return inputs[input].Get(sym * 1000 + tf).core;
}

const CoreIO& CoreIO::GetInput(int input, int sym, int tf) const {
	return *inputs[input].Get(sym * 1000 + tf).core;
}

String CoreIO::GetCacheDirectory() {
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
	
	String coredir = Format("%d-%d-%d-%d-", sym_id, tf_id, factory, hash) + IntStr64(arghash);
	String dir = AppendFileName(ConfigFile("corecache"), coredir);
	RealizeDirectory(dir);
	return dir;
}

void CoreIO::StoreCache() {
	if (!serialized)
		return;
	
	if (outputs.IsEmpty() || outputs.GetCount() == 1 && outputs[0].buffers.IsEmpty())
		return;
	
	String dir = GetCacheDirectory();
	
	if (serializer_lock.TryEnter()) {
		String file = AppendFileName(dir, "core.bin");
		FileOut out(file);
		if (!out.IsOpen())
			Panic("Couldn't open file: " + file);
		
		Put(out, dir, 0);
		Core* c = dynamic_cast<Core*>(this);
		if (c) {
			for(int i = 0; i < c->subcores.GetCount(); i++)
				c->subcores[i].Put(out, dir, 1+i);
		}
		
		serializer_lock.Leave();
	}
}

void CoreIO::Put(Stream& out, const String& dir, int subcore_id) {
	int output_count = outputs.GetCount();
	int persistent_count = persistents.GetCount();
	out % output_count % persistent_count % counted % bars;
	
	for(int i = 0; i < persistents.GetCount(); i++) {
		Persistent& p = persistents[i];
		p.Serialize(out);
	}
	
	for(int i = 0; i < outputs.GetCount(); i++) {
		Output& output = outputs[i];
		out % output.phase % output.type % output.visible;
		
		// VectorBool label is typically around 1-10K bytes and that's not too much.
		out % output.label;
		
		for(int j = 0; j < output.buffers.GetCount(); j++) {
			Buffer& buf = output.buffers[j];
			
			// Store values incrementally
			String file = AppendFileName(dir, Format("%d-%d-%d.bin", i, j, subcore_id));
			FileAppend out(file);
			if (!out.IsOpen())
				Panic("Couldn't open file: " + file);
			
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
	in % counted % bars;
	
	for(int i = 0; i < persistents.GetCount(); i++) {
		Persistent& p = persistents[i];
		p.Serialize(in);
	}
	
	for(int i = 0; i < outputs.GetCount(); i++) {
		int phase, type, visible;
		in % phase % type % visible;
		
		Output& output = outputs[i];
		
		in % output.label;
		
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
