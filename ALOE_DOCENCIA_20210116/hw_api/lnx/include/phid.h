/*
 * phid.h
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>. All rights reserved.
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
#ifndef ID_INCLUDED
#define ID_INCLUDED

/** @defgroup id ALOE Id
 * @ingroup common_phal
 *
 * Definition of types for most of numerical identifications.
 *
 * @{
 */ 

typedef unsigned int 	uint;	


typedef unsigned short 	ushort;
#define objid_t		ushort
#define itfid_t		ushort
#define varid_t		ushort
#define peid_t		ushort
#define pltid_t		ushort
#define varsz_t		ushort
#define cmdid_t		ushort
#define dmid_t		ushort

typedef unsigned char 	uchar;
#define xitfid_t	uchar
#define status_t	uchar
#define hwitf_t         uchar
#define appid_t 	uchar

#define vartype_t       int


/** @} */ 
  
#undef T
#endif /*  */
