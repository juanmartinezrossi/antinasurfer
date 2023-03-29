#ifndef _DSPTOOLS_H
#define	_DSPTOOLS_H


int conv_int(int *filter, int *state, int length, int din);
short conv_sint(short *filter, short *state, int length, short din);
float conv_float(float *filter, float *state, int length, float din);
float power_cxfloat(_Complex float *in, int inlength);



#endif
