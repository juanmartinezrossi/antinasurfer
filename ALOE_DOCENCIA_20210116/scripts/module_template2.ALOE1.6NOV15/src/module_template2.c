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

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "module_template2_interfaces.h"
#include "module_template2_functions.h"
#include "module_template2.h"

char mname[STR_LEN]="module_template2";
// Module Defined Parameters.
#define BYPASS	0
#define NORMAL	1
int p_opMODE=BYPASS;
int p_num_operations=100;
float p_constant=2.77;
#define SLENGTH	64
char p_datatext[SLENGTH]="DEFAULT";

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
	/* Get control parameters*/
	param_get_int("p_opMODE", &p_opMODE);							//Initialized by hand or config file
	param_get_int("p_num_operations", &p_num_operations);			//Initialized by hand or config file
	param_get_float("p_constant", &p_constant);
	param_get_string("p_datatext", &p_datatext[0]);

	/* Verify control parameters */
	if (p_num_operations > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: p_num_operations=%d > INPUT_MAX_DATA=%d\n", p_num_operations, INPUT_MAX_DATA);
		moderror("Check your module_template2_interfaces.h file\n");
		return -1;
	}
	/*Include the file name and line number when printing*/
	modinfo_msg("Parameter p_num_operations is %d\n",p_num_operations);	//Print message and parameter
	modinfo("Parameter p_num_operations \n");							//Print only message

	/* Print Module Init Parameters */
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s.Initialize()\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      p_opMODE=%d, p_num_operations=%d, p_constant=%3.3f\n", p_opMODE, p_num_operations, p_constant);
	printf("O      p_datatext=%s, p_datatext=%s\n", p_datatext, p_datatext);
	printf("O--------------------------------------------------------------------------------------------O\n");

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
 *
 */
int work(input_t *inp, output_t *out) {
	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
//	int i,k;
	
	// Check if data received
	if (rcv_samples == 0)return(0);
	// Check if data exceed maximum expected data
	if (rcv_samples > p_num_operations) {
		/* ... */
	}
	printf("WORK() IN: rcv_samples=%d, p_num_operations=%d\n", rcv_samples, p_num_operations);
	/* do DSP stuff here */
	if(p_opMODE==BYPASS){
	//	if(IN_TYPE=="COMPLEXFLOAT" && OUT_TYPE=="COMPLEXFLOAT"){
		if(strcmp(IN_TYPE,"COMPLEXFLOAT")==0){
			functionA_COMPLEX((_Complex float *)inp, rcv_samples, (_Complex float *)out);
		}
		if(strcmp(IN_TYPE,"FLOAT")==0){
			functionB_FLOAT((float *)inp, rcv_samples, (float *)out);
		}
		if(strcmp(IN_TYPE,"INT")==0){
			functionC_INT((int *)inp, rcv_samples, (int *)out);
		}
		if(strcmp(IN_TYPE,"CHAR")==0){
			functionD_CHAR((char *)inp, rcv_samples, (char *)out);
		}

	}

	// Indicate the number of samples at output number N
	//	set_output_samples(N, out_samples_at_N_port);
	printf("WORK() OUT: rcv_samples=%d\n", rcv_samples);
	// Indicate the number of samples at autput 0 with return value
	return rcv_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


