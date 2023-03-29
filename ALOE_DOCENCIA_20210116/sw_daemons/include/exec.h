/*
 * execinfo.h
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
#ifndef EXECINFO_INCLUDED
#define EXECINFO_INCLUDED

#define T execinfo_o

/** @defgroup execinfo Executable Information
 *
 * @ingroup common_obj
 *
 * 
 * Defines information of executable versions. Platform, memory if applicable,
 * available versions (compiled, source, etc.)
 */
typedef struct T *T;

struct T {
    str_o name;
    Set_o versions;
};

T execinfo_new();
void execinfo_delete(T * exec);
T execinfo_dup(T exec);
void execinfo_xdelete(void **x);
void * execinfo_xdup(const void *x);
int execinfo_xsizeof(const void *x);
int execinfo_findname(const void *x, const void *member);

#undef T
#define T execimp_o


enum exec_mode {BINARY,SOURCE};

typedef struct T *T;

struct T {
    struct pinfo pinfo;
    int platform;
    enum exec_mode mode;
    int fd;
    unsigned int checksum;
    str_o name;
};

T execimp_new();
void execimp_delete(T * imp);
T execimp_dup(T imp);
void execimp_xdelete(void **x);
void * execimp_xdup(const void *x);
int execimp_xsizeof(const void *x);
int execimp_popbindata(void *x, char **start, char *end);
int execimp_pinfotopkt(void *x, char **start, char *end);
int execimp_findplatformbinary(const void *x, const void *member);
int execimp_findplatformsource(const void *x, const void *member);
int execimp_open(T obj);
void execimp_close(T obj);

/** @} */
#undef T  
#endif /*  */
