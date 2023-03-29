/*
 * hwman.c
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

/* standard libraries */
#include <stdlib.h>
#include <string.h>
#include "phal_hw_api.h"

#include "phid.h"
#include "phal_daemons.h"

/* common objects */
#include "pkt.h"
#include "set.h"
#include "daemon.h"

/* include command declarations */
#include "swload_cmds.h"
#include "hwman_cmds.h"

/* main function declaration */
void hwman_run();
void hwman_alloc();
int hwman_init(daemon_o daemon);
void hwman_background(int);

#ifdef RUN_AS_PROC

int main()
{
	if (!hwman_init_p())
		return 0;
	hwman_run();
		return 1;
}
#endif

cmd_t hwman_cmds[] = {
    {
        HWMAN_LISTCPU, hwman_incmd_listcpu, "List processors", "Error listing processors"
    },
    {
        HWMAN_MAPOBJ, hwman_incmd_swmapper, "Map (allocate) software", "Error mapping software"
    },
    {
        HWMAN_UNMAPOBJ, hwman_incmd_swunmap, "Unmap (deallocate) software", "Error unmapping software"
    },
    {
        HWMAN_ADDITF, hwman_incmd_additf, "Add interface", "Error adding interface"
    },
    {
        HWMAN_UPDCPU, hwman_incmd_updcpu, "Update processor", "Error updating processor"
    },
    {
        HWMAN_ADDCPU, hwman_incmd_addcpu, "Add processor", "Error adding processor"
    },
    { 0, NULL, NULL, NULL}};

daemon_o hwman_daemon;


int hwman_init_p(void *ptr, int daemon_idx) {
	int n;

#ifdef RUN_AS_PROC
	if (!hwapi_init ()) {
#else
	if (!hwapi_init_noprocess(ptr,daemon_idx)) {
#endif
		printf("HWMAN: Error initiating api\n");
		return 0;
	}




	hwman_alloc();
	hwman_daemon = daemon_new(DAEMON_HWMAN, hwman_cmds);
	if (!hwman_daemon) {
		xprintf("HWMAN: Error creating hwman_daemon\n");
		return 0;
	}
	hwman_init(hwman_daemon);

	daemon_info(hwman_daemon,"Init ok");

	return 1;
}

int hwman_process_cmd(int blocking) {
	int n;
	do {
		n = daemon_rcvpkt(hwman_daemon, blocking);
		if (n > 0) {
			daemon_processcmd(hwman_daemon);
		}
		if (n < 0) {
			return 0;
		}
	} while (n);
	if (n < 0) {
		return 0;
	}
	return 1;
}


void hwman_delete() {
	daemon_delete(&hwman_daemon);
}


/** hwman hwman_daemon Main Routine
 */
void hwman_run() {

#ifndef RUN_NONBLOCKING
	hwapi_addperiodfunc(hwman_background,CHECK_PROCESSORS_PERIOD);
#endif

	while (1) {

#ifdef RUN_NONBLOCKING
		hwman_process_cmd(0);
		hwman_background(-1);
		hwapi_relinquish_daemon();
#else
		hwman_process_cmd(1);
#endif
	}

	hwman_delete(&hwman_daemon);

	exit(-1);
}
