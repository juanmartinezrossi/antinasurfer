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

#include "computeEVM_functions.h"


/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

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



float computeEVM_3GGP_LTE128(_Complex float *inputMeasured, _Complex float *inputReference, int nofsamples){

	int i, k;		
	double EQMdiff=0.0, EQMsignal=0.0;
	static double EVM=0.0, EVMaver=0.0;
	static int nofIterations=0;


//	printf("INIT EVM=%f, nofiterations=%d, nofsamples=%d\n", EVM, nofIterations, nofsamples);

	nofIterations++;
	EQMdiff=0.0;
	EQMsignal=0.0;
	for(k=0; k<nofsamples; k++){
		EQMsignal += pow(cabsf(*(inputReference+k)), 2);
		EQMdiff += pow(cabsf(*(inputMeasured+k) - *(inputReference+k)), 2);
		//EQMsignal += cabsf(*(inputReference+k));
		//EQMdiff += cabsf(*(inputMeasured+k) - *(inputReference+k));
/*		if(k<5){
			printf("RefI=%3.3f, RefQ=%3.3f, SigI=%3.3f, SigQ=%3.3f\n", 
				__real__ *(inputReference+k), __imag__ *(inputReference+k), 
				__real__ *(inputMeasured+k), __imag__ *(inputMeasured+k));
		}
*/
	}

	if(EQMsignal > 0.000001)EVM += sqrt(EQMdiff/EQMsignal);
	else EVM += 0.00001;

	EVMaver=EVM/(float)nofIterations;
	if(EVMaver > 0.000000000001)printf("computeEVM: EVMc=%f%, EQMsignal=%f, EQMdiff=%f, SNR=%f dBs\n", EVMaver*100, EQMsignal, EQMdiff, 20*log10(1.0/EVMaver));

	if(nofIterations==10){
		nofIterations=0;
		EVM=0.0;
		EQMsignal=0.0;
		EQMdiff=0.0;
	}

	return((float)(EVMaver*100.0));
}

#define MAXVALUE	1.0

int normA(_Complex float *inout, int length){
	int i;
	float maxval, auxR, auxI;
	static float ratio;

	maxval = 0.00000001;
	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		if(maxval < auxR){
			maxval = auxR;
		}
		if(maxval < auxI){
			maxval = auxI;
		}
	}
	ratio = (MAXVALUE)/maxval;		

	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}

int normB(_Complex float *inout, int length){
	int i;
	float maxval, auxR, auxI;
	static float ratio;

	maxval = 0.00000001;
	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		if(maxval < auxR){
			maxval = auxR;
		}
		if(maxval < auxI){
			maxval = auxI;
		}
	}
	ratio = (MAXVALUE)/maxval;		

	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}

