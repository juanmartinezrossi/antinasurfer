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

// Module Variables and Defines
#define PI			3.14159265359				/* pi approximation */
#define PIx2		6.28318530718
#define STR_LENGTH			128
#define BYPASS					2
#define NORMAL					3

//#define MAXOPERATIONS	20000

#define FILTERLENGTH			2048
#define MAXINTERPOLATION		64

typedef struct MODparams{
    int opMODE;
		float samplingfreqHz;
    float f0_freqHz;
    float gain;
    int Ninterpol;
		int Ndecimate;
		char filtercoefffile[STR_LEN];
}MODparams_t;

static MODparams_t oParam={NORMAL, 48000.0, 3500.0, 1.0, 9, 3, "DUC17.coeff"};		// Initialize module params struct

/*************************************************************************************************/
// Functions Predefinition
int bypass(_Complex float *input, int datalength,_Complex float *output);
void readREALfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername);
int stream_conv_CPLX_DDC(	_Complex float *ccinput, 
											int datalength,
											_Complex float *filtercoeff, 
											int filterlength, 
											_Complex float *ccoutput, 
											MODparams_t oParam);
int init_toneCOMPLEX(_Complex float *table, int length, float gain);
int gen_toneCOMPLEX(_Complex float *func_out, _Complex float *tablel, int tablesz,
					int datalen, float tone_freq, float sampl_freq);
int IQ_demodulator_carrier_f0(_Complex float *phasor_f0, _Complex float *in, _Complex float *out, int length);
void initDDC(MODparams_t oParam);
int IQ_DEMOD(_Complex float *ccinput, int datalength, _Complex float *ccoutput, MODparams_t oParam);
#endif
