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
#define SLENGTH			32
#define DEBUGG			0
#define GENERATOR		1
#define BYPASS			2
#define NORMAL			3
#define MAXOPERATIONS	300

typedef struct MODparams{
    int opMODE;
		int datalength;
    int num_operations;
    float constant;
    char datatext[SLENGTH];
}MODparams_t;

/*************************************************************************************************/
// Functions Predefinition
void check_config_params();
int init_functionA_COMPLEX(_Complex float *input, int length);
int functionA_COMPLEX(_Complex float *input, int lengths, _Complex float *output);
int init_functionB_FLOAT(float *input, int length);
int functionB_FLOAT(float *input, int lengths, float *output);
int functionC_INT(int *in, int length, int *out);
int functionD_CHAR(char *in, int length, char *out);
int bypass_func(char *datatypeIN, char *datatypeOUT, void *datin, int datalength, void *datout);

#endif
