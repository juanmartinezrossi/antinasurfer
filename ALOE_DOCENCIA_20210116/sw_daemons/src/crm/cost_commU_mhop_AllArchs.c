/*
 * cost_commU_mhop_AllArchs.c
 *
 * Copyright (c) 2009 Vuk Marojevic, UPC <marojevic at tsc.upc.edu>. All rights reserved.
 *
 *
 * This file is part of ALOE.
 *
 * ALOE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ALOE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// function that calculates the communication cost for DAGs with any ordering
// FULL-DUPLEX, HALF-DUPLEX, or BUS architectures

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mapper.h"

extern int N;			// number of platform's processors
extern int M;			// number of application's tasks/SDR functions/processes

extern int I[Nmax][Nmax];		// bandwidth resources

extern float q2;		// cost function parameter
extern int mhop;		// multi-hop indicator; mhop = 0: single hops only, mhop = 1: 2-hop only if necessary, mhop = 2: single & 2-hops 

void update(float bx, float B_rem[], int indx1, int indx2);

float cost_commU(float bx1, float bx2, float B_rem[], int indx1, int indx2)
{
	// local variables
	int x, p, n, sel_path;		// path indices
	int path_num = 0;			// number of paths
	int num_hops[Nmax];			// number of hops per path
	int path[Nmax][Nmax];				// characterizes the N possible 2-paths, e.g. path[2] = (1 2 3), indicates that path 3 is [P1 P2 P3], a 2-hop path from P1 to P3.
	//unsigned B_eff[N];		// effective bandwidth = minimum bandwith along the path			

	float cost[Nmax];				// path cost
	float cost_total = 0;		// communication cost due to the selected path

	// intialization
	for (x=0; x<N; x++)
		num_hops[x] = 0;

	if (mhop == 0)	// single hops only
	{
		if (bx1 > 0)	// non-zero bandwidth requirement
		{
			//if (bx1 > B_rem[indx1][indx2])					// bandwidth requirement greater than bandwidth capacity
			if ((bx1 - B_rem[I[indx1][indx2]])	> threshold)		// bandwidth requirement greater than bandwidth capacity
				cost_total = infinite;							// infinite cost
			else
			{
				cost_total = q2*bx1/B_rem[I[indx1][indx2]];	// cost = required_bandwidth / available_bandwidth
				update(bx1, B_rem, indx1, indx2);				// update remaining bandwidth
			}
		}
		else if (bx2 > 0)										// non-zero bandwidth requirement
		{
			//if (bx2 > B_rem[indx2][indx1])					// bandwidth requirement greater than bandwidth capacity
			if ((bx2 - B_rem[I[indx2][indx1]]) > threshold)		// bandwidth requirement greater than bandwidth capacity
				cost_total = infinite;							// infinite cost
			else
			{
				cost_total = q2*bx2/B_rem[I[indx2][indx1]];	// cost = required_bandwidth / available_bandwidth
				update(bx2, B_rem, indx2, indx1);				// update remaining bandwidth
			}
		}
	}

	else		// single and 2-hops (mhop = 1: 2-hops only if necessary)
	{
		if (bx1 > 0)	// non-zero bandwidth requirement for receiving data by current module pre-mapped to P[indx2] from module pre-mapped to P[indx1]
		{
			cost_total = infinite;	// there will be a communication cost since bx1 > 0; initiate with infinite
			// ################## 1st-hop ##################
			for (x=0; x<N; x++)
			{
				//if ((x == indx1) || (bx1 > B_rem[indx1][x]))
				if ((x == indx1) || ((bx1 - B_rem[I[indx1][x]]) > threshold))
					;
				else
				{
					//B_eff[path_num] = B_rem[indx1][x];
					path[path_num][num_hops[path_num]] = indx1;
					num_hops[path_num]++;
					path[path_num][num_hops[path_num]] = x;

					cost[path_num] = q2*bx1/B_rem[I[indx1][x]];	// cost function specific
					path_num++;
				}
			}

			if (path_num > 0)
			{
				// ################## 2nd- and FINAL hop ##################
				for (p=0; p<path_num; p++)
				{
					if (path[p][num_hops[p]] == indx2)	// already terminated path (single-hop path)
						;
					else
					{
						//if (bx1 > B_rem[path[p][num_hops[p]]][indx2])
						if ((bx1 - B_rem[I[path[p][num_hops[p]]][indx2]]) > threshold)
							cost[p] = infinite;
						else
						{
							//if (((B_rem[path[p][num_hops[p]]][indx2]) - B_eff[p]) < 0)
				            //    B_eff[p] = B_rem[path[p][num_hops[p]]][indx2];

							if (mhop == 1)
								cost[p] += q2*bx1/B_rem[I[path[p][num_hops[p]]][indx2]] + (float)1000; // cost function specific; + 1000 to potentiate single hops (direct links) and pre-mappings that avoid interprocessor data flow
							else
								cost[p] += q2*bx1/B_rem[I[path[p][num_hops[p]]][indx2]]; // cost function specific

							num_hops[p]++;
						    path[p][num_hops[p]] = indx2;

						}
					}
				}

				// CHOOSE PATH ...
				for (p=0; p<path_num; p++)
				{
					if ((cost_total - cost[p]) > threshold) //(cost[p] < cost_total))
					{
						cost_total = cost[p];
						sel_path = p;
					}
				}

				// UPDATE BANDWIDTH RESOURCES ...
				if (cost_total < (infinite - 1))
				{
					for (n=0; n<num_hops[sel_path]; n++)
						update(bx1, B_rem, path[sel_path][n], path[sel_path][n+1]);
				}

			} // if (path_num > 0)

		} // if (bx1 > 0)

		else if (bx2 > 0)	// non-zero bandwidth requirement for sending data from current module pre-mapped to P[indx2] to module pre-mapped to P[indx1]
		{
			cost_total = infinite;	// there will be a communication cost since bx1 > 0; initiate with infinite
			// ################## 1st-hop ##################
			for (x=0; x<N; x++)
			{
				//if ((x == indx2) || (bx2 > B_rem[indx2][x]))
				if ((x == indx2) || ((bx2 - B_rem[I[indx2][x]]) > threshold))
					;
				else
				{
					//B_eff[path_num] = B_rem[indx2][x];
					path[path_num][num_hops[path_num]] = indx2;
					num_hops[path_num]++;
					path[path_num][num_hops[path_num]] = x;

					cost[path_num] = q2*bx2/B_rem[I[indx2][x]];	// cost function specific
					path_num++;
				}
			}

			if (path_num > 0)
			{
				// ################## 2nd- and FINAL hop ##################
				for (p=0; p<path_num; p++)
				{
					if (path[p][num_hops[p]] == indx1)	// already terminated path (single-hop path)
						;
					else
					{
						//if (bx2 > B_rem[path[p][num_hops[p]]][indx1])
						if ((bx2 - B_rem[I[path[p][num_hops[p]]][indx1]]) > threshold)
							cost[p] = infinite;
						else
						{
							//if (((B_rem[path[p][num_hops[p]]][indx1]) - B_eff[p]) < 0)
						    //    B_eff[p] = B_rem[path[p][num_hops[p]]][indx1];

							if (mhop == 1)
								cost[p] += q2*bx2/B_rem[I[path[p][num_hops[p]]][indx1]] + (float)1000; // cost function specific; + 1000 to potentiate single hops (direct links) and pre-mappings that avoid interprocessor data flow
							else
								cost[p] += q2*bx2/B_rem[I[path[p][num_hops[p]]][indx1]]; // cost function specific

							num_hops[p]++;
						    path[p][num_hops[p]] = indx1;

						}
					}
				}

				// CHOOSE PATH ...
				for (p=0; p<path_num; p++)
				{
					if ((cost_total - cost[p]) > threshold) //(cost[p] < cost_total))
					{
						cost_total = cost[p];
						sel_path = p;
					}
				}

				// UPDATE BANDWIDTH RESOURCES ...
				if (cost_total < (infinite - 1))
				{
					for (n=0; n<num_hops[sel_path]; n++)
						update(bx2, B_rem, path[sel_path][n], path[sel_path][n+1]);
				}

			} // if (path_num > 0)

		} // else if (bx2 > 0)
	}

	return cost_total;
}
