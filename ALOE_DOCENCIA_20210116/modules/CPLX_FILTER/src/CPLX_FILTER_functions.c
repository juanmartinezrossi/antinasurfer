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
#include <string.h>

#include "CPLX_FILTER_functions.h"

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


/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
void readCPLXfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername){

	char str[128];
	float auxf;
	int num, k=0;
	double real=0.0, imag=0.0;
	char creal[32], cimag[32];
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
		while( (num = fscanf( hFile, "%lf %c %lfi\n", &real, &sign, &imag)) > 0 ) {
			if( sign == '-' )imag *= -1;
	    	filtercoeff[k] = (float)real + ((float)imag)*I;
//			printf("real=%1.9f, imag=%1.9f\n", __real__ filtercoeff[k], __imag__ filtercoeff[k]);
			k++;
		}

     	if(k != *filterlength){
			printf("\033[1m\033[31mWARNING!!!!. Please, verify the %s file. The filter length do not match the captured coefficients\033[0m\n", filtername);
         	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
		}
    }
//	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
    fclose(hFile);
}


void readREALfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername){

	char str[128];
	float auxf;
	int num, k=0;
	float real=0.0, imag=0.0;
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
		while( (num = fscanf( hFile, "%f\n", &real)) > 0 ) {
	    	__real__ filtercoeff[k] = real;
				__imag__ filtercoeff[k] = 0.0;
				//__imag__ filtercoeff[k] = real;
//			printf("real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[k], __imag__ filtercoeff[k]);
			k++;
		}
     	if(k != *filterlength){
			printf("\033[1m\033[31mWARNING!!!!. Please, verify the %s file. The filter length do not match the captured coefficients\033[0m\n", filtername);
         	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
		}
    }
//	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
    fclose(hFile);
}


int bypass(_Complex float *input, int datalength,_Complex float *output){
	int i;
	for(i=0; i<datalength; i++){
		*(output+i)=*(input+i);
	}
	return datalength;
}


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

/**
 * @stream_conv_CPLX_DDC(): Perform continous convolution and interpolate by INTER and 
 *                          decimate by DECIM
 * @param.
 * @oParaml: Refers to the struct that containts the module parameters
 * @return: Number of output samples
 */

int stream_conv_CPLX_INT_DEC(	_Complex float *ccinput, 
											int datalength,
											_Complex float *filtercoeff, 
											int filterlength, 
											_Complex float *ccoutput, 
											MODparams_t oParam){

	int i, j, k=0;
	static int first=0;
	static _Complex float aux[FILTERLENGTH];
	int INTER=9, DECIM=3;
	static int n=0, m=0;

	INTER=oParam.Ninterpol;
	DECIM=oParam.Ndecimate;

//	printf("readCPLXfilecoeff(): real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[10], __imag__ filtercoeff[10]);
//	printf("ccinput(): real=%3.6f, imag=%3.6f\n", __real__ ccinput[10], __imag__ ccinput[10]);
//	printf("INT=%d, DEC=%d, datalength=%d, n=%d, m=%d\n", INTER, DEC, datalength, n, m);

	if(first==0){
		for(j=0; j<filterlength; j++){
			aux[j]=0.0+0.0i;
		}
		first=1;
	}

	for (i=0;i<datalength*INTER;i++) {
		if(n==0){
			for (j=filterlength-2;j>=0;j--) {
				aux[j+1]=aux[j];
			}
			aux[0]=ccinput[i/INTER];
		}
		
		if(m==0){
			ccoutput[k]=0.0;
			for (j=0;j<filterlength/INTER;j++) {
				ccoutput[k]+=aux[j]*filtercoeff[j*INTER+n];
			}
			k++;
		}
		n++;							
		if(n==INTER)n=0;		
		m++;
		if(m==DECIM)m=0;
	}
	//printf("datalength*INT/DEC=%d\n", datalength*INTER/DEC);
	return (datalength*INTER/DECIM);
}




