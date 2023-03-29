/*
 * strdata.h
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
#ifndef STRDATA_INCLUDED
#define STRDATA_INCLUDED

#define T strdata_o

#define strdata_sizeof (sizeof (struct strdata_o))

typedef struct T *T;

struct T
{
  Set_o strings;
  int cur_readed;
};


T strdata_new();
void strdata_delete(T *s);

int strdata_push(T s, void *data, int type);
int strdata_pop(T s, void *data, int type);

int strdata_write(T s, int fd);
int strdata_read(T s, int fd);

void strdata_clear(T s);

#undef T  
#endif /*  */
