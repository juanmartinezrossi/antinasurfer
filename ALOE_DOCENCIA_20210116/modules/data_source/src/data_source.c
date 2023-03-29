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

#include "data_source_interfaces.h"
#include "data_source_functions.h"
#include "data_source.h"

//ALOE Module Defined Parameters. Do not delete.
// Normal Execution Mode
char mname[STR_LEN]="data_source";
char signaltype[STR_LEN]="TONE_COMPLEX";
int block_length=48000;
float samplingfreqHz=48000.0;
float gain=1.0;
float reffreq=1.0;
float tonefreq=1024.0;

// Standalone Mode
int run_times=1;

char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";



//Module User Defined Parameters
char outputtype[STR_LEN]=OUT_TYPE;

//Global Variables
#define TABLESZ		48000*5
_Complex float tableA[TABLESZ];
_Complex float bufferA[INPUT_MAX_DATA];
_Complex float bufferB[INPUT_MAX_DATA];
_Complex float bufferC[INPUT_MAX_DATA];
_Complex float bufferD[INPUT_MAX_DATA];
_Complex float bufferE[INPUT_MAX_DATA];
/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {
	float aux;

	//printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("block_length", &block_length);		//Initialized by hand or config file
	param_get_int("run_times", &run_times);			//Initialized by hand or config file
	param_get_float("samplingfreqHz", &samplingfreqHz);
	param_get_float("gain", &gain);
	param_get_float("tonefreq", &tonefreq);
	param_get_string("signaltype", &signaltype[0]);
	param_get_string("plot_modeOUT", &plot_modeOUT[0]);
//	param_get_string("plot_modeIN", &plot_modeIN[0]);	//Initialized by hand or config file

	aux=(float)TABLESZ;
	reffreq=samplingfreqHz/aux;

	/* Verify control parameters */
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your data_source_interfaces.h file\n");
		return -1;
	}
	/*Include the file name and line number when printing*/
	modinfo_msg("Parameter block_length is %d\n",block_length);	//Print message and parameter 
	modinfo("Parameter block_length \n");				//Print only message

	/* Print Module Init Parameters */
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: %s.initialize().\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      block_length=%d, run_times=%d, signaltype=%s\n", block_length, run_times, signaltype);
	printf("O      sampligfreqHz=%3.1fHz, gain=%3.1f, reffreq=%3.1fHz, tonefreq=%3.1fHz\n",
				   samplingfreqHz, gain, reffreq, tonefreq);
	printf("O      plot_modeOUT=%s\n", plot_modeOUT);
	printf("O--------------------------------------------------------------------------------------------O\n");

	/* Check if output data are fine*/
	if(strcmp(signaltype, "TONE_COMPLEX")==0)if(strcmp(outputtype, "COMPLEXFLOAT")!=0){
		printf("   ERROR!!!: %s. Signal generated don't match output data type\n", mname);
		printf("   ERROR!!!: Please, modify data type accordingly in %s_interfaces.h\n", mname);
	}

	if(tonefreq > (samplingfreqHz/3.0)){
		printf("%s: WARNING!!! tone_frequency generated exceeding (sampling frequency)/3\n", mname);
		printf("%s: Please, reduce tonefreq in paramcfg/%s.params\n", mname, mname);
	}
		
	/* Initialize generate functions */
	init_toneCOMPLEX(tableA, TABLESZ, gain, samplingfreqHz);


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
	int i;
	//printf("WORK\n");
	/* GENERATE SIGNALS ACCORDING MODE AND PARAMS */
	/* COMPLEX OUTPUTS*/
	if(strcmp(outputtype, "COMPLEXFLOAT")==0){
		if(strcmp(signaltype, "TONE_COMPLEX")==0){
			gen_toneCOMPLEX(out, tableA, TABLESZ, block_length, tonefreq, samplingfreqHz);
		}
		if(strcmp(signaltype, "TWO_TONE_COMPLEX")==0){
			gen_toneCOMPLEX(bufferA, tableA, TABLESZ, block_length, samplingfreqHz-13000.0, samplingfreqHz);
			gen_toneCOMPLEX2(bufferB, tableA, TABLESZ, block_length, samplingfreqHz-11000.0, samplingfreqHz);
			gen_toneCOMPLEX4(bufferC, tableA, TABLESZ, block_length, samplingfreqHz-9000.0, samplingfreqHz);
			gen_toneCOMPLEX3(bufferD, tableA, TABLESZ, block_length, 6750.0, samplingfreqHz);
			gen_toneCOMPLEX4(bufferE, tableA, TABLESZ, block_length, 13500.0, samplingfreqHz);
			//for(i=0; i<block_length; i++)*(out+i) = 1.0*bufferA[i]+2.0*bufferB[i]+3.0*bufferC[i]+4.0*bufferD[i]+5.0*bufferE[i];
			for(i=0; i<block_length; i++)*(out+i) = 1.0*bufferA[i]+1.0*bufferB[i]+1.0*bufferC[i]+1.0*bufferD[i]+1.0*bufferE[i];

/*			gen_NtonesCOMPLEX(bufferA, tableA, TABLESZ, block_length, samplingfreqHz, 0.0, 1000.0, 14000.0);
			gen_NtonesCOMPLEX(bufferB, tableA, TABLESZ, block_length, samplingfreqHz, 48000.0-14000.0, 1000.0, 48000.0);
			for(i=0; i<block_length; i++)*(out+i) = 5.0*bufferA[i]+5.0*bufferB[i];
*/
		}
		if(strcmp(signaltype, "LTESPECTRUM")==0){
			gen_spectrum(out, block_length);
		}
	}

	return block_length;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


