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

#include "UPLINK_SYNCHRO_interfaces.h"
#include "UPLINK_SYNCHRO_functions.h"
#include "UPLINK_SYNCHRO.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="UPLINK_SYNCHRO";
int block_length;
int Tslot=0;

//Module User Defined Parameters
const int FFTsize=FFTSIZE;

//Global Variables
// DMRS sequence
#define BUFFER_SZ	2048
_Complex float DMRS_SEQ[BUFFER_SZ];
_Complex float DMRS_TX_TIME[BUFFER_SZ];
_Complex float DMRS_RX_TIME[BUFFER_SZ];
int DMRS_length=0;
int M_RS_SC = (NUMRB-2)*RBCARRIERS;

// CORRELATION process
#define BUFFERSIZE	64*1024
_Complex float CorrResult[BUFFERSIZE];

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {
	int i;

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/

	/* Verify control parameters */

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      FFTsize=%d\n", FFTsize);
	printf("O--------------------------------------------------------------------------------------------O\n");

	// CALCULATE DMRS sequence 
	//M_RS_SC = 48;
	DMRS_length=genRSsignalargerThan3RB(1, 0, 10, M_RS_SC, DMRS_SEQ, 0);
	//DMRS_length=genRSsignalargerThan3RB(0, 1, 10, M_RS_SC, DMRS_SEQ, 0);
	//createDMRS(DMRS_SEQ);
	// Generate Transmitter DMRS_PUSCH time sequence
	genDMRStime_seq(FFTsize, DMRS_SEQ, M_RS_SC, DMRS_TX_TIME, TXMODE);
	// Generate Receiver DMRS_PUSCH time sequence
	genDMRStime_seq(FFTsize, DMRS_SEQ, M_RS_SC, DMRS_RX_TIME, RXMODE);

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
	input_t *input0 = in(inp,0);
	output_t *output0 = out(out,0);
	output_t *output1 = out(out,1);
	output_t *output2 = out(out,2);

	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples0=0, snd_samples1=0, snd_samples2=0;
	int i,k;
	int pMAX;
	float Dphase=0.0;
	
	// Check if data received
	if (rcv_samples == 0)return(0);

//	printf("%s IN Tslot=%d: rcv_samples=%d\n", mname, Tslot, rcv_samples);

	/* do DSP stuff here */
	stream_conv_CPLX(input0, rcv_samples, DMRS_RX_TIME, FFTsize, CorrResult);
	pMAX=detect_DMRS_in_subframe(CorrResult, rcv_samples);
	if(pMAX >= 0)printf("DMRS sequence found: pMAX=%d\n", pMAX);
	// DETECT CFO
	Dphase=checkPhaseOffset(&CorrResult[0]+pMAX);
	printf("DetectedPhase=%f\n", Dphase);
	// CORRECT CFO
	rotateCvector(input0, input0, rcv_samples, -Dphase);
	// WRITE INTO ALIGNED SUBFRAME BUFFER
	if(pMAX>=0)write_subframe_buffer(input0, pMAX, rcv_samples, FFTsize);
	// READ FROM ALIGNED SUBFRAME AND COPY TO OUTPUT
	snd_samples0=read_subframe_buffer(output0, FFTsize);

	// OUTPUT 1
	memcpy(output1, CorrResult, sizeof(_Complex float)*rcv_samples);
	snd_samples1=rcv_samples;

	// OUTPUT 2
	memcpy(output2, output0, sizeof(_Complex float)*rcv_samples);
	snd_samples2=rcv_samples;

	// TIMESLOT COUNT
	Tslot++;
	// Indicate the number of samples at output number N
	set_output_samples(1, snd_samples1);
	set_output_samples(2, snd_samples2);
//	printf("%s OUT: snd_samples0=%d\n", mname, snd_samples0);
	// Indicate the number of samples at output 0 with return value
	return snd_samples0;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


