#ifndef _Overlook_Features_h_
#define _Overlook_Features_h_

namespace Overlook {


class FeatureKeypoint : Moveable<FeatureKeypoint> {
	Vector<double> descriptors;
	int group, impact, shift;
	double abssum;
	
public:
	FeatureKeypoint();
	
	virtual void Serialize(Stream& s) {
		s % descriptors % group;
	}
	
	int GetGroup() const {return group;}
	String ToString() const;
	bool IsMatch(const FeatureKeypoint& kp) const;
	int GetImpact() const {return impact;}
	int GetShift() const {return shift;}
	double GetAbsChangeSum() const {return abssum;}
	
	void SetData(const Vector<double>& data);
	void SetGroup(int i) {group = i;}
	void SetImpact(int i) {impact = i;}
	void SetShift(int i) {shift = i;}
	
};

class FeatureDetector : public Pipe {
	
protected:
	friend class GroupFeatureDetector;
	//DataVar buf;
	Pipe *cont;
	int counted;
	VectorMap<int, FeatureKeypoint> keypoints;
	VectorMap<int, Vector<int> > keypoint_groups, keypoint_group_pos;
	OnlineVariance keypoint_distance;
	int group_counter;
	
protected:
	virtual void Start();
	
public:
	FeatureDetector();
	
	virtual void Serialize(Stream& s) {
		s % counted % keypoints % keypoint_groups % keypoint_group_pos
		  % keypoint_distance % group_counter;
	}
	
	virtual void Init();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	int GetKeypointCount() const {return keypoints.GetCount();}
	const FeatureKeypoint& GetKeypoint(int i) const {return keypoints[i];}
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class FeatureKeypointPair : Moveable<FeatureKeypointPair> {
	typedef Tuple2<int, int> KeypointSource;
	
public:
	FeatureKeypointPair() {}
	
	virtual void Serialize(Stream& s) {s % sources % max_matches % period % factor % keypoints;}
	
	void Add(int sym, int group) {KeypointSource& s = sources.Add(); s.a = sym; s.b = group;}
	
	Vector<KeypointSource> sources;
	int max_matches, period;
	double factor;
	Vector<int> keypoints;
	
};


}

#endif
