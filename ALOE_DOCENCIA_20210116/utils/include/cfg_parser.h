/*
 * cfg_parser.h
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>. All rights reserved.
 *
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
#ifndef PARSER_INCLUDED
#define PARSER_INCLUDED
#define T cfg_o
#define S sect_o
#define K key_o

#define cfg_sizeof (sizeof(struct cfg_o))
#define sect_sizeof (sizeof(struct sect_o))
#define key_sizeof (sizeof(struct key_o))

struct T
{
	char *buffer;
	int offset;
	Set_o sects;
};

struct S
{
	str_o title;
	Set_o keys;
	Set_o sub_sects;
};

struct K
{
	str_o pname;
	str_o pvalue;
};



#define PARAM_CHAR   10
#define PARAM_INT    30
#define PARAM_HEX    32
#define PARAM_FLOAT  4
#define PARAM_STRING 6

typedef struct T *T;
typedef struct K *K;
typedef struct S *S;


/** @defgroup cfg Config Parser library
 * @ingroup common_base
 * @{
 */

T       cfg_new (char *buffer, int offset);
void    cfg_delete (T * cfg);
S       cfg_default_sect (T cfg);
Set_o   cfg_sections(T cfg);

void    sect_delete(S * sect);
Set_o   sect_subsects(S sect);
Set_o   sect_keys (S sect);
char *  sect_title (S sect);

void    key_delete (K * key);
int     key_nofelems (K key, int type);
int     key_type (K key);
int  	key_value (K key, int size, int type, void *value);

int     sect_findtitle (const void *data, const void *x);
int     key_findname (const void *data, const void *x);

void cfg_xdelete (void **x);
void sect_xdelete (void **x);
void key_xdelete (void **x);

int get_ptype(char *str);

/** @} */

#undef T
#undef S
#undef K
#endif


