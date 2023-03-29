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
#include "LTEturboCOD_functions.h"

/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */
/* Define the module control parameters & control structures and configure default values*/
/* Do not initialize here the extern variables*/
/*extern int opmode;
extern int rcvbitsSubframe;
extern sframeBits_t sframeBITS;
extern char datatypeIN[];
extern int Numiterations;
extern int datalength;
extern int modeRS;
*/
/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */
/* Configure the control parameters */
#define NOF_PARAMS	10

param_t parameters[] = {
		{"opmode", 	/* Parameter name*/
		  INT,			/* Parameter type (see skeleton.h) */
		  1},			/* Number of samples (of size the type) */
		{"CodeRate",FLOAT,1},
		{"datatypeIN",STRING,1},
		{"DECODIterations",INT,1},
		{"datalength",INT,1},
		{"modeRS",INT,1},
		{"UpDownLink",INT,1},
		{"MQAMOrder",INT,1},
		{NULL, 0, 0} /* need to end with null name */
};

/* This number MUST be equal or greater than the number of parameters*/
const int nof_parameters = NOF_PARAMS;
/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */


#endif
