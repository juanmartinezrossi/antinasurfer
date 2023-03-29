/* 
 * Copyright (c) 2012
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

/* Functions that generate the test data fed into the DSP modules being developed */


#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <jack/jack.h>
#include <sys/time.h>
#include <phal_sw_api.h>

#include "print_utils.h"
#include "skeletonSK15.h"
#include "DAC_JACK1_functions.h"

extern MODparams_t oParam;


int NsamplesOUT=0, NsamplesIN=0; 
int SamplingFrequency=48000, timeSlotLength=0;


jack_port_t *input_port1, *input_port2;
jack_port_t *output_port1, *output_port2;
jack_client_t *client;

static void signal_handler(int sig)
{
	jack_client_close(client);
	fprintf(stderr, "signal received, exiting ...\n");
	exit(0);
}

void stop_jack() {
	jack_client_close(client);

}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg)
{
	exit (1);
}


/*
 * Generate different and alternated tones to left and right outputs.
 */
void audiodac_output_test(sample_t *out1, sample_t *out2, jack_nframes_t nframes, paTestData *data){
	int i;
	static int j=0;

	for( i=0; i<nframes; i++ ){
        if(j>MAXJ/2){
			out1[i] = data->sine[data->left_phase];  /* left */
			out2[i] = 0;
		}
		else {
			out1[i] = 0;
        	out2[i] = data->sine[data->right_phase];  /* right */
		}
		
		data->left_phase += 3;
		if( data->left_phase >= TABLE_SIZE ) data->left_phase -= TABLE_SIZE;
		data->right_phase += 5; /* higher pitch so we can distinguish left and right. */
		if( data->right_phase >= TABLE_SIZE ) data->right_phase -= TABLE_SIZE;
    }
	j++;
	if(j>=MAXJ)j=0;
}




#define IDLE			0
#define DEBUGTX			1
#define DEBUGRXTX		2
#define NORMAL			3
int exec_mode=DEBUGTX; //DEBUGRXTX;
/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client follows a simple rule: when the JACK transport is
 * running, copy the input port to the output.  When it stops, exit.
 */

int process (jack_nframes_t nframes, void *arg)
{
	static int first=0;

	jack_default_audio_sample_t *out1, *out2;
	jack_default_audio_sample_t *in1, *in2;
	paTestData *data = (paTestData*)arg;

	out1 = (jack_default_audio_sample_t*)jack_port_get_buffer (output_port1, nframes);
	out2 = (jack_default_audio_sample_t*)jack_port_get_buffer (output_port2, nframes);

	in1 = (jack_default_audio_sample_t*)jack_port_get_buffer (input_port1, nframes);
	in2 = (jack_default_audio_sample_t*)jack_port_get_buffer (input_port2, nframes);

	if(exec_mode==DEBUGTX)audiodac_output_test(out1, out2, nframes, data);
	if(exec_mode==DEBUGRXTX){
		memcpy(out1, in2, sizeof(jack_default_audio_sample_t) * nframes);
		memcpy(out2, in1, sizeof(jack_default_audio_sample_t) * nframes);
	}

	if(first == 0){
		printf("NofFrames=%d\n", nframes);
		first=1;
	}
return 0;      
}














int jack_init() {

	const char **ports;
	const char *client_name = "ALOE";
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;
//	paTestData data;
	int aux;

//	printf("SNDCARD_INIT\n");

	/* Initialize CBufferOUT*/
//	initCbuff(&out_buffCtrl, CBuffer_OUT, CBUFFER_SZ, 0);

	
	/* Capture input/output buffer pointers*/
//	input = (_Complex float *)&cfg->dacinbuff[0][0];
//	output = (_Complex float *)&cfg->dacoutbuff[0][0];

//	syncFunction=sync;

	/* Generate Test Signal*/
//	gensintable();


	/* open a client connection to the JACK server */
	client = jack_client_open(client_name, options, &status, server_name);
	if (client == NULL) {
		fprintf(stderr, "jack_client_open() failed, "
			"status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf(stderr, "Unable to connect to JACK server\n");
		}
		return -1;
	}
	if (status & JackServerStarted) {
		fprintf(stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf(stderr, " Unique name `%s' assigned\n", client_name);
	}

	/* tell the JACK server to call `process()' whenever
	 there is work to be done.
	 */
	jack_set_process_callback(client, process,  0);	

	/* tell the JACK server to call `jack_shutdown()' if
	 it ever shuts down, either entirely, or if it
	 just decides to stop calling us.
	 */
	jack_on_shutdown(client, jack_shutdown, 0);





//	jack_set_error_function(error);

/*	if (jack_set_buffer_size(client,2048)) {
		 printf("ERROR setting buffer size to %d. It should not excess 2048\n",cfg->NsamplesIn);
		 printf("Please, correct the blocksize value at hw_api/lnx/cfg/dacsound.conf file currently being used\n");
		 return -1;
	 } 	
*/
	

//	NsamplesIN=cfg->NsamplesIn;
//	NsamplesOUT=cfg->NsamplesOut; 	

	/* display the current sample rate.
	 */
	printf("\nO__SOUND CARD PROPERLY INITIALLIZED___________________________________________________________________ \n");
	printf("O  NsamplesIn=%d, NsamplesOut=%d                                                                  O \n", 
			NsamplesIN, NsamplesOUT);
	printf("O  DAC buffer size = %" PRIu32 ", DAC sample rate: %" PRIu32"                                                     O \n",
			jack_get_buffer_size(client),  jack_get_sample_rate(client));

//	SamplingFrequency = (int)(cfg->inputFreq);
	timeSlotLength=(int)(1000000.0*((float)(NsamplesOUT)/(float)(SamplingFrequency)));			

	printf("timeSlotLength=%d (us), cfg->NsamplesOut=%d, SamplingFrequency=%d\n", timeSlotLength, NsamplesOUT, SamplingFrequency);

	//printf("O  WARNING; Required TimeSlot=%d (us). Please, setup accordingly at hw_api/lnx/cfg/platform.conf   O \n", *timeSlotLength);
	printf("O  WARNING!!!; Required TimeSlot=%d (us). Please, setup accordingly at platformJACK.conf   O \n", timeSlotLength);
	printf("O__SOUND CARD PROPERLY INITIALLIZED___________________________________________________________________O \n");
	
/*	jack_set_sample_rate_callback(client, srate, 0);

	jack_set_buffer_size_callback(client, bufferchg, 0);

	if (jack_set_buffer_size(client,NofSamples)) {
		 printf("error setting buffer size to %d\n",NofSamples);
		 return -1;
	 } 	 	jackctl_server_t 
*/

	/* create two input and two output ports */
	printf("JACK Create Input Ports\n");
	//INPUTS
	input_port1 = jack_port_register (client, "input1",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsInput, 0);
	input_port2 = jack_port_register (client, "input2",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsInput, 0);
	if ((input_port1 == NULL) || (input_port2 == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (1);
	}	

	//OUTPUT PORTS
	printf("JACK Create Output Ports\n");
	output_port1 = jack_port_register (client, "output1",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);
	output_port2 = jack_port_register (client, "output2",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);
	if ((output_port1 == NULL) || (output_port2 == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (1);
	}

	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */
	printf("JACK Activate Client\n");
	if (jack_activate(client)) {
		printf("JACK Client cannot be activated\n");
		fprintf(stderr, "cannot activate client");
//		return -1;
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */
 	//OUTPUT
	printf("JACK Connect Output Ports\n");
	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (1);
	}
	if (jack_connect (client, jack_port_name (output_port1), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}
	if (jack_connect (client, jack_port_name (output_port2), ports[1])) {
		fprintf (stderr, "cannot connect output ports\n");
	}
	jack_free (ports);
	//INPUT
	printf("JACK Connect Input Ports\n");
	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (1);
	}
	if (jack_connect (client, ports[0], jack_port_name (input_port1))) {
		fprintf (stderr, "cannot connect output ports\n");
	}
	if (jack_connect (client, ports[1], jack_port_name (input_port2))) {
		fprintf (stderr, "cannot connect output ports\n");
	}
	printf("JACK Free Ports\n");
	jack_free (ports);

	printf("JACK Install Sigal Handler\n");
 	/* install a signal handler to properly quits jack client */
	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	printf("JACK Install Sigal Handler End\n");
	/* keep running until the Ctrl+C */
/*	sleep(2);
	while (1) {
		sleep (1);
	}
	jack_client_close (client);
*/
	return 1;
}

























/* INIT PHASE FUNCTIONS #####################################################################################################*/

void print_params(char *mnamel, MODparams_t oParaml){

//	printf(RESET BOLD T_WHITE "==============================================================================================\n");
	printf(RESET);
	printf(RESET BOLD T_LIGHTRED		"    INITIALIZED PARAMS FOR: %s  \n", mnamel);
	printf(RESET BOLD T_GREEN			"      opMODE=%d, num_operations=%d, constant=%3.3f\n", oParaml.opMODE, oParaml.num_operations, oParaml.constant);

	printf(RESET BOLD T_GREEN			"      datatext=%s\n", oParaml.datatext);
	printf(RESET BOLD T_BLACK "==============================================================================================\n");
}

/**
 * @check_config_params(). Check any value that should be checked and act accordingly before RUN phase
 * @param.
 * @oParaml Refers to the struct that containts the module parameters
 * @return: Void
 */
void check_config_params(MODparams_t *oParaml){

	if (oParaml->num_operations > MAXOPERATIONS) {
		printf("ERROR: p_num_operations=%d > MAXOPERATIONS=%d\n", oParaml->num_operations, MAXOPERATIONS);
		printf("Check your module_interfaces.h file\n");
		exit(EXIT_FAILURE);
	}
}

/**
 * @init_functionA_COMPLEX(). Generates and array of complex float
 * @param.
 * @input: Pointer to the complex float array
 * @length: Number of samples generated
 * @return: 1
 */
int init_functionA_COMPLEX(_Complex float *input, int length){
	int i;

	for (i=0;i<length;i++) {
		__real__ input[i]=(float)(i);
		__imag__ input[i]=(float)(-i);;
	}
	return(1);
}

/**
 * @init_functionB_FLOAT(). Generates and array of floats
 * @params.
 * @input: Pointer to the complex float array
 * @length: Number of samples generated
 * @return: 1
 */
int init_functionB_FLOAT(float *input, int length){
	int i;

	for (i=0;i<length;i++) {
		if(i>20) input[i]=(float)(i*i);
	}
	return(1);
}


/* WORK PHASE FUNCTIONS #####################################################################################################3*/
/**
 * @functionA_COMPLEX(): Copy length samples from input to output
 * @param.
 * @input: 
 * @inlengths Save on n-th position the number of samples generated for the n-th interface
 * @return the number of samples copied
 */
int functionA_COMPLEX(_Complex float *input, int inlength, _Complex float *output){
	int i,outlength;
	static int z=0;

	for (i=0;i<inlength;i++) {
		__real__ output[i] = __real__ input[i];
		__imag__ output[i] = __imag__ input[i];
	}
	z++;
	outlength=inlength;
	return outlength;
}

int functionB_FLOAT(float *input, int inlength,float *output){
	int i, outlength;

	for (i=0;i<inlength;i++) {
		output[i] = input[i];
	}
	outlength=inlength;
	return outlength;
}


int functionC_INT(int *in, int length, int *out){
	int i;

	for(i=0; i<length; i++){
		*(out+i)=*(in+i);
	}
	return 1;
}


int functionD_CHAR(char *in, int length, char *out){
	int i;

	for(i=0; i<length; i++){
		*(out+i)=*(in+i);
	}
	return 1;
}

/**
 * @bypass_func(): Copy length samples from input to output
 * @param.
 * @datatypeIN: Identifies the input data type 
 * @datatypeOUT: Identifies the output data type 
 * @datalength: Number of samples to be copied 
 * @return 1
 */
int bypass_func(char *datatypeIN, char *datatypeOUT, void *datin, int datalength, void *datout){

	if(strcmp(datatypeIN,datatypeOUT) != 0){
		printf("ERROR!!!: Bypass not possible. Input and Output data types are different\n");
		printf("ERROR!!!: Please, check the IN_TYPE and OUT_TYPE in module_interfaces.h file.\n");
	}
	if(strcmp(datatypeIN,"COMPLEXFLOAT")==0){
		functionA_COMPLEX((_Complex float *)datin, datalength, (_Complex float *)datout);
	}
	if(strcmp(datatypeIN,"FLOAT")==0){
		functionB_FLOAT((float *)datin, datalength, (float *)datout);
	}
	if(strcmp(datatypeIN,"INT")==0){
		functionC_INT((int *)datin, datalength, (int *)datout);
	}
	if(strcmp(datatypeIN,"CHAR")==0){
		functionD_CHAR((char *)datin, datalength, (char *)datout);
	}
	return(1);
}





