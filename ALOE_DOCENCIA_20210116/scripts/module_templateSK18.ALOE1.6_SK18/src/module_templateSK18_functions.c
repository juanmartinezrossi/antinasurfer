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
#include "module_templateSK18_functions.h"

/** Initialize Module Parameters. A default value is ussually required */
MODparams_t oParam={BYPASS, 256, 100, 7.9, "BABY"};	

/* INIT PHASE FUNCTIONS #####################################################################################################*/
/**
 * @check_config_params(). Check any value that should be checked and act accordingly before RUN phase
 * @param.
 * @oParaml Refers to the struct that containts the module parameters
 * @return: Void
 */
void check_config_params(){
	char maux[128];

	/** Perfom the check of all parameters*/
	/** In case of unnaceptable parameters values stop execution*/
	if (oParam.num_operations > MAXOPERATIONS) {
		// Generate text to be printed
		sprintf(maux, "ERROR!!!: p_num_operations=%d > MAXOPERATIONS=%d\n", oParam.num_operations, MAXOPERATIONS);
		// printCOLORtext(int position, char *fore, char *ground,  int end, char *data2print)
		// with fore and groun values between 0-255. Find color numbers at:
		// http://www.bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux
		printCOLORtextline(5, "208", "000",  100, maux);
		sprintf(maux, "Check your module_templateSK18_functions.h file");
		printCOLORtextline(5, "208", "000",  100, maux);
		// Stop execution due to errors
		exit(EXIT_FAILURE);
	}
	/** In case of aceptable parameters values but not the expected one show WARNING*/
	// This is an example. Modify according your needs.
	if (oParam.num_operations*2 > MAXOPERATIONS) {
		// Generate text to be printed
		sprintf(maux, "WARNING!!!: p_num_operations=%d > MAXOPERATIONS=%d", oParam.num_operations*2, MAXOPERATIONS);
		// printCOLORtext(int position, char *fore, char *ground,  int end, char *data2print)
		// with fore and groun values between 0-255. Find color numbers at:
		// http://www.bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux
		printCOLORtextline(5, "208", "000",  100, maux);
		sprintf(maux, "Check your module_templateSK18_functions.h file");
		printCOLORtextline(5, "208", "000",  100, maux);
	}

}



/**
 * @init_functionA_COMPLEX(). Generates and array of complex float
 * @param.
 * @input: Pointer to the complex float array
 * @length: Number of samples generated
 * @return: 1
 */
int init_functionA_COMPLEX(_Complex float *input, int length){
	int i;

	for (i=0;i<length;i++) {
		__real__ input[i]=(float)(i);
		__imag__ input[i]=(float)(-i);
//		printf("init_functionA_COMPLEX\n");
	}
	return(1);
}

/**
 * @init_functionB_FLOAT(). Generates and array of floats
 * @params.
 * @input: Pointer to the complex float array
 * @length: Number of samples generated
 * @return: 1
 */
int init_functionB_FLOAT(float *input, int length){
	int i;

	for (i=0;i<length;i++) {
		if(i>20) input[i]=(float)(i*i);
	}
	return(1);
}


/* WORK PHASE FUNCTIONS #####################################################################################################3*/
/**
 * @functionA_COMPLEX(): Copy length samples from input to output
 * @param.
 * @input: 
 * @inlengths Save on n-th position the number of samples generated for the n-th interface
 * @return the number of samples copied
 */
int functionA_COMPLEX(_Complex float *input, int inlength, _Complex float *output){
	int i,outlength;
	static int z=0;

	for (i=0;i<inlength;i++) {
		__real__ output[i] = __real__ input[i];
		__imag__ output[i] = __imag__ input[i];
	}
	z++;
	outlength=inlength;
	return outlength;
}

int functionB_FLOAT(float *input, int inlength,float *output){
	int i, outlength;

	for (i=0;i<inlength;i++) {
		output[i] = input[i];
	}
	outlength=inlength;
	return outlength;
}


int functionC_INT(int *in, int length, int *out){
	int i;

	for(i=0; i<length; i++){
		*(out+i)=*(in+i);
	}
	return 1;
}


int functionD_CHAR(char *in, int length, char *out){
	int i;

	for(i=0; i<length; i++){
		*(out+i)=*(in+i);
	}
	return 1;
}

/**
 * @bypass_func(): Copy length samples from input to output
 * @param.
 * @datatypeIN: Identifies the input data type 
 * @datatypeOUT: Identifies the output data type 
 * @datalength: Number of samples to be copied 
 * @return 1
 */
int bypass_func(char *datatypeIN, char *datatypeOUT, void *datin, int datalength, void *datout){

	if(strcmp(datatypeIN,datatypeOUT) != 0){
		printf("ERROR!!!: Bypass not possible. Input and Output data types are different\n");
		printf("ERROR!!!: Please, check the IN_TYPE and OUT_TYPE in module_interfaces.h file.\n");
	}
	if(strcmp(datatypeIN,"COMPLEXFLOAT")==0){
		functionA_COMPLEX((_Complex float *)datin, datalength, (_Complex float *)datout);
	}
	if(strcmp(datatypeIN,"FLOAT")==0){
		functionB_FLOAT((float *)datin, datalength, (float *)datout);
	}
	if(strcmp(datatypeIN,"INT")==0){
		functionC_INT((int *)datin, datalength, (int *)datout);
	}
	if(strcmp(datatypeIN,"CHAR")==0){
		functionD_CHAR((char *)datin, datalength, (char *)datout);
	}
	return(1);
}





