#ifndef _Overlook_Optimizer_h_
#define _Overlook_Optimizer_h_

#include <random>

namespace Forecast {

#define Element(a,b,c)  a[b][c]
#define RowVector(a,b)  a[b]
#define CopyVector(a,b) {\
	double* i1 = a; double* i2 = b;\
	for (int i = 0; i < dimension; i++, i1++, i2++) \
		*i1 = *i2;}
#define StrategyBest1Exp			0
#define StrategyRandom1Exp			1
#define StrategyRandToBest1Exp		2
#define StrategyBest2Exp			3
#define StrategyRandom2Exp			4
#define StrategyBest1Bin			5
#define StrategyRandom1Bin			6
#define StrategyRandToBest1Bin		7
#define StrategyBest2Bin			8
#define StrategyRandom2Bin			9


class Optimizer {
	
public:
	Vector<double> min_values, max_values;
	Vector<Vector<double> > trial_solutions;
	Vector<double> best_solution;
	Vector<double> pop_energy;
	Vector<Vector<double> > population;
	int dimension = 0;
	int popcount = 0;
	int round = 0;
	int random_type = 0;
	int max_rounds = 100;
	bool use_limits = 0;
	
	int generation = 0;
	int max_gens = 100;
	//int candidate = 0;
	int strategy = StrategyBest1Exp;
	double scale = 0.7;
	double probability = 0.9;
	double trial_energy = 0.0;
	double best_energy = 0.0;
	int64 idum;
	int64 idum2;
	int64 iy;
	Vector<int64> iv;
	int max_value = 100;
	int min_value = -100;
	
	
	
public:
	typedef Optimizer CLASSNAME;
	Optimizer() {}
	
	void Serialize(Stream& s) {
		s % min_values % max_values
		  % trial_solutions
		  % best_solution
		  % pop_energy
		  % population
		  % dimension
		  % popcount
		  % round
		  % random_type
		  % max_rounds
		  % use_limits
		  % generation
		  % max_gens
		  % strategy
		  % scale
		  % probability
		  % trial_energy
		  % best_energy
		  % idum
		  % idum2
		  % iy
		  % iv
		  % max_value
		  % min_value;
	}
	
	enum {RAND_UNIFORM, RAND_NORMDIST, RAND_MANUAL};
	
	
	
	int		Dimension() {return dimension;}
	int		Population() {return popcount;}
	double	Energy() {return(best_energy);}
	void	ResetBestEnergy() {best_energy = 0;}
	void	FactorBestEnergy(double f) {best_energy *= f;}
	int		Generations() {return generation;}
	Vector<double>& Min() {return min_values;}
	Vector<double>& Max() {return max_values;}
	
	void SolveStart() {
		best_energy = -DBL_MAX;
		generation = 0;
	}
	
	bool SolveCheck(){
		if (generation < max_gens)
			return 1;
		return 0;
	}
	
	void SetTrialEnergy(int candidate, double energy) {
		trial_energy = energy;
		
		if (trial_energy > pop_energy[candidate]) {
			
			// New high for this candidate
			pop_energy[candidate] = trial_energy;
			double* vec = population[ candidate ];
			double* trial_solution = trial_solutions[candidate];
			CopyVector(vec, trial_solution);
	
			// Check if all-time high
			if (trial_energy >= best_energy) {
				best_energy = trial_energy;
				CopyVector(best_solution, trial_solution);
			}
		}
	}
	
	double GetBestEnergy() {
		return best_energy;
	}
	
	double RandomUniform(double min_value, double max_value) {
		return min_value + Randomf() * (max_value - min_value);
	}
	
	double RandomUniform() {return RandomUniform(min_value, max_value);}
	
protected:
	void SelectSamples(int candidate,int *r1,int *r2=0,int *r3=0, int *r4=0,int *r5=0) {
		if (r1) {
			do {
				*r1 = (int)RandomUniform(0.0,(double)popcount);
			} while (*r1 == candidate);
		}
	
		if (r2) {
			do {
				*r2 = (int)RandomUniform(0.0,(double)popcount);
			} while ((*r2 == candidate) || (*r2 == *r1));
		}
	
		if (r3) {
			do {
				*r3 = (int)RandomUniform(0.0,(double)popcount);
			} while ((*r3 == candidate) || (*r3 == *r2) || (*r3 == *r1));
		}
	
		if (r4) {
			do {
				*r4 = (int)RandomUniform(0.0,(double)popcount);
			} while ((*r4 == candidate) || (*r4 == *r3) || (*r4 == *r2) || (*r4 == *r1));
		}
	
		if (r5) {
			do {
				*r5 = (int)RandomUniform(0.0,(double)popcount);
			} while ((*r5 == candidate) || (*r5 == *r4) || (*r5 == *r3)
					 || (*r5 == *r2) || (*r5 == *r1));
		}
	
		return;
	}
	


	
private:
	
	void Best1Exp(int candidate, double* trial_solution) {
		int r1, r2;
		int n;
	
		SelectSamples(candidate,&r1,&r2);
		n = (int)RandomUniform(0.0,(double)dimension);
	
		CopyVector(trial_solution, RowVector(population, candidate));
		for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
			trial_solution[n] = best_solution[n]
							   + scale * (Element(population, r1, n)
										  - Element(population, r2, n));
			n = (n + 1) % dimension;
		}
	
		return;
	}
	
	void Random1Exp(int candidate, double* trial_solution) {
		int r1, r2, r3;
		int n;
	
		SelectSamples(candidate,&r1,&r2,&r3);
		n = (int)RandomUniform(0.0,(double)dimension);
	
		CopyVector(trial_solution, RowVector(population, candidate));
		for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
			trial_solution[n] = Element(population,r1,n)
							   + scale * (Element(population,r2,n)
										  - Element(population,r3,n));
			n = (n + 1) % dimension;
		}
	
		return;
	}
	
	void RandToBest1Exp(int candidate, double* trial_solution) {
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
	
	void Best2Exp(int candidate, double* trial_solution) {
		int r1, r2, r3, r4;
		int n;
	
		SelectSamples(candidate,&r1,&r2,&r3,&r4);
		n = (int)RandomUniform(0.0,(double)dimension);
	
		CopyVector(trial_solution, RowVector(population,candidate));
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
	
	void Random2Exp(int candidate, double* trial_solution) {
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
	
	void Best1Bin(int candidate, double* trial_solution) {
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
	
	void Random1Bin(int candidate, double* trial_solution) {
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
	
	void RandToBest1Bin(int candidate, double* trial_solution) {
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
	
	void Best2Bin(int candidate, double* trial_solution) {
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
	
	void Random2Bin(int candidate, double* trial_solution) {
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
	



public:
	
	void Init(int dimension, int popcount, int strategy=StrategyBest1Exp) {
		this->popcount = popcount;
		this->dimension = dimension;
		this->strategy = strategy;
		
		max_rounds = max_gens * popcount;
		
		best_solution.SetCount(dimension, 0);
		pop_energy.SetCount(popcount, 0);
		population.SetCount(popcount);
		for(int i = 0; i < population.GetCount(); i++)
			population[i].SetCount(dimension);
		trial_solutions.SetCount(popcount);
		for(int i = 0; i < trial_solutions.GetCount(); i++)
			trial_solutions[i].SetCount(dimension);
		
		
		for(int i = 0; i < popcount; i++)
			for(int j = 0; j < dimension; j++)
				trial_solutions[i][j] = 0;
		
		min_values.SetCount(dimension, 0);
		max_values.SetCount(dimension, 0);
		for(int i = 0; i < dimension; i++) {
			min_values[i] = min_value;
			max_values[i] = max_value;
		}
	
		for(int i = 0; i < dimension; i++ ) {
			best_solution[i] = 0;
		}
		
		for(int i = 0; i < popcount; i++ ) {
			pop_energy[i] = 0;
			for(int j = 0; j < dimension; j++ )
				population[i][j] = 0;
		}
		
		if (random_type == RAND_UNIFORM) {
			for (int i = 0; i < popcount; i++)
				for (int j = 0; j < dimension; j++)
					Element(population,i,j) = RandomUniform(min_values[j], max_values[j]);
		}
		else if (random_type == RAND_NORMDIST) {
			for (int j = 0; j < dimension; j++) {
				std::random_device rd;
			    std::mt19937 gen(rd());
			    std::normal_distribution<> d(min_values[j], max_values[j]); // mean, stddev
				for (int i = 0; i < popcount; i++)
					Element(population,i,j) = d(gen);
			}
		}
		else if (random_type == RAND_MANUAL) {
			
		}
		else Panic("Invalid random type");
		
		
		for (int i=0; i < popcount; i++)
			pop_energy[i] = -DBL_MAX;
	
		for (int i=0; i < dimension; i++)
			best_solution[i] = 0.0;
		
		
		SolveStart();
		round = 0;
	}
	
	void SetRandomTypeUniform() {random_type = RAND_UNIFORM;}
	void SetRandomTypeNormDist() {random_type = RAND_NORMDIST;}
	void SetRandomTypeManual() {random_type = RAND_MANUAL;}
	
	void SetPopulation(int i, const Vector<double>& solution, double result) {
		pop_energy[i] = result;
		population[i] <<= solution;
	}
	
	void Start() {
		for(int i = 0; i < popcount; i++) {
			GenerateTrialSolution(i, trial_solutions[i]);
		}
	}
	
	void Stop(double* energy) {
		for(int i = 0; i < popcount; i++)
			SetTrialEnergy(i, energy[i]);
		round += popcount;
		generation++;
	}
	
	void GenerateTrialSolution(int candidate, double* trial_solution) {
		switch (strategy) {
			case StrategyBest1Exp:			Best1Exp(candidate, trial_solution); break;
			case StrategyRandom1Exp:		Random1Exp(candidate, trial_solution); break;
			case StrategyRandToBest1Exp:	RandToBest1Exp(candidate, trial_solution); break;
			case StrategyBest2Exp:			Best2Exp(candidate, trial_solution); break;
			case StrategyRandom2Exp:		Random2Exp(candidate, trial_solution); break;
			case StrategyBest1Bin:			Best1Bin(candidate, trial_solution); break;
			case StrategyRandom1Bin:		Random1Bin(candidate, trial_solution); break;
			case StrategyRandToBest1Bin:	RandToBest1Bin(candidate, trial_solution); break;
			case StrategyBest2Bin:			Best2Bin(candidate, trial_solution); break;
			case StrategyRandom2Bin:		Random2Bin(candidate, trial_solution); break;
			default: Panic("no strategy in DiffSolver");
		}
	}
	
	const Vector<double>& GetTrialSolution(int i) {
		return trial_solutions[i];
	}
	
	const Vector<double>& GetBestSolution() {return best_solution;}
	
	int GetMaxRounds() const {return max_rounds;}
	int GetMaxGenerations() const {return max_gens;}
	int GetSize() const {return dimension;}
	int GetPopulation() const {return popcount;}
	int GetRound() const {return round;}
	bool IsEnd() const {return round >= max_rounds;}
	
	void SetMaxGenerations(int i) {max_gens = i;}
	
};

}

#endif
