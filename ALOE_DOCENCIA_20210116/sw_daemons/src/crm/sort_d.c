/*
 * sort_d.c
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

// sort processes in descending order

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mapper.h"

int M;	// number of application's tasks/SDR functions/processes

extern float b[Mmax][Mmax];		// first unsorted, then sorted b-matrix
extern float m_sort[Mmax];		// sorted m-vector


void sort_d(float m[])
{
	//general
	int i, j;					// loop indices: i - row index, j - column index
	int idx_temp;				// temporary sort index
	float m_temp;
	int idx[Mmax];				// sort indices
	float b_sortx[Mmax][Mmax];	// intermediate sorted b-matrix


	// initialization
	for (i=0; i<M; i++)
		idx[i] = i;

	for (i=0; i<M; i++)
		m_sort[i] = m[i];


	for (i=0; i<(M-1); i++)
	{
		for (j=0; j<(M-1); j++)
		{	// sort in descending order!
			if (m_sort[j+1] > m_sort[j])	// if m[j+1] > m[j]: interchange m[j+1] with m[j]
			{
				idx_temp = idx[j];
				idx[j] = idx[j+1];
				idx[j+1] = idx_temp;

				m_temp = m_sort[j];
				m_sort[j] = m_sort[j+1];
				m_sort[j+1] = m_temp;

			}
		}
	}

	for (i=0; i<M; i++)
		for (j=0; j<M; j++)
			b_sortx[i][j] = b[idx[i]][j];
	
	for (i=0; i<M; i++)
		for (j=0; j<M; j++)
			b[j][i] = b_sortx[j][idx[i]];
}
