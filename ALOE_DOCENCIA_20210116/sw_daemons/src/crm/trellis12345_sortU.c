/*
 * trellis12345_sortU.c
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

// function that calculates the t-mapping cost, where the actual cost calculation
// and the resource updating is done in the seperate functions cost_compU() and cost_commU()

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mapper.h"

extern int N;	// number of platform's processors
extern int M;	// number of application's tasks/SDR functions/processes

extern float P[Nmax];					// processing powers
//extern float B[Nmax][Nmax];				// bandwidth resources
extern int I[Nmax][Nmax];		// bandwidth resources
extern float B[Nmax*Nmax];			// bandwidth resources

extern float m_sort[Mmax];				// application's processing requirements sorted
extern float b[Mmax][Mmax];				// bandwidth requirements

extern float a_pathsU[Nmax+1][Nmax*Mmax];	// path costs
extern int PP[Nmax][Mmax][Mmax];			// N*M matrix of Trellis nodes, each nodes contains the whole mapping up to this step

//... information for any w: mmap[i] indicates where m[i] is finally mapped
extern int mmap[Mmax];

/** IGM: Force idx vector
 */
extern int force_idx[Mmax];

// function prototypes
float cost_compU(float mx, float P_rem[], int force_idx, int indx);
float cost_commU(float bx1, float bx2, float B_rem[], int indx1, int indx2);

float m[Mmax];					// processing requirements

	int i,j,k,s,u,v;				// loop indices
	float P_tempGS[Nmax][Nmax];		// resting Processing capacities, temporary-Global-Saved (step i-1)
	float P_tempG[Nmax][Nmax];		// resting Processing capacities, temporary-Global (step i)
	
	//float B_tempGS[Nmax][Nmax][Nmax];	// resting Bandwidth capacities, temporary-Global-Saved (step i-1)
	//float B_tempG[Nmax][Nmax][Nmax];	// resting Bandwidths, temporary-Global (step i)
	float B_tempGS[Nmax][Nmax*Nmax];			// resting Bandwidth capacities, temporary-Global-Saved (step i-1)
	float B_tempG[Nmax][Nmax*Nmax];			// resting Bandwidths, temporary-Global (step i)
	
	float P_remL[Nmax][Nmax];

	//float B_remL[Nmax][Nmax][Nmax];
	float B_remL[Nmax][Nmax*Nmax];

	float P_remG[Nmax][Nmax];

	//float B_remG[Nmax][Nmax][Nmax];
	float B_remG[Nmax][Nmax*Nmax];

	int indx[Nmax][Mmax];			// points to the selected edge (decision) at a t-node
	int index = -1;					// index that points to the minimum-cost t-node at step M (initialized with -1 as we suppose infeasibility)

	// w > 1
	int k1, k2, k3, k4, k5;														// loop indices for levels 1, 2, 3, 4, and 5
	float P_tempL1[Nmax][Nmax], P_tempL2[Nmax], P_tempL3[Nmax], P_tempL4[Nmax], P_tempL5[Nmax];	// resting computing capacities at levels 1 (k1), 2 (k2), 3 (k3), 4 (k4), and 5 (k5)

	//float B_tempL1[Nmax][Nmax][Nmax], B_tempL2[Nmax][Nmax], B_tempL3[Nmax][Nmax], B_tempL4[Nmax][Nmax], B_tempL5[Nmax][Nmax];	// resting Bandwidths at levels 1 (k1), 2 (k2), 3 (k3), 4 (k4), and 5 (k5)
	float B_tempL1[Nmax][Nmax*Nmax], B_tempL2[Nmax*Nmax], B_tempL3[Nmax*Nmax], B_tempL4[Nmax*Nmax], B_tempL5[Nmax*Nmax];	// resting Bandwidths at levels 1 (k1), 2 (k2), 3 (k3), 4 (k4), and 5 (k5)


	float costw_comm1, costw_comm2, costw_comm3, costw_comm4, costw_comm5;		// cost of communication at levels  2 (k2), 3 (k3), 4 (k4), and 5 (k5)
	float costw2, costw3, costw4, costw5;										// cost at levels 2 (k2), 3 (k3), 4 (k4), and 5 (k5)
	float costw_aux[Nmax];														// cost at levels 2, 3, 4, or 5 - local decision
	float costw_auxx[Nmax];														// cost at levels 2, 3, 4 or - global decision

	// final mapping ...
	// ... indices of the forward-tracking (w > 1)
	int indx1_aux, indx2_aux, indx3_aux, indx4_aux;			// auxiliary indices
	int indx1[Nmax], indx2[Nmax], indx3[Nmax], indx4[Nmax];	// indx1[j] (w > 1), indx2[j] (w > 2), indx3[j] (w > 3), indx4[j] (w > 4) point from t-node (j,M-w+1) to t-nodes at step (M-w+1) + 1, (M-w+1) + 2, (M-w+1) + 3, and (M-w+1) + 4
	int i1, i2, i3, i4;


float tw_mapping(int (*ptr_nodes)[Mmax], int w)
//float tw_mapping(int **ptr_nodes, int w)
{
								// point to the final mapping's t-nodes at step (M-w+1) + 1 in case of w > 1, (M-w+1) + 2 in case of w > 2, (M-w+1) + 3 in case of w > 3, and (M-w+1) + 4 in case of w > 4
	float cost_t = infinite;		// final mapping cost (infeasibility supposed)

	// intitialization of processing requirements
	for (i=0; i<M; i++)
		m[i] = m_sort[i];

	// initialization of final mapping representation
	for (i=0; i<M; i++)
		mmap[i] = -1; // a mmap[i] = -1 indicates that m[i] is not mapped

	// initialization of processor assignments
	for (i=0; i<N; i++)
		for (j=0; j<M; j++)
			for (k=0; k<M; k++)
				PP[i][j][k] = 0;
	
	// initialization of path costs
	for (i=0; i<N+1; i++)
		for (j=0; j<N*M; j++)
			a_pathsU[i][j] = 0;

	// set initial processing powers
	for (u=0; u<N; u++)
        for (v=0; v<N; v++)
			P_tempG[u][v] = P[v];

	// set initial bandwidth capacities
	for (k=0; k<N; k++)
		for (u=0; u<(N*N); u++)
			B_tempG[k][u] = B[u];


	// assignment of m[0] to all N proc's and update remaining processing powers
	for (j=0; j<N; j++)
	{
		PP[j][0][0] = j;
		a_pathsU[N][ptr_nodes[j][0]] = cost_compU(m[0], P_tempG[j], force_idx[0],j);	// pre-mapping cost for m[0]
	}

	for (i=1; i<(M-w+1); i++) // STEP i = 2..M-w+1
	{
		// save...
		for (u=0; u<N; u++)		// ...for each t-node at step i-1...
			for (v=0; v<N; v++)	// ...the N-element vector defining the remaining processing capacities at t-node (u, i-1):
				P_tempGS[u][v] = P_tempG[u][v];

		for (k=0; k<N; k++)
			for (u=0; u<(N*N); u++)
				B_tempGS[k][u] = B_tempG[k][u];

		for (j=0; j<N; j++) // PROCESSORS - nodes 1..N at step i
		{
			// cost initializations
			a_pathsU[N][ptr_nodes[j][i]] = infinite;	// used for w = 1; initialization with infinite (infeasibility supposed)
			costw_auxx[j] = infinite;					// used for w > 1; initialization with infinite (infeasibility supposed)

			// initialization of indices for final mapping in case that ...
			indx[j][i] = 0;	// ... w > 0 (i.e. for any w)
			indx1_aux  = 0;	// ... w > 1
			indx2_aux  = 0;	// ... w > 2
			indx3_aux  = 0;	// ... w > 3
			indx4_aux  = 0;	// ... w > 4

			for (k1=0; k1<N; k1++)		// PATHS - kth path arriving at node (i,j)
			{
				if (a_pathsU[N][ptr_nodes[k1][i-1]] > (infinite - 1))
				{
					a_pathsU[k1][ptr_nodes[j][i]] = infinite;
					continue;
				}

				// COMPUTATION COST
				for (u=0; u<N; u++)			// N vectors per step
					P_tempL1[k1][u] = P_tempGS[k1][u];

				a_pathsU[k1][ptr_nodes[j][i]] = cost_compU(m[i], P_tempL1[k1], force_idx[i],j);
				
                if (a_pathsU[k1][ptr_nodes[j][i]] < (infinite - 1))
				{	
					// COMMUNICATION COST
					for (u=0; u<(N*N); u++)		// initializing dataflow matix
						B_tempL1[k1][u] = B_tempGS[k1][u];

					costw_comm1 = 0;	// initializing communication cost
					
					// calculate path weight
					for (s=0; s<i; s++)
					{
						if (PP[k1][i-1][s] == j)
							;//mac2x++;
						else
						{
							costw_comm1 += cost_commU(b[s][i], b[i][s], B_tempL1[k1], PP[k1][i-1][s], j);
							if (costw_comm1 > (infinite - 1))
								break;
						}
					}

					if (costw_comm1 < (infinite - 1))
						a_pathsU[k1][ptr_nodes[j][i]] += a_pathsU[N][ptr_nodes[k1][i-1]] + costw_comm1;	// a_pathsU[k1][ptr_nodes[j][i]][0] is total cost at t-node (j,i) for path k1
					else
						a_pathsU[k1][ptr_nodes[j][i]] = infinite;
				
				} //if (a_pathsU[k1][ptr_nodes[j][i]][0] < (infinite - 1))

                if ((w > 1) && (a_pathsU[k1][ptr_nodes[j][i]] < (infinite - 1))) /////////////// w > 1 ////////////////////////////////////
				{
					costw_aux[k1] = infinite; // initialization with infinite (infeasibility supposed)

					for (k2=0; k2<N; k2++)	// level 2
					{
						// ===================== COMPUTATION + COMMUNICATION COST @ level 2 =====================
						// COMPUTATION COST
						for (u=0; u<N; u++)
							P_tempL2[u] = P_tempL1[k1][u]; // initializing remaining processing powers

						costw2 = cost_compU(m[i+1], P_tempL2, force_idx[i+1],k2);

						if (costw2 < (infinite - 1))
						{
							// COMMUNICATION COST
							for (u=0; u<(N*N); u++)				// initializing dataflow matix
									B_tempL2[u] = B_tempL1[k1][u];

							costw_comm2 = 0;	// initializing communication cost

							// communication cost from m[0],m[1],...,m[i-1] to m[i+1]
							for (s=0; s<i; s++)
							{
								if (PP[k1][i-1][s] == k2)
									;//mac2x++;
								else 
								{
									costw_comm2 += cost_commU(b[s][i+1], b[i+1][s], B_tempL2, PP[k1][i-1][s], k2);
									if (costw_comm2 > (infinite - 1))
										break;
								}
							}

							// communication cost from m[i] to m[i+1]
							if (k2 == j)	// m[i+1] is mapped to the same proc as m[i]
								;//mac2x++;
							else if (costw_comm2 < (infinite - 1))	// m[i+1] is mapped to a different proc than m[i]
								costw_comm2 += cost_commU(b[i][i+1], b[i+1][i], B_tempL2, j, k2);

							if (costw_comm2 < (infinite - 1))
								costw2 += a_pathsU[k1][ptr_nodes[j][i]] + costw_comm2; // accumulated cost up to t-node (k2, i+1) due to w-path <k1,k2>
							else
								costw2 = infinite;

						} // ------------------- COMPUTATION + COMMUNICATION COST @ level 2 -------------------

						if ((w > 2) && (costw2 < (infinite - 1))) ////////////////// w = 3, 4, or 5 ///////////////////////////
						{
		                    for (k3=0; k3<N; k3++)	// level 3
							{
								// ===================== COMPUTATION + COMMUNICATION COST @ level 3 =====================
								// COMPUTATION COST
								for (u=0; u<N; u++)
									P_tempL3[u] = P_tempL2[u]; // initializing

								costw3 = cost_compU(m[i+2], P_tempL3, force_idx[i+2], k3);

								if (costw3 < (infinite - 1))
								{
									// COMMUNICATION COST
									for (u=0; u<(N*N); u++)					// initializing dataflow matix
										B_tempL3[u] = B_tempL2[u];

									costw_comm3 = 0;					// initializing communication cost

									// communication cost from m[0],m[1],...,m[i-1] to m[i+2]
									for (s=0; s<i; s++)
									{
										if (PP[k1][i-1][s] == k3)
											;//mac2x++;
										else 
										{
											costw_comm3 += cost_commU(b[s][i+2], b[i+2][s], B_tempL3, PP[k1][i-1][s], k3);
											if (costw_comm3 > (infinite - 1))
												break;
										}
									}

									// communication cost from m[i] to m[i+2]
									if (k3 == j)	// m[i+2] is mapped to the same proc as m[i]
										;//mac2x++;
									else if (costw_comm3 < (infinite - 1))	// m[i+2] is mapped to a different proc than m[i]
										costw_comm3 += cost_commU(b[i][i+2], b[i+2][i], B_tempL3, j, k3);
											
									// communication cost from m[i+1] to m[i+2]
									if (k3 == k2)	// m[i+2] is mapped to the same proc as m[i+1]
										;//mac2x++;
									else if (costw_comm3 < (infinite - 1)) // m[i+2] is mapped to a different proc than m[i+1]
										costw_comm3 += cost_commU(b[i+1][i+2], b[i+2][i+1], B_tempL3, k2, k3);

									if (costw_comm3 < (infinite - 1))
										costw3 += costw2 + costw_comm3;	// accumulated cost up to t-node (k3, i+2) due to w-path <k1,k2,k3>
									else
										costw3 = infinite;

								} // ------------------- COMPUTATION + COMMUNICATION COST @ level 3 -------------------

								if ((w > 3) && (costw3 < (infinite - 1))) ////////////////// w = 4 or 5 ///////////////////////////
								{
									for (k4=0; k4<N; k4++)	// level 4
									{
										// ===================== COMPUTATION + COMMUNICATION COST @ level 4 =====================
										// COMPUTATION COST
										for (u=0; u<N; u++)
											P_tempL4[u] = P_tempL3[u]; // initializing

										costw4 = cost_compU(m[i+3], P_tempL4, force_idx[i+3],k4);

										if (costw4 < (infinite - 1))
										{
											// COMMUNICATION COST
											for (u=0; u<(N*N); u++)	// initializing dataflow matix
		 								        B_tempL4[u] = B_tempL3[u];	// initializing dataflow matix

											costw_comm4 = 0; // initializing communication cost

											// communication cost from m[0],m[1],...,m[i-1] to m[i+3]
											for (s=0; s<i; s++)
											{
												if (PP[k1][i-1][s] == k4)
													;//mac2x++;
												else 
												{
													costw_comm4 += cost_commU(b[s][i+3], b[i+3][s], B_tempL4, PP[k1][i-1][s], k4);
													if (costw_comm4 > (infinite - 1))
													{
														costw_comm4 = infinite;
														break;
													}
												}
											}

											// communication cost from m[i] to m[i+3]
											if (k4 == j)	// m[i+1] is mapped to the same proc as m[i]
												;//mac2x++;
											else if (costw_comm4 < (infinite - 1)) // m[i+1] is mapped to a different proc than m[i]
												costw_comm4 += cost_commU(b[i][i+3], b[i+3][i], B_tempL4, j, k4);
			
											// communication cost from m[i+1] to m[i+3]
											if (k4 == k2)	// m[i+3] is mapped to the same proc as m[i+1]
												;//mac2x++;
											else if (costw_comm4 < (infinite - 1))	// m[i+3] is mapped to a different proc than m[i+1]
												costw_comm4 += cost_commU(b[i+1][i+3], b[i+3][i+1], B_tempL4, k2, k4);
													
											// communication cost from m[i+2] to m[i+3]
											if (k4 == k3)	// m[i+3] is mapped to the same proc as m[i+2]
												;//mac2x++;
											else if (costw_comm4 < (infinite - 1))	// m[i+3] is mapped to a different proc than m[i+2]
												costw_comm4 += cost_commU(b[i+2][i+3], b[i+3][i+2], B_tempL4, k3, k4);

											if (costw_comm4 < (infinite - 1))
											    costw4 += costw3 + costw_comm4;	// accumulated cost up to t-node (k4, i+3) due to w-path <k1,k2,k3,k4>
											else
												costw4 = infinite;
										
										} // ------------------- COMPUTATION + COMMUNICATION COST @ level 4 -------------------
												
										if ((w > 4) && (costw4 < (infinite - 1))) ////////////////// w = 5 ///////////////////////////
										{
											for (k5=0; k5<N; k5++)	// level 5
											{
												// ===================== COMPUTATION + COMMUNICATION COST @ level 5 =====================
												// COMPUTATION COST
												for (u=0; u<N; u++)
													P_tempL5[u] = P_tempL4[u]; // initializing

												costw5 = cost_compU(m[i+4], P_tempL5, force_idx[i+4], k5);

												if (costw5 < (infinite - 1))
												{
													// COMMUNICATION COST
													for (u=0; u<(N*N); u++)	// initializing dataflow matix
	 											        B_tempL5[u] = B_tempL4[u];	// initializing dataflow matix

													costw_comm5 = 0; // initializing communication cost

													// communication cost from m[0],m[1],...,m[i-1] to m[i+4]
													for (s=0; s<i; s++)
													{
														if (PP[k1][i-1][s] == k5)
															;//mac2x++;
														else 
														{
															costw_comm5 += cost_commU(b[s][i+4], b[i+4][s], B_tempL5, PP[k1][i-1][s], k5);
															if (costw_comm5 > (infinite - 1))
															{
																costw_comm5 = infinite;
																break;
															}
														}
													}
									
													// communication cost from m[i] to m[i+4]
													if (k5 == j)	// m[i+4] is mapped to the same proc as m[i]
														;//mac2x++;
													else if (costw_comm5 < (infinite - 1))	// m[i+4] is mapped to a different proc than m[i]
														costw_comm5 += cost_commU(b[i][i+4], b[i+4][i], B_tempL5, j, k5);

													// communication cost from m[i+1] to m[i+4]
													if (k5 == k2)	// m[i+4] is mapped to the same proc as m[i+1]
														;//mac2x++;
													else if (costw_comm5 < (infinite - 1))	// m[i+4] is mapped to a different proc than m[i+1]
														costw_comm5 += cost_commU(b[i+1][i+4], b[i+4][i+1], B_tempL5, k2, k5);

													// communication cost from m[i+2] to m[i+4]
													if (k5 == k3)	// m[i+4] is mapped to the same proc as m[i+2]
														;//mac2x++;
													else if (costw_comm5 < (infinite - 1))	// m[i+4] is mapped to a different proc than m[i+2]
														costw_comm5 += cost_commU(b[i+2][i+4], b[i+4][i+2], B_tempL5, k3, k5);

													// communication cost from m[i+3] to m[i+4]
													if (k5 == k4)	// m[i+4] is mapped to the same proc as m[i+3]
														;//mac2x++;
													else if (costw_comm5 < (infinite - 1))	// m[i+4] is mapped to a different proc than m[i+3]
														costw_comm5 += cost_commU(b[i+3][i+4], b[i+4][i+3], B_tempL5, k4, k5);

													if (costw_comm5 < (infinite - 1))
														costw5 += costw4 + costw_comm5;	// accumulated cost up to t-node (k5, i+4) due to w-path <k1,k2,k3,k4,k5>
													else
														costw5 = infinite;
												
												} // ------------------- COMPUTATION + COMMUNICATION COST @ level 5 -------------------

												if ((costw_aux[k1] - costw5) > threshold)	// correct
												{
													costw_aux[k1] = costw5;

													// final mapping indices & remaining resources
													if (i == M-w)
													{
														indx1_aux = k2;
														indx2_aux = k3;
														indx3_aux = k4;
														indx4_aux = k5;
													
														for (u=0; u<N; u++)
															P_remL[k1][u] = P_tempL5[u];
													
														for (u=0; u<(N*N); u++)
															B_remL[k1][u] = B_tempL5[u];
													}

												}
															
											} // for (k5=0; k5<N; k5++)	// w = 5

										} // if ((w > 4) && (costw4 < (infinite - 1))) ////////////////// w = 5 ///////////////////////////

										else // w = 4
										{
											if ((costw_aux[k1] - costw4) > threshold)	// correct
											{
												costw_aux[k1] = costw4;

												// final mapping indices & remaining resources
												if (i == M-w)
												{
													indx1_aux = k2;
													indx2_aux = k3;
													indx3_aux = k4;

													for (u=0; u<N; u++)
														P_remL[k1][u] = P_tempL4[u];

													for (u=0; u<(N*N); u++)
														B_remL[k1][u] = B_tempL4[u];
												}
											}
										}

									} // for (k4=0; k4<N; k4++)	// w = 4
											
								} // if ((w > 3) && (costw3 < (infinite - 1))) ////////////////// w = 4 ///////////////////////////
										
								else // w = 3
								{
									if ((costw_aux[k1] - costw3) > threshold)	// correct
									{
										costw_aux[k1] = costw3;

										// final mapping indices & remaining resources
										if (i == M-w)
										{
											indx1_aux = k2;
											indx2_aux = k3;

											for (u=0; u<N; u++)
												P_remL[k1][u] = P_tempL3[u];

											for (u=0; u<(N*N); u++)
												B_remL[k1][u] = B_tempL3[u];
										}
									}
								}

							} // for (k3=0; k3<N; k3++)	// w = 3

						} // if ((w > 2) && (costw2 < (infinite - 1))) ////////////////// w = 3 ///////////////////////////
						else // w = 2
						{
							if ((costw_aux[k1] - costw2) > threshold)	// correct
							{
								costw_aux[k1] = costw2;
								// final mapping indices & remaining resources
								if (i == M-w)
								{
									indx1_aux = k2;
									for (u=0; u<N; u++)
										P_remL[k1][u] = P_tempL2[u];

									for (u=0; u<(N*N); u++)
										B_remL[k1][u] = B_tempL2[u];
								}
							}
						}

					} // for (k2=0; k2<N; k2++)	// w = 2
					
					if ((costw_auxx[j] - costw_aux[k1]) > threshold)	// decision over a window of w (correct)
					{
						costw_auxx[j] = costw_aux[k1];
						indx[j][i] = k1; // only the edge index k1 is of interest

						if (i == (M-w))	 // the indices of the for(ward)-tracking in the post-processing stage
						{
							if (w == 5)
							{
								indx1[j] = indx1_aux;
								indx2[j] = indx2_aux;
								indx3[j] = indx3_aux;
								indx4[j] = indx4_aux;
							}
							else if (w == 4)
							{
								indx1[j] = indx1_aux;
								indx2[j] = indx2_aux;
								indx3[j] = indx3_aux;
							}
							else if (w == 3)
							{
								indx1[j] = indx1_aux;
								indx2[j] = indx2_aux;
							}
							else // w = 2
								indx1[j] = indx1_aux;

							for (u=0; u<N; u++)
								P_remG[j][u] = P_remL[indx[j][i]][u] ;

							for (u=0; u<(N*N); u++)
								B_remG[j][u] = B_remL[indx[j][i]][u];
						}
					}

				} // if ((w > 1) && (a_pathsU[k1][ptr_nodes[j][i]][0] < (infinite - 1))) /////////////// w > 1 ////////////////////////////////////

				if (w == 1)
				{
					if ((a_pathsU[N][ptr_nodes[j][i]] - a_pathsU[k1][ptr_nodes[j][i]]) > threshold)	// correct
					{
						a_pathsU[N][ptr_nodes[j][i]] = a_pathsU[k1][ptr_nodes[j][i]];
						indx[j][i] = k1;
					}
				}

			} // PATHS - k1-th path arriving at node (i,j)

			if (w > 1)
                a_pathsU[N][ptr_nodes[j][i]] = a_pathsU[indx[j][i]][ptr_nodes[j][i]];

			// for any w
			if (a_pathsU[N][ptr_nodes[j][i]] < (infinite - 1))
			{
				// save updated processing capacities at t-node (i,j) due to chosen path
				for (u=0; u<N; u++)
					P_tempG[j][u] = P_tempL1[indx[j][i]][u];

				// update bandwidth matrix at t-node (j,i) due to chosen path
				for (u=0; u<(N*N); u++)
					B_tempG[j][u] = B_tempL1[indx[j][i]][u];
			}
			//else
			//{
			//	// set all processing capacities at t-node (j,i) to 0 (may be unnecessary)
			//	for (u=0; u<N; u++)
			//		P_tempG[j][u] = 0;
			//}

			// update pre-mapping information of m[0], m[1], ..., m[i-1] at t-node (j,i) due to the chosen path k1 = indx[j][i]
			for (s=0; s<i; s++)
	            PP[j][i][s] = PP[indx[j][i]][i-1][s];

			// add the pre-mapping of m[i] to P[j] to complete the mapping information at t-node (j,i)
			PP[j][i][i] = j;
		
		} // PROCESSORS - nodes 1..N at step i

	} // STEP i = 2..M-w+1

	// final mapping cost and indices that point to the selected t-nodes at step M, M-1, ..., M-w+1
	if (w == 1)
	{
		for (j=0; j<N; j++)
		{
			if ((cost_t - a_pathsU[N][ptr_nodes[j][M-1]]) > threshold)
			{
				cost_t = a_pathsU[N][ptr_nodes[j][M-1]];
				index = j;
			}
		}
	}
	else // w > 1
	{
		for (j=0; j<N; j++)
		{
			if ((cost_t - costw_auxx[j]) > threshold)	// correct
			{
				cost_t = costw_auxx[j];
				index = j;
				if (w == 5)
				{
					i1 = indx1[j];
					i2 = indx2[j];
					i3 = indx3[j];
					i4 = indx4[j];
				}
				else if (w == 4)
				{
					i1 = indx1[j];
					i2 = indx2[j];
					i3 = indx3[j];
				}
				else if (w == 3)
				{
					i1 = indx1[j];
					i2 = indx2[j];
				}
				else // w = 2
					i1 = indx1[j];
			}
		}
	}

	// final mapping
	if (cost_t < (infinite - 1)) // feasible mapping
	{
		if (w == 5)
		{
			mmap[M-1] = i4;
			mmap[M-2] = i3;
			mmap[M-3] = i2;
			mmap[M-4] = i1;
		}
		else if (w == 4)
		{
			mmap[M-1] = i3;
			mmap[M-2] = i2;
			mmap[M-3] = i1;
		}
		else if (w == 3)
		{
			mmap[M-1] = i2;
			mmap[M-2] = i1;
		}
		else if (w == 2)
			mmap[M-1] = i1;

		mmap[M-w] = index;
		for (i=(M-w); i>0; i--)
			mmap[i-1] = indx[mmap[i]][i];
	}

	// final mapping
	//if (cost_t < (infinite -1))
	//	printf("\nt%d-mapping  (cost = %f):\t",w, cost_t);
	//else
	//	printf("\nt%d-mapping  (cost = %.0f):\t",w, cost_t);
	//for (i=0; i<M; i++)
	//	printf("%d\t",mmap[i]);
	//printf("\n");
	
	// save remaining processing and bandwidth capacities for the next application
	if (cost_t < (infinite - 1))
	{
		if (w==1)
		{
			for (v=0; v<N; v++)
				P[v] = P_tempG[index][v];

			for	(u=0; u<(N*N); u++)
				B[u] = B_tempG[index][u];
		}
		else // w > 1
		{
			for (v=0; v<N; v++)
				P[v] = P_remG[index][v];

			for	(u=0; u<(N*N); u++)
				B[u] = B_remG[index][u];
		}
	}

	return cost_t;
}
