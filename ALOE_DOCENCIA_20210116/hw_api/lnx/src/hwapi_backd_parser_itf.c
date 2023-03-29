/*
 * hwapi_backd_parser_itf.c

 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>
 * All rights reserved.
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



/** System includes */
#define _GNU_SOURCE
#define _SVID_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <time.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <pthread.h>
#include <sys/msg.h>


/** HWAPI includes */
#include "phal_hw_api.h"
#include "hwapi_backd.h"
#include "hwapi_netd.h"
#include "params.h"



#include "hwapi_backd_parser.h"

extern int verbose;



int parse_section_interfaces(Set_o keys, struct hw_api_db *hwapi, int idx) {
	key_o key;
	struct ext_itf *ext_itf;

	ext_itf=&hwapi->net_db.ext_itf[idx];

	/** Read interface id */
	if ((key = Set_find(keys, "id", key_findname))) {
		key_value(key,1,PARAM_HEX,&ext_itf->id);
		ext_itf->info.id=ext_itf->id;
	} else {
		printf("Error missing interface id\n");
		return 0;
	}

	/** Read interface bw */
	if ((key = Set_find(keys, "mbps", key_findname))) {
		key_value(key,1,PARAM_FLOAT,&ext_itf->info.BW);
	} else {
		printf("Error missing interface bandwidth, setting to 100 MBPS\n");
		ext_itf->info.BW=100.0;
	}

	/** Read ip address */
	if ((key = Set_find(keys, "address", key_findname))) {
		str_get(key->pvalue,ext_itf->address,16);
	} else {
		printf("Error missing interface address\n");
		return 0;
	}

	/** Read port */
	if ((key = Set_find(keys, "port", key_findname))) {
		key_value(key,1,PARAM_INT,&ext_itf->port);
	} else {
		printf("Error missing interface port\n");
		return 0;
	}

	/** Read itf mode */
	if ((key = Set_find(keys, "mode", key_findname))) {
		if (!str_scmp(key->pvalue, "out"))
			ext_itf->mode = FLOW_WRITE_ONLY;
		else if (!str_scmp(key->pvalue, "in"))
			ext_itf->mode = FLOW_READ_ONLY;
		else if (!str_scmp(key->pvalue, "inout"))
			ext_itf->mode = FLOW_READ_WRITE;
		else if (!str_scmp(key->pvalue, "outin"))
			ext_itf->mode = FLOW_WRITE_READ;
		else {
			printf("Invalid xitf mode %s\n",str_str(key->pvalue));
			return 0;
		}
		ext_itf->info.mode=ext_itf->mode;
	} else {
		printf("Error missing interface mode\n");
		return 0;
	}

	/** Read itf mode */
	if ((key = Set_find(keys, "type", key_findname))) {
		if (!str_scmp(key->pvalue, "tcp"))
			ext_itf->type=NET_TCP;
		else if (!str_scmp(key->pvalue, "udp")) {
			ext_itf->type=NET_UDP;
		} else {
			printf("Unknown interface type %s\n",str_str(key->pvalue));
		}
	} else {
		ext_itf->type=NET_TCP;
	}

	if ((key = Set_find(keys, "fragment", key_findname))) {
		key_value(key,1,PARAM_BOOLEAN,&ext_itf->fragment);
	} else {
		ext_itf->fragment=1;
	}

	hwapi->net_db.nof_ext_itf++;

	return 1;
}



