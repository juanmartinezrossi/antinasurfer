/*
 * sort_b.c
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

// sort processes in descending order of bandwith requirements

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mapper.h"

int M;	// number of application's tasks/SDR functions/processes

extern float b[Mmax][Mmax];		// first unsorted, then sorted b-matrix
extern float m_sort[Mmax];		// sorted m-vector


void sort_b(float m[])
{
	//general
	int i, j;						// loop indices: i - row index, j - column index
	int n = 0;						// counter variable used for sorting processes
	float b_aux[Mmax][Mmax];		// b-matrix copy
	float b_sortx[Mmax][Mmax];	// intermediate sorted b-matrix
	int ord[Mmax];
	int stop = 0;
	float aux;
	int i_indx, j_indx;

	// initialization
	for (i=0; i<M; i++)
		ord[i] = -1;

	for (i=0; i<M; i++)
		for (j=0; j<M; j++)
			b_aux[i][j] = b[i][j];


	while (stop == 0)
	{
		stop = 1;
		aux = 0;
		for (i=0; i<(M-1); i++)
		{
			for (j=i+1; j<M; j++)
			{
				if (b_aux[i][j] > aux)	// if m[j+1] > m[j]: interchange m[j+1] with m[j]
				{
					aux = b_aux[i][j];
					i_indx = i;
					j_indx = j;
					stop = 0;	// continue
				}
			}
		}
		if (stop == 0)
		{
			b_aux[i_indx][j_indx] = 0;
			if (ord[i_indx] == -1)
			{
				ord[i_indx] = n;
				n++;
			}
			if (ord[j_indx] == -1)
			{
				ord[j_indx] = n;
				n++;
			}
		}
	}

	for (i=0; i<M; i++)
		for (j=0; j<M; j++)
			b_sortx[ord[i]][j] = b[i][j];

	for (i=0; i<M; i++)
		for (j=0; j<M; j++)
			b[j][ord[i]] = b_sortx[j][i];

	for (i=0; i<M; i++)
		m_sort[ord[i]] = m[i];
}
