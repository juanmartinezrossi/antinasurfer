#include "DSP_tools.h"

#include "phal_sw_api.h"

/******************************
 *    CONVOLUTION FUNCTION    *
 ******************************/
int conv_int(int *filter, int *state, int length, int din)
{
	int i;
	int res=0;

	for (i=length-1; i>0; i--)
		state[i]=state[i-1];
	state[0]=din;

	for (i=0; i<length; i++)
		res+=filter[i]*state[i];

	return(res);
}

short conv_sint(short *filter, short *state, int length, short din)
{
	int i;
	short res=0;

	for (i=length-1; i>0; i--)
		state[i]=state[i-1];
	state[0]=din;

	for (i=0; i<length; i++)
		res+=filter[i]*state[i];

	return(res);
}

float conv_float(float *filter, float *state, int length, float din)
{
	int i;
	float res=0;

	for (i=length-1; i>0; i--)
		state[i]=state[i-1];
	state[0]=din;

	for (i=0; i<length; i++)
		res+=filter[i]*state[i];

	return(res);
}

/******************************
 *    POWER FUNCTIONs    *
 ******************************/
float power_cxfloat(_Complex float *in, int inlength){
	int i;
	float power=0.0, a, b;

	for(i=0; i<inlength; i++){
		a = __real__ *(in+i);
		b = __imag__ *(in+i);
		power = power + a*a + b*b;
	}
	//printf("POWERMEAS=%3.12f\n", power/((float) inlength));
	return(power/((float) inlength));

}






