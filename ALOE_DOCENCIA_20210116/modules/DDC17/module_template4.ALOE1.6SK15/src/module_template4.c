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
#include "module_template4.h"
#include "module_template4_params.h"
#include "module_template4_functions.h"


//#define DEBUG_STANDALONE									// Uncomment that line when in StandAlone Mode

/* MODULE DEFINED PARAMETERS.*/
// Do not modify here
char mname[STR_LEN]="module_template4";
extern MODparams_t oParam;
unsigned long int numTS;			/* Time slot number */

//Global Variables
// Module variables: Modify according your needs
_Complex float bufferA[2048];
float bufferB[2048];

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
	static int Tslot=0;
	int snd_samples, rcv_samples;
	input_t *input0;
	output_t *output0;
		
#ifndef DEBUG_STANDALONE
	printf(BOLD T_RED"WORK() IN %s: Tslot=%d////////////////////////////////////////////////////////////////////////////////////////////////d"RESET"\n", GetObjectName(), Tslot);
#endif

	/* GET THE POINTER FOR INPUT AND OUTPUT DATA BUFFERS*/
	input0 = in(inp,0);
	output0 = out(out,0);

	/* GET THE NUMBER OF SAMPLES RECEIVED */
	rcv_samples=get_input_samples(0);

	/* DO YOUR CHECKS BEFORE DSP TASKS
	//	if (rcv_samples == 0)return(1); 							// Only as example. Check if data received: Nothing to do if no data received ??
	//  if (wrong status) return (0);								// Return 0 if wrong condition for normal operation

	// Check if data exceed maximum expected data
	if (rcv_samples > oParam.num_operations) {
		...
		printf("Wrong condition\n");
		return(0);
	}
	*/

	/* DEBUG ONLY: PRINT INPUT DATA*/
	print_array("IN", IN_TYPE, input0, 16, 8);


	/* PUT HERE YOUR DSP TASKS ///////////////////////////////*/
	if(oParam.opMODE==GENERATOR){
		functionA_COMPLEX(bufferA, oParam.datalength, output0);
		snd_samples=oParam.datalength;
	}
	if(oParam.opMODE==BYPASS){
		bypass_func(IN_TYPE, OUT_TYPE, input0, rcv_samples, output0);
		snd_samples=oParam.datalength;
	}

	/* DEBUG ONLY: PRINT OUTPUT DATA*/
	print_array("OUT", OUT_TYPE, output0, 16, 8);

	/* INDICATE THE NUMBER OF OUTPUT SAMPLES AT EACH OUTPUT */
	set_output_samples(0, snd_samples);								//	set_output_samples(Output_number, number_of_samples);

	// UPDATE TIME SLOT COUNTER
	Tslot++;
	return 1;														// Always return 1 when successful
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


