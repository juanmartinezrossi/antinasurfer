/*
 * downexe.h
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

#ifndef DOWNEXE_INCLUDED
#define DOWNEXE_INCLUDED

#define T downexe_o

#define downexe_sizeof sizeof(struct downexe_o)

/** @defgroup ALOE Interface Propierties
 *
 * @ingroup common_phal
 *
 * @todo Define this
 * 
 * ALOE Interface
 */


typedef struct T *T;

struct T {
    str_o name;
    Set_o objects;
    int id;
    struct pinfo pinfo;
    unsigned int checksum;
    int rpm;
    char* data;

};


T downexe_new();
void downexe_delete(T * exe);
int downexe_topkt(const void *x, void *data, int size);
int downexe_findname(const void *x, const void *member);
int downexe_findid(const void *x, const void *member);




/** @} */
#undef T  
#endif /*  */
