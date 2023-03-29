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

#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <phal_sw_api.h>

//Skeleton includes
#include "print_utils.h"
#include "paramsSK15.h"
#include "skeletonSK15.h"

//Module includes
#include "CIRC_BUFFER.h"
#include "CIRC_BUFFER_params.h"
#include "CIRC_BUFFER_functions.h"

/* MODULE DEFINED PARAMETERS.*/
// Do not modify here
char mname[STR_LEN]="CIRC_BUFFER";
extern MODparams_t oParam;
unsigned long int Tslot=0;			/* Time slot number */
int debugg=0;

//Global Variables
// Module variables: Modify according your needs
_Complex float bufferA[2048];
float bufferB[2048];

// Circular buffer
#define CBUFFER_SZ	128*1024
_Complex float CBuffer[CBUFFER_SZ];
buffctrl buffCtrl;



/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

#ifndef DEBUG_STANDALONE
	sprintf(mname, "%s", GetObjectName());
#endif

	/* Get control parameters value from modulename.params file*/
	get_config_params(&oParam);

	/* Verify control parameters */
	check_config_params(&oParam);

	/* Print Module Init Parameters */
	print_itfs(IN_TYPE, OUT_TYPE);					// Print interfaces information
	print_params(mname, oParam);					// Print module parameters after configuration values

	/* do some other initialization stuff */
	init_functionA_COMPLEX(bufferA, 1024);
	init_functionB_FLOAT(bufferB, 1024);
	//Initialize Circular Buffer
	initCbuff(&buffCtrl, CBuffer, CBUFFER_SZ, 0);

	return 0;
}

/**
 * @brief Function documentation
 *
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address. To obtain the number of received samples use the function
 * int get_input_samples(int idx) where idx is the interface index.
 *
 * @param out Input interface buffers. Data to other interfaces must be stacked in the buffer.
 * Use out(ptr,idx) to access the address.
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 *
 * @code
 * 	input_t *first_interface = inp;
	input_t *second_interface = in(inp,1);
	output_t *first_output_interface = out;
	output_t *second_output_interface = out(out,1);
 */

int work(void *inp, void *out) {

	int snd_samples, rcv_samples;
	input_t *input0;
	output_t *output0;
		

	//printf(BOLD T_RED"WORK() IN %s: Tslot=%d////////////////////////////////////////////////////////////////////////////////////////////////d"RESET"\n", GetObjectName(), Tslot);


	/* GET THE POINTER FOR INPUT AND OUTPUT DATA BUFFERS*/
	input0 = in(inp,0);
	output0 = out(out,0);

	/* GET THE NUMBER OF SAMPLES RECEIVED */
	rcv_samples=get_input_samples(0);


	/* PUT HERE YOUR DSP TASKS ///////////////////////////////*/
	if(debugg){
		printf("Tslot=%d, %s, buffCtrl->writeIndex=%d: rcv_samples=%d\n", Tslot, mname, buffCtrl.writeIndex, rcv_samples);
		printf("%s, buffCtrl->occuplevel=%d: \n", mname,buffCtrl.occuplevel);
		printf("%s, buffCtrl->readIndex=%d: \n", mname,buffCtrl.readIndex);
		printf("%s, bufferCtrl->buffsize=%d: \n", mname,buffCtrl.buffsize);	
	}
	// Copy internal working buffer to output Circular buffer
	if(rcv_samples>0){
		if(writeCbuff(&buffCtrl, CBuffer, input0, rcv_samples)==-1){
			printf("%s ERROR in writing C buffer\n", mname);
			return -1;
		}
	}
	if(debugg)printf("%s: buffCtrl.occuplevel=%d\n", mname, buffCtrl.occuplevel);
	if(buffCtrl.occuplevel >= oParam.outlength){
		readCbuff(&buffCtrl, CBuffer, out, oParam.outlength);
		snd_samples=oParam.outlength;
		if(debugg)printf("%s, outlength=%d\n", mname, oParam.outlength);
	}else snd_samples=0;


	/* INDICATE THE NUMBER OF OUTPUT SAMPLES AT EACH OUTPUT */
	set_output_samples(0, snd_samples);								//	set_output_samples(Output_number, number_of_samples);

	// UPDATE TIME SLOT COUNTER
	Tslot++;
	return 0;														// Always return 1 when successful
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

void print_itfs(char *intype, char *outtype){

	printf(RESET BOLD T_BLACK "==============================================================================================\n");
	printf(RESET);
	printf(RESET BOLD T_BLACK		"    INTERFACES FOR: %s  \n", mname);
	printf(RESET BOLD T_BLUE   			"      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       												nof_input_itf, intype, nof_output_itf, outtype);
/*	printf(RESET BOLD T_BLACK "==============================================================================================\n");*/
}


