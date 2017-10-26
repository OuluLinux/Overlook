#include "Optimizer.h"

namespace Overlook {

GeneticOptimizer::GeneticOptimizer() {
	arraycount = -1;
	count = -1;
	dim = -1;
	pop = -1;
	max_gens = -1;
	use_limits = false;
	round = 0;
	random_type = DiffSolver::RAND_UNIFORM;
}

void GeneticOptimizer::Init() {
	ASSERT(arraycount > 0);
	ASSERT(count > 0);
	ASSERT(pop > 0);
	ASSERT(max_gens > 0);
	
	max_rounds = max_gens * pop;
	
	RefreshDim();
	
	for(int i = 1; i < arraycount; i++ ) {
		for(int j = 0; j < count; j++ ) {
			min_[i * count + j] = min_[j];
			max_[i * count + j] = max_[j];
			div_[i * count + j] = div_[j];
		}
	}
	Setup(dim, pop, min_, max_, 0, 0.7, 1, random_type);
	SolveStart(max_gens);
	round = 0;
}

void GeneticOptimizer::RefreshDim() {
	ASSERT(arraycount > 0 && count > 0);
	dim = arraycount * count;
	min_.SetCount(dim,0);
	max_.SetCount(dim,0);
	div_.SetCount(dim,0);
	desc_.SetCount(count);
	
}

void GeneticOptimizer::Start() {
	DiffSolver::GetTrialSolution();
	source = 0;
}

void GeneticOptimizer::Stop(double energy) {
	SetTrialEnergy(-1 * energy);
	SolveNext();
	round++;
}

void GeneticOptimizer::Best() {
	source = 1;
	/*for(int i = 0; i < solution->GetCount(); i++ ) {
		LOG("Best solution: " << i << " " << (*solution)[i]);
	}*/
}

double GeneticOptimizer::Get(int a, int i) {
	int pos = a * count + i;
	
	double value;
	if (source == 0) {
		value = trial_solution[pos];
	} else {
		value = best_solution[pos];
	}
	if (!use_limits) return value;
	
	if (value < min_[pos]) value = min_[pos];
	else if (value >= max_[pos]) value = max_[pos] - div_[pos];
	else {
		int count = value / div_[pos];
		value = count * div_[pos];
	}
	return value;
}

double GeneticOptimizer::Round(int pos, double value) {
	if (value < min_[pos]) value = min_[pos];
	else if (value >= max_[pos]) value = max_[pos] - div_[pos];
	else {
		int count = value / div_[pos];
		value = count * div_[pos];
	}
	return value;
}

Vector<String> GeneticOptimizer::GetLabels() {
	Vector<String> out;
	for(int i = 0; i < count; i++ )
		out.Add( desc_[i] );
	return out;
}

}
