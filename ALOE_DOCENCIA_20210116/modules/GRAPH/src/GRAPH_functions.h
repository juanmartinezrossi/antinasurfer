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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H
int init_timePlot(gnuplot_ctrl *plot);
int timePlot(_Complex float *input, int inlength, _Complex float *output);
int bypass_COMPLEX(_Complex float *input, int lengths, _Complex float *output);
//long int readIDbin(int *databuff);
/*
FILE *datafile_open(char * name, char *mode);
void datafile_write_complex(FILE *f, _Complex float* buf, int size);
long int addCPLXbin(char filename, _Complex float *databuff, int datalength);
int readIDfilebin(int *databuff);
void createIDsCPLX(int *IDs, _Complex float *buff, int numIDs);
void write_CPLX(FILE *f, _Complex float* buf, int size);
void writeID(FILE *f, int* buf, int size);
*/

int readIDfilebin(int *databuff);
void writeID(FILE *f, int* buf, int size);
FILE *datafile_open(char * name, char *mode);
void datafile_write_complex(FILE *f, _Complex float* buf, int size, int fileFormat);
int datafile_read_complex(FILE *hFile, _Complex float* buf, int length, int fileFormat);
void delchar(char *x,int a, int b);


int bindatafile_read_complex(FILE *f, _Complex float* *buff, int size);
int bypass_FLOAT(float *input, int lengths, float *output);
int bypass_INT(int *in, int length, int *out);
int bypass_CHAR(char *in, int length, char *out);
int bypass_func(char *datatypeIN, char *datatypeOUT, void *datin, int datalength, void *datout);

#endif
