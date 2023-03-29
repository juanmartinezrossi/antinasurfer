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
#include "DUC17.h"
#include "DUC17_params.h"
#include "DUC17_functions.h"

/* MODULE DEFINED PARAMETERS.*/
char mname[STR_LEN]="DUC17";
extern MODparams_t oParam;
unsigned long int numTS;			/* Time slot number */

//Global Variables
_Complex float aux[INPUT_MAX_SAMPLES*MAXINTERPOLATION];

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

	// INIT DUC
	initDUC(oParam);

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
	static int Tslot=-1;
	int snd_samples=0, rcv_samples;
	input_t *input0;
	output_t *output0;

	Tslot++;

	/* GET THE POINTER FOR INPUT AND OUTPUT DATA BUFFERS*/
	input0 = in(inp,0);
	output0 = out(out,0);

	if(oParam.opMODE==BYPASS){
		rcv_samples=get_input_samples(0);
//	printf("DUC-BYPASS: Tslot=%d, rcv_samples=%d\n", Tslot, rcv_samples);
		if(rcv_samples==0)return(1);
		bypass_func( input0, rcv_samples, output0);
		snd_samples=rcv_samples;
	}
	if(oParam.opMODE==NORMAL){
		// Check if data received
		rcv_samples=get_input_samples(0);
//			printf("DUC-NORMAL: Tslot=%d, rcv_samples=%d\n", Tslot, rcv_samples);
		if(rcv_samples==0)return(1);
		snd_samples=IQ_MOD(input0, rcv_samples, output0, oParam);
	}
	/* INDICATE THE NUMBER OF OUTPUT SAMPLES AT EACH OUTPUT */
	set_output_samples(0, snd_samples);	//	set_output_samples(Output_number, number_of_samples);

	// UPDATE TIME SLOT COUNTER

	return 1;														// Always return 1 when successful
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}




