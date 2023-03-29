/*
 * ratematching.c
 *
 *  Created on: 27/06/2012
 *      Author: xavier
 */

#include <stdio.h>
#include <string.h>
#include "ctrl_ratematching.h"

int float_wrap (float * in0, float * in1, float * in2, float * out, int insize)
{
	int i;
	for (i=0; i<insize; i++)
	{
		*(out++)=*(in0++);
		*(out++)=*(in1++);
		*(out++)=*(in2++);
	}
//	printf("NofOutput URM=%d\n", insize*3);
	return insize*3;
}

int char_unwrap (char * input, char * out0, char * out1, char * out2, int insize)
{
	int i;
	insize /= 3;

	for (i=0; i<insize; i++)
	{
		*(out0++)=*(input++);
		*(out1++)=*(input++);
		*(out2++)=*(input++);
	}

	return insize;
}

/* Return the result of floor(x/y) in integer */
#define intfloor(X, Y) ((X%Y>0)?((X-X%Y)/Y):(X/Y))
#define intceil(X, Y) ((X%Y>0)?((X-X%Y)/Y+1):(X/Y))
#define COLS 32
#define ROWS intceil(inlen, COLS)
int P [32] = {0, 16, 8, 24, 4, 20, 12, 28,\
		2, 18, 10, 26, 6, 22, 14, 30, \
		1, 17, 9, 25, 5, 21, 13, 29, \
		3, 19, 11, 27, 7, 23, 15, 31};
#define index ((P[intfloor(k, rows)]+COLS*(k%rows))%blockSize)
#define DUMMYBIT 2

int char_interleave (char *in, char *out, int inlen)
{
	int k, blockSize, rows;

	blockSize = COLS*ROWS;
	rows = ROWS;

	// Add dummy bits
	for (k=inlen; k<blockSize; k++)
		in[k]=DUMMYBIT;

	for (k=0; k<blockSize; k++)
	{
		out[k] = in[index];
	}

	return blockSize;
}

int float_deinterleave (float *in, float *out, int blockSize)
{
	int k;
	int rows = blockSize/COLS;

	for (k=0; k<blockSize; k++)
	{
		out[index] = in[k];
	}

	return blockSize;
}

#define MAXCODEDSIZE (6144*3+32+64)		//+64 AGB MAY20
#define R (interleaversize/32)

int char_ratematching (char * subblock0, char * subblock1, char * subblock2, char * output, int interleaversize, int outlen, int rvidx)
{
	int i, k0, k;
	char bit;
	char circularBuffer [MAXCODEDSIZE];
	int buffersize = 3*interleaversize;


	for (i=0; i<interleaversize; i++)
	{
		circularBuffer[i]=subblock0[i];
		circularBuffer[interleaversize+2*i+0]=subblock1[i];
		circularBuffer[interleaversize+2*i+1]=subblock2[i];
	}

	k0 = R*(2*intceil(buffersize, (8*R))*rvidx+2);
	k=0;
	i=0;
	while(k<outlen)
	{
		bit = circularBuffer[(k0+i)%buffersize];
		if (bit!=DUMMYBIT){
			output[k]=bit;
			k++;
		}
		i++;
	}
//	printf("outlen=%d\n", outlen);
	return outlen;
}

int float_unratematching (float * input, float * subblock0, float * subblock1, float * subblock2, int interleaversize, int inlen, int rvidx, char * dummyMatrix)
{
	int i, k0, k;
	float circularBuffer [MAXCODEDSIZE];
	int buffersize = interleaversize*3;

	memset(circularBuffer, 0, sizeof(float)*MAXCODEDSIZE);

	k0 = R*(2*intceil(buffersize, (8*R))*rvidx+2);
	k=0;
	i=0;
	while(k<inlen)
	{
		if (dummyMatrix[(k0+i)%buffersize]!=DUMMYBIT)
			circularBuffer[(k0+i)%buffersize] = input[k++];
		else
			circularBuffer[(k0+i)%buffersize] = 8.8;
		i++;
	}

	for (i=0; i<interleaversize; i++)
	{
		subblock0[i]=circularBuffer[i];
		subblock1[i]=circularBuffer[2*i+interleaversize+0];
		subblock2[i]=circularBuffer[2*i+interleaversize+1];
	}

	return interleaversize;
}

int getDummyMatrix(char * dummymatrix, int codewordsize)
{
	int matrixsize, i;
	char interleaved0 [MAXCODEDSIZE/3];
	char interleaved1 [MAXCODEDSIZE/3];
	char interleaved2 [MAXCODEDSIZE/3];
	char zeros [MAXCODEDSIZE/3];

	memset(zeros, 0, MAXCODEDSIZE/3);

	matrixsize = char_interleave (zeros, interleaved0, codewordsize);
	matrixsize = char_interleave (zeros, interleaved1, codewordsize);
	matrixsize = char_interleave (zeros, interleaved2, codewordsize);

	for (i=0; i<matrixsize; i++)
	{
		dummymatrix[i]=interleaved0[i];
		dummymatrix[2*i+matrixsize+0]=interleaved1[i];
		dummymatrix[2*i+matrixsize+1]=interleaved2[i];
	}

	return 3*matrixsize;
}

int float_UNRM_block (float * input, float * output, ctrl_ratematching_t * control)
{
	int inlen = control->insize;
	int outsize = control->outsize;
	int rvidx = control->rvidx;

	int subblocklen, intblocklen;

	float block0 [MAXCODEDSIZE/3];
	float block1 [MAXCODEDSIZE/3];
	float block2 [MAXCODEDSIZE/3];

	float intblock0 [MAXCODEDSIZE/3];	// Interleaved blocks
	float intblock1 [MAXCODEDSIZE/3];
	float intblock2 [MAXCODEDSIZE/3];

	subblocklen = intceil(outsize, 3);

	char dummymatrix [MAXCODEDSIZE];
	int interleaversize = getDummyMatrix(dummymatrix, subblocklen)/3;

	float_unratematching(input, intblock0, intblock1, intblock2, interleaversize, inlen, rvidx, dummymatrix);

	intblocklen = float_deinterleave(intblock0, block0, interleaversize);
	intblocklen = float_deinterleave(intblock1, block1, interleaversize);
	intblocklen = float_deinterleave(intblock2, block2, interleaversize);

	return float_wrap(block0, block1, block2, output, intblocklen);
}



int char_RM_block (char * input, char * output, ctrl_ratematching_t * control)
{
	int insize = control->insize;
	int outsize = control->outsize;
	int rvidx = control->rvidx;
	int out; 

	int subblocklen, interleaversize;

	char block0 [MAXCODEDSIZE];
	char block1 [MAXCODEDSIZE];
	char block2 [MAXCODEDSIZE];

	char intblock0 [MAXCODEDSIZE];	// Interleaved blocks
	char intblock1 [MAXCODEDSIZE];
	char intblock2 [MAXCODEDSIZE];

	subblocklen = char_unwrap(input, block0, block1, block2, insize);
	interleaversize = char_interleave(block0, intblock0, subblocklen);
	interleaversize = char_interleave(block1, intblock1, subblocklen);
	interleaversize = char_interleave(block2, intblock2, subblocklen);
	

	out=char_ratematching(intblock0, intblock1, intblock2, output, interleaversize, outsize, rvidx);
//	printf("out=%d\n", out);
	return out;
}

int ratematching (void * input, void * output, ctrl_ratematching_t * control)
{

//	printf("control->mode=%d\n", control->mode);
	if (control->mode == CHAR_RM)
	{
		return char_RM_block (input, output, control);
	}
	else if (control->mode == FLOAT_UNRM)
	{
		return float_UNRM_block (input, output, control);
	}

	return (-1);
}


