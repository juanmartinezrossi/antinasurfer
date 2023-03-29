#include <complex.h>
#include <phal_sw_api.h>
#include <swapi_utils.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <typetools.h>		//Eliminar

#include "module_SK18_params.h"
#include "module_SK18_interfaces.h"
#include "skeletonExtVarsSK18.h"


/* GLOBAL BUFFERS*/
char input_itf_str[NUM_INPUT_ITFS][64];
char output_itf_str[NUM_OUTPUT_ITFS][64];
char input_itf_str2[NUM_INPUT_ITFS][64];
char output_itf_str2[NUM_OUTPUT_ITFS][64];
char input_itf_str3[NUM_INPUT_ITFS][64];
char output_itf_str3[NUM_OUTPUT_ITFS][64];


/* GLOBAL VARIABLES */
int inputLen[NUM_INPUT_ITFS];
int outputLen[NUM_OUTPUT_ITFS];
int dataTypesIn[NUM_INPUT_ITFS];
int dataTypesOut[NUM_OUTPUT_ITFS];
int lengthTypesIn[NUM_INPUT_ITFS];
int TimeSlot;

/* PREDEFINED FUNCTIONS*/
int rcv_input(int len);
int get_input_len(void);

void print_interface(int printMode, struct printopt *opts, void *buffer, 
										int type, int length, char *Itfname, int time_slot);
void print_array(void *x, int offset, int Dtype, int Dlength, int Dcolums);
void print_config(struct printopt *opts, char * print_buffer);

/** EXTERNAL VARIABLES*/
extern struct utils_variables my_vars[];
extern struct utils_param params[];
extern int currentInterfaceIdx;
extern struct itf_info IN_ITF_INF[];
extern struct itf_info OUT_ITF_INF[];

#define addStat(a,b,c,d,e) \
		stats[j].name = a; \
		stats[j].type = b; \
		stats[j].size = c; \
		stats[j].id = &stat_ids[j]; \
		stats[j].value = d; \
		stats[j].update_mode = e; \
		j++;

#define endStats stats[j].name=NULL;


void * in_p(int idx){
	return input_itfs[idx].buffer;
}

void * out_p(int idx){
	return output_itfs[idx].buffer;
}

void free_buffers_memory() {
	int i;
	for (i = 0; i < NUM_INPUT_ITFS; i++){
		free(input_itfs[i].buffer);
	}
	for (i = 0; i < NUM_OUTPUT_ITFS; i++){
		free(output_itfs[i].buffer);
	}
}


/*-----------------------  SQUELETON CALLED FUNCTIONS ------------------------*/
/** This function is called by background skeleton after initializing parameters,
 * before creating interfaces. Here we will define the input and output interfaces,
 * declare signals to be used by stats and configure specific interface parameters.
 */
void ConfigInterfaces() {
	int i;
	int j=0;

	printf("MY SK18 WARNING!!! initiating parameters from %s.params file\n", GetObjectName());

	/** configure inputs */
	memset(input_itfs, 0, sizeof(struct utils_itf) * NUM_INPUT_ITFS);
	if (!InitParamFile()) {
		printf("WARNING!!! initiating parameters from %s.params file\n", GetObjectName());
	}
/*AGBDEC16*/
	for (i = 0; i < NUM_INPUT_ITFS; i++) {

		sprintf(input_itf_str[i], "input%d", i);
		input_itfs[i].name = input_itf_str[i];
		input_itfs[i].sample_sz = IN_ITF_INFO[i].sample_sz;
		input_itfs[i].max_buffer_len = IN_ITF_INFO[i].max_buffer_len*IN_ITF_INFO[i].sample_sz; 
		input_itfs[i].buffer = malloc(input_itfs[i].max_buffer_len);	
		assert(input_itfs[i].buffer);
		input_itfs[i].process_fnc = rcv_input;
		dataTypesIn[i] = IN_ITF_INFO[i].sample_type; 
		if (lengthTypesIn[i]) {
			input_itfs[i].get_block_sz = get_input_len;
		}

		/** configure stats for input interfaces */
		addStat(input_itf_str[i], dataTypesIn[i],VIEW_CAPTURE_SAMPLES,&input_itfs[i].buffer[VIEW_CAPTURE_OFFSET] ,WRITE); //&inputbuffer[i][VIEW_CAPTURE_OFFSET]
	}
	input_itfs[i].name = NULL;

	/** configure outputs */
	memset(output_itfs, 0, sizeof(struct utils_itf) * NUM_OUTPUT_ITFS);
	for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
		sprintf(output_itf_str[i], "output%d", i);
		output_itfs[i].name = output_itf_str[i];
		output_itfs[i].sample_sz = OUT_ITF_INFO[i].sample_sz;
		output_itfs[i].max_buffer_len = OUT_ITF_INFO[i].max_buffer_len*OUT_ITF_INFO[i].sample_sz;
		output_itfs[i].buffer = malloc(output_itfs[i].max_buffer_len);
		assert(output_itfs[i].buffer);
		dataTypesOut[i] = OUT_ITF_INFO[i].sample_type; 

		/** configure stats for input interfaces */
		addStat(output_itf_str[i], dataTypesOut[i],VIEW_CAPTURE_SAMPLES,&output_itfs[i].buffer[VIEW_CAPTURE_OFFSET] ,WRITE); 
	}
	output_itfs[i].name = NULL;
///VARS
	for (i = 0; my_vars[i].name && i < MAX_VARS; i++) {
		/** get param */
		if (!GetParam(my_vars[i].name, my_vars[i].value, my_vars[i].type, my_vars[i].size)) {
			Log("Caution parameter %s not initated\n", my_vars[i].name);
		}
//		printf("skeletonExtSK18.c: Init(): my_vars=%s, value=%d\n", my_vars[i].name, (int)my_vars[i].value);
		/* copy stats structure, will be initialized later */
		addStat(my_vars[i].name,my_vars[i].type,my_vars[i].size,my_vars[i].value,my_vars[i].update_mode);
	}
///VARS
///PARAMS
	for (i = 0; params[i].name; i++) {
		/** get param */

		if(!GetParam(params[i].name, params[i].value, params[i].type, params[i].size)){
			Log("Caution parameter %s not initated\n", params[i].name);
		}
//		printf("skeletonExtSK18.c: Init(): Param=%s, value=%d\n", params[i].name, (int)params[i].value);
		/* copy stats structure, will be initialized later */
		addStat(params[i].name, params[i].type, params[i].size, params[i].value, WRITE);
	}
///PARAMS

	endStats;
//	printf("ConfigInterfaces() for %s finihing succesfully\n", GetObjectName());
}

/** Number of samples to read from each interface. Called before receiving data.
 */
int get_input_len(void) {
	printf("%s.get_input_len(): lengthTypesIns[%d]=%d\n", currentInterfaceIdx, lengthTypesIn[currentInterfaceIdx]);
	return lengthTypesIn[currentInterfaceIdx];
}
/** Called after data is received, we save the number of bytes that have been received.
 */
int rcv_input(int len) {
	inputLen[currentInterfaceIdx] = len;
//	printf("%s.rcv_input(): inputLen[%d]=%d\n", GetObjectName(), currentInterfaceIdx, inputLen[currentInterfaceIdx]);

	return 1;
}
/** User function to get the length, in number of samples of each interface.
 * Since this function is called from work(), the value
 * in inputLen[] is chars.
 */
int get_input_samples(int idx) {
	if(idx>=0 && idx<NUM_INPUT_ITFS)return inputLen[idx]/input_itfs[idx].sample_sz; 
	else printf("ERROR in %s: 0	> idx=%d <  NUM_INPUT_ITFS=%d", GetObjectName(), idx, NUM_INPUT_ITFS);
	return(-1);
} 

/** Sends nsamples through output interface idx
 * 
 */
void set_output_samples(int idx, int nsamples) {
	assert(idx>=0 && idx<NUM_OUTPUT_ITFS);
	outputLen[idx] = nsamples*output_itfs[idx].sample_sz; 
}

/** User function to set an special type for an input interface */
/*void set_special_type_IN(int itfIndex, int type) {
	dataTypesIn[itfIndex] = type;
}*/

/** User function to set an special type for an output interface */
/*void set_special_type_OUT(int itfIndex, int type) {
	dataTypesOut[itfIndex] = type;
}*/

/** RunCustom() function process the received data by calling the user processing function.
 *  return 1 if ok, 0 if error.
 */
int RunCustom() {
	int n, i;

	TimeSlot++;

	/** Print received data*/
	if (printopts.mode & DATARECEIVED) {
		for (i = 0; i < NUM_INPUT_ITFS; i++) {
			print_interface(DATARECEIVED, 
											&printopts, 
											input_itfs[i].buffer, 
											dataTypesIn[i],
					 						inputLen[i]/input_itfs[i].sample_sz,
											input_itf_str[i],
											TimeSlot);
//		if(i==NUM_INPUT_ITFS-1)printf("\n");
		}
	}
	// WORK
	n = work();
	if (n <= 0){
		printf("Error!!!: Leaving %s's work function in controlled way\n",GetObjectName());
		return 0;
	}
	/** set output length to a default value */
	for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
		if (!outputLen[i] < 0){
			outputLen[i] = 0;
		}
	}

	/** Print send out data*/
	if (printopts.mode & DATASENT) {
		for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
			print_interface(DATASENT, 
											&printopts, 
											output_itfs[i].buffer, 
											dataTypesOut[i],
											outputLen[i]/output_itfs[i].sample_sz, 
											output_itf_str[i],
											TimeSlot);
//			if(i==NUM_OUTPUT_ITFS-1)printf("\n");
		}
	}

	/* send output data */
	for (i = 0; i < NUM_OUTPUT_ITFS; i++) {
		SendItf(i, outputLen[i]);
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

	TimeSlot=-1;
	print_config(&printopts, print_buffer);
	err = initialize();

	return err;
}



int StopCustom() {
	//stop();
	free_buffers_memory();
	return 1;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/**
 * @print_itfs_setup(). Print data from interfaces each time slot
 * @param.
 * @mode: Options: NOPRINT, DATARECEIVED, DATASENT
 * @ncolums: Number of printed data columns format
 * @TS2print: Initial Printing Time-slot
 * @nprintTS: Number of printing Time-slots
 * @offset: pointer offset to print
 * @maxlen: maximum number of samples to print
 * @return: 1
 */
void print_itfs_setup(int mode, int ncolums, int TS2print, int nprintTS, int offset, int maxlen){
		
	/*printf("A%s printopts.mode=%d, printopts.ncolums=%d\n", GetObjectName(), printopts.mode, printopts.ncolums);*/

	printopts.mode=mode;
	printopts.ncolums=ncolums;
	printopts.TS2print=TS2print;
	printopts.offset=offset;
	printopts.maxlen=maxlen;

	/*printf("B%s printopts.mode=%d, printopts.ncolums=%d\n", GetObjectName(), printopts.mode, printopts.ncolums);*/

}


/** print_config: Prints the initial configuration for such module
 */
void print_config(struct printopt *opts, char * print_buffer)
{
//#ifdef kk
	sprintf(print_buffer, "%sO------------------------------------------------------------------------------O\n", print_buffer);
	sprintf(print_buffer, "%sO    MODULE: %s.\n", print_buffer, GetObjectName());
	sprintf(print_buffer, "%sO    GENERIC PARAMETERS SETUP: %s.InitCustom.print_config().\n", print_buffer, GetObjectName());
	if(opts->mode | NOPRINT){
		sprintf(print_buffer, "%sO    PRINT CONFIG: ---- Printing Length=%d \n", print_buffer, opts->maxlen);
		sprintf(print_buffer, "%sO      Print Mode: ", print_buffer);
		if(opts->mode & DATARECEIVED)	sprintf(print_buffer, "%s--DATARECEIVED", print_buffer);
		if(opts->mode & DATASENT)		sprintf(print_buffer, "%s--DATASENT", print_buffer);
		sprintf(print_buffer, "%s\n", print_buffer);
		sprintf(print_buffer, "%sO      Starting Time Slot=%d, Printed Time Slots=%d \n", print_buffer, opts->TS2print, opts->nprintTS);
		sprintf(print_buffer, "%sO      Colums Number=%d, Pointer Offset=%d \n", print_buffer, opts->ncolums, opts->offset);
	}
//#endif
}

/** print_interface: Prints an array according their type at selected Time-Slot
 *  and between Time-Slot printingTS and Time-Slot printingTS+nprintTS.
 *  print parameter allow to desactivate printing.
 *
 */

void print_interface(int printMode, struct printopt *opts, void *buffer, int type, int length, char *Itfname, int time_slot) {

	int len;

	if(length == 0)return;

	if ((opts->TS2print <= time_slot) && ((opts->TS2print + opts->nprintTS)	> time_slot)) {
		if(printMode & DATARECEIVED)printf("\n@#### %s -- DATARECEIVED -- %s -- length %d -- TimeSlot=%d", GetObjectName(), Itfname, length, time_slot);
		if(printMode & DATASENT)printf("\n@#### %s -- DATASENT -- %s -- length %d -- TimeSlot=%d ", GetObjectName(), Itfname, length, time_slot);

		len = length > opts->maxlen ? opts->maxlen : length;
		print_array(buffer, opts->offset, type, len, opts->ncolums);
	}
}

void print_array(void *x, int offset, int Dtype, int Dlength, int Dcolums) {
	int i, k;
	float *input_f;
	_Complex float *input_cf;
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
	if (Dtype == TYPE_COMPLEX)
		input_cf = (_Complex float *) x;

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
			//printf("%03d ", (unsigned char) input_c[k + offset]);
			printf("%x ", ((int)input_c[k + offset]) & 0xFF);
		if (Dtype == TYPE_SHORT)
			printf("%d ", (short) input_s[k + offset]);
		if (Dtype == TYPE_INT)
			printf("%d ", (int) input_i[k + offset]);
		if (Dtype == TYPE_FLOAT)
			printf("%4.2f ", (float) input_f[k + offset]);
		if (Dtype == TYPE_COMPLEX)
			printf("%4.3f+%4.3f·I ", (float)(__real__ input_cf[k + offset]), (float)(__imag__ input_cf[k + offset]));
	}
	printf("\n");
}


//////////////////////////////////////////////////////////////////////
#define BACKGROUNDLEN	100
char background[BACKGROUNDLEN]=""; 
char backgroundREF[BACKGROUNDLEN]="                                                                                                 ";
char lineREF[BACKGROUNDLEN]="=================================================================================================";
char aux[BACKGROUNDLEN];

                                                                                
//http://www.bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux
char format[BACKGROUNDLEN]="\33[1m\33[38;5;021;48;5;226m";
char reset[10]="\033[0m";


void print_params(char *fore, char *ground){
	int init, end;
	int j=0;
	char *pchar;
	int *pint;
	float *pfloat;
	char *pstring;
	char mname[128];
	int length=0;
	int ret0, ret1, ret2;


//	printf("NEW print_params()\n");
	sprintf(mname, "%s", GetObjectName());

//void printCOLORtext(int position, char *fore, char *ground,  int end, char *data2print)

	init=4,
	end=init+strlen(mname)+4;
	printCOLORtext(init, "196", "015", end, mname);
	init=0;
	end=90-end;
	printCOLORtext(init, fore, ground, end, "=============================================================================================");
	printf("\n");

	sprintf(aux, "Interfaces");
	printCOLORtext(6, "002", ground, 90, aux);
	printf("\n");
	// PRINT INPUTS
	cleanstring(aux);
	j=0;
	while(IN_ITF_INFO[j].sample_sz){
		if(IN_ITF_INFO[j].sample_type==TYPE_CHAR){
			length += sprintf(aux+length, "InITF%d[CHARs],  ",  j);
		}
		if(IN_ITF_INFO[j].sample_type==TYPE_SHORT){
			length += sprintf(aux+length, "InITF%d[SHORTs],  ",  j);
		}
		if(IN_ITF_INFO[j].sample_type==TYPE_INT){
			length += sprintf(aux+length, "InITF%d[INTs],  ",  j);
		}
		if(IN_ITF_INFO[j].sample_type==TYPE_FLOAT){
			length += sprintf(aux+length, "InITF%d[FLOATs],  ",  j);
		}	
		if(IN_ITF_INFO[j].sample_type==TYPE_COMPLEX){
			length += sprintf(aux+length, "InITF%d[COMPLEXs],  ",  j);
		}
		j++;
		if(length > BACKGROUNDLEN-10){
			printCOLORtext(7, fore, ground, 90, aux);
			printf("\n");
			cleanstring(aux);
			length=0;
		}
	}
	printCOLORtext(7, fore, ground, 90, aux);
	printf("\n");

	// PRINT OUTPUTS
	length=0;
	cleanstring(aux);
	j=0;
	while(OUT_ITF_INFO[j].sample_sz){
		if(OUT_ITF_INFO[j].sample_type==TYPE_CHAR){
			length += sprintf(aux+length, "OutITF%d[CHARs],  ",  j);
		}
		if(OUT_ITF_INFO[j].sample_type==TYPE_SHORT){
			length += sprintf(aux+length, "OutITF%d[SHORTs],  ",  j);
		}
		if(OUT_ITF_INFO[j].sample_type==TYPE_INT){
			length += sprintf(aux+length, "OutITF%d[INTs],  ",  j);
		}
		if(OUT_ITF_INFO[j].sample_type==TYPE_FLOAT){
			length += sprintf(aux+length, "OutITF%d[FLOATs],  ",  j);
		}	
		if(OUT_ITF_INFO[j].sample_type==TYPE_COMPLEX){
			length += sprintf(aux+length, "OutITF%d[COMPLEXs],  ",  j);
		}
		j++;
		if(length > BACKGROUNDLEN-10){
			printCOLORtext(7, fore, ground, 90, aux);
			printf("\n");
			cleanstring(aux);
			length=0;
		}
	}
	printCOLORtext(7, fore, ground, 90, aux);
	printf("\n");

	cleanstring(aux);
	sprintf(aux, "Params"); 
	printCOLORtext(6, "027", ground, 90, aux);
	printf("\n");

	// PRINT VARS
	length=0;
	cleanstring(aux);
	j=0;
	while(my_vars[j].name){


/*		if(my_vars[j].type==STAT_TYPE_CHAR){
			ret=strncmp(my_vars[j].name, "LINEBREAK", 9);
			if(ret==0)printf("EQUAL: my_vars[j].name=%s\n", my_vars[j].name);
			if(ret!=0)printf("NO EQUAL: my_vars[j].name=%s\n", my_vars[j].name);
		}
*/

		if(my_vars[j].type==STAT_TYPE_CHAR){
			ret0=strncmp(my_vars[j].name, "LINEBREAK", 9);
			ret1=strncmp(my_vars[j].name, "TEXTN", 5);
			ret2=strncmp(my_vars[j].name, "TEXTC", 5);
			if(ret0==0){
				printCOLORtext(7, fore, ground, 90, aux);
				printf("\n");
				cleanstring(aux);
				length=0;
			}
			if(ret1==0){
				length += sprintf(aux+length, "%s ", (char *)my_vars[j].value);
				printCOLORtext(7, fore, ground, 90, aux);
				printf("\n");
				cleanstring(aux);
				length=0;
			}
			if(ret2==0){
				length += sprintf(aux+length, "%s ", (char *)my_vars[j].value);
				printCOLORtext(7, "124", ground, 90, aux);
				printf("\n");
				cleanstring(aux);
				length=0;
			}
			if(ret0!=0 && ret1!=0 && ret2!=0){
				pchar=(char *)my_vars[j].value;
				length += sprintf(aux+length, "%s=%c, ", my_vars[j].name, pchar[0]);
			}
		}
		if(my_vars[j].type==STAT_TYPE_INT){
			pint=(int *)my_vars[j].value;
			length += sprintf(aux+length, "%s=%d, ", my_vars[j].name, pint[0]);
		}
		if(my_vars[j].type==STAT_TYPE_FLOAT){
			pfloat=(float *)my_vars[j].value;
			length += sprintf(aux+length, "%s=%3.1f, ", my_vars[j].name, pfloat[0]);
		}
		if(my_vars[j].type==STAT_TYPE_STRING){
			pstring = (char *)my_vars[j].value;
			length += sprintf(aux+length, "%s=%s, ", my_vars[j].name, pstring);
			
//			printf("print_params(): TO BE DONE\n");
		}
		if(my_vars[j].type==STAT_TYPE_COMPLEX){
			printf("print_params(): TO BE DONE\n");
		}
		if(length > BACKGROUNDLEN-30){
			printCOLORtext(7, fore, ground, 90, aux);
			printf("\n");
			cleanstring(aux);
			length=0;
		}
		j++;
	}
	printCOLORtext(7, fore, ground, 90, aux);
	printf("\n");

	cleanstring(aux);
	printCOLORtext(0, fore, ground, 90, "=============================================================================================");
	printf("\n");
}

void printCOLORtext(int position, char *fore, char *ground,  int end, char *data2print){
	
	int length=(int)BACKGROUNDLEN;
	char formatt[BACKGROUNDLEN]="\33[1m\33[38;5;021;48;5;226m";

	if(strlen(data2print)>length){
		printf("EQUALIZER0:printCOLORtext() ERROR!!! data2print length = %d > BACKGROUNDLEN=%d\n", strlen(data2print), length);
		exit(0);
	}

	memcpy(formatt+11, fore, strlen(fore));
	memcpy(formatt+20, ground, strlen(ground));
	memcpy(background, backgroundREF, strlen(backgroundREF));	
	memcpy(background+position, data2print, strlen(data2print));
//	background[position+strlen(data2print)+1]=NULL;
	background[end]=NULL;
	printf("%s%s%s", formatt, background, reset);
}


void cleanstring(char *string){
	*string=NULL;
}

void printCOLORtextline(int position, char *fore, char *ground,  int end, char *data2print){

	printCOLORtext(position, fore, ground,  end, data2print);
	printf("\n");
}



