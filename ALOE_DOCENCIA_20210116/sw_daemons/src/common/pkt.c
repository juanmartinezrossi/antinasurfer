/*
 * pkt.c
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

#include "phid.h"
#include "set.h"
#include "pkt.h"

#include "phal_hw_api.h"

#define T pkt_o

#define PKT_FIELD_MAGIC 	0xE8

unsigned short read_ushort(void *x)
{
	unsigned char *z = (unsigned char*) x;
	
	return (unsigned short) (z[1]<<8 | z[0]);
}

unsigned int read_uint(void *x)
{
	unsigned char *z = (unsigned char*) x;
	
	return (unsigned int) (z[3]<<24 | z[2]<<16 | z[1]<<8 | z[0]);
}

void write_ushort(void *x, unsigned short value)
{
	unsigned char *z = (unsigned char*) x;
	
	z[1] = (unsigned char) (value>>8);
	z[0] = (unsigned char) value;

}

void write_uint(void *x, unsigned int value)
{
	unsigned char *z = (unsigned char*) x;
	
	z[3] = (unsigned char) (value>>24);
	z[2] = (unsigned char) (value>>16);
	z[1] = (unsigned char) (value>>8);
	z[0] = (unsigned char) (value);
}


/** Pkt object Constructor
 *
 * Create a pkt object and associates it with a region
 * of memory.
 *
 * @param ptr pointer to the memory region
 * @param buff_sz size of the region
 *
 * @return !=0 pointer to the pkt object
 * @return =0 error creating
 */
T pkt_new(int *ptr, int buff_sz)
{
    T pkt;

    assert(ptr && buff_sz >= 0);

    NEW(pkt);
    if (!pkt) {
        return NULL;
    }

    pkt->ptr = ptr;
    pkt->buff_sz = buff_sz;
    pkt->len = HEAD_SZ; /* this len is in bytes */
    pkt->head = (struct header *) ptr;
    pkt->num_fields = 0;

    /* set first field to null  */
    pkt->fields[0] = NULL;

    return pkt;
}

/** Destructor
 * @warning It does not de-allocate the buffer, just the packet structure
 * for handling it.
 */
void pkt_delete(T * pkt)
{
    assert(pkt);
    assert(*pkt);

    DELETE(*pkt);
    pkt = NULL;
}

/** Read Packet Values
 *
 * This function may be called when a new raw data has been inserted at packet buffer.
 * (for example, when a new packet has been received). This function will first of all
 * clear current packet contents and second scan the packet for fields and prepare
 * the T structure for a faster access to its contents
 *
 * @param pkt Packet object
 * @param size_pkt Size (in bytes) of the packet
 *
 * @return 1 Processed Ok
 * @return 0 Error processing packet
 */
int pkt_readvalues(T pkt, int size_pkt)
{
    int i;
    char *p;
	fieldsz_t size;

    pkt_clear(pkt);

    /* set first pointer to beggining of the packet (after the header) */
    pkt->fields[0] = (struct fields*) (pkt->head + 1);

    i = 0;
    while (pkt->fields[i]->magic == PKT_FIELD_MAGIC && pkt->len < size_pkt && i<MAX_FIELDS-1) {

		size = read_ushort(&pkt->fields[i]->size);

        p = (char*) (pkt->fields[i] + 1);
        p += size;
        pkt->len += size + FIELD_SZ;
        pkt->fields[i+1] = (struct fields*) p;
        pkt->num_fields++;
        i++;
    }

    return 1;
}

void pkt_clear(T pkt)
{
    assert(pkt);

    pkt->len = HEAD_SZ;
    pkt->num_fields = 0;
    memset(pkt->fields, 0, (MAX_FIELDS) * sizeof (struct fields *));
}

int pkt_freesz(T pkt)
{
    assert(pkt);

    return pkt->buff_sz - pkt->len;
}

int pkt_len(T pkt)
{
    assert(pkt);

    return pkt->len;
}

int pkt_getfieldcount(T pkt)
{
    assert(pkt);
    return pkt->num_fields;
}

int find_field(T pkt, fieldcode_t field_code)
{
    int i;
    for (i = 0; pkt->fields[i] && i < pkt->num_fields; i++) {
        if (pkt->fields[i]->code == field_code) {
            break;
        }
    }
    if (i == pkt->num_fields) {
        return -1;
    } else {
        return i;
    }
}

/** Get value
 *
 * Returns a pointer where field contents associated with code
 * val_code are placed in the packet buffer.
 *
 * @param pkt pointer to pkt object
 * @param field_code Code of the field
 *
 * @return pointer T� where received data can be found
 */
void * pkt_getptr(T pkt, fieldcode_t field_code)
{
    return pkt_getptrandsz(pkt, field_code, NULL);
}

/** Get value and size.
 * Same as pkt_getval but saves into sz pointer the size of the packet
 */
void * pkt_getptrandsz(T pkt, fieldcode_t field_code, int *sz)
{
    int i;

    assert(pkt && field_code);

    i = find_field(pkt, field_code);
    if (i < 0) {
        return NULL;
    }
    if (sz) {
        *sz = read_ushort(&pkt->fields[i]->size);
    }
    return (void *) (pkt->fields[i] + 1);
}

/** Get value size
 *
 * This function returns the size of a field in a packet
 *
 * @param pkt pointer to pkt object
 * @param field_code Code of the field
 *
 * @return >=0 Size of the field
 * @return -1 field not found
 */
fieldsz_t pkt_getvaluesz(T pkt, fieldcode_t field_code)
{
    int i;

    assert(pkt && field_code);

    i = find_field(pkt, field_code);

    if (i < 0) {
        return 0;
    } else {
        return read_ushort(&pkt->fields[i]->size);
    }
}

/** Get value contents
 *
 * Use this function to quickly access the contents of an integer
 * (or shorter) value.
 * It is useful to use when pkt_putvalue has been used to put some
 * data in the packet.
 * This function allways returns an unsigned integer. If shorter
 * values have to be used, user should cast the return value to the
 * desired type.
 *
 * @warning This function returns 0 if error. User must find other
 * ways to distinguish this from a correct value.
 *
 * @param pkt pointer to pkt object
 * @param field_code Code of the field
 *
 * @return Value contents
 */
uint pkt_getvalue(T pkt, fieldcode_t field_code)
{
    uint *p;

    p = pkt_getptr(pkt, field_code);
    if (p) {
        return read_uint(p);
    } else {
        return 0;
    }
}

/** Get object
 *
 * Uses the callcack function newfrompkt to create a new
 * object given a field in a packet.
 * Useful to create a new object when pkt_put was used to put
 * it in the packet.
 *
 * @param pkt pointer to pkt object
 * @param field_code Code of the field
 * @param newfrompkt Callback funtion to newfrompkt() object function
 *
 * @return Void pointer to object, NULL if error
 */
void * pkt_get(T pkt, fieldcode_t field_code, void * newfrompkt(char **start, char *end))
{
    char *p;
    int sz;

    assert(pkt && newfrompkt);

    p = pkt_getptrandsz(pkt, field_code, &sz);
    if (p) {
        return newfrompkt(&p, p + sz);
    } else {
        return NULL;
    }
}

/** Put value
 *
 * Creates a new field in the packet of size val_sz and
 * associates it with code val_code
 *
 * If field code exists and val_sz is the same, the contents are overwritten.
 * if the size is not the same, error is returned.
 *
 * A pointer is received indicating the beggining of the memory
 * where user should write field contents.
 *
 * @warning User should be carefull not writting more than val_sz words
 * because they will be overwritten if a new value is put.
 *
 * @param pkt pointer to pkt object
 * @param field_code Code we want to associate with the field (it
 *		should agree with received getval code)
 * @param val_sz Size of the field in bytes
 *
 * @return pointer to where packet data must be written
 */
void * pkt_putptr(T pkt, fieldcode_t field_code, fieldsz_t val_sz)
{
    int i;
    char *p;

    assert(pkt && field_code);

    if (pkt->len + val_sz + FIELD_SZ > pkt->buff_sz) {
        printf("PKT: Error putting data. Size exceeds packet buffer (%d+%d>%d)\n",pkt->len,val_sz+FIELD_SZ,pkt->buff_sz);
        return NULL;
    }

    if (pkt->num_fields == MAX_FIELDS) {
        printf("PKT: Error putting data. No more fields allowed (%d)\n",MAX_FIELDS);
        return NULL;
    }

    i = find_field(pkt, field_code);

    /* if it does not exist, append to the end */
    if (i < 0) {
        i = pkt->num_fields;
    }

    if (i == 0) {
        /* if it's the first one, put it after the header */
        pkt->fields[i] = (struct fields *) (pkt->head + 1);

    } else {
        /* rest of cases, data begins after the field header */
        p = (char*) (pkt->fields[i - 1] + 1);

        /* next field begins N bytes after the beggining of the data */
        pkt->fields[i] = (struct fields*) (p + read_ushort(&pkt->fields[i - 1]->size));
    }

    pkt->fields[i]->magic = PKT_FIELD_MAGIC;
    pkt->fields[i]->code = field_code;
    write_ushort(&pkt->fields[i]->size,val_sz);

    pkt->len += val_sz + FIELD_SZ;
    pkt->num_fields++;

    /* return pointer to data */
    return (void *) (pkt->fields[i] + 1);
}

/** Put single value
 *  
 * The contents are got from value param. It receives an unsigned integer
 * as this is the shortest word size that can be written into a packet. User
 * should cast to this type when calling this function.
 *
 * @param pkt pointer to pkt object
 * @param field_code Code we want to associate with the field (it
 *		should agree with received getval code)
 * @param value Unsigned int value
 *
 * @return 1 if ok, 0 if error
 */
int pkt_putvalue(T pkt, fieldcode_t field_code, uint value)
{
    uint *p;

    p = pkt_putptr(pkt, field_code, sizeof (uint));
    if (p) {
        write_uint(p, value);
	    return 1;
    } else {
        return 0;
    }
}

int pkt_putfloat(T pkt, fieldcode_t field_code, float value)
{
    float *p;

    p = pkt_putptr(pkt, field_code, sizeof (float));
    if (p) {
        *p=value;
	    return 1;
    } else {
        return 0;
    }
}

float pkt_getfloat(T pkt, fieldcode_t field_code)
{
    float *p;

    p = pkt_getptr(pkt, field_code);
    if (p) {
        return *p;
    } else {
        return 0;
    }
}

/** Put object in a packet
 *
 * In this function, an abstract object is copied into a packet using
 * a function applied to it given as a parameter. This function will be
 * applied to the object and will return the amount of bytes copied into the
 * buffer.
 *
 * @param pkt pointer to pkt object
 * @param field_code Code we want to associate with the field (it
 *		should agree with received getval code)
 * @param object Object to wich apply the function
 * @param topktf Function to copy the object contents to the buffer
 *
 * @return 1 if ok, 0 if error
 */
int pkt_put(T pkt, fieldcode_t field_code, void *object, int topktf(void *x, char **start, char *end))
{
    int i, objsize;
    char *p, *s;

    assert(pkt && field_code && object && topktf);

    p = (char *) pkt_putptr(pkt, field_code, 0);
    if (!p) {
        printf("Can't put ptr\n");
        return 0;
    }
    s = p;
    if (!topktf(object, &p, p + pkt_freesz(pkt))) {
        pkt_clear(pkt);
        printf("Can't put object\n");
        return 0;
    }
    objsize = p - s;

    /* execution error if object size is negative */
    assert(objsize >= 0);

    i = find_field(pkt, field_code);

    /* if not found, means strange error because we just create it */
    assert(i >= 0);

    write_ushort(&pkt->fields[i]->size, objsize);
    pkt->len += objsize;

    return objsize;
}

void pkt_setdestpe(T pkt, peid_t pe)
{
    assert(pkt);

    pkt->head->dst_pe = pe;
    pkt->head->src = 0;

}

peid_t pkt_getsrcpe(T pkt)
{
    assert(pkt);

    return pkt->head->src;
}

void pkt_setdestdaemon(T pkt, dmid_t daemon)
{
    assert(pkt);

    pkt->head->dst_daemon = daemon;
}

cmdid_t pkt_getcmd(T pkt)
{
    assert(pkt);

    return pkt->head->cmd;
}

void pkt_setcmd(T pkt, cmdid_t cmd)
{
    assert(pkt);

    pkt->head->cmd = cmd;
}

