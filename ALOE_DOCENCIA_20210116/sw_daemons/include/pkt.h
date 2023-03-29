/*
 * pkt.h
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
/** @defgroup pkt ALOE Command Packet
 * @ingroup common_phal
 *
 * This object is used to manage in/out packets.
 *
 * Indeed, this function manages just buffers, it does
 * not neither create them or send/receive them. Just
 * a buffer is assigned to them during their creation
 * and functions are provided to access both its contents
 * and its header (src processor, destination processor
 * and daemon, etc).
 *
 * The contents of the packet are accessed in terms of
 * fields. A field is a portion of data associated
 * with a constant. In the transmission of the packet
 * fields are created and associated. In the reception,
 * user can quickly access the contents by just referencing
 * the desired field code.
 *
 * @{
 */

#ifndef INPKT_INCLUDED
#define INPKT_INCLUDED

#define T pkt_o

#define uint unsigned int

#define pkt_sizeof (sizeof (struct pkt_o))

typedef unsigned char fieldcode_t;
typedef unsigned short fieldsz_t;

typedef struct T *T;

#define MAX_FIELDS 		10

struct fields {
    fieldsz_t size;
    fieldcode_t code;
    unsigned char magic;
};

#define FIELD_SZ (sizeof(struct fields))

struct T {
    int *ptr;
    int len; /**< internal len is in bytes */
    int buff_sz;
    int num_fields;
    struct header *head;
    struct fields * fields[MAX_FIELDS];
};

/** Daemon packets 
 */
struct header {
    peid_t dst_pe;
    peid_t src;
    cmdid_t cmd;
    dmid_t dst_daemon;
};

#define HEAD_SZ (sizeof(struct header))


typedef int (*pkt_putfnc) (const void *x, void *data, int max_size);
typedef void * (*pkt_getfnc) (void *data, int *len);

T pkt_new(int *ptr, int buff_sz);
void pkt_delete(T * pkt);
int pkt_readvalues(T pkt, int size_pkt);
void pkt_clear(T pkt);
int pkt_getfieldcount(T pkt);
int pkt_freesz(T pkt);
int pkt_len(T pkt);
int pkt_wlen(T pkt);
void * pkt_get(T pkt, fieldcode_t field_code,
        void * newfrompkt(char **start, char *end));
void * pkt_getptrandsz(T pkt, fieldcode_t field_code, int *sz);
void * pkt_getptr(T pkt, fieldcode_t field_code);
fieldsz_t pkt_getvaluesz(T pkt, fieldcode_t field_code);
uint pkt_getvalue(T pkt, fieldcode_t field_code);
int pkt_put(T pkt, fieldcode_t field_code,
        void *member,
        int topktf(void *x, char **start, char *end));
void * pkt_putptr(T pkt, fieldcode_t field_code,
        fieldsz_t val_sz);
int pkt_putvalue(T pkt, fieldcode_t field_code,
        uint value);
void pkt_setdestpe(T pkt, peid_t pe);
peid_t pkt_getsrcpe(T pkt);
void pkt_setdestdaemon(T pkt, dmid_t daemon);
cmdid_t pkt_getcmd(T pkt);
void pkt_setcmd(T pkt, cmdid_t cmd);
int pkt_putfloat(T pkt, fieldcode_t field_code, float value);
float pkt_getfloat(T pkt, fieldcode_t field_code);

/** @} */


#undef T

#endif /*  */
