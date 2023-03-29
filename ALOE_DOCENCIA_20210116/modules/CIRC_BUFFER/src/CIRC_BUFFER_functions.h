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

#define BYPASS			1
#define NORMAL			2

#define MAXOPERATIONS	20000

typedef struct MODparams{
    int opMODE;
	int expectedinputlength;
    int outlength;
}MODparams_t;

static MODparams_t oParam={NORMAL, 1793, 254};		// Initialize module params struct


/*************************************************************************************************/
// Functions Predefinition
int init_functionA_COMPLEX(_Complex float *input, int length);
int functionA_COMPLEX(_Complex float *input, int lengths, _Complex float *output);
int init_functionB_FLOAT(float *input, int length);
int functionB_FLOAT(float *input, int lengths, float *output);
int functionC_INT(int *in, int length, int *out);
int functionD_CHAR(char *in, int length, char *out);
int bypass_func(char *datatypeIN, char *datatypeOUT, void *datin, int datalength, void *datout);
void check_config_params(MODparams_t *oParaml);
void print_params(char *mnamel, MODparams_t oParaml);

typedef struct buffer{
	int readIndex;
	int writeIndex;
	int buffsize;
	int occuplevel;	//Occupancy level of buffer for reading
	int roomlevel;	//Available room in buffer for writing
}buffctrl;

void initCbuff(buffctrl *bufferCtrl, _Complex float *bufferC, int buffersz, int firstpcketsz);
int writeCbuff(buffctrl *buffer, _Complex float *buffdata, _Complex float *in, int length);
int readCbuff(buffctrl *buffer,_Complex float *buffdata, _Complex float *out, int length);
int lookCbuff(buffctrl* buffer,_Complex float *buffdata, _Complex float *out, int length);


#endif
