/*
	Differential Evolution
	an algorithm by Kenneth Price and Rainer Storn
	http://www1.icsi.berkeley.edu/~storn/code.html
*/

#ifndef _Overlook_DiffSolver_h_
#define _Overlook_DiffSolver_h_

#include <Core/Core.h>
using namespace Upp;

namespace Overlook {

class DiffSolver {
	typedef DiffSolver CLASSNAME;
	
public:
	enum {RAND_UNIFORM, RAND_NORMDIST};
	
	DiffSolver();

	void Serialize(Stream& s) {
		s % trial_solution % best_solution % pop_energy % population % dimension % population_count
		  % generations % strategy % scale % probability % trial_energy % best_energy
		  % idum % idum2 % iy % iv % generation % max_generations % candidate;
	}
	
	void	Setup(int dim,int pop_size, Vector<double>& a, Vector<double>& b, int de_strategy, double diff_scale ,double crossover_prob, int random_type=RAND_UNIFORM);
	int		Dimension() {return dimension;}
	int		Population() {return population_count;}
	double	Energy() {return(best_energy);}
	void	ResetBestEnergy() {best_energy = 0;}
	void	FactorBestEnergy(double f) {best_energy *= f;}
	int		Generations() {return generations;}
	void	SolveStart(int max_generations);
	bool	SolveCheck();
	void	SolveNext();
	void	SetTrialEnergy(double energy);
	Vector<double>& Solution() {return best_solution;}
	Vector<double>& GetTrialSolution();
	
	Vector<double> trial_solution;
	Vector<double> best_solution;
	Vector<double> pop_energy;
	Vector<Vector<double> > population;
	
	
protected:
	void SelectSamples(int candidate,int *r1,int *r2=0,int *r3=0, int *r4=0,int *r5=0);

	double RandomUniform(double min_value, double max_value);
	double RandomNormalDist(double mean, double stddev);

	int dimension;
	int population_count;
	int generations;
	int strategy;
	double scale;
	double probability;
	double trial_energy;
	double best_energy;
	int64 idum;
	int64 idum2;
	int64 iy;
	Vector<int64> iv;
	
	
private:
	void Best1Exp(int candidate);
	void Random1Exp(int candidate);
	void RandToBest1Exp(int candidate);
	void Best2Exp(int candidate);
	void Random2Exp(int candidate);
	void Best1Bin(int candidate);
	void Random1Bin(int candidate);
	void RandToBest1Bin(int candidate);
	void Best2Bin(int candidate);
	void Random2Bin(int candidate);

	int generation, max_generations;
	int candidate;

};

}

#endif
