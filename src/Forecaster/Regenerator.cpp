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
		opt.SetMaxGenerations(100);
		opt.Init(gen[0].stream.GetColumnCount() * INDIPRESSURE_PARAMS + APPLYPRESSURE_PARAMS, popcount);
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
				//if (j == 0)
				//	g.DoNext();
				ASSERT(g.err != -DBL_MAX);
				err[i] = -g.err;
				//if (j == 0)
				//	last_energy = -g.err;
				
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

void Regenerator::Forecast() {
	result_lock.Enter();
	Sort(results, RegenResult());
	forecasts.Clear();
	result_lock.Leave();
	
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());
	for(int i = 0; i < results.GetCount() && i < 30; i++) {
		co & THISBACK1(ForecastOnce, i);
	}
	co.Finish();
	
	result_lock.Enter();
	Sort(forecasts, ForecastResult());
	result_lock.Leave();
}

void Regenerator::ForecastOnce(int i) {
	while (true) {
		for(int j = 0; j < gen.GetCount(); j++) {
			Generator& g = gen[j];
			if (g.lock.TryEnter()) {
				RegenResult& rr = results[i];
				
				g.PopWarmup(rr.params);
				while (g.state < Generator::FORECASTED)
					g.DoNext();
				
				int begin = this->real_data.GetCount();
				int size = g.real_data.GetCount() - begin;
				Heatmap fcast_hmap;
				g.image.CopyRight(fcast_hmap, size);
				StringStream ss;
				ss.SetStoring();
				ss % fcast_hmap;
				ss.Seek(0);
				String heatmap = BZ2Compress(ss.Get(ss.GetSize()));
				
				result_lock.Enter();
				ForecastResult& fr = forecasts.Add();
				fr.heatmap = heatmap;
				fr.data.SetCount(size);
				fr.id = i;
				for(int i = 0; i < size; i++)
					fr.data[i] = g.real_data[begin + i];
				result_lock.Leave();
				g.lock.Leave();
				return;
			}
		}
	}
}

}
