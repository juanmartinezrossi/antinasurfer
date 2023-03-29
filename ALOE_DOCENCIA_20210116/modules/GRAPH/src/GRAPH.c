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
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <fftw3.h>
#include <stdlib.h>
#include <string.h>


#include "gnuplot_i.h"
#include <sys/time.h>
#include <time.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "GRAPH_interfaces.h"
#include "GRAPH_functions.h"
#include "GRAPH.h"


//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="GRAPH";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
#define BACKGROUNDLEN	90
//char stringplot[STR_LEN]="O--------------------------------------------------------------------------------------------O";
char background[BACKGROUNDLEN]="";
char backgroundREF[BACKGROUNDLEN]="                                                                                              ";
char stringplot[BACKGROUNDLEN]="";
char auxchar[BACKGROUNDLEN]="";
float samplingfreqHz=1.0;
float fo_carrier=0.0;

//Module User Defined Parameters
int bypass=0; //0: No Bypass, 1: Bypass

int plot_mode=0x0; 		/* 	0x0: None;
							0x1: Time;
							0x2: Spectrum;
							0x4: Constellation
						*/
int captureDATA=0; 	// 0: No Capture, 1: Text Data Files, 2: Bin Data Files

//Time parameters
int inlength=0;
//Sepectrum
#define MAXFFT	8192*2
fftw_plan fft_plan;
int fftsize=1024*4;
int numsampl2view;
int averagefft=0;		/* 0: FFT non averaged; 1: FFT averaged*/
float mu=0.9;			/* Persistence: Contribution of history in current average*/
#define PLOT_PERIODTS		1
int plot_periodTS=PLOT_PERIODTS;


fftw_complex fftbuff_IN[INPUT_MAX_DATA*PLOT_PERIODTS];
fftw_complex fftbuff_OUT[INPUT_MAX_DATA*PLOT_PERIODTS];
int logscale=0;			/* 0: Linear scale; 1: Log Scale*/
double arg_scale;

struct timespec tdata[3];

char inputtype[STR_LEN]=IN_TYPE;
char outputtype[STR_LEN]=IN_TYPE;

//Global Variables
// Plotting
#define MINDOUBLE		0.0000000000000000001
#define MAXDOUBLE		1000000000000000000000.0
double plot_real[INPUT_MAX_DATA];
double plot_imag[INPUT_MAX_DATA];
complex double fftDATA[INPUT_MAX_DATA];
double module[INPUT_MAX_DATA];
double module_aux[INPUT_MAX_DATA];
double aux1[MAXFFT];
double aux2[MAXFFT];
double arg[INPUT_MAX_DATA];
double arg_aux[INPUT_MAX_DATA];
double xTIME_axis[INPUT_MAX_DATA];
double xFFT_axis[INPUT_MAX_DATA];
double umu=0.0;

gnuplot_ctrl *realplot, *imagplot,*modulplot, *argplot, *constplot;
time_t t;

int delayBypass=0;


char REAL[STR_LEN]="Real";
char IMAG[STR_LEN]="Imag";
char SPECMOD[STR_LEN]="Magnitude";
char SPECPHASE[STR_LEN]="Phase";
char CONST[STR_LEN]="Constellation";

// Identity
char identity[STR_LEN]="DEFAULT";
#define IDDEFAULT 12345678
int IDnumber=12345678;
char output_FILE[STR_LEN]="NULL";
int TS2capture=1000000;
char IDchar[STR_LEN]="44444444";
char aux[STR_LEN];

// Save Data in files
char timefile[STR_LEN]="reports/";
FILE *timep, *readfileP;
char READ_FILE[STR_LEN]="reports/";
char input_FILE[STR_LEN];
int readfilelength=0; 	//0: not read input file, 1: read input file length and copy at the input of GRAPH
int fileFORMAT=0;	//0: REAL, IMAGi; 1: REAL+IMAGi
// Time
unsigned long int Tslot=0;


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	int i, IDcapt[STR_LEN], numidsread, numidswrite, compare=0;
	_Complex float buff[STR_LEN];

	// Get parameters
	param_get_int("numsampl2view", &numsampl2view);
	param_get_int("plot_mode", &plot_mode);
	param_get_float("samplingfreqHz", &samplingfreqHz);
	param_get_float("fo_carrier", &fo_carrier);
	param_get_int("fftsize", &fftsize);
	param_get_int("averagefft", &averagefft);
	param_get_float("mu", &mu);
	param_get_int("logscale", &logscale);
	param_get_int("plot_periodTS", &plot_periodTS);
	param_get_int("bypass", &bypass);
	param_get_string("inputtype", &inputtype[0]);
	param_get_string("outputtype", &outputtype[0]);
//	
	param_get_int("readfilelength", &readfilelength);
	param_get_string("input_FILE", &input_FILE[0]);

	param_get_int("captureDATA", &captureDATA);
	param_get_int("fileFORMAT", &fileFORMAT);
	if(captureDATA==1){
		param_get_int("TS2capture", &TS2capture);
		param_get_string("output_FILE", &output_FILE[0]);
	}

	strcpy(mname, GetObjectName());

	//Check Configuration Values
	if(numsampl2view > OUTPUT_MAX_DATA){
			modinfo_msg("ERROR!!! numsampl2view=%d > OUTPUT_MAX_DATA=%d\n",numsampl2view, OUTPUT_MAX_DATA);
			return(-1);
	}

	if(fftsize > (INPUT_MAX_DATA*plot_periodTS))
		printf("Module %s ERROR: fftsize=%d must be lower than INPUT_MAX_DATA*plot_periodTS=%d\n",
				mname, fftsize, INPUT_MAX_DATA*plot_periodTS);

	strcat(aux, mname);

	//Plotting Time
	if((plot_mode & 0x01) == 0x01 || (plot_mode & 0x08) == 0x08){
//		printf("Plotting Time: %x\n", plot_mode & 0x01);
		realplot = gnuplot_init() ;
		gnuplot_setstyle(realplot,"lines");
		gnuplot_set_background(realplot, "gold");
		gnuplot_set_ylabel(realplot, "Voltage [V]");
		gnuplot_set_xlabel(realplot, "Time [s]");

		imagplot = gnuplot_init() ;
		gnuplot_setstyle(imagplot,"lines");
		gnuplot_set_background(imagplot, "gold");
		gnuplot_set_ylabel(imagplot, "Voltage [V]");
		gnuplot_set_xlabel(imagplot, "Time [s]");

		//numsampl2view=fftsize;
		for(i=0; i<numsampl2view; i++)xTIME_axis[i]=((double)i)/(samplingfreqHz); ///(double)fftsize)*(double)i;

		strcat(REAL, aux);
		strcat(IMAG, aux);

	}

	//Plotting Spectrum
	if((plot_mode & 0x02) == 0x02){
		modulplot = gnuplot_init() ;
		gnuplot_setstyle(modulplot,"lines");
		if(averagefft==0)gnuplot_set_background(modulplot, "greenyellow");
		if(averagefft==1)gnuplot_set_background(modulplot, "#FFF8DC");

		gnuplot_set_xlabel(modulplot, "Frequency [Hz]");
		if(logscale==0)gnuplot_set_ylabel(modulplot, "Magnitude [Linear]");
		if(logscale==1)gnuplot_set_ylabel(modulplot, "Magnitude [dB]");

		argplot = gnuplot_init() ;
		gnuplot_setstyle(argplot,"lines");
		if(averagefft==0)gnuplot_set_background(argplot, "greenyellow");
		if(averagefft==1)gnuplot_set_background(argplot, "#FFF8DC");
		gnuplot_set_ylabel(argplot, "Phase [rad]");
		gnuplot_set_xlabel(argplot, "Frequency [Hz]");

		if(fo_carrier==0.0)for(i=0; i<fftsize; i++)xFFT_axis[i]=(samplingfreqHz/(double)fftsize)*(double)i;
		else{
			for(i=0; i<fftsize; i++)xFFT_axis[i]=fo_carrier-(samplingfreqHz/2.0)+(samplingfreqHz/(double)fftsize)*(double)i;
		}
		//Create fftw3 plan
		fft_plan = fftw_plan_dft_1d(fftsize, fftbuff_IN, fftbuff_OUT, FFTW_FORWARD, FFTW_ESTIMATE);
		arg_scale = 180/PI;

		strcat(SPECMOD, aux);
		strcat(SPECPHASE, aux);
	}

	//Plotting Constellation
	if((plot_mode & 0x04) == 0x04){
		constplot = gnuplot_init() ;
		gnuplot_setstyle(constplot,"points");
		gnuplot_set_background(constplot, "#00FFFF");
		gnuplot_set_xlabel(constplot, "Inphase");
		gnuplot_set_ylabel(constplot, "Quadrature");

		strcat(CONST, aux);
	}

	// Create Write data files
	if(captureDATA>0){
		compare=strncmp(&output_FILE[0], "NULL", 4);
		if(compare==0){
			// NO output File Name read from para.ms
			strcat(timefile, mname);
		}else {
			// Output File Name read from .params
			strcat(timefile, output_FILE);
		}
		timep=datafile_open(timefile, "w");
	}
	// Create Read data files
	if(readfilelength){
		strcat(READ_FILE, input_FILE);
		readfileP=datafile_open(READ_FILE, "r");
//		printf("READ FILE:%s\n", READ_FILE);
	}



	//http://bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux

	/* Print Module Init Parameters */
//	printf("\33[1m\33[38;5;232;48;5;226mO--------------------------------------------------------------------------------------------O\t\033[0m\n");
	printf("\33[1m\33[38;5;232;48;5;226m################################################################################################\033[0m\n");
	memcpy(background, backgroundREF, strlen(backgroundREF));
	strcat(stringplot, mname);
	memcpy(background+5, stringplot, strlen(stringplot));
	printf("\33[1m\33[38;5;196;48;5;226m%s\t\033[0m\n", background);

	memcpy(background, backgroundREF, strlen(backgroundREF));
	memcpy(background+5, "SPECIFIC PARAMETERS SETUP:", strlen("SPECIFIC PARAMETERS SETUP:"));
	printf("\33[1m\33[38;5;232;48;5;226m%s\t\033[0m\n", background);

	memcpy(background, backgroundREF, strlen(backgroundREF));
	sprintf(stringplot, "Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	memcpy(background+8, stringplot, strlen(stringplot));
	printf("\33[1m\33[38;5;232;48;5;226m%s\t\033[0m\n", background);

	if(bypass==1){
		memcpy(background, backgroundREF, strlen(backgroundREF));
		sprintf(stringplot, "BYPASS: DataTypeIN=%s ===> DataTypeOUT=%s", inputtype, outputtype);
		memcpy(background+7, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;91;48;5;226m%s\t\033[0m\n", background);
	}

	if((plot_mode&0x01)==1 || (plot_mode&0x08)==0x8){
		memcpy(background, backgroundREF, strlen(backgroundREF));
		sprintf(stringplot, "PLOT TIME or SAMPLES:");
		memcpy(background+7, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;172;48;5;226m%s\t\033[0m\n", background);

		memcpy(background, backgroundREF, strlen(backgroundREF));
		sprintf(stringplot, "numsampl2view=%d, plot_periodTS=%d, samplingfreqHz=%3.1f", numsampl2view, plot_periodTS, samplingfreqHz);
		memcpy(background+8, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;172;48;5;226m%s\t\033[0m\n", background);
	}

	if((plot_mode&0x02)==2){
		memcpy(background, backgroundREF, strlen(backgroundREF));
		sprintf(stringplot, "PLOT SPECTRUM:");
		memcpy(background+7, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;28;48;5;226m%s\t\033[0m\n", background);

		memcpy(background, backgroundREF, strlen(backgroundREF));
		sprintf(stringplot, "fftsize=%d, fo_carrier=%3.1f, ", fftsize, fo_carrier);
		if(logscale==0)strcat(stringplot, " Lineal scale, ");
		if(logscale==1)strcat(stringplot, " Logaritmic scale, ");
		if(averagefft==0)strcat(stringplot, " Non averaged spectrum");
		if(averagefft==1){
			sprintf(auxchar, " Averaged spectrum with mu=%3.1f", mu);
			strcat(stringplot, auxchar);
		}
		memcpy(background+8, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;28;48;5;226m%s\t\033[0m\n", background);
	}

	if((plot_mode&0x04)==4){
		memcpy(background, backgroundREF, strlen(backgroundREF));
		sprintf(stringplot, "PLOT CONSTELLATION:");
		memcpy(background+7, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;21;48;5;226m%s\t\033[0m\n", background);

		memcpy(background, backgroundREF, strlen(backgroundREF));
		sprintf(stringplot, "Number of Points=%d", numsampl2view);
		memcpy(background+8, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;21;48;5;226m%s\t\033[0m\n", background);
	}

	if(captureDATA > 0){
		memcpy(background, backgroundREF, strlen(backgroundREF));
		sprintf(stringplot, "CAPTURE DATA:");
		memcpy(background+7, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;33;48;5;226m%s\t\033[0m\n", background);

		if(TS2capture==0){
			memcpy(background, backgroundREF, strlen(backgroundREF));
			sprintf(stringplot, "WARNING!!! Number of Time Slots to capture=%d", TS2capture);
			memcpy(background+8, stringplot, strlen(stringplot));
			printf("\33[1m\33[38;5;33;48;5;226m%s\t\033[0m\n", background);

			memcpy(background, backgroundREF, strlen(backgroundREF));
			sprintf(stringplot, "MAY BE YOU NEED TO SETUP THE TS2capture VALUE AT paramcfg/GRAPH_NAME.params");
			memcpy(background+8, stringplot, strlen(stringplot));
			printf("\33[1m\33[38;5;33;48;5;226m%s\t\033[0m\n", background);
		}
		else{
			memcpy(background, backgroundREF, strlen(backgroundREF));
			sprintf(stringplot, "Number of Time Slots to capture=%d", TS2capture);
			memcpy(background+8, stringplot, strlen(stringplot));
			printf("\33[1m\33[38;5;33;48;5;226m%s\t\033[0m\n", background);

			memcpy(background, backgroundREF, strlen(backgroundREF));
			//sprintf(stringplot, "File Name=%s, File Format=%d [0: RE;IMi # 1: RE+IMi]", timefile, fileFORMAT);
			sprintf(stringplot, "File Name=%s", timefile);
			memcpy(background+8, stringplot, strlen(stringplot));
			printf("\33[1m\33[38;5;33;48;5;226m%s\t\033[0m\n", background);
		}
	}

	if(readfilelength>0){
		memcpy(background, backgroundREF, strlen(backgroundREF));
		//sprintf(stringplot, "READ DATA FILE = %s, File Format=%d [0: RE;IMi # 1: RE+IMi]", READ_FILE, fileFORMAT);
		sprintf(stringplot, "READ DATA FILE = %s", READ_FILE);
		memcpy(background+7, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;201;48;5;226m%s\t\033[0m\n", background);

		memcpy(background, backgroundREF, strlen(backgroundREF));
		sprintf(stringplot, "File Format=%d [0: RE;IMi # 1: RE+IMi]", fileFORMAT);
		//sprintf(stringplot, "READ DATA FILE = %s", READ_FILE);
		memcpy(background+7, stringplot, strlen(stringplot));
		printf("\33[1m\33[38;5;201;48;5;226m%s\t\033[0m\n", background);
	}
//	printf("\33[1m\33[38;5;232;48;5;226mO--------------------------------------------------------------------------------------------O\t\033[0m\n\n");
	printf("\33[1m\33[38;5;232;48;5;226m################################################################################################\033[0m\n");



	/* Intializes random number generator */
	srand((unsigned) time(&t));
	for (i=0; i<fftsize; i++){
		module_aux[i] = 0.0;
	}
	umu=1.0-(double)mu;

	for(i=0; i<numsampl2view;i++){
		plot_real[i] = 0.0;
		plot_imag[i] = 0.0;
	}

	return 0;
}



/**
 * @brief Function documentation
 *
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address. To obtain the number of received samples use the function
 * int get_input_samples(int idx) where idx is the interface index.
 *
 * @param out Input interface buffers. Data to other interfaces must be stacked in the buffer.
 * Use out(ptr,idx) to access the address.
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 *
 * @code
 * 	input_t *first_interface = inp;
	input_t *second_interface = in(inp,1);
	output_t *first_output_interface = out;
	output_t *second_output_interface = out(out,1);
 *
 */
int work(input_t *inp, output_t *out) {
	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples=0;
	int i;
	static int tslot=0, num_INTIMEdades=0, num_INFFTdades=0, FFTplotEnabled=0, TIMEplotEnabled=0;
	static int tslot2=0;
	_Complex float buff[2048];
	int writen, readn;
	double *dinicip, *dfinalp;

//	printf("SINK_GRAPH: rcv-samples=%d, inp=%u\n", rcv_samples, inp);


//	printf("%s: rcv-samples=%d\n", mname, rcv_samples);
//	printf("%s: inp=%p, out=%p\n", mname, (void *)inp, (void *)out);

	if(readfilelength){
		rcv_samples=datafile_read_complex(readfileP, inp, readfilelength, fileFORMAT);
		if(rcv_samples==-1){
			printf("\33[1m\33[38;5;232;48;5;226m @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\t\033[0m\n");
			printf("\33[1m\33[38;5;232;48;5;226m    \t\t\t\t\t\t\t\t\t\t\t\t\t\033[0m\n", READ_FILE);
			printf("\33[1m\33[38;5;232;48;5;226m    ATENTION!!! DATA FILE READ %s FINALIZED. THIS WILL SHUDOWN THE APP\t\t\033[0m\n", READ_FILE);
			printf("\33[1m\33[38;5;232;48;5;226m    \t\t\t\t\t\t\t\t\t\t\t\t\t\033[0m\n", READ_FILE);
			printf("\33[1m\33[38;5;232;48;5;226m @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\t\033[0m\n");
			return(-1);
		}
		if(Tslot%50==0)printf("\033[1;31m    ATENTION!!! %d samples read from DATA FILE %s \033[0m\n", readfilelength, READ_FILE);
	}


	if (rcv_samples > INPUT_MAX_DATA) {
			modinfo_msg("WARNING!!! Truncating input %d to %d samples\n",rcv_samples, INPUT_MAX_DATA);
			rcv_samples = INPUT_MAX_DATA;
	}
	
	// CAPTURE DATA IN
	// TO TIME PLOTS
	if(((plot_mode & 0x01) == 0x01) || ((plot_mode & 0x4) == 0x4)){
		if(num_INTIMEdades < numsampl2view){
			for(i=0; i<rcv_samples;i++){
				if(strcmp(inputtype, "COMPLEXFLOAT")==0){
					//printf("INPUT COMPLEXFLOAT\n");
					plot_real[num_INTIMEdades+i] = (double) __real__ inp[i];
					plot_imag[num_INTIMEdades+i] = (double) __imag__ inp[i];
				}
				if(strcmp(inputtype, "COMPLEXFLOAT")!=0){
					plot_real[num_INTIMEdades+i] = (double)inp[i];
					plot_imag[num_INTIMEdades+i] = 0.0;
				}

			}
			num_INTIMEdades += rcv_samples;
			if(num_INTIMEdades>INPUT_MAX_DATA){
				printf("GRAPH.c(): ERROR num_INTIMEdades=%d>INPUT_MAX_DATA\n", num_INTIMEdades);
				return(-1);
			}
		}
	}
	if((plot_mode & 0x08) == 0x08){
//		printf("%s: rcv_samples=%d\n", mname, rcv_samples);
/*
		if(myTslot%5==0){
				if(EVMidx < 2048){
					EVMidx++;
					EVMvalues[EVMidx]=EVMG.EVMvalue;
				}
				else 	for(i=1; i<2048; i++){
					EVMvalues[i-1]=EVMvalues[i];
					EVMvalues[2047]=EVMG.EVMvalue;
				}
		}
*/


	//		printf("GRAPH.c(): plot_mode = 0x08\n");
		num_INTIMEdades=0;
		if(rcv_samples > 0){
	/*		printf("GRAPH: Tslot=%d, p_inp=%p, rcv_samples=%d, BER=%f\n", Tslot, inp, rcv_samples, (float)__real__ inp[0]);
			printf("INPUT0\n");
			printf("BER=%f\n", __real__ *inp);
			for(i=0; i<32; i++){
				//printf("%c", (char) *(inp+i));
				printf("%x", ((int)*(inp+i))&0xFF);
			}
			printf("\n");
*/
			if(numsampl2view >= rcv_samples){
				for(i=0; i<(numsampl2view-rcv_samples);i++){
					plot_real[i]=plot_real[i+rcv_samples];
					plot_imag[i]=plot_imag[i+rcv_samples];
				}
			}else{
				printf("GRAPH.c(): ERROR!!! numsampl2view=%d < rcv_samples=%d\n", numsampl2view, rcv_samples);
				return(-1);
			}
			for(i=0; i<rcv_samples;i++){
				if(strcmp(inputtype, "COMPLEXFLOAT")==0){
					plot_real[numsampl2view-rcv_samples+i] = (double) __real__ inp[i]+0.00000001;
					plot_imag[numsampl2view-rcv_samples+i] = (double) __imag__ inp[i]+0.00000001;
				}
			}
//			num_INTIMEdades += rcv_samples;
			num_INTIMEdades=numsampl2view;
		}
	}

	// TO FFT PLOTS
	if((plot_mode & 0x02) == 0x02){
		if(strcmp(inputtype, "COMPLEXFLOAT")!=0){
			printf("%s; ERROR: Type IN not a COMPLEX_FLOAT. Please, check at GRAPH.c().interfaces.h \n", mname);
		}
		if(num_INFFTdades < fftsize){
			for(i=0; i<rcv_samples;i++){
				fftbuff_IN[num_INFFTdades+i]=(fftw_complex)inp[i];
			}
			num_INFFTdades += rcv_samples;
			if(num_INFFTdades>INPUT_MAX_DATA*PLOT_PERIODTS){
				printf("GRAPH.c(): ERROR num_INFFTdades=%d>INPUT_MAX_DATA\n", num_INFFTdades);
				return(-1);
			}
		}
	}

	//CHECK IF PLOTTING
	tslot++;
	if(tslot==plot_periodTS){
		tslot=0;
		if(num_INTIMEdades >= numsampl2view){
			TIMEplotEnabled=1;
		}
		if(num_INFFTdades >= fftsize){
			FFTplotEnabled=1;
		}
//		printf("%s:TIMEplotEnabled=%d, FFTplotEnabled=%d\n", mname, TIMEplotEnabled, FFTplotEnabled);
	}

//	printf("SINKGRAPH.c: Tslot=%d, plot_periodTS=%d, num_INTIMEdades=%d, numsamples2view=%d, TIMEplotEnabled=%d\n",
//				tslot, plot_periodTS, num_INTIMEdades, numsampl2view, TIMEplotEnabled);



	// PLOTTING TIME
	if(TIMEplotEnabled==1){
		TIMEplotEnabled=0;
		if((plot_mode & 0x08) != 0x08)num_INTIMEdades=0;
		// Plotting Time
		if(((plot_mode & 0x01) == 0x01) || ((plot_mode & 0x08) == 0x08)){
//			printf("mname=%s, Tslot=%d, PLOT\n", mname, tslot);
			gnuplot_resetplot(realplot);
			//gnuplot_plot_xy(realplot, xTIME_axis, plot_real, numsampl2view, "Real");
			gnuplot_plot_xy(realplot, xTIME_axis, plot_real, numsampl2view, REAL);
			gnuplot_resetplot(imagplot);
			gnuplot_plot_xy(imagplot, xTIME_axis, plot_imag, numsampl2view, IMAG);
		}
		// Plotting Constellation
		if((plot_mode & 0x4) == 0x04){
			//Plotting
			gnuplot_resetplot(constplot);
			gnuplot_plot_xy(constplot, plot_imag, plot_real, numsampl2view, CONST);
		}
	}

	if(FFTplotEnabled==1){
		FFTplotEnabled=0;
		num_INFFTdades=0;

		/// Plotting Spectrum
		if((plot_mode & 0x02) == 0x02){
			// Perform FFT
			fftw_execute(fft_plan);

			//Process Magnitude and Arg of Data
			arg_scale=1.0;
			// No averaged
			for (i=0; i<fftsize; i++){
				module[i] = (double)(cabs((complex double)fftbuff_OUT[i]));
				arg[i] = (double)(arg_scale*cargf(fftbuff_OUT[i]));
				// Average
				if(averagefft==1){
					module_aux[i] = umu*module_aux[i] + (double)mu*module[i];
					arg_aux[i] = umu*arg_aux[i] + (double)mu*arg[i];
					module[i] = module_aux[i];
					arg[i] = arg_aux[i];
				}
				if(logscale==1)module[i] = 20.0*log10(module[i]+0.000000001);
			}

			if(fo_carrier>0.0){
				for (i=0; i<fftsize; i++){
					aux1[i]=module[i];
					aux2[i]=arg[i];
				}
				for (i=0; i<fftsize/2; i++){
					module[i]=aux1[i+fftsize/2];
					module[i+fftsize/2]=aux1[i];
					arg[i]=aux2[i+fftsize/2];
					arg[i+fftsize/2]=aux2[i];
				}
			}

			//Plotting
			gnuplot_resetplot(modulplot);
			gnuplot_plot_xy(modulplot, xFFT_axis, module, fftsize, SPECMOD);
			gnuplot_resetplot(argplot);
			gnuplot_plot_xy(argplot, xFFT_axis, arg, fftsize, SPECPHASE);

		}
	}
	if(bypass==1){
		if(bypass_func(IN_TYPE, OUT_TYPE, inp, rcv_samples, out)==-1)return(-1);
		snd_samples=rcv_samples;
	}

	// CAPTURE DATA
	// TEXT FILE
	if(captureDATA==1){
		if(rcv_samples>0){
			datafile_write_complex(timep, (_Complex float *) inp, rcv_samples, fileFORMAT);
		}
	}
	// BIN FILE
	if(captureDATA==2){
		printf("ERROR: BINARI FILE CAPTURE NOT YET AVAILABLE\n");
		printf("PLEASE, DO NOT USE YET 'captureDATA=2' IN PARAMS FILE\n");
		printf("LEAVING....");
		return(-1);
	}


	if(Tslot == TS2capture){
		printf("\033[1;31m @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\033[0m\n");
		printf("\033[1;31m           ATENTION!!! DATA CAPTURE %s FINALIZED. THIS WILL SHUDOWN THE APP\033[0m\n", mname);
		printf("\033[1;31m           TIMESLOTS TO CAPTURE = %d\033[0m\n", TS2capture);
		printf("\033[1;31m           PLEASE; CHECK THAT THE CAPTURED DATA FILES ARE OK IN reports/ FOLDER\033[0m\n");
		printf("\033[1;31m @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\033[0m\n");

		return(-1);
	}
	Tslot++;
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


