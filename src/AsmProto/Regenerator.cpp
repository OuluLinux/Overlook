#include "AsmProto.h"

Regenerator::Regenerator() {
	
}

void Regenerator::Iterate() {
	trains_total++;
	
	opt.min_value = -1;
	opt.max_value = +1;
	opt.Init();
	
	while (!opt.IsEnd() && !Thread::IsShutdownThreads()) {
		opt.Start();
		
		int iter[CUDA_CORES];
		double err[CUDA_CORES];
		
		
		try {
			array_view<Generator, 1> gen_view(CUDA_CORES, gen);
			//concurrency::array<double, 1> err_view(CUDA_CORES, err);
			for(int i = 0; i < CUDA_CORES; i++) {
				err[i] = i;
				gen[i].ResetGenerate();
			}
			array_view<double, 1> err_view(CUDA_CORES, err);
			array_view<int, 1> iter_view(CUDA_CORES, iter);
			array_view<double, 1> trial_view(CUDA_CORES * ARG_COUNT, opt.GetTrialSolution(0));
			array_view<double, 1> real_data_view(Generator::data_count, real_data);
			//concurrency::array<Generator, 1> gen_view(CUDA_CORES, gen);
			
			
			while (true) {
				parallel_for_each(gen_view.extent, [=](index<1> idx) PARALLEL {
					int id = idx[0];
					int arg_begin = id * ARG_COUNT;
					double* trial = &trial_view[arg_begin];
					
					Generator& gen = gen_view[idx];
					gen.mom_inc = trial[MOM_INC];
					gen.mom_dec = trial[MOM_DEC];
					gen.ma_inc = trial[MA_INC];
					gen.ma_dec = trial[MA_DEC];
					gen.ap_inc = trial[AP_INC];
					gen.ap_dec = trial[AP_DEC];
					
					gen_view[idx].GenerateData(&real_data_view[0], Generator::data_count * 0.8);
					err_view[idx] = -gen_view[idx].err;
					iter_view[idx] = gen_view[idx].iter;
				});
				
				iter_view.synchronize();
				
				bool finished = true;
				for(int i = 0; i < CUDA_CORES && finished; i++) {
					if (iter[i] < Generator::data_count)
						finished = false;
				}
				if (finished)
					break;
			}
			
			//gen_view.synchronize();
			err_view.synchronize();
			//completion_future w = err_view.synchronize_async();
			//w.wait();
		}
		/*catch(accelerator_view_removed& ex)
		{
		    LOG(ex.what());
			LOG(ex.get_view_removed_reason());
		}*/
		catch (runtime_exception& e) {
			LOG(e.what());
		}
		catch (...) {
			LOG("unknown error");
		}
		opt.Stop(err);
		
		last_energy = err[CUDA_CORES-1];
		for(int i = 0; i < CUDA_CORES; i++)
			LOG(err[i]);
	}
	
}


