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
#include <time.h>
#include <string.h>
#include <math.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "UPLINK_MAPPING_interfaces.h"
#include "UPLINK_MAPPING_functions.h"
#include "UPLINK_MAPPING.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="UPLINK_MAPPING";
unsigned long int Tslot=0;


//Module User Defined Parameters
extern MODparams_t oParam;		// Initialize module params struct
extern MODvars_t oVars;

//Global Variables
// ZC sequence
#define BUFFER_SZ	2048
_Complex float DMRS_SEQ0[BUFFER_SZ];
_Complex float DMRS_SEQ1[BUFFER_SZ];
int DMRS_length=0;
// SUBFRAME-SLOT STRUCT

//_Complex float SLOT[14][NUMRB*NUMCARRIERS];

int M_RS_SC = (NUMRB-2)*NUMCARRIERS;



/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
/*	param_get_int("opMODE", &oParam.opMODE);		//Initialized by hand or config file
	param_get_float("cellBwMHz", &oParam.cellBwMHz);
	param_get_int("Csrs", &oParam.Csrs);
	param_get_int("SubConfsrs", &oParam.SubConfsrs);
	param_get_int("Bsrs", &oParam.Bsrs);
	param_get_int("nRRC", &oParam.nRRC);
	param_get_int("Isrs", &oParam.Isrs);
	param_get_int("kTC", &oParam.kTC);
*/
	param_get_int("dataPeriod", &oParam.dataPeriod);
	param_get_int("delayTOask", &oParam.delayTOask);
	param_get_int("NofDataSampl2ask", &oParam.NofDataSampl2ask);

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      opMODE=%d, cellBwMHz=%f, Csrs=%d\n", oParam.opMODE, oParam.cellBwMHz, oParam.Csrs);
	printf("O      dataPeriod=%d, delayTOask=%d, NofDataSampl2ask=%d\n", 
					oParam.dataPeriod, oParam.delayTOask, oParam.NofDataSampl2ask);
	printf("O--------------------------------------------------------------------------------------------O\n");

	// CALCULATE DMRS sequence 
	//M_RS_SC = 48;
	DMRS_length=genRSsignalargerThan3RB(1, 0, 10, M_RS_SC, DMRS_SEQ0, 0);
	DMRS_length=genRSsignalargerThan3RB(0, 1, 10, M_RS_SC, DMRS_SEQ1, 0);

	//DMRS_length=genRSsignalargerThan3RB(0, 1, 10, M_RS_SC, DMRS_SEQ0, 0);
	//DMRS_length=genRSsignalargerThan3RB(1, 1, 10, M_RS_SC, DMRS_SEQ1, 0);

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
	input_t *input0;
	input0=in(inp,0);
	output_t *output0;
	output0=out(out,0);
	unsigned int *output1;
	output1=out(out,1);

	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples0=0;
	int snd_samples1=1;
	int i,k;
	time_t t;
	
//	printf("MAPPING: rcv_samples=%d\n", rcv_samples);

	// NORMAL MAPPING WITHOUT ADDING RS
	if(oParam.opMODE == NORMAL_NORS /*&& Tslot > 0*/){
		// CONTROL OF SUBFRAME
		*output1=askFORdata(Tslot, &oParam);

		// GENERATE SUBFRAME
		snd_samples0=generate_uplinkLTE_subframe2(Tslot, (int)(*output1), &oVars, &oParam, 
								input0, rcv_samples, 
								DMRS_SEQ0, DMRS_SEQ1, DMRS_length,
								output0);

	}
	// Indicate the number of samples at output number N
	set_output_samples(1, snd_samples1);
//	printf("MAPPING: rcv_samples=%d\n", snd_samples0);
	Tslot++;
	// Indicate the number of samples at output 0 with return value
	return snd_samples0;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


