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

#include "CRC_interfaces.h"
#include "CRC_functions.h"
#include "CRC.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="CRC";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;

//Module User Defined Parameters
#define ADD__CRC	0
#define CHECKCRC	1
int mode=ADD__CRC; //0: ADD CRC; 1: CHECK_CRC

//Global Variables
char bufferA[2048];
char bufferB[2048];
int numdata=11;
//float bufferB[2048];
int crc_length = 24;
unsigned int crc_poly = LTE_CRC24A; //0x1864CFB;
unsigned int crc_word;
crc_t crc_p;

int numblocksrcv=0;
int numblocksNOK=0;
float BLER=0.0;
int Tslot=0;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {
	int i;

	strcpy(mname, GetObjectName());
	printf("INITIALIZE oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("mode", &mode);
	param_get_int("crc_length", &crc_length);
	param_get_int("crc_poly", &crc_poly);

	/* Verify control parameters */

	/* Print Module Init Parameters */
	
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      mode=%d [0: ADD CRC; 1: CHECK CRC]\n", mode);
	printf("O      crc_length=%d, crc_poly=0x%X\n", crc_length, crc_poly);
	printf("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */
	//Initialize CRC params and tables
	if(crc_init(&crc_p, crc_poly, crc_length))return(-1);
	if(crc_set_init( &crc_p, 0x00000000))return(-1);
	if(crc_set_xor( &crc_p, 0x00000000))return(-1);

	for (i=0;i<256;i++) {
		bufferA[i] = (char)(i&0xFF); //(char)(rand()&0xFF);
	}
//	printf("initialize(): crc_p->data0=%p\n", (void *)&(crc_p.data0));

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
	int snd_samples = 0;
	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int i,k;
	unsigned int crc=77;
	char a, b, c;
	int crcLengthBytes=(crc_p.order)/8;
	_Complex float *aux; 

	
	// Check if data received
	if (rcv_samples == 0){
//		printf("%s: No data received\n", mname);
		return(0);
	}//else printf("Tslot=%d: %s: rcv_samples=%d\n", Tslot, mname, rcv_samples);
	// Check if data exceed maximum expected data

	//ADD CRC
	if(mode==ADD__CRC){
		add_CRC(inp, rcv_samples, &crc_p);
		snd_samples=rcv_samples+crcLengthBytes;
		memcpy(out, inp, sizeof(char)*snd_samples);
/*		printf("CRC: Tslot=%d, p_inp=%p, rcv_samples=%d\n", Tslot, inp, rcv_samples);
		printf("INPUTCRC\n");
		aux=inp;
		printf("BER=%f\n", __real__ *aux);
		for(i=0; i<32; i++){
			//printf("%c", (char) *(inp+i));
			printf("%x", ((int)*(inp+i))&0xFF);
		}
		printf("\n");
*/
	}
	//CHECK CRC
	if(mode==CHECKCRC){
		numblocksrcv++;
		crc=check_CRC(inp, rcv_samples, &crc_p);
		if(crc!=0)numblocksNOK++;
		snd_samples=rcv_samples-crcLengthBytes;
		memcpy(out, inp, sizeof(char)*snd_samples);
		if(Tslot%10==0){
			BLER=((float)numblocksNOK)/((float)numblocksrcv);
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			printf("!!! %s: BLER=%1.5f\n", mname, BLER);
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		}
	}
//	printf("Tslot=%d || %s: rcv_samples=%d, snd_samples=%d, crc=%u\n", Tslot, mname, rcv_samples, snd_samples, crc);
	// Indicate the number of samples at autput 0 with return value
	Tslot++;
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


