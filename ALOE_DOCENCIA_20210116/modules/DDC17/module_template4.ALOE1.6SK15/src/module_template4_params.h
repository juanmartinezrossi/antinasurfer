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

#ifndef _MOD_PARAMS_H
#define _MOD_PARAMS_H
#include <swapi_utils.h>
#include <phal_sw_api.h>
#include "module_template4_functions.h"

/*////////////////////////////////////////////////////////////////////////////////////////////////////*/

/* PARAMS: Set of variables that can be set from external configuration file during Init phase /////*/
#define NOF_PARAMS	10
extern param_t parameters[] = {
		{"opMODE", 				/* Parameter name*/
		 STAT_TYPE_INT,			/* Parameter type (see skeleton.h) */
		  1},					/* Number of samples (of size the type) */
		{"num_operations",STAT_TYPE_INT,1},
		{"datatext",STAT_TYPE_STRING,1},
		{"constant",STAT_TYPE_FLOAT,1},
		{NULL, 0, 0} /* need to end with null name */
};

/*////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* DO NOT MODIFY BEYOND THIS LINE**********************************************************************/
#define NOF_EXTRA_VARS	5				//Define the current number of Stats published by this module

const int nof_parameters=NOF_PARAMS;
const int nof_extra_vars = NOF_EXTRA_VARS;

int get_config_params(MODparams_t *oParam);
#endif
/*////////////////////////////////////////////////////////////////////////////////////////////////////*/

