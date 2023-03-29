/*
 * bridge.h
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


#define BRIDGE_IDENT_MAGIC	0x00000000 /* replaces header, can't be any combination of possible obj_id or itf_id */

typedef struct ph_header {
    itfid_t ItfId;
    objid_t ObjId;
    int obj_tstamp;
} data_header_t;

#define PH_HEADER_SZ (sizeof (struct ph_header))

/* DEFINITIONS FOR BRIDGE itf TABLE*/
struct bridge_itf {
    int fd_r;
    int fd_w;
    data_header_t dest;
    int mode;
};

struct ext_itf {
    xitfid_t id;
    hwitf_t itf_id;
    int fd;
    int xitf_pos;
};

struct bridge_ident {
    int magic;
    int pe_id;
    struct hwapi_xitf_i xitf;
};

#define BIDENT_PKT_SZ	(sizeof(struct bridge_ident))
#ifndef BRIDGE_H_
#define BRIDGE_H_

#endif /*BRIDGE_H_*/
