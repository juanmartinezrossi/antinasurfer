/*
 * swman.h
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
#ifndef SWMAN_H_
#define SWMAN_H_

#define MAX_PATH_LEN 512


#define NOF_PLATS 5

/** EXECUTABLE RELATED CONSTANTS
*/  
#define SWMAN_PROCMODE_LINKED           0x1
#define SWMAN_PROCMODE_COMPILED         0x2
#define SWMAN_PROCMODE_SOURCE           0x3
#define SWMAN_PROCMODE_RELOCATABLE      0x4
#define SWMAN_PROCMODE_EXECUTABLE       0x5

#define DEFAULT_PKT_SZ                  (10*1024)
/*AGBJULY15 
	#define EXECS_BASE_DIR                  "swman_execs"
  #define EXECINFO_PATH                   "swman_execs/execinfo"
*/

#define EXECS_BASE_DIR                  "../../../SWMAN_EXECS"
#define EXECINFO_PATH                   "../../../SWMAN_EXECS/execinfo"

struct pinfo
{
  int blen;
  unsigned int mem_start;
  unsigned int mem_sz;
};


enum mem_mode {ABSOLUTE,RELATIVE};

struct platforms_cfg {
    pltid_t plt_id;
    enum mem_mode mem_mode;
    char *dir;
    char *name;
};



#endif /*SWMAN_H_*/
