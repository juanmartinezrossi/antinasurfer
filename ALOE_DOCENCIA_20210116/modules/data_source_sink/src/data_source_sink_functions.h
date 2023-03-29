/* 
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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H
long int read_file(char *filename, char *databuff, long int size);
long int read_file_bin(char *filename, char *databuff, long int size);
long int write_file(char *filename, char *databuff, long int size);
long int write_file_bin(char *filename, char *databuff, long int size);
long int addIDbin(int *databuff);
int compare_data(char *file, long int filelength, char *data, int datalength, float *BER);
void writemy(FILE *f, int* buf, int size);
FILE *datafile_open(char * name, char *mode);
void datafile_write_float(FILE *f,  float* buf, int size);

void initFrame2send(char *databuff, int MaxLength);
int sendDATA(char *databuff, char *data_out, int datalength);
int compare_data1(char *DATA, char *data_in, int datalength, float *BER);

#endif
