/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
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
#include <math.h>

#include "data_source_functions.h"

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */

int init_toneCOMPLEX(_Complex float *table, int tablesz, float gain, float sampl_freq){
	int i;
	double arg=PIx2/((float)tablesz);

	for (i=0;i<tablesz;i++) {
		__real__ table[i]=gain*(float)cos(arg*(double)i);
		__imag__ table[i]=gain*(float)sin(arg*(double)i);
	}
	return(1);
}

int gen_toneCOMPLEX(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float tone_freq, float sampl_freq){
	int i, k=1;
	static unsigned long int j=0;
	float ref_freq_c;

	double arg=PIx2*(double)tone_freq/(double)sampl_freq;

/*	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		func_out[i] = tablel[j];
		j += k;
		if(j>=tablesz)j-=tablesz;
	}
*/

	for (i=0;i<datalen;i++) {
		__real__ func_out[i] = (float)cos(arg*(double)j);
		__imag__ func_out[i] = (float)sin(arg*(double)j);
		j++;
	}
	return(1);
}

int gen_toneCOMPLEX2(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float tone_freq, float sampl_freq){
	int i, k=1;
	static unsigned long int j=0;
	float ref_freq_c;
	double arg=PIx2*(double)tone_freq/(double)sampl_freq;

/*	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		func_out[i] = tablel[j];
		j += k;
		if(j>=tablesz)j-=tablesz;
	}
*/
	for (i=0;i<datalen;i++) {
		__real__ func_out[i] = (float)cos(arg*(double)j);
		__imag__ func_out[i] = (float)sin(arg*(double)j);
		j++;
	}

	return(1);
}

int gen_toneCOMPLEX3(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float tone_freq, float sampl_freq){
	int i, k=1;
	static unsigned long int j=0;
	float ref_freq_c;
	double arg=PIx2*(double)tone_freq/(double)sampl_freq;

/*	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		func_out[i] = tablel[j];
		j += k;
		if(j>=tablesz)j-=tablesz;
	}
*/
	for (i=0;i<datalen;i++) {
		__real__ func_out[i] = (float)cos(arg*(double)j);
		__imag__ func_out[i] = (float)sin(arg*(double)j);
		j++;
	}

	return(1);
}

int gen_toneCOMPLEX4(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float tone_freq, float sampl_freq){
	int i, k=1;
	static unsigned long int j=0;
	float ref_freq_c;
	double arg=PIx2*(double)tone_freq/(double)sampl_freq;

/*	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		func_out[i] = tablel[j];
		j += k;
		if(j>=tablesz)j-=tablesz;
	}
*/
	for (i=0;i<datalen;i++) {
		__real__ func_out[i] = (float)cos(arg*(double)j);
		__imag__ func_out[i] = (float)sin(arg*(double)j);
		j++;
	}

	return(1);
}

int gen_toneCOMPLEX5(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float tone_freq, float sampl_freq){
	int i, k=1;
	static unsigned long int j=0;
	float ref_freq_c;
	double arg=PIx2*(double)tone_freq/(double)sampl_freq;

/*	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		func_out[i] = tablel[j];
		j += k;
		if(j>=tablesz)j-=tablesz;
	}
*/
	for (i=0;i<datalen;i++) {
		__real__ func_out[i] = (float)cos(arg*(double)j);
		__imag__ func_out[i] = (float)sin(arg*(double)j);
		j++;
	}

	return(1);
}


int gen_NtonesCOMPLEX(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float sampl_freq, float init_freq, float freq_step, float final_freq){
	int i, k=1, N=1, m;
	static unsigned long int j=0;
	float ref_freq_c, tone_freq;
	double arg=PIx2*(double)tone_freq/(double)sampl_freq;

	N = (int)((final_freq-init_freq)/(freq_step));

/*	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		func_out[i] = tablel[j];
		j += k;
		if(j>=tablesz)j-=tablesz;
	}
*/
	for (i=0;i<datalen;i++)func_out[i] = 0.0+0.0*I;
	
	for(m=0; m<N; m++){
		tone_freq = init_freq+freq_step*(float)m;
		arg=PIx2*(double)tone_freq/(double)sampl_freq;
		for (i=0;i<datalen;i++) {
			__real__ func_out[i] += (float)cos(arg*(double)j);
			__imag__ func_out[i] += (float)sin(arg*(double)j);
			j++;
		}
	}
	return(1);
}

void gen_spectrum(_Complex float *out, int block_length){
	int i, j=0;

		for (i=0;i<block_length;i++) {
			if(j<37 || j > 128-36){
				__real__ out[i] = 1.0;
				__imag__ out[i] = 1.0;
				if(i%2 == 0)out[i]=0.0+0.0*I;
			}else out[i]=0.0+0.0*I;
		j++;
	}
}



