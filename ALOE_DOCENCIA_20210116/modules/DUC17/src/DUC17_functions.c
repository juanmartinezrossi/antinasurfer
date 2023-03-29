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
#include "DUC17_functions.h"

// Global Variables
// Sinus cosinus generation
#define TABLESZ		48000
_Complex float tableA[TABLESZ];
_Complex float sentcosDUC[1024*1024];
// Filter
int filterlength=0;
_Complex float COEFFs[FILTERLENGTH];
extern _Complex float aux[];
// Module
extern MODparams_t oParam;


/* INIT PHASE FUNCTIONS #####################################################################################################*/

void readREALfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername){

	char str[128];
	float auxf;
	int num, k=0;
	double real=0.0, imag=0.0;
	char sign;
	
	//Reading the file from .txt file
  FILE *hFile;
	hFile = fopen(filtername, "r");

    if (hFile == NULL){
        printf("\033[1m\033[31mERROR!!!. FILE %s NOT FOUND\033[0m\n", filtername);// Error, file not found
    }
    else{
        fscanf(hFile, "%f", &auxf);
		*filterlength=(int)auxf;
		while( (num = fscanf( hFile, "%lf\n", &real)) > 0 ) {
				filtercoeff[k] = (float)real + ((float)imag)*I;
	    	//			printf("readCPLXfilecoeff(): real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[k], __imag__ filtercoeff[k]);
			k++;
		}
     	if(k != *filterlength){
			printf("\033[1m\033[31mWARNING!!!!. Please, verify the %s file. The filter length do not match the captured coefficients\033[0m\n", filtername);
         	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
		}
    }
	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
    fclose(hFile);
}



/**
 * @stream_conv_CPLX(): Perform continous convolution and interpolate by INTER and 
 *                          decimate by DECIM
 * @param.
 * @oParaml: Refers to the struct that containts the module parameters
 * @return: Number of output samples
 */
int stream_conv_CPLX(	_Complex float *ccinput, 
											int datalength,
											_Complex float *filtercoeff, 
											int filterlength, 
											_Complex float *ccoutput, 
											MODparams_t oParam){

	int i, j, k=0;
	static int first=0;
	static _Complex float aux[FILTERLENGTH];
	int INT=9, DEC=3;
	static int n=0, m=0;

	INT=oParam.Ninterpol;
	DEC=oParam.Ndecimate;

	if(first==0){
		for(j=0; j<filterlength; j++){
			aux[j]=0.0+0.0i;
		}
		first=1;
	}
	for (i=0;i<datalength*INT;i++) {
		if(n==0){
			for (j=filterlength-2;j>=0;j--) {
				aux[j+1]=aux[j];
			}
			aux[0]=ccinput[i/INT];
		}
		if(m==0){
			ccoutput[k]=0.0;
			for (j=0;j<filterlength/INT;j++) {
				ccoutput[k]+=aux[j]*filtercoeff[j*INT+n];
			}
			k++;
		}
		n++;							
		if(n==INT)n=0;		
		m++;
		if(m==DEC)m=0;
	}
	return (datalength*INT/DEC);
}


/*
// OK but not efficient
int stream_conv_CPLX(_Complex float *ccinput, int datalength,_Complex float *filtercoeff, int filterlength, _Complex float *ccoutput){

	int i, j;
	static int first=0;
	static _Complex float aux[FILTERLENGTH];

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
			ccoutput[i]+=aux[j]*filtercoeff[j];
		}
	}
	return datalength;
}
*/


int init_toneCOMPLEX(_Complex float *table, int length, float gain){
	int i;
	double arg=PIx2/((float)length);

	for (i=0;i<length;i++) {
		__real__ table[i]=gain*(float)cos(arg*(double)i);
		__imag__ table[i]=-gain*(float)sin(arg*(double)i);
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


int IQ_modulator_carrier_f0(_Complex float *phasor_f0, _Complex float *in, _Complex float *out, int length){
	int i;

	for(i=0; i<length; i++){
		__real__ *(out+i)=(__real__*(phasor_f0+i)) * (__real__ *(in+i)) + (__imag__*(phasor_f0+i)) * (__imag__ *(in+i));
		__imag__ *(out+i)=0.0;
	}
	return length;
}


/**
 * @bypass_func(): Copy length samples from input to output
 * @param.
 * @datatypeIN: Identifies the input data type 
 * @datatypeOUT: Identifies the output data type 
 * @datalength: Number of samples to be copied 
 * @return 1
 */
int bypass_func(_Complex float *datin, int datalength, _Complex float *datout){
	int i;

	memcpy(datout, datin, sizeof(_Complex float)*datalength);
	return(1);
}


int IQ_MOD(_Complex float *ccinput, int datalength, _Complex float *ccoutput, MODparams_t oParam){

	// Filter Low Pass
	datalength = stream_conv_CPLX(ccinput, datalength, COEFFs, filterlength, aux, oParam);
	// Move spectrum fo Hz
	// 1) Generate phasor f0 Hz
	gen_toneCOMPLEX(sentcosDUC, tableA, TABLESZ, datalength, oParam.f0_freqHz, oParam.samplingfreqHz);
	// 2) Modulate IQ carrier fo
	datalength=IQ_modulator_carrier_f0(sentcosDUC, aux, ccoutput, datalength);
	return(datalength);
}



void initDUC(MODparams_t oParam){
	// INIT TABLE SIN COS
	init_toneCOMPLEX(tableA, TABLESZ, oParam.gain);
	// READ FILTER COEFF
	readREALfilecoeff(COEFFs, &filterlength, &oParam.filtercoefffile);
}


