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

#ifndef _INTERFACES_H
#define _INTERFACES_H

/* input and output data types */
typedef _Complex float  input_t;			//_Complex float, float, int, char
#define IN_TYPE				"COMPLEXFLOAT"		//COMPLEXFLOAT, FLOAT, INT, CHAR
#define INPUT_MAX_SAMPLES 	10*2048*64
#define NOF_INPUT_ITF		1

//typedef _Complex float  output_t;
typedef _Complex float  output_t;			//_Complex float, float, int, char
#define OUT_TYPE			"COMPLEXFLOAT"		//COMPLEXFLOAT, FLOAT, INT, CHAR
#define OUTPUT_MAX_SAMPLES 	10*2048
#define NOF_OUTPUT_ITF		1


/********* DO NOT MODIFY BEYOND HERE */
#ifndef INCLUDE_DEFS_ONLY
#define INCLUDE_DEFS_ONLY
/* Input and output buffer sizes (in number of samples) */
const int input_max_samples = INPUT_MAX_SAMPLES;
const int output_max_samples = OUTPUT_MAX_SAMPLES;

/* leave these two lines unmodified */
const int input_sample_sz = sizeof(input_t);
const int output_sample_sz = sizeof(output_t);

/* Number of I/O interfaces. All have the same maximum size */
const int nof_input_itf = NOF_INPUT_ITF;
const int nof_output_itf = NOF_OUTPUT_ITF;
/* Number of I/O interfaces. All have the same maximum size */
const char* in_type = IN_TYPE;
const char* out_type = OUT_TYPE;

#endif
#endif
