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

#include "CPLX_FILTER_interfaces.h"
#include "CPLX_FILTER_functions.h"
#include "CPLX_FILTER.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="CPLX_FILTER";
float samplingfreqHz=1.0;

//Module User Defined Parameters
int opmode=0;										//CPLX=0, ...
char filtercoefffile[STR_LEN]="FIR.coeff";
int filterlength=0;
_Complex float COEFFs[2048];

MODparams_t oParam={opMODEBYPASS, 1, 1, "FIR.coeff"};

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	sprintf(mname, "%s", GetObjectName());
	/* Get control parameters*/
//	param_get_int("opmode", &opmode);
//	param_get_string("filtercoefffile", &filtercoefffile[0]);	
	param_get_int("opmode", &oParam.opMODE);
	param_get_int("Ninterpol", &oParam.Ninterpol);
	param_get_int("Ndecimate", &oParam.Ndecimate);
	param_get_string("filtercoefffile", &oParam.filtercoefffile[0]);			

	/* Verify control parameters */
	if(oParam.Ninterpol == 0 || oParam.Ndecimate == 0){
		printf("%s: ERROR!!! values for Ninterpol=%d or Ndecimate=%d must be different from zero.\n", 
					mname, oParam.Ninterpol, oParam.Ndecimate);
		exit(0);
	}


	// CPLX
	//if(opmode==1)readCPLXfilecoeff(COEFFs, &filterlength, filtercoefffile);
	if(oParam.opMODE==1)readCPLXfilecoeff(COEFFs, &filterlength, &oParam.filtercoefffile[0]);
	// REAL
	//if(opmode==2)readREALfilecoeff(COEFFs, &filterlength, filtercoefffile);
	if(oParam.opMODE==2)readREALfilecoeff(COEFFs, &filterlength, &oParam.filtercoefffile[0]);

	/* Print Module Init Parameters */
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      opmode=%d: 0-Bypass, 1-CPLX filter, 2-Real filter\n", oParam.opMODE);
	printf("O      filtercoefffile=%s, filterlength=%d\n", &oParam.filtercoefffile[0], filterlength);
	printf("O      Ninterpol=%d, Ndecimate=%d\n", oParam.Ninterpol, oParam.Ndecimate);
	printf("O--------------------------------------------------------------------------------------------O\n");


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
	int snd_samples=0;
	input_t *input = inp;
	input_t *output = out;
	int i;
	
	// Check if data received. Do nothing if data not received.
	if (rcv_samples == 0)return(0);

	/* do DSP stuff here */

	//if(opmode == 1 || opmode == 2)snd_samples = stream_conv_CPLX(input, rcv_samples, COEFFs, filterlength, output);
	if(oParam.opMODE == 1 || oParam.opMODE == 2){
		snd_samples = stream_conv_CPLX_INT_DEC(input, rcv_samples, COEFFs, filterlength, output, oParam);
	}
	else snd_samples=bypass(input, rcv_samples, output);

	// Indicate the number of samples at output number N
	//	set_output_samples(N, out_samples_at_N_port);

	// Indicate the number of samples at autput 0 with return value
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


