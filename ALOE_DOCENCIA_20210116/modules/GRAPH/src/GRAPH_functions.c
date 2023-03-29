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
#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
#include "string.h"

#include "gnuplot_i.h"
#include "GRAPH_functions.h"
char f[32]="logs/.log";


/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int init_timePlot(gnuplot_ctrl *plot){
	int i;

	plot = gnuplot_init() ;
	gnuplot_setstyle(plot,"lines");
	gnuplot_set_background(plot, "#00FFFF");
	return(1);
}



/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int timePlot(_Complex float *input, int inlength, _Complex float *output)
{
	int i,outlength;

//	printf("RUN MY FUNCTION\n");	
	for (i=0;i<inlength;i++) {
		__real__ output[i] = __real__ input[i];
		__imag__ output[i] = __real__ input[i];
	}
	outlength=inlength;
	return outlength;
}


/**
 * @functionA_COMPLEX(): Copy length samples from input to output
 * @param.
 * @input:
 * @inlengths Save on n-th position the number of samples generated for the n-th interface
 * @return the number of samples copied
 */
int bypass_COMPLEX(_Complex float *input, int inlength, _Complex float *output){
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

int bypass_FLOAT(float *input, int inlength,float *output){
	int i, outlength;

	for (i=0;i<inlength;i++) {
		output[i] = input[i];
	}
	outlength=inlength;
	return outlength;
}


int bypass_INT(int *in, int length, int *out){
	int i;

	for(i=0; i<length; i++){
		*(out+i)=*(in+i);
	}
	return 1;
}


int bypass_CHAR(char *in, int length, char *out){
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
		return(-1);
	}
	if(strcmp(datatypeIN,"COMPLEXFLOAT")==0){
		bypass_COMPLEX((_Complex float *)datin, datalength, (_Complex float *)datout);
	}
	if(strcmp(datatypeIN,"FLOAT")==0){
		bypass_FLOAT((float *)datin, datalength, (float *)datout);
	}
	if(strcmp(datatypeIN,"INT")==0){
		bypass_INT((int *)datin, datalength, (int *)datout);
	}
	if(strcmp(datatypeIN,"CHAR")==0){
		bypass_CHAR((char *)datin, datalength, (char *)datout);
	}
	return(1);
}



FILE *datafile_open(char * name, char *mode) {
	return fopen(name, mode);
}



int readIDfilebin(int *databuff){
	long int readlength=0;
	FILE *fp;

	//printf("%s\n", filename);
	fp=fopen(f, "rb");
	readlength=fread(databuff,sizeof(int),16,fp);
	fclose(fp);
	return(readlength);
}

void writeID(FILE *f, int* buf, int size) {
	int i;
	char aux[16];
	int a[8]={1,5,7,3,2,0,4,6};


	for (i=0;i<size;i++) {
		sprintf(aux, "%d", buf[i]);
		if(buf[i] > 0)fprintf(f,"0.%c%c%c%c%c%c%c%c+0i\n",
					aux[a[0]], aux[a[1]],aux[a[2]],aux[a[3]],
					aux[a[4]],aux[a[5]],aux[a[6]],aux[a[7]]);
		else fprintf(f,"0+0i\n");
	}
}



/**
 * Writes up to size complex values from the buffer pointed by buf to the
 * file referred to by the file descriptor fd.
 *
 * @param fd File descriptor returned by rtdal_datafile_open()
 * @param buf Pointer to the memory to write to the file
 * @param size Number of bytes to write
 * @return On success, the number of bytes written is returned (zero indicates
 * nothing was written). On error, -1 is returned
 */
void datafile_write_complex(FILE *f, _Complex float* buf, int size, int fileFormat) {
	int i;

	if(fileFormat==0){
		for (i=0;i<size;i++) {
			if (__imag__ buf[i] >= 0)
				fprintf(f,"%g,+%gi\n",__real__ buf[i],__imag__ buf[i]);
			else
				fprintf(f,"%g,-%gi\n",__real__ buf[i],fabsf(__imag__ buf[i]));
		}
	}

	if(fileFormat==1){
		for (i=0;i<size;i++) {
			if (__imag__ buf[i] >= 0)
				fprintf(f,"%1.6g+%1.6gi\n",__real__ buf[i],__imag__ buf[i]);
			else
				fprintf(f,"%1.6g-%1.6gi\n",__real__ buf[i],fabsf(__imag__ buf[i]));
		}
	}

}

/**
 * Writes up to size complex values from the buffer pointed by buf to the
 * file referred to by the file descriptor fd.
 * @param fd File descriptor returned by rtdal_datafile_open()
 * @param buf Pointer to the memory to write to the file
 * @param size Number of bytes to write
 * @return On success, the number of bytes written is returned (zero indicates
 * nothing was written). On error, -1 is returned
 */
int datafile_read_complex(FILE *hFile, _Complex float* buf, int length, int fileFormat) {
	int i, j=0, num;
	double real=0.0, imag=0.0;
	char sign, sep; 

//	length=1;

	if(fileFormat==0){
		for (i=0;i<length;i++) {
			if(num = fscanf( hFile, "%lf%c%lfi\n", &real, &sep, &imag) > 0 ){
				//if( sign == '-' )imag *= -1;
				*(buf+i) = (float)real + ((float)imag)*I;
//				printf("real=%1.9f, imag=%1.9f\n", __real__ buf[i], __imag__ buf[i]);
				j++;
			}else{
				fclose(hFile);
				printf("GRAPH file read finished\n");
				return(-1);
			}
		}
	}
	if(fileFormat==1){
		for (i=0;i<length;i++) {
			if(num = fscanf( hFile, "%lf%lfi\n", &real, &imag) > 0 ){
				*(buf+i) = (float)real + ((float)imag)*I;
//				printf("real=%1.9f, imag=%1.9f\n", __real__ buf[i], __imag__ buf[i]);
				j++;
			}else{
				fclose(hFile);
				printf("GRAPH file read finished\n");
				return(-1);
			}
		}
	}
	return(j);
}



// Function to delete n characters
void delchar(char *x,int a, int b)
{
  if ((a+b-1) <= strlen(x))
  {
    strcpy(&x[b-1],&x[a+b-1]);
    puts(x);
    }
}




