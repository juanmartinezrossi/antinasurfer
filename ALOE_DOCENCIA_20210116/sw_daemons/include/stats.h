/*
 * stats.h
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

#define STATS_MAX_LENGTH 64*1024

#define VAR_IS_PMVALOK	1
#define VAR_IS_PMVALERR	2
#define VAR_IS_PMINIT	5
#define VAR_IS_PMCLOSE	6
#define VAR_IS_PMGET	7
#define VAR_IS_PMINITOK	8
#define VAR_IS_PMINITERR 9


#define VAR_IS_STINIT	3
#define VAR_IS_STCLOSE	4
#define VAR_IS_STOK		10
#define VAR_IS_STERR	11

#define VAR_IS_STOBJECTINIT     	20
#define VAR_IS_STOBJECTOK			21
#define VAR_IS_STOBJECTCLOSE		22
#define VAR_IS_STOBJECTERR			23
#define VAR_IS_SETOBJECT     		24

#define VAR_IS_LOGINIT		12
#define VAR_IS_LOGCLOSE		13
#define VAR_IS_LOGINITOK	14
#define VAR_IS_LOGINITERR	15
#define VAR_IS_LOGWRITE		16
#define VAR_IS_LOGFLUSHED       19


#define STAT_TYPE_CHAR		1
#define STAT_TYPE_UCHAR		2
#define STAT_TYPE_FLOAT		3
#define STAT_TYPE_INT		4
#define STAT_TYPE_SHORT		5
#define STAT_TYPE_TEXT		6
#define STAT_TYPE_STRING	6
#define STAT_TYPE_COMPLEX 	7



