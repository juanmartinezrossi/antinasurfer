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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

//#define DEBUGG


#define UPLINK					1
#define DOWNLINK 				0

#define STRLEN		16
#define TCBITS		0
#define TCCHAR		1
#define TCINT			2
#define TCFLOAT		3
#define TCCPLX		4

int turboTEST();
void initLTETurboCoder(int codeblocklength);
void runLTETurboCoder(char *in, char *out, int length);
// TURBO DECODER
#define HALT_METHOD_MIN_		1
#define HALT_METHOD_MEAN_		2
#define HALT_METHOD_NONE_		0
#define DT						10
void initLTETurboDeCoder(int codeblocklength, int MaxIterations, int TurboDT, int HaltMethod);
void runLTETurboDeCoder(float *in, char *out, int codeblocklength, int halt);
//float computeBER(int length, char *odata, char *rdata, int BERperiod);
float computeBERchars(int length, char *odata, char *rdata, int BERperiod);
float computeBERbits(int length, char *odata, char *rdata, int BERperiod);
void bits2floats(char *in, float *out, int length);
int CHARS2BITS (char *input, char *output, int inputlength);
int BITS2CHARS (char *input, char *output, int inputlength);
void printarray(char *title, void *datain, int datatype, int length);
int getCodeBlock_length(int nbits);
// DATA SOURCE
#define TC_BIT_ALLZEROS 		0
#define TC_BIT_ALLONES  		1
#define TC_BIT_RANDOM	 			2
#define TC_PSEUDORANDOM	 		3
#define TC_CHAR_RANDOM	 		4


void initDataSource(int sourcetype, int datalength);
int runDataSource(char *data, int datalength);

// RATEMATCHING
void initRateMatching(int codeblocklength, int numoutbits);
int runRateMatching(char *in, char *out, int codeblocklength, int numoutbits);
// UNRATEMATCHING
void initUnRateMatching(int codeblocklength, int numoutbits);
int runUnRateMatching(float *in, float *out, int codeblocklength, int numoutbits);

// NOISE
void gen_noise(float *x, float variance, int len);
float aver_power(float *in, int length);
float rand_gauss (void);
float get_variance(float snr_db,float scale);

// NUMBER OF BITS
#define MAXCBSIZE	6144 //4096


typedef struct {
	int subframeType;					// DOWNLINK  0: 14 FFTs, 1: 13 FFTs, 2: 7 FFTs
										// UPLINK    0: 12 FFTs, 1: 12 FFTs, 2: 12 FFTs
	int MAXsubframeBits[3];				// Holds the MAX number of bits of different subframes
	int bits2rcvSF[3];					// Holds de expected bits to be coded.
	int codeblocklengthSF[3];			// Number of bits at the input of turbocoder. Not all values possible.
	int codedBitsSZ[3];					// Expected nof coded bits after ratematching for each codeblocklength
	int residualbits[3];				// Number of bits to add to adjust bits2rcvSF with codeblocklength for each subframe
	float CodeRate;						// CodeRate=bits2rcvSF/MAXsubframeBits;
	int NumRBs;							// Number or Resource Blocks
	int NumCBlocks;						// Number of Code Blocks
	int ResidualCBsz;					// Residual Code Block size
	int ResidualCodedSZ; 				// Expected nof coded bits after ratematching for ResidualCBsz
	int Totalbits2rcv;					// Total bits expected to receive
} sframeBits_t;

void calculateCodeBlockLengths(sframeBits_t *sframeBITS, int NofFFTs, int NofRB, int ModOrder, int UpDownLink);
//void calculateCodeBlockLengthsUPLINK(sframeBits_t *sframeBITS, int NofFFTs, int NofRB, int ModOrder)
void calculateCodeBlockLengthswithRS(sframeBits_t *sframeBITS, int NofFFTs, int NofRB, int ModOrder);
int checkDECOD_inlength(sframeBits_t *sframeBITS, int inDATAlength);
//int checkENCOD_inlength(sframeBits_t *sframeBITS, int inDATAlength);
int checkENCOD_DOWNLINK_inlength(sframeBits_t *sframeBITS, int inDATAlength);
int checkENCOD_UPLINK_inlength(sframeBits_t *sframeBITS, int inDATAlength);
float computeBERchars(int length, char *odata, char *rdata, int BERperiod);
int turboTEST(sframeBits_t sframeBITS, int length); 

//MAY20
//int encoder(char *in0, int rcv_samples, sframeBits_t *sframeBITS, char *out0);
int encoder(char *in0, int rcv_samples, int expectedOUTbits, char *out0);
//int decoder(char *in0, int rcv_samples, char *datatypeIN, int datain_sz, sframeBits_t *sframeBITS, char *out0);
int decoder(float *in0, int rcv_samples, char *datatypeIN, int datain_sz, sframeBits_t *sframeBITS, char *out0, int expectedOUTbits);
const char *byte_to_binary(int x);


#endif
