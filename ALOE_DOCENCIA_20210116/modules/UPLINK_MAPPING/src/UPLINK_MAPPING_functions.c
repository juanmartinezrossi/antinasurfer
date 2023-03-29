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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "UPLINK_MAPPING_functions.h"

MODparams_t oParam={NORMAL_NORS, 1.4, 0, 9, 0, 0, 0, 0 };		// Initialize module params struct
MODvars_t oVars={NUMRB, 156, 256};

extern char mname; 

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */

void generateDMRSsequence(_Complex float DMRS_SEQ, int length){


}



/**
 * @brief Function documentation: genRSsignalargerThan3RB()
 * This function calculates the PUSCH reference sequence, DMRS.
 * @params
 * @params  _Complex float *DMRSseq: Output array.
 * @params int TxRxMode: -1 (Tx Mode), 1 (Rx Mode) .
 *
 * @return On success returns 1.
 * On error returns -1.
 */

int genRSsignalargerThan3RB(int u, int v, int m, int M_RS_SC, _Complex float *DMRSseq, int TxRxMode){
	int i, sequence_length;
	double arg, rootidx;
	int N_RS_ZC = largestprime_lower_than(M_RS_SC);
	float qNfloat = ((float)(N_RS_ZC*(u+1)))/31.0;
	int qNint = qNfloat;
	float q = (float)(int)((qNfloat + 0.5)) + (float)v*(float)pow((double)(-1.0), (double)(int)(2.0*qNfloat));

	sequence_length=M_RS_SC;

	printf("u=%d, v=%d, N_RS_ZC=%d, qNfloat=%f, qNint=%d, q=%f\n", u, v, N_RS_ZC, qNfloat, qNint, q);

	for(i=0; i<sequence_length; i++){
		arg=(((double)q)*((double)(i%N_RS_ZC)*((double)(i%N_RS_ZC)+1.0)))/(double)N_RS_ZC;
		__real__ DMRSseq[i]=(float)cos(arg)*5.0;
		__imag__ DMRSseq[i]=(float)sin(arg)*5.0;
	}
	return sequence_length;
}


#define NODATA	0
#define DATA	1
#define CARRIERSperRB	12
#define PUCCHRBs		2
#define NOFOFDMSYMB		14
#define PUSCH_RS		2

/**
 * @brief Defines the function activity.
 * @param int dataFLAG: Indicates if module asked for data.
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int generate_uplinkLTE_subframe(int iTslot, int dataFLAG, MODvars_t *Mvars, MODparams_t *oParam, 
							_Complex float *inMQAMsymb, int rcv_samples, 
							_Complex float *RSseq, int RSlength,
							_Complex float *outbuffer){

	static int flag, DataAskTslot;
	int numOutSamples=0, subframe, expectedSMPLs=((Mvars->numRBs - PUCCHRBs)*CARRIERSperRB*(NOFOFDMSYMB-PUSCH_RS));

//	printf("dataFLAG=%d\n", dataFLAG);

	if(dataFLAG > 0){
		flag=1;	// Indicates that a new packet is expected
		DataAskTslot=iTslot;
	}
	if(rcv_samples==expectedSMPLs && flag==1){
//		printf(">>>>>>>>>>>>>>>>>>>>>>>>> ProcessingChainDelay%d, expectedSMPLs=%d, rcv_samples=%d\n", iTslot-DataAskTslot, expectedSMPLs, rcv_samples);
		numOutSamples=create_data_SUBFRAME(Mvars, inMQAMsymb, rcv_samples, RSseq,  outbuffer);
		flag=0;
	}

	return(numOutSamples);
}

/**
 * @brief Defines the function activity.
 * @param int dataFLAG: Indicates if module asked for data.
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int generate_uplinkLTE_subframe2(int iTslot, int dataFLAG, MODvars_t *Mvars, MODparams_t *oParam, 
							_Complex float *inMQAMsymb, int rcv_samples, 
							_Complex float *RSseq0, _Complex float *RSseq1, int RSlength,
							_Complex float *outbuffer){

	static int flag, DataAskTslot;
	int numOutSamples=0, subframe, expectedSMPLs=((Mvars->numRBs - PUCCHRBs)*CARRIERSperRB*(NOFOFDMSYMB-PUSCH_RS));

	if(dataFLAG > 0){
		flag=1;	// Indicates that a new packet is expected
		DataAskTslot=iTslot;
	}

	if(rcv_samples==expectedSMPLs && flag==1){
//		printf(">>>>>>>>>>>>>>>>>>>>>>>>> ProcessingChainDelay%d, expectedSMPLs=%d, rcv_samples=%d\n", iTslot-DataAskTslot, expectedSMPLs, rcv_samples);
		numOutSamples=create_data_SUBFRAME2(Mvars, inMQAMsymb, rcv_samples, RSseq0, RSseq1, outbuffer);
		flag=0;
	}

	return(numOutSamples);
}



#define CARRIERS1RB	12

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int create_PUSCHspectrum(int numRBs,
							_Complex float *MQAMsymb, int datalength, 
							int FFTlength,  _Complex float *out_spectrum){
	int i, j;
	

	j=FFTlength-datalength/2;
	for(i=0; i<datalength; i++){
		*(out_spectrum+j)=*(MQAMsymb+i);
		j++;
		if(j==FFTlength) j=0;
	}


	return(1);
}


int create_data_SUBFRAME(MODvars_t *Mvars, _Complex float *inbuffer, int inputlength,
							_Complex float *DMRS,  _Complex float *outbuffer){
	int k,j;
	int datasize=(Mvars->numRBs-2)*12;

	//ADD DATA
	j=0;
	for(k=0; k<14; k++){
		create_PUSCHspectrum(Mvars->numRBs, inbuffer+k*datasize, datasize, Mvars->FFTsz, outbuffer+j*Mvars->FFTsz);
		j++;
		if(j==3 || j==10)j++;
	}	
	//ADD DMRS
	create_PUSCHspectrum(Mvars->numRBs, DMRS, Mvars->DMRSsz, Mvars->FFTsz, outbuffer+3*Mvars->FFTsz);
	create_PUSCHspectrum(Mvars->numRBs, DMRS, Mvars->DMRSsz, Mvars->FFTsz, outbuffer+10*Mvars->FFTsz);
	return(14*Mvars->FFTsz);
}


int create_data_SUBFRAME2(MODvars_t *Mvars, _Complex float *inbuffer, int inputlength,
							_Complex float *DMRS0, _Complex float *DMRS1, _Complex float *outbuffer){
	int k,j;
	int datasize=(Mvars->numRBs-2)*12;

	//ADD DATA
	j=0;
	for(k=0; k<14; k++){
		create_PUSCHspectrum(Mvars->numRBs, inbuffer+k*datasize, datasize, Mvars->FFTsz, outbuffer+j*Mvars->FFTsz);
		j++;
		if(j==3 || j==10)j++;
	}	
	//ADD DMRS
	create_PUSCHspectrum(Mvars->numRBs, DMRS0, Mvars->DMRSsz, Mvars->FFTsz, outbuffer+3*Mvars->FFTsz);
	create_PUSCHspectrum(Mvars->numRBs, DMRS1, Mvars->DMRSsz, Mvars->FFTsz, outbuffer+10*Mvars->FFTsz);
	return(14*Mvars->FFTsz);
}


int create_zero_SUBFRAME(MODvars_t *Mvars, _Complex float *inbuffer, int inputlength,
							_Complex float *DMRS,  _Complex float *outbuffer){

	int k;
	int datasize=14*Mvars->FFTsz;

	for(k=0; k<datasize; k++)*(outbuffer+k)=0.0+0.0*I;
	return(datasize);
}


/**
 * @brief Calculates the number of samples to ask for.
 * @param .
 *
 * @param 
 * @return -1 if error, the number of output data if OK

 */
int askFORdata(int tslot, MODparams_t *Mvars){

	int datalength=0;	

	if(tslot > Mvars->delayTOask){
		if(tslot%Mvars->dataPeriod == 0){
			datalength=Mvars->NofDataSampl2ask;
		}
	}
//	printf("MAPPING_FUNCTIONS: Tslot=%d, datalength=%d\n", tslot, datalength);
	return(datalength);
}




//////////////////////////////////////////////LIBRARY/////////////////////////////////////////////////////////
int largestprime_lower_than(int number){

	int i; 

	for(i=1; i<number; i++){
		if(check_if_prime(number-i)==1) break;
	}
	return(number-i);
}

int check_if_prime(int number){

    int n, i, flag = 1;

    for(i = 2; i <= number/2; ++i)
    {
        // condition for nonprime number
        if(number%i == 0)
        {
            flag = 0;
            break;
        }
    }
	// flag 1 if prime
	// flag 0 if not
	return flag;
	
}



