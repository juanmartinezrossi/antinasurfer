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
#include "CIRC_BUFFER_functions.h"

extern MODparams_t oParam;


/* INIT PHASE FUNCTIONS #####################################################################################################*/

void print_params(char *mnamel, MODparams_t oParaml){

//	printf(RESET BOLD T_WHITE "==============================================================================================\n");
	printf(RESET);
	printf(RESET BOLD T_LIGHTRED		"    INITIALIZED PARAMS FOR: %s  \n", mnamel);
	printf(RESET BOLD T_GREEN			"      opMODE=%d, expectedinputlength=%d, outlength=%d\n", oParaml.opMODE, oParaml.expectedinputlength, oParaml.outlength);

	printf(RESET BOLD T_BLACK "==============================================================================================\n");
}

/**
 * @check_config_params(). Check any value that should be checked and act accordingly before RUN phase
 * @param.
 * @oParaml Refers to the struct that containts the module parameters
 * @return: Void
 */
void check_config_params(MODparams_t *oParaml){

	//if (oParaml->num_operations > MAXOPERATIONS) {
	//	printf("ERROR: p_num_operations=%d > MAXOPERATIONS=%d\n", oParaml->num_operations, MAXOPERATIONS);
	//	printf("Check your module_interfaces.h file\n");
	//	exit(EXIT_FAILURE);
	//}
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
		__imag__ input[i]=(float)(-i);;
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


void initCbuff(buffctrl *bufferCtrl, _Complex float *bufferC, int buffersz, int firstpcketsz){

	memset(bufferC, 0, sizeof(_Complex float)*buffersz);
	bufferCtrl->writeIndex = firstpcketsz;
	bufferCtrl->readIndex = firstpcketsz;
	bufferCtrl->buffsize = buffersz;
	bufferCtrl->occuplevel=0;
	bufferCtrl->roomlevel=buffersz;

/*	printf("buffCtrl->writeIndex=%d: \n", bufferCtrl->writeIndex);
	printf("buffCtrl->occuplevel=%d: \n", bufferCtrl->occuplevel);
	printf("buffCtrl->readIndex=%d: \n", bufferCtrl->readIndex);
	printf("bufferCtrl->buffsize=%d: \n", bufferCtrl->buffsize);
	printf("*bufferC=%u: \n", (_Complex float *)bufferC);
*/
}


/**
* void write(buffer* buffer, _Complex float value, int length)
* writes value into the buffer
* @param buffer* buffer
*   pointer to buffer to be used
* @param _Complex float value
*   value to be written in buffer
*/

int writeCbuff(buffctrl *buffer, _Complex float *buffdata, _Complex float *in, int length){
	int i;

	if(buffer->writeIndex >= buffer->readIndex){
		buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
	}
	else{
		buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
	}
	buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
	if(buffer->roomlevel >= length){
		for(i=0; i<length; i++){
			buffdata[buffer->writeIndex]=*(in+i);
			buffer->writeIndex++;
			if(buffer->writeIndex==buffer->buffsize){
				buffer->writeIndex=0;
			}
		}
		if(buffer->writeIndex >= buffer->readIndex){
			buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
		}
		else{
			buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
		}
		buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
		return 1;
	}else{
		printf("Error writeCbuff: Not enough space in buffer\n");
		printf("buffCtrl->writeIndex=%d: \n", buffer->writeIndex);
		printf("buffCtrl->occuplevel=%d: \n", buffer->occuplevel);
		printf("buffCtrl->readIndex=%d: \n", buffer->readIndex);
		printf("bufferCtrl->buffsize=%d: \n", buffer->buffsize);	
		return -1;
	}
}
/**
* void readn(buffer* buffer, int Xn)
* reads specified value from buffer
* @param buffer* buffer
*   pointer to buffer to be read from
* @param int Xn
*   specifies the value to be read from buffer counting backwards from the most recently written value
*   i.e. the most recently writen value can be read with readn(buffer, 0), the value written before that with readn(buffer, 1)
*/
int readCbuff(buffctrl* buffer,_Complex float *buffdata, _Complex float *out, int length){
	int i;

	if(buffer->writeIndex >= buffer->readIndex){
		buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
	}
	else{
		buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
	}
	buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
	if(buffer->occuplevel >= length){
//		printf("readCbuff(): buffer->occuplevel=%d, length=%d, buffer->readIndex=%d\n", buffer->occuplevel, length, buffer->readIndex);
		for(i=0; i<length; i++){
//			if(i==0)printf("*bufferC=%u: \n", (_Complex float *)(buffdata+buffer->readIndex));//printf("buffer->readIndex=%d\n", buffer->readIndex);
			*(out+i) = *(buffdata+buffer->readIndex);
			buffer->readIndex++;

			if(buffer->readIndex==buffer->buffsize){
				buffer->readIndex=0;
			}
		}
		if(buffer->writeIndex >= buffer->readIndex){
			buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
		}
		else{
			buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
		}
		buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
		return 1;
	}else{
/*		modinfo("Error readCbuff: Not enough data in buffer.\n");
		modinfo_msg("buffer->occuplevel=%d, length=%d\n", buffer->occuplevel, length);
		modinfo_msg("buffer->buffsize=%d, buffer->writeIndex=%d, buffer->readIndex=%d\n", buffer->buffsize, buffer->writeIndex, buffer->readIndex);
*/
		printf("Error readCbuff: Not enough data in buffer.\n");
		printf("buffer->occuplevel=%d, length=%d\n", buffer->occuplevel, length);
		printf("buffer->buffsize=%d, buffer->writeIndex=%d, buffer->readIndex=%d\n", buffer->buffsize, buffer->writeIndex, buffer->readIndex);

		return -1;
	}
}


