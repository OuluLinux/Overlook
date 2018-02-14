#include <random>
#include "Overlook.h"

namespace Overlook {

#define Element(a,b,c)  a[b][c]
#define RowVector(a,b)  a[b]
#define CopyVector(a,b) a <<= b

BitOptimizer::BitOptimizer() {
	
}

void BitOptimizer::Init(int w, int h, int d, int strategy) {
	this->w				= w;
	this->h				= h;
	this->d				= d;
	dimension			= w * h * d;
	population_count	= 1000;
	max_gens			= 1000;
	round				= 0;
	generations			= 0;
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
	for (int i=0; i < population_count; i++)
		for (int j=0; j < dimension; j++)
			Element(population,i,j) = Random(2);
	for (int i=0; i < population_count; i++)
		pop_energy[i] = DBL_MAX;
	for (int i=0; i < dimension; i++)
		best_solution[i] = 0.0;
	
	
	this->strategy	= strategy;
	probability		= 0.99;
	max_rounds		= max_gens * population_count;
	
	SolveStart();
}

void BitOptimizer::Start() {
	NextTrialSolution();
}

void BitOptimizer::Stop(double energy) {
	SetTrialEnergy(-1 * energy);
	SolveNext();
	round++;
}

void BitOptimizer::SolveStart() {
	best_energy = DBL_MAX;
	generation = 0;
	candidate = 0;
}

bool BitOptimizer::SolveInspect() {
	if (generation < max_gens) return 1;
	generations = generation;
	return 0;
}

void BitOptimizer::SolveNext() {
	candidate++;
	if (candidate >= population_count) {
		candidate=0;
		generation++;
	}
}

void BitOptimizer::NextTrialSolution() {
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
		default: Panic("no strategy in BitOptimizer");
	}
}

void BitOptimizer::SetTrialEnergy(double energy) {
	trial_energy = energy;
	if (trial_energy < pop_energy[candidate]) {
		// New low for this candidate
		pop_energy[candidate] = trial_energy;
		Vector<bool>* vec = &population[ candidate ];
		vec->SetCount(trial_solution.GetCount());
		Vector<bool>::Iterator i1, i2;
		for (i1 = vec->Begin(), i2 = trial_solution.Begin(); i2 != trial_solution.End(); i1++, i2++)
			*i1 = *i2;

		// Inspect if all-time low
		if (trial_energy <= best_energy) {
			best_energy = trial_energy;
			CopyVector(best_solution,trial_solution);
		}
	}
}


void BitOptimizer::Best1Exp(int candidate) {
	int r1, r2;
	int n;

	SelectSamples(candidate,&r1,&r2);
	n = Random(dimension);

	Vector<bool> *pop_ptr = &population[candidate];
	int c1 = pop_ptr->GetCount();
	trial_solution.SetCount(c1);
	for(int i = 0; i < c1; i++ )
		trial_solution[i] = (*pop_ptr)[i];
	
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] = best_solution[n] + (Element(population,r1,n) - Element(population,r2,n));
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::Random1Exp(int candidate) {
	int r1, r2, r3;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3);
	n = Random(dimension);

	CopyVector(trial_solution,RowVector(population, candidate));
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] = Element(population,r1,n) + (Element(population,r2,n) - Element(population,r3,n));
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::RandToBest1Exp(int candidate) {
	int r1, r2;
	int n;

	SelectSamples(candidate,&r1,&r2);
	n = Random(dimension);

	CopyVector(trial_solution,RowVector(population, candidate));
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] += (best_solution[n] - trial_solution[n]) + (Element(population,r1,n) - Element(population,r2,n));
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::Best2Exp(int candidate) {
	int r1, r2, r3, r4;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3,&r4);
	n = Random(dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] = best_solution[n]
									+ Element(population,r1,n)
									+ Element(population,r2,n)
									- Element(population,r3,n)
									- Element(population,r4,n);
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::Random2Exp(int candidate) {
	int r1, r2, r3, r4, r5;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3,&r4,&r5);
	n = Random(dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] = Element(population,r1,n)
									+ Element(population,r2,n)
									+ Element(population,r3,n)
									- Element(population,r4,n)
									- Element(population,r5,n);
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::Best1Bin(int candidate) {
	int r1, r2;
	int n;

	SelectSamples(candidate,&r1,&r2);
	n = Random(dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i == (dimension - 1)))
			trial_solution[n] = best_solution[n]
								+ Element(population,r1,n)
								- Element(population,r2,n);
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::Random1Bin(int candidate) {
	int r1, r2, r3;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3);
	n = Random(dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i  == (dimension - 1)))
			trial_solution[n] = Element(population,r1,n)
								+ Element(population,r2,n)
								- Element(population,r3,n);
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::RandToBest1Bin(int candidate) {
	int r1, r2;
	int n;

	SelectSamples(candidate,&r1,&r2);
	n = Random(dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i  == (dimension - 1)))
			trial_solution[n] += (best_solution[n] - trial_solution[n])
									+ (Element(population,r1,n)
									- Element(population,r2,n));
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::Best2Bin(int candidate) {
	int r1, r2, r3, r4;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3,&r4);
	n = Random(dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i  == (dimension - 1)))
			trial_solution[n] = best_solution[n]
									+ Element(population,r1,n)
									+ Element(population,r2,n)
									- Element(population,r3,n)
									- Element(population,r4,n);
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::Random2Bin(int candidate) {
	int r1, r2, r3, r4, r5;
	int n;

	SelectSamples(candidate,&r1,&r2,&r3,&r4,&r5);
	n = Random(dimension);

	CopyVector(trial_solution,RowVector(population,candidate));
	for (int i=0; i < dimension; i++) {
		if ((RandomUniform(0.0,1.0) < probability) || (i  == (dimension - 1)))
			trial_solution[n] = Element(population,r1,n)
										+ Element(population,r2,n)
										+ Element(population,r3,n)
										- Element(population,r4,n)
										- Element(population,r5,n);
		n = (n + 1) % dimension;
	}

	return;
}

void BitOptimizer::SelectSamples(int candidate,int *r1,int *r2, int *r3,int *r4,int *r5) {
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

double BitOptimizer::RandomUniform(double min_value,double max_value) {
	return min_value + Randomf() * (max_value - min_value);
}

}
