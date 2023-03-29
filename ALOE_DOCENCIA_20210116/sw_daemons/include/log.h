/*
 * log.h
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
#ifndef LOG_H_
#define LOG_H_

#define LOG_EXT		"log"

#define LOG_LINE_SZ	512

#define T log_o

struct T {
    int id;
    int fd;
    str_o name;
};

#define log_sizeof sizeof(struct log_o)

typedef struct T *T;

T log_new(int id, char *name);
void log_delete(T * log);
T log_dup(T log);
int log_Write(T log, char *output);
int log_findid(const void *x, const void *member);
int log_Write_var(T log, vartype_t type, varsz_t size, void *data, int *ts_vec, int report_len);
void log_xdelete(void ** x);

#undef T

#endif /*LOG_H_*/
