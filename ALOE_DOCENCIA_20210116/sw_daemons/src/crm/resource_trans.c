/*
 * resource_trans.c
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mapper.h"

// GLOBAL VARIABLES
// actual number of processors and tasks
int N;	// number of platform's processors
int M;	// number of application's tasks/SDR functions/processes

// resource models
extern float L[Nmax][Nmax];		// bandwidth resources
extern int I[Nmax][Nmax];		// bandwidth resources
extern float B[Nmax*Nmax];		// bandwidth resources

// architecture
extern int arch;		// architecture indicator; arch = 0: FD, arch = 1: HD, arch = 2: Bus

void resource_trans(void)
{

	int i, j;		// loop indices
	int count;			// integer counter

	if (arch == fd)
	{
		count = N;
		for (i=0; i<N; i++)
		{
			for (j=0; j<N; j++)
			{
				if (i == j)
				{
					I[i][i] = i;
					B[i] = inf;
				}
				else
				{
					I[i][j] = count;
					B[count] = L[i][j];
					count++;
				}
			}
		}
	}
	else if (arch == hd)
	{
		count = N;
		for (i=0; i<N; i++)
		{
			for (j=i; j<N; j++)
			{
				if (i == j)
				{
					I[i][i] = i;
					B[i] = inf;
				}
				else
				{
					I[i][j] = count;
					I[j][i] = count;
					B[count] = L[i][j];
					count++;
				}
			}
		}
	}
	else if (arch == bus)
	{
		count = N;
		for (i=0; i<N; i++)
		{
			for (j=0; j<N; j++)
			{
				if (i == j)
				{
					I[i][i] = i;
					B[i] = inf;
				}
				else
				{
					I[i][j] = count;
				}
			}
		}
		B[count] = L[0][1];	// B_bus
	}
	else
		exit(1);
}	
