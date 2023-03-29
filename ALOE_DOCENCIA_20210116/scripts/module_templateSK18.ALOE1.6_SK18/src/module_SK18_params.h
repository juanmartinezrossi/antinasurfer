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
#include "module_templateSK18_functions.h"

extern MODparams_t oParam;

char kk[32];
// ADD THE TEXT YOU WISH
char text0[100] = "opMODE: 0=DEBUGG, 1=GENERATOR, 2=BYPASS, 3=NORMAL";
char text1[100] = "opMODE: 0=DEBUGG, 1=GENERATOR, 2=BYPASS, 3=NORMAL";

/** Fill the structure with the parameters you would like to setup during INIT phase			*/
/** These parameters are automatically loaded if they included in the following list and 	*/
/** they are found in the .params file 												
														*/

struct utils_variables my_vars[] = {

//		NORMAL PARAMETER FORMAT
//		{"opMODE", STAT_TYPE_INT, 1, &oParam.opMODE, READ},	

//      TEXT EDITION PARAMETERS
//		{"TEXTN", STAT_TYPE_CHAR, 1, text0, READ},					ADD NORMAL TEXT LINE
//		{"LINEBREAK", STAT_TYPE_CHAR, 1, &kk[0], READ},				ADD LINE BREAK
//		{"TEXTC", STAT_TYPE_CHAR, 1, text1, READ},					ADD COLORED TEXT LINE (RED)


		{"opMODE",						/** variable name */
		STAT_TYPE_INT,					/** type */
		1,								/** size, in number of type elements */
		&oParam.opMODE,					/** pointer to the variable */
		READ},							/** Automatic mode:
											OFF: Do nothing
											READ: Obtains the value at the begining of the timeslot.
												  It is continously read from .params file???
											WRITE: Sets the value at the end of the timeslot
										*/
		{"datalength", STAT_TYPE_INT, 1, &oParam.datalength, READ},
		{"num_operations", STAT_TYPE_INT, 1, &oParam.num_operations, READ},
		/** end */
		{NULL, 0, 0, 0, 0}};

/** ISSUES:
			1) float values (STAT_TYPE_FLOAT) are not properly readed from .params file if contain negative values.
			2) _Complex float values (STAT_TYPE_COMPLEX) are not properly readed from .params file.
			3) user_defined_t values (STAT_TYPE_CUSTOM) are not properly readed from .params file.
*/

#endif


