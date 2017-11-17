#ifndef _Overlook_Optimizer_h_
#define _Overlook_Optimizer_h_


namespace Overlook {


enum {
	StrategyBest1Exp,
	StrategyRandom1Exp,
	StrategyRandToBest1Exp,
	StrategyBest2Exp,
	StrategyRandom2Exp,
	StrategyBest1Bin,
	StrategyRandom1Bin,
	StrategyRandToBest1Bin,
	StrategyBest2Bin,
	StrategyRandom2Bin
};

class DiffSolver {
	typedef DiffSolver CLASSNAME;
	
public:
	enum {RAND_UNIFORM, RAND_NORMDIST};
	
	DiffSolver();

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
	
	void Serialize(Stream& s) {
		s % trial_solution % best_solution % pop_energy % population % dimension % population_count
		  % generations % strategy % scale % probability % trial_energy % best_energy
		  % idum % idum2 % iy % iv % generation % max_generations % candidate;
	}
	
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

class GeneticOptimizer : protected DiffSolver {

	int dim, pop, arraycount, count, source, max_gens, max_rounds, round;
	int random_type;
	Vector<double> min_, max_, div_;
	Vector<String> desc_;
	bool use_limits;
	
	void RefreshDim();

public:
	typedef GeneticOptimizer CLASSNAME;
	GeneticOptimizer();
	
	void Serialize(Stream& s) {
		DiffSolver::Serialize(s);
		s % dim % pop % arraycount % count % source % max_gens % max_rounds % round % random_type
		  % min_ % max_ % div_ % desc_ % use_limits;
	}
	
	void Init(int strategy=StrategyBest1Exp);
	void SetArrayCount(int i) {arraycount = i; }
	void SetCount(int i) {count = i; }
	void Set(int i, double minv, double maxv, double divider = 0, String desc = "" ) {
		ASSERT(i < count);
		if (min_.GetCount() != count*arraycount) RefreshDim();
		min_[i] = minv; max_[i] = maxv; div_[i] = divider; desc_[i] = desc;
	}
	void SetPopulation(int i) {pop = i;}
	void SetMaxGenerations(int i) {max_gens = i;}
	void UseLimits(bool b=true) {use_limits = b;}
	void SetRandomTypeUniform() {random_type = DiffSolver::RAND_UNIFORM;}
	void SetRandomTypeNormDist() {random_type = DiffSolver::RAND_NORMDIST;}
	
	void Start();
	void Stop(double energy);
	void Best();
	
	const Vector<double>& GetTrialSolution() {return trial_solution;}
	const Vector<double>& GetBestSolution() {return best_solution;}
	void GetLimitedTrialSolution(Vector<double>& solution);
	void GetLimitedBestSolution(Vector<double>& solution);
	
	int GetMaxRounds() const {return max_rounds;}
	int GetMaxGenerations() const {return max_gens;}
	int GetArrayCount() const {return arraycount;}
	int GetCount() const {return count;}
	int GetSize() const {return dim;}
	int GetPopulation() const {return pop;}
	int GetRound() const {return round;}
	bool IsEnd() const {return round >= max_rounds;}
	
	String GetLabel(int i) {return desc_[i];}
	Vector<String> GetLabels();
	double Round(int i, double value);
	
	double Get(int a, int i);
	double Get(int i) {return Get(0, i);}
	bool IsBest() {return source == 1;}
};

}

#endif
