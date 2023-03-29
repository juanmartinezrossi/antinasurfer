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
#include <math.h>

#include "CHANNEL_ANALYZER_functions.h"

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param datalength: Number of samples
 * @param pulselength: Length of the pulse generated.
 * @param gain: Value of the pulse.
 * @return -1 if error, the number of output data if OK

 */
int init_pulse_COMPLEX(_Complex float *input, int datalength, int pulselength, float gain){
	int i;

	for (i=0;i<datalength;i++) {
		if(i < pulselength)__real__ input[i]=gain*1.0;
		else __real__ input[i]=0.0;
		__imag__ input[i]=0.0;
	}
	return(1);
}


/**
 RES

 */
int bypass_COMPLEX(_Complex float *input, int inlength, _Complex float *output)
{
	int i,outlength;

	for (i=0;i<inlength;i++) {
		__real__ output[i] = __real__ input[i];
		__imag__ output[i] = __imag__ input[i];
	}
	outlength=inlength;
	return outlength;
}
//Initcialitzem TO Pas 1
int init_toneCOMPLEX(_Complex float *table, int length, float ref_freq, float gain, float sampl_freq){
	int i;
	double arg=PIx2/((float)length);

	for (i=0;i<length;i++) {
		__real__ table[i]=gain*(float)cos(arg*(float)i);
		__imag__ table[i]=gain*(float)sin(arg*(float)i);
	}
	return(1);
}
//PAs 2
int gen_toneCOMPLEX(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float ref_freq, float tone_freq, float sampl_freq){
	int i, k=1;
	static int j=0;
	float ref_freq_c;

	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		func_out[i] = tablel[j];
		j += k;
		if(j>=tablesz)j-=tablesz;
	}
	return(1);
}

int gen_cosinus_REAL_COMPLEX(_Complex float *func_out, _Complex float *tablel, int tablesz,
					int datalen, float ref_freq, float tone_freq, float sampl_freq){
	int i, k=1;
	static int j1=0;
	float ref_freq_c;

	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		__real__ func_out[i]=__real__  tablel[j1];
		__imag__ func_out[i]=__imag__  tablel[j1];
		j1 += k;
		if(j1>=tablesz)j1-=tablesz;
	}
	return(1);
}


// Detect if signal is over some threshold
int detect_rcv_data(_Complex float *in, int length, float threshold){

	int i;
	int numsample=0;
	for(i=0; i<length; i++){
		if((__real__ *(in+i)) >= threshold)numsample=1+i;	// if signal has been detected
		if((__imag__ *(in+i)) >= threshold)numsample=1+i;
		if(numsample>0){
			printf("Detected Sample: Re=%f, Im=%f\n", __real__ *(in+i),  __imag__ *(in+i));
			return(numsample);
		}
	}
	return 0; 	// If no signal detected
}


int delay(_Complex float *in, int length, float threshold, int Tslot, float samplingfreq){

	static int detected=0;

	if(detected==0){
		detected=detect_rcv_data(in, length, threshold);
		printf("\nNumber of Tslot delayed=%d\n", Tslot);
		printf("Number of samples delayed from Tslot origin=%d\n", detected);
		printf("Total Delay=%3.6f sec\n", ((float)(length*Tslot) + detected)/samplingfreq);
		if(detected > 0)return(Tslot);
	}
	return(0);
}

float get_CH_gaindBs(_Complex float *in, int length){
	int i;
	static int TSLOT=0;
	float MAX=-1000000.0,MIN=1000000.0, gaindBs, Amplitude;

	for(i=200; i<length; i++){
		if(MAX < __real__ in[i]){
			MAX=__real__ in[i];
		}
		if(MIN > __real__ in[i]){
			MIN=__real__ in[i];
		}
	}
	Amplitude=(MAX-MIN)/2.0;
	if(Amplitude >= 2.0){
		printf("WARNING!!! MICROPHONE IN SATURATION. RECEIVED LEVEL TOO HIGH\n");
		printf("MEASURE PERIOD=%d. PLEASE, REDUCE THE OUTPUT OR INPUT GAIN IN YOUR LAPTOP AUDIO CONTROLS\n", TSLOT);
	}
	gaindBs=(float)(20.0*log10(Amplitude));

	TSLOT++;
	return(gaindBs);
}


/*#define AVERBLOCK	100

float get_CH_gaindBs(_Complex float *in, int length){
	int i, k, j=0;
	static int TSLOT=0;
	float myMAX=-1000000.0, myMIN=1000000.0, gaindBs, Amplitude;
	float averMAX=0.0, averMIN=0.0;
	int numBLOCKS=1, blocklength=length;

	if(TSLOT < 10){
		numBLOCKS=1;
		blocklength=length;
	}
	else {
		numBLOCKS=length/AVERBLOCK;
		blocklength=AVERBLOCK;
	}

	for(i=0; i<numBLOCKS; i++){
		myMAX=-1000000.0;
		myMIN=1000000.0;
		for(k=0; k<blocklength; k++){
			if(myMAX < __real__ in[j]){
				myMAX=__real__ in[j];
			}
			if(myMIN > __real__ in[j]){
				myMIN=__real__ in[j];
			}
			j++;
		}
		averMAX += myMAX;
		averMIN += myMIN;
	}
	averMAX=averMAX/numBLOCKS;
	averMIN=averMIN/numBLOCKS;

	Amplitude=(averMAX-averMIN)/2.0;
	if(Amplitude >= 1.0){
		printf("WARNING!!! MICROPHONE IN SATURATION. RECEIVED LEVEL TOO HIGH\n");
		printf("MEASURE PERIOD=%d. PLEASE, REDUCE THE OUTPUT OR INPUT GAIN IN YOUR LAPTOP AUDIO CONTROLS\n", TSLOT);
	}
//	printf("Amplitude=%3.6f\n", Amplitude);
	gaindBs=(float)(20.0*log10(Amplitude));

	TSLOT++;
	return(gaindBs);
}
*/

/*OK
float get_CH_phase_rad(_Complex float *localcos, _Complex float *input, int length, _Complex float *Cprod){

	int i;
	float product[2048*10], MAXa=-100000000.0, MINa=100000000.0, VM, phase, recvA, to, freq, aux;
	double  arg;
	static float lastval=-10.0;
	static float signn=-1.0;
	static int first=0;
	static float MAXA=-3.14, MINA=3.14;

	// Compute product and Find input Amplitude
	for(i=0; i<length;i++){
		product[i]=__real__ localcos[i]*__real__ input[i];
		__real__ Cprod[i] = product[i];
		__imag__ Cprod[i] = 0.0; 
	}
	// Find product MAX/MIN
	MAXa=-100000000.0, MINa=100000000.0;
	for(i=200; i<length-200; i++){
		if(MAXa < product[i]){
			MAXa=product[i];
		}
		if(MINa > product[i]){
			MINa=product[i];
		}
	}
	VM=(MAXa+MINa)/2.0;
	recvA=(MAXa-MINa)/2.0;


	arg=(double)(VM/(recvA));

//	printf("\nMAXa=%3.5f, MINa=%3.5f, VM=%3.5f, recvA=%3.5f, arg=%3.5lf, ", MAXa, MINa, VM, recvA, arg);

	if(arg > 1.0)arg=1.0;
	if(arg < -1.0)arg=-1.0;
	phase=(float)acos(arg);
	if(isnan(phase)){
		printf("ISNAN\n");
		phase=0.0;
	}
//	printf("phase0=%3.5f\n", phase);
	return(phase);
}

*/

#define MAXSLOPE	0.01	

void correct_phase(float *phasein, int length, float *cphaseout, int modephase, float Afreq){

	int i;
	float slope=0.0;
	int signNN[1204*8];
	float group_delay=0.0;
	float Gdelay[1204*8];

	signNN[0]=1.0;
	for(i=1; i<length; i++){
		//printf("slope=%3.6f\n", phasein[i]-phasein[i-1]);
		if((phasein[i]-phasein[i-1]) > MAXSLOPE)signNN[i]=-1.0;
		else signNN[i]=1.0;
	}
	// Correct Phase
	cphaseout[0]=phasein[0];
	for(i=1; i<length; i++){
		cphaseout[i]=phasein[i]*signNN[i];
	}

	if(modephase==1){
		// Calculate Group Delay for frequency step = 50Hz
		Gdelay[i]=0.0;
		for(i=1; i<length; i++){
			Gdelay[i]=(cphaseout[i]-cphaseout[i-1])/(PIx2*Afreq);
			if(Gdelay[i] > 0.0)Gdelay[i]=Gdelay[i-1];
			else Gdelay[i]=-Gdelay[i];
		}
		for(i=0; i<length; i++){
			cphaseout[i]=Gdelay[i];
		}
	}

}

#define MAXPHASESZ	1204*8

void computeGroupDelay(float *phasein, int length, float *Gdelay, float Afreq){

	int i, k, n;
	float cphaseout[MAXPHASESZ];
	float addPI=0.0;



	if(length > MAXPHASESZ)printf("ERROR!!!: CHANNEL_ANALYZER_FUNCTIONS.computeGroupDelay() length > MAXPHASESZ\n");

	printf("Afreq=%f\n", Afreq);
	// Calculate Group Delay for frequency step = 50Hz
	cphaseout[0]=0.0;
	for(i=1; i<length; i++){
		if((phasein[i]-phasein[i-1]) > 1.0)addPI -= PIx2;
		cphaseout[i] = phasein[i] + addPI;
	}
	for(i=1; i<length; i++)Gdelay[i-1]=(cphaseout[i]-cphaseout[i-1])/(2.0*PI*Afreq);
	Gdelay[length-1]=Gdelay[length-2];	
}


/*Tranforma un senyal passa-banda al seu equivalent passa-baixos en 
 *components I/Q
 */
/*void complex_base_band(int *pint, int length, float freq, float phase)
{
	int i;
	float t;
	
	t=0.0;
	
	for (i=0; i<length; i++)
	{
		it[i]=pint[i]*(int)(cos(TWOPI*freq*t+phase)*2000.0);
		qt[i]=pint[i]*(int)(sin(TWOPI*freq*t+phase)*-2000.0);
		
		t=t+1.0;
	}
}
*/
/* Calcula l'amplitud derivada del valor en DC contingut
 * en les components I/Q obitngudes amb "complex_base_band"
 */
/*float get_gain(int dc_i, int dc_q)
{
	float mag;
	float a,b;
	
	a=(float)dc_i;
	a*=a;
	b=(float)dc_q;
	b*=b;
	
	mag=(float)sqrt(a+b);
	
	return(mag);	
}
*/
/* Calcula la fase derivada del valor en DC contingut
 * en les components I/Q obtingudes amb "complex_base_band"
 */
/*float get_phase(double dc_i, double dc_q)
{
	float ph;
//	double dc_i = (double)I, dc_q = (double)Q;
	
	if (dc_i==0)
	{
		if (dc_q>=0)
			ph=PI/2.0;
		else
			ph=-PI/2.0;
	}
	else if (dc_q==0)
	{
		if (dc_i>=0)
			ph=0.0;
		else
			ph=-PI;
	}
	else
	{
		ph=(float)atan((double)dc_q/(double)dc_i);
		if (dc_i<0)
		{
			if (dc_q<0)
				ph-=PI;
			else
				ph+=PI;
		}
	}
	return(ph);
}*/

float get_phase(double dc_i, double dc_q)
{
	float ph;

	ph=(float)atan2(dc_q, dc_i);

	return(ph);
}


float get_CH_phase_radA(_Complex float *localcos, _Complex float *input, int length, _Complex float *Cprod){

	int i;
	double prod_I[2048*10], prod_Q[2048*10];
	double aver_I = 0.0, aver_Q = 0.0;
	float phase;

	// Compute IQ components of received signal (real signal)
	for(i=0; i<length;i++){
		prod_I[i] = (double)__real__ localcos[i]*(double)__real__ input[i];
		prod_Q[i] = ((double)(-(__imag__ localcos[i]))*(double)__real__ input[i]);
		aver_I += prod_I[i];
		aver_Q += prod_Q[i];
	}
	aver_I = aver_I/(double)length;
	aver_Q = aver_Q/(double)length;

	phase = get_phase(aver_I, aver_Q);

//	printf("DCI=%3.6f, DCQ=%3.6f, phase=%3.6f\n", aver_I, aver_Q, phase);

	return(phase);
}


/*float get_CH_phase_radA(_Complex float *localcos, _Complex float *input, int length, _Complex float *Cprod){

	int i;
	double prod_I[2048*10], prod_Q[2048*10];
	double aver_I = 0.0, aver_Q = 0.0;
	float phase;

	_Complex float PRODUCT;

	// Compute IQ components of received signal (real signal)
	for(i=0; i<length;i++){

		//PRODUCT=conjf(localcos[i])*input[i];
		PRODUCT=localcos[i]*input[i];

		prod_I[i] = (double)(__real__ PRODUCT);
		prod_Q[i] = (double)(__imag__ PRODUCT);
		aver_I += prod_I[i];
		aver_Q += prod_Q[i];
	}
	aver_I = aver_I/(double)length;
	aver_Q = aver_Q/(double)length;

	phase = get_phase(aver_I, aver_Q);

//	printf("DCI=%3.6f, DCQ=%3.6f, phase=%3.6f\n", aver_I, aver_Q, phase);

	return(phase);
}*/



float get_CH_phase_radB(_Complex float *localcos, _Complex float *input, int length, _Complex float *Cprod){

	int i;
	float prod_I[2048*10], prod_Q[2048*10];
	float MAXa=-100000000.0, MINa=100000000.0, VM_I, VM_Q, phase, recvA, to, freq, aux;

	// Compute product and Find input Amplitude
	for(i=0; i<length;i++){
		prod_I[i]=__real__ localcos[i]*__real__ input[i];
		prod_Q[i]=-(__imag__ localcos[i])*__real__ input[i];
		__real__ Cprod[i] = prod_I[i];
		__imag__ Cprod[i] = prod_Q[i]; 
	}
	// Find product MAX/MIN and VM_I
	MAXa=-100000000.0, MINa=100000000.0;
	for(i=200; i<length; i++){
		if(MAXa < prod_I[i]){
			MAXa=prod_I[i];
		}
		if(MINa > prod_I[i]){
			MINa=prod_I[i];
		}
	}
	VM_I=(MAXa+MINa)/2.0;
	// Find product MAX/MIN and VM_I
	MAXa=-100000000.0, MINa=100000000.0;
	for(i=200; i<length; i++){
		if(MAXa < prod_Q[i]){
			MAXa=prod_Q[i];
		}
		if(MINa > prod_Q[i]){
			MINa=prod_Q[i];
		}
	}
	VM_Q=(MAXa+MINa)/2.0;
	// CALCULATE PHASE
	phase = get_phase(VM_I, VM_Q);

	return(phase);
}





