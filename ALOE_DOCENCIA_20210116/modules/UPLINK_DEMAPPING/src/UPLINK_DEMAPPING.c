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

#include "UPLINK_DEMAPPING_interfaces.h"
#include "UPLINK_DEMAPPING_functions.h"
#include "UPLINK_DEMAPPING.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="UPLINK_DEMAPPING";


//Module User Defined Parameters

//Global Variables
int FFTsize=256;
int DATAsize=0;

#define NORMAL				0				// Normal Operation Mode
int opMODE=NORMAL;


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("opMODE", &opMODE);		//Initialized by hand or config file
	param_get_int("FFTsize", &FFTsize);

	//Number of carriers per FFTsize
	if(FFTsize==128)DATAsize=72-CARRIERS1RB*2;
	if(FFTsize==256)DATAsize=180-CARRIERS1RB*2;
	if(FFTsize==512)DATAsize=300-CARRIERS1RB*2;
	if(FFTsize==1024)DATAsize=600-CARRIERS1RB*2;
	if(FFTsize==1536)DATAsize=900-CARRIERS1RB*2;
	if(FFTsize==2048)DATAsize=1200-CARRIERS1RB*2;

	/* Verify control parameters */


	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      opMODE=%d, FFTsize=%d, DATAsize=%d\n", opMODE, FFTsize, DATAsize);
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
	int i,j,numFFTs;
	
	// Check if data received
	if (rcv_samples == 0)return(0);
	// Check if data exceed maximum expected data
	numFFTs=rcv_samples/FFTsize;
	if (rcv_samples != numFFTs*FFTsize) {
		printf("%s| ERROR: Received samples=%d is not multiple of FFTsize=%d\n", rcv_samples, FFTsize);
		return(-1);
	}
	/* do DSP stuff here */
	j=0;
	if(opMODE==NORMAL){
		for(i=0; i<numFFTs; i++){
//			if(i!=3 && i!=10){
				getDATAfromLTEspectrum(&inp[FFTsize*i], FFTsize, DATAsize, &out[DATAsize*j]);
				snd_samples += DATAsize;
				j++;
//			}
		}
	}

//	printf("%s OUT: snd_samples=%d\n", mname, snd_samples);
	// Indicate the number of samples at output 0 with return value
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


