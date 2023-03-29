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

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "paramsSK15.h"
//#include "DDC17.h"

#include "test_generate.h"

int offset=200;

char* input_type="COMPLEXFLOAT";

/**
 * @brief Generates input signal. VERY IMPORTANT to fill length vector with the number of
 * samples that have been generated.
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address.
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 **/


int generate_input_signal(void *input, int *lengths)
{
	int i, data_length=0;
	char *p_aux_c;
	int *p_aux_i;
	float *p_aux_f;
	_Complex float *p_aux_cf;


	//Module Defined Parameters
	float freq=0.05;

	printf("GENERATE INPUT SIGNAL oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
	printf("o  INPUT SIGNAL %s TYPE: Please check in test_generate.generate_input_signal() \n", input_type);
	printf("o  Modify 'input_type' parameter accordingly \n");
	printf("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");

	// Generate Signal
	//Define the buffer pointer for input i: input_t *inputI = in(inp,i); 
	data_length=lengths[0];

	//CHAR
	if(strcmp(input_type, "CHAR")==0){
		p_aux_c=(char *)input;
		printf("Generate CHAR data\n");
		for (i=0;i<data_length;i++) {
			*(p_aux_c+i)=((i+offset)%(data_length));
		}
	}
	//INT
	if(strcmp(input_type, "INT")==0){
		p_aux_i=(int *)input;
		printf("Generate INT data\n");
		for (i=0;i<data_length;i++) {
			*(p_aux_i+i)=(int)((i+offset)%(data_length));
		}
	}
	//FLOAT
	if(strcmp(input_type, "FLOAT")==0){
		p_aux_f=(float *)input;
		printf("Generate FLOAT data\n");;
		for (i=0;i<data_length;i++) {
			*(p_aux_f+i)=(float)((i+offset)%(data_length));
		}
	}

	//COMPLEX
	if(strcmp(input_type, "COMPLEXFLOAT")==0){
		p_aux_cf=(_Complex float *)input;
		printf("Generate COMPLEXFLOAT data\n");
		for (i=0;i<data_length;i++) {
			__real__ *(p_aux_cf+i)=(float)(cos(2*3.1415*((double)i)*freq)+0.1*cos(2*3.1415*((double)i)*2*freq));
			__imag__ *(p_aux_cf+i)=(float)(sin(2*3.1415*((double)i)*freq)+0.1*sin(2*3.1415*((double)i)*2*freq));
		}
	}

	//Provide the data length generated
	lengths[0] = data_length;
	return 0;
}
