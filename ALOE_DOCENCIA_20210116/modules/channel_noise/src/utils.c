/*
 * utils.c
 *
 *  Created on: Apr 11, 2012
 *      Author: odissey09
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>

#include <phal_sw_api.h>



#include "utils.h"


void initCbuff(buffctrl *bufferCtrl, _Complex float *bufferC, int buffersz, int firstpcketsz){

	memset(bufferC, 0, sizeof(_Complex float)*buffersz);
	bufferCtrl->writeIndex = firstpcketsz;
	bufferCtrl->readIndex = firstpcketsz;
	bufferCtrl->buffsize = buffersz;
	bufferCtrl->occuplevel=0;
	bufferCtrl->roomlevel=buffersz;

	printf("buffCtrl->writeIndex=%d: \n", bufferCtrl->writeIndex);
	printf("buffCtrl->occuplevel=%d: \n", bufferCtrl->occuplevel);
	printf("buffCtrl->readIndex=%d: \n", bufferCtrl->readIndex);
	printf("bufferCtrl->buffsize=%d: \n", bufferCtrl->buffsize);
	printf("*bufferC=%u: \n", (_Complex float *)bufferC);
}


/**
* void write(buffer* buffer, _Complex float value, int length)
* writes value into the buffer
* @param buffer* buffer
*   pointer to buffer to be used
* @param _Complex float value
*   value to be written in buffer
*/

int writeCbuff(buffctrl *buffer, _Complex float *buffdata, _Complex float *in, int length){
	int i;

	if(buffer->writeIndex >= buffer->readIndex){
		buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
	}
	else{
		buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
	}
	buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
	if(buffer->roomlevel >= length){
		for(i=0; i<length; i++){
			buffdata[buffer->writeIndex]=*(in+i);
			buffer->writeIndex++;
			if(buffer->writeIndex==buffer->buffsize){
				buffer->writeIndex=0;
			}
		}
		if(buffer->writeIndex >= buffer->readIndex){
			buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
		}
		else{
			buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
		}
		buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
		return 1;
	}else{
		printf("Error writeCbuff: Not enough space in buffer\n");
		return -1;
	}
}
/**
* void readn(buffer* buffer, int Xn)
* reads specified value from buffer
* @param buffer* buffer
*   pointer to buffer to be read from
* @param int Xn
*   specifies the value to be read from buffer counting backwards from the most recently written value
*   i.e. the most recently writen value can be read with readn(buffer, 0), the value written before that with readn(buffer, 1)
*/
int readCbuff(buffctrl* buffer,_Complex float *buffdata, _Complex float *out, int length){
	int i;

	if(buffer->writeIndex >= buffer->readIndex){
		buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
	}
	else{
		buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
	}
	buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
	if(buffer->occuplevel >= length){
//		printf("readCbuff(): buffer->occuplevel=%d, length=%d, buffer->readIndex=%d\n", buffer->occuplevel, length, buffer->readIndex);
		for(i=0; i<length; i++){
//			if(i==0)printf("*bufferC=%u: \n", (_Complex float *)(buffdata+buffer->readIndex));//printf("buffer->readIndex=%d\n", buffer->readIndex);
			*(out+i) = *(buffdata+buffer->readIndex);
			buffer->readIndex++;

			if(buffer->readIndex==buffer->buffsize){
				buffer->readIndex=0;
			}
		}
		if(buffer->writeIndex >= buffer->readIndex){
			buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
		}
		else{
			buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
		}
		buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
		return 1;
	}else{
/*		modinfo("Error readCbuff: Not enough data in buffer.\n");
		modinfo_msg("buffer->occuplevel=%d, length=%d\n", buffer->occuplevel, length);
		modinfo_msg("buffer->buffsize=%d, buffer->writeIndex=%d, buffer->readIndex=%d\n", buffer->buffsize, buffer->writeIndex, buffer->readIndex);
*/
		printf("Error readCbuff: Not enough data in buffer.\n");
		printf("buffer->occuplevel=%d, length=%d\n", buffer->occuplevel, length);
		printf("buffer->buffsize=%d, buffer->writeIndex=%d, buffer->readIndex=%d\n", buffer->buffsize, buffer->writeIndex, buffer->readIndex);

		return -1;
	}
}

#define DFT_MIRROR	1
#define DFT_NOMIRROR	2

inline static void copy_in(char *dst, char *src, int size_d, int len, int options) {
	int hlen;
	if (options & DFT_MIRROR) {
		//hlen = div(len,2);
		hlen=len/2;
		printf("hlen=%d\n", hlen);
		memcpy(dst, &src[hlen*size_d], size_d*hlen);
		memcpy(&dst[hlen*size_d], src, size_d*(len - hlen));
	} else {
		memcpy(dst,src,size_d*len);
	}
}

/* Subtract the `struct timeval' values X and Y,
    storing the result in RESULT.
    Return 1 if the difference is negative, otherwise 0.  */

void timeval_subtract (struct timeval *result, struct timeval *init, struct timeval *end){

	//int nsec;


	if(init->tv_sec == end->tv_sec){
		result->tv_sec = 0;
		result->tv_usec = end->tv_usec-init->tv_usec;
	}

	if(init->tv_sec < end->tv_sec){
		result->tv_sec = end->tv_sec - 1 -init->tv_sec;
		result->tv_usec = end->tv_usec + 1000000 - init->tv_usec;
	}
 }

/*FFT of complex signal*/
/* __real__ output contains the power density in dB
 * __imag__ output contains the phase in radians
 */
#define PI 3.1416

/*void c_spectrum(_Complex float *input, _Complex float *output, int datasz){

	int i;
	float auz;
	fftwf_plan customplan;

	//Move to time domain
	customplan = fftwf_plan_dft_1d(datasz, input, output, FFTW_FORWARD, FFTW_ESTIMATE);
	fftwf_execute(customplan);
	for(i=0; i<datasz;i++){
//		auz=sqrt((__real__ *(output+i))*(__real__ *(output+i)) + (__imag__ *(output+i))*(__imag__ *(output+i)));
//		__real__ *(output+i) = 20*log(auz);

		__real__ *(output+i) = 20*log(cabsf(*(output+i)));
		__imag__ *(output+i) = atan2(__imag__ *(output+i),__real__ *(output+i));
	}
}

*/


