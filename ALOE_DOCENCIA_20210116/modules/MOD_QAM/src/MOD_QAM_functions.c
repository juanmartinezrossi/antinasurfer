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
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <phal_sw_api.h>

#include "print_utils.h"
#include "skeletonSK15.h"
#include "MOD_QAM_functions.h"

extern MODparams_t oParam;


#define QAM4_LEVEL      0.7071    ///< QAM4 amplitude (RMS=1)

#define QAM16_LEVEL_1   0.3162    ///< Low 16-QAM amplitude (RMS=1)
#define QAM16_LEVEL_2   0.9487    ///< High 16-QAM amplitude (RMS=1)
#define QAM16_THRESHOLD	0.6324    ///< 16-QAM threshold for RMS=1 signal

#define QAM64_LEVEL_1	0.1543    ///< Low 16-QAM amplitude (RMS=1)
#define QAM64_LEVEL_2	0.4629    ///< High 16-QAM amplitude (RMS=1)
#define QAM64_THRESHOLD_1	0.3086    ///< 16-QAM threshold for RMS=1 signal
#define QAM64_THRESHOLD_2	0.6172    ///< 16-QAM threshold for RMS=1 signal


/* INIT PHASE FUNCTIONS #####################################################################################################*/



/* WORK PHASE FUNCTIONS #####################################################################################################3*/
//
//  1011    1001    0001    0011
//  1010    1000    0000    0010
//  1110    1100    0100    0110
//  1111    1101    0101    0111
//
int mod_16QAM (char *bits, int numbits, _Complex float *symbols)
{
	int i, j=0;

	if(numbits==0)return(0);

	for (i=0;i<numbits;i+=4)
	{
		symbols[j]  =   ((bits[i+0]==1)?(-1):(+1))*((bits[i+2]==1)?(QAM16_LEVEL_2):(QAM16_LEVEL_1));
		symbols[j] += I*((bits[i+1]==1)?(-1):(+1))*((bits[i+3]==1)?(QAM16_LEVEL_2):(QAM16_LEVEL_1));
		j++;
	}
	if((numbits/4) != j){
		printf("mod_16QAM(): ERROR numbits=%d, j=%d\n", numbits, j);
		exit(0);
	}

//	for(i=0; i<numbits/4; i++)*(symbols+i)=(float)i-((float)i)*I;

	return j;
}


int hard_demod_16QAM (char * bits, complex * symbols, int numsymb)
{
	int i, j=0;
	float in_real, in_imag;

	for (i=0;i<numsymb;i++)
	{
		in_real = creal(symbols[i]);
		in_imag = cimag(symbols[i]);
		bits[j++] = (in_real<0)?1:0;
		bits[j++] = (in_imag<0)?1:0;
		bits[j++] = (fabs(in_real)-QAM16_THRESHOLD<0)?1:0;
		bits[j++] = (fabs(in_imag)-QAM16_THRESHOLD<0)?1:0;
	}
	return j;
}

int soft_demod_16QAM(_Complex float *symbols, int numinputsymb, float *softbits)
{
	int i, j=0;
	float in_real, in_imag;

	// Normalize input
	norm16QAM(symbols, numinputsymb);
	// Demodulate
	for (i=0;i<numinputsymb;i++)
	{
		in_real = -creal(symbols[i]);
		in_imag = -cimag(symbols[i]);
		softbits[j++] = in_real;
		softbits[j++] = in_imag;
		softbits[j++] = fabs(in_real)-QAM16_THRESHOLD;
		softbits[j++] = fabs(in_imag)-QAM16_THRESHOLD;
	}
//	printf("soft_demod_16QAM(): numoutputbits=%d\n", j);
	return j;
}


int norm16QAM(_Complex float *inout, int length){
	int i;
	float auxR, auxI, averg=0.0, Q=0.0; 
	static float ratio=0.0;

	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		averg = averg + auxR + auxI;
	}
	averg=averg/(2.0*(float)length+0.00000001);
	ratio = (QAM16_LEVEL_2)/(averg+Q);		
	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}



