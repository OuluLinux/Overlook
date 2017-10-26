#ifndef _Overlook_Optimizer_h_
#define _Overlook_Optimizer_h_

#include "DiffSolver.h"

namespace Overlook {
	
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
	
	void Init();
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
	
	int GetMaxRounds() const {return max_rounds;}
	int GetMaxGenerations() const {return max_gens;}
	int GetArrayCount() const {return arraycount;}
	int GetCount() const {return count;}
	int GetSize() const {return dim;}
	int GetPopulation() const {return pop;}
	int GetRound() const {return round;}
	
	String GetLabel(int i) {return desc_[i];}
	Vector<String> GetLabels();
	double Round(int i, double value);
	
	double Get(int a, int i);
	double Get(int i) {return Get(0, i);}
	bool IsBest() {return source == 1;}
};

}

#endif
