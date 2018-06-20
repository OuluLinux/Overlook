#ifndef _Overlook_BeamSearchOptimization_h_
#define _Overlook_BeamSearchOptimization_h_

namespace Overlook {

class BeamSearchOptimizer {
	Vector<int> trial_solution;
	Index<int> fixed_solution;
	Vector<double> layer_score;
	int layer_iter = 0;
	int layer_size = 0;
	int layer = 0;
	int max_layers = 100;
	int round = 0, max_round = 0;
	bool fail = false;
	
public:
	
	void Serialize(Stream& s) {
		s % trial_solution % fixed_solution
		  % layer_score
		  % layer_iter
		  % layer_size
		  % layer
		  % max_layers
		  % round % max_round % fail;
	}
	
	void Init(int dimension, int max_layers) {
		this->max_layers = max_layers;
		layer_size = dimension;
		layer_score.SetCount(layer_size, 0);
		round = 0;
		max_round = layer_size * max_layers;
		fail = false;
	}
	
	void Start() {
		int size = fixed_solution.GetCount() + 1;
		trial_solution.SetCount(size);
		for(int i = 0; i < fixed_solution.GetCount(); i++)
			trial_solution[i] = fixed_solution[i];
		trial_solution[size-1] = layer_iter;
	}
	
	void Stop(double score) {
		layer_score[layer_iter] = score;
		
		layer_iter++;
		if (layer_iter >= layer_size) {
			layer_iter = 0;
			layer++;
			
			int max_i = -1;
			double max_score = -DBL_MAX;
			for(int i = 0; i < layer_score.GetCount(); i++) {
				if (fixed_solution.Find(i) != -1)
					continue;
				if (layer_score[i] > max_score) {
					max_score = layer_score[i];
					max_i = i;
				}
			}
			
			if (max_i == -1)
				fail = true;
			else
				fixed_solution.Add(max_i);
		}
		
		round++;
	}
	
	int GetRound() const {return round;}
	int GetMaxRounds() {return max_round;}
	bool IsEnd() const {return round >= max_round || fail;}
	const Vector<int>& GetTrialSolution() {return trial_solution;}
	
};

}

#endif
