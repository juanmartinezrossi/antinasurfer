/*
 * cfg_parser.c
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "phal_hw_api.h"

#include "str.h"
#include "set.h"
#include "cfg_parser.h"
#include "params.h"

#define T cfg_o
#define S sect_o
#define K key_o

#define LINE_LEN        168
char string[LINE_LEN];

const char title_default[] = "default";

int is_quote(char c)
{
    if (c == '\'' || c == '\"') {
        return 1;
    } else {
        return 0;
    }
}

/** Get parameter size given a string and a type.
 * Size will be number of letters if type is a string
 * or number of "commas" + 1 if comma-sepparated values
 */
int get_psize(char *str, int type)
{
    int size;
    char *s = str;

    switch (type) {
    case PARAM_STRING:
        return (strlen(str) + 1);
    default:
        /* calc size of parameter (sepparated by commas) */
        for (size = 0; s; size++) {
            s++;
            s = strchr(s, ',');
        }
    }

    return size;

}

/* Get type of a paramter given a string. 
 * 
 * This function tries to guess the type of a string parameter
 * by analysing its contents. Types are:
 * Characters:
 *      - string if a character follows another or double/single quotes begin 
 *      - character otherwise 
 * Numbers:
 *      - Decimal Integer if a number if found
 *      - Hexadecimal integer if string beggins with 0x
 *      - Float if a point if found between numbers
 * 
 * @todo char/short types
 * @todo Check all string
 * 
 * @returns Type (>0) if guessed, -1 otherwise
 */
int get_ptype(char *str)
{
    if (isalpha(*str) || is_quote(*str)) {
        str++;
        if (isalpha(*str)) {
            return PARAM_STRING;
        } else {
            return PARAM_CHAR;
        }
    } else if (isdigit(*str)) {
        if (str[0] == '0' && str[1] == 'x') {
            return PARAM_HEX;
        }
        for (; *str != '\0'; str++) {
            if (*str == '.') {
                return PARAM_FLOAT;
            }
        }
        return PARAM_INT;
    }

    /* not guessed */
    return -1;
}

int sizeof_type(int flag)
{
    switch (flag) {
    case PARAM_CHAR:
    case PARAM_STRING:
        return sizeof (char);
    case PARAM_INT:
    case PARAM_HEX:
        return sizeof (int);
    case PARAM_FLOAT:
        return sizeof (double);
    default:
        return 0;

    }
}

/** Read single value and saves in position i
 * 
 * @param svalue Value in string format
 * @param pvalue Buffer where parsed data will be saved in position pos
 * @param pos Position in buffer where save the value
 */
char * parse_val(char *svalue, void *pvalue, int pos, int type)
{
    int i = 0;
    char *pc, *ret;
    int *pint;
    float *pfloat;
    int hex_value;

    assert(svalue);
    assert(pvalue);

    ret = NULL;
    pc = pvalue;

    switch (type) {
    case PARAM_BOOLEAN:
    	pint = (int *) pvalue;
    	*pint = !strncmp(svalue,"yes",3);
    	break;
    case PARAM_CHAR:
        pc[pos] = *svalue;
        ret = svalue + 1;
        break;
    case PARAM_STRING:
        strcpy(pvalue, svalue);
        ret = &pc[strlen(pvalue) + 1];
        break;
    case PARAM_INT:
        pint = (int *) pvalue;
        pint[pos] = (int) strtoul(svalue, &ret, 10);
        break;
    case PARAM_HEX:
        hex_value = (unsigned int) strtoul(svalue, &ret, 16);
        pint = (int *) pvalue;
        *pint = hex_value;
        break;
    case PARAM_FLOAT:
        pfloat = (float *) pvalue;
        pfloat[pos] = (float) strtod(svalue, &ret);
        break;
    default:
        i = -1;
        break;
    }

    return ret;
}

/** Parse a string value.
 * 
 * A buffer is allocated to store the value.
 * NULL Is returned if error.
 */
void read_value(char *str, int size, int type, void *value)
{
    assert(str);
    char *s;
    int i, size_type;

    size_type = sizeof_type(type);
    if (type == PARAM_STRING) {
        parse_val(str, value, 0, type);
    } else {
        s = str;
        for (i = 0; i < size && s; i++) {
            /* parse a single value */
            s = parse_val(s, value, i, type);
            if (s) {
                s++;
            }
        }
        if (i < size) {
            printf("Caution some data was not read\n");
        }
    }
}

/*****************
 *   copy LINE   *
 *****************/
int copy_line(char *origin, int offset, char *dest, int max)
{
    int i, j;

    i = offset;
    j = 0;

    while (origin[i] != '\n' && j < max) {
        dest[j++] = origin[i++];
    }
    if (j == max) {
        dest[j - 1] = '\0';
    } else {
        dest[j++] = '\0';
    }

    return j;
}

S parse_section(char *buffer, int *i, int file_len)
{
    int n;
    int next_section = 0;
    char *c;
    char *pname, *pvalue;
    K key;
    S sect;
    S sub_sect;

    NEW(sect);
    if (!sect) {
        return NULL;
    }

    sect->sub_sects = Set_new(0, NULL);
    if (!sect->sub_sects) {
        sect_delete(&sect);
        return NULL;
    }
    n = copy_line(buffer, *i, string, LINE_LEN);
    if (n <= 0) {
        sect_delete(&sect);
        return NULL;
    }

    c = strchr(string, '{');
    if (!c) {
        sect->title = str_snew((char*) title_default);
    } else {
        *c = '\0';
        sect->title = str_snew(string);
        *i += n;
    }

    sect->keys = Set_new(0, NULL);
    if (!sect->keys) {
        return NULL;
    }

    /* parse key values */
    while (!next_section && *i < file_len) {
        n = copy_line(buffer, *i, string, LINE_LEN);
        if (strchr(string, '}')) {
            next_section = 1;
            *i += n;
        } else if (strchr(string, '{')) {
            if (!sect->sub_sects) {
                sect->sub_sects = Set_new(0, NULL);

            }
            sub_sect = parse_section(buffer, i, file_len);
            if (!sub_sect) {
                sect_delete(&sect);
                return NULL;
            }
            Set_put(sect->sub_sects, sub_sect);
        } else {
            *i += n;
            pname = string;
            pvalue = strchr(string, '=');
            pvalue[0] = '\0';
            pvalue++;

            NEW(key);
            if (!key) {
                sect_delete(&sect);
                return NULL;
            }

            key->pname = str_snew(pname);
            key->pvalue = str_snew(pvalue);
            Set_put(sect->keys, key);
        }
    }

    return sect;
}

/** Create new config handler.
 * 
 * Parses a buffer of characters into
 * sections and key-values pairs.
 */
T
cfg_new(char *buffer, int offset)
{
    int i;
    S sect;
    T cfg;

    NEW(cfg);
    if (!cfg) {
        return NULL;
    }

    cfg->buffer = buffer;
    cfg->offset = offset;

    i = 0;

    cfg->sects = Set_new(0, NULL);

    while (i < offset) {
        sect = parse_section(buffer, &i, offset);
        Set_put(cfg->sects, sect);
    }

    if (i < offset) {
        xprintf("unexpected end of file\n");
    }
    return cfg;
}

void cfg_delete(T * cfg)
{
    assert(cfg && *cfg);

    if ((*cfg)->sects) {
        Set_destroy(&(*cfg)->sects, sect_xdelete);
    }

    DELETE(*cfg);
    cfg = NULL;
}

void sect_delete(S * sect)
{
    assert(sect && *sect);


    str_delete(&(*sect)->title);

    if ((*sect)->sub_sects) {
        Set_destroy(&(*sect)->sub_sects, sect_xdelete);
    }

    if ((*sect)->keys) {
        Set_destroy(&(*sect)->keys, key_xdelete);
    }

    DELETE(*sect);
    sect = NULL;
}

void key_delete(K * key)
{
    assert(key && *key);

    str_delete(&(*key)->pname);
    str_delete(&(*key)->pvalue);

    DELETE(*key);
    key = NULL;
}

int key_nofelems(K key, int type)
{
    assert(key);
    return get_psize(str_str(key->pvalue), type);
}

int key_type(K key)
{
    assert(key);
    return get_ptype(str_str(key->pvalue));
}

int key_value(K key, int size, int type, void *value)
{
    assert(key && value);

    if (!type) {
        type = get_ptype(str_str(key->pvalue));
    }
    if (type == -1) {
        return 0;
    }
    if (!size) {
        size = get_psize(str_str(key->pvalue), type);
    }
    read_value(str_str(key->pvalue), size, type, value);
    return 1;
}

S cfg_default_sect(T cfg)
{
    assert(cfg);

    if (Set_length(cfg->sects) > 1) {
        return NULL;
    }
    return Set_get(cfg->sects, 0);
}

Set_o cfg_sections(T cfg)
{
    assert(cfg);
    return cfg->sects;
}

Set_o sect_subsects(S sect)
{
    assert(sect);

    return sect->sub_sects;
}

Set_o sect_keys(S sect)
{
    assert(sect);

    return sect->keys;
}

char * sect_title(S sect)
{
    assert(sect);

    return str_str(sect->title);
}

int key_findname(const void *data, const void *x)
{
    K key = (K) x;
    assert(x && data);

    return str_scmp(key->pname, (char *) data);
}

int sect_findtitle(const void *data, const void *x)
{
    S sect = (S) x;
    assert(x && data);

    return str_scmp(sect->title, (char *) data);
}

void cfg_xdelete(void **x)
{
    return cfg_delete((T*) x);
}

void sect_xdelete(void **x)
{
    return sect_delete((S*) x);
}

void key_xdelete(void **x)
{
    return key_delete((K*) x);
}

