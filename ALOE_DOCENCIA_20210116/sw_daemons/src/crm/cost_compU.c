/*
 * cost_compU.c
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

// Function that calculates cost_comp

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mapper.h"

extern float q1;

/** IGM: Included constrain variable
 */
float cost_compU(float mx, float P_rem[], int force_idx, int Pindx)
{
	//int i;
	float cost;	// computing cost

	/** IGM: Constrain the mapping to force_idx processor
	 */
	if (force_idx>=0) {
		if (force_idx!=Pindx)
			return infinite;
	}

	//if (mx > P_rem[indx])				// infeasible
	if ((mx - P_rem[Pindx]) > threshold)	// infeasible
		cost = infinite;				// cost = infinite
	else
	{
		cost = q1*mx/P_rem[Pindx];		// cost = required_computing_power / available_computing_power
		P_rem[Pindx] -= mx;	// update remaining processing capacity
	}

	return cost;
}
