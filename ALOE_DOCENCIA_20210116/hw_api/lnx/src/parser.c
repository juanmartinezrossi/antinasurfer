/*
 * parser.c
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"

#ifdef PARSE_LINE_LEN
#define LINE_LENGTH PARSE_LINE_LEN
#else
#define LINE_LENGTH 163
#endif

#ifndef MEM_CHUNK
#define MEM_CHUNK 4096
#endif

char string[LINE_LENGTH];

extern int errno;

/*****************
 *   READ FILE   *
 *****************/
int read_file(FILE *f1, char **pbuffer, int opt)
{
    int n, line;
    int available, offset;
    int check_llength;
    char *buffer;
    char *pchar;

    /*Get memory for pre-processing buffer*/
    buffer = (char *) malloc(MEM_CHUNK);
    if (buffer == NULL) {
        if (!(opt & OPT_SILENT)) {
            fprintf(stderr, "ERROR: Could not allocate memory for internal buffer\n");
            perror("malloc");
        }
        return (EMEM);
    }
    available = MEM_CHUNK;
    offset = 0;
    line = 0;
    /*Read an clean file*/
    while (!feof(f1)) {
        check_llength = 0;
        pchar = fgets(string, LINE_LENGTH - 1, f1);
        if (pchar != NULL) {
            /*Check for long lines*/
            n = strlen(string);
            if (string[n - 1] != '\n') {
                check_llength = 1;
                if (feof(f1)) {
                    check_llength = 0;
                    string[n] = '\n';
                    string[n + 1] = '\0';
                    n++;
                    if (!(opt & OPT_SILENT)) {
                        fprintf(stderr, "WARNING: File does not end with newline.\n");
                        fprintf(stderr, "WARNING: Insert a newline at the end of file to avoid this warning\n");
                    }
                }
            }
            /*Pre process string*/
            line++;
            erase_space_label(string);

            if (check_llength) {
                /*Is a comment?*/
                if (string[0] == '#')
                    do
                        pchar = fgets(string, LINE_LENGTH, f1);
                    while ((pchar != NULL) && (string[strlen(string) - 1] != '\n'));
                else {
                    if (!(opt & OPT_SILENT))
                        fprintf(stderr, "ERROR: line too long (max %d chars) in file: line %d\n", LINE_LENGTH - 3, line);
                    return (ELONG);
                }
            } else if ((n = strlen(string)) > 0) {
                /*Append a newline ('\n')*/
                string[n++] = '\n';
                string[n] = '\0';

                /*Is not a comment?*/
                if (string[0] != '#') {
                    set_lower_case_label(string);
                    /*Check available memory*/
                    if (available < n) {
                        pchar = (char *) realloc(buffer, offset + available + MEM_CHUNK);
                        if (pchar == NULL) {
                            free(buffer);
                            if (!(opt & OPT_SILENT)) {
                                printf("ERROR: Could not allocate memory for internal buffer\n");
                                perror("realloc");
                                fclose(f1);
                            }
                            return (EMEM);
                        }
                        buffer = pchar;
                        available += MEM_CHUNK;
                    }

                    /*Write the newline*/
                    memcpy(&(buffer[offset]), string, n);
                    offset += n;
                    available -= n;
                }
            } /*End if n>0*/
        } /*End if pchar!=NULL*/
    } /* End while*/

    *pbuffer = buffer;
    return (offset);

}

/**********************
 *   PARAMETER NAME   *
 **********************/
int param_name(char *c, char **list_of_params)
{
    int i, retv = 1;

    i = 0;
    if (c != NULL) {
        while ((*(list_of_params[i]) != '\0') && (retv > 0)) {
            retv = abs(strcmp(list_of_params[i], c));
            i++;
        }

        if (retv == 0)
            retv = i - 1;
        else
            retv = -1;
    } else retv = -1;
    return (retv);
}

int param_name_start(char *c, char **list_of_params)
{
    int i, retv = 1;
    int n;

    i = 0;
    if (c != NULL) {
        while ((*(list_of_params[i]) != '\0') && (retv > 0)) {
            n = strlen(list_of_params[i]);
            retv = abs(strncmp(list_of_params[i], c, n));
            i++;
        }

        if (retv == 0)
            retv = i - 1;
        else
            retv = -1;
    } else retv = -1;
    return (retv);
}

/**********************
 *   SET LOWER CASE   *
 **********************/
void set_lower_case_label(char *string)
{
    int i = 0;

    /*If '=' is found, do not modify string after it*/
    while ((string[i] != '\0') && (string[i] != '=')) {
        if (isupper(string[i]))
            string[i] = tolower(string[i]);
        i++;
    }
}

void set_lower_case_all(char *string)
{
    int i = 0;

    /*If '=' is found, do not modify string after it*/
    while (string[i] != '\0') {
        if (isupper(string[i]))
            string[i] = tolower(string[i]);
        i++;
    }
}

/*******************
 *   QUOTE CHECK   *
 *******************/
int quote(char *string)
{
    int i, j, w, stq;

    i = 0;
    j = 0;
    w = 0;
    stq = 0;
    while (string[i] != '\0') {
        if (string[i] == '"')
            stq = (stq == 0 ? 1 : 0);
        else {
            /*if (!isalnum(string[i]) && (string[i]!=' ') && stq)*/
            if (!isprint(string[i])) {
                printf("WARNING: Found a non printable character (0x%x) in between quotes\n", string[i]);
                w = 1;
                string[j++] = ' ';
            } else
                string[j++] = string[i];
        }
        i++;
    }

    if (stq) {
        printf("WARNING: Quotes not closed\n");
        w = 1;
    }
    /*Terminate the string*/
    string[j] = '\0';

    return (w);
}

/**************************
 *   ERASE WHITE SPACES   *
 **************************/
void erase_space_label(char *string)
{
    int i, j;
    int eq;

    i = 0;
    j = 0;
    eq = 0;

    /*Erase white spaces until the end or the '=' sign*/
    /* space, page feed ('\f'), line feed ('\n'),  carry return  ('\r'),
   horizontal tab ('\t'), and vertical tab ('\v')*/
    while ((string[i] != '\0') && (!eq)) {
        if (!isspace(string[i])) {
            string[j++] = string[i];
            if (string[i] == '=')
                eq = 1;
        }
        i++;
    }

    /*Remove spaces after '=' and before any text*/
    while (isspace(string[i]))
        i++;

    /*If '=' has been found, continue removing only "strange" chars up to the end*/
    while (string[i] != '\0') {
        if (!isspace(string[i]) || (string[i] == ' '))
            string[j++] = string[i];
        i++;
    }

    /*Delete traling spaces*/
    j--;
    while (isspace(string[j]))
        j--;

    /*Terminate string*/
    string[j + 1] = '\0';

}

/*********/
void erase_space_all(char *string)
{
    int i, j;

    i = 0;
    j = 0;

    /*Erase white spaces until the end */
    /* space, page feed ('\f'), line feed ('\n'),  carry return  ('\r'),
   horizontal tab ('\t'), and vertical tab ('\v')*/
    while (string[i] != '\0') {
        if (!isspace(string[i]))
            string[j++] = string[i];
        i++;
        printf("%d,%d\n", i, j);
    }

    /*Terminate the string*/
    string[j] = '\0';
}

void erase_space_extra(char *string)
{
    int i, j;

    i = 0;
    j = 0;

    /*Erase white spaces at the beginning of the string */
    /* space, page feed ('\f'), line feed ('\n'),  carry return  ('\r'),
   horizontal tab ('\t'), and vertical tab ('\v')*/
    while (string[i] != '\0') {
        if (isspace(string[i]))
            i++;
        else break;
    }

    /*Erase spaces in between*/
    while (string[i] != '\0') {
        if (isspace(string[i])) {
            string[j++] = ' ';
            i++;
            while (isspace(string[i]))
                i++;
        } else string[j++] = string[i++];
    }

    if (isspace(string[j - 1]))
        j--;

    /*Terminate the string*/
    string[j] = '\0';
}

/*****************
 *   READ LINE   *
 *****************/
int read_line(char *origin, int offset, char *dest)
{
    int i, j;

    i = offset;
    j = 0;

    while (origin[i] != '\n') {
        dest[j++] = origin[i++];
    }
    dest[j++] = '\0';

    return j;
}

/**************************************
 *   OBTAIN A FIELD WITHIN A STRING   *
 **************************************/
void string_field(char *source, char *field, char sep, int count)
{
    int j;
    int fcount = 0;
    char *p1, *p2;

    p2 = p1 = source;
    field[0] = '\0';

    if (count <= 0) return;

    while (*p1 != '\0') {
        if ((*p2 == sep) || (*p2 == '\0')) {
            fcount++;
            if (fcount == count) {
                for (j = 0; j < (p2 - p1); j++)
                    field[j] = p1[j];
                field[j] = '\0';
                break;
            } else {
                p1 = p2;
            }
        }
        p2++;
    }
}

/**********************************************
 *   COPY ALL THE STRING BUT THE FIRST WORD   *
 **********************************************/
void copy_args(char *source, char *dest)
{
    int n = 0;

    while ((source[n] != ' ') && (source[n] != '\0'))
        n++;

    if (source[n] == '\0') {
        dest[0] = '\0';
        return;
    }

    n++;
    strcpy(dest, &(source[n]));
}

int read_command(int fd, char *s, int opt)
{
    int i = 0;
    int retv = 0;
    int n;
    char c = 0;

    n = read(fd, &c, sizeof (char));
    if (n < 0) {
        if (!(opt & OPT_SILENT))
            fprintf(stderr, "Error reading command");
        retv = EREAD;
    } else while (c != '\n') {
            s[i % (PARSE_CMD_LEN - 1)] = c;
            if (read(fd, &c, sizeof (char)) < 0) {
                if (!(opt & OPT_SILENT))
                    fprintf(stderr, "Error reading command");
                retv = EREAD;
                break;
            }
            if (i >= PARSE_CMD_LEN) {
                i = PARSE_CMD_LEN - 1;
                if (!(opt & OPT_SILENT))
                    fprintf(stderr, "Command too long\n");
                retv = ELONG;
                break;
            }
            i++;
        }
    s[i] = '\0';

    if (!(opt & OPT_CASE))
        set_lower_case_all(s);

    return (retv);
}

