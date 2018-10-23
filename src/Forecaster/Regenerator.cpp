#include "Forecaster.h"

namespace Forecast {

Regenerator::Regenerator() {
	
}

void Regenerator::Iterate() {
	trains_total++;
	
	
	// Get and refresh indicators
	Vector<CoreItem> work_queue;
	Vector<FactoryDeclaration> decl;
	AddDefaultDeclarations(decl);
	System::GetCoreQueue(*real_data, work_queue, decl);
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = work_queue[i];
		ci.core->SetBars(Generator::data_count);
		ci.core->Refresh();
	}
	
	
	// Get data stream for generator
	BitStream stream;
	System::GetBitStream(work_queue, stream);
	
	
	opt.min_value = -1;
	opt.max_value = +1;
	opt.SetMaxGenerations(10);
	opt.Init(stream.GetColumnCount() * 3, POPCOUNT);
	
	
	while (!opt.IsEnd() && !Thread::IsShutdownThreads()) {
		opt.Start();
		
		int iter[POPCOUNT];
		
		CoWork co;
		for(int i = 0; i < POPCOUNT; i++) {
			gen[i].ResetGenerate();

			co & [=, &stream]
			{
				BitStream stream2(stream);
				stream2.SetBit(0);
				const Vector<double>& trial = opt.GetTrialSolution(i);
				Generator& gen = this->gen[i];
				gen.GenerateTest(stream2, *real_data, trial);
				err[i] = -gen.err;
			};
		}
		co.Finish();
		
		opt.Stop(err);
		
		last_energy = err[0];
		for(int i = 0; i < POPCOUNT; i++)
			LOG(err[i]);
	}
	
	
	
	Vector<ConstLabelSignal*> lbls;
	System::GetLabels(work_queue, lbls);
	
	stream.SetColumnCount(lbls.GetCount());
	stream.SetCount(1);
	stream.SetBit(0);
	const Vector<double>& params = opt.GetBestSolution();
	
	data.SetCount(0);
	
	for(int i = Generator::data_count; i < Generator::data_count+2*1440; i++) {
		
		gen[0].Iteration(stream, i, params);
		
		data.Add(gen[0].price);
		real_data->Add(gen[0].price);
		
		for(int j = 0; j < work_queue.GetCount(); j++) {
			CoreItem& ci = work_queue[j];
			ci.core->SetBars(i+1);
			ci.core->Refresh(true);
		}
		
		stream.SetBit(0);
		for(int j = 0; j < lbls.GetCount(); j++) {
			bool value = lbls[j]->signal.Get(i);
			stream.Write(value);
		}
		stream.SetBit(0);
	}
}

}
