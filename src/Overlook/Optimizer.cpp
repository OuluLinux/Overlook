#include <random>
#include "Overlook.h"

namespace Overlook {

#define Element(a,b,c)  a[b][c]
#define RowVector(a,b)  a[b]
#define CopyVector(a,b) a <<= b

DiffSolver::DiffSolver() :
	generations(0), strategy(StrategyBest1Exp),
	scale(0.7), probability(0.9), best_energy(0.0) {
	iv.SetCount(32, 0);
	candidate = 0;
}

void DiffSolver::Setup(int dim, int pop_size, Vector<double>& a, Vector<double>& b,
					 int de_strategy, double diff_scale, double crossover_prob, int random_type) {
	dimension = dim;
	population_count = pop_size;

	
	trial_solution	.SetCount(dimension);
	best_solution	.SetCount(dimension);
	pop_energy		.SetCount(population_count);
	population		.SetCount(population_count);
	
	for(int i = 0; i < dimension; i++ ) {
		trial_solution[i] = 0;
		best_solution[i] = 0;
	}
	for(int i = 0; i < population_count; i++ ) {
		pop_energy[i] = 0;
		population[i].SetCount(dimension);
		for(int j = 0; j < dimension; j++ )
			population[i][j] = 0;
	}
	
	strategy	= de_strategy;
	scale		= diff_scale;
	probability = crossover_prob;
	
	if (random_type == RAND_UNIFORM) {
		for (int i=0; i < population_count; i++)
			for (int j=0; j < dimension; j++)
				Element(population,i,j) = RandomUniform(a[j],b[j]);
	}
	else if (random_type == RAND_NORMDIST) {
		for (int j=0; j < dimension; j++) {
			std::random_device rd;
		    std::mt19937 gen(rd());
		    std::normal_distribution<> d(a[j], b[j]); // mean, stddev
			for (int i=0; i < population_count; i++)
				Element(population,i,j) = d(gen);
		}
	}
	else Panic("Invalid random type");
	
	
	for (int i=0; i < population_count; i++)
		pop_energy[i] = DBL_MAX;

	for (int i=0; i < dimension; i++)
		best_solution[i] = 0.0;
	
	return;
}

void DiffSolver::SolveStart(int max_generations) {
	best_energy = DBL_MAX;
	generation = 0;
	candidate = 0;
	this->max_generations = max_generations;
}

bool DiffSolver::SolveInspect() {
	if (generation < max_generations) return 1;
	generations = generation;
	return 0;
}

void DiffSolver::SolveNext() {
	candidate++;
	if (candidate >= population_count) {
		candidate=0;
		generation++;
	}
}

Vector<double>& DiffSolver::GetTrialSolution() {
	switch (strategy) {
		case StrategyBest1Exp:			Best1Exp(candidate); break;
		case StrategyRandom1Exp:		Random1Exp(candidate); break;
		case StrategyRandToBest1Exp:	RandToBest1Exp(candidate); break;
		case StrategyBest2Exp:			Best2Exp(candidate); break;
		case StrategyRandom2Exp:		Random2Exp(candidate); break;
		case StrategyBest1Bin:			Best1Bin(candidate); break;
		case StrategyRandom1Bin:		Random1Bin(candidate); break;
		case StrategyRandToBest1Bin:	RandToBest1Bin(candidate); break;
		case StrategyBest2Bin:			Best2Bin(candidate); break;
		case StrategyRandom2Bin:		Random2Bin(candidate); break;
		default: Panic("no strategy in DiffSolver");
	}
	return trial_solution;
}

void DiffSolver::SetTrialEnergy(double energy) {
	trial_energy = energy;
	if (trial_energy < pop_energy[candidate]) {
		// New low for this candidate
		pop_energy[candidate] = trial_energy;
		Vector<double>* vec = &population[ candidate ];
		vec->SetCount(trial_solution.GetCount());
		Vector<double>::Iterator i1, i2;
		for (i1 = vec->Begin(), i2 = trial_solution.Begin(); i2 != trial_solution.End(); i1++, i2++)
			*i1 = *i2;

		// Inspect if all-time low
		if (trial_energy <= best_energy) {
			best_energy = trial_energy;
			CopyVector(best_solution,trial_solution);
		}
	}
}


void DiffSolver::Best1Exp(int candidate) {
	int r1, r2;
	int n;

	SelectSamples(candidate,&r1,&r2);
	n = (int)RandomUniform(0.0,(double)dimension);

	Vector<double> *pop_ptr = &population[candidate];
	int c1 = pop_ptr->GetCount();
	trial_solution.SetCount(c1);
	for(int i = 0; i < c1; i++ )
		trial_solution[i] = (*pop_ptr)[i];
	
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] = best_solution[n]
						   + scale * (Element(population,r1,n)
									  - Element(population,r2,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::Random1Exp(int candidate) {
	int r1, r2, r3;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3);
	n = (int)RandomUniform(0.0,(double)dimension);

	CopyVector(trial_solution,RowVector(population, candidate));
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] = Element(population,r1,n)
						   + scale * (Element(population,r2,n)
									  - Element(population,r3,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::RandToBest1Exp(int candidate) {
	int r1, r2;
	int n;

	SelectSamples(candidate,&r1,&r2);
	n = (int)RandomUniform(0.0,(double)dimension);

	CopyVector(trial_solution,RowVector(population, candidate));
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] += scale * (best_solution[n] - trial_solution[n])
							+ scale * (Element(population,r1,n)
									   - Element(population,r2,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::Best2Exp(int candidate) {
	int r1, r2, r3, r4;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3,&r4);
	n = (int)RandomUniform(0.0,(double)dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] = best_solution[n] +
						   scale * (Element(population,r1,n)
									+ Element(population,r2,n)
									- Element(population,r3,n)
									- Element(population,r4,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::Random2Exp(int candidate) {
	int r1, r2, r3, r4, r5;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3,&r4,&r5);
	n = (int)RandomUniform(0.0,(double)dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] = Element(population,r1,n)
						   + scale * (Element(population,r2,n)
									  + Element(population,r3,n)
									  - Element(population,r4,n)
									  - Element(population,r5,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::Best1Bin(int candidate) {
	int r1, r2;
	int n;

	SelectSamples(candidate,&r1,&r2);
	n = (int)RandomUniform(0.0,(double)dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i == (dimension - 1)))
			trial_solution[n] = best_solution[n]
							   + scale * (Element(population,r1,n)
										  - Element(population,r2,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::Random1Bin(int candidate) {
	int r1, r2, r3;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3);
	n = (int)RandomUniform(0.0,(double)dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i  == (dimension - 1)))
			trial_solution[n] = Element(population,r1,n)
							   + scale * (Element(population,r2,n)
										  - Element(population,r3,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::RandToBest1Bin(int candidate) {
	int r1, r2;
	int n;

	SelectSamples(candidate,&r1,&r2);
	n = (int)RandomUniform(0.0,(double)dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i  == (dimension - 1)))
			trial_solution[n] += scale * (best_solution[n] - trial_solution[n])
								+ scale * (Element(population,r1,n)
										   - Element(population,r2,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::Best2Bin(int candidate) {
	int r1, r2, r3, r4;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3,&r4);
	n = (int)RandomUniform(0.0,(double)dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i  == (dimension - 1)))
			trial_solution[n] = best_solution[n]
							   + scale * (Element(population,r1,n)
										  + Element(population,r2,n)
										  - Element(population,r3,n)
										  - Element(population,r4,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::Random2Bin(int candidate) {
	int r1, r2, r3, r4, r5;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3,&r4,&r5);
	n = (int)RandomUniform(0.0,(double)dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i  == (dimension - 1)))
			trial_solution[n] = Element(population,r1,n)
							   + scale * (Element(population,r2,n)
										  + Element(population,r3,n)
										  - Element(population,r4,n)
										  - Element(population,r5,n));
		n = (n + 1) % dimension;
	}

	return;
}

void DiffSolver::SelectSamples(int candidate,int *r1,int *r2, int *r3,int *r4,int *r5) {
	if (r1) {
		do {
			*r1 = (int)RandomUniform(0.0,(double)population_count);
		} while (*r1 == candidate);
	}

	if (r2) {
		do {
			*r2 = (int)RandomUniform(0.0,(double)population_count);
		} while ((*r2 == candidate) || (*r2 == *r1));
	}

	if (r3) {
		do {
			*r3 = (int)RandomUniform(0.0,(double)population_count);
		} while ((*r3 == candidate) || (*r3 == *r2) || (*r3 == *r1));
	}

	if (r4) {
		do {
			*r4 = (int)RandomUniform(0.0,(double)population_count);
		} while ((*r4 == candidate) || (*r4 == *r3) || (*r4 == *r2) || (*r4 == *r1));
	}

	if (r5) {
		do {
			*r5 = (int)RandomUniform(0.0,(double)population_count);
		} while ((*r5 == candidate) || (*r5 == *r4) || (*r5 == *r3)
				 || (*r5 == *r2) || (*r5 == *r1));
	}

	return;
}

double DiffSolver::RandomUniform(double min_value,double max_value) {
	return min_value + Randomf() * (max_value - min_value);
}



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

void GeneticOptimizer::Init(int strategy) {
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
	Setup(dim, pop, min_, max_, strategy, 0.7, 1, random_type);
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

void GeneticOptimizer::GetLimitedTrialSolution(Vector<double>& solution) {
	solution.SetCount(trial_solution.GetCount());
	
	for(int i = 0; i < solution.GetCount(); i++) {
		double value = trial_solution[i];
		
		if (value < min_[i]) value = min_[i];
		else if (value >= max_[i]) value = max_[i];
		
		solution[i] = value;
	}
}

void GeneticOptimizer::GetLimitedBestSolution(Vector<double>& solution) {
	solution.SetCount(best_solution.GetCount());
	
	for(int i = 0; i < solution.GetCount(); i++) {
		double value = best_solution[i];
		
		if (value < min_[i]) value = min_[i];
		else if (value >= max_[i]) value = max_[i];
		
		solution[i] = value;
	}
}

}
