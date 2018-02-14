#ifndef _Overlook_BitOptimizer_h_
#define _Overlook_BitOptimizer_h_

namespace Overlook {

class BitOptimizer {

	int w = 0, h = 0, d = 0;
	int max_gens = 0;
	int max_rounds = 0;
	int round = 0;
	int dimension = 0;
	int population_count = 0;
	int generations = 0;
	int strategy = StrategyBest1Exp;
	int generation = 0;
	int candidate = 0;
	double probability = 0.9;
	double trial_energy;
	double best_energy = 0.0;
	
	
	Vector<bool> trial_solution;
	Vector<bool> best_solution;
	Vector<double> pop_energy;
	Vector<Vector<bool> > population;
	
	
	void SelectSamples(int candidate,int *r1,int *r2=0,int *r3=0, int *r4=0,int *r5=0);

	double RandomUniform(double min_value, double max_value);
	double RandomNormalDist(double mean, double stddev);
	

public:
	typedef BitOptimizer CLASSNAME;
	BitOptimizer();
	
	void Serialize(Stream& s) {
		s % w % h % d % max_gens % max_rounds % round
		  % dimension % population_count % generations % strategy % generation % candidate
		  % probability % trial_energy % best_energy
		  % trial_solution % best_solution % pop_energy % population;
	}
	
	void Init(int w, int h, int d, int strategy=StrategyBest1Exp);
	
	void Start();
	void Stop(double energy);
	
	const Vector<bool>& GetTrialSolution() {return trial_solution;}
	const Vector<bool>& GetBestSolution() {return best_solution;}
	void GetLimitedTrialSolution(Vector<double>& solution);
	void GetLimitedBestSolution(Vector<double>& solution);
	
	int GetMaxRounds() const {return max_rounds;}
	int GetMaxGenerations() const {return max_gens;}
	int GetRound() const {return round;}
	bool IsEnd() const {return round >= max_rounds;}
	
	int		GetDimension() {return dimension;}
	int		GetPopulation() {return population_count;}
	double	Energy() {return(best_energy);}
	void	ResetBestEnergy() {best_energy = 0;}
	void	FactorBestEnergy(double f) {best_energy *= f;}
	int		Generations() {return generations;}
	void	SolveStart();
	bool	SolveInspect();
	void	SolveNext();
	void	NextTrialSolution();
	void	SetTrialEnergy(double energy);
	
	
	
	
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

	
};


}

#endif
