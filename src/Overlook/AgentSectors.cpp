#include "Overlook.h"

#define DLOG(x) LOG(x)

namespace Overlook {

void AgentSystem::RefreshClusters() {
	TimeStop ts;
	RefreshResultClusters();
	RefreshIndicatorClusters();
	DLOG("Clusters refreshed in " << ts.ToString());
}

void AgentSystem::RefreshResultClusters() {
	
	if (initial_result_clustering) {
		VectorMap<ResultTuple, int> result_stats;
		result_stats.Reserve(snaps.GetCount() * SYM_COUNT * MEASURE_PERIODCOUNT);
		
		for(int i = 0; i < snaps.GetCount(); i++) {
			const Snapshot& snap = snaps[i];
			for(int j = 0; j < SYM_COUNT; j++)
				for(int k = 0; k < MEASURE_PERIODCOUNT; k++)
					result_stats.GetAdd(snap.GetResultTuple(j, k), 0)++;
		}
		
		// Sort values by "x"
		SortByKey(result_stats, StdLess<ResultTuple>());
		
		
		// Debug print
		#ifdef flagDEBUG
		for (int i = 0; i < result_stats.GetCount(); i++) {
			const ResultTuple& t = result_stats.GetKey(i);
			DLOG(Format("%d x %d, %d", t.volat, t.change, result_stats[i]));
		}
		#endif
		
		
		// Add initial centroids uniformily
		int cols = sqrt(GROUP_COUNT);
		int rows = GROUP_COUNT / cols;
		int max_x = result_stats.TopKey().volat;
		int xstep = max_x / cols;
		int cur = 0;
		ASSERT(cols * rows == GROUP_COUNT);
		
		for(int i = 0; i < cols; i++) {
			int x = xstep / 2 + i * xstep;
			int x1 = i * xstep;
			int x2 = (i + 1) * xstep;
			
			int min_y = INT_MAX;
			int max_y = INT_MIN;
			while (cur < result_stats.GetCount()) {
				const ResultTuple& t = result_stats.GetKey(cur++);
				if (t.volat >= x2) break;
				if (t.change < min_y) min_y = t.change;
				if (t.change > max_y) max_y = t.change;
			}
			
			int range_y = max_y - min_y;
			int ystep = range_y / rows;
			
			for(int j = 0; j < rows; j++) {
				int y = min_y + ystep / 2 + j * ystep;
				result_centroids.Add(ResultTuple(x, y));
			}
		}
		
		// Debug print centroids
		#ifdef flagDEBUG
		for(int i = 0; i < result_centroids.GetCount(); i++) {
			const ResultTuple& ctr = result_centroids[i];
			double v = ctr.volat * VOLAT_DIV;
			double c = ctr.change * CHANGE_DIV * 1000;
			DLOG("Center " << i << ": " << v << " x " << c);
		}
		#endif
		
		
		// Add points to vectors of closest centroids
		Vector<Vector<ResultTupleCounter > > pts;
		pts.SetCount(result_centroids.GetCount());
		
		for (int i = 0; i < result_stats.GetCount(); i++) {
			const ResultTuple& t = result_stats.GetKey(i);
			int t_count = result_stats[i];
			
			// Find closest centroid to the point
			int c_id = -1;
			int min_dist = INT_MAX;
			for(int j = 0; j < result_centroids.GetCount(); j++) {
				const ResultTuple& ctr = result_centroids[j];
				
				// Calculate Euclidean distance
				int vd = (ctr.volat - t.volat) * VOLINTMUL;
				int cd = ctr.change - t.change;
				int dist = root(vd * vd + cd * cd);
				if (dist < min_dist) {
					min_dist = dist;
					c_id = j;
				}
			}
			
			// Add point to centroid's point vector and update mean average
			Vector<ResultTupleCounter >& c_pts = pts[c_id];
			c_pts.Add(ResultTupleCounter(t, t_count));
		}
		
		
		// Calculate average centroids after initial point locating
		for(int i = 0; i < result_centroids.GetCount(); i++) {
			Vector<ResultTupleCounter >& c_pts = pts[i];
			if (c_pts.IsEmpty()) {
				for (int j = 1; j < result_centroids.GetCount(); j++) {
					int k = (i + j) % result_centroids.GetCount();
					Vector<ResultTupleCounter >& c_pts2 = pts[k];
					if (c_pts2.GetCount() >= 2) {
						c_pts.Add(c_pts2.Pop());
						break;
					}
				}
			}
				
			ResultTuple av_pt;
			int64 total = 0;
			for(int l = 0; l < c_pts.GetCount(); l++) {
				ResultTupleCounter& c_pt = c_pts[l];
				av_pt.volat += c_pt.volat * c_pt.count;
				av_pt.change += c_pt.change * c_pt.count;
				total += c_pt.count;
			}
			av_pt.volat /= total;
			av_pt.change /= total;
			result_centroids[i] = av_pt;
		}
		
		
		// Debug print
		#ifdef flagDEBUG
		for(int j = 0; j < result_centroids.GetCount(); j++) {
			const ResultTuple& ctr = result_centroids[j];
			double v = ctr.volat * VOLAT_DIV;
			double c = ctr.change * CHANGE_DIV * 1000;
			DLOG("After first loop, centroid " << j << ": " << v << " x " << c);
		}
		DLOG("");
		#endif
		
		
		// Iteratively move points to closest centroid vectors and recalculate centroids
		bool changes = true;
		for(int i = 0; i < 100 && changes; i++) {
			
			// Move points to closest centroid vectors
			changes = false;
			for(int j = 0; j < pts.GetCount(); j++) {
				Vector<ResultTupleCounter >& c1_pts = pts[j];
				
				for(int k = 0; k < c1_pts.GetCount() && c1_pts.GetCount() > 1; k++) {
					const ResultTupleCounter& t = c1_pts[k];
					
					// Find closest centroid to the point
					int c_id = -1;
					int min_dist = INT_MAX;
					for(int l = 0; l < result_centroids.GetCount(); l++) {
						const ResultTuple& ctr = result_centroids[l];
						
						// Calculate Euclidean distance
						int vd = (ctr.volat - t.volat) * VOLINTMUL;
						int cd = ctr.change - t.change;
						int dist = root(vd * vd + cd * cd);
						if (dist < min_dist) {
							min_dist = dist;
							c_id = l;
						}
					}
					
					// Continue if the closest centroid is same
					if (c_id == j) continue;
					changes = true;
					
					// Move point to centroid's point vector and update mean average
					Vector<ResultTupleCounter >& c2_pts = pts[c_id];
					c2_pts.Add(t);
					c1_pts.Remove(k--);
				}
			}
			
			
			// Recalculate centroids
			for(int j = 0; j < pts.GetCount(); j++) {
				Vector<ResultTupleCounter >& c_pts = pts[j];
				ResultTuple av_pt;
				int64 total = 0;
				for(int l = 0; l < c_pts.GetCount(); l++) {
					ResultTupleCounter& c_pt = c_pts[l];
					av_pt.volat += c_pt.volat * c_pt.count;
					av_pt.change += c_pt.change * c_pt.count;
					total += c_pt.count;
				}
				av_pt.volat /= total;
				av_pt.change /= total;
				result_centroids[j] = av_pt;
			}
			
			// Debug print
			#ifdef flagDEBUG
			for(int j = 0; j < result_centroids.GetCount(); j++) {
				const ResultTuple& ctr = result_centroids[j];
				double v = ctr.volat * VOLAT_DIV;
				double c = ctr.change * CHANGE_DIV * 1000;
				DLOG("Iter " << i << ": Center " << j << ": " << v << " x " << c);
			}
			DLOG("");
			#endif
		}
		
		initial_result_clustering = false;
	}
	
	TimeStop ts;
	
	// Write clusters to snapshots
	for (; result_cluster_counter < snaps.GetCount(); result_cluster_counter++) {
		Snapshot& snap = snaps[result_cluster_counter];
		
		for(int i = 0; i < SYM_COUNT; i++) {
			for(int j = 0; j < MEASURE_PERIODCOUNT; j++) {
				int v = snap.GetPeriodVolatilityInt(i, j);
				int c = snap.GetPeriodChangeInt(i, j);
				
				// Find closest centroid to the point
				int c_id = -1;
				int min_dist = INT_MAX;
				for(int l = 0; l < result_centroids.GetCount(); l++) {
					const ResultTuple& ctr = result_centroids[l];
					
					// Calculate Euclidean distance
					int vd = (ctr.volat - v) * VOLINTMUL;
					int cd = ctr.change - c;
					int dist = root(vd * vd + cd * cd);
					if (dist < min_dist) {
						min_dist = dist;
						c_id = l;
					}
				}
				
				//LOG(result_cluster_counter << "\t" << i << "\t" << j << "\t" << c_id);
				snap.SetResultCluster(i, j, c_id);
			}
		}
	}

	LOG("Cluster ids written in " << ts.ToString());
	
}










void AgentSystem::RefreshIndicatorClusters() {
	TimeStop ts;
	
	if (initial_indicator_clustering) {
		
		// Find indi value min/max
		IndicatorTuple min, max;
		for(int i = 0; i < SENSOR_SIZE; i++)
			min.values[i] = 255;
		
		for(int i = 0; i < snaps.GetCount(); i++) {
			const IndicatorTuple& it = snaps[i].GetIndicatorTuple();
			for(int j = 0; j < SENSOR_SIZE; j++) {
				uint8 v = it.values[j];
				if (v < min.values[j]) min.values[j] = v;
				if (v > max.values[j]) max.values[j] = v;
			}
		}
		byte mid[SENSOR_SIZE];
		for(int i = 0; i < SENSOR_SIZE; i++)
			mid[i] = (max.values[i] + min.values[i]) / 2;
		
		
		// Add initial centroids uniformily
		int cols = SENSOR_SIZE;
		int rows = INPUT_COUNT / cols;
		int extrarow_cols = (INPUT_COUNT - (cols * rows)) % cols;
		
		int ic = 0;
		indi_centroids.SetCount(SENSOR_SIZE);
		
		int row = 0, col = 0;
		for (int ic = 0; ic < SENSOR_SIZE; ic++) {
			IndicatorTuple& t = indi_centroids[ic];
			int this_rows = rows + (col < extrarow_cols ? 1 : 0);
			
			for(int i = 0; i < cols; i++) {
				if (i == col) {
					double ystep = Upp::max(1.0, (double)(max.values[i] - min.values[i]) / this_rows);
					t.values[i] = min.values[i] + row * ystep;
				}
				else
					t.values[i] = mid[i];
			}
			
			row++;
			if (row >= this_rows) {
				row = 0;
				col++;
			}
		}
		
		
		// Debug print centroids
		#ifdef flagDEBUG
		for(int i = 0; i < indi_centroids.GetCount(); i++) {
			DLOG("Center " << i << ": " << indi_centroids[i].ToString());
		}
		#endif
		
		
		// Add points to vectors of closest centroids
		Vector<Vector<int> > pts;
		pts.SetCount(indi_centroids.GetCount());
		
		for (int i = 0; i < snaps.GetCount(); i++) {
			const IndicatorTuple& it = snaps[i].GetIndicatorTuple();
			
			// Find closest centroid to the point
			int c_id = -1;
			int min_dist = INT_MAX;
			for(int j = 0; j < indi_centroids.GetCount(); j++) {
				const IndicatorTuple& ctr = indi_centroids[j];
				
				// Calculate Euclidean distance
				int dist = ctr.GetDistance(it);
				if (dist < min_dist) {
					min_dist = dist;
					c_id = j;
				}
			}
			
			// Add point to centroid's point vector and update mean average
			pts[c_id].Add(i);
		}
		
		
		// Calculate average centroids after initial point locating
		for(int i = 0; i < indi_centroids.GetCount(); i++) {
			IndicatorTuple& it = indi_centroids[i];
			
			Vector<int>& c_pts = pts[i];
			if (c_pts.IsEmpty()) {
				for (int j = 1; j < indi_centroids.GetCount(); j++) {
					int k = (i + j) % indi_centroids.GetCount();
					Vector<int>& c_pts2 = pts[k];
					if (c_pts2.GetCount() >= 2) {
						c_pts.Add(c_pts2.Pop());
						break;
					}
				}
			}
				
			int64 av_pt[SENSOR_SIZE];
			for(int i = 0; i < SENSOR_SIZE; i++) av_pt[i] = 0;
			
			for(int l = 0; l < c_pts.GetCount(); l++) {
				const IndicatorTuple& c_pt = snaps[c_pts[l]].GetIndicatorTuple();
				for(int j = 0; j < SENSOR_SIZE; j++)
					av_pt[j] += c_pt.values[j];
			}
			for(int j = 0; j < SENSOR_SIZE; j++)
				it.values[j] = av_pt[j] / c_pts.GetCount();
		}
		
		
		// Debug print
		#ifdef flagDEBUG
		for(int j = 0; j < indi_centroids.GetCount(); j++) {
			DLOG("After first loop, centroid " << j << " " << indi_centroids[j].ToString());
		}
		DLOG("");
		#endif
		
		
		// Iteratively move points to closest centroid vectors and recalculate centroids
		bool changes = true;
		int iters = 100;
		#ifdef flagDEBUG
		iters = 3;
		#endif
		
		for(int i = 0; i < iters && changes; i++) {
			
			// Move points to closest centroid vectors
			changes = false;
			for(int j = 0; j < pts.GetCount(); j++) {
				Vector<int>& c1_pts = pts[j];
				
				for(int k = 0; k < c1_pts.GetCount() && c1_pts.GetCount() > 1; k++) {
					int snap_id = c1_pts[k];
					const IndicatorTuple& it = snaps[snap_id].GetIndicatorTuple();
					
					// Find closest centroid to the point
					int c_id = -1;
					int min_dist = INT_MAX;
					for(int l = 0; l < indi_centroids.GetCount(); l++) {
						const IndicatorTuple& ctr = indi_centroids[l];
						
						// Calculate Euclidean distance
						int dist = ctr.GetDistance(it);
						if (dist < min_dist) {
							min_dist = dist;
							c_id = l;
						}
					}
					
					// Continue if the closest centroid is same
					if (c_id == j) continue;
					changes = true;
					
					// Move point to centroid's point vector and update mean average
					Vector<int>& c2_pts = pts[c_id];
					c2_pts.Add(snap_id);
					c1_pts.Remove(k--);
				}
			}
			
			
			// Recalculate centroids
			for(int j = 0; j < pts.GetCount(); j++) {
				IndicatorTuple& it = indi_centroids[j];
				int64 av_pt[SENSOR_SIZE];
				for(int i = 0; i < SENSOR_SIZE; i++) av_pt[i] = 0;
				
				const Vector<int>& c_pts = pts[j];
				
				for(int l = 0; l < c_pts.GetCount(); l++) {
					const IndicatorTuple& c_pt = snaps[c_pts[l]].GetIndicatorTuple();
					for(int k = 0; k < SENSOR_SIZE; k++)
						av_pt[k] += c_pt.values[k];
				}
				for(int k = 0; k < SENSOR_SIZE; k++)
					it.values[k] = av_pt[k] / c_pts.GetCount();
			}
			
			// Debug print
			#ifdef flagDEBUG
			for(int j = 0; j < indi_centroids.GetCount(); j++) {
				const IndicatorTuple& ctr = indi_centroids[j];
				DLOG("Iter " << i << ": Center " << j << ": " << ctr.ToString());
			}
			DLOG("");
			#endif
		}
		
		initial_indicator_clustering = false;
		
		LOG("Indicator clusters found in " << ts.ToString());
		Cout() << "Indicator clusters found in " << ts.ToString() << EOL;
		ts.Reset();
	}
	
	// Write clusters to snapshots
	for (; indi_cluster_counter < snaps.GetCount(); indi_cluster_counter++) {
		Snapshot& snap = snaps[indi_cluster_counter];
		
		const IndicatorTuple& it = snap.GetIndicatorTuple();
		
		// Find closest centroid to the point
		int c_id = -1;
		int min_dist = INT_MAX;
		for(int l = 0; l < indi_centroids.GetCount(); l++) {
			const IndicatorTuple& ctr = indi_centroids[l];
			
			// Calculate Euclidean distance
			int dist = ctr.GetDistance(it);
			if (dist < min_dist) {
				min_dist = dist;
				c_id = l;
			}
		}
		
		//LOG(indi_cluster_counter << "\t" << c_id);
		snap.SetIndicatorCluster(c_id);
	}
	
	LOG("Indicator cluster ids written in " << ts.ToString());
	Cout() << "Indicator cluster ids written in " << ts.ToString() << EOL;
	
}

}
