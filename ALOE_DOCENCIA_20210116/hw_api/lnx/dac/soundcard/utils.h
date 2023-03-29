#ifndef _UTILS_H
#define _UTILS_H

#define MAXFFTSIZE 4096

typedef struct buffer{
	int readIndex;
	int writeIndex;
	int buffsize;
	int occuplevel;	//Occupancy level of buffer for reading
	int roomlevel;	//Available room in buffer for writing
}buffctrl;

void initCbuff(buffctrl *bufferCtrl, _Complex float *bufferC, int buffersz, int firstpcketsz);
int writeCbuff(buffctrl *buffer, _Complex float *buffdata, _Complex float *in, int length);
int readCbuff(buffctrl *buffer,_Complex float *buffdata, _Complex float *out, int length);
int lookCbuff(buffctrl* buffer,_Complex float *buffdata, _Complex float *out, int length);
void timeval_subtract (struct timeval *result, struct timeval *init, struct timeval *end);

#endif
