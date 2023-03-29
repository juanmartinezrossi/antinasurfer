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

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "data_source_sink_functions.h"
char f[32]="logs/.log";

extern char mname[];

#define NUMMIN_ERRORS		0
#define NUMTS2PRINT_BER		10

// Mode 1
#define MAXTSLOTSEXEC	1000
#define MAXLENGTH		32
int sentDataLength[MAXLENGTH];		// Saves the length of sent packets

void initFrame2send(char *databuff, int MaxLength){
	int i, first=0, second=1, next=1;
	
	for(i=0; i<MaxLength; i++){
		//*(databuff+i)=(char)(rand()&0xFF);

		*(databuff+i)=(char)(next&0xFF);
		next = first+second;
		first=second;
		second=next;
	}
}

int sendDATA(char *databuff, char *data_out, int datalength){
	int i;
	static int w_idx=0;

	if(datalength == 0)return(0);
	
	for(i=0; i<datalength; i++){
		*(data_out+i)=*(databuff+i);
	}
	sentDataLength[w_idx]=datalength;
	w_idx++;
	if(w_idx==MAXLENGTH)w_idx=0;
	return(1);
}


#define TSLOT_s		0.021333333

int compare_data1(char *DATA, char *data_in, int datalength, float *BER){

    static long int count=0;
	static int numbitserror=0;
    unsigned char byte;
    int i, n;
    static int Tslot=-1;
	static int r_idx=0;
	float throughtput=0.0;

	Tslot++;
	*BER=10e-6;
	if(datalength<=0)return(1);
/*	if(datalength != sentDataLength[r_idx]){
		printf("data_source_sink.compare_data1(): datalength=%d != sentDataLength[%d]=%d\n", 
				datalength, r_idx, sentDataLength[r_idx]);
		return(-1);
	}*/
	r_idx++;
	if(r_idx==MAXLENGTH)r_idx=0;

	for (i=0; i<datalength; i++){
		byte=*(data_in+i)^*(DATA+i);
		if(byte!=0){
			for(n=0; n<8; n++){
				numbitserror=numbitserror+(byte&0x01);
				byte=byte>>1;
			}
		}
	}
	count += datalength*8;
	*BER=(float)(numbitserror)/(float)(count);
	throughtput = (float)count/(((float)Tslot)*TSLOT_s);

	if(Tslot==0)printf("\n");
	if(Tslot%NUMTS2PRINT_BER == 0){
		*BER=(float)(numbitserror)/(float)(count);
		printf("\033[1;31;47m \t==========================================================================\t\033[0m\n");
		printf("\033[1;31;47m \t!!!!TSLOT=%d: CURRENT BER = %1.6f, numbitserror=%d, throughtput=%6.1f         \t\033[0m\n",Tslot, *BER, numbitserror, throughtput);
		printf("\033[1;31;47m \t==========================================================================\t\033[0m\n");
		numbitserror=0;
		count=0;
	}
	if(Tslot==MAXTSLOTSEXEC){
		printf("\033[1;31;47m \tO   ################################################################     O\t\033[0m\n");
		printf("\033[1;31;47m \tO     APP EXECUTION FINISHED!!!: DATA BLOCKS SENT & RECEIVED              \t\033[0m\n");
		printf("\033[1;31;47m \tO     DATA RECEIVED WITH BER = %1.6f, numbitserror=%d                     \t\033[0m\n",*BER, numbitserror);
		printf("\033[1;31;47m \tO   ################################################################     O\t\033[0m\n");
		return(-1);
	}


    return(1);
}



/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
long int read_file(char *filename, char *databuff, long int size){
	long int readlength=0;
	FILE *fp;

//	printf("%s\n", filename);
	fp=fopen(filename, "r");
	readlength=fread(databuff,1,size,fp);
	if(size != readlength){
		//printf("Reading filename %s: read data=%ld, expected=%ld\n", filename, readlength, size);
	}
	fclose(fp);
	return(readlength);
}


long int read_file_bin(char *filename, char *databuff, long int size){
	long int readlength=0;
	FILE *fp;

//	printf("%s\n", filename);
	fp=fopen(filename, "rb");
	readlength=fread(databuff,1,size,fp);
	if(size != readlength){
		//printf("Reading filename %s: read data=%ld, expected=%ld\n", filename, readlength, size);
	}
	fclose(fp);
	return(readlength);
}


long int write_file(char *filename, char *databuff, long int size){
	long int writelength=0;
	FILE *fp;

	fp=fopen(filename, "w");
	writelength=fwrite(databuff,1,size,fp);
	if(size != writelength){
		printf("Writing filename %s: wrote data=%ld, expected=%ld\n", filename, writelength, size);
	}
	fclose(fp);
	return(writelength);
}

long int write_file_bin(char *filename, char *databuff, long int size){
	long int writelength=0;
	FILE *fp;

	fp=fopen(filename, "wb");
	writelength=fwrite(databuff,1,size,fp);
	if(size != writelength){
		printf("Writing filename %s: wrote data=%ld, expected=%ld\n", filename, writelength, size);
	}
	fclose(fp);
	return(writelength);
}


/**
 * @brief Compare the received bytes with the reference file.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int compare_data(char *file, long int filelength, char *data_in, int datalength, float *BER){

    static long int count=0;
	static int numbitserror=0;
    unsigned char byte;
    int i, n;
    static int Tslot=0;
	float throughtput=0.0;

	if(count<filelength){		//Check the file length
		for (i=0; i<datalength; i++){
			byte=*(data_in+i)^*(file+count);
			if(byte!=0){
				for(n=0; n<8; n++){
//					printf("byte=%x\n",byte&0x01);
					numbitserror=numbitserror+(byte&0x01);
					byte=byte>>1;
//					printf("numbitserror=%d\n",numbitserror);
				}
			}
//			printf("count=%d\n", count);
			count++;	
		}
		//printf("numbitserror=%d\n",numbitserror);
	}
	else {
		printf("Data bytes received = %ld longer than file length=%ld\n", count, filelength);
		if(numbitserror>=NUMMIN_ERRORS){
			//printf("AAnumbitserror=%d\n",numbitserror);
			*BER=(float)(numbitserror)/(float)(count*8);
			printf("\033[1;31m O       #####################################################        O\033[0m\n");
			printf("\033[1;31m O       APP EXECUTION FINISHED!!!: FILE SENT & RECEIVED \033[0m\n");
			printf("\033[1;31m O       FILE RECEIVED WITH BER = %1.6f, numbitserror=%d\033[0m\n",*BER, numbitserror);
			printf("\033[1;31m O       #####################################################        O \033[0m\n");
		}else{
			printf("FILE RECEIVED: numerrors=%d < NUMMIN_ERRORS=%d and BER can not be calculated\n",numbitserror, NUMMIN_ERRORS);
		}
		return(-1);
	}
	if(Tslot==0)printf("\n");
	if(Tslot%NUMTS2PRINT_BER == 0 && Tslot>0){
		*BER=(float)(numbitserror)/(float)(count*8);
		throughtput = (float)count/(((float)Tslot)*TSLOT_s);
/*		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("DATA_SOURCE_SINK !!! TSLOT=%d: CURRENT BER = %1.6f, numbitserror=%d\n",Tslot, *BER, numbitserror);
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
*/
		printf("\033[1;31;47m =====================================================================================================\t\033[0m\n");
		printf("\033[1;31;47m %s!!!! TSLOT=%d: CURRENT BER = %1.6f, numbitserror=%d, throughtput=%6.1f bps        \t\033[0m\n",
					mname, Tslot, *BER, numbitserror, throughtput);
		printf("\033[1;31;47m =====================================================================================================\t\033[0m\n");
	}

	Tslot++;
    return(1);
}

long int addIDbin(int *databuff){
	long int writelength=0;
	int readbuff[32];
	int readlength=0;
	int i, flag=0;
	FILE *fp;

	fp=fopen(f, "ab+");
	readlength=fread(readbuff,sizeof(int),32,fp);
	for(i=0; i<readlength; i++){
		if(readbuff[i] == (*databuff))flag=1;
	}
	if(flag==0){
		writelength=fwrite(databuff,1,sizeof(int),fp);
	} else{
		fseek(fp, sizeof(int), SEEK_END);
		fread(databuff, sizeof(int), 1, fp);
	}
	fclose(fp);
	return(writelength);
}

int readIDfilebin(int *databuff){
	long int readlength=0;
	FILE *fp;

	//printf("%s\n", filename);
	fp=fopen(f, "rb");
	if(fp == NULL)return(-1);
	readlength=fread(databuff,sizeof(int),16,fp);
	fclose(fp);
	return(readlength);
}


void writemy(FILE *f, int* buf, int size) {
	int i;
	char aux[16];
	int a[8]={7,2,0,1,5,4,3,6};

	for (i=0;i<size;i++) {
		sprintf(aux, "%d", buf[i]);
		if(buf[i] > 0)fprintf(f,"0.%c%c%c%c%c%c%c%c\n",
				aux[a[0]], aux[a[1]],aux[a[2]],aux[a[3]],
				aux[a[4]],aux[a[5]],aux[a[6]],aux[a[7]]);
		else fprintf(f,"0.000000\n");
	}
}




FILE *datafile_open(char * name, char *mode) {
	return fopen(name, mode);
}


void datafile_write_float(FILE *f,  float* buf, int size) {
	int i;
	fprintf(f,"%1.6f\n", buf[0]);

}
