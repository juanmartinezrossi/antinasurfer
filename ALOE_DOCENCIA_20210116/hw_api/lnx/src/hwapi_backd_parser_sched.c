/*
 * hwapi_backd_parser_sched.c

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

char valgrind_tool[SYSTEM_STR_MAX_LEN];



int search_in_csv(char *value, char *find) {
	char *token;
	int j=0;

	token = strtok (value, ",");

	while(token!=NULL) {
		if (strcmp(token,find)) {
			token = strtok (NULL, ",");
			j++;
		} else {
			return j;
		}
	}
	return -1;
}


int missing_section_objects(struct hw_api_db *hwapi) {

	int i;
	hwapi->default_delay=1;
	hwapi->external_delay=1;
	hwapi->obj_prio=SCHED_PRIO;
	hwapi->cpu_info.exec_order=SCHED_REVERSE;
	for (i=0;i<MAX_CORES;i++) {
		hwapi->core_mapping[i]=i;
	}
	return 1;
}


int read_cores(char *pvalue, char *cpuid_mask, int max_c)  {
    int i,j;
    char *x;
    char num[16];

    j=0;
    for (i=0;i<max_c;i++) {
        x=strdup(pvalue);
        snprintf(num,16,"%d",i);
    	if (search_in_csv(x,num)>-1) {
    		cpuid_mask[j]=i;
    		j++;
    	}
        free(x);
    }
    return i;
}


int parse_section_objects(Set_o keys, struct hw_api_db *hwapi, int idx) {
	key_o key;
	int i;

	if ((key = Set_find(keys, "valgrind", key_findname))) {
		str_get(key->pvalue, hwapi->valgrind_objects, SYSTEM_STR_MAX_LEN);
	}
	if ((key = Set_find(keys, "valgrind_tool", key_findname))) {
		str_get(key->pvalue, hwapi->valgrind_tool, SYSTEM_STR_MAX_LEN);
	}

	if ((key = Set_find(keys, "int_obj_delay", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->default_delay);
	} else {
		printf("Missing internal objects delay in scheduling config file. Setting to 1.\n");
		hwapi->default_delay=1;
	}

	if ((key = Set_find(keys, "ext_obj_delay", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->external_delay);
	} else {
		printf("Missing external objects delay in scheduling config file. Setting to 1.\n");
		hwapi->external_delay=1;
	}

	if ((key = Set_find(keys, "obj_prio", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->obj_prio);
	} else {
		printf("Missing objects maximum priority. Setting to %d\n",SCHED_PRIO);
		hwapi->obj_prio=SCHED_PRIO;
	}

	if ((key = Set_find(keys, "core_mapping", key_findname))) {
		for (i=0;i<MAX_CORES;i++) {
			hwapi->core_mapping[i]=-1;
		}
		read_cores(str_str(key->pvalue),hwapi->core_mapping,MAX_CORES);
	} else {
		for (i=0;i<MAX_CORES;i++) {
			hwapi->core_mapping[i]=i;
		}
	}

	if ((key = Set_find(keys, "obj_sched", key_findname))) {
		if (!str_scmp(key->pvalue,"ord")) {
			hwapi->cpu_info.exec_order=SCHED_ORDERED;
		} else if (!str_scmp(key->pvalue,"rev")) {
			hwapi->cpu_info.exec_order=SCHED_REVERSE;
		} else {
			printf("Unkown scheduling type %s\n",str_str(key->pvalue));
			return 0;
		}
	} else {
		printf("Missing scheduling order. Setting to ordered\n");
		hwapi->cpu_info.exec_order=SCHED_ORDERED;
	}

	return 1;
}


int missing_section_network(struct hw_api_db *hwapi) {
	hwapi->net_db.base_prio=BASE_PRIO;
	hwapi->net_db.itf_prio=ITF_PRIO;
	hwapi->net_db.base_cpuid=0;
	hwapi->net_db.itf_cpuid=0;
	return 1;
}

int parse_section_network(Set_o keys, struct hw_api_db *hwapi, int idx) {
	key_o key;

	if ((key = Set_find(keys, "base_prio", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->net_db.base_prio);
	} else {
		printf("Missing base_prio. Setting to %d.\n",BASE_PRIO);
		hwapi->net_db.base_prio=BASE_PRIO;
	}

	if ((key = Set_find(keys, "itf_prio", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->net_db.itf_prio);
	} else {
		printf("Missing itf_prio. Setting to %d.\n",ITF_PRIO);
		hwapi->net_db.itf_prio=ITF_PRIO;
	}

	if ((key = Set_find(keys, "base_cpuid", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->net_db.base_cpuid);
	} else {
		printf("Missing base_cpuid. Setting to 0.\n");
		hwapi->net_db.base_cpuid=0;
	}

	if ((key = Set_find(keys, "itf_cpuid", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->net_db.itf_cpuid);
	} else {
		printf("Missing itf_cpuid. Setting to 0.\n");
		hwapi->net_db.itf_cpuid=0;
	}

	return 1;
}

int read_daemons(char *pvalue, struct launch_daemons *daemons, int max_d)  {
    int i;
    char *x;

    /** read comma-sepparated values */
    for (i=0;i<max_d;i++) {
        x=strdup(pvalue);
    	if (search_in_csv(x,daemons[i].path)>-1) {
    		daemons[i].runnable=1;
    	}
        free(x);
    }
    return i;
}


int read_valgrind(char *pvalue, struct launch_daemons *daemons, int max_d)  {
    int i;
    char *x;

    /** read comma-sepparated values */
    for (i=0;i<max_d;i++) {
        x=strdup(pvalue);
        if (search_in_csv(x,daemons[i].path)>-1) {
    		daemons[i].valgrind=1;
    	}
        free(x);
    }

    return i;
}

int missing_section_priorities(struct hw_api_db *hwapi) {
	int i;
	for (i=0;i<MAX_DAEMONS;i++) {
		hwapi->daemons[i].prio=DAEMONS_PRIO;
	}
	return 1;
}

int parse_section_priorities(Set_o keys, struct hw_api_db *hwapi, int idx) {
	key_o key;
	int i;

	for (i=0;i<MAX_DAEMONS;i++) {
		if ((key = Set_find(keys, hwapi->daemons[i].path, key_findname))) {
			key_value(key,1,PARAM_INT,&hwapi->daemons[i].prio);
		} else {
			printf("Missing priority for daemon %s. Setting to %d.\n",hwapi->daemons[i].path,DAEMONS_PRIO);
			hwapi->daemons[i].prio=DAEMONS_PRIO;
		}
	}
	return 1;
}


int missing_section_affinity(struct hw_api_db *hwapi) {
	int i;
	for (i=0;i<MAX_DAEMONS;i++) {
		hwapi->daemons[i].cpuid=-1;
	}
	return 1;
}

int parse_section_affinity(Set_o keys, struct hw_api_db *hwapi, int idx) {
	key_o key;
	int i;

	for (i=0;i<MAX_DAEMONS;i++) {
		if ((key = Set_find(keys, hwapi->daemons[i].path, key_findname))) {
			key_value(key,1,PARAM_INT,&hwapi->daemons[i].cpuid);
		} else {
			printf("Missing cpuid for daemon %s. Setting to 0.\n",hwapi->daemons[i].path);
			hwapi->daemons[i].cpuid=0;
		}
	}
	return 1;
}

int parse_section_kernel(Set_o keys, struct hw_api_db *hwapi, int idx) {
	key_o key;

	if ((key = Set_find(keys, "prio", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->kernel_prio);
	} else {
		printf("Missing kernel_prio. Setting to %d.\n",RUNPH_PRIO);
		hwapi->kernel_prio=RUNPH_PRIO;
	}

	if ((key = Set_find(keys, "cpuid", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->kernel_cpuid);
	} else {
		printf("Missing kernel_cpuid. Setting to %d.\n",0);
		hwapi->kernel_cpuid=0;
	}

	if ((key = Set_find(keys, "cmd_prio", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->kernel_cmd_prio);
	} else {
		printf("Missing cmd_prio. Setting to %d.\n",RUNPH_CMD_PRIO);
		hwapi->kernel_cmd_prio=RUNPH_CMD_PRIO;
	}

	if ((key = Set_find(keys, "cmd_cpuid", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->kernel_cmd_cpuid);
	} else {
		printf("Missing cmd_cpuid. Setting to %d.\n",0);
		hwapi->kernel_cmd_cpuid=0;
	}

	if ((key = Set_find(keys, "dac_prio", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->kernel_dac_prio);
	} else {
		printf("Missing dac_prio. Setting to %d.\n",RUNPH_DAC_PRIO);
		hwapi->kernel_dac_prio=RUNPH_DAC_PRIO;
	}

	if ((key = Set_find(keys, "dac_cpuid", key_findname))) {
		key_value(key,1,PARAM_INT,&hwapi->kernel_dac_cpuid);
	} else {
		printf("Missing dac_cpuid. Setting to %d.\n",0);
		hwapi->kernel_dac_cpuid=0;
	}

	if ((key = Set_find(keys, "daemons", key_findname))) {
		read_daemons(str_str(key->pvalue),hwapi->daemons,MAX_DAEMONS);
	} else {
		printf("Missing daemons ");
	}

	if ((key = Set_find(keys, "valgrind_daemons", key_findname))) {
		read_valgrind(str_str(key->pvalue),hwapi->daemons,MAX_DAEMONS);
	}

	if ((key = Set_find(keys, "valgrind_tool", key_findname))) {
		str_get(key->pvalue, valgrind_tool, SYSTEM_STR_MAX_LEN);
	} else {
		strcpy(valgrind_tool,VALGRIND_DEFAULT_TOOL);
	}

	return 1;
}
