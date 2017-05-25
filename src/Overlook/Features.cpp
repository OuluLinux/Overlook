#include "Overlook.h"


namespace Overlook {


FeatureDetector::FeatureDetector() {
	counted = 0;
	group_counter = 0;
}

void FeatureDetector::Init() {
	AddSubCore<ZigZag>();
}

void FeatureDetector::Start() {
	int bars = GetBars();
	ConstBuffer& keypoints = At(0).GetBuffer(1);
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	int counted_keypoints = this->keypoints.GetCount();
	
	for(int i = counted; i < bars; i++) {
		double d = keypoints.Get(i);
		
		if (d != 0.0) {
			// Collect statistics about distance between keypoints
			int kp_count = this->keypoints.GetCount();
			if (kp_count) {
				int distance = i - this->keypoints.GetKey(kp_count-1);
				keypoint_distance.AddResult(distance);
			}
			
			// Add new keypoint
			FeatureKeypoint& kp = this->keypoints.Add(i);
			kp.SetShift(i);
		}
	}
	
	if (counted_keypoints == this->keypoints.GetCount())
		return;
	
	// Gather keypoint data
	int distance = keypoint_distance.GetMean() + 0.5;
	if (distance % 2 == 0) distance++;
	int distance2 = distance / 2;
	Vector<double> data;
	data.SetCount(distance);
	
	for(int i = counted_keypoints; i < this->keypoints.GetCount(); i++) {
		FeatureKeypoint& kp = this->keypoints[i];
		int pos = this->keypoints.GetKey(i);
		
		
		// Fill data vector
		int begin = pos - distance2;
		int end   = pos + distance2 + 1;
		if (begin < 0 || end > open.GetCount()) {
			// Remove keypoints too close to the edge
			this->keypoints.Remove(i);
			i--;
			continue;
		}
		
		for(int j = begin, k = 0; j < end; j++, k++) {
			data[k] = open.Get(j);
		}
		
		// Process data vector
		kp.SetData(data);
		
		//LOG(i << ":\t" << pos << ":\t" << kp.ToString());
	}
	
	
	// Match keypoints
	for(int i = counted_keypoints; i < this->keypoints.GetCount(); i++) {
		
		// These keypoints are unprocessed, and they don't have group id yet.
		FeatureKeypoint& kp_a = this->keypoints[i];
		int kp_a_pos = this->keypoints.GetKey(i);
		int kp_a_id = i;
		
		// Don't test same groups again (tests only the initial keypoint, could be optimized more)
		Index<int> tested_groups;
		
		// Loop all possible pair keypoints
		for(int j = 0; j < i; j++) {
			
			// These keypoints are already processed, and they has a group id.
			FeatureKeypoint& kp_b = this->keypoints[j];
			int kp_b_id = j;
			int kp_b_pos = this->keypoints.GetKey(j);
			int joinable_group = kp_b.GetGroup();
			if (tested_groups.Find(joinable_group) != -1) continue;
			
			// Check if this keypoint matches to other keypoint
			if (kp_a.IsMatch(kp_b)) {
				
				//LOG(i << " matches " << j);
				//LOG("\t" << i << ":\t" << kp_a.ToString());
				//LOG("\t" << j << ":\t" << kp_b.ToString());
				
				// Join found keypoint in the existing group
				kp_a.SetGroup(joinable_group);
				
				Vector<int>& group_list = keypoint_groups.GetAdd(joinable_group);
				Vector<int>& group_pos_list = keypoint_group_pos.GetAdd(joinable_group);
				
				// Group is not added without at least one pair, so add first now, not earlier.
				if (group_list.IsEmpty()) {
					group_list.Add(kp_b_id);
					group_pos_list.Add(kp_b_pos);
				}
				
				// Add the found match
				group_list.Add(kp_a_id);
				group_pos_list.Add(kp_a_pos);
				
				break;
			} else {
				
				// Set this group tested
				tested_groups.Add(joinable_group);
			}
		}
		
		// If joinable group was not found
		if (kp_a.GetGroup() == -1) {
			
			// Create new group, but don't initialize lists yet without matching pair
			kp_a.SetGroup(group_counter);
			group_counter++;
		}
	}
	
	// Sort by biggest group
	struct Sorter {bool operator() (const Vector<int>& a, const Vector<int>& b) const {
		return a.GetCount() > b.GetCount();
	}};
	Sort(keypoint_groups, Sorter());
	
	
	// Get group impacts
	VectorMap<int, double> means;
	for(int i = 0; i < keypoint_groups.GetCount(); i++) {
		const Vector<int>& group = keypoint_groups[i];
		double mean = 0;
		for(int j = 0; j < group.GetCount(); j++) {
			FeatureKeypoint& kp = this->keypoints[group[j]];
			mean += kp.GetAbsChangeSum();
		}
		mean /= group.GetCount();
		means.Add(i, mean);
	}
	SortByValue(means, StdLess<double>());
	
	
	// Set impacts
	for(int i = 0; i < means.GetCount(); i++) {
		int group_id = means.GetKey(i);
		const Vector<int>& group = keypoint_groups[group_id];
		int impact = i * 4 / means.GetCount();
		for(int j = 0; j < group.GetCount(); j++) {
			FeatureKeypoint& kp = this->keypoints[group[j]];
			kp.SetImpact(impact);
		}
	}
	
	counted = bars;
}



#define DESC_COUNT 17
FeatureKeypoint::FeatureKeypoint() {
	group = -1;
	impact = 0;
	shift = -1;
}

void FeatureKeypoint::SetData(const Vector<double>& data) {
	ASSERT(data.GetCount() > 2);
	abssum = 0;
	
	// Linearly interpolate data values to fixed length vector
	descriptors.SetCount(DESC_COUNT);
	for(int i = 0; i < DESC_COUNT; i++) {
		double pos = (double)i * (data.GetCount() - 1.0) / (DESC_COUNT - 1.0);
		double int_part;
		double frac_part = modf(pos, &int_part);
		double value;
		int a = int_part;
		if (frac_part == 0.0) {
			value = data[a];
		} else {
			int b = a + 1;
			value = data[a] * frac_part + data[b] * (1.0 - frac_part);
		}
		
		abssum += fabs(value);
		descriptors[i] = value;
	}
	
	
	// Calculate base line (like regression line, but cheaper in this case...?)
	double av = 0.0;
	for(int i = 0; i < data.GetCount(); i++)
		av += data[i];
	av /= data.GetCount();
	
	double slope = 0.0;
	for(int i = 1; i < data.GetCount(); i++)
		slope += data[i] - data[i-1];
	slope /= data.GetCount() - 1;
	
	Vector<double> base;
	base.SetCount(DESC_COUNT);
	double len_2 = (descriptors.GetCount() - 1) / 2.0;
	double begin = av - len_2 * slope;
	for(int i = 0; i < DESC_COUNT; i++) {
		base[i] = begin + i * slope;
	}
	
	
	// Remove base from descriptors
	double low = DBL_MAX, high = -DBL_MAX;
	for(int i = 0; i < DESC_COUNT; i++) {
		double value = descriptors[i] - base[i];
		descriptors[i] = value;
		if (value < low) low = value;
		if (value > high) high = value;
	}
	double range = high - low;
	
	
	// Normalize values
	for(int i = 0; i < DESC_COUNT; i++) {
		double value = (descriptors[i] - low) / range;
		descriptors[i] = value;
	}
}

String FeatureKeypoint::ToString() const {
	String s;
	for(int i = 0; i < DESC_COUNT; i++) {
		s += Format("%4!,n   ", descriptors[i]);
	}
	return s;
}

bool FeatureKeypoint::IsMatch(const FeatureKeypoint& kp) const {
	double d = 0;
	for (int k = 0; k < DESC_COUNT; k++) {
		double value = descriptors[k] - kp.descriptors[k];
		d += value * value;
	}
	double limit = 0.1;
	d /= DESC_COUNT;
	return d <= limit;
}





}
