#include "Forecaster.h"
#include <plugin/bz2/bz2.h>


namespace Forecast {

Regenerator::Regenerator() {
	
}

void Regenerator::Init() {
	ASSERT(!is_init);
	
	int cores = GetUsedCpuCores();
	gen.SetCount(0);
	gen.SetCount(cores);
	CoWork co;
	for(int i = 0; i < gen.GetCount(); i++) {
		co & [=] {
			double point = real_data[0] > 65 ? 0.01 : 0.0001;
			Generator& g = gen[i];
			g.Init(point, real_data, Vector<double>());
			while (g.state < Generator::BITSTREAMED)
				g.DoNext();
		};
	}
	co.Finish();
	
	if (opt.GetRound() == 0) {
		opt.min_value = -1;
		opt.max_value = +1;
		#ifdef flagDEBUG
		opt.SetMaxGenerations(10);
		#else
		opt.SetMaxGenerations(100);
		#endif
		opt.Init(gen[0].stream.GetColumnCount() * INDIPRESSURE_PARAMS + APPLYPRESSURE_PARAMS, popcount);
		
		
		dqn.Init(1, NNSample::single_count * NNSample::single_size, NNSample::fwd_count);
		dqn.Reset();
	}
	
	is_init = true;
}

void Regenerator::Iterate(int ms) {
	TimeStop ts;
	int cores = GetUsedCpuCores();
	
	
	while (!opt.IsEnd() && !Thread::IsShutdownThreads() && ts.Elapsed() < ms) {
		opt.Start();
		
		
		err.SetCount(popcount, -DBL_MAX);
	
		CoWork co;
		co.SetPoolSize(cores);
		for(int i = 0; i < popcount; i++) {
			co & THISBACK1(RunOnce, i);
		}
		co.Finish();
		
		opt.Stop(err.Begin());
		
		
		for(int i = 0; i < popcount; i++)
			LOG(err[i]);
	}
}

void Regenerator::RunOnce(int i) {
	while (true) {
		for(int j = 0; j < gen.GetCount(); j++) {
			Generator& g = gen[j];
			if (g.lock.TryEnter()) {
				g.PopWarmup(opt.GetTrialSolution(i));
				g.DoNext();
				ASSERT(g.err != -DBL_MAX);
				err[i] = -g.err;
				
				StringStream ss;
				ss.SetStoring();
				ss % g.image;
				ss.Seek(0);
				String heatmap = BZ2Compress(ss.Get(ss.GetSize()));
				
				
				result_lock.Enter();
				RegenResult& rr = results.Add();
				rr.id = results.GetCount() - 1;
				rr.gen_id = j;
				rr.err = g.err;
				rr.heatmap = heatmap;
				rr.params <<= g.params;
				result_errors.Add(-g.err);
				result_lock.Leave();
				
				g.lock.Leave();
				return;
			}
		}
	}
}

void Regenerator::RefreshNNSamples() {
	gen[0].nnsamples = &nnsamples;
	
	for(int i = 0; i < NNSample::single_count; i++) {
		const Vector<double>& trial = results[i].params;
		Generator& g = gen[0];
		g.result_id = i;
		g.PopWarmup(trial);
		g.DoNext();
		g.DoNext();
		ASSERT(g.state == Generator::ERROR_CALCULATED_FULLY);
		
	}
}

void Regenerator::GetProgress(int& actual, int& total, String& state) {
	if (dqn_iters == max_dqniters) {
		actual = 1;
		total = 1;
		state = "Forecast";
	}
	else if (dqn_iters > 0 && dqn_iters < max_dqniters) {
		actual = dqn_iters;
		total = max_dqniters;
		state = "DQN";
	}
	else if ((!IsInit() || opt.IsEnd()) && HasGenerators()) {
		actual = GetGenerator(0).actual;
		total = GetGenerator(0).total;
		if (!IsInit()) state = "Init";
		else state = "NNSamples";
	}
	else {
		actual = opt.GetRound();
		total = opt.GetMaxRounds();
		state = "Optimizing";
	}
	
}

void Regenerator::IterateNN(int ms) {
	TimeStop ts;
	while (ts.Elapsed() < ms && dqn_iters < max_dqniters && !Thread::IsShutdownThreads()) {
		NNSample& s = nnsamples[Random(nnsamples.GetCount())];
		dqn.Learn(&s.input[0][0], &s.output[0]);
		dqn_iters++;
	}
}

void Regenerator::Forecast() {
	NNSample s;
	
	for(int i = 0; i < NNSample::single_count; i++) {
		const Vector<double>& trial = results[i].params;
		Generator& g = gen[0];
		g.result_id = -1; // no need, skips sample refresh
		g.PopWarmup(trial);
		g.DoNext();
		g.DoNext();
		ASSERT(g.state == Generator::ERROR_CALCULATED_FULLY);
		g.result_id = i;
		g.GetSampleInput(s);
		ASSERT(g.iter == g.real_data->GetCount());
	}
	
	
	dqn.Evaluate(&s.input[0][0], &s.output[0]);
	
	result_lock.Enter();
	ForecastResult& fr = forecasts.Add();
	fr.id = forecasts.GetCount() - 1;
	fr.data.SetCount(NNSample::fwd_count);
	for(int i = 0; i < NNSample::fwd_count; i++)
		fr.data[i] = s.output[i];
	result_lock.Leave();
}


}
