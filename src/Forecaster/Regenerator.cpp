#include "Forecaster.h"
#include <plugin/bz2/bz2.h>


namespace Forecast {

Regenerator::Regenerator() {
	
}

void Regenerator::Iterate() {
	int cores = GetUsedCpuCores();
	
	gen.SetCount(0);
	gen.SetCount(cores);
	CoWork co;
	for(int i = 0; i < gen.GetCount(); i++) {
		co & [=] {
			Generator& g = gen[i];
			g.Init(0.0001, real_data, Vector<double>());
			while (g.state < Generator::BITSTREAMED)
				g.DoNext();
		};
	}
	co.Finish();
	
	int popcount = 100;
	opt.min_value = -1;
	opt.max_value = +1;
	opt.SetMaxGenerations(100);
	opt.Init(gen[0].stream.GetColumnCount() * INDIPRESSURE_PARAMS + APPLYPRESSURE_PARAMS, popcount);
	
	
	while (!opt.IsEnd() && !Thread::IsShutdownThreads()) {
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
	
	
	gen[0].PopWarmup(opt.GetBestSolution());
	while (gen[0].state < Generator::FORECASTED)
		gen[0].DoNext();
}

void Regenerator::RunOnce(int i) {
	for(int j = 0; j < gen.GetCount(); j++) {
		Generator& g = gen[j];
		if (g.lock.TryEnter()) {
			g.PopWarmup(opt.GetTrialSolution(i));
			g.DoNext();
			//if (j == 0)
			//	g.DoNext();
			ASSERT(g.err != -DBL_MAX);
			err[i] = -g.err;
			//if (j == 0)
			//	last_energy = -g.err;
			g.lock.Leave();
			
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
			break;
		}
	}
}

}
