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

#include "DDC_interfaces.h"
#include "DDC_functions.h"
#include "DDC.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="DDC";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;

//Module User Defined Parameters
float floatp=111.11;
char stringp[STR_LEN]="MY_DEFAULT_INIT";

//Global Variables

#define TABLESZ		48000
float reffreq=1.0;
_Complex float tableA[TABLESZ];
_Complex float bufferA[2048];
float bufferB[2048];

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 48000 Hz

* 0 Hz - 1750 Hz
  gain = 1
  desired ripple = 1 dB
  actual ripple = 0.6933174974612244 dB

* 3000 Hz - 24000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.70526605009705 dB

*/

#define FILTER_TAP_NUM 64

static double filter_taps[FILTER_TAP_NUM] = {
  -0.0015278787824383208,
  0.005189546423673026,
  0.00444319107260174,
  0.005167946588298267,
  0.006091641683589329,
  0.006796667555213517,
  0.007066941255539607,
  0.006754702779521099,
  0.005757303638846453,
  0.004026338362771543,
  0.0015930544817086595,
  -0.0014386537668729944,
  -0.004877021851001289,
  -0.008454187219170628,
  -0.011835165678550535,
  -0.014641794743453776,
  -0.016476933627169208,
  -0.016960287154148202,
  -0.015763182028569876,
  -0.012642561526388357,
  -0.00746909997864814,
  -0.00025207281056707515,
  0.008853489130940756,
  0.019543557634210493,
  0.03137465752998033,
  0.04379235152406595,
  0.05615917722379909,
  0.06781060516273235,
  0.07807466736391258,
  0.08637649250874552,
  0.0922046678429991,
  0.0952065325525313,
  0.0952065325525313,
  0.0922046678429991,
  0.08637649250874552,
  0.07807466736391258,
  0.06781060516273235,
  0.05615917722379909,
  0.04379235152406595,
  0.03137465752998033,
  0.019543557634210493,
  0.008853489130940756,
  -0.00025207281056707515,
  -0.00746909997864814,
  -0.012642561526388357,
  -0.015763182028569876,
  -0.016960287154148202,
  -0.016476933627169208,
  -0.014641794743453776,
  -0.011835165678550535,
  -0.008454187219170628,
  -0.004877021851001289,
  -0.0014386537668729944,
  0.0015930544817086595,
  0.004026338362771543,
  0.005757303638846453,
  0.006754702779521099,
  0.007066941255539607,
  0.006796667555213517,
  0.006091641683589329,
  0.005167946588298267,
  0.00444319107260174,
  0.005189546423673026,
  -0.0015278787824383208
};

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("block_length", &block_length);		//Initialized by hand or config file
	param_get_int("run_times", &run_times);			//Initialized by hand or config file
	param_get_string("plot_modeIN", &plot_modeIN[0]);	//Initialized by hand or config file
	param_get_string("plot_modeOUT", &plot_modeOUT[0]);
	param_get_float("samplingfreqHz", &samplingfreqHz);
	param_get_float("floatp", &floatp);
	param_get_string("stringp", &stringp[0]);

	/* Verify control parameters */
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your DDC_interfaces.h file\n");
		return -1;
	}
	/*Include the file name and line number when printing*/
	modinfo_msg("Parameter block_length is %d\n",block_length);	//Print message and parameter 
	modinfo("Parameter block_length \n");				//Print only message

	/* Print Module Init Parameters */
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      block_length=%d, run_times=%d, sampligfreqHz=%3.3f\n", block_length, run_times, samplingfreqHz);
	printf("O      plot_modeIN=%s, plot_modeOUT=%s\n", plot_modeIN, plot_modeOUT);
	printf("O      floatp=%3.3f, stringp=%s\n", floatp, stringp);
	printf("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */
	init_functionA_COMPLEX(bufferA, 1024);
//	init_functionB_FLOAT(bufferB, 1024);
	init_toneCOMPLEX(tableA, TABLESZ, 1.0);

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
	if (rcv_samples == 0)return(0);
	static int Tslot=-1;
	Tslot++;
	int N=8; //factor interpol
	int snd_samples=rcv_samples/N;
	_Complex float tone[rcv_samples];
	_Complex float ans[rcv_samples];
	gen_toneCOMPLEX(tone, tableA,TABLESZ,rcv_samples, 3500, 48000); // cos, -sin
	float cos;
	float sin;
	//I/Q DEMOD
	int i, k;

	for(i=0;i<rcv_samples;i++)
	{
		cos = __real__ tone[i];
		sin = __imag__ tone[i];
		__real__ ans[i] =   cos * __real__ inp[i];
		__imag__ ans[i] =   sin * __real__ inp[i];
	}
	
	//LPF CONV
	stream_conv_2REALS(ans,rcv_samples,filter_taps,64,ans);

	
	//DE-INTERPOL
	
	/*	for(int i=0;i<rcv_samples;i++){
		if(i%N==0) out[i/N]=ans[i];
	}    */
	
	for(i=0;i<snd_samples;i++){
		k=0;
		out[i] = 0.0+0.0i;
		while(k<N){
			out[i] += ans[i*N+k];
			k++;
		}
	}
//	memcpy(out,ans,sizeof(_Complex float)*rcv_samples); //I/Qout+LPF -> out
//  return rcv_samples;
	if(Tslot == 0) for(i=686;i<696;i++) printf("DDC %d: %f + %fi\n",i,__real__ out[i], __imag__ out[i]);
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


