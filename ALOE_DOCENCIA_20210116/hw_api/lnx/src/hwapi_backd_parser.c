/*
 * hwapi_backd_parser.c
 *
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


struct sections {
	char *name;
	int (*parse_fnc) (Set_o keys, struct hw_api_db *hwapi, int idx);
	int (*missing_fnc) (struct hw_api_db *hwapi);
};

#define PLTCONF_NOF_SECT 3
struct sections pltconf_sects[]= {
	{"platform",parse_section_platform, NULL},
	{"dac",parse_section_dac, NULL},
	{"other",parse_section_other, NULL},
};

#define XITFCONF_NOF_SECT 1
struct sections xitfconf_sects[] = {
	{"xitf",parse_section_interfaces, NULL},
};

#define SCHEDCONF_NOF_SECT 5
struct sections schedconf_sects[] = {
	{"objects",parse_section_objects, missing_section_objects},
	{"kernel",parse_section_kernel, NULL},
	{"swdaemon_priorities",parse_section_priorities, missing_section_priorities},
	{"swdaemon_cpuid",parse_section_affinity, missing_section_affinity},
	{"network",parse_section_network, missing_section_network},
};

int parse_file(struct hw_api_db *hwapi, char *filename, struct sections *sections, int nof_sections) {
	int n,i,j;
	cfg_o cfg;
	char *buffer;
	sect_o sect;
	key_o key;
	int error;
	int offset;


	printf("hwapi_backd_parser.c==>parse_file()=%s\n", filename);

	offset = hwapi_res_parseall_(filename, &buffer,1);
	if (offset < 0) {
		printf ("HWAPI_BACKD: Error reading file %s\n", filename);
		return 0;
	}

	cfg = cfg_new(buffer, offset);
	if (!cfg) {
		printf ("HWAPI_BACKD: Error reading configuration in file name %s\n", filename);
		free(buffer);
		return 0;
	}
	error=0;

	/* First set defaults if any section is missing in the configuration file */
	for (i=0;i<nof_sections;i++) {
		if (!Set_find(cfg_sections(cfg),sections[i].name,sect_findtitle)) {
			if (!sections[j].missing_fnc) {
				printf("Pre-defined section %s not found in filename %s\n",sections[i].name,filename);
				error=1;
			} else {
				if (!sections[j].missing_fnc(hwapi)) {
					printf("Error setting default values for section %s\n",sections[i].name);
					error=1;
				}
			}
		}
	}

	/** Now read and process rest of sections */
	for (i=0;i<Set_length(cfg_sections(cfg)) && !error;i++) {
		sect = Set_get(cfg_sections(cfg),i);
		assert(sect);
		j=0;
		while (j<nof_sections && !error && strcmp(sect_title(sect),sections[j].name)) {
			j++;
		}
		if (j==nof_sections && !error) {
			printf("Unknown section %s in filename %s\n",sect_title(sect),filename);
			error=1;
		} else {
			if (!sections[j].parse_fnc(sect_keys(sect),hwapi,i)) {
				printf("Error reading values in section %s filename %s\n",sections[j].name,filename);
				error=1;
			}
		}
	}

	free(buffer);
	cfg_delete(&cfg);

	if (error) {
		return 0;
	} else {
		return 1;
	}

}


int parse_platform_file(struct hw_api_db *hwapi, char *filename) {
	char *c;

	printf("O============================================================================================O\n");
	printf("\033[1;35mInitiate Parsing of Platform Config Files\033[0m\n");
	printf("Platform_file(): \033[1;32m%s\033[0m\n", filename);

//	printf("1 parse_platform_file(): %s, c=%s, hwapi->sched_cfg=%s\n", filename, c, hwapi->sched_cfg);

	filename=strdup(filename);

//	printf("2 parse_platform_file(): %s, c=%s, hwapi->sched_cfg=%s\n", filename, c, hwapi->sched_cfg);

	c=rindex(filename,'/');
//	printf("3 parse_platform_file(): %s, c=%s, hwapi->sched_cfg=%s\n", filename, c, hwapi->sched_cfg);
	if (!c) {
		c=filename;
	}
//	printf("4 parse_platform_file(): %s, c=%s, hwapi->sched_cfg=%s\n", filename, c, hwapi->sched_cfg);
//  c++;
	c++;
	if (!parse_file(hwapi,filename,pltconf_sects,PLTCONF_NOF_SECT)) {
		printf("Error parsing platform configuration.\n");
		free(filename);
		return 0;
	}
//	printf("5 parse_platform_file(): %s, c=%s, hwapi->sched_cfg=%s\n", filename, c, hwapi->sched_cfg);
	strcpy(c,hwapi->sched_cfg);
//	printf("parse_scheduling_file(): %s, c=%s, hwapi->sched_cfg=%s\n", filename, c, hwapi->sched_cfg);
	printf("Scheduling_file(): \033[1;32m%s\033[0m\n", filename);
	if (!parse_file(hwapi,filename,schedconf_sects,SCHEDCONF_NOF_SECT)) {
		printf("Error parsing scheduling configuration.\n");
		free(filename);
		return 0;
	}
	strcpy(c,hwapi->xitf_cfg);
	printf("Interfacing_file(): \033[1;32m%s\033[0m\n", filename);
	if (!parse_file(hwapi,filename,xitfconf_sects,XITFCONF_NOF_SECT)) {
		printf("Error parsing external interfaces configuration.\n");
		free(filename);
		return 0;
	}

	free(filename);
	printf("O============================================================================================O\n");
	return 1;
}


