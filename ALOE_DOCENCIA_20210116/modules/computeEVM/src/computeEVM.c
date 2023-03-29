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

#include "computeEVM_interfaces.h"
#include "computeEVM_functions.h"
#include "computeEVM.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="computeEVM";
int block_length=896;

//Module User Defined Parameters

//Global Variables
#define MAXBUFFER	4096
#define NOFFRAMES	25
_Complex float bufferTX[MAXBUFFER][NOFFRAMES];
int TXflag[NOFFRAMES], RXflag[NOFFRAMES];
int TXidxW=0, TXidxR=0, RXidxW=0, RXidxR=0;
int TXbuffsz=0, RXbuffsz=0;
_Complex float bufferRX[MAXBUFFER][NOFFRAMES];

_Complex float bufferAUX0[MAXBUFFER];
_Complex float bufferAUX1[MAXBUFFER];

float EVM=0.0;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	int i;

	/* Get control parameters*/
	param_get_int("block_length", &block_length);		//Initialized by hand or config file

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);

	printf("O      block_length=%d\n", block_length);
	printf("O--------------------------------------------------------------------------------------------O\n");

	// INIT BUFFER FLAGS
	for(i=0; i<NOFFRAMES; i++){
		TXflag[i]=0;
		RXflag[i]=0;
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
	
	int i, k;
	int rcv_samplesTX = get_input_samples(0); /** number of samples at itf 0 buffer */
	int rcv_samplesRX = get_input_samples(1); /** number of samples at itf 1 buffer */
	int snd_samples0=0;

	input_t *TXinput = inp;
	input_t *RXinput = in(inp,1);
	output_t *TXoutput = out;
	output_t *RXoutput = out(out,1);


	// TX INPUT TO TXBUFFER
	if (rcv_samplesTX > 0){
		// BYPASS TX
		bypass_COMPLEX(TXinput, rcv_samplesTX, TXoutput);
		// SAVE DATA INTO FRAME
		if(TXflag[TXidxW]==0){
			// COPY RECEIVED DATA INTO FRAME
			normA(TXinput, rcv_samplesTX);
			//memcpy(&bufferTX[0][TXidxW], TXinput, sizeof(_Complex float)*rcv_samplesTX);
			for(i=0; i<rcv_samplesTX; i++)bufferTX[i][TXidxW]=TXinput[i];
			TXflag[TXidxW]=rcv_samplesTX;					// FRAME FULL: A VALUE > 0
			TXidxW++;							// MOVE TO NEXT FRAME
			if(TXidxW==NOFFRAMES)TXidxW=0;		// UPDATE TX WRITE INDEX
		}else{
			printf("%s: ERROR!!! NO TX BUFFER AVAILABLE\n");
		}
	}

	// RX INPUT TO RX BUFFER
	if (rcv_samplesRX > 0){
		// BYPASS TX
		bypass_COMPLEX(RXinput, rcv_samplesRX, RXoutput);
		// SAVE DATA INTO FRAME
		if(RXflag[RXidxW]==0){
			// COPY RECEIVED DATA INTO FRAME
			normB(RXinput, rcv_samplesRX);
			//memcpy(&bufferRX[0][RXidxW], RXinput, sizeof(_Complex float)*rcv_samplesRX);
			for(i=0; i<rcv_samplesRX; i++)bufferRX[i][RXidxW]=RXinput[i];
			RXflag[RXidxW]=rcv_samplesRX;					// FRAME FULL
			RXidxW++;							// MOVE TO NEXT FRAME
			if(RXidxW==NOFFRAMES)RXidxW=0;		// UPDATE RX WRITE index
		}else{
			printf("%s: ERROR!!! NO RX BUFFER AVAILABLE\n");
		}
	}

	// COMPUTE EVM
	if((TXflag[TXidxR]>0) && (RXflag[RXidxR]>0)){
		// COMPUTE EVM
		for(i=0; i<block_length; i++)bufferAUX0[i]=bufferRX[i][RXidxR];
		for(i=0; i<block_length; i++)bufferAUX1[i]=bufferTX[i][TXidxR];
		EVM=computeEVM_3GGP_LTE128(&bufferAUX0[0], &bufferAUX1[0], block_length);
		// UPDATE BUFFERs FLAGS
		TXflag[TXidxR]=0;		// FREE TX USED BUFFER
		RXflag[RXidxR]=0;
		TXidxR++;
		if(TXidxR==NOFFRAMES)TXidxR=0;
		RXidxR++;
		if(RXidxR==NOFFRAMES)RXidxR=0;
	}
	

	//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);

	// Indicate the number of samples at output number N
	set_output_samples(1, rcv_samplesRX);
	//printf("%s IN: snd_samples=%d\n", mname, snd_samples);
	// Indicate the number of samples at output 0 with return value
	return rcv_samplesTX;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


