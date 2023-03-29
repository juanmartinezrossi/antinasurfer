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
#include "DUC17_functions.h"

int get_config_params(MODparams_t *oParaml){

	/* Get control parameters value from modulename.params file*/
	// Params: local to module control parameters
	param_get_int("opMODE", &oParaml->opMODE);											//Initialized by hand or .params file
	param_get_float("samplingfreqHz", &oParaml->samplingfreqHz);
	param_get_float("f0_freqHz", &oParaml->f0_freqHz);
	param_get_float("gain", &oParaml->gain);
	param_get_int("Ninterpol", &oParaml->Ninterpol);
	param_get_int("Ndecimate", &oParaml->Ndecimate);
	param_get_string("filtercoefffile", &oParaml->filtercoefffile[0]);
	// Please, add here the variables to be setup from the external parameters file. 
	return(0);
}

/**
 * @check_config_params(). Check any value that should be checked and act accordingly before RUN phase
 * @param.
 * @oParaml Refers to the struct that containts the module parameters
 * @return: Void
 */
void check_config_params(MODparams_t *oParaml){

	if (oParaml->Ninterpol > MAXINTERPOLATION) {
		printf("ERROR: Ninterpol=%d > MAXINTERPOLATION=%d\n", oParaml->Ninterpol, MAXINTERPOLATION);
		printf("ERROR: Interpolation level exceeding MAXINTERPOLATION\n");
		exit(EXIT_FAILURE);
	}
}

extern const int nof_input_itf;
extern const int nof_output_itf;
extern char mname[];

void print_itfs(char *intype, char *outtype){

	printf(RESET BOLD T_BLACK "==============================================================================================\n");
	printf(RESET);
	printf(RESET BOLD T_BLACK		"    INTERFACES FOR: %s  \n", mname);
	printf(RESET BOLD T_BLUE   			"      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       												nof_input_itf, intype, nof_output_itf, outtype);
/*	printf(RESET BOLD T_BLACK "==============================================================================================\n");*/
}

void print_params(char *mnamel, MODparams_t oParaml){

	printf(RESET);
	printf(RESET BOLD T_LIGHTRED		"    INITIALIZED PARAMS FOR: %s  \n", mnamel);
	printf(RESET BOLD T_GREEN			"      opMODE=%d, samplingfreqHz=%3.1f, f0_freqHz=%3.1f\n", oParaml.opMODE, oParaml.samplingfreqHz, oParaml.f0_freqHz);
	printf(RESET BOLD T_GREEN			"      gain=%3.1f, Ninterpol=%d, Ndecimate=%d\n", oParaml.gain, oParaml.Ninterpol, oParaml.Ndecimate);
	printf(RESET BOLD T_GREEN			"      Coeff File=%s\n", oParaml.filtercoefffile);
	printf(RESET BOLD T_BLACK "==============================================================================================\n");
}


