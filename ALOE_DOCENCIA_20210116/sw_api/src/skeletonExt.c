#include <phal_sw_api.h>
#include <swapi_utils.h>
#include <typetools.h>

#include "moduleConfig.h"
#include "skeletonExtVars.h"

// If cuda_interface_enabled then include CUDA library interfaces
#ifdef CUDA_ENABLED
#include "cudalib_interface.h"
#endif

/* GLOBAL BUFFERS*/
char input_itf_str[NUM_INPUT_ITFS][64];
char output_itf_str[NUM_OUTPUT_ITFS][64];
char input_itf_str2[NUM_INPUT_ITFS][64];
char output_itf_str2[NUM_OUTPUT_ITFS][64];
char input_itf_str3[NUM_INPUT_ITFS][64];
char output_itf_str3[NUM_OUTPUT_ITFS][64];

char inputbuffer[NUM_INPUT_ITFS][BUFFER_INPUT_KBYTES * 1024];
char outputbuffer[NUM_OUTPUT_ITFS][BUFFER_OUTPUT_KBYTES * 1024];

/* no m'agrada que amb el typetools com el tenim necessitem el doble de buffers pero bueno */
char inputbufferProc[NUM_INPUT_ITFS * BUFFER_INPUT_KBYTES * 1024];
char outputbufferProc[NUM_OUTPUT_ITFS * BUFFER_OUTPUT_KBYTES * 1024];


/* GLOBAL VARIABLES */
int inputLen[NUM_INPUT_ITFS];
int outputLen[NUM_OUTPUT_ITFS];
int inputNsamples[NUM_INPUT_ITFS];
int outputNsamples[NUM_OUTPUT_ITFS];
int dataTypesIn[NUM_INPUT_ITFS];
int dataTypesOut[NUM_OUTPUT_ITFS];
int transportTypesIn[NUM_INPUT_ITFS];
int transportTypesOut[NUM_OUTPUT_ITFS];
int lengthTypesIn[NUM_INPUT_ITFS];
float inputgains[NUM_INPUT_ITFS];
float outputgains[NUM_OUTPUT_ITFS];
char ingainsstr[NUM_INPUT_ITFS][64];
char outgainsstr[NUM_OUTPUT_ITFS][64];
int time_slot;

/* PREDEFINED FUNCTIONS*/
int rcv_input(int len);
int get_input_len(void);
void print_interface(int printMode, struct printopt *opts, void *buffer, int type, int length,
		char *Itfname);
void print_array(void *x, int offset, int Dtype, int Dlength, int Dcolums);
void print_config(struct printopt *opts, char * print_buffer);

/** EXTERNAL VARIABLES*/
extern struct utils_variables my_vars[];
extern int currentInterfaceIdx;

char aux[64];

/** MACROS */
#define INS(b) &inputbufferProc[b*BUFFER_INPUT_KBYTES*1024]
#define OUTS(b) &outputbufferProc[b*BUFFER_OUTPUT_KBYTES*1024]

#define addStat(a,b,c,d,e) \
		stats[j].name = a; \
		stats[j].type = b; \
		stats[j].size = c; \
		stats[j].id = &stat_ids[j]; \
		stats[j].value = d; \
		stats[j].update_mode = e; \
		j++;

#define endStats stats[j].name=NULL;

/*-----------------------  SQUELETON CALLED FUNCTIONS ------------------------*/
/** This function is called by background skeleton after initializing parameters,
 * before creating interfaces. Here we will define the input and output interfaces,
 * declare signals to be used by stats and configure specific interface parameters.
 */
void ConfigInterfaces() {
	int i;
	int j=0;

	/** configure inputs */
	memset(input_itfs, 0, sizeof(struct utils_itf) * NUM_INPUT_ITFS);
/*AGB DEC16*/
	if (!InitParamFile()) {
		printf("WARNING!!! initiating parameters from %s.params file\n", GetObjectName());
	}
/*AGBDEC16*/
	for (i = 0; i < NUM_INPUT_ITFS; i++) {
		/** get special transport type, if any */
		sprintf(aux, "%s_%d", INPUT_TYPE_STR, i);
		if (!GetParam(aux, &transportTypesIn[i], STAT_TYPE_INT, 1)) {
			transportTypesIn[i] = transporttypeIN;
		}

		/** get special length, if any */
		sprintf(aux, "%s_%d", INPUT_LENGTH_STR, i);
		if (!GetParam(aux, &lengthTypesIn[i], STAT_TYPE_INT, 1)) {
			lengthTypesIn[i] = inputdatalength;
		}

		/** get special gain, if any */
		sprintf(ingainsstr[i], "%s_%d", INPUT_GAIN_STR, i);
		if (!GetParam(ingainsstr[i], &inputgains[i], STAT_TYPE_FLOAT, 1)) {
			inputgains[i] = inputgain;
		}

		sprintf(input_itf_str[i], "input%d", i);
		input_itfs[i].name = input_itf_str[i];
		input_itfs[i].sample_sz = 1;
		input_itfs[i].max_buffer_len = BUFFER_INPUT_KBYTES * 1024;
		input_itfs[i].buffer = inputbuffer[i];
		input_itfs[i].process_fnc = rcv_input;
		dataTypesIn[i] = PROCESSING_TYPE_IN;
		if (lengthTypesIn[i]) {
			input_itfs[i].get_block_sz = get_input_len;
		}

		/** configure stats for input interfaces */
/*		addStat(input_itf_str[i],type2stats(transporttypeIN),VIEW_CAPTURE_SAMPLES,&inputbuffer[i][VIEW_CAPTURE_OFFSET],WRITE);
*/
		/** input after type conversion */
		sprintf(input_itf_str2[i], "input%d", i);
		addStat(input_itf_str2[i],type2stats(dataTypesIn[i]),VIEW_CAPTURE_SAMPLES,INS(i),WRITE);

		sprintf(input_itf_str3[i], "input%d_nsamples", i);
		addStat(input_itf_str3[i],STAT_TYPE_INT,1,&inputNsamples[i],WRITE);

		/* scale variable */
		addStat(ingainsstr[i],STAT_TYPE_FLOAT,1,&inputgains[i],READ);
	}
	input_itfs[i].name = NULL;

	/** configure outputs */
	memset(output_itfs, 0, sizeof(struct utils_itf) * NUM_OUTPUT_ITFS);
	for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
		sprintf(output_itf_str[i], "output%d", i);
		output_itfs[i].name = output_itf_str[i];
		output_itfs[i].sample_sz = 1;
		output_itfs[i].max_buffer_len = BUFFER_OUTPUT_KBYTES * 1024;
		output_itfs[i].buffer = outputbuffer[i];
		dataTypesOut[i] = PROCESSING_TYPE_OUT;

		/** get special transport type, if any */
		sprintf(aux, "%s_%d", OUTPUT_TYPE_STR, i);
		if (!GetParam(aux, &transportTypesOut[i], STAT_TYPE_INT, 1)) {
			transportTypesOut[i] = transporttypeOUT;
		}

		/** get special gain, if any */
		sprintf(outgainsstr[i], "%s_%d", OUTPUT_GAIN_STR, i);
		if (!GetParam(outgainsstr[i], &outputgains[i], STAT_TYPE_FLOAT, 1)) {
			outputgains[i] = outputgain;
		}

		/* type before conversion */
		sprintf(output_itf_str2[i], "output%d", i);
		addStat(output_itf_str2[i],type2stats(dataTypesOut[i]),VIEW_CAPTURE_SAMPLES,OUTS(i),WRITE);

		sprintf(output_itf_str3[i], "output%d_nsamples", i);
		addStat(output_itf_str3[i],STAT_TYPE_INT,1,&outputNsamples[i],WRITE);

		/* scale variable */
		addStat(outgainsstr[i],STAT_TYPE_FLOAT,1,&outputgains[i],READ);
	}

	output_itfs[i].name = NULL;

	for (i = 0; my_vars[i].name && i < MAX_VARS; i++) {
		/** get param */
		if (!GetParam(my_vars[i].name, my_vars[i].value, my_vars[i].type,
				my_vars[i].size)) {
			Log("Caution parameter %s not initated\n", my_vars[i].name);
		}

		/* copy stats structure, will be initialized later */
		addStat(my_vars[i].name,my_vars[i].type,my_vars[i].size,my_vars[i].value,my_vars[i].update_mode);
	}

	endStats;

}

/** Number of samples to read from each interface. Called before receiving data.
 */
int get_input_len(void) {
	return lengthTypesIn[currentInterfaceIdx];
}
/** Called after data is received, we save the number of bytes that have been received.
 */
int rcv_input(int len) {
	inputNsamples[currentInterfaceIdx] = typeNsamplesArray(dataTypesIn[currentInterfaceIdx],len);
	inputLen[currentInterfaceIdx] = len;
	return 1;
}
/** User function to get the length, in number of samples of each interface.
 * Since this function is called from execute(), after type conversion, the value
 * in inputLen[] is in samples.
 */
int getLength(int itfIndex) {
	return inputLen[itfIndex];
}
/** User function to set the number of samples to send through an interface.
 */
void setLength(int itfIndex, int len) {
	outputNsamples[itfIndex] = typeNsamplesArray(dataTypesIn[itfIndex],len);
	outputLen[itfIndex] = len;
}

/** User function to set an special type for an input interface */
void set_special_type_IN(int itfIndex, int type) {
	dataTypesIn[itfIndex] = type;
}

/** User function to set an special type for an output interface */
void set_special_type_OUT(int itfIndex, int type) {
	dataTypesOut[itfIndex] = type;
}

/** RunCustom() function process the received data by calling the user processing function.
 *  return 1 if ok, 0 if error.
 */
int RunCustom() {
	int n, i;

	//printf("%s.RunCustom(): %d\n", GetObjectName(), printopts.mode);

	time_slot++;

	/** Print received data*/
	if (printopts.mode & DATARECEIVED) {
		for (i = 0; i < NUM_INPUT_ITFS; i++) {
			//printf("%s.RunCustom(): inputLen[%d]=%d\n", GetObjectName(), i, inputLen[i]);
			print_interface(DATARECEIVED, &printopts, inputbuffer[i], transportTypesIn[i],
					typeNsamplesArray(transportTypesIn[i], inputLen[i]),
					input_itf_str[i]);
		}
	}

#ifndef CUDA_ENABLED // Preprocessor directive to compile the executable CUDA enabled or not
	/** change types from transport_in to PROCESSING_TYPE_IN */
	for (i = 0; i < NUM_INPUT_ITFS; i++) {
		if (transportTypesIn[i] != dataTypesIn[i] || inputgains[i]!=1.0) {
			inputLen[i] = type2type(inputbuffer[i], INS(i),
					transportTypesIn[i], dataTypesIn[i], typeNsamplesArray(
							transportTypesIn[i], inputLen[i]), inputgains[i]);
			if(inputLen[i]<0)printf("%s TYPE2TYPE: Transport(type=%d) to Processing(type=%d) Conversion ERROR: Output %d\n",\
							GetObjectName(), transporttypeIN, PROCESSING_TYPE_IN, i);
		} else {
			memcpy(INS(i), inputbuffer[i], inputLen[i]);
			inputLen[i] = typeNsamplesArray(transportTypesIn[i], inputLen[i]);
		}
	}
#else
	/** change types from transport_in to PROCESSING_TYPE_IN */
	for (i = 0; i < NUM_INPUT_ITFS; i++) {
		if (transportTypesIn[i] != dataTypesIn[i] || inputgains[i]!=1.0) {
			inputLen[i] = type2type(inputbuffer[i], INS(i),
					transportTypesIn[i], dataTypesIn[i], typeNsamplesArray(
							transportTypesIn[i], inputLen[i]), inputgains[i]);
			if (cuda_interface_enabled==1)
				setInputData((float*)INS(i), i, sizeof(float)*inputLen[i]);
			if (inputLen[i]<0)
				printf("%s TYPE2TYPE: Transport(type=%d) to Processing(type=%d) Conversion ERROR: Output %d\n",\
							GetObjectName(), transporttypeIN, PROCESSING_TYPE_IN, i);
		} else {
			if (cuda_interface_enabled==1) setInputData((float*)inputbuffer[i], i, inputLen[i]);
			else memcpy(INS(i), inputbuffer[i], inputLen[i]);
			inputLen[i] = typeNsamplesArray(transportTypesIn[i], inputLen[i]);
		}
	}
#endif

	/** Print processing in data*/
	if (printopts.mode & PROCESS_IN) {
		for (i = 0; i < NUM_INPUT_ITFS; i++) {
			print_interface(PROCESS_IN, &printopts, INS(i), dataTypesIn[i], inputLen[i],
					input_itf_str[i]);
		}
	}

	/** run user-code */
#ifndef CUDA_ENABLED
	n = execute();
#else
	if (cuda_interface_enabled==1) n = cuexecute(inputLen[0]);
	else n = execute();
#endif
	if (n < 0)
		return 0;

	/** set output length to a default value */
	for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
		if (!outputLen[i]) {
			outputNsamples[i] = n;
			outputLen[i] = n;
		}
	}

	/** Print processing out data*/
	if (printopts.mode & PROCESS_OUT) {
		for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
			print_interface(PROCESS_OUT, &printopts, OUTS(i), dataTypesOut[i], outputLen[i], output_itf_str[i]);
		}
	}

#ifndef CUDA_ENABLED // Preprocessor directive to compile the executable CUDA enabled or not
	/** change types from PROCESSING_TYPE_OUT to transport_out */
	for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
		if (transportTypesOut[i] != dataTypesOut[i] || outputgains[i]!=1.0) {
			outputLen[i] = type2type(OUTS(i), outputbuffer[i], dataTypesOut[i],
					transportTypesOut[i], outputLen[i], outputgains[i]);
			if(outputLen[i]<0){
				printf("%s TYPE2TYPE: Processing(type=%d) to Transport(type=%d) Conversion ERROR: Output %d\n",\
							GetObjectName(), PROCESSING_TYPE_OUT, transporttypeOUT, i);
				printf("%s -> Transport: %d; Processing: %d\n", GetObjectName(), transportTypesOut[i], dataTypesOut[i]);
				/*transportTypesOut[i] = 4;
				dataTypesOut[i] = 4;*/
			}
		} else {
			memcpy(outputbuffer[i], OUTS(i), typeSizeArray(
					transportTypesOut[i], outputLen[i]));
		}
	}
#else
	/** change types from PROCESSING_TYPE_OUT to transport_out */
	for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
		if (transportTypesOut[i] != dataTypesOut[i] || outputgains[i]!=1.0) {
			if (cuda_interface_enabled==1)
				getOutputData((float*)OUTS(i), i, typeSizeArray(transportTypesOut[i], outputLen[i]));
			outputLen[i] = type2type(OUTS(i), outputbuffer[i], dataTypesOut[i],
					transportTypesOut[i], outputLen[i], outputgains[i]);
			if(outputLen[i]<0)
				printf("%s TYPE2TYPE: Processing(type=%d) to Transport(type=%d) Conversion ERROR: Output %d\n",\
							GetObjectName(), PROCESSING_TYPE_OUT, transporttypeOUT, i);
		} else {
			if (cuda_interface_enabled==1){
				getOutputData((float*)outputbuffer[i], i, typeSizeArray(transportTypesOut[i], outputLen[i]));
			} else {
				memcpy(outputbuffer[i], OUTS(i), typeSizeArray(transportTypesOut[i], outputLen[i]));
			}
		}
	}
#endif

	/** Print send out data*/
	if (printopts.mode & DATASENT) {
		for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
			//printf("%s.RunCustom(): time_slot=%d, outputLen[%d]=%d\n", GetObjectName(), GetTstamp(), i, outputLen[i]);
			print_interface(DATASENT, &printopts, outputbuffer[i], transportTypesOut[i],
					outputLen[i], output_itf_str[i]);
		}
	}

	/* send output data */
	for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
		SendItf(i, typeSizeArray(transportTypesOut[i], outputLen[i]));
	}

	/* reset len counters */
	memset(outputLen, 0, sizeof(int) * NUM_OUTPUT_ITFS);
	memset(inputLen, 0, sizeof(int) * NUM_INPUT_ITFS);

	return 1;
}

/** function called by background skeleton after all the initialization process.
 * Here we call User init function to initialize object-specific variables.
 */
int InitCustom() {
	char print_buffer [1024];
	print_buffer [0] = '\0';
	int err;

	time_slot=-1;
	print_config(&printopts, print_buffer);
	err = myinit(print_buffer);

	return err;
}

/** print_config: Prints the initial configuration for such module
 */
void print_config(struct printopt *opts, char * print_buffer)
{
#ifdef kk
	sprintf(print_buffer, "%sO------------------------------------------------------------------------------O\n", print_buffer);
	sprintf(print_buffer, "%sO    MODULE: %s.\n", print_buffer, GetObjectName());
	sprintf(print_buffer, "%sO    GENERIC PARAMETERS SETUP: %s.InitCustom.print_config().\n", print_buffer, GetObjectName());
	sprintf(print_buffer, "%sO      TransportTypeIN=%d, TransportTypeOUT=%d, InputDataLength=%d\n", print_buffer,\
				transporttypeIN, transporttypeOUT, inputdatalength);
	sprintf(print_buffer, "%sO      InputGain=%4.2f, OutputGain=%4.2f\n", print_buffer, inputgain, outputgain);
	/** Print Configuration*/
	if(opts->mode | NOPRINT){
		sprintf(print_buffer, "%sO    PRINT CONFIG: ---- Printing Length=%d \n", print_buffer, opts->maxlen);
		sprintf(print_buffer, "%sO      Print Mode: ", print_buffer);
		if(opts->mode & DATARECEIVED)	sprintf(print_buffer, "%s--DATARECEIVED", print_buffer);
		if(opts->mode & PROCESS_IN)		sprintf(print_buffer, "%s--PROCESS_IN", print_buffer);
		if(opts->mode & PROCESS_OUT)	sprintf(print_buffer, "%s--PROCESS_OUT", print_buffer);
		if(opts->mode & DATASENT)		sprintf(print_buffer, "%s--DATASENT", print_buffer);
		sprintf(print_buffer, "%s\n", print_buffer);
		sprintf(print_buffer, "%sO      Starting Time Slot=%d, Printed Time Slots=%d \n", print_buffer, opts->TS2print, opts->nprintTS);
		sprintf(print_buffer, "%sO      Colums Number=%d, Pointer Offset=%d \n", print_buffer, opts->ncolums, opts->offset);
	}
#endif
}




/** print_interface: Prints an array according their type at selected Time-Slot
 *  and between Time-Slot printingTS and Time-Slot printingTS+nprintTS.
 *  print parameter allow to desactivate printing.
 *
 */

void print_interface(int printMode, struct printopt *opts, void *buffer, int type, int length, char *Itfname) {

	int len;

	if(length == 0)return;

	if ((opts->TS2print <= time_slot) && ((opts->TS2print + opts->nprintTS)	> time_slot)) {
		if(printMode & DATARECEIVED)printf("\nO---- %s -- DATARECEIVED -- %s -- length %d ----O", GetObjectName(), Itfname, length);
		if(printMode & PROCESS_IN)printf("\nO---- %s -- PROCESS_IN -- %s -- length %d ----O", GetObjectName(), Itfname, length);
		if(printMode & PROCESS_OUT)printf("\nO---- %s -- PROCESS_OUT -- %s -- length %d ----O", GetObjectName(), Itfname, length);
		if(printMode & DATASENT)printf("\nO---- %s -- DATASENT -- %s -- length %d ----O", GetObjectName(), Itfname, length);

		len = length > opts->maxlen ? opts->maxlen : length;
		print_array(buffer, opts->offset, type, len, opts->ncolums);
		printf("---------------------------\n\n");
	}
}

void print_array(void *x, int offset, int Dtype, int Dlength, int Dcolums) {
	int i, k;
	float *input_f;
	char *input_c;
	int *input_i;
	short *input_s;

	if (Dtype == TYPE_BITSTREAM){
		input_c = (unsigned char *) x;
		Dlength=typeSizeArray(TYPE_BITSTREAM, Dlength);
	}
	if (Dtype == TYPE_BIT8)
		input_c = (unsigned char *) x;
	if (Dtype == TYPE_CHAR)
		input_c = (unsigned char *) x;
	if (Dtype == TYPE_SHORT)
		input_s = (short *) x;
	if (Dtype == TYPE_INT)
		input_i = (int *) x;
	if (Dtype == TYPE_FLOAT)
		input_f = (float *) x;

	for (i = 1; i <= Dlength; i++) {
		k=i-1;
		if (k % Dcolums == 0) {
			printf("\n");
			printf("[%03d] ", k / Dcolums);
		}
		if (Dtype == TYPE_BITSTREAM)
			printf("%03d ", (unsigned char) input_c[k + offset]);
		if (Dtype == TYPE_BIT8)
			printf("%03d ", (unsigned char) input_c[k + offset]);
		if (Dtype == TYPE_CHAR)
			printf("%03d ", (unsigned char) input_c[k + offset]);
		if (Dtype == TYPE_SHORT)
			printf("%d ", (short) input_s[k + offset]);
		if (Dtype == TYPE_INT)
			printf("%d ", (int) input_i[k + offset]);
		if (Dtype == TYPE_FLOAT)
			printf("%4.2f ", (float) input_f[k + offset]);
	}
	printf("\n");
}

#ifdef CUDA_ENABLED
void enable_cuda_interface (void * setData, void * getData, int * executeFunction){
	setInputData = setData;
	getOutputData = getData;
	cuexecute = executeFunction;
	cuda_interface_enabled = 1;
}
#endif

void printDataOformat(){
	printf("%s -> out 0 -> Transport: %d; Processing: %d\n", GetObjectName(), transportTypesOut[0], dataTypesOut[0]);
	printf("%s -> out 1 -> Transport: %d; Processing: %d\n", GetObjectName(), transportTypesOut[1], dataTypesOut[1]);
}
