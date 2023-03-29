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
#include "paramsSK15.h"
#include "skeletonSK15.h"

//Module includes
#include "DAC_JACK1_functions.h"


int get_config_params(MODparams_t *oParaml){

	/* Get control parameters value from modulename.params file*/
	// Params: local to module control parameters
	param_get_int("opMODE", &oParaml->opMODE);							//Initialized by hand or .params file
//	param_get_int("datalength", &oParaml->datalength);					//Initialized by hand or .params file
	param_get_int("num_operations", &oParaml->num_operations);			//Initialized by hand or .params file
	param_get_float("constant", &oParaml->constant);
	param_get_string("datatext", &oParaml->datatext[0]);
	// Please, add here the variables to be setup from the external parameters file. 
	return(0);
}



