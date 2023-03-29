/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
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

#include "channel_noise_interfaces.h"
#include "channel_noise_functions.h"
#include "channel_noise.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="channel_noise";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;

//Module User Defined Parameters
float SNR=-1.0;
float gain=1.0;
int mode=0;

//Global Variables
#define MAXOFFSET	20
_Complex float noise[INPUT_MAX_DATA];
float bufferB[2048];
float variance=0.0;
int Tslot=0;
_Complex float buffer[INPUT_MAX_DATA];
int maxoffset=MAXOFFSET;
int moffset=0;
/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	/* Get control parameters*/
	param_get_int("block_length", &block_length);		//Initialized by hand or config file
	param_get_int("run_times", &run_times);			//Initialized by hand or config file
	param_get_string("plot_modeIN", &plot_modeIN[0]);	//Initialized by hand or config file
	param_get_string("plot_modeOUT", &plot_modeOUT[0]);
	param_get_float("samplingfreqHz", &samplingfreqHz);
	param_get_int("mode", &mode);
	param_get_float("SNR", &SNR);

	SNR=SNR-10.0;

	/* Verify control parameters */
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your channel_noise_interfaces.h file\n");
		return -1;
	}
	/*Include the file name and line number when printing*/
	modinfo_msg("Parameter block_length is %d\n",block_length);	//Print message and parameter 
	modinfo("Parameter block_length \n");				//Print only message
	

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("\033[1;32;40m\tO--------------------------------------------------------------------------------------------O\t\033[0m\n");
	printf("\033[1;32;40m\tO    SPECIFIC PARAMETERS SETUP: %s						\t\033[0m\n", mname);
	printf("\033[1;32;40m\tO      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s	\t\033[0m\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("\033[1;32;40m\tO      block_length=%d, run_times=%d, sampligfreqHz=%3.3f				\t\033[0m\n", block_length, run_times, samplingfreqHz);
	printf("\033[1;32;40m\tO      MODE=%d: 0) Calculate Noise Power; 1) Variance, gain=%1.1f, SNR=%3.1fdBs		\t\033[0m\n", mode, gain, SNR);
	printf("\033[1;32;40m\tO--------------------------------------------------------------------------------------------O\t\033[0m\n");
//	printf("\n");

	/* do some other initialization stuff */
	variance=get_variance(SNR,1.0);
	//SNR=(pow(10,-SNR/10));
	SNR=(pow(10,SNR/10));


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
	float power_in, power_noise;
	int i, j;
	output_t *out2 = out(out,1);


	// Do nothing if data not received
	if (rcv_samples == 0)return(0);

	if(mode == 0){
		// Calculate average power of input data
		power_in=aver_power(inp, rcv_samples);
		// Generate Noise
		gen_noise_c(noise, 1.0, rcv_samples);
		// Calculate power generated noise
		power_noise=aver_power(noise, rcv_samples);
		// Correct noise level according expected SNR. SNR=Psignal/(Gain*Pnoise)
		gain=power_in/(SNR*power_noise);
		gain=sqrt(gain);
//		printf("%s: SNR_lineal=%1.1f, power_in=%1.6f W, power_noise=%1.6f, gain=%1.6f \n", 
//					mname, SNR, power_in, power_noise, gain);
	}
	if(mode == 1){
		// Calculate Variance
		gen_noise_c(noise,variance,rcv_samples);
		gain=1.0;
	}

	// Add Noise
	for(i=0; i<rcv_samples; i++){
		*(out+i)=*(inp+i)+gain*(*(noise+i));
		*(out2+i)=*(out+i);
	}
	set_output_samples(1, rcv_samples);
	
	Tslot++;
	return rcv_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


