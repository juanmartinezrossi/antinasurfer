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

#ifndef SKELETON_H
#include "skeleton.h"
#endif

#define FILTERLENGTH	2048
#define CCINIT			0
#define CCSTREAM		1

#define opMODEBYPASS	0
#define opMODECPLX		1
#define opMODEREAL		2

#define STR_LENGTH		128

typedef struct MODparams{
    int opMODE;
    int Ninterpol;
	int Ndecimate;
	char filtercoefffile[STR_LENGTH];
}MODparams_t;




int init_functionA_COMPLEX(_Complex float *input, int length);
int functionA_COMPLEX(_Complex float *input, int lengths, _Complex float *output);
int init_functionB_FLOAT(float *input, int length);
int functionB_FLOAT(float *input, int lengths, float *output);
void readCPLXfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername);
void readREALfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername);
int stream_conv_CPLX(_Complex float *ccinput, int datalength,_Complex float *filtercoeff, int filterlength, _Complex float *ccoutput);
int stream_conv_CPLX_INT_DEC(	_Complex float *ccinput, 
											int datalength,
											_Complex float *filtercoeff, 
											int filterlength, 
											_Complex float *ccoutput, 
											MODparams_t oParam);
int bypass(_Complex float *input, int datalength,_Complex float *output);
#endif
