#if 0

#include "Overlook.h"

#define DLOG(x) LOG(x)

namespace Overlook {

void AgentSystem::RefreshClusters() {
	TimeStop ts;
	SubProgress(0, 4);
	RefreshResultClusters();
	SubProgress(1, 4);
	RefreshIndicatorClusters();
	SubProgress(2, 4);
	RefreshClusterConnections();
	SubProgress(3, 4);
	RefreshPoleNavigation();
	SubProgress(4, 4);
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
		const int extra_centers = RESULT_EXTRACENTERS; // add some room for worst centers
		const int max_centroids = GROUP_COUNT + extra_centers;
		int cols = sqrt(max_centroids);
		int rows = max_centroids / cols;
		int extrarow_cols = max_centroids - cols * rows;
		int max_x = result_stats.TopKey().volat;
		int xstep = max_x / cols;
		int cur = 0;
		ASSERT(cols * rows + extrarow_cols == max_centroids);
		
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
			
			int this_rows = rows + (i < extrarow_cols ? 1 : 0);
			int range_y = max_y - min_y;
			int ystep = range_y / this_rows;
			
			for(int j = 0; j < this_rows; j++) {
				int y = min_y + ystep / 2 + j * ystep;
				result_centroids.Add(ResultTuple(x, y));
			}
		}
		ASSERT(result_centroids.GetCount() == max_centroids);
		
		
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
				
			int64 av_pt_volat = 0;
			int64 av_pt_change = 0;
			int64 total = 0;
			for(int l = 0; l < c_pts.GetCount(); l++) {
				ResultTupleCounter& c_pt = c_pts[l];
				av_pt_volat += c_pt.volat * c_pt.count;
				av_pt_change += c_pt.change * c_pt.count;
				total += c_pt.count;
			}
			ResultTuple av_pt;
			av_pt.volat		= av_pt_volat / total;
			av_pt.change	= av_pt_change / total;
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
				int64 av_pt_volat = 0;
				int64 av_pt_change = 0;
				int64 total = 0;
				for(int l = 0; l < c_pts.GetCount(); l++) {
					ResultTupleCounter& c_pt = c_pts[l];
					av_pt_volat += c_pt.volat * c_pt.count;
					av_pt_change += c_pt.change * c_pt.count;
					total += c_pt.count;
				}
				ResultTuple av_pt;
				av_pt.volat		= av_pt_volat / total;
				av_pt.change	= av_pt_change / total;
				result_centroids[j] = av_pt;
			}
			
			Sort(result_centroids, StdLess<ResultTuple>());
			for(int j = 0; j < result_centroids.GetCount(); j++) {
				ResultTuple& rt1 = result_centroids[j];
				for(int k = j+1; k < result_centroids.GetCount(); k++) {
					ResultTuple& rt2 = result_centroids[k];
					if (rt1 == rt2 || (rt2.volat == 0 && rt2.change == 0)) {
						rt2.change+=k;
						rt2.volat+=k;
					}
					else if (rt1.volat == 0 && rt1.change == 0) {
						rt1.change+=k;
						rt1.volat+=k;
					}
				}
			}
			
			
			// Remove too small groups
			if (i > 90) {
				for(int j = 0; j < pts.GetCount(); j++) {
					Vector<ResultTupleCounter >& c_pts = pts[j];
					int64 total = 0;
					for(int l = 0; l < c_pts.GetCount(); l++) {
						total += c_pts[l].count;
					}
					if (total < 10000) {
						int j2 = (j + 1) % pts.GetCount();
						pts[j2].Append(c_pts);
						pts.Remove(j);
						result_centroids.Remove(j);
						j--;
					}
				}
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
		
		
		// Remove smallest centers
		#ifndef flagVERY_FAST
		int a = (result_centroids.GetCount() - GROUP_COUNT ) / 2;
		int b = result_centroids.GetCount() - GROUP_COUNT - a;
		result_centroids.Remove(0, a);
		result_centroids.Remove(GROUP_COUNT, b);
		#else
		result_centroids.Remove(GROUP_COUNT, result_centroids.GetCount() - GROUP_COUNT);
		#endif
		if (result_centroids.GetCount() != OUTPUT_COUNT)
			Panic("Result centroids mismatch.");
		
		// Debug print
		#ifdef flagDEBUG
		for(int j = 0; j < result_centroids.GetCount(); j++) {
			const ResultTuple& ctr = result_centroids[j];
			double v = ctr.volat * VOLAT_DIV;
			double c = ctr.change * CHANGE_DIV * 1000;
			DLOG("Final center " << j << ": " << v << " x " << c);
		}
		DLOG("");
		#endif
		
		
		initial_result_clustering = false;
	}
	
	TimeStop ts;
	
	// Write clusters to snapshots
	for (; result_cluster_counter < snaps.GetCount(); result_cluster_counter++) {
		Snapshot& snap = snaps[result_cluster_counter];
		
		if (result_cluster_counter % 100 == 0)
			SubProgress(result_cluster_counter * 100 / snaps.GetCount(), 400);
		
		
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
		Cout() << "Initialize indicator clustering" << EOL;
		
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
		int rows = Upp::max(1, INPUT_COUNT / cols);
		int extrarow_cols = INPUT_COUNT > SENSOR_SIZE ? (INPUT_COUNT - (cols * rows)) % cols : 0;
		
		indi_centroids.SetCount(INPUT_COUNT);
		
		int row = 0, col = 0;
		for (int ic = 0; ic < INPUT_COUNT; ic++) {
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
	
	Cout() << "Writing indicator clusters to snapshots" << EOL;
	
	
	// Write clusters to snapshots
	for (; indi_cluster_counter < snaps.GetCount(); indi_cluster_counter++) {
		Snapshot& snap = snaps[indi_cluster_counter];
		
		if (indi_cluster_counter % 100 == 0)
			SubProgress(100 + indi_cluster_counter * 100 / snaps.GetCount(), 400);
		
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

void AgentSystem::RefreshClusterConnections() {
	
	// Run this function only at first time
	if (result_sectors.IsEmpty()) {
		ASSERT(result_centroids.GetCount() == OUTPUT_COUNT);
		ASSERT(indi_centroids.GetCount() == INPUT_COUNT);
		result_sectors.SetCount(OUTPUT_COUNT);
		indi_sectors.SetCount(INPUT_COUNT);
		
		for (int s = 0; s < snaps.GetCount(); s++) {
			Snapshot& snap = snaps[s];
			
			for(int i = 0; i < SYM_COUNT; i++) {
				for(int j = 0; j < MEASURE_PERIODCOUNT; j++) {
					int shift_diff = MEASURE_PERIOD(j);
					int shift = s - shift_diff;
					if (shift < 0) continue;
					
					Snapshot& prev_snap = snaps[shift];
					int indi_c_id = prev_snap.GetIndicatorCluster();
					IndicatorSector& is = indi_sectors[indi_c_id];
					
					int result_c_id = snap.GetResultCluster(i, j);
					ResultSector& rs = result_sectors[result_c_id];
					
					rs.AddSource(i, j, indi_c_id);
					is.AddDestination(i, j, result_c_id);
				}
			}
		}
		
		for(int i = 0; i < indi_sectors.GetCount(); i++) {
			IndicatorSector& is = indi_sectors[i];
			if (is.IsEmpty()) {
				indi_sectors.Remove(i);
				indi_centroids.Remove(i);
				i--;
			}
		}
		
		for(auto& rs : result_sectors)	rs.Sort();
		for(auto& ri : indi_sectors)	ri.Sort();
		
		
		
		// Connect inputs to outputs in uniform way.
		// Todo: should weight maximum per output with connection count
		
		// Sort in -> out connections by their probabilities. Use most probable first.
		Vector<VectorMap<int, double> > in_out;
		in_out.SetCount(OUTPUT_COUNT);
		for(int i = 0; i < indi_sectors.GetCount(); i++) {
			IndicatorSector& is = indi_sectors[i];
			for(int j = 0; j < is.sector_conn_counts.GetCount(); j++) {
				int out = is.sector_conn_counts.GetKey(j);
				int count = is.sector_conn_counts[j];
				double prob = (double)count / is.conn_total;
				in_out[out].GetAdd(i, prob);
			}
		}
		for(int i = 0; i < in_out.GetCount(); i++)
			 SortByValue(in_out[i], StdGreater<double>());
		
		// Limit _predicted_ connection count by _known_ connection count
		Vector<int> per_output;
		per_output.SetCount(OUTPUT_COUNT, 0);
		int64 total_connections = 0;
		for (const auto& rs : result_sectors) total_connections += rs.conn_total;
		int64 total_connected = 0;
		for(int i = 0; i < per_output.GetCount(); i++) {
			int& out = per_output[i];
			out = INPUT_COUNT * result_sectors[i].conn_total / total_connections;
			total_connected += out;
		}
		while (total_connected < INPUT_COUNT) {
			for(int i = 0; i < per_output.GetCount() && total_connected < INPUT_COUNT; i++) {
				per_output[i]++;
				total_connected++;
			}
		}
		
		// Process per most probable connection, but in limits.
		// Usually, one single output is preferred by all, so don't let it accumulate.
		Vector<Vector<int> > out_in;
		out_in.SetCount(OUTPUT_COUNT);
		total_connected = 0;
		Index<int> connected_inputs;
		while (true) {
			int max_prob_i = -1;
			double max_prob = -DBL_MAX;
			for(int i = 0; i < in_out.GetCount(); i++) {
				if (out_in[i].GetCount() >= per_output[i] || in_out[i].IsEmpty())
					continue;
				double prob = in_out[i][0];
				if (prob > max_prob) {
					max_prob_i = i;
					max_prob = prob;
				}
			}
			if (max_prob_i == -1) {
				if (total_connected >= INPUT_COUNT) {
					LOG("Connected " << total_connected << "/" << indi_sectors.GetCount());
					break;
				} else {
					// Pick any (the limit too high compared to available data)
					for(int i = 0; i < in_out.GetCount(); i++) {
						if (!in_out[i].IsEmpty()) {
							max_prob_i = i;
							break;
						}
					}
					ASSERT(max_prob_i != -1);
				}
			}
			int in = in_out[max_prob_i].GetKey(0);
			in_out[max_prob_i].Remove(0);
			if (connected_inputs.Find(in) == -1) {
				out_in[max_prob_i].Add(in);
				total_connected++;
				connected_inputs.Add(in);
			}
		}
		for(int i = 0; i < out_in.GetCount(); i++) {
			for(int j = 0; j < out_in[i].GetCount(); j++) {
				DLOG(i << ", " << j << ": " << out_in[i][j]);
				IndicatorSector& is = indi_sectors[out_in[i][j]];
				ASSERT(is.result_id == -1);
				is.result_id = i;
			}
		}
		// This shouldn't happen in correct implementation
		int fail = 0;
		for (auto& is : indi_sectors) {
			//ASSERT(is.result_id != -1);
			if (is.result_id != -1) continue;
			is.result_id = is.sector_conn_counts.GetKey(0);
			fail++;
		}
		LOG("fails " << fail << "/" << indi_sectors.GetCount());
		
		
		#ifdef flagDEBUG
		for(int i = 0; i < indi_sectors.GetCount(); i++) {
			IndicatorSector& is = indi_sectors[i];
			LOG("Indisec\t" << i << "\t-->\t" << is.GetResultSector() << "\t" << is.GetResultProbability() << "\t" << is.result_id);
		}
		#endif
	}
	
	
	// Update:
	// The lightweight version is broken and the full refresh is required every time.
	// The fault is in long predicted periods, which must be forcily splitted sometimes.
	
	// Clear poles for full refresh
	for(int i = 0; i < result_sectors.GetCount(); i++) {
		ResultSector& rs = result_sectors[i];
		rs.pnd.Clear();
		for(int j = 0; j < TARGET_COUNT; j++) {
			rs.pos[j].Clear();
			rs.neg[j].Clear();
		}
	}
	
	// Clear snapshots for full refresh
	for(int i = 0; i < snaps.GetCount(); i++)
		snaps[i].ClearResultClusterPredicted();
	
	
	int begin = 1;
	Vector<int> pred_sector_begins, pred_sector_indis;
	Vector<double> pred_sector_volatsum, pred_sector_open;
	pred_sector_begins		.SetCount(OUTPUT_COUNT * SYM_COUNT,   0);
	pred_sector_indis		.SetCount(OUTPUT_COUNT * SYM_COUNT,  -1);
	pred_sector_volatsum	.SetCount(OUTPUT_COUNT * SYM_COUNT, 0.0);
	pred_sector_open		.SetCount(OUTPUT_COUNT * SYM_COUNT, 0.0);
	for (int s = begin; s < snaps.GetCount(); s++) {
		Snapshot& snap = snaps[s];
		Snapshot& prev_snap = snaps[s-1];
		
		if (s % 100 == 0)
			SubProgress(200 + s * 100 / snaps.GetCount(), 400);
		
		int indi_c_id = snap.GetIndicatorCluster();
		const IndicatorSector& is = indi_sectors[indi_c_id];
		
		int result_c_id = is.result_id;
		ASSERT(result_c_id != -1);
		const ResultTuple& it = result_centroids[result_c_id];
		double volat_limit = it.volat * VOLAT_DIV;
		
		for(int i = 0; i < SYM_COUNT; i++) {
			double volat_sum = 0.0;
			bool skip_once = true;
			for(int pos = s; pos < snaps.GetCount() && volat_sum <= volat_limit; pos++) {
				Snapshot& pred_snap = snaps[pos];
				pred_snap.SetResultClusterPredicted(i, result_c_id);
				if (!skip_once)
					volat_sum += fabs(pred_snap.GetChange(i));
				else
					skip_once = false;
			}
		}
		
		int k = 0;
		for(int i = 0; i < SYM_COUNT; i++) {
			for(int j = 0; j < OUTPUT_COUNT; j++) {
				bool enabled_now  = snap.IsResultClusterPredicted(i, j);
				bool enabled_prev = prev_snap.IsResultClusterPredicted(i, j);
				if (enabled_now && !enabled_prev) {
					pred_sector_begins[k] = s;
					pred_sector_open[k] = snap.GetOpen(i);
					pred_sector_volatsum[k] = 0;
				}
				else if (!enabled_now && enabled_prev &&
					pred_sector_begins[k] > 0) {
					AnalyzeSectorPoles(pred_sector_begins[k], s, i, j);
					pred_sector_begins[k] = 0;
				}
				else if (enabled_now && enabled_prev &&
					pred_sector_begins[k] > 0 &&
					pred_sector_volatsum[k] > result_centroids[j].volat * VOLAT_DIV) {
					prev_snap.SetResultClusterPredicted(i, j, false);
					AnalyzeSectorPoles(pred_sector_begins[k], s, i, j);
					pred_sector_begins[k] = s;
					pred_sector_open[k] = snap.GetOpen(i);
					pred_sector_volatsum[k] = 0;
				}
				
				
				// Write change and volatility sum since the begin of predicted cluster
				if (enabled_now) {
					
					// Change from open
					double change = snap.GetOpen(i) / pred_sector_open[k] - 1.0;
					
					
					// Change from previous
					double abs_step_change = fabs(snap.GetChange(i));
					pred_sector_volatsum[k] += abs_step_change;
					snap.SetResultClusterPredictedChanged(i, j, change);
					snap.SetResultClusterPredictedVolatiled(i, j, pred_sector_volatsum[k]);
				}
				
				k++;
			}
		}
	}
}

void AgentSystem::AnalyzeSectorPoles(int snap_begin, int snap_end, int sym_id, int result_c_id) {
	ASSERT(snap_begin > 0);
	int len = snap_end - snap_begin;
	if (len < 16) return;
	
	const ResultTuple& rt = result_centroids[result_c_id];
	bool down = rt.change < 0; // long=0, short=1
	
	double begin_open = snaps[snap_begin].GetOpen(sym_id);
	double volatsum;
	int snap_step = len / (TARGET_COUNT + 1);
	
	double step_volatsum[TARGET_COUNT];
	for(int i = 0; i < TARGET_COUNT; i++) step_volatsum[i] = 0;
	for(int i = 1; i < snap_step * TARGET_COUNT; i++)
		step_volatsum[i / snap_step] += fabs(snaps[snap_begin + i].GetChange(sym_id));
	for(int i = 0; i < TARGET_COUNT; i++)
		for(int j = i+1; j < TARGET_COUNT; j++)
			step_volatsum[j] += step_volatsum[i];
	
	// Find pending point
	volatsum = 0;
	int pnd_target_i = -1;
	double pnd_target = down == false ? +DBL_MAX : -DBL_MAX;
	double pnd_volatsum = 0;
	int step_begin = snap_begin + 1;
	int step_end   = snap_begin + snap_step;
	for(int i = step_begin; i < step_end; i++) {
		const Snapshot& snap = snaps[i];
		double open = snap.GetOpen(sym_id);
		volatsum += fabs(snap.GetChange(sym_id));
		if (down == false) {
			if (open < pnd_target) {
				pnd_target = open;
				pnd_target_i = i;
				pnd_volatsum = volatsum;
			}
		} else {
			if (open > pnd_target) {
				pnd_target = open;
				pnd_target_i = i;
				pnd_volatsum = volatsum;
			}
		}
	}
	double pnd_change = pnd_target / begin_open - 1.0;
	
	
	// Add point to averages
	ResultSector& rs = result_sectors[result_c_id];
	ASSERT(pnd_target_i != -1);
	rs.pnd.Add(pnd_volatsum / VOLAT_DIV, pnd_change / CHANGE_DIV);
	
	
	for (int target = 0; target < TARGET_COUNT; target++) {
		int step_begin = snap_begin + (target + 1) * snap_step;
		int step_end   = snap_begin + (target + 2) * snap_step;
		
		// Find +target point
		volatsum = step_volatsum[target];
		int pos_target_i = -1;
		double pos_target = down == false ? -DBL_MAX : +DBL_MAX;
		double pos_volatsum = 0;
		for(int i = step_begin; i < step_end; i++) {
			const Snapshot& snap = snaps[i];
			double open = snap.GetOpen(sym_id);
			volatsum += fabs(snap.GetChange(sym_id));
			if (down == false) {
				if (open > pos_target) {
					pos_target = open;
					pos_target_i = i;
					pos_volatsum = volatsum;
				}
			} else {
				if (open < pos_target) {
					pos_target = open;
					pos_target_i = i;
					pos_volatsum = volatsum;
				}
			}
		}
		double pos_change = pos_target / begin_open - 1.0;
		
		
		// Find -target point
		volatsum = step_volatsum[target];
		int neg_target_i = -1;
		double neg_target = down == false ? +DBL_MAX : -DBL_MAX;
		double neg_volatsum = 0;
		for(int i = step_begin; i < step_end; i++) {
			const Snapshot& snap = snaps[i];
			double open = snap.GetOpen(sym_id);
			volatsum += fabs(snap.GetChange(sym_id));
			if (down == false) {
				if (open < neg_target) {
					neg_target = open;
					neg_target_i = i;
					neg_volatsum = volatsum;
				}
			} else {
				if (open > neg_target) {
					neg_target = open;
					neg_target_i = i;
					neg_volatsum = volatsum;
				}
			}
		}
		double neg_change = neg_target / begin_open - 1.0;
		
		
		// Add point to averages
		ASSERT(pos_target_i != -1);
		ASSERT(neg_target_i != -1);
		rs.pos[target].Add(pos_volatsum / VOLAT_DIV, pos_change / CHANGE_DIV);
		rs.neg[target].Add(neg_volatsum / VOLAT_DIV, neg_change / CHANGE_DIV);
	}
}

void AgentSystem::RefreshPoleNavigation() {
	const int shift = MEASURE_PERIOD(-4);
	
	int begin = 1;
	
	
	// Clear snapshots for full refresh
	for(int i = 0; i < snaps.GetCount(); i++)
		snaps[i].ClearResultClusterPredictedTarget();
	
	
	Vector<int> phase_begin, phase_state;
	Vector<int> pnd_dist_buf, pos_dist_buf, neg_dist_buf;
	
	enum {STATE_PENDING, STATE_TARGETING, STATE_WAITING};
	
	for (int target = 0; target < TARGET_COUNT; target++) {
		phase_begin.SetCount(SYM_COUNT * OUTPUT_COUNT, 0);
		phase_state.SetCount(SYM_COUNT * OUTPUT_COUNT, STATE_PENDING);
		pnd_dist_buf.SetCount(SYM_COUNT * OUTPUT_COUNT * shift, 0);
		pos_dist_buf.SetCount(SYM_COUNT * OUTPUT_COUNT * shift, 0);
		neg_dist_buf.SetCount(SYM_COUNT * OUTPUT_COUNT * shift, 0);
		
		
		for (int s = begin; s < snaps.GetCount(); s++) {
			Snapshot& snap = snaps[s];
			Snapshot& prev_snap = snaps[s - 1];
			
			if (s % 100 == 0)
				SubProgress(300 + s * 100 / snaps.GetCount(), 400);
			
			int k = 0;
			for(int i = 0; i < SYM_COUNT; i++) {
				for(int j = 0; j < OUTPUT_COUNT; j++) {
					int cur_shift_id = s % shift;
					int& pnd_dist = pnd_dist_buf[k * shift + cur_shift_id];
					int& pos_dist = pos_dist_buf[k * shift + cur_shift_id];
					int& neg_dist = neg_dist_buf[k * shift + cur_shift_id];
					int& k_phase_begin = phase_begin[k];
					int& k_phase_state = phase_state[k];
					
					bool enabled_now  = snap.IsResultClusterPredicted(i, j);
					bool enabled_prev = prev_snap.IsResultClusterPredicted(i, j);
					
					if (enabled_now) {
						#if HAVE_POLES
						const AveragePoint& pnd = result_sectors[j].pnd;
						const AveragePoint& pos = result_sectors[j].pos[target];
						const AveragePoint& neg = result_sectors[j].neg[target];
						
						int volatsum = snap.GetResultClusterPredictedVolatiledInt(i, j);
						int change = snap.GetResultClusterPredictedChangedInt(i, j);
						int vd, cd;
						
						vd = (volatsum - pnd.x_mean_int) * VOLINTMUL;
						cd = change - pnd.y_mean_int;
						pnd_dist = root(vd * vd + cd * cd);
						
						vd = (volatsum - pos.x_mean_int) * VOLINTMUL;
						cd = change - pos.y_mean_int;
						pos_dist = root(vd * vd + cd * cd);
						
						vd = (volatsum - neg.x_mean_int) * VOLINTMUL;
						cd = change - neg.y_mean_int;
						neg_dist = root(vd * vd + cd * cd);
						
						if (!enabled_prev) {
							k_phase_begin = s;
							k_phase_state = STATE_PENDING;
						}
						// enabled_prev == true
						else {
							
							if (k_phase_state == STATE_PENDING) {
								int snap_dist = s - k_phase_begin;
								
								if (snap_dist >= shift) {
									int prev_shift_id = (s + 1) % shift; // yes, +1, because s - shift + 1
									int& prev_pnd_dist = pnd_dist_buf[k * shift + prev_shift_id];
									
									
									// If distance from pending-pole is larger currently, activate targeting.
									if (pnd_dist > prev_pnd_dist) {
										k_phase_state = STATE_TARGETING;
									}
								}
							}
							
							if (k_phase_state == STATE_TARGETING) {
								
								int prev_shift_id = (s + 1) % shift; // yes, +1, because s - shift + 1
								int& prev_pos_dist = pos_dist_buf[k * shift + prev_shift_id];
								int& prev_neg_dist = neg_dist_buf[k * shift + prev_shift_id];
								int pos_diff = pos_dist - prev_pos_dist;
								int neg_diff = neg_dist - prev_neg_dist;
								
								// If distance to positive pole is increasing and negative pole is
								// getting closer
								if (pos_dist > prev_pos_dist && pos_diff > neg_diff) {
									k_phase_state = STATE_WAITING;
								}
								// Don't write before new snapshots
								else {
									snap.SetResultClusterPredictedTarget(i, j, target);
								}
							}
						}
						#else
						
						snap.SetResultClusterPredictedTarget(i, j, target);
						
						#endif
					}
					
					k++;
				}
			}
		}
	}
}

}
#endif
