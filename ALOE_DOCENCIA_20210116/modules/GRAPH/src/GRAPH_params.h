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

/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */
/* Define the module control parameters & control structures and configure default values*/
/* Do not initialize here the extern variables*/
extern int run_times;
extern int block_length;
extern int numsampl2view;
extern int plot_mode;
extern float samplingfreqHz;
extern float fo_carrier;
extern int fftsize;
extern int bypass;
extern float mu;
extern int logscale;
extern int averagefft;
extern int plot_periodTS;
extern char inputtype[];
extern char outputtype[];
extern int captureDATA;
extern int TS2capture;
extern char output_FILE[STR_LEN];
extern char input_FILE[STR_LEN];
extern int readfileactive;
extern int fileFORMAT;	//0: REAL, IMAGi; 1: REAL+IMAGi


/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */
/* Configure the control parameters */
#define NOF_PARAMS	25

param_t parameters[] = {
				{"numsampl2view",INT,1},
				{"plot_mode",INT,1},
				{"samplingfreqHz",FLOAT,1},
				{"fo_carrier",FLOAT,1},
				{"fftsize",INT,1},
				{"bypass",INT,1},
				{"averagefft",INT,1},
				{"mu",FLOAT,1},
				{"logscale",INT,1},
				{"plot_periodTS",INT,1},
				{"inputtype",STRING,1},
				{"outputtype",STRING,1},
				{"input_FILE",STRING,1},
				{"readfilelength",INT,1},
				{"captureDATA",INT,1},
				{"TS2capture",INT,1},
				{"output_FILE",STRING,1},
				{"fileFORMAT",INT,1},
				{NULL, 0, 0} /* need to end with null name */
};

/* This number MUST be equal or greater than the number of parameters*/
const int nof_parameters = NOF_PARAMS;
/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */


#endif
