/* 
 * Copyright (c) 2012
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
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


/* 
 * This file containts the functions related with the capture of module
 * parameter configuration from the external file "object_name.params".
 * 
 * ALL parameters to be captured must be listed in the parameters[] variable 
 * definition located at the module_params.h file
 */


#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <swapi_utils.h>
#include <phal_sw_api.h>

//Skeleton includes
#include "print_utils.h"
#include "paramsSK15.h"
#include "skeletonSK15.h"

//Module includes
#include "MOD_QAM_functions.h"


int get_config_params(MODparams_t *oParaml){

	/* Get control parameters value from modulename.params file*/
	// Params: local to module control parameters
	param_get_int("opMODE", &oParaml->opMODE);							//Initialized by hand or .params file
	param_get_int("modulation", &oParaml->modulation);			//Initialized by hand or .params file
	param_get_int("debugg", &oParaml->debugg);

	// Please, add here the variables to be setup from the external parameters file. 
	return(0);
}

extern char mname[];
extern const int nof_input_itf;
extern const int nof_output_itf;
extern const char* in_type;
extern const char* in_type;

void print_itfs(char *intype, char *outtype){

	printf(RESET BOLD T_BLACK "==============================================================================================\n");
	printf(RESET);
	printf(RESET BOLD T_BLACK		"    INTERFACES FOR: %s  \n", mname);
//	printf(RESET BOLD T_BLUE   			"      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
//		       												nof_input_itf, intype, nof_output_itf, outtype);
/*	printf(RESET BOLD T_BLACK "==============================================================================================\n");*/
}


void print_params(char *mnamel, MODparams_t oParaml){

//	printf(RESET BOLD T_WHITE "==============================================================================================\n");
	printf(RESET);
	printf(RESET BOLD T_LIGHTRED		"    INITIALIZED PARAMS FOR: %s  \n", mnamel);
	printf(RESET BOLD T_GREEN			"      opMODE=%d (0: Modulate, 1: Hard Demodulate, 2: Soft Demodulate)\n", 
																				oParaml.opMODE);
	printf(RESET BOLD T_GREEN			"      modulation=%d, 0:BPSK, 1:4QAM, 2:16QAM, 3:64QAM\n", 
																				oParaml.modulation);

	printf(RESET BOLD T_BLACK "==============================================================================================\n");
}
#define MODULATE					0
#define HARD_DEMODULATE		1
#define SOFT_DEMODULATE		2

/**
 * @check_config_params(). Check any value that should be checked and act accordingly before RUN phase
 * @param.
 * @oParaml Refers to the struct that containts the module parameters
 * @return: Void
 */
void check_config_params(MODparams_t *oParaml){

/*	if (oParaml->num_operations > MAXOPERATIONS) {
		printf("ERROR: p_num_operations=%d > MAXOPERATIONS=%d\n", oParaml->num_operations, MAXOPERATIONS);
		printf("Check your module_interfaces.h file\n");
		exit(EXIT_FAILURE);
	}
*/
}

