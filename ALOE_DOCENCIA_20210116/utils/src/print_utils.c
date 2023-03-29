/*
 * print_utils.c
 *
 * This file is part of ALOE.
 *
 * ALOE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ALOE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_cplx(char *name, _Complex float *in, int datalength, int rowlength){

	int i, z=1;
	printf("%s\n", name);
	for(i=0; i<datalength; i++){
		printf("%6.1f+%6.1fi, ", __real__ *(in+i), __imag__ *(in+i));
		if(z==rowlength){
			printf("\n");
			z=0;
		}
		z++;
	}
	printf("\n");
}

void print_float(char *name, float *in, int datalength, int rowlength){

	int i, z=1;
	printf("%s\n", name);
	for(i=0; i<datalength; i++){
		printf("%6.1f, ", *(in+i));
		if(z==rowlength){
			printf("\n");
			z=0;
		}
		z++;
	}
	printf("\n");
}

void print_int(char *name, int *in, int datalength, int rowlength){

	int i, z=1;
	printf("%s\n", name);
	for(i=0; i<datalength; i++){
		printf("%6d, ", *(in+i));
		if(z==rowlength){
			printf("\n");
			z=0;
		}
		z++;
	}
	printf("\n");
}

void print_bit(char *name, char *in, int datalength, int rowlength){

	int i, z=1;
	printf("%s\n", name);
	for(i=0; i<datalength; i++){
		printf("%1d, ", ((int)(*(in+i)))&0xFF);
		if(z==rowlength){
			printf("\n");
			z=0;
		}
		z++;
	}
	printf("\n");
}

void print_char(char *name, char *in, int datalength, int rowlength){

	int i, z=1;
	printf("%s\n", name);
	for(i=0; i<datalength; i++){
		printf("%c, ", *(in+i));
		if(z==rowlength){
			printf("\n");
			z=0;
		}
		z++;
	}
	printf("\n");
}

int print_array(char *name, char *datatype, void *in, int datalength, int rowlength){


	if(strcmp(datatype,"COMPLEXFLOAT")==0){
		print_cplx(name, (_Complex float*) in, datalength, rowlength);
		return(0);
	}
	if(strcmp(datatype,"FLOAT")==0){
		print_float(name, (float*)in, datalength, rowlength);
		return(0);
	}
	if(strcmp(datatype,"INT")==0){
		print_int(name, (int*)in, datalength, rowlength);
		return(0);
	}
	if(strcmp(datatype,"BIT")==0){
		print_bit(name, (int*)in, datalength, rowlength);
		return(0);
	}
	if(strcmp(datatype,"CHAR")==0){
		print_char(name, (char*)in, datalength, rowlength);
		return(0);
	}
	printf("Valid datatype: COMPLEXFLOAT, FLOAT, INT, CHAR. Please, review your code!!!\n");
}



