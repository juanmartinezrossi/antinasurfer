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

#define PI		3.14159265359	/* pi approximation */
#define PIx2	6.28318530718

#define FILTERLENGTH	512

int init_pulse_COMPLEX(_Complex float *input, int datalength, int pulselength, float gain);
int bypass_COMPLEX(_Complex float *input, int inlength, _Complex float *output);
int init_toneCOMPLEX(_Complex float *table, int length, float ref_freq, float gain, float sampl_freq);
int gen_toneCOMPLEX(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float ref_freq, float tone_freq, float sampl_freq);
int gen_cosinus_REAL_COMPLEX(_Complex float *func_out, _Complex float *tablel, int tablesz,
					int datalen, float ref_freq, float tone_freq, float sampl_freq);
int detect_rcv_data(_Complex float *in, int length, float threshold);
int delay(_Complex float *in, int length, float threshold, int Tslot, float samplingfreq);
int getSpectrum(_Complex float *input0, _Complex float *oncos, _Complex float  *spectrum, int Tslot);
float get_CH_gaindBs(_Complex float *in, int length);
float get_CH_phase_radA(_Complex float *localcos, _Complex float *input, int length, _Complex float *Cprod);
float get_CH_phase_radB(_Complex float *localcos, _Complex float *input, int length, _Complex float *Cprod);
//void readCPLXfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername);
//int stream_conv_CPLX(_Complex float *ccinput, int datalength,_Complex float *filtercoeff, int filterlength, _Complex float *ccoutput);
//void correct_phase(float *phasein, int length, float *cphaseout);
void correct_phase(float *phasein, int length, float *cphaseout, int modephase, float Afreq);
//void complex_base_band(int *pint, int length, float freq, float phase);
//float get_gain(int dc_i, int dc_q);
float get_phase(double dc_i, double dc_q);
void computeGroupDelay(float *phasein, int length, float *cphaseout, float Afreq);

#endif
