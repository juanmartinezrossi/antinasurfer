/*
 * strdata.c
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
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "params.h"
#include "set.h"
#include "str.h"
#include "strdata.h"

#define T strdata_o

#define SEP "||"
#define SEP_SZ 2

T strdata_new()
{
    T s = calloc(1, sizeof (struct T));
    if (!s) {
        return NULL;
    }
    s->strings = Set_new(0, NULL);
    if (!s->strings) {
        return NULL;
    }
    return s;
}

void strdata_delete(T *s)
{
    assert(s);
    assert(*s);
    free(*s);
    s = NULL;
}

int strdata_push(T s, void *data, int type)
{
    assert(s && data);
    char *datastr;
    datastr = param_to_string(data, type);
    if (!datastr) {
        return 0;
    }
    Set_put(s->strings, datastr);
    return 1;
}

int strdata_pop(T s, void *data, int type)
{
    char *datastr;
    assert(s && data);
    datastr = Set_get(s->strings, s->cur_readed++);
    if (!datastr) {
        return 0;
    }
    return string_to_param(datastr, data, type);
}

int strdata_write(T s, int fd)
{
    int i, size, n;
    char *str;
    char *buffer;
    assert(s);
    size = 0;
    for (i = 0; i < Set_length(s->strings); i++) {
        str = Set_get(s->strings, i);
        assert(str);
        size += strlen(str) + SEP_SZ;
    }
    buffer = calloc(1, size);
    n = 0;
    for (i = 0; i < Set_length(s->strings); i++) {
        str = Set_get(s->strings, i);
        assert(str);
        memcpy(&buffer[n], str, strlen(str));
        n += strlen(str);
        free(str);
        memcpy(&buffer[n], SEP, SEP_SZ);
        n += SEP_SZ;
    }
    assert(n == size);
    n = htonl(size);
    if (write(fd, &n, sizeof (int)) < 0)
        return -1;
    if (write(fd, buffer, size) < 0)
        return -1;

    free(buffer);
    Set_clear(s->strings);
    return 1;
}

int strdata_read(T s, int fd)
{
    int i, j, size, n;
    char *str1, *str2, *str;
    char *buffer;
    assert(s);
    strdata_clear(s);

    if (read(fd, &n, sizeof (int)) < 0)
        return -1;
    size = ntohl(n);
    if (size < 0)
        return -1;
    if (!size)
        return 0;

    buffer = calloc(1, size + 1);
    if (!buffer) {
        return -1;
    }
    n = 0;
    do {
        i = read(fd, &buffer[n], size);
        if (i < 0)
            return -1;
        n += i;
    } while (n < size);
    buffer[n] = '\0';
    str1 = buffer;
    n = 0;
    j = 0;
    while (n < size) {
        str2 = strstr(str1, SEP);
        if (!str2)
            break;
        *str2 = '\0';
        str = strdup(str1);
        if (!str)
            break;
        Set_put(s->strings, str);
        n += strlen(str1) + SEP_SZ;
        str1 = str2 + SEP_SZ;
        j++;
    }
    free(buffer);
    return j;
}

void strdata_clear(T s)
{
    int i;
    assert(s);
    for (i = 0; i < Set_length(s->strings); i++) {
        free(Set_get(s->strings, i));
    }
    Set_clear(s->strings);
    s->cur_readed = 0;
}



