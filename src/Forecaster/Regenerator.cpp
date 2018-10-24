#include "Forecaster.h"

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
			g.Init(0.0001, *real_data, Vector<double>());
			while (g.state < Generator::BITSTREAMED)
				g.DoNext();
		};
	}
	co.Finish();
	
	static const int POPCOUNT = 100;
	opt.min_value = -1;
	opt.max_value = +1;
	opt.SetMaxGenerations(30);
	opt.Init(gen[0].stream.GetColumnCount() * 3 + APPLYPRESSURE_PARAMS, POPCOUNT);
	
	
	while (!opt.IsEnd() && !Thread::IsShutdownThreads()) {
		opt.Start();
		
		double err[POPCOUNT];
	
		CoWork co;
		co.SetPoolSize(cores);
		for(int i = 0; i < POPCOUNT; i++) {
		
			co & [=, &err]
			{
				for(int j = 0; j < gen.GetCount(); j++) {
					Generator& g = gen[j];
					if (g.lock.TryEnter()) {
						g.PopWarmup(opt.GetTrialSolution(i));
						g.DoNext();
						if (j == 0)
							g.DoNext();
						err[i] = -g.err;
						last_energy = -g.err;
						g.lock.Leave();
						break;
					}
				}
				
			};
		}
		co.Finish();
		
		opt.Stop(err);
		
		last_energy = err[0];
		for(int i = 0; i < POPCOUNT; i++)
			LOG(err[i]);
	}
	
	
	gen[0].PopWarmup(opt.GetBestSolution());
	while (gen[0].state < Generator::FORECASTED)
		gen[0].DoNext();
}

}
