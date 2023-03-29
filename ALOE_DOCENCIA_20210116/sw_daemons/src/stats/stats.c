/*
 * stats.c
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
/**	PHAL STATS SW Daemon Implementation
 * 
 * FILE: stats.c
 * 
 * Daemon type: Sensor
 * 
 * Communicates with: STATS MANAGER
 * 
 * Functions: This sensor manages statistics variables and 
 * 	initialization parameter values for the objects.
 * 	
 * 	In one side, it comunicates with every object (through
 * 	an interface for initialization procedures and through
 * 	hwapi services during run-time).
 * 	
 * 	In the other, it comunicates with statsAGER initializating
 * 	or providing/modifying values under request.
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
#include "stats_cmds.h"

#include "stats.h"

/* main function declaration */
void stats_run();
void stats_alloc();
int stats_init(daemon_o daemon);
int stats_background();

static cmd_t stats_cmds[] = {{
	STATS_STINITOK, stats_incmd_initack, "Stat initialization ack", "Error initializing stats"}, {
	STATS_STINITERR, stats_incmd_initack, "Stat initialization error", "Error initializing stats"}, {
	STATS_PMINITOK, stats_incmd_initack, "Parameter initialization ack", "Error initializing params"}, {
	STATS_PMINITERR, stats_incmd_initack, "Parametere initialization error", "Error initializing params"}, {
	STATS_LOGINITOK, stats_incmd_initack, "Log initialization ack", "Error initializing logs"}, {
	STATS_LOGINITERR, stats_incmd_initack, "Log initialization error", "Error initializing logs"}, {
	STATS_STSET, stats_incmd_stact, "Set stat value", "Error setting statistics"}, {
	STATS_STGET, stats_incmd_stact, "Get stat value", "Error getting statistics"}, {
	STATS_STREPORT, stats_incmd_stact, "Report stat value", "Error reporting statistics"}, {
	STATS_PMVALOK, stats_incmd_pmval, "Parameter value", "Error getting parameter"}, {
	STATS_PMVALERR, stats_incmd_pmval, "Parameter value error", "Error getting parameter"}, { 0, NULL, NULL, NULL}};



#ifdef RUN_AS_PROC
int main()
{
	if (!stats_init_p())
		return 0;
	stats_run();
		return 1;
}
#endif

daemon_o stats_daemon;


int stats_init_p(void *ptr, int daemon_idx) {
	int n;

#ifdef RUN_AS_PROC
	if (!hwapi_init ()) {
#else
	if (!hwapi_init_noprocess(ptr, daemon_idx)) {
#endif
		printf("STATS: Error initiating api\n");
		return 0;
	}

	hwapi_schedtimer();
	stats_alloc();

	stats_daemon = daemon_new(DAEMON_STATS, stats_cmds);
	if (!stats_daemon) {
		xprintf("STATS: Error creating stats_daemon\n");
		return 0;
	}

	stats_init(stats_daemon);

	daemon_info(stats_daemon,"Init ok");

	return 1;
}

int stats_process_cmd(int blocking) {
	int n;
	do {
		n = daemon_rcvpkt(stats_daemon, blocking);
		if (n > 0) {
			daemon_processcmd(stats_daemon);
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

void stats_delete() {
	daemon_delete(&stats_daemon);
}

/** stats stats_daemon Main Routine
 */
void stats_run() {

	while (1) {

		stats_process_cmd(0);

		stats_background();

		hwapi_relinquish_daemon();
	}

	stats_delete(&stats_daemon);

	exit(-1);
}
