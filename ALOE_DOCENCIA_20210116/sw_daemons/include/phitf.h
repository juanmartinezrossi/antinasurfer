/*
 * phitf.h
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
#ifndef PHITF_INCLUDED
#define PHITF_INCLUDED

#define T phitf_o

#define phitf_sizeof sizeof(struct phitf_o)

/** @defgroup ALOE Interface Propierties
 *
 * @ingroup common_phal
 *
 * @todo Define this
 * @todo rtcitf reqs??
 * 
 * ALOE Interface
 */

typedef struct T *T;

struct T {
    char mode;
    unsigned char delay;
    itfid_t id;
    itfid_t remote_id;
    objid_t remote_obj_id;
    xitfid_t xitf_id;
    float bw_req;
    int fifo_usage;

    str_o name;
    str_o remotename;
    str_o remoteobjname;
};

struct topkt {
    char mode;
    unsigned char delay;
    itfid_t id;
    objid_t remote_obj_id;
    itfid_t remote_id;
    short dummy2;
    xitfid_t xitf_id;
    float bw_req;
    int fifo_usage;
};

T phitf_new(char mode);
void phitf_delete(T *itf);
T phitf_dup(T itf);
int phitf_isinternal(T itf);
int phitf_iswriteonly(T itf);
int phitf_isreadonly(T itf);
int phitf_isreadwrite(T itf);
int phitf_findname(const void *x, const void *member);
int phitf_topkt(void *x, char **start, char *end);
void * phitf_newfrompkt(char **start, char *end);
void phitf_xdelete(void **x);
void * phitf_xdup(const void *x);


/** @} */
#undef T  
#endif /*  */
