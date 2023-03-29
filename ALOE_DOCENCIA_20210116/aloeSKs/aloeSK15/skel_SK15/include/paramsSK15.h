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

#ifndef _PARAMS_H
#define _PARAMS_H

/** PARAMETERS MANAGER */
#define MAX_PARAMS 	50
#define STR_LEN		128

/** Parameter type */
typedef enum {CHAR=1, UCHAR=2, FLOAT=3, INT=4, SHORT=5, TEXT=6, STRING=6, COMPLEX=7, DOUBLE=8} param_type_t;

#define STAT_TYPE_CHAR		1
#define STAT_TYPE_UCHAR		2
#define STAT_TYPE_FLOAT		3
#define STAT_TYPE_INT		4
#define STAT_TYPE_SHORT		5
#define STAT_TYPE_TEXT		6
#define STAT_TYPE_STRING	6
#define STAT_TYPE_COMPLEX 	7
#define STAT_TYPE_DOUBLE	8


/** Parameter object */
typedef struct {
	char *name;
	param_type_t type;
	int size;
}param_t;

int 			param_init(param_t *user_params, int buff_sz);
int				param_close();
char*			param_error_str();

int 			param_packet_size();
void*			param_packet_addr();
int 			param_packet_read(int rcv_len);
int 			param_packet_write();

int			param_nof();
int			param_get_idx(char *name);
param_t*		param_get(char *name);
param_t*		param_get_i(int idx);
void*			param_get_addr(char *name);
void*			param_get_addr_i(int idx);
int 			param_get_aloe_type(char *name);
int 			param_get_aloe_type_i(int idx);
int 			param_get_int(char *name, int *value);
int 			param_get_float(char *name, float *value);
int 			param_get_string(char *name, char *value);


#endif
