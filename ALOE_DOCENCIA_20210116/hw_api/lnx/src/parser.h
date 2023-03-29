/*
 * parser.h
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>,
 *                    Xavier Reves, UPC <xavier.reves at tsc.upc.edu>
 * All rights reserved.
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

#define PARSE

/*File processing options*/
#define PARSE_LINE_LEN 163

/*Read command options*/
#define PARSE_CMD_LEN 1024

/*Options for functions*/
#define OPT_SILENT  0x01
#define OPT_CASE	0x02

/*Errors*/
#define EREAD -1
#define ELONG -2
#define EMEM  -3

int read_file(FILE *f1, char **buffer, int opt);
int read_command(int fd, char *s, int opt);
int read_line(char *origin, int offset, char *dest);
int get_command(char *c);
int param_name(char *c, char **list_of_params);
int param_name_start(char *c, char **list_of_params);
void erase_space_label(char *string);
void erase_space_all(char *string);
void erase_space_extra(char *string);
int quote(char *string);
void set_lower_case_label(char *string);
void set_lower_case_all(char *string);
void string_field(char *source, char *field, char sep, int count);
void copy_args(char *source, char *dest);


