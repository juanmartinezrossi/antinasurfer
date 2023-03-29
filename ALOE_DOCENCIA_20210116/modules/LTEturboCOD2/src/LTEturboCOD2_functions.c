/* 
 * Copyright (c) 2012
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

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "source.h"
#include "turbocoder.h"
#include "turbodecoder.h"
#include "permute.h"
#include "ctrl_ratematching.h"

#include "LTEturboCOD2_interfaces.h"
#include "LTEturboCOD2_functions.h"

// LTE Defines
#define SUBFRAMES 	10		// Number of LTE SUBFRAMES
#define NUMFRAMES		1		// Number of simulated frames

// OPERATION defines
//#define MAXDATA		1024*64
/* TURBOCODE CONFIGURATION*/
//#define TURBOCODERONLY			0
//#define TURBO_RATE_MATCHING		1
//#define MAXITER 1




ctrl_turbocoder_t ctrlTurboCoder;
ctrl_turbodecoder_t ctrlTurboDecoder;
ctrl_source_t ctrlSource;
ctrl_ratematching_t ctrlRateMatching;
ctrl_ratematching_t ctrlUnRateMatching;

// TEST data
//const int numbits[SUBFRAMES] = {1872,2016,2016,2016,2016,1872,2016,2016,2016,2016};

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */

#define MAX_DATA	8192
#define FLOAT			"FLOAT"
#define CHAR			"CHAR"				/* Defines a char where all bits used*/
#define BITS			"BITS"				/* Defines a char where only LSB bit is used*/

int turboTEST(sframeBits_t sframeBITS, int length){
	int i,j,k;
	int nbits, errcnt=0, a, b, err;
	float BER=0.0;
	static int Tslot=0;
	int encod_rcv_samples, dec_rcv_samples, rcv_samples, subframe_bits, codeblocklength, residualbits, snd_samples, opmode, Rresidualbits, Rcodeblocklength, error=0;
	

	char odata[MAX_DATA], bitdata[MAX_DATA], testdata[MAX_DATA], edata[MAX_DATA], rdata[MAX_DATA], rmdata[MAX_DATA];
	float demapped [MAX_DATA], unrmdata[MAX_DATA], noise[MAX_DATA];
	char datatypeIN[32]=CHAR;
	int datain_sz=sizeof(char);
	float *pfloat;
	
	for(i=0; i<NUMFRAMES; i++){

		encod_rcv_samples=length;
		// GENERATE DATA
		runDataSource(odata, encod_rcv_samples);

		// ENCODER //////////////////////////////////////////////////////////////////
		if(encod_rcv_samples==0)return(0);

		// CHECK THAT NOF DATA RECEIVED MACHT A VALID LENGTH
//		sframeBITS.subframeType=checkENCOD_inlength(&sframeBITS, encod_rcv_samples);
//		if(sframeBITS.subframeType==-1)exit(0);

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
		// END ENCODER //////////////////////////////////////////////////////////////////


		rcv_samples=snd_samples;
		

		//DECODER //////////////////////////////////////////////////////////////////
		dec_rcv_samples=rcv_samples;

		// GET INPUT DATA AS CHARS
		pfloat=demapped;
		if(strcmp(datatypeIN, CHAR)==0)bits2floats(rmdata, demapped, dec_rcv_samples);
			
		// GET INPUT DATA AS FLOATS
		bits2floats(rmdata, demapped, dec_rcv_samples);		// Emulates the Symbol_Level processing chain
		if(strcmp(datatypeIN, FLOAT)==0)pfloat=(float *)demapped;
	
		// DO NOTHING IF NO DATA
		if(dec_rcv_samples==0)return(0);

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

		// CHARS RO BITS
		snd_samples=BITS2CHARS(rmdata, edata, Rcodeblocklength-Rresidualbits);

		// COMPUTE BER (FOR TESTING)
		computeBERchars(snd_samples,edata, odata, 1);
//		printarray("edata", edata, TCCHAR, snd_samples);
//		printarray("odata", odata, TCCHAR, snd_samples);
	
		// END DECODER //////////////////////////////////////////////////////////////////
	}
}



void printarray(char *title, void *datain, int datatype, int length){
	int i;
	char DatType[STRLEN];
	char *charIN;
	int *intIN;
	float *floatIN;

	if(datatype == TCBITS){
		strcpy(DatType, "BITS");
		charIN=datain;
	} 
	if(datatype == TCCHAR){
		strcpy(DatType, "CHAR");
		charIN=datain;
	}
	if(datatype == TCFLOAT){
		strcpy(DatType, "FLOAT");
		floatIN=datain;
	}
	printf("@      %s, Data Type = %s, Data length=%d\n", title, DatType, length);
	for(i=0; i<length; i++){
		if(datatype == TCBITS)printf("%d", charIN[i]&0x01);
		if(datatype == TCFLOAT)printf("%1.0f", floatIN[i]);
		if(datatype == TCCHAR)printf("%x", charIN[i]&0xFF);
	}
	printf("\n");
}


void bits2floats(char *in, float *out, int length){
	int k;

	for(k=0; k<length; k++){
				if(in[k])out[k]=1.0; 
				else out[k]=-1.0; 
	}
}
/*
	Returns the number of bits.
*/

int CHARS2BITS (char *input, char *output, int inputlength){
	int i, k; 
	char a;
	
	for(k=0; k<inputlength; k++){
		a=*(input+k);
		*(output+k*8+7)=a&0x01;
		a=a>>1;
		*(output+k*8+6)=a&0x01;
		a=a>>1;
		*(output+k*8+5)=a&0x01;
		a=a>>1;
		*(output+k*8+4)=a&0x01;
		a=a>>1;
		*(output+k*8+3)=a&0x01;
		a=a>>1;
		*(output+k*8+2)=a&0x01;
		a=a>>1;
		*(output+k*8+1)=a&0x01;
		a=a>>1;
		*(output+k*8)=a&0x01;
	}
	return(inputlength*8);	//Number of output bits
}

/*
	Returns the number of bits.
*/
int BITS2CHARS (char *input, char *output, int inputlength)
{
    int i, k;
		char a=0x00;
		inputlength=inputlength>>3;

  for (i=0; i<inputlength; i++){
			k=i*8;
			*(output+i) =    *(input+k)<<7 | *(input+k+1)<<6
									 | *(input+k+2)<<5 | *(input+k+3)<<4
									 | *(input+k+4)<<3 | *(input+k+5)<<2 
									 | *(input+k+6)<<1 | *(input+k+7);
//			printf("%x\n", (int)(*(output+i))&0xFF);
	}
	return(inputlength); //Number of output chars
}

void initDataSource(int sourcetype, int datalength){

	ctrlSource.typesource = sourcetype; //TC_ALLONES; //RANDOM;
	ctrlSource.datalength = datalength;

}

int runDataSource(char *data, int datalength){

	ctrlSource.datalength = datalength;
	source(&ctrlSource, data);
	return(ctrlSource.datalength);
}


float computeBERchars(int length, char *odata, char *rdata, int BERperiod){
	int i, j=0, k;
	int a, b, c;
	static int errcnt=0;
	static int TotalData=0;
	static int period=0;
	float BER = 0.0;

//	printf("Inside computeBER(): length=%d\n", length);

	period++;
	for (i=0; i<length; i++){
		a=(int)(odata[i]&0xFF);
		b=(int)(rdata[i]&0xFF);
//		printf("i=%d, a=%x, b=%x\n", i, a&0xFF, b&0xFF);
		c=a ^ b;
		if(c){
			for(k=0; k<8; k++){
				if(c&0x01 == 1){
						errcnt++;
				}
				c=c>>1;
			}
		}
		TotalData++;
		if(period == BERperiod){
			period = 0;
			BER = ((float)errcnt)/((float)TotalData);
			printf("Total Data = %d, num errors = %d, BER=%3.6f\r", 
											TotalData, errcnt, BER);
		}
	}
	return(BER);
}


float computeBERbits(int length, char *odata, char *rdata, int BERperiod){
	int i, j=0;
	int a, b;
	static int errcnt=0;
	static int TotalData=0;
	static int period=0;
	float BER = 0.0;

	period++;
	for (i=0; i<length; i++){
		a=(int)(odata[i]&0x01);
		b=(int)(rdata[i]&0x01);
		if(a ^ b){
			errcnt++;
		}
		TotalData++;
		if(period == BERperiod){
			period = 0;
			BER = ((float)errcnt)/((float)TotalData);
			printf("Total Data = %d, num errors = %d, BER=%3.6f\n", 
											TotalData, errcnt, BER);
		}
	}

	return(BER);
}


void initLTETurboCoder(int codeblocklength){

		ctrlTurboCoder.type = PER_LTE;
		ctrlTurboCoder.blocklen = codeblocklength;
}


void runLTETurboCoder(char *in, char *out, int length){

		ctrlTurboCoder.blocklen=length;
		TurboCoder(in, out, &ctrlTurboCoder);
}


void initLTETurboDeCoder(int codeblocklength, int MaxIterations, int TurboDT, int HaltMethod){
	static int iterations=0;
	static int num_decoding_ops=0;

	ctrlTurboDecoder.Long_CodeBlock = codeblocklength;
	ctrlTurboDecoder.Turbo_iteracions = MaxIterations;
	ctrlTurboDecoder.Turbo_Dt = 10;
	ctrlTurboDecoder.haltMethod = HALT_METHOD_MIN;
	//ctrlTurboDecoder.halt = PER_LTE;
	ctrlTurboDecoder.type = PER_LTE;
}

void runLTETurboDeCoder(float *in, char *out, int codeblocklength, int halt){
	static int iterations=0;
	static int num_decoding_ops=0;

	ctrlTurboDecoder.Long_CodeBlock = codeblocklength;
	iterations += TurboDecoder(in, out, &ctrlTurboDecoder);
	num_decoding_ops++;
//	printf("Averaged Number of Iterations = %3.3f\n", (float)iterations/(float)num_decoding_ops);
}


int getCodeBlock_length(int nbits){
	//return(getLTEcbsize((nbits-12)/3));
	return(getLTEcbsize(nbits-12));
}


// RATEMATCHING
void initRateMatching(int codeblocklength, int numoutbits){

	/* RATE MATCHING CONFIG. */
	ctrlRateMatching.mode = CHAR_RM;
	ctrlRateMatching.insize = codeblocklength*3+12;
	ctrlRateMatching.outsize = numoutbits;
	ctrlRateMatching.rvidx = 0;
}


int runRateMatching(char *in, char *out, int codeblocklength, int numoutbits){
	int err;

	ctrlRateMatching.insize = codeblocklength*3+12;
	ctrlRateMatching.outsize = numoutbits;

//	printf("RM: ctrlRateMatching.insize=%d, ctrlRateMatching.outsize=%d\n", ctrlRateMatching.insize, ctrlRateMatching.outsize);

	err = ratematching (in, out, &ctrlRateMatching);
	if (err <=0){
		printf("ERROR: Rate Matching bad configuration, exiting\n");
		return (-1);
	}
	return(err);
}



// UNRATEMATCHING
void initUnRateMatching(int codeblocklength, int numoutbits){

	/* UNRATE MATCHING CONFIG. */
	ctrlUnRateMatching.mode = FLOAT_UNRM;
	ctrlUnRateMatching.insize = numoutbits;
	ctrlUnRateMatching.outsize = codeblocklength*3+12;
	ctrlUnRateMatching.rvidx = 0;
}


int runUnRateMatching(float *in, float *out, int codeblocklength, int numoutbits){
	int err;

	ctrlUnRateMatching.insize = numoutbits; 
	ctrlUnRateMatching.outsize = codeblocklength*3+12;


	err = ratematching (in, out, &ctrlUnRateMatching);
//	printf("URM: insize=%d, outsize=%d, Nof OUT=%d\n", ctrlUnRateMatching.insize, ctrlUnRateMatching.outsize, err); 


	if (err <=0){
		printf("ERROR: Un Rate Matching bad configuration, exiting\n");
		//return (-1);
	}

	return(0);
}



float get_variance(float snr_db,float scale) {
	return sqrt(pow(10,-snr_db/10)*scale);
}

float aver_power(float *in, int length){
	int i;
	float aver_power=0.0;

	for(i=0; i<length; i++){
		aver_power += (*(in+i)*(*(in+i)));
	}
	aver_power=aver_power/(float)length;
	return(aver_power);
}

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
float rand_gauss (void) {
  float v1,v2,s;

  do {
    v1 = 2.0 * ((float) rand()/RAND_MAX) - 1;
    v2 = 2.0 * ((float) rand()/RAND_MAX) - 1;

    s = v1*v1 + v2*v2;
  } while ( s >= 1.0 );

  if (s == 0.0)
    return 0.0;
  else
    return (v1*sqrt(-2.0 * log(s) / s));
}

void gen_noise(float *x, float variance, int len) {
	int i;
	for (i=0;i<len;i++) {
		x[i] = rand_gauss();
		x[i] *= variance;
	}
}

// Calculate CodeBlockLength assuming all carrier available. No RSs incorporated.

void calculateCodeBlockLengths(sframeBits_t *sframeBITS, int NofFFTs, int NofRB, int ModOrder, int UpDownLink){
	int i;
	int aux;
	
//	sframeBITS->CodeRate=0.5;

	sframeBITS->subframeType=0;
	if(UpDownLink==DOWNLINK){
		sframeBITS->MAXsubframeBits[0]=NofFFTs*12*NofRB*ModOrder;			// Number of bits normal subframe (14 FFTs)
		sframeBITS->MAXsubframeBits[1]=(NofFFTs-1)*12*NofRB*ModOrder;		// Number of bits PSS subframe (13 FFTs)
		sframeBITS->MAXsubframeBits[2]=7*12*NofRB*ModOrder;					// Number of bits starting subframe (7 FFTs)


		for(i=0; i<3; i++){
			sframeBITS->bits2rcvSF[i]=(int)((float)sframeBITS->MAXsubframeBits[i]*sframeBITS->CodeRate);

			if(i==0)printf("   A calculateCodeBlockLengths(): \n\tMAXsubframeBits[%d]=%d, \n\tcodeblocklengthSF=%d, \n\tbits2rcvSF=%d, \n\tresidualbits=%d\n", 
											i, sframeBITS->MAXsubframeBits[i], 
											sframeBITS->codeblocklengthSF[i], 
											sframeBITS->bits2rcvSF[i], 
											sframeBITS->residualbits[i]);

			sframeBITS->codeblocklengthSF[i]=getCodeBlock_length(sframeBITS->bits2rcvSF[i]*3+12);
			sframeBITS->bits2rcvSF[i]=sframeBITS->codeblocklengthSF[i];
			sframeBITS->residualbits[i]=sframeBITS->codeblocklengthSF[i]-sframeBITS->bits2rcvSF[i];
			if(i==0)printf("   B calculateCodeBlockLengths(): \n\tMAXsubframeBits[%d]=%d, \n\tcodeblocklengthSF=%d, \n\tbits2rcvSF=%d, \n\tresidualbits=%d\n", 
											i, sframeBITS->MAXsubframeBits[i], 
											sframeBITS->codeblocklengthSF[i], 
											sframeBITS->bits2rcvSF[i], 
											sframeBITS->residualbits[i]);
		}
	}

	if(UpDownLink==UPLINK){

/*		sframeBITS->MAXsubframeBits[0]=(NofFFTs-2)*12*(NofRB)*ModOrder;			
		sframeBITS->MAXsubframeBits[1]=(NofFFTs-2)*12*(NofRB)*ModOrder;			
		sframeBITS->MAXsubframeBits[2]=(NofFFTs-2)*12*(NofRB)*ModOrder;	

		for(i=0; i<3; i++){
			sframeBITS->bits2rcvSF[i]=(int)((float)sframeBITS->MAXsubframeBits[i]*sframeBITS->CodeRate);

			if(i==0)printf("   AUP calculateCodeBlockLengths(): \n\tMAXsubframeBits[%d]=%d, \n\tcodeblocklengthSF=%d, \n\tbits2rcvSF=%d, \n\tresidualbits=%d\n", 
											i, sframeBITS->MAXsubframeBits[i], 
											sframeBITS->codeblocklengthSF[i], 
											sframeBITS->bits2rcvSF[i], 
											sframeBITS->residualbits[i]);

			sframeBITS->codeblocklengthSF[i]=getCodeBlock_length(sframeBITS->bits2rcvSF[i]*3+12);
			sframeBITS->bits2rcvSF[i]=sframeBITS->codeblocklengthSF[i];
			sframeBITS->residualbits[i]=sframeBITS->codeblocklengthSF[i]-sframeBITS->bits2rcvSF[i];
			if(i==0)printf("   BUP calculateCodeBlockLengths(): \n\tMAXsubframeBits[%d]=%d, \n\tcodeblocklengthSF=%d, \n\tbits2rcvSF=%d, \n\tresidualbits=%d\n", 
											i, sframeBITS->MAXsubframeBits[i], 
											sframeBITS->codeblocklengthSF[i], 
											sframeBITS->bits2rcvSF[i], 
											sframeBITS->residualbits[i]);
		}
*/

		sframeBITS->ResidualCBsz=0;
		sframeBITS->MAXsubframeBits[0]=(NofFFTs-2)*12*(NofRB)*ModOrder;		

		// Nof input bit Encoder
		sframeBITS->bits2rcvSF[0]=(int)((float)sframeBITS->MAXsubframeBits[0]*sframeBITS->CodeRate);

		// Check if input CodeBlock > 6144
		sframeBITS->NumCBlocks=sframeBITS->bits2rcvSF[0]/MAXCBSIZE;

		// Compute Code Block Segmentation
		if(sframeBITS->NumCBlocks>0){
			// Define the codeblock size at input from LTE table
			sframeBITS->codeblocklengthSF[0]=getCodeBlock_length(MAXCBSIZE);
			// Compute the nof of coded bits expected to output for each codeblock
			sframeBITS->codedBitsSZ[0]=MAXCBSIZE/sframeBITS->CodeRate;							//((sframeBITS->MAXsubframeBits[0])*MAXCBSIZE)/(sframeBITS->bits2rcvSF[0]);


			// Define the codeblock size for the remaining bits
			sframeBITS->ResidualCBsz=getCodeBlock_length(sframeBITS->bits2rcvSF[0]-(sframeBITS->codeblocklengthSF[0]*sframeBITS->NumCBlocks));
			// Define the nof of coded bits expected to output for ResidualCBsz
			sframeBITS->ResidualCodedSZ=sframeBITS->MAXsubframeBits[0]-sframeBITS->NumCBlocks*sframeBITS->codedBitsSZ[0];
		}

		if(sframeBITS->NumCBlocks==0){
			sframeBITS->codeblocklengthSF[0]=0;
			sframeBITS->codedBitsSZ[0]=0;
			sframeBITS->ResidualCBsz=getCodeBlock_length(sframeBITS->bits2rcvSF[0]);
			// Define the nof of coded bits expected to output for ResidualCBsz
			sframeBITS->ResidualCodedSZ=sframeBITS->MAXsubframeBits[0];

		}


		sframeBITS->Totalbits2rcv=sframeBITS->NumCBlocks*sframeBITS->codeblocklengthSF[0]+sframeBITS->ResidualCBsz;
		printf("MAXsubframeBits[0]=%d, bits2rcvSF[0]=%d, codeblocklength=%d, ResidualCBsz=%d\n", 
						sframeBITS->MAXsubframeBits[0], sframeBITS->bits2rcvSF[0], sframeBITS->codeblocklengthSF[0], sframeBITS->ResidualCBsz);
	}
}

/////////////////////////////////////////////////////////
// Calculate CodeBlockLength assuming all carrier available. No RSs incorporated.

/*void calculateCodeBlockLengthsUPLINK(sframeBits_t *sframeBITS, int NofFFTs, int NofRB, int ModOrder){
	int i;
	int aux;
	
//	sframeBITS->CodeRate=0.5;

	sframeBITS->subframeType=0;
	sframeBITS->MAXsubframeBits[0]=(NofFFTs-2)*12*(NofRB-2)*ModOrder;			// Number of bits normal subframe (14 FFTs)
	sframeBITS->MAXsubframeBits[1]=(NofFFTs-2)*12*(NofRB-2)*ModOrder;	// Number of bits PSS subframe (13 FFTs)
	sframeBITS->MAXsubframeBits[2]=(NofFFTs-2)*12*(NofRB-2)*ModOrder;						// Number of bits starting subframe (7 FFTs)
	
	for(i=0; i<3; i++){
		sframeBITS->bits2rcvSF[i]=(int)((float)sframeBITS->MAXsubframeBits[i]*sframeBITS->CodeRate);
		sframeBITS->codeblocklengthSF[i]=getCodeBlock_length(sframeBITS->bits2rcvSF[i]*3+12);
		sframeBITS->bits2rcvSF[i]=sframeBITS->codeblocklengthSF[i];
		sframeBITS->residualbits[i]=sframeBITS->codeblocklengthSF[i]-sframeBITS->bits2rcvSF[i];

	}
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////


// Calculate CodeBlockLength assuming all carrier available. No RSs incorporated.

void calculateCodeBlockLengthswithRS(sframeBits_t *sframeBITS, int NofFFTs, int NofRB, int ModOrder){
	int i;
	int aux;
	
//	sframeBITS->CodeRate=0.5;

	sframeBITS->subframeType=0;
	sframeBITS->MAXsubframeBits[0]=NofFFTs*12*NofRB*ModOrder-(16*NofRB*ModOrder);			// Number of bits normal subframe (14 FFTs)
	sframeBITS->MAXsubframeBits[1]=(NofFFTs-1)*12*NofRB*ModOrder-(16*NofRB*ModOrder);	// Number of bits PSS subframe (13 FFTs)
	sframeBITS->MAXsubframeBits[2]=7*12*NofRB*ModOrder-(16*NofRB*ModOrder/2);						// Number of bits starting subframe (7 FFTs)
	
	for(i=0; i<3; i++){
		sframeBITS->bits2rcvSF[i]=(int)((float)sframeBITS->MAXsubframeBits[i]*sframeBITS->CodeRate);
		sframeBITS->codeblocklengthSF[i]=getCodeBlock_length(sframeBITS->bits2rcvSF[i]*3+12);
		sframeBITS->bits2rcvSF[i]=sframeBITS->codeblocklengthSF[i];
		sframeBITS->residualbits[i]=sframeBITS->codeblocklengthSF[i]-sframeBITS->bits2rcvSF[i];
/*		printf("   calculateCodeBlockLengths(): \n\tMAXsubframeBits[%d]=%d, \n\tcodeblocklengthSF=%d, \n\tbits2rcvSF=%d, \n\tresidualbits=%d\n", 
										i, sframeBITS->MAXsubframeBits[i], 
										sframeBITS->codeblocklengthSF[i], 
										sframeBITS->bits2rcvSF[i], 
										sframeBITS->residualbits[i]);
*/
	}
}



/*
	Check Decoder Input Data Length Value
*/
int checkDECOD_inlength(sframeBits_t *sframeBITS, int inDATAlength){
		int i;
		int RsfIdx=-1;

//		for(i=0; i<3; i++)printf("sframeBITS->MAXsubframeBits=%d, inDATAlength=%d\n", sframeBITS->MAXsubframeBits[i], inDATAlength);
		for(i=0; i<3; i++)if((sframeBITS->MAXsubframeBits[i]) == inDATAlength)RsfIdx=i;

/*		printf("MAXsubframeBits[0]=%d, MAXsubframeBits[1]=%d, MAXsubframeBits[2]=%d\n", 
						sframeBITS->MAXsubframeBits[0], sframeBITS->MAXsubframeBits[1], sframeBITS->MAXsubframeBits[2]);
*/
		if(RsfIdx == -1){
			printf("\033[1;31;47m \tO   ###########################################################################     O\t\033[0m\n");
			printf("\033[1;31;47m \t    TurboDecoder: Data length received = %d does not match any of expected ones \t\033[0m\n", inDATAlength);
			printf("\033[1;31;47m \t    TurboDecoder: Please, check received values according CodeRate parameter	\t\033[0m\n", inDATAlength);
			printf("\033[1;31;47m \tO   ###########################################################################     O\t\033[0m\n");
			exit(0);
		}
		return(RsfIdx);
}

/*
	Check Decoder Input Data Length Value
*/
int checkENCOD_DOWNLINK_inlength(sframeBits_t *sframeBITS, int inDATAlength){
		int i;
		int RsfIdx=-1;

//		for(i=0; i<3; i++)printf("sframeBITS->bits2rcvSF[i]/8=%d, inDATAlength=%d\n", sframeBITS->bits2rcvSF[i]/8, inDATAlength);
		for(i=0; i<3; i++)if(sframeBITS->bits2rcvSF[i] == inDATAlength)RsfIdx=i;
		if(inDATAlength==sframeBITS->ResidualCBsz)RsfIdx=0;
		if(RsfIdx == -1){
			printf("\033[1;31;47m \tO   ###########################################################################     O\t\033[0m\n");
			printf("\033[1;31;47m \t    TurboEncoder: Data length received = %d does not match any of expected ones\t\033[0m\n", inDATAlength);
			printf("\033[1;31;47m \t    TurboEncoder: Please, check received values according CodeRate parameter	\t\033[0m\n", inDATAlength);
			printf("\033[1;31;47m \tO   ###########################################################################     O\t\033[0m\n");
			exit(0);
		}
		return(RsfIdx);
}


int checkENCOD_UPLINK_inlength(sframeBits_t *sframeBITS, int inDATAlength){
		int i;

		if(sframeBITS->Totalbits2rcv != inDATAlength){
			printf("\033[1;31;47m \tO   ###########################################################################     O\t\033[0m\n");
			printf("\033[1;31;47m \t    TurboEncoder: Data length received = %d does not match expected ones\t\033[0m\n", inDATAlength);
			printf("\033[1;31;47m \t    TurboEncoder: Please, check received values according CodeRate parameter	\t\033[0m\n", inDATAlength);
			printf("\033[1;31;47m \tO   ###########################################################################     O\t\033[0m\n");
			exit(0);
		}
		return(0);
}

//int encoder(char *in0, int rcv_samples, sframeBits_t *sframeBITS, char *out0){
int encoder(char *in0, int rcv_samples, int expectedOUTbits, char *out0){

		int i;		
		int error=0;
		int dec_rcv_samples, snd_samples;
		int subframe_bits=0, codeblocklength, residualbits;
		char edata[INPUT_MAX_DATA];

		static int Tslot=0;

		codeblocklength=rcv_samples;

		// INITIALIZE
#ifdef DEBUGG
		printf("NEW ENCODER CALL: INPUTcodeblocklength=%d, CODEDbits=%d\n", rcv_samples, expectedOUTbits);
#endif
		initDataSource(TC_CHAR_RANDOM, rcv_samples);	//TC_BIT_RANDOM, TC_CHAR_RANDOM
		initLTETurboCoder(rcv_samples);
		initRateMatching(rcv_samples, expectedOUTbits);


//if(Tslot<2){

/*		printf("2    INPUT ENCODER\n");
		for(i=0; i<512; i++){
			printf("%d", *(in0+i));
			if((i+1)%128==0)printf("\n");
		}
		printf("\n");
*/


		// RUN TURBOENCODER
		runLTETurboCoder(in0, edata, codeblocklength); 

/*		printf("2    OUTPUT ENCODER\n");
		for(i=0; i<512; i++){
				printf("%d", (int)((char)((*(edata+i))*0x01)));
				//*(out0+i)=*(edata+i);
				if(i%128==0)printf("\n");
		}
		printf("\n");
		subframe_bits=3*codeblocklength;
*/	

///////////////////////

		// RUN RATE_MATCHING
		subframe_bits=runRateMatching(edata, out0, codeblocklength, expectedOUTbits);
		if(subframe_bits == -1){
			printf("runRateMatching(): ERROR!!!");
			return(-1);
		}

/*		printf("2    OUTPUT RATEMATCHING\n");
		for(i=0; i<512; i++){
			printf("%d", (int)((char)((*(out0+i))*0x01)));
//			printf("%d", *(out0+i));
			if(i%128==0)printf("\n");
		}
		printf("\n");
*/
#ifdef DEBUGG
		printf("rcv_samples=%d, IN codeblocklength=%d, subframe_bits=%d, expectedOUTbits=%d\n", rcv_samples,codeblocklength, subframe_bits, expectedOUTbits);
#endif

///////////////////////
		// SEND DATA TO OUTPUT BUFFER
//}	
		Tslot++;
		return(subframe_bits);
}

/*
int encoder(char *in0, int rcv_samples, sframeBits_t *sframeBITS, char *out0){

		int i;		
		int error=0;
		int dec_rcv_samples, snd_samples;
		int subframe_bits, codeblocklength, residualbits;
		char bitdata[INPUT_MAX_DATA], edata[INPUT_MAX_DATA];

		
		// CHECK THAT NOF DATA RECEIVED MACHT A VALID LENGTH
		sframeBITS->subframeType=checkENCOD_inlength(sframeBITS, rcv_samples);

		// SET WORKING PARAMETERS
		subframe_bits=sframeBITS->MAXsubframeBits[sframeBITS->subframeType];
		codeblocklength=sframeBITS->codeblocklengthSF[sframeBITS->subframeType];
		residualbits=sframeBITS->residualbits[sframeBITS->subframeType];

		// RUN TURBOENCODER
		runLTETurboCoder(in0, edata, codeblocklength); 

		// RUN RATE_MATCHING
		error=runRateMatching(edata, out0, codeblocklength, subframe_bits);
		if(error == -1)return(-1);

		// SEND DATA TO OUTPUT BUFFER
		return(subframe_bits);
}
*/


extern int NumIterations;

int decoder(float *in0, int rcv_samples, char *datatypeIN, int datain_sz, sframeBits_t *sframeBITS, char *out0, int expectedOUTbits){

		int i;		
		int error=0;
		int dec_rcv_samples, snd_samples;
		int Rcodeblocklength=0;
		int Rresidualbits=0;
		float *pfloat;
static int Tslot=0;
		float demapped[INPUT_MAX_DATA], unrmdata[INPUT_MAX_DATA], noise[INPUT_MAX_DATA], rmdata[INPUT_MAX_DATA];

		dec_rcv_samples=rcv_samples;

		// INITIALIZE
		initDataSource(TC_CHAR_RANDOM, expectedOUTbits);	//TC_BIT_RANDOM, TC_CHAR_RANDOM
		initLTETurboDeCoder(expectedOUTbits, NumIterations, DT, HALT_METHOD_MIN_);	
		initUnRateMatching(expectedOUTbits, rcv_samples);


//if(Tslot<2){
		// SET WORKING PARAMETERS
		Rcodeblocklength=sframeBITS->codeblocklengthSF[sframeBITS->subframeType];
//		Rresidualbits=sframeBITS->residualbits[sframeBITS->subframeType];

		// UNRATE_MATCHING
#ifdef DEBUGG
		printf("NEW DECODER CALL: Rcodeblocklength=%d, dec_rcv_samples=%d, expectedOUTbits=%d\n", Rcodeblocklength, dec_rcv_samples, expectedOUTbits);
#endif
/*		printf("3    INPUT UNRATEMATCHING\n");
		for(i=0; i<512; i++){
			if(*(in0+i)>0.0)printf("1");
			if(*(in0+i)<=0.0)printf("0");
			if(i%64==0)printf("\n");
		}
		printf("\n");
*/
		snd_samples=runUnRateMatching(in0, unrmdata, expectedOUTbits, dec_rcv_samples);
		if(snd_samples == -1){
			printf("Decoder Error!!!");
			return(-1);
		}

/*		printf("3    INPUT DECODER\n");
		for(i=0; i<512; i++){
			//*(unrmdata+i)=*(in0+i);
			if(*(unrmdata+i)>0.0)printf("1");
			if(*(unrmdata+i)<=0.0)printf("0");

			//printf("%1.0f", *(unrmdata+i));
			if(i%64==0)printf("\n");
		}
		printf("\n");
*/
		// TURBODECODER
		runLTETurboDeCoder(unrmdata, out0, expectedOUTbits, 0);

/*		printf("3    OUTPUT DECODER\n");
		for(i=0; i<512; i++){
				printf("%d", (int)((*(out0+i)&0xFF)));
				if((i+1)%128==0)printf("\n");
				//*(out0+i)=*(edata+i);
		}
		printf("\n");
*/

		// COMPUTE BER (FOR TESTING)
		//OK computeBERchars(Rcodeblocklength-Rresidualbits,testdata, rmdata, 100);
	
		// CONVERT BITS TO CHARS AND SEND DATA TO OUTPUT BUFFER
//		snd_samples=BITS2CHARS(rmdata, out0, Rcodeblocklength-Rresidualbits);
//		printf("Decod snd_samples(BYTES)=%d\n", snd_samples);
		


		snd_samples=expectedOUTbits;
//}

		Tslot++;
		return(snd_samples);

}

/*int decoder(float *in0, int rcv_samples, char *datatypeIN, int datain_sz, sframeBits_t *sframeBITS, char *out0){

		int i;		
		int error=0;
		int dec_rcv_samples, snd_samples;
		int Rcodeblocklength=0;
		int Rresidualbits=0;
		float *pfloat;
		float demapped[INPUT_MAX_DATA], unrmdata[INPUT_MAX_DATA], noise[INPUT_MAX_DATA], rmdata[INPUT_MAX_DATA];

		dec_rcv_samples=rcv_samples;

		// CHECK THAT NOF DATA RECEIVED MACHT A VALID LENGTH
		sframeBITS->subframeType=checkDECOD_inlength(sframeBITS, dec_rcv_samples);

		// SET WORKING PARAMETERS
		Rcodeblocklength=sframeBITS->codeblocklengthSF[sframeBITS->subframeType];
		Rresidualbits=sframeBITS->residualbits[sframeBITS->subframeType];

		// UNRATE_MATCHING
		error=runUnRateMatching(in0, unrmdata, Rcodeblocklength, dec_rcv_samples);
		if(error == -1)return(-1);

		// TURBODECODER
		runLTETurboDeCoder(unrmdata, rmdata, Rcodeblocklength, 0);

		// COMPUTE BER (FOR TESTING)
		//OK computeBERchars(Rcodeblocklength-Rresidualbits,testdata, rmdata, 100);
	
		// CONVERT BITS TO CHARS AND SEND DATA TO OUTPUT BUFFER
//		snd_samples=BITS2CHARS(rmdata, out0, Rcodeblocklength-Rresidualbits);
//		printf("Decod snd_samples(BYTES)=%d\n", snd_samples);
		
		return(snd_samples);

}*/




const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}



