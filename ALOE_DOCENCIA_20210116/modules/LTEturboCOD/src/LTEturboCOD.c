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

#include "typetools.h"
#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "LTEturboCOD_interfaces.h"
#include "LTEturboCOD_functions.h"
#include "LTEturboCOD.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="LTEturboCOD";
char datatypeIN[STR_LEN]=IN_TYPE;
int datain_sz=sizeof(char);
#define ENCODER_ONLY			1
#define DECODER_ONLY			2
#define TEST_ALL					3
int opmode=TEST_ALL; 
#define WITHOUT_RS				0		// Do not consider including RS
#define WITH_RS						1		// Consider including RS
int modeRS=WITHOUT_RS;

int UpDownLink=DOWNLINK; 			

// Module General variables
int ModOrder = 4; // NUmber of bits of MQAM symbol
int NofRB = 15; 		// Number of RBs: 6 (FFT 128)	
int NofFFTs = 14;	// Number of FFTs in a subframe

sframeBits_t sframeBITS;

int subframeType=0;				// 0: 14 FFTs, 1: 13 FFTs, 2: 7 FFTs
int MAXbitsSubframes[3];	// Holds the MAX number of bits of different subframes
int rcvbitsSubframe[3]={2000, 1800, 1000};	//Holds de expected bits to be coded.
int codeblocklength=0;		// Number of bits to the input of turbocoder. Not all values possible.
int residualbits;		// Number of bits to add to adjust received bits with codeblocklength for each subframe
float CodeRate=0.0;

int NumIterations=1;

int datalength=360;

// Global Buffers
char odata[INPUT_MAX_DATA], bitdata[INPUT_MAX_DATA], testdata[INPUT_MAX_DATA], edata[INPUT_MAX_DATA], rdata[INPUT_MAX_DATA], rmdata[INPUT_MAX_DATA];
float demapped [INPUT_MAX_DATA], unrmdata[INPUT_MAX_DATA], noise[INPUT_MAX_DATA];

int subframe_bits=0; 			// Number of bits for a subframe with 14 OFDM symbols of 72 carriers and QAM modulation.

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {
	int i;

//	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* GET CONTROL PARAMETERS*/
	param_get_int("opmode", &opmode);
	param_get_float("CodeRate", &(sframeBITS.CodeRate));
	// CHOOSE INPUT DATA TYPE SIZE FOR DECODER
	if(opmode==DECODER_ONLY)param_get_string("datatypeIN", &datatypeIN);
	if(strcmp(datatypeIN, CHAR)==0)datain_sz=sizeof(char);
	if(strcmp(datatypeIN, FLOAT)==0)datain_sz=sizeof(float);
	// CAPTURE PARAMS DECODER
	param_get_int("DECODIterations", &NumIterations);
//	printf("NumIterations=%d\n", NumIterations);
	param_get_int("datalength", &datalength);
	param_get_int("modeRS", &modeRS);

	param_get_int("UpDownLink", &UpDownLink);
	param_get_int("MQAMOrder", &ModOrder);

	/* CALCULATE INPUT DATA BLOCK LENGTH*/
	if(modeRS==WITHOUT_RS)calculateCodeBlockLengths( &sframeBITS, NofFFTs, NofRB, ModOrder, UpDownLink);
	//if(modeRS==WITHOUT_RS && UpDownLink==UPLINK)calculateCodeBlockLengthsUPLINK( &sframeBITS, NofFFTs, NofRB, ModOrder);

	if(modeRS==WITH_RS)calculateCodeBlockLengthswithRS( &sframeBITS, NofFFTs, NofRB, ModOrder);

	/* INITIALIZE TURBO CODER-DECODER INTERNALS*/
	initDataSource(TC_CHAR_RANDOM, codeblocklength);	//TC_BIT_RANDOM, TC_CHAR_RANDOM
	initLTETurboCoder(codeblocklength);
	initLTETurboDeCoder(codeblocklength, NumIterations, DT, HALT_METHOD_MIN_);	
	initRateMatching(codeblocklength, subframe_bits);
	initUnRateMatching(codeblocklength, subframe_bits);

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("\033[1;37;42m\tO--------------------------------------------------------------------------------------------O\t\033[0m\n");
	if(opmode==ENCODER_ONLY)printf("\033[1;37;42m\tO    SPECIFIC PARAMETERS SETUP: %s                                   	        \t\033[0m\n", mname);
	if(opmode==DECODER_ONLY)printf("\033[1;37;42m\tO    SPECIFIC PARAMETERS SETUP: %s                                   	      \t\033[0m\n", mname);
	printf("\033[1;37;42m\tO      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s                    \t\033[0m\n", 
		       NOF_INPUT_ITF, datatypeIN, NOF_OUTPUT_ITF, OUT_TYPE);

		printf("\033[1;37;42m\tO      opmode=%d [1=ENCODER_ONLY; 2=DECODER_ONLY, 3=TEST_ALL]                                 \t\033[0m\n", 
									opmode);
		printf("\033[1;37;42m\tO      modeRS=%d [0-Without RS, 1-With RS], UpDownLink=%d [0-DOWNLINK, 1-UPLINK]  	\t\033[0m\n", 
									modeRS, UpDownLink);
		printf("\033[1;37;42m\tO      CodeRate=%3.2f,  DECODIterations=%d						\t\033[0m\n", 
									sframeBITS.CodeRate, NumIterations);
		printf("\033[1;37;42m\tO      MQAMOrder=%d [1-BPSK, 2-4QAM, 4-16QAM, 6-64QAM, 8-256QAM, 10-1024QAM] 		\t\033[0m\n", ModOrder);

	if(opmode==ENCODER_ONLY || opmode==TEST_ALL){
		if(UpDownLink==DOWNLINK)printf("\033[1;37;42m\tO      Expected nof input bytes: %d (14FFTs), %d (13FFTs), %d (7FFTs)              	\t\033[0m\n", 
															sframeBITS.bits2rcvSF[0]/8, sframeBITS.bits2rcvSF[1]/8, sframeBITS.bits2rcvSF[2]/8);
		if(UpDownLink==UPLINK)printf("\033[1;37;42m\tO      Expected nof input bytes: %d (12FFTs), %d (12FFTs), %d (12FFTs)              	\t\033[0m\n", 
															sframeBITS.bits2rcvSF[0]/8, sframeBITS.bits2rcvSF[1]/8, sframeBITS.bits2rcvSF[2]/8);
	}
	if(opmode==DECODER_ONLY){
		if(UpDownLink==DOWNLINK)printf("\033[1;37;42m\tO      Expected nof input floats: %d (14FFTs), %d (13FFTs), %d(7FFTs)            	\t\033[0m\n", 
							sframeBITS.MAXsubframeBits[0], sframeBITS.MAXsubframeBits[1], sframeBITS.MAXsubframeBits[2]);
		if(UpDownLink==UPLINK)printf("\033[1;37;42m\tO      Expected nof input floats: %d (12FFTs), %d (12FFTs), %d(12FFTs)          	\t\033[0m\n", 
							sframeBITS.MAXsubframeBits[0], sframeBITS.MAXsubframeBits[1], sframeBITS.MAXsubframeBits[2]);

	}
	printf("\033[1;37;42m\tO--------------------------------------------------------------------------------------------O\t\033[0m\n");


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
	int dec_rcv_samples, encod_rcv_samples, rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples=0;
	int out_turbo_samples;
	int error=0;
	static int first=0;
	float gain=1.0, power_noise=0.0, power_signal=0.0, SNR=0.75;
	int RsfIdx=-1;
	int Rcodeblocklength=0;
	int Rresidualbits=0;
	static int Tslot=0;

 	input_t *in0 = inp;
	output_t *out0 = out;
	float *pfloat;
		
	// TEST ALL
	if(opmode == TEST_ALL){
		turboTEST(sframeBITS, datalength);
	}

	// ENCODER
	if(opmode == ENCODER_ONLY){
//		if(Tslot%100==0)printf("Encod encod_rcv_samples=%d\n", encod_rcv_samples);
		encod_rcv_samples=rcv_samples;
//		printf("Encod encod_rcv_samples(BYTES)=%d\n", encod_rcv_samples);
		if(encod_rcv_samples==0)return(0);
		// CHECK THAT NOF DATA RECEIVED MACHT A VALID LENGTH
		sframeBITS.subframeType=checkENCOD_inlength(&sframeBITS, encod_rcv_samples);
		//if(UpDownLink=UPLINK)sframeBITS.subframeType=checkENCOD_UPLINK_inlength(&sframeBITS, encod_rcv_samples);

		// GET DATA
		for(i=0; i<encod_rcv_samples; i++)*(odata+i)=*(in0+i);
//		printarray("odata", odata, TCCHAR, 4);

		// CONVERT RECEIVED CHARS (8bits) TO BITS (1 bit)
		encod_rcv_samples=CHARS2BITS(odata, bitdata, encod_rcv_samples);

		// SET WORKING PARAMETERS
		subframe_bits=sframeBITS.MAXsubframeBits[sframeBITS.subframeType];
		codeblocklength=sframeBITS.codeblocklengthSF[sframeBITS.subframeType];
		residualbits=sframeBITS.residualbits[sframeBITS.subframeType];

		// RUN TURBOENCODER
		runLTETurboCoder(bitdata, edata, codeblocklength); 

		// RUN RATE_MATCHING
		error=runRateMatching(edata, rmdata, codeblocklength, subframe_bits);
		if(error == -1)return(-1);

		// SEND DATA TO OUTPUT BUFFER
		snd_samples=subframe_bits;
		for(i=0; i<subframe_bits; i++)*(out0+i)=*(rmdata+i);
//		if(Tslot%100==0)printf("Encod snd_samples=%d\n", snd_samples);
//		printf("Encod snd_samples(BITS)=%d\n", snd_samples);
	}


	// DECODER
	if(opmode == DECODER_ONLY){
		
		dec_rcv_samples=rcv_samples;
		dec_rcv_samples=dec_rcv_samples/datain_sz;
//		printf("Decod rcv_samples(FLOATS)=%d\n", dec_rcv_samples);

		// GET INPUT DATA AS CHARS
		pfloat=demapped;
		if(strcmp(datatypeIN, CHAR)==0)bits2floats(in0, demapped, dec_rcv_samples);
			
		// GET INPUT DATA AS FLOATS
		if(strcmp(datatypeIN, FLOAT)==0)pfloat=(float *)in0;
	
		// DO NOTHING IF NO DATA
		if(dec_rcv_samples==0)return(0);
//		if(Tslot%100==0)printf("Decod dec_rcv_samples=%d\n", dec_rcv_samples);

		// CHECK THAT NOF DATA RECEIVED MACHT A VALID LENGTH
		sframeBITS.subframeType=checkDECOD_inlength(&sframeBITS, dec_rcv_samples);

		// SET WORKING PARAMETERS
		Rcodeblocklength=sframeBITS.codeblocklengthSF[sframeBITS.subframeType];
		Rresidualbits=sframeBITS.residualbits[sframeBITS.subframeType];

		// UNRATE_MATCHING
		error=runUnRateMatching(pfloat, unrmdata, Rcodeblocklength, dec_rcv_samples);
		if(error == -1)return(-1);

		// TURBODECODER
		runLTETurboDeCoder(unrmdata, rmdata, Rcodeblocklength, 0);

		// COMPUTE BER (FOR TESTING)
		//OK computeBERchars(Rcodeblocklength-Rresidualbits,testdata, rmdata, 100);
	
		// CONVERT BITS TO CHARS AND SEND DATA TO OUTPUT BUFFER
		snd_samples=BITS2CHARS(rmdata, out0, Rcodeblocklength-Rresidualbits);
//		printf("Decod snd_samples(BYTES)=%d\n", snd_samples);
	}

	first=1;
	Tslot++;

	// Indicate the number of samples at output 0 with return value
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


