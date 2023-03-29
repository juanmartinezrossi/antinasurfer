/*
 * update.c
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

// function that updates the remaining resources
// FULL-DUPLEX, HALF-DUPLEX, or BUS architectures

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mapper.h"

//extern int N;			// number of platform's processors

extern int I[Nmax][Nmax];

//extern int arch;		// architecture indicator; arch = 0: FD, arch = 1: HD, arch = 2: Bus

void update(float bx, float B_rem[], int indx1, int indx2)
{
	// update remaining bandwidth
		B_rem[I[indx1][indx2]] -= bx;
}
