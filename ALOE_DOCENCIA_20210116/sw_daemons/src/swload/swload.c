/*
 * swload.c
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

#include "pkt.h"
#include "set.h"
#include "daemon.h"

#include "swload_cmds.h"

/* main function declaration */
void swload_run();
void swload_alloc();
int swload_init(daemon_o dameon);


int swload_init(daemon_o d);

static cmd_t swload_cmds[] = {{
	SWLOAD_LOADOBJ, swload_incmd_loadobj, "Load a list of objects", "Error loading objects"}, {
	SWLOAD_EXECERR, swload_incmd_execerr, "Error loading object", "Error loading object"}, {
        SWLOAD_EXECINFO, swload_incmd_execinfo, "Allocate object", "Error allocating object"}, {
	SWLOAD_LOADSTART, swload_incmd_loadstart, "Start object download", "Error loading object"}, {
	SWLOAD_LOADING, swload_incmd_loading, "Download object", "Error loading object"}, {
	SWLOAD_LOADEND, swload_incmd_loadend, "Launch object", "Error launching object"}, { 0, NULL, NULL, NULL}};


#ifdef RUN_AS_PROC
int main()
{
	if (!swload_init_p())
		return 0;
	swload_run();
		return 1;
}
#endif

daemon_o swload_daemon;


int swload_init_p(void *ptr, int daemon_idx) {
	int n;

#ifdef RUN_AS_PROC
	if (!hwapi_init ()) {
#else
	if (!hwapi_init_noprocess(ptr, daemon_idx)) {
#endif
		printf("SWLOAD: Error initiating api\n");
		return 0;
	}

	swload_alloc();

	swload_daemon = daemon_new(DAEMON_SWLOAD, swload_cmds);
	if (!swload_daemon) {
		xprintf("SWLOAD: Error creating swload_daemon\n");
		return 0;
	}

	swload_init(swload_daemon);

	daemon_info(swload_daemon,"Init ok");

	return 1;
}

int swload_process_cmd(int blocking) {
	int n;
	do {
		n = daemon_rcvpkt(swload_daemon, blocking);
		if (n > 0) {
			daemon_processcmd(swload_daemon);
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

void swload_delete() {
	daemon_delete(&swload_daemon);
}
/** stats swload_daemon Main Routine
 */
void swload_run() {

	while (1) {

#ifndef RUN_NONBLOCKING
		swload_process_cmd(1);
#else
		swload_process_cmd(0);
		hwapi_relinquish_daemon();
#endif
	}

	swload_delete(&swload_daemon);

	exit(-1);
}

