#include "Overlook.h"

#if 0

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

CoreIO* CoreIO::GetInputCore(int input, int sym, int tf) const {
	return inputs[input].Get(HashSymTf(sym, tf)).core;
}

CoreIO* CoreIO::GetInputCore(int input) const {
	return inputs[input].Get(HashSymTf(this->sym_id, this->tf_id)).core;
}

const CoreIO& CoreIO::GetInput(int input, int sym, int tf) const {
	return *inputs[input].Get(HashSymTf(sym, tf)).core;
}


}
#endif
