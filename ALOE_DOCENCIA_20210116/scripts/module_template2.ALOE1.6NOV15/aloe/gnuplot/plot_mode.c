#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Includes */
#include "gnuplot_i.h"
#include "complexplot.h"
#include "plot_mode.h"

#define PI 3.14159265359
#define FRAMELEN 1024
#define NUMFRAMES 10
#define PERIODS	 10.0
_Complex float data[FRAMELEN*NUMFRAMES];
gnuplot_ctrl *plot;

int plotcomplex(char *plotmode, complex_t* tmp_c, int datalen, gnuplot_ctrl *plot,
		char *itfname, SComplexplot* scomplexplot, int samplingfsHz, int runtimes) {

	char key[128];

	/* Complex plot structure initialization */
	memset(scomplexplot, 0, sizeof(SComplexplot));	// Set default values (all zeros)
//	printf("plotcomplex_PLOT_MODE\n");
//	printf("plotmode=%s, datalen=%d\n", plotmode, datalen);
//	printf("itfname=%s\n", itfname);
	strcpy(key, plotmode);
//	printf("plotmode=%s\n", key);

	if(strcmp(plotmode, "DEFAULT")==0){
		plotmode = "C1WLS";
		printf("Please, define according your needs\n");
	}
	else if(strcmp(plotmode, "C1WLS")==0){	//Complex, 1 Window, Linear, Single
		strcpy(scomplexplot->title, "Real - Imag Time Plot");			// Set the title
		scomplexplot->mode = REAL_IMAG;						// Ploting mode
		scomplexplot->domain = TIME;						// Domain, useful for x axis label
		scomplexplot->sampling_frequency = samplingfsHz; // Hz 			// Useful for x axis scaling
		scomplexplot->xscale = UNITY;						// Units for the x axis
		scomplexplot->mag_scale = LINEARSCALE;
		scomplexplot->frame_length = datalen; //FRAMELEN;			// Number of samples of each frame
		scomplexplot->multiple_frame = SINGLE;					// Set it multiple for a 3D plot for every frame inside the data
	}
	else if(strcmp(plotmode, "C2WLS")==0){	//Complex, 2 Window, Linear, Single
		strcpy(scomplexplot->title, "Real - Imag Time Plot");
		scomplexplot->mode = REAL + IMAG;					// Plot the imaginary and real part in different plots
		scomplexplot->domain = TIME;
		scomplexplot->sampling_frequency = samplingfsHz; // Hz
		scomplexplot->xscale = UNITY;
		scomplexplot->mag_scale = LINEARSCALE;
		scomplexplot->frame_length = datalen;	//FRAMELEN;
		scomplexplot->multiple_frame = SINGLE;
	}
	else if(strcmp(plotmode, "CFFTMF")==0){	//Complex, FFT, MultiFrame
		strcpy(scomplexplot->title, "Multi-Frame FFT (Spectral Power Density)");
		scomplexplot->mode = FFT_MAG;
		scomplexplot->domain = FREQUENCY;
		scomplexplot->sampling_frequency = samplingfsHz; // Hz
		scomplexplot->xscale = UNITY;
		scomplexplot->FFTorder = 16;
		scomplexplot->mag_scale = LOGSCALE;
		scomplexplot->frame_length = datalen/runtimes;		
		scomplexplot->multiple_frame = MULTIPLE;
	}
	else if(strcmp(plotmode, "CFFTSF")==0){ //Complex, FFT, Single Frame, 
		strcpy(scomplexplot->title, "Single Frame FFT (Spectral Power Density & Phase)");
		scomplexplot->mode = FFT_ARG + FFT_MAG;
		scomplexplot->domain = FREQUENCY;
		scomplexplot->FFTorder = 16;
		scomplexplot->sampling_frequency = samplingfsHz; // Hz
		scomplexplot->frame_length = datalen;
		scomplexplot->xscale = UNITY;
		scomplexplot->arg_unit = RADIANS; //DEGREES;
		scomplexplot->multiple_frame = SINGLE;
		scomplexplot->mag_scale = LOGSCALE;

	}else if(strcmp(plotmode, "CCSF")==0){ //Complex, Constellation, Single Frame, 
		strcpy(scomplexplot->title, "Constellation");		//- Vector diagramm Example
		scomplexplot->mode = CONSTELLATION; // + VECTOR;
		scomplexplot->xscale = UNITY;
		scomplexplot->multiple_frame = SINGLE; //MULTIPLE;
		scomplexplot->frame_length = FRAMELEN;
		scomplexplot->autoscale = AUTOSCALE;
		//scomplexplot->xmin = -1.0;
		//scomplexplot->xmax = 1.0;
		//scomplexplot->ymin = -1.0;
		//scomplexplot->ymax = 1.0;
	}

	return 0;
}

