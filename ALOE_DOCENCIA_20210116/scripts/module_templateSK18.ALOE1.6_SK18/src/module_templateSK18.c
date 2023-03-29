/**
 * Module Name: provaExt
 * Description:
 * Files: module_templateSK18.c
 *
 * Author: me
 * Created: Wed Jan 31 08:57:10 2018
 * Revision: 0.1
 *
 * Comments:
 *
 */

#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include <phal_sw_api.h>
#include <math.h>
#include <swapi_utils.h>

#include "typetools.h"
#include <skeletonExtSK18.h>
#include "module_templateSK18_functions.h"

/* MODULE DEFINED PARAMETERS.*/
char mname[64]="module_templateSK18";
extern struct printopt printopts;
extern MODparams_t oParam;
unsigned long int Tslot=0;			/* Time slot number */

//Global Variables
int my_variable;
_Complex float buffer[1024];


/** Initialization function. This code is executed during the INIT phase only.
 * @return 0 fails initialization and stops execution of application.
 */
int initialize(){

	/* DEBUGGING: Print data interfaces for debugging purposes*/
	/*void print_itfs_setup(int mode, int ncolums, int TS2print, int nprintTS, int offset, int maxlen)*/
	/** @mode: Options: NOPRINT: 0, DATARECEIVED: 1, DATASENT:2, ALL: 3
	 * 	@ncolums: Number of printed data columns format
	 * 	@TS2print: Initial Printing Time-slot
	 * 	@nprintTS: Number of printing Time-slots
	 * 	@offset: pointer offset to print
	 * 	@maxlen: maximum number of samples to print
	*/
	/* If included this line overwrite the .params file configuration */
	//print_itfs_setup(3, 8, 0, 5, 0, 64);

	/* Capture the name of the module in current APP*/
	sprintf(mname, "%s", GetObjectName());

	/* Print Module Init Parameters */
	/* print_params(char *fore, char *ground)*/
	/* "fore" refers to text color and "ground" to background color*/
	/* fore and ground with values from http://www.bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux*/
	print_params("000", "115");					

	/* or for output interfaces use set_special_type_OUT() *
	/* Verify control parameters */
	check_config_params();

	/* Example initialization function */
	init_functionA_COMPLEX(buffer, 27);

	return 1;
}


/**
 * @brief Function documentation
 *
 * @param No params
 *
 * @usefulfunctions: in_p(idx). Returns input interface buffers address (pointer). 
 * Use in_p(idx) to access the buffer address of input index idx. 
 * 
 * @usefulfunctions: get_input_samples(int idx). To obtain the number of received samples use the function
 * int get_input_samples(int idx) where idx is the interface index.
 *
 * @usefulfunctions: out_p(idx). Returns output interface buffer address (pointer)
 * Use out_p(idx) to access the address the buffer address of output index idx. 
 *
 * @usefulfunctions:  set_output_samples(int idx, int len). To specify the length in samples
 * for certain interface, use the function set_output_samples(int idx, int len)
 * 
 * @return 1 On success, On error returns -1, stops execution.
 *
 * @code
 * 	input_t *first_interface = in_p(0);
		input_t *second_interface = in_p(1);
		output_t *first_output_interface = out_p(0);
		output_t *second_output_interface = out_p(1);
		etc..
 */

int work() {

	/** Create pointer with the appropriate datatype for the input itfs and according XX_YYSK18_functions.h file */
	char *input0 = in_p(0);
	int *input1 = in_p(1);
	float *input2 = in_p(2);
	_Complex float *input3 = in_p(3);
	/** Create pointer with the appropriate datatype for the output itfs and according XX_YYSK18_functions.h file */
	char *output0 = out_p(0);
	int *output1 = out_p(1);
	float *output2 = out_p(2);
	_Complex float *output3 = out_p(3);

	int snd_samples0=0, snd_samples1=0, snd_samples2=0, snd_samples3=0;
	int rcv_samples0=get_input_samples(0);	// Capture the received samples for interface 0.
	int rcv_samples1=get_input_samples(1);	// Capture the received samples for interface 1.
	int rcv_samples2=get_input_samples(2);	// Capture the received samples for interface 2.
	int rcv_samples3=get_input_samples(3);	// Capture the received samples for interface 3.

	int i;
	int length0;



	/* DO YOUR CHECKS BEFORE DSP TASKS
	//	if (rcv_samples == 0)return(1); 				// Only as example. Check if data received: Nothing to do if no data received ??
	//  if (wrong status) return (0);					// Return 0 if wrong condition for normal operation

	// Check if data exceed maximum expected data
	if (rcv_samples > oParam.num_operations) {
		...
		printf("Wrong condition\n");
		return(0);
	}
	*/

	/** your code goes here */
	for (i=0;i<16;i++) {
		output0[i]=(char)(Tslot+i)%8;
	}
	snd_samples0=16;

	for (i=0;i<16;i++) {
		output1[i]=input1[i]+Tslot+2*i;
	}
	snd_samples1=16;

	for (i=0;i<16;i++) {
		output2[i]=(float)(Tslot+i);
	}
	snd_samples2=16;

	for (i=0;i<16;i++) {
		output3[i]=(_Complex float)(Tslot+(2*i)*I);
	}
	snd_samples3=16;

	/** Define the number of output samples for all output interfaces*/
	set_output_samples(0, snd_samples0);
	set_output_samples(1, snd_samples1);
	set_output_samples(2, snd_samples2);
	set_output_samples(3, snd_samples3);
	/** Increase Time Slot counter */
	Tslot++;
	return 1;
}




