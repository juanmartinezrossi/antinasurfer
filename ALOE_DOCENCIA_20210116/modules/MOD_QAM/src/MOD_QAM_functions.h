/* 
 * Copyright (c) 2012.
 * This file is part of ALOE (http://flexnets.upc.edu/)
 * 
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

// Module Variables and Defines

#define SLENGTH						32
#define MODULATE					0
#define HARD_DEMODULATE		1
#define SOFT_DEMODULATE		2

#define M_BPSK						0
#define M_4QAM						1
#define M_16QAM						2
#define M_64QAM						3

#define NODEBUGG					0
#define DEBUGG						1

typedef struct MODparams{
    int opMODE;									// 0: Modulate; 1: Hard Demodulate; 2: Soft Demodulate
		int modulation;							// 0: BPSK, 1: 4QAM, 2: 16QAM, 3: 64QAM
		int debugg;									// 0: No debugg, 1: Debug
}MODparams_t;

static MODparams_t oParam={MODULATE, M_16QAM, NODEBUGG};		// Initialize module params struct


/*************************************************************************************************/
// Functions Predefinition
int mod_16QAM (char *bits, int numbits, _Complex float *symbols);
int hard_demod_16QAM (char *bits, complex *symbols, int numsymb);
int soft_demod_16QAM (_Complex float *symbols, int numinputsymb, float * softbits);
int norm16QAM(_Complex float *inout, int length);



#endif
