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
#define PI 3.14159265358979323846

#define SLENGTH					32
#define NUMRB					15
#define RBCARRIERS				12
#define NOFOFDMSYMBPERSUBFRAME	14
#define NOFOFDMSYMBPERSLOT		7
#define NOFSLOTSPERSUBFRAME		2
#define FFTSIZE					256

#define TXMODE					1
#define RXMODE					-1



int genRSsignalargerThan3RB(int u, int v, int m, int M_RS_SC, _Complex float *DMRSseq, int TxRxMode);
int genDMRStime_seq(int FFTsize, _Complex float  *in_DMRSfreq, int DMRSlength, _Complex float  *out_DMRStime, int TxRxmode);

int detect_DMRS_in_subframe(_Complex float *correl, int numsamples);


int write_subframe_buffer(_Complex float *datain, int pMAX, int rcv_samples, int FFTsize);
int read_subframe_buffer(_Complex float *datain, int FFTsize);

int largestprime_lower_than(int number);
int check_if_prime(int number);
int stream_conv_CPLX(_Complex float *ccinput, int datalength, _Complex float *filtercoeff, int filterlength, _Complex float *ccoutput);
int createDMRS(_Complex float *DMRS);
//int setPSS(int phylayerID, _Complex float *PSSsymb, int TxRxMode);

void rotateCvector(_Complex float *in, _Complex float *out, int length, float Adegrees);
float checkPhaseOffset(_Complex float *DMRScorrelation);
#endif
