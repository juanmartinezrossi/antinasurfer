/* 
 * Copyright (c) 2012
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

#include "DDC_functions.h"

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int init_functionA_COMPLEX(_Complex float *input, int length){
	int i;

	printf("INIT MY FUNCTION\n");

	for (i=0;i<length;i++) {
		if(i>20) __real__ input[i]=(float)(i*i);
		__imag__ input[i]=(length-i-1)%(length);
	}
	return(1);
}

int init_toneCOMPLEX(_Complex float *table, int length, float gain){
	int i;
	double arg=PIx2/((float)length);

	for (i=0;i<length;i++) {
		__real__ table[i]=gain*(float)cos(arg*(float)i);
		__imag__ table[i]=-1*gain*(float)sin(arg*(float)i);
	}
	return(1);
}

int gen_toneCOMPLEX(_Complex float *func_out, _Complex float *tablel, int tablesz,
					int datalen, float tone_freq, float sampl_freq){
	int i, k=1;
	static int j=0;
	float ref_freq_c;

	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		func_out[i] = tablel[j];
		j += k;
		if(j>=tablesz)j-=tablesz;
	}
	return(1);
}

int init_functionB_FLOAT(float *input, int length){
	int i;

	printf("INIT MY FUNCTION\n");

	for (i=0;i<length;i++) {
		if(i>20) input[i]=(float)(i*i);
	}
	return(1);
}

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int functionA_COMPLEX(_Complex float *input, int inlength, _Complex float *output)
{
	int i,outlength;

//	printf("RUN MY FUNCTION\n");	
	for (i=0;i<inlength;i++) {
		__real__ output[i] = __real__ input[i];
		__imag__ output[i] = __imag__ input[i];
	}
	outlength=inlength;
	return outlength;
}

int stream_conv_2REALS(_Complex float *ccinput, int datalength,double *filtercoeff, int filterlength, _Complex float *ccoutput)
{

	int i, j;
	static int first=0;
	static _Complex float aux[64];

	if(first==0){
		for(j=0; j<filterlength; j++){
			aux[j]=0.0+0.0i;
			//printf("FIRST CCONV real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[j], __imag__ filtercoeff[j]);
		}
		first=1;
	}

	for (i=0;i<datalength;i++) {
		for (j=filterlength-2;j>=0;j--) {
			aux[j+1]=aux[j];
		}
		aux[0]=ccinput[i];
		ccoutput[i]=0.0;
		for (j=0;j<filterlength;j++) {
			__real__ ccoutput[i]+=__real__ aux[j]*(float)filtercoeff[j];
			__imag__ ccoutput[i]+=__imag__ aux[j]*(float)filtercoeff[j];

		}
	}
	return datalength;
}

int functionB_FLOAT(float *input, int inlength,float *output)
{
	int i, outlength;

//	printf("RUN MY FUNCTION\n");	
//	for(k=0; k<length; k++)printf("Real input[%d]=%3.4f\n",k,  __real__ input[k]);
	for (i=0;i<inlength;i++) {
		output[i] = input[i];
	}
//	for(k=0; k<length; k++)printf("Real output[%d]=%3.4f\n",k,  __real__ output[k]);
	outlength=inlength;
	return outlength;
}

