/*
 * params.h
 *
 * Copyright (c) 2009 Xavier Reves, UPC <xavier.reves at tsc.upc.edu>. All rights reserved.
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
#define PARAM_CHAR	 10
#define PARAM_UCHAR  11
#define PARAM_SHORT  20
#define PARAM_USHORT 21
#define PARAM_INT	 30
#define PARAM_UINT   31
#define PARAM_HEX	 32
#define PARAM_FLOAT  4
#define PARAM_DOUBLE 5
#define PARAM_STRING 6
#define PARAM_BOOL   7

int string_to_argv (char *string, char **argv, int max_args);
int get_param(int argc, char *argv[], char *marker, void *value, int flag);
int string_to_param(char *svalue, void *pvalue, int flag);


