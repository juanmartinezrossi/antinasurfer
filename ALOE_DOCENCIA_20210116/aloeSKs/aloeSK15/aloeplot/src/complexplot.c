/*
 * complexplot.c
 *
 *  Created on: Mar 22, 2013
 *      Author: Xaxier Arteaga
 */

/*---------------------------------------------------------------------------
                                Includes
---------------------------------------------------------------------------*/

#include <complex.h>
#include <math.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gnuplot_i.h"				/* Interface header to the gnuplot source*/
#include "complexplot.h"			/* Interface header and defined constants for this source*/


#define PI 		3.14159265359
#define FRAMELEN 	1024
#define NUMFRAMES 	10
#define PERIODS	 	10.0
#define LEGENLEN	128

/*-------------------------------------------------------------------------*/
/**
  @brief    Subroutine to setup a Real-Imag Time Plot in different plots.
  @param    scomplexplot pointer to the plot configuration structure.
  @return   void
 
*/
/*-------------------------------------------------------------------------*/
void realimag2timelinearsingleplots(SComplexplot* scomplexplot, float samplingfsHz) {
	strcpy(scomplexplot->title, "Real - Imag Time Plot");
	scomplexplot->mode = REAL + IMAG;					/* Plot the imaginary and real part in different plots*/
	scomplexplot->domain = TIME;
	scomplexplot->sampling_frequency = samplingfsHz; 	/* Hz*/
	scomplexplot->xscale = UNITY;
	scomplexplot->FFTorder = 15;
	scomplexplot->mag_scale = LINEARSCALE;
	scomplexplot->frame_length = FRAMELEN;
	scomplexplot->multiple_frame = SINGLE;
	scomplexplot->arg_unit = DEGREES;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Subroutine for convert a linear value to decibel units.
  @param    val Value to convert into logarithmic scale.
  @return   double with the result of the conversion
  
  This subroutine converts a linear value to a power value in decibels.
  It is equivalent to out = 20·log10(val).
*/
/*-------------------------------------------------------------------------*/
/*/inline double lin2dB(double val) {*/
double lin2dB(double val) {
	return 20*log10(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Sets the environment of the plot.
  @param    plot Gnuplot session control handle.
  @param    scomplexplot pointer to the plot configuration structure.
  @param    datatype spicifies the kind of data of the plot.
  @return   void.
                       
  Sets the environment of the plot. It sets the title, labels, background
  colors, units, etc.
*/
/*--------------------------------------------------------------------------*/
/*inline void set_environment (gnuplot_ctrl *plot, SComplexplot* scomplexplot, int datatype){*/
void set_environment (gnuplot_ctrl *plot, SComplexplot* scomplexplot, int datatype){

	/* Set title */
	gnuplot_set_title(plot, scomplexplot->title);

	/*
	 * If constellation or vector diagram.
	 */
	if (scomplexplot->mode == CONSTELLATION || scomplexplot->mode == VECTOR) {
		/* Set background color */
		gnuplot_set_background(plot, "#00FFFF");
		/* If the data is only in one Frame */
		if (scomplexplot->multiple_frame == SINGLE){
			gnuplot_set_xlabel(plot, "In-Phase");
			gnuplot_set_ylabel(plot, "Quadrature");
		}
		/* If the data is composed by multiple frames */
		else if (scomplexplot->multiple_frame == MULTIPLE) {
			gnuplot_set_xlabel(plot, "In-Phase");
			gnuplot_set_zlabel(plot, "Quadrature");
			gnuplot_set_ylabel(plot, "Frame");
		}
		
		/* Set x-y range */
		if (scomplexplot->autoscale == NOAUTOSCALE){
			gnuplot_set_xy_range(plot, scomplexplot->xmin, scomplexplot->xmax, \
		               	scomplexplot->ymin, scomplexplot->ymax);
		}
		
		/* Exit */
		return;
	} 

	/* Time Domain */
	if (scomplexplot->domain == TIME){
		/* Set background color */
		gnuplot_set_background(plot, "gold"); /* "#999999");//"lemonchiffon");//"#ccffcc");*/
/*		gnuplot_line_color(plot, "#ccffcc");*/

		/* Set x label title */
		if (scomplexplot->xscale == MICRO){
			gnuplot_set_xlabel(plot, "Time [us]");
		} else if (scomplexplot->xscale == MILI) {
			gnuplot_set_xlabel(plot, "Time [ms]");
		} else if (scomplexplot->xscale == UNITY) {
			gnuplot_set_xlabel(plot, "Time [s]");
		} else if (scomplexplot->xscale == KILO) {
			gnuplot_set_xlabel(plot, "Time [ks]");
		} else if (scomplexplot->xscale == MEGA) {
			gnuplot_set_xlabel(plot, "Time [Ms]");
		}

	/* Frequency Domain */
	} else if (scomplexplot->domain == FREQUENCY){
		/* Set background color */
		gnuplot_set_background(plot, "greenyellow");

		/* Set x label title */
		if (scomplexplot->xscale == MICRO){
			gnuplot_set_xlabel(plot, "Frequency [uHz]");
		} else if (scomplexplot->xscale == MILI) {
			gnuplot_set_xlabel(plot, "Frequency [mHz]");
		} else if (scomplexplot->xscale == UNITY) {
			gnuplot_set_xlabel(plot, "Frequency [Hz]");
		} else if (scomplexplot->xscale == KILO) {
			gnuplot_set_xlabel(plot, "Frequency [kHz]");
		} else if (scomplexplot->xscale == MEGA) {
			gnuplot_set_xlabel(plot, "Frequency [MHz]");
		}
	}
	/* If the plot is 3D (Multiple frames) */
	if (scomplexplot->multiple_frame == MULTIPLE) {

		if (datatype == MAGNITUDE){
			if (scomplexplot->mag_scale == LOGSCALE) {
				gnuplot_set_zlabel(plot, "[dB]");				/*"Magnitude [dB]"*/
			} else { 
				gnuplot_set_zlabel(plot, "[linear]");			/*"Magnitude [linear]"*/
			}
		} else if (datatype == ARGUMENT){
			if (scomplexplot->arg_unit == RADIANS) {
				gnuplot_set_zlabel(plot, "Phase [rad]");
			} else {
				gnuplot_set_zlabel(plot, "Phase [º]");
				return;
			}
		}
		gnuplot_set_ylabel(plot, "Number of Frame");

	/* If it is 2D plot (Single Frame) */
	}else {
		/* Set background color */
/*		gnuplot_set_background(plot, "azure");*/

		if (datatype == MAGNITUDE){
			if (scomplexplot->mag_scale == LOGSCALE) {
				gnuplot_set_ylabel(plot, "Magnitude [dB]");
			} else if (scomplexplot->mag_scale == LINEARSCALE) {
				gnuplot_set_ylabel(plot, "Magnitude [lin]");
			}
		} else if (datatype == ARGUMENT){
			if (scomplexplot->arg_unit == RADIANS) {
				gnuplot_set_ylabel(plot, "Phase [rad]");
			} else {
				gnuplot_set_ylabel(plot, "Phase [º]");
			}
		}
	}
	
	/* Set plot range if configured as no autoscale */
	if (scomplexplot->autoscale == NOAUTOSCALE){
		gnuplot_set_xy_range(plot, scomplexplot->xmin, scomplexplot->xmax, \
			scomplexplot->ymin, scomplexplot->ymax);
	}
	
	/* Subroutine end */
	return;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Subroutine which plots any data stored in 1-D.
  @param    plot_buff_y pointer to the data array in double* format.
  @param    datalen number of samples to plot.
  @param    plot Pointer to the ploting session object.
  @param    itfname interface name which is bing ploted.
  @param    scomplexplot data sctructure with the information about the plot properties.
  @param    datatype Type of data which is being plot. Argument or magnitude.
  @return   void It does not return any value.
  
  This subroutine plot a graph from any real data stored in 1-D. Also adds the labels automatically.

*/
/*-------------------------------------------------------------------------*/
void plot_data (double* plot_buff_y,
		int datalen,
		gnuplot_ctrl *plot,
		char *itfname,
		SComplexplot* scomplexplot,
		int datatype
	) {
	int j;

	double* plot_buff_x = malloc(sizeof(double)*datalen);
	double* plot_buff_z = (double*) malloc(sizeof(double)*datalen);

	int frame_length = scomplexplot->frame_length;
	double sampling_frequency = scomplexplot->sampling_frequency;
	double xscale = scomplexplot->xscale;

	double inc_x;	/* X-Axis increment*/

	/* Calculate the x-axis incremental value */
	if (scomplexplot->domain == FREQUENCY)
		inc_x = sampling_frequency/frame_length/xscale;
	else
		inc_x = xscale*1.0/sampling_frequency;

	/* Calculate x-axis values and y-axis values */
	for (j = 0; j < datalen; j++) {
		plot_buff_x [j] = (double)(j%frame_length) * inc_x;
	}

	/* Common code */
	plot = gnuplot_init() ;
	gnuplot_setstyle(plot,"lines");
	set_environment (plot, scomplexplot, datatype);

	/* Single frame Plot, 2-D Plot */
	if (scomplexplot->multiple_frame == SINGLE){
		/* 2-D Plot */
		gnuplot_plot_xy(plot, plot_buff_x, plot_buff_y, datalen, itfname);
	}
	/* Multiple frame plot, 3-D Plot */
	else {
		/* Generate z-axis values */
		for (j=0;j<datalen;j++){
			plot_buff_z[j] = (double)(j/frame_length);
		}
		/* 3-D Plot */
		gnuplot_plot_xyz(plot, plot_buff_x, plot_buff_z, plot_buff_y, datalen, itfname);
	}
	/* Free memory */
	free(plot_buff_x);
	free(plot_buff_z);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Subroutine which plots the real and imaginary parts on the same plot.
  @param    tmp_c Array with the temporary data to plot.
  @param    datalen Number of samples to plot.
  @param    plot Pointer to the ploting session object.
  @param    itfname interface name which is bing ploted.
  @param    scomplexplot data sctructure with the information about the plot properties.
  @return   void It does not return any value.
  
  This subroutine plots the input complex data in to one plot. It plots the complex and
  imaginary data in the same plot (two different graphs).
*/
/*-------------------------------------------------------------------------*/
void plot_real_imag (	complex_t* tmp_c,
			int datalen,
			gnuplot_ctrl *plot,
			char *itfname,
			SComplexplot* scomplexplot
		) {
	int j;
	char tmp[64];

	double* plot_buff_re = malloc(sizeof(double)*datalen);
	double* plot_buff_im = malloc(sizeof(double)*datalen);
	double* plot_buff_x = malloc(sizeof(double)*datalen);
	double* plot_buff_z = malloc(sizeof(double)*datalen);

	int frame_length = scomplexplot->frame_length;
	double sampling_frequency = scomplexplot->sampling_frequency;

	for (j=0;j<datalen;j++) {
		plot_buff_re[j] = (double) crealf(tmp_c[j]);
		plot_buff_im[j] = (double) cimagf(tmp_c[j]);
		plot_buff_x [j] = scomplexplot->xscale*(double)j/scomplexplot->sampling_frequency;
	}
	if (scomplexplot->multiple_frame == SINGLE){
		/* Single frame Plot */
		plot = gnuplot_init() ;
		gnuplot_setstyle(plot,"lines");
		snprintf(tmp,64,"%s Real ", itfname);
		set_environment (plot, scomplexplot, 0);
		gnuplot_plot_xy(plot, plot_buff_x, plot_buff_re, datalen, tmp);	/*"Real");*/
		snprintf(tmp,64,"%s Imag", itfname);
		gnuplot_plot_xy(plot, plot_buff_x, plot_buff_im, datalen, tmp);	/*"Imaginary");*/
	} else {
		/* Multiframe plot */
		/* Generate buffers x and z */
		for (j=0;j<datalen;j++){
			plot_buff_z[j] = (float)(j/frame_length);
		}
		if (scomplexplot->domain == FREQUENCY){
			/* Generate frequency domain for x*/
			for (j=0;j<datalen;j++){
				plot_buff_x[j] = sampling_frequency*(float)(j%frame_length)/(float)frame_length;
			}
		} else {
			/* Generate Time domain for x */
			for (j=0;j<datalen;j++){
				plot_buff_x[j] = (float)(j%frame_length)/sampling_frequency;
			}
		}
		plot = gnuplot_init() ;
		gnuplot_setstyle(plot,"lines");
		snprintf(tmp,64,"%s Real ", itfname);
		set_environment (plot, scomplexplot, 0);
		gnuplot_plot_xyz(plot, plot_buff_x, plot_buff_z, plot_buff_re, datalen, tmp);

		plot = gnuplot_init() ;
		gnuplot_setstyle(plot,"lines");
		snprintf(tmp,64,"%s Imag", itfname);
		set_environment (plot, scomplexplot, 1);
		gnuplot_plot_xyz(plot, plot_buff_x, plot_buff_z, plot_buff_im, datalen, tmp);
	}
	/* Free memory */
	free(plot_buff_re);
	free(plot_buff_im);
	free(plot_buff_x);
	free(plot_buff_z);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Subroutine which plots the data with a  constellation diagram
  @param    tmp_c Array with the temporary data to plot.
  @param    datalen Number of samples to plot.
  @param    plot Pointer to the ploting session object.
  @param    itfname interface name which is bing ploted.
  @param    scomplexplot data sctructure with the information about the plot properties.
  @return   void It does not return any value.

  This subroutine plots the input complex data in to one plot. It plots the real and
  imaginary data in the same plot using a constellation diagram.
*/
/*-------------------------------------------------------------------------*/
void plot_constellation (complex_t* tmp_c, int datalen, gnuplot_ctrl *plot, char *itfname, SComplexplot* scomplexplot) {
	int j;
	char tmp[64];

	double* plot_buff_re = malloc(sizeof(double)*datalen);
	double* plot_buff_im = malloc(sizeof(double)*datalen);
	double* plot_buff_z = malloc(sizeof(double)*datalen);

	for (j=0;j<datalen;j++) {
		plot_buff_re[j] = (double) crealf(tmp_c[j]);
		plot_buff_im[j] = (double) cimagf(tmp_c[j]);
	}

	plot = gnuplot_init() ;
	gnuplot_setstyle(plot,"x");
	snprintf(tmp,64,"%s Real/Imag %d", itfname, 0);
	set_environment (plot, scomplexplot, 0);

	if (scomplexplot->multiple_frame == SINGLE) {
		gnuplot_plot_xy(plot, plot_buff_re, plot_buff_im, datalen, tmp);
	} else {
		/* Generate buffer x */
		for (j=0;j<datalen;j++){
			plot_buff_z[j] = (float)(j/scomplexplot->frame_length);
		}
		gnuplot_plot_xyz(plot, plot_buff_re, plot_buff_z, plot_buff_im, datalen, tmp);
	}
	
	/* Free memory */
	free(plot_buff_re);
	free(plot_buff_im);
	free(plot_buff_z);

}

/*-------------------------------------------------------------------------*/
/**
  @brief    Subroutine which plots the data with a  vector diagram
  @param    tmp_c Array with the temporary data to plot.
  @param    datalen Number of samples to plot.
  @param    plot Pointer to the ploting session object.
  @param    itfname interface name which is bing ploted.
  @param    scomplexplot data sctructure with the information about the plot properties.
  @return   void It does not return any value.

  This subroutine plots the input complex data in to one plot. It plots the real and
  imaginary data in the same plot using a vector diagram.
*/
/*-------------------------------------------------------------------------*/
void plot_vector (complex_t* tmp_c, int datalen, gnuplot_ctrl *plot, char *itfname, SComplexplot* scomplexplot) {
	int j;
	char tmp[64];

	double* plot_buff_re = malloc(sizeof(double)*datalen);
	double* plot_buff_im = malloc(sizeof(double)*datalen);
	double* plot_buff_z = malloc(sizeof(double)*datalen);

	for (j=0;j<datalen;j++) {
		plot_buff_re[j] = (double) crealf(tmp_c[j]);
		plot_buff_im[j] = (double) cimagf(tmp_c[j]);
	}

	plot = gnuplot_init() ;
	gnuplot_setstyle(plot,"lines");
	snprintf(tmp,64,"%s_real_%d", itfname, 0);
	set_environment (plot, scomplexplot, 0);

	if (scomplexplot->multiple_frame == SINGLE) {
		gnuplot_plot_xy(plot, plot_buff_re, plot_buff_im, datalen, tmp);
	} else {
		/* Generate buffer x */
		for (j=0;j<datalen;j++){
			plot_buff_z[j] = (float)(j/scomplexplot->frame_length);
		}
		gnuplot_plot_xyz(plot, plot_buff_re, plot_buff_z, plot_buff_im, datalen, tmp);
	}
	/* Free memory */
	free(plot_buff_re);
	free(plot_buff_im);
	free(plot_buff_z);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Subroutine which computes the FFT and recall the complexplot
  	  	  	  subroutine.
  @param    tmp_c Array with the temporary data to plot.
  @param    datalen Number of samples to plot.
  @param    plot Pointer to the ploting session object.
  @param    itfname interface name which is bing ploted.
  @param    scomplexplot data sctructure with the information about the plot
  	  	  	  properties.
  @return   void It does not return any value.

  This subroutine compute the FFT of the input data and recall the complexplot
  function for plot its module or argument.
*/
/*-------------------------------------------------------------------------*/
void plot_fft (	complex_t* data, int datalen, gnuplot_ctrl *plot, char *itfname, SComplexplot* scomplexplot)
{
	int i, j;
	int nffts;
	char    legend[LEGENLEN];
	double * temp_f;
	temp_f = malloc(sizeof(double)*datalen);
	int fftsize = 1 << scomplexplot->FFTorder;
	fftw_plan fft_plan;

	complex * temp_c;
	complex * temp_out;
	complex_t* temp_plot;

/*	SComplexplot temp_scomplexplot;*/

/*	printf("fftsize=%d, frame_length=%d\n", fftsize, scomplexplot->frame_length);*/

	if (fftsize < scomplexplot->frame_length){
		printf("ERROR in graph %s, %s; The FFT order (%d) is less than expected log2(%d)\n", 
			scomplexplot->title, itfname, scomplexplot->FFTorder, scomplexplot->frame_length);
		return;
	} else if (scomplexplot->FFTorder > MAXFFTORDER) {
		printf("ERROR in graph %s, %s; The FFT order (%d) exceeds the maximum (%d)\n", 
			scomplexplot->title, itfname, scomplexplot->FFTorder, MAXFFTORDER);
		return;
	}

	if (scomplexplot->multiple_frame == MULTIPLE){
		nffts = datalen/scomplexplot->frame_length;
	} else {
		nffts = 1;
	}
	/* Allocate needed memory */
	temp_c = malloc(sizeof(complex)*fftsize);
	temp_out = malloc(sizeof(complex)*fftsize);
	temp_plot = malloc(sizeof(double)*nffts*fftsize);

	/* Creates fft plan */
	fftsize=scomplexplot->frame_length;
	fft_plan = fftw_plan_dft_1d(fftsize, temp_c, temp_out, FFTW_FORWARD, FFTW_ESTIMATE);

	/* Calculates FFTs */
	for (i=0; i<nffts; i++){
		/* Set zeros the input array */
		memset(temp_c, 0, sizeof(complex)*fftsize);

		/* Put data into the input FFT array */
		for (j=0; j<scomplexplot->frame_length; j++)
			temp_c[j] = data[j+i*scomplexplot->frame_length];

		/* Perform FFT */
		fftw_execute(fft_plan);

		/* Store the FFT result */
		for (j=0; j<fftsize; j++)
			if(j!=(fftsize-1))temp_plot[i*fftsize + j] = temp_out[j];
	}
	/* Modify scomplexplot properties according to the fft plot  */
	scomplexplot->domain = FREQUENCY;

	/* Call ploting function */
	/* Plot magnitude of the data */
	if ((scomplexplot->mode & FFT_MAG) == FFT_MAG){
		scomplexplot->mode += MAG;
		if (scomplexplot->mag_scale == LOGSCALE) {
			for (i=0; i<datalen; i++)
				temp_f[i] = lin2dB(cabs(temp_plot[i]));
		} else {
			for (i=0; i<datalen; i++)
				temp_f[i] = cabs(temp_plot[i]);
		}
		sprintf(legend, "%s Magnitude", itfname);
		plot_data(temp_f, fftsize*nffts, plot, legend, scomplexplot, MAGNITUDE);
	}

	/* Plot argument of the data */
	if ((scomplexplot->mode & FFT_ARG) == FFT_ARG){
		scomplexplot->mode += ARG;
		double arg_scale = (scomplexplot->arg_unit==DEGREES)?(180/PI):1;
		for (i=0; i<datalen; i++){
			temp_f[i] = arg_scale*cargf(temp_plot[i]);
		}
		sprintf(legend, "%s Phase", itfname);
		plot_data(temp_f, datalen, plot, legend, scomplexplot, ARGUMENT);
	}

	fftw_destroy_plan(fft_plan);
	free(temp_c);
	free(temp_out);
	free(temp_plot);
}

void complexplot (complex_t* tmp_c, int datalen, gnuplot_ctrl *plot, char *itfname, SComplexplot* scomplexplot){
	int i;
	double * temp_f;
	unsigned int mode = scomplexplot->mode;
	temp_f = malloc(sizeof(double)*datalen);	/* Allocate data*/
	char    legend[LEGENLEN];

/*	printf("COMPLEX_PLOT\n");*/

	/* Check x-scale */
	if (	scomplexplot->xscale != MICRO && \
			scomplexplot->xscale != MILI && \
			scomplexplot->xscale != UNITY && \
			scomplexplot->xscale != KILO && \
			scomplexplot->xscale != MEGA ){
		printf("Warning! x-scale is not well defined (%f). Setting Unity (1.0) as default.\n", scomplexplot->xscale);
		printf("X-Scale Compatible types: 0.000001 (MICRO), 0.001 (MILI), 1.0 (UNITY), 1000.0 (KILO) and 1000000.0 (MEGA)\n");
		scomplexplot->xscale = UNITY;
	}

	/* Check multiframe */
	if (scomplexplot->multiple_frame == MULTIPLE && \
			( \
					scomplexplot->frame_length <= 0 || \
					datalen%scomplexplot->frame_length != 0 \
			) \
		){
		printf("ERROR! The frame length (%d) has a bad configuration.\n", \
				scomplexplot->frame_length);
		return;
	}else if (scomplexplot->multiple_frame == SINGLE) {
		/*printf("Warning! The frame length has been configured as 0 and it is multiframe");*/
		scomplexplot->frame_length = datalen;
	}

	/* Compute FFT */
	if ((mode & FFT_MAG) == FFT_MAG || (mode & FFT_ARG) == FFT_ARG){
		plot_fft (tmp_c, datalen, plot, itfname, scomplexplot);
	}

	/* Plot real data */
	if ((mode & REAL) == REAL){
		for (i=0; i<datalen; i++)
			temp_f[i] = creal(tmp_c[i]);
		sprintf(legend, "%s Real", itfname);
		plot_data(temp_f, datalen, plot, legend, scomplexplot, MAGNITUDE);
	}

	/* Plot imaginary data */
	if ((mode & IMAG) != 0){
		for (i=0; i<datalen; i++)
			temp_f[i] = cimag(tmp_c[i]);
		sprintf(legend, "%s Imag", itfname);
		plot_data(temp_f, datalen, plot, legend, scomplexplot, MAGNITUDE);
	}

	/* Plot real and imaginary data on same graph */
	if ((mode & REAL_IMAG) == REAL_IMAG){
		plot_real_imag (tmp_c, datalen, plot, itfname, scomplexplot);
	}

	/* Plot magnitude of the data */
	if ((mode & MAG) == MAG){
		if (scomplexplot->mag_scale == LOGSCALE) {
			for (i=0; i<datalen; i++)
				temp_f[i] = lin2dB(cabs(tmp_c[i]));
		} else {
			for (i=0; i<datalen; i++)
				temp_f[i] = cabs(tmp_c[i]);
		}

		plot_data(temp_f, datalen, plot, "Magnitude", scomplexplot, MAGNITUDE);
	}

	/* Plot magnitude of the data */
	if ((mode & ARG) == ARG){
		double arg_scale = (scomplexplot->arg_unit==DEGREES)?(180/PI):1;
		for (i=0; i<datalen; i++)
			temp_f[i] = arg_scale*cargf(tmp_c[i]);

		plot_data(temp_f, datalen, plot, "Phase", scomplexplot, ARGUMENT);
	}

	if ((mode & CONSTELLATION) == CONSTELLATION){
		plot_constellation (tmp_c, datalen, plot, itfname, scomplexplot);
	}
	
	if ((mode & VECTOR) == VECTOR){
		plot_vector(tmp_c, datalen, plot, itfname, scomplexplot);
	}
/*	printf("COMPLEX_PLOT END\n");*/
	free(temp_f); /* Free allocated data*/
	return;
}


