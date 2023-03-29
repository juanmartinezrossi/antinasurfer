/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
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

#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "data_source_sink_interfaces.h"
#include "data_source_sink_functions.h"
#include "data_source_sink.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="data_source_sink";
int run_times=1;
int block_length=36;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;

float gain=1.0;
float reffreq=1.0;
float tonefreq=1024.0;


//Module User Defined Parameters
char myfilename[1024]= "data/rfc793.txt"; //"../../../modules/data_source_sink/data/rfc793.txt";
int ctrl_flow = 0; 												// 0: No control data flow, 1: With control of data flow
int mode=0; 															// 0: Send File, 1: Sent Data Blocks

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];
long int myfilelength=0;
#define FILEMAXLENGTH  			50000000
char filebuffer[FILEMAXLENGTH];
unsigned int snd_flag=1;	
int debug=0; //0: NO DEBUG, 1: DEBUG

// Identity
#define IDNUMBER_DEFAULT	12345678
int IDnumber=12345678;
char IDfilename[1024]=".NOFILE";
char IDbuffer[256];;
long int IDlength=0;
char IDchar[STR_LEN]="44444444";

// BER
char aux[STR_LEN];
float BER=0.0;
char DATA[64*1024];

//char input_FILE[STR_LEN];

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	int i;
	int IDcapt[STR_LEN], numidsread, numidswrite;

//	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("block_length", &block_length);		//Initialized by hand or config file
	param_get_int("ctrl_flow", &ctrl_flow);
	param_get_int("debug", &debug);
	param_get_int("mode", &mode);
	param_get_string("input_FILE", &myfilename[0]);

	/* Verify control parameters */
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your data_source_sink_interfaces.h file\n");
		return -1;
	}
	if(ctrl_flow == 1)snd_flag=0;
	else snd_flag=block_length;


	/*Include the file name and line number when printing*/
	modinfo_msg("Parameter block_length is %d\n",block_length);	//Print message and parameter 
	modinfo("Parameter block_length \n");				//Print only message

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());

	printf("\033[1;33;40m\tO--------------------------------------------------------------------------------------------OO\t\033[0m\n");
	printf("\033[1;33;40m\tO    SPECIFIC PARAMETERS SETUP: %s                                   	\t\033[0m\n", mname);
	printf("\033[1;33;40m\tO      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s                    \t\033[0m\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("\033[1;33;40m\tO      block_length=%d, ctrl_flow=%d, snd_flag=%d                                             \t\033[0m\n", block_length, ctrl_flow, snd_flag);
	printf("\033[1;33;40m\tO      mode=%d [0: Send File, 1: Sent Data Blocks], File Name=%s		\t\033[0m\n", mode, myfilename);
	printf("\033[1;33;40m\tO      debug=%d                                                                               \t\033[0m\n", debug);
	printf("\033[1;33;40m\tO--------------------------------------------------------------------------------------------O\t\033[0m\n");

	// SEND A DATA FILE
	if(mode==0){
		myfilelength=read_file(myfilename, filebuffer, FILEMAXLENGTH);
	}
	// SEND A DATA PATTERN
	if(mode==1){
		initFrame2send(DATA, 1024);
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
	int i;
	static int Tslot=0;
	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int rcv_samples1=get_input_samples(1); /** number of samples at itf 0 buffer */
	int snd_samples=block_length;
	int snd_samples1=sizeof(_Complex float);

	unsigned int *ctrl = in(inp,1);
	_Complex float *output1 = out(out,1);
	static long int counter=0;


	if(debug==1){
		printf("Tslot=%d || %s: rcv_samples0=%d, rcv_samples1=%d\n", Tslot, mname, rcv_samples, rcv_samples1);
	}

	//Check control interface
	if(ctrl_flow == 1){
		snd_samples=0;
		if(rcv_samples1 > 0){
			snd_flag=((unsigned int)(*ctrl))&0xFFFF;
			snd_samples=snd_flag;
		}
		if(debug==1)printf("Tslot=%d || DATA_SOURCE_SINK:rcv_samplesDATA=%d, rcv_samplesCTRL=%d, CTRL=%d\n", Tslot, rcv_samples, rcv_samples1, snd_flag);
	}
	// SEND DATA FILE
	if(mode==0){
		BER=-1.0;
		memcpy(out, (char *)(filebuffer+counter), snd_samples);
		counter += snd_samples;
		// Compare received data
		if(rcv_samples>0){
			if(compare_data(filebuffer, myfilelength, inp, rcv_samples, &BER)==-1){
				return(-1);
			}
		}
/*		if(snd_samples>0){
			printf("OUTPUT0\n");
			for(i=0; i<32; i++){
				printf("%x", ((int)*(out+i))&0xFF);
			}
			printf("\n");
		}
*/
	}
	// SEND DATA PATTERN
	if(mode==1){
		BER=-1.0;
		sendDATA(DATA, out, snd_samples);
		if(compare_data1(DATA, inp, rcv_samples, &BER)==-1)return(-1);
/*		printf("OUTPUT0\n");
		for(i=0; i<32; i++){
			printf("%x", ((int)*(out+i))&0xFF);
		}
		printf("\n");
*/
	}
	if(debug==1){
		printf("TTslot=%d || %s: snd_samples=%d\n", Tslot, mname, snd_samples);
		for(i=0; i<32; i++){
			printf("%c", (char) *(out+i));
		}
	}
	// OUTPUT 1
	snd_samples1=0;
	if(rcv_samples > 0 && BER >= 10e-6){
		*output1=BER+10e-5*I;
		snd_samples1=1;
/*		printf("Tslot=%d, pout=%p, pOUTPUT=%p, BER=%f, sizeofCOMPLEX=%d, snd_samples=%d\n", 
					Tslot, out, output1, BER, sizeof(_Complex float), snd_samples1);
		printf("OUTPUT1\n");
		printf("BER=%f\n", __real__ *output1);
		for(i=0; i<32; i++){
			printf("%x", ((int)*(output1+i))&0xFF);
		}
		printf("\n");
*/
	}
	set_output_samples(1, snd_samples1);

	// UPDATE TSLOT
	Tslot++;
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


