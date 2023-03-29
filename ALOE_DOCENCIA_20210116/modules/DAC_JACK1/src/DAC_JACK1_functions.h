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

#include <jack/jack.h>
// Module Variables and Defines

#define SLENGTH			32
#define DEBUGG			0
#define GENERATOR		1
#define BYPASS			2
#define NORMAL			3

#define MAXOPERATIONS	20000

typedef struct MODparams{
    int opMODE;
	int datalength;
    int num_operations;
    float constant;
    char datatext[SLENGTH];
}MODparams_t;

static MODparams_t oParam={BYPASS, 256, 100, 7.9, "BABY"};		// Initialize module params struct


#define TABLE_SIZE   (200)
typedef struct
{
    float sine[TABLE_SIZE];
    int left_phase;
    int right_phase;
}paTestData;

#define MAXJ	100
typedef jack_default_audio_sample_t sample_t;
/*************************************************************************************************/
// Functions Predefinition
static void signal_handler(int sig);
void stop_jack();
void jack_shutdown (void *arg);
void audiodac_output_test(sample_t *out1, sample_t *out2, jack_nframes_t nframes, paTestData *data);
int process (jack_nframes_t nframes, void *arg);
int jack_init();


int init_functionA_COMPLEX(_Complex float *input, int length);
int functionA_COMPLEX(_Complex float *input, int lengths, _Complex float *output);
int init_functionB_FLOAT(float *input, int length);
int functionB_FLOAT(float *input, int lengths, float *output);
int functionC_INT(int *in, int length, int *out);
int functionD_CHAR(char *in, int length, char *out);
int bypass_func(char *datatypeIN, char *datatypeOUT, void *datin, int datalength, void *datout);
void check_config_params(MODparams_t *oParaml);
void print_params(char *mnamel, MODparams_t oParaml);
#endif
