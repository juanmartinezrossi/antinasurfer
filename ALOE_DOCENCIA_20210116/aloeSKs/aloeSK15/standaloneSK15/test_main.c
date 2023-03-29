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
#include <complex.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "phal_sw_api.h"
#include <swapi_utils.h>
#include "skeletonSK15.h"
#include "paramsSK15.h"

#include "rtdal_datafile.h"
#include "test_generate.h"
#include "gnuplot_i.h"
#include "complexplot.h"
#include "graphs_mode.h"
#include "plot_mode.h"

#define STRSIZE				32
//Test Main Control Parameters//////////////
#define NOF_TEST_PARAMS		5
#define NOFCHARS			16
typedef struct {
//	char mname[STR_LEN];
	int run_times;
	int block_length;
	char inputMode[NOFCHARS];					//GEN: Test_generate; FILE_TEXT: Read File;  FILE_BIN: Read Binary File
	char outputMode[NOFCHARS];					//PLOT: Plot; FILE_TEXT: Save in File;  FILE_BIN: Read Binary File
	char plot_modeIN[STR_LEN];
	char plot_modeOUT[STR_LEN];
										/*Plotting Options:
											C1WLS: Real-Imaginary, 1 Window, Linear, Single frame
											C2WLS: Real + Imaginary (2 Windows); Linear, Single frame.
											CFFTMF: Complex, FFT, Multiple Frames.
											CFFTSF: Complex, FFT, Magnitude & Argument, Single Frame.
											CCSF: Complex, Constellation. Single Frame.
											DEFAULT: User defined at src/graph_mode.h
											NO_PLOT: Do not plot
										*/
	float samplingfreqHz;
}test_gen_t;

test_gen_t ctrl = {1, 1024, "GEN", "PLOT", "NO_PLOT", "NO_PLOT", 1.0};	//"module_template2",
////////////////////////////////////////////

extern const int input_max_samples;
extern const int output_max_samples;
extern const int input_sample_sz;
extern const int output_sample_sz;
extern const int nof_input_itf;
extern const int nof_output_itf;
extern const char* in_type;
extern const char* out_type;

FILE *dat_input=NULL, *dat_output=NULL;
char *dat_input_name=NULL, *dat_output_name=NULL;

extern char mname[];

//static input_t *input_data;
//static output_t *output_data;
static void *input_data;
static void *output_data;
static void **input_ptr, **output_ptr;

static int *input_lengths;
static int *output_lengths;
static int *auxinput_lengths;
static int *auxoutput_lengths;

param_t parameters[MAX_PARAMS];

int use_gnuplot=0;		/*0: no plot; 1:plot*/
int use_binary_input=-1;	/*-1: No file, 0: text input; 1: binary input*/
int use_binary_output=-1;	/*-1: No file, 0: text input; 1: binary input*/
int show_help=0;

typedef struct {
	char *name;
	char *value;
} saparam_t;

saparam_t *parametersA;
int nof_params;

int parse_paramters(int argc, char**argv);
int param_setup(char *name, char *value, int nofdefparams);

inline int get_input_samples(int idx) {
	return input_lengths[idx];
}

inline void set_output_samples(int idx, int len) {
	output_lengths[idx] = len;
}

inline int get_input_max_samples() {
	return input_max_samples;
}
inline int get_output_max_samples() {
	return output_max_samples;
}

inline void check_output_samples(char *modname, int idx){
	printf("%s: number of samples at output_%d = %d\n", modname, idx, output_lengths[idx]);
}

void check_input_samples(char *modname, int idx){
	printf("%s: number of samples at input_%d = %d\n", modname, idx, input_lengths[idx]);
}


void allocate_memory() {
	int aux;
	aux=nof_input_itf;
	if(nof_input_itf <= 0)aux=nof_input_itf+1;
	posix_memalign((void**)&input_data,64,input_max_samples*nof_input_itf*input_sample_sz);
	assert(input_data);
	input_lengths = calloc(sizeof(int),aux);
	assert(input_lengths);
	auxinput_lengths = calloc(sizeof(int),aux);
	assert(auxinput_lengths);
	input_ptr = calloc(sizeof(void*),aux);
	assert(input_ptr);

	aux=nof_output_itf;
	if(nof_output_itf <= 0)aux=nof_output_itf+1;
	posix_memalign((void**)&output_data,64,output_max_samples*nof_output_itf*output_sample_sz);
	assert(output_data);
	output_lengths = calloc(sizeof(int),aux);
	assert(output_lengths);
	auxoutput_lengths = calloc(sizeof(int),aux);
	assert(auxoutput_lengths);
	output_ptr = calloc(sizeof(void*),aux);
	assert(output_ptr);
}

void free_memory() {
	free(input_data);
	free(output_data);
	free(input_lengths);
	free(output_lengths);
	if (parametersA) {
		for (int i=0;i<nof_params;i++) {
			if (parametersA[i].name) free(parametersA[i].name);
			if (parametersA[i].value) free(parametersA[i].value);
		}
		free(parametersA);
	}
}

int get_time(struct timespec *x) {

	if (clock_gettime(CLOCK_MONOTONIC,x)) {
		return -1;
	}
	return 0;
}

void get_time_interval(struct timespec * tdata) {

	tdata[0].tv_sec = tdata[2].tv_sec - tdata[1].tv_sec;
	tdata[0].tv_nsec = tdata[2].tv_nsec - tdata[1].tv_nsec;
	if (tdata[0].tv_nsec < 0) {
		tdata[0].tv_sec--;
		tdata[0].tv_nsec += 1000000000;
	}
}

int convert_data2Cfloat(void *datainV, _Complex float *dataoutC, char *type, int length);
int read_BIN_file(char *input_name, FILE *input_pointer, int length, char *datatype, void *indata);
int read_TEXT_file(char *input_name, FILE *input_pointer, int length, char *datatype, void *indata);
int write_TEXT_file(char *output_name, FILE *p_output_file,
		void *out_data, int length, char *data_type);
int write_BIN_file(char *output_filename, FILE *p_output_file,
		void *out_data,  int length, char *data_type);



/**
 * Testsuite:
 * Generates random symbols and calls the module ....
 * @param ...
 * @param ...
 * @param ... */
int main(int argc, char **argv)
{
	struct timespec tdata[3];
	gnuplot_ctrl *plot;
	char auxINTYPE[STRSIZE];
	char auxOUTTYPE[STRSIZE];
	/* Complex plot structure definition */
//	SComplexplot complexplot_ctrl;
//	char tmp[64];
	int ret=0, i, z, k;
//	_Complex float *pCfloat;
//	float *pfloat;
//	char *pchar;

	_Complex float *plot_buffC;
//	char *datatype;
//	char *inputmode="FILE";
//	int file_read_sz=0;
//	int out_length0;

	strcpy(auxINTYPE, in_type);
	strcpy(auxOUTTYPE, out_type);
//	printf("%s, %s\n", auxINTYPE, in_type);

	parametersA = NULL;

	allocate_memory();
	param_init(parameters,nof_parameters);
	parse_paramters(argc, argv);

	if(show_help==0)printf("Put -h for standalone execution options!!!\n");
	if(show_help==1){
		printf("\033[01;37;45mo STANDALONE EXECUTION oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\033[0m\n");
		printf("\033[01;37;46mo Two alternatives: a)Autogenerated input data, b)Load input file.                        o\033[0m\n");
		printf("\033[01;37;46mo Options: -p:  plot input/output                                                         o\033[0m\n");
		printf("\033[01;37;46mo          -it: input text file according module input data type format.                  o\033[0m\n");
		printf("\033[01;37;46mo          -ib: input binari file according module input data type format.                o\033[0m\n");
		printf("\033[01;37;46mo          -ot: output text file according module output data type format.                o\033[0m\n");
		printf("\033[01;37;46mo          -ob: output binari data file according module output data type format.         o\033[0m\n");
		printf("\033[01;37;46mo          block_length=XX: Number of samples sent to input each time slot                o\033[0m\n");
		printf("\033[01;37;46mo          run_times=XX: Number of time slot executed                                     o\033[0m\n");
		printf("\033[01;37;46mo          plot_mode=XX: plotting options                                                 o\033[0m\n");
		printf("\033[01;37;46mo Autogenerated input data example:                                                       o\033[0m\n");
		printf("\033[01;37;46mo     ./module_name -p block_length=10 run_times=2                                        o\033[0m\n");
		printf("\033[01;37;46mo Load input data example:                                                                o\033[0m\n");
		printf("\033[01;37;46mo     ./module_name -p -ib ../../../LTEcaptures/aloe.dat block_length=10 run_times=1      o\033[0m\n");
		printf("\033[01;37;46mo Save Module Output example:                                                             o\033[0m\n");
		printf("\033[01;37;46mo     ./module_name -ot ../../../LTEcaptures/filename.txt block_length=10 run_times=1     o\033[0m\n");
		printf("\033[01;37;46mo Plotting Options: Substitute XX by IN or OUT                                            o\033[0m\n");
		printf("\033[01;37;46mo          plot_modeXX=C1WLS: Real-Imaginary, 1 Window, Linear, Single frame              o\033[0m\n");
		printf("\033[01;37;46mo          plot_modeXX=C2WLS: Real + Imaginary (2 Windows); Linear, Single frame.         o\033[0m\n");
		printf("\033[01;37;46mo          plot_modeXX=CFFTMF: Complex, FFT, Multiple Frames.                             o\033[0m\n");
		printf("\033[01;37;46mo          plot_modeXX=CFFTSF: Complex, FFT, Magnitude & Argument, Single Frame.          o\033[0m\n");
		printf("\033[01;37;46mo          plot_modeXX=CCSF: Complex, Constellation. Single Frame.                        o\033[0m\n");
		printf("\033[01;37;46mo          plot_modeXX=DEFAULT: User defined at src/graph_mode.h                          o\033[0m\n");
		printf("\033[01;37;46mo          plot_modeXX=NO_PLOT: No plotting the IN or OUT data                            o\033[0m\n");
		printf("\033[01;37;46mo Plot example with autogenerated data:                                                   o\033[0m\n");
		printf("\033[01;37;46mo     ./module_name -p block_length=10 run_times=2 plot_modeIN=CFFTSF plot_modeOUT=C1WLS  o\033[0m\n");
		printf("\033[01;37;43mooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\033[0m\n");
		return(0);
	}
	printf("Capture command line parameteroooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
	//Capture command line parameters
	for(i=0; i<nof_params; i++){
		z=param_setup(parametersA[i].name, parametersA[i].value, nof_params);
		if(z==0){
			if(!strcmp(parametersA[i].name,"block_length"))ctrl.block_length=atoi(parametersA[i].value);
			if(!strcmp(parametersA[i].name,"run_times"))ctrl.run_times=atoi(parametersA[i].value);
			if(!strcmp(parametersA[i].name,"samplingfreqHz"))ctrl.samplingfreqHz=atof(parametersA[i].value);
			if(!strcmp(parametersA[i].name,"plot_modeIN"))strcpy((char *)ctrl.plot_modeIN, parametersA[i].value);
			if(!strcmp(parametersA[i].name,"plot_modeOUT"))strcpy((char *)ctrl.plot_modeOUT, parametersA[i].value);
		}
	}
	// Check Parameters Values
//	check_ctrl_params(ctrl);	//TBD


	// Print Module Init Parameters
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    PLOTTING SPECIFIC CTRL PARAMETERS SETUP: \033[1;34m%s.generate_input_signal().\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n",
		       nof_input_itf, in_type, nof_output_itf, auxOUTTYPE);
	printf("O      block_length=%d, run_times=%d, sampligfreqHz=%3.3f\n",
					ctrl.block_length, ctrl.run_times, ctrl.samplingfreqHz);
	printf("O      Input_Mode=%s, Output_Mode=%s\n",
						ctrl.inputMode, ctrl.outputMode);
	printf("O      Input_File=%s, Output_File=%s\n",
							dat_input_name, dat_output_name);
	printf("O      plot_modeIN=%s, plot_modeOUT=%s\n",
					ctrl.plot_modeIN, ctrl.plot_modeOUT);
	printf("O--------------------------------------------------------------------------------------------O\n");

	//Call Initialize() function
	printf("Test Initialize module ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
	if (initialize()) {
		printf("Error initializing\n");
		exit(1); /* the reason for exiting should be printed out beforehand */
	}
	// GET INPUT DATA
	printf("Test Generate Input Signalsooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
	// 1-Generate Input Data
//	printf("ctrl.inputMode=%s\n", ctrl.inputMode);

	if (strcmp(ctrl.inputMode,"GEN")==0){
		if(nof_input_itf > 0){
			input_lengths[0]=ctrl.block_length*ctrl.run_times;
			printf("TEST_GENERATE: IN input_lengths[0]=%d\n", input_lengths[0]);
			if (generate_input_signal(input_data, input_lengths)) {
				printf("Error generating input signal\n");
				exit(1);
			}
		}
	}
	// 2-Read Input TEXT File
	if(strcmp(ctrl.inputMode,"FILE_TEXT")==0){
		printf("FILE_TEXT TO LOAD: IN input_lengths[0]=%d\n", input_lengths[0]);

		if(nof_input_itf > 0){
			input_lengths[0]=ctrl.block_length*ctrl.run_times;
			z=read_TEXT_file(dat_input_name, dat_input, input_lengths[0], auxINTYPE, input_data);
			if(z==-1){
				printf("Error reading input data text file\n");
				exit(1);
			}
		}
	}
//	read_bin_file(dat_input_name, dat_input, int length, char *datatype);
	// 2-Read Input BIN File
	if(strcmp(ctrl.inputMode,"FILE_BIN")==0){
		printf("FILE_BIN TO LOAD: IN input_lengths[0]=%d\n", input_lengths[0]);
		if(nof_input_itf > 0){
			input_lengths[0]=ctrl.block_length*ctrl.run_times;
			z=read_BIN_file(dat_input_name, dat_input, input_lengths[0], auxINTYPE, input_data);
			if(z==-1){
				printf("Error reading input data text file\n");
				exit(1);
			}
		}
	}


	// Open Input File

/*	printf("o Test Captured Config ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
	printf("o dat_input_name = %s, dat_output_name = %s\n", dat_input_name,dat_output_name);
	printf("o inputmode=%s, datatype=%s, fileread_sz=%d \n", inputmode, datatype, file_read_sz);
	printf("o block_length=%d, run_times=%d\n", block_length, run_times);
	printf("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
*/
	printf("Test Check Input Buffers ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
	for (i=0;i<nof_input_itf;i++) {
		if (!input_lengths[i]) {
			printf("Warning, input interface %d has zero length\n",i);
		}
	}
	printf("Test Clock Init oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
	clock_gettime(CLOCK_MONOTONIC,&tdata[1]);

	printf("Test Execute STANDALONE Work ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
	printf("block_length=%d, run_times=%d,input_sample_sz=%d\n", ctrl.block_length, ctrl.run_times, input_sample_sz);
	printf("A block_length=%d, run_times=%d,output_lengths[0]=%d\n", ctrl.block_length, ctrl.run_times, output_lengths[0]);

	for(i=0; i<nof_output_itf; i++){
		output_lengths[i]=0;
		auxoutput_lengths[i]=0;
	}
	input_lengths[0]=ctrl.block_length;
	for (i=0;i<ctrl.run_times;i++) {
		work(&input_data[i*ctrl.block_length], &output_data[output_lengths[0]]);
		for(k=0; k<nof_output_itf; k++){
			auxoutput_lengths[k]+=output_lengths[k];
		}

	}
	printf("\033[01;33;40mDATA LENGTHS: block_length=%d, run_times=%d, module_OUT_size=%d, data_out_length=%d\033[0m\n",
						ctrl.block_length, ctrl.run_times, output_lengths[0], output_lengths[0]*ctrl.run_times);






	input_lengths[0] = ctrl.block_length*ctrl.run_times;
	output_lengths[0]=output_lengths[0]*ctrl.run_times;
/*	for(k=0; k<nof_output_itf; k++){
		output_lengths[k]=auxoutput_lengths[k];
	}
*/
//	printf("A block_length=%d, run_times=%d,input_lengths[0]=%d\n", ctrl.block_length, ctrl.run_times, input_lengths[0]);
//	printf("B block_length=%d, run_times=%d,output_lengths[0]=%d\n", ctrl.block_length, ctrl.run_times, output_lengths[0]);


	printf("Capture Execution Time ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");

	clock_gettime(CLOCK_MONOTONIC,&tdata[2]);
	stop();
	if (ret == -1) {
		printf("Error running\n");
		exit(-1);
	}
	get_time_interval(tdata);
	printf("Execution time: %d ns.\n", (int) tdata[0].tv_nsec);
	printf("Check Output Buffers ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
//	printf("Nof nof_output_itf=%d\n", nof_output_itf);
	for (i=0;i<nof_output_itf;i++) {
		if (!output_lengths[i]) {
			output_lengths[i] = ret;
		}
		if (!output_lengths[i]) {
			printf("Warning output interface %d has zero length\n",i);
		}
	}
	printf("Output File oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
//////////////////
	// Save put Text File
	// Open Output File
	if(strcmp(ctrl.outputMode,"FILE_TEXT")==0){
		write_TEXT_file(dat_output_name, dat_output, output_data,  output_lengths[0], auxOUTTYPE);
	}
	if(strcmp(ctrl.outputMode,"FILE_BIN")==0){
		write_BIN_file(dat_output_name, dat_output, output_data,  output_lengths[0], auxOUTTYPE);
	}

//////////////////
	printf("Input/Output Plot oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
/*	ctrl.block_length=block_length;
	ctrl.run_times=run_times;
	ctrl.samplingfreqHz=samplingfreqHz;
*/
//	if((strcmp(ctrl.plot_modeIN, "NO_PLOT") != 0) || (strcmp(ctrl.plot_modeOUT, "NO_PLOT") != 0)){

		if(strcmp(ctrl.plot_modeIN, "NO_PLOT") != 0){
			for (i=0;i<nof_input_itf;i++) {
//				printf("WARNING!!! test_main(): input_lengths[%d]=%d\n", i, input_lengths[i]);
				plot_buffC = malloc(sizeof(_Complex float)*input_lengths[i]);

				convert_data2Cfloat(input_data, plot_buffC, auxINTYPE, input_lengths[i]);
				if(strcmp(ctrl.plot_modeIN, "DEFAULT") != 0){
					plotcomplex(ctrl.plot_modeIN, plot_buffC, input_lengths[i], plot, "Input", &complexplot_ctrl, ctrl.samplingfreqHz, ctrl.run_times);
				}else {
					printf("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
					printf("WARNING!!!: Plot default configuration defined at 'graphs_mode.h' setup.                     o\n");
					printf("Please, modify according your needs.                                                         o\n");
					printf("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
				}
				complexplot(plot_buffC, input_lengths[i], plot, "Input", &complexplot_ctrl);
				free(plot_buffC);
			}
		}
		if(strcmp(ctrl.plot_modeOUT, "NO_PLOT") != 0){
			for (i=0;i<nof_input_itf;i++) {
//				printf("WARNING!!! test_main(): output_lengths[%d]=%d\n", i, output_lengths[i]);
				plot_buffC = malloc(sizeof(_Complex float)*output_lengths[i]);

				convert_data2Cfloat(output_data, plot_buffC, auxOUTTYPE, output_lengths[i]);
				if(strcmp(ctrl.plot_modeOUT, "DEFAULT") != 0){
					plotcomplex(ctrl.plot_modeOUT, plot_buffC, output_lengths[i], plot, "Output", &complexplot_ctrl, ctrl.samplingfreqHz, ctrl.run_times);
				}else {
					printf("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
					printf("WARNING!!!: Plot default configuration defined at 'graphs_mode.h' setup.                     o\n");
					printf("Please, modify according your needs.                                                         o\n");
					printf("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
				}
				complexplot(plot_buffC, output_lengths[i], plot, "Output", &complexplot_ctrl);
				free(plot_buffC);
			}
		}
		printf("Type ctrl+c to exit\n");fflush(stdout);
		free_memory();
		pause();
		/* make sure we exit here */
		exit(1);
//	}

	free_memory();

	return 0;

}



/* Define test environment functions here */
int parse_paramters(int argc, char**argv)
{
	int i;
	char *key,*value;
	int k = 0;

	nof_params = argc-1;

//printf("nof_params=%d\n", nof_params);
	for (i=1;i<argc;i++) {
		if (!strcmp(argv[i],"-p")) {		//Plot input/output option
			use_gnuplot = 1;
			nof_params--;
//			printf("-p\n");
		} else if (!strcmp(argv[i],"-it")) {	//Input text file
			dat_input_name = argv[i+1];
			i++;
			nof_params-=2;
			use_binary_input=0;
//			ctrl.inputMode="FILE_TEXT";
			strcpy(ctrl.inputMode, "FILE_TEXT");
//			printf("-it\n");
		} else if (!strcmp(argv[i],"-ib")) {	//Input binari file
			dat_input_name = argv[i+1];
			use_binary_input = 1;
			strcpy(ctrl.inputMode, "FILE_BIN");
			i++;
			nof_params-=2;
//			printf("-ib\n");
		}else if (!strcmp(argv[i],"-ot")) {	//Output text file
			dat_output_name = argv[i+1];
			i++;
			nof_params-=2;
			use_binary_output=0;
			strcpy(ctrl.outputMode, "FILE_TEXT");
//			printf("-ot\n");
		}else if (!strcmp(argv[i],"-ob")) {	//Output binari file
			dat_output_name = argv[i+1];
			i++;
			nof_params-=2;
			use_binary_output=1;
			strcpy(ctrl.outputMode, "FILE_BIN");
//			printf("-ob\n");
		}else if (!strcmp(argv[i],"-h")) {	//Show Help
			nof_params--;
			show_help=1;
//			printf("-help\n");
		}
	
	}
	if (!nof_params) {
		return 0;
	}

	parametersA = calloc(sizeof(saparam_t),nof_params);

	for (i=1;i<argc;i++) {
		if (strcmp(argv[i],"-p")) {
			key = argv[i];
			value = index(argv[i],'=');
			if (value) {
				*value = '\0';
				value++;
				parametersA[k].name = strdup(key);
				parametersA[k].value = strdup(value);
/*				printf("name=%s\n", parametersA[k].name);
				printf("value=%s\n", parametersA[k].value);
				printf("value=%d\n", atoi(parametersA[k].value));
				printf("value=%3.3f\n", atof(parametersA[k].value));
*/
				k++;
			}
		}
	}
	return 0;
}

/*INITIALIZE params with captured values*/
int param_setup(char *name, char *value, int nofcaptparams){

	int i, type, idx;
	void *param=NULL;

	//Check if captured params were defined in modulename_params.h
//	printf("param_setup(): name=%s\n", name);

	if((strcmp(name,"block_length")==0) ||
			(strcmp(name,"run_times")==0) ||
			(strcmp(name,"sampligfreqHz")==0) ||
			(strcmp(name,"plot_modeIN")==0) ||
			(strcmp(name,"plot_modeOUT")==0)){
		printf("Test Parameter:%s Found\n", name);
		return(0);
	}

	param=param_get_addr(name);

	idx=param_get_idx(name);
	
	printf("param_adr0=%u, idx=%d, para_adr1=%u\n", (unsigned int )param, idx, (unsigned int )param_get_addr_i(idx));

	if(param==NULL){
		printf("WARNING: Param '%s' not included in parameters[] list of data_source_params.h      :-Ç!\n", name);
		for(i=0; i<3; i++){
			printf("Please, do not use param '%s' in command line                                    :-Ç!\n", name);
		}
		return(0);
	}
	else{
//		printf("Param %s initialized by command line\n", name);
	}
//	printf("param_adr=%d\n", param);
	type=param_get_aloe_type(name);
	printf("type=%d\n", type);
	switch(type) {
		case STAT_TYPE_INT:
//			printf("INT\n");
			*((int*)param)=atoi(value);
//			printf("*param=%d\n", *((int*)param));
			return 1;
		case STAT_TYPE_FLOAT:
//			printf("FLOAT\n");
			*((float*)param)=atof(value);
//			printf("*param=%3.3f\n", *((float*)param));
			return 1;
		case STAT_TYPE_COMPLEX:
//			printf("COMPLEX\n");
			printf("Warning complex parameters not yet supported\n");
			return 1;
		case STAT_TYPE_STRING:
//			printf("STRING\n");
//			printf("*param=%d\n", param);	
//			len=strlen(value);
//			printf("len=%d\n", len);
			strcpy((char *)param, value);
//			printf("*param=%s\n", param);			
			return 1;
		default:
			printf("DEFAULT\n");
			printf("%s.params_setup(): Error. Default ", "sink_plot");
			return -1;
	}
	return(1);
}




int check_ctrl_params(test_gen_t *ctrl){
	int error=0;

	printf("check_ctrl_params: NO ERROR. TBD\n");
	//Return -1 on error, 0 no error
	return(error);
}

int convert_data2Cfloat(void *datainV, _Complex float *dataoutC, char *type, int length){

	int j;
	_Complex float *cpxfloat=datainV;
	float *pfloat=datainV;
	int *pint=datainV;
	char *pchar=datainV;

	if(strcmp(type, "COMPLEXFLOAT") == 0){
		printf("COMPLEXFLOAT CONVERT\n");
		for (j=0;j<length;j++)dataoutC[j] = cpxfloat[j];
	}
	if(strcmp(type, "FLOAT") == 0){
		printf("FLOAT CONVERT\n");
		for (j=0;j<length;j++){
			__real__ dataoutC[j] = (float)pfloat[j];
			__imag__ dataoutC[j] = 0.0;
		}
	}
	if(strcmp(type, "INT") == 0){
		printf("INTT CONVERT\n");
		for (j=0;j<length;j++){
			__real__ dataoutC[j] = (float)pint[j];
			__imag__ dataoutC[j] = 0.0;
		}
	}
	if(strcmp(type, "CHAR") == 0){
		printf("CHAR CONVERT\n");
		for (j=0;j<length;j++){
			__real__ dataoutC[j] = (float)pchar[j];
			__imag__ dataoutC[j] = 0.0;
		}
	}
	return(0);
}



int read_TEXT_file(char *input_name, FILE *input_pointer, int length, char *datatype, void *indata){

	int n;

	if (input_name) {
//		printf("dat_input_name = %s\n",dat_input_name);			//LINE ADDED
//		printf("0____dat_input=%d\n", dat_input);
		input_pointer = rtdal_datafile_open(input_name, "r");
//		printf("1____dat_input=%d\n", dat_input);
		if (!input_pointer) {
			printf("Error opening file %s\n",input_name);
			return(-1);
		}
		//COMPLEXFLOAT
		if(strcmp(datatype, "COMPLEXFLOAT") == 0){
//			printf("Complex length=%d, pointer input_data=%u\n", length, indata);
			n = rtdal_datafile_read_complex(input_pointer,(_Complex float *)indata,length);
//			printf("Complex n=%d, pointer input_data=%u\n", n, indata);
		}
		//FLOAT
		if(strcmp(datatype, "FLOAT") == 0){
			n = rtdal_datafile_read_real(input_pointer,(float *)indata,length);
//			printf("Float n=%d, pointer input_data=%u\n", n, input_data);
		}
		//CHAR-BIN
		if(strcmp(datatype, "CHAR") == 0){
			n = rtdal_datafile_read_bin(input_pointer,(char *)indata,length);
//			printf("Char n=%d, pointer input_data=%u\n", n, input_data);
		}
	}
	//printf("n=%d, length=%d\n", n, length);
	if(n < length){
		//printf("read_text_file(): ERROR: Readed length=%d do not match the expected length=%d\n", n, length);
		printf("\033[01;31m read_text_file(): ERROR!!!: File length=%d lower than the expected length=%d\033[0m\n", n, length);
		return(-1);
	}else return(n);
}


int read_BIN_file(char *input_name, FILE *input_pointer, int length, char *datatype, void *indata){
	int n;
	int sample_sz;

	if (input_name) {
//		printf("dat_input_name = %s\n",dat_input_name);			//LINE ADDED
//		printf("0____dat_input=%d\n", dat_input);
		input_pointer = rtdal_datafile_open(input_name, "r");
//		printf("1____dat_input=%d\n", dat_input);
		if (!input_pointer) {
			printf("Error opening file %s\n",input_name);
			return(-1);
		}
		if(strcmp(datatype, "CHAR") == 0)sample_sz=sizeof(char);
		if(strcmp(datatype, "FLOAT") == 0)sample_sz=sizeof(float);
		if(strcmp(datatype, "COMPLEXFLOAT") == 0)sample_sz=sizeof(_Complex float);
		n = rtdal_datafile_read_bin(input_pointer,indata,length*sample_sz);
		if(n < length*sample_sz){
			//printf("ERROR: Readed length=%d do  not match the expected length=%d\n", n/sample_sz, length);
			printf("\033[01;31m read_text_file(): ERROR!!!: File length=%d lower than the expected length=%d\033[0m\n", n, length);
			return(-1);
		}
	}
	return(n/sample_sz);
}



/*	if (dat_input_name) {
//		printf("dat_input_name = %s\n",dat_input_name);			//LINE ADDED
//		printf("0____dat_input=%d\n", dat_input);
		dat_input = rtdal_datafile_open(dat_input_name, "r");
//		printf("1____dat_input=%d\n", dat_input);
		if (!dat_input) {
			printf("Error opening mat file %s\n",dat_input_name);
			exit(1);
		}
		if(strcmp(ctrl.inputMode,"FILE_TEXT")==0){
			datatype=IN_TYPE;
			input_lengths[0]=ctrl.block_length*ctrl.run_times;
			//COMPLEXFLOAT
			if(strcmp(datatype, "COMPLEXFLOAT") == 0){
				n = rtdal_datafile_read_complex(dat_input,(_Complex float *)input_data,input_lengths[0]);
				printf("Complex n=%d, pointer input_data=%u\n", n, input_data);
			}
			//FLOAT
			if(strcmp(datatype, "FLOAT") == 0){
				n = rtdal_datafile_read_real(dat_input,(float *)input_data,input_lengths[0]);
				printf("Float n=%d, pointer input_data=%u\n", n, input_data);
			}
			//CHAR-BIN
			if(strcmp(datatype, "CHAR") == 0){
				n = rtdal_datafile_read_bin(dat_input,(char *)input_data,input_lengths[0]);
				printf("Char n=%d, pointer input_data=%u\n", n, input_data);
			}
		}
		if(strcmp(ctrl.inputMode,"FILE_BIN")==0){
			datatype=IN_TYPE;
			if(strcmp(datatype, "CHAR") == 0)input_sample_sz=sizeof(char);
			if(strcmp(datatype, "FLOAT") == 0)input_sample_sz=sizeof(float);
			if(strcmp(datatype, "COMPLEXFLOAT") == 0)input_sample_sz=sizeof(_Complex float);
			input_lengths[0]=ctrl.block_length*ctrl.run_times;
			n = rtdal_datafile_read_bin(dat_input,input_data,input_lengths[0]*input_sample_sz);

		}
	}
*/


int write_TEXT_file(char *output_name, FILE *p_output_file, void *out_data,  int length, char *data_type){

	_Complex float *p2Cfloat;
	float *p2float;

	if (output_name) {
//		printf("output_name = %s\n",output_name);			//LINE ADDED
//		printf("0____p_output_file=%d\n", p_output_file);
		p_output_file = rtdal_datafile_open(output_name, "w");
//		printf("1____p_output_file=%d\n", p_output_file);
		if (!p_output_file) {
			printf("Error opening OUTPUT file %s\n",output_name);
			exit(1);
		}
		//COMPLEXFLOAT
		if(strcmp(data_type, "COMPLEXFLOAT") == 0){
			p2Cfloat=(_Complex float *)out_data;
			printf("output_lengths[0]=%d, length=%d\n", output_lengths[0], length);
			rtdal_datafile_write_complex(p_output_file, p2Cfloat, output_lengths[0]);
		}
		//FLOAT
		if(strcmp(data_type, "FLOAT") == 0){
			//printf("data_type %s\n",data_type);
			p2float=(float *)out_data;
			rtdal_datafile_write_real(p_output_file,p2float, output_lengths[0]);
			//printf("Complex output_lengths[0]=%d, pointer output_data=%u\n", output_lengths[0], p_output_file);
		}
		//CHAR-BIN
		if(strcmp(data_type, "CHAR") == 0){
			//printf("data_type %s\n",data_type);
			rtdal_datafile_write_bin(p_output_file,(char *)out_data, output_lengths[0]);
			//printf("Complex output_lengths[0]=%d, pointer output_data=%u\n", output_lengths[0], p_output_file);
		}
		rtdal_datafile_close(p_output_file);
	}
	return(0);
}

/*	if (dat_output_name) {
		printf("dat_output_name = %s\n",dat_output_name);			//LINE ADDED
		printf("0____dat_output=%d\n", dat_output);
		dat_output = rtdal_datafile_open(dat_output_name, "w");
		printf("1____dat_output=%d\n", dat_output);
		if (!dat_output) {
			printf("Error opening mat file %s\n",dat_output_name);
			exit(1);
		}
		printf("ctrl.outputMode %s\n",ctrl.outputMode);
		if(strcmp(ctrl.outputMode,"FILE_TEXT")==0){
			datatype=OUT_TYPE;
			//COMPLEXFLOAT
			if(strcmp(datatype, "COMPLEXFLOAT") == 0){
				printf("datatype %s\n",datatype);
				pCfloat=(_Complex float *)output_data;
				rtdal_datafile_write_complex(dat_output, pCfloat, output_lengths[0]);
				printf("Complex output_lengths[0]=%d, pointer output_data=%u\n", output_lengths[0], output_data);
			}
			//FLOAT
			if(strcmp(datatype, "FLOAT") == 0){
				pfloat=(float *)output_data;
				rtdal_datafile_write_real(dat_output,pfloat, output_lengths[0]);
				printf("Float n=%d, pointer output_data=%u\n", n, output_data);
			}
			//CHAR-BIN
			if(strcmp(datatype, "CHAR") == 0){
				rtdal_datafile_write_bin(dat_output,(char *)output_data, output_lengths[0]);
				printf("Char n=%d, pointer output_data=%u\n", n, output_data);
			}
		}
		rtdal_datafile_close(dat_output);
	}
*/

int write_BIN_file(char *output_filename, FILE *p_output_file, void *out_data,  int length, char *data_type){

	int sample_sz=0, lengthinbytes=0;

	if (output_filename) {
//		printf("output_name = %s\n",output_name);			//LINE ADDED
//		printf("0____p_output_file=%d\n", p_output_file);
		p_output_file = rtdal_datafile_open(output_filename, "w");
//		printf("1____p_output_file=%d\n", p_output_file);
		if (!p_output_file) {
			printf("Error opening OUTPUT file %s\n",output_filename);
			exit(1);
		}
	}
	if(strcmp(data_type, "CHAR") == 0)sample_sz=sizeof(char);
	if(strcmp(data_type, "FLOAT") == 0)sample_sz=sizeof(float);
	if(strcmp(data_type, "COMPLEXFLOAT") == 0)sample_sz=sizeof(_Complex float);
	lengthinbytes=length*sample_sz;
	rtdal_datafile_write_bin(p_output_file, out_data, lengthinbytes);
	rtdal_datafile_close(p_output_file);

	return (0);
}






