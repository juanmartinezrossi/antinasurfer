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

#include "LTEturboCOD2_interfaces.h"
#include "LTEturboCOD2_functions.h"
#include "LTEturboCOD2.h"

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
//int NofRB = 6; 		// Number of RBs: 6 (FFT 128)	
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
char bitdata[INPUT_MAX_DATA],rmdata[INPUT_MAX_DATA];
float testdata[INPUT_MAX_DATA], demapped [INPUT_MAX_DATA];

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
	param_get_int("NumRBs", &(sframeBITS.NumRBs));

	/* CALCULATE INPUT DATA BLOCK LENGTH*/
	if(modeRS==WITHOUT_RS)calculateCodeBlockLengths( &sframeBITS, NofFFTs, sframeBITS.NumRBs, ModOrder, UpDownLink);
	//if(modeRS==WITHOUT_RS && UpDownLink==UPLINK)calculateCodeBlockLengthsUPLINK( &sframeBITS, NofFFTs, sframeBITS.NumRBs, ModOrder);

	if(modeRS==WITH_RS)calculateCodeBlockLengthswithRS( &sframeBITS, NofFFTs, sframeBITS.NumRBs, ModOrder);

	/* INITIALIZE TURBO CODER-DECODER INTERNALS*/
/*	initDataSource(TC_CHAR_RANDOM, codeblocklength);	//TC_BIT_RANDOM, TC_CHAR_RANDOM
	initLTETurboCoder(codeblocklength);
	initLTETurboDeCoder(codeblocklength, NumIterations, DT, HALT_METHOD_MIN_);	
	initRateMatching(codeblocklength, subframe_bits);
	initUnRateMatching(codeblocklength, subframe_bits);
*/
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
	printf("\033[1;37;42m\tO      CodeRate=%3.2f,  NumRBs=%d, DECODIterations=%d						\t\033[0m\n", 
									sframeBITS.CodeRate, sframeBITS.NumRBs, NumIterations);
	printf("\033[1;37;42m\tO      MQAMOrder=%d [1-BPSK, 2-4QAM, 4-16QAM, 6-64QAM, 8-256QAM, 10-1024QAM] 		\t\033[0m\n", ModOrder);

	if(opmode==ENCODER_ONLY || opmode==TEST_ALL){
		if(UpDownLink==DOWNLINK)printf("\033[1;37;42m\tO      Expected nof input bytes: %d (14FFTs), %d (13FFTs), %d (7FFTs)              	\t\033[0m\n", 
															sframeBITS.bits2rcvSF[0]/8, sframeBITS.bits2rcvSF[1]/8, sframeBITS.bits2rcvSF[2]/8);
//		if(UpDownLink==UPLINK)printf("\033[1;37;42m\tO      Expected nof input bytes: %d (12FFTs), %d (12FFTs), %d (12FFTs)              	\t\033[0m\n", 
//															sframeBITS.bits2rcvSF[0]/8, sframeBITS.bits2rcvSF[1]/8, sframeBITS.bits2rcvSF[2]/8);
		if(UpDownLink==UPLINK)printf("\033[1;37;42m\tO      Expected input bytes: %d (12FFTs), NofCodeBlocks=%d, CodeBlocksz=%d, ResidualCBsz=%d         \t\033[0m\n", 
															sframeBITS.Totalbits2rcv/8, sframeBITS.NumCBlocks, sframeBITS.codeblocklengthSF[0]/8, sframeBITS.ResidualCBsz/8);
	}
	if(opmode==DECODER_ONLY){
		if(UpDownLink==DOWNLINK)printf("\033[1;37;42m\tO      Expected nof input floats: %d (14FFTs), %d (13FFTs), %d(7FFTs)            	\t\033[0m\n", 
							sframeBITS.MAXsubframeBits[0], sframeBITS.MAXsubframeBits[1], sframeBITS.MAXsubframeBits[2]);
		if(UpDownLink==UPLINK)printf("\033[1;37;42m\tO      Expected nof input floats: %d (12FFTs), %d (12FFTs), %d(12FFTs)          	\t\033[0m\n", 
							sframeBITS.MAXsubframeBits[0], sframeBITS.MAXsubframeBits[1], sframeBITS.MAXsubframeBits[2]);

	}
	printf("\033[1;37;42m\tO--------------------------------------------------------------------------------------------O\t\033[0m\n");


	printf("codeblocklength=%d\n", codeblocklength);

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
	int dec_rcv_samples, encod_rcv_samples, rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples=0;
	int out_turbo_samples;
	int error=0;
	float gain=1.0, power_noise=0.0, power_signal=0.0, SNR=0.75;
	int RsfIdx=-1;
	int Rcodeblocklength=0;
	int Rresidualbits=0;
	static int Tslot=0;

 	input_t *in0 = inp;
	output_t *out0 = out;
	float *pfloat;
	int numOutbitsEncoder;
	int inputCount=0;
	char a;


	int CODEDbits; //=sframeBITS.codedBitsSZ[0];
	int INPUTcodeblocklength; //=sframeBITS.codeblocklengthSF[0];
		
	//A=CODEDbits;
	// TEST ALL
	if(opmode == TEST_ALL){
		turboTEST(sframeBITS, datalength);
	}

	// ENCODER
	if(opmode == ENCODER_ONLY){
		if(rcv_samples==0)return(0);
#ifdef DEBUGG
		printf("ENCODER Received chars\n");
		for(i=0; i<rcv_samples; i++){
			printf("%c", *(in0+i));
//			if((i+1)%128==0)printf("\n");
		}
		printf("\n");
#endif

		// CONVERT RECEIVED CHARS (8bits) TO BITS (1 bit)
		rcv_samples=CHARS2BITS(in0, bitdata, rcv_samples);

/*		printf("ENCODER ALL recv bits\n");
		for(i=0; i<rcv_samples; i++){
				printf("%d", (int)((*(bitdata+i)&0xFF)));
				if((i+1)%128==0)printf("\n");
				//*(out0+i)=*(edata+i);
		}
		printf("\n");
*/

		
		// CHECK THAT NOF DATA RECEIVED MACHT A VALID LENGTH

		checkENCOD_UPLINK_inlength(&sframeBITS, rcv_samples);

		// PROCESS CODEBLOCKS
		snd_samples=0;
		for(i=0;i<sframeBITS.NumCBlocks; i++){
			// INITIALIZE TURBO CODER INTERNALS
			INPUTcodeblocklength=sframeBITS.codeblocklengthSF[0];
			CODEDbits=sframeBITS.codedBitsSZ[0];

			//ENCODER
			snd_samples+=encoder(bitdata+inputCount, INPUTcodeblocklength, CODEDbits, out0+snd_samples);
			inputCount += INPUTcodeblocklength;
//			printf("snd_samples=%d, subframeCODEDbits=%d\n", snd_samples, CODEDbits);
		}

		// RESIDUAL
		if(sframeBITS.ResidualCBsz > 0){
			INPUTcodeblocklength=sframeBITS.ResidualCBsz;
			CODEDbits=sframeBITS.ResidualCodedSZ;

			//ENCODER
			snd_samples+=encoder(bitdata+inputCount, INPUTcodeblocklength, CODEDbits, out0+snd_samples);
//			printf("snd_samples=%d, subframeCODEDbits=%d\n", snd_samples, CODEDbits);			
		}
#ifdef DEBUGG
		printf("%s: snd_samples=%d, subframeCODEDbits=%d\n", mname, snd_samples, CODEDbits+(sframeBITS.NumCBlocks)*sframeBITS.codedBitsSZ[0]);	
		printf("------------------------------------------\n");	
#endif

	}


	// DECODER
	if(opmode == DECODER_ONLY){
		// DO NOTHING IF NO DATA
		if(rcv_samples==0)return(0);


		//dec_rcv_samples=rcv_samples;
		rcv_samples=rcv_samples/datain_sz;
#ifdef DEBUGG
		printf("BBBB %s: Tslot=%d, rcv_samples=%d, datain_sz=%d\n", mname, Tslot, rcv_samples, datain_sz);
#endif
		// GET INPUT DATA AS CHARS
		pfloat=demapped;
		if(strcmp(datatypeIN, CHAR)==0)bits2floats(in0, demapped, rcv_samples);	
		// GET INPUT DATA AS FLOATS
		if(strcmp(datatypeIN, FLOAT)==0)pfloat=(float *)in0;	

		memcpy(testdata, pfloat, sizeof(float)*rcv_samples);

/*		for(i=0; i<rcv_samples; i++){
			if(*(testdata+i)>0.0)printf("1");
			if(*(testdata+i)<=0.0)printf("0");
			if(i%128==0)printf("\n");
		}
		printf("\n");
*/

		// CHECK IF rcv_samples MATCH THE EXPECTED VALUE

		snd_samples=0;
		// PROCESS CODEBLOCKS
		for(i=0;i<sframeBITS.NumCBlocks; i++){
			// INITIALIZE TURBO CODER INTERNALS
			INPUTcodeblocklength=sframeBITS.codeblocklengthSF[0];
			CODEDbits=sframeBITS.codedBitsSZ[0];			

			// DECODER
			snd_samples+=decoder(pfloat+inputCount, CODEDbits, datatypeIN, datain_sz, &sframeBITS, rmdata+snd_samples, INPUTcodeblocklength);
			inputCount += CODEDbits;
		}


		// RESIDUAL
		if(sframeBITS.ResidualCBsz > 0){
			INPUTcodeblocklength=sframeBITS.ResidualCBsz;
			CODEDbits=sframeBITS.ResidualCodedSZ;
/*			initDataSource(TC_CHAR_RANDOM, INPUTcodeblocklength);	//TC_BIT_RANDOM, TC_CHAR_RANDOM
			initLTETurboDeCoder(INPUTcodeblocklength, NumIterations, DT, HALT_METHOD_MIN_);	
			initUnRateMatching(INPUTcodeblocklength, CODEDbits);
*/
			// DECODER
/*			printf("inputCount=%d\n", inputCount);
			printf("4    INPUT DECODER MAIN PROGRAM\n");
			for(i=0; i<512; i++){
				if(*(testdata+i)>0.0)printf("1");
				if(*(testdata+i)<=0.0)printf("0");
				if(i%64==0)printf("\n");
			}
			printf("\n");
*/

			snd_samples+=decoder(pfloat+inputCount, CODEDbits, datatypeIN, datain_sz, &sframeBITS, rmdata+snd_samples, INPUTcodeblocklength);
			
		}
		//printf("A1 Decod snd_samples(BYTES)=%d\n", snd_samples);
/*		printf("DECODER ALL sent bits\n");
		for(i=0; i<snd_samples; i++){
				printf("%d", (int)((*(out0+i)&0xFF)));
				if((i+1)%128==0)printf("\n");
				//*(out0+i)=*(edata+i);
		}
		printf("\n");
*/
		// CONVERT BITS TO CHARS AND SEND DATA TO OUTPUT BUFFER
		snd_samples=BITS2CHARS(rmdata, out0, snd_samples);
#ifdef DEBUGG
		printf("DECODER sent chars(BYTES)=%d\n", snd_samples);
		for(i=0; i<snd_samples; i++){
			printf("%c", *(out0+i));
		}
		printf("\n");
#endif		
	}

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


