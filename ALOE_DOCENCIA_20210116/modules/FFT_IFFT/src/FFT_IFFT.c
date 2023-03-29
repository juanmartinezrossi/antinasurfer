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


#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <fftw3.h>
#include <stdlib.h>
#include <string.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "FFT_IFFT_interfaces.h"
#include "FFT_IFFT_functions.h"
#include "FFT_IFFT.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="FFT_IFFT";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;

//Module User Defined Parameters
int fftsize=128;
int numffts=1;
int op_mode=1; //1: FFT; 2: IFFT,
int FFTdir;

//Global Variables
#define FFTMAXSZ	8192
fftw_complex fftin[FFTMAXSZ], fftout[FFTMAXSZ];
fftw_plan fftplan;
fftw_plan iffplan;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

//	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
//	param_get_int("block_length", &block_length);		//Initialized by hand or config file
//	param_get_int("run_times", &run_times);			//Initialized by hand or config file
//	param_get_string("plot_modeIN", &plot_modeIN[0]);	//Initialized by hand or config file
//	param_get_string("plot_modeOUT", &plot_modeOUT[0]);
	param_get_float("samplingfreqHz", &samplingfreqHz);
	param_get_int("fftsize", &fftsize);			//Initialized by hand or config file
	param_get_int("numffts", &numffts);			//Initialized by hand or config file
	param_get_int("op_mode", &op_mode);			//Initialized by hand or config file

	/* Verify control parameters */
	block_length=fftsize*numffts;
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your FFT_IFFT_interfaces.h file\n");
		return -1;
	}
	/*Include the file name and line number when printing*/
	modinfo_msg("Parameter block_length is %d\n",block_length);	//Print message and parameter 
	modinfo("Parameter block_length \n");				//Print only message

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1m\033[34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
//	printf("O      block_length=%d, run_times=%d, sampligfreqHz=%3.3f\n", block_length, run_times, samplingfreqHz);
	printf("O      sampligfreqHz=%3.3f\n", samplingfreqHz);
//	printf("O      plot_modeIN=%s, plot_modeOUT=%s\n", plot_modeIN, plot_modeOUT);
	printf("O      fftsize=%d, numffts=%d, op_mode=%d (1:FFT, 2:IFFT)\n", fftsize, numffts, op_mode);
	printf("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */
	//my_plan = fftw_plan_dft_1d(128, (fftw_complex *)datain, (fftw_complex *)dataout, FFTdir, FFTW_ESTIMATE);
	if(op_mode==1){
		fftplan = fftw_plan_dft_1d(fftsize, fftin, fftout, FFTW_FORWARD, FFTW_ESTIMATE);
	}else if(op_mode==2){
		fftplan = fftw_plan_dft_1d(fftsize, fftin, fftout, FFTW_BACKWARD, FFTW_ESTIMATE);
	}else {
		printf("FFT_IFFT.initialize(): ERROR op_mode=%d Non Valid\n", op_mode);
		return(-1);
	}

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
	input_t *input;
	output_t *output;
	int rcv_samples = get_input_samples(0), snd_samples; /** number of samples at itf 0 buffer */
	int i,k,n, j=0;
	static unsigned long int Tslot=0;
	input_t AUX[INPUT_MAX_DATA];

	static int num=0;

	input=in(inp,0);
	output=out(out,0);

//	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>FFT_IFFT: rcv_samples=%d\n", rcv_samples);

	if (rcv_samples == 0)return(0);
	if (rcv_samples%fftsize != 0 ) {
		printf("%s.work(): ERROR rcv_samples = %d is not multiple of FFTsize=%d\n", mname, rcv_samples, fftsize);
		return(-1);
	}
	numffts=rcv_samples/fftsize;

	for(i=0; i<numffts; i++){
		j=i*fftsize;
		for(n=0; n<fftsize; n++)*(fftin+n)=(fftw_complex) *(input+j+n);
		//Execute FFT
		fftw_execute(fftplan);
		//Copy processed data to output buffer
			if(op_mode==1)for(n=0; n<fftsize; n++)*(output+j+n)=(_Complex float) *(fftout+n);
			if(op_mode==2)for(n=0; n<fftsize; n++)*(output+j+n)=((_Complex float) *(fftout+n))/fftsize;
	}
	snd_samples=rcv_samples;
	// Indicate the number of samples at autput 0 with return value

	Tslot++;
	return rcv_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


