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

//////////// Global Control Struct ////////////////////////////////////

#define SLENGTH					32
#define NUMRB					15
#define NUMCARRIERS				12
// opMODE
#define NORMAL_NORS				0	// NO Sounding Reference Signal




typedef struct MODparams{
    int opMODE;							// Options: 0-DMRS_ONLY, NO_SRS, 1: SRS_ALSO
	float cellBwMHz;					// Options: 1.4, 3.0, 5.0, 10.0, 15.0, 20.0 
	int Csrs;							// SRS Bandwidth config. Options: 0, 1, 2, 3, 4, 5, 6, 7
	int SubConfsrs; 					// SRS subframe config. Options: 0-15
	int Bsrs; 							// SRS bandwidth. Options: 0-3
	int nRRC; 							// Frequency domain position. Options: 1-23
	int Isrs; 							// SRS configuration index. Options: 
	int kTC; 							// Transmission comb. Options: 0, 1
										// Go to http://niviuk.free.fr/lte_srs.php for details. (visited 9-18-19)
	int dataPeriod;						// Period to ask for user data
	int delayTOask;						// Time slots to wait before starting ask for data
	int NofDataSampl2ask;				// Number of data to ask for.
	int ExptdProcDelay;					// Expected processing delay between asking for data and receiving them.

	int BitsperSymbQAM;					// options: 1-BPSK, 2-4QAM, 4-16QAM, 6-64QAM, 8-256QAM
}MODparams_t;

typedef struct MODvars{
    int numRBs;							// Number of RBs: 6, 15, 25, 50, 75, 100
	int DMRSsz;
	int FFTsz;							// FFT size
}MODvars_t;






//////////// Functions: Predefine ////////////////////////////////////
//#define PI 3.14159265358979323846

int genRSsignalargerThan3RB(int u, int v, int m, int M_RS_SC, _Complex float *ZCseq, int TxRxMode);
int generate_uplinkLTE_subframe(int iTslot, int dataFLAG, MODvars_t *Mvars, MODparams_t *oParam, 
							_Complex float *inMQAMsymb, int rcv_samples, 
							_Complex float *RSseq, int RSlength,
							_Complex float *outbuffer);
int generate_uplinkLTE_subframe2(int iTslot, int dataFLAG, MODvars_t *Mvars, MODparams_t *oParam, 
							_Complex float *inMQAMsymb, int rcv_samples, 
							_Complex float *RSseq0, _Complex float *RSseq1, int RSlength,
							_Complex float *outbuffer);

int create_data_SUBFRAME(MODvars_t *Mvars, _Complex float *inbuffer, int inputlength,
							_Complex float *DMRS,  _Complex float *outbuffer);
int create_data_SUBFRAME2(MODvars_t *Mvars, _Complex float *inbuffer, int inputlength,
							_Complex float *DMRS0, _Complex float *DMRS1, _Complex float *outbuffer);

int create_zero_SUBFRAME(MODvars_t *Mvars, _Complex float *inbuffer, int inputlength,
							_Complex float *DMRS,  _Complex float *outbuffer);
int create_PUSCHspectrum(int numRBs,
							_Complex float *MQAMsymb, int datalength, 
							int FFTlength,  _Complex float *out_spectrum);
int askFORdata(int tslot, MODparams_t *Mvars);


int check_if_prime(int number);
int largestprime_lower_than(int number);

#endif
