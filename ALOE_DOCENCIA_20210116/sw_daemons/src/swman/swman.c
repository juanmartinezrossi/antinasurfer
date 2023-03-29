/*
 * swman.c
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

#include <stdlib.h>
#include <string.h>

#include "phal_hw_api.h"

#include "phid.h"
#include "phal_daemons.h"

#include "set.h"
#include "pkt.h"
#include "daemon.h"

#include "swman_cmds.h"

void swman_run();
void swman_alloc();
int swman_init(daemon_o daemon);


/** SW MANAGER DAEMON MAIN FUNCTION
 *
 * Configures man_daemon_cfg structure and launches UTILS main function
 *
 * The following commands are accepted (constants are defined at swman_cmds.h)
 * and produce the following results:
 *								=== command produced ==
 *   Commands from CMD_MAN:
 * 	- SWMAN_LOADAPP: Command to load an applicatoin		--> swman_LOADOBJ/CMDMAN_SWLOADERR
 * 	- SWMAN_APPSTATUS: Modify the status of an app		--> EXEC_STATUS/CMDMAN_SWSTATUSERR
 *
 *   Commands from SENSOR(S):
 *	(from SWLOAD)
 *	- SWMAN_EXECREQ: Processor requests an executable	--> swman_EXECLOAD/swman_EXECERR/CMDMAN_SWLOADOK/ERR
 *	- SWMAN_EXECINFO: Processor requests exec. info		--> swman_EXECINFO/swman_EXECERR
 *	- SWMAN_EXECERR: Error while loading executable		--> CMDMAN_SWLOADERR
 *
 *	(from EXEC)
 *	- SWMAN_STATUSOK: Object notifies status change		--> CMDMAN_SWSTATUSOK
 *	- SWMAN_STATUSERR: Object not changed at time		--> CMDMAN_SWSTATUSERR
 *
 *   todo commands:
 *	- SWMAN_RTCFAULT: Exec reports failure in real-time	--> CMDMAN_RTCFAULT
 */
int swman_init(daemon_o d);

cmd_t swman_cmds[] = {{
	SWMAN_LOADAPP, swman_incmd_loadapp, "Load a waveform", "Error loading waveform"}, {
	SWMAN_APPSTATUS, swman_incmd_appstatus, "Change waveform status", "Error setting waveform status"}, {
	SWMAN_RTFAULT, swman_incmd_rtfault, "Object Real-Time Failure", "Error reading Real-Time Failure"}, {
	SWMAN_EXECREQ, swman_incmd_execreq, "Request executable", "Error requesting executable"}, {
	SWMAN_EXECINFO, swman_incmd_execinfo, "Request executable info", "Error requesting executable info"}, {
	SWMAN_EXECERR, swman_incmd_apperror, "Error with waveform", "Error loading waveform"}, {
	SWMAN_STATUSOK, swman_incmd_statusrep, "Object status report", "Error with status report"}, {
	SWMAN_STATUSERR, swman_incmd_statusrep, "Object status report error", "Error with status report"}, {
	SWMAN_APPINFO, swman_incmd_appinfo, "Get waveform information", "Error getting waveform information"}, {
	SWMAN_APPLS, swman_incmd_applist, "Get waveform list", "Error getting waveform list"}, {
	SWMAN_REPORTSTART, swman_incmd_appinfocmd, "Start reports", "Error starting report"}, {
	SWMAN_REPORTSTOP, swman_incmd_appinfocmd, "Stop reports", "Error stopping report"}, {
	SWMAN_LOGSSTART, swman_incmd_appinfocmd, "Start exec-logs", "Error starting exec-logs"}, {
	SWMAN_LOGSSTOP, swman_incmd_appinfocmd, "Stop exec-logs", "Error stopping exec-logs"}, {
	SWMAN_APPINFOREPORT, swman_incmd_appinforeport, "Exec information report", "Error with report information"}, {
	SWMAN_MAPAPPERROR, swman_incmd_apperror, "Error mapping waveform", "Error mapping waveform"}, { 0, NULL, NULL, NULL}};




#ifdef RUN_AS_PROC
int main()
{
	if (!swman_init_p())
		return 0;
	swman_run();
		return 1;
}
#endif

daemon_o swman_daemon;


int swman_init_p(void *ptr, int daemon_idx) {
	int n;

#ifdef RUN_AS_PROC
	if (!hwapi_init ()) {
#else
	if (!hwapi_init_noprocess(ptr, daemon_idx)) {
#endif
		printf("SWMAN: Error initiating api\n");
		return 0;
	}

	swman_alloc();

	swman_daemon = daemon_new(DAEMON_SWMAN, swman_cmds);
	if (!swman_daemon) {
		xprintf("SWMAN: Error creating swman_daemon\n");
		return 0;
	}

	swman_init(swman_daemon);

	daemon_info(swman_daemon, "Init ok");

	return 1;
}

int swman_process_cmd(int blocking) {
	int n;
	do {
		n = daemon_rcvpkt(swman_daemon, blocking);
		if (n > 0) {
			daemon_processcmd(swman_daemon);
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

void swman_delete() {
	daemon_delete(&swman_daemon);
}

/** stats swman_daemon Main Routine
 */
void swman_run() {

	while (1) {

#ifndef RUN_NONBLOCKING
		swman_process_cmd(1);
#else
		swman_process_cmd(0);
		hwapi_relinquish_daemon();
#endif
	}

	swman_delete(&swman_daemon);

	exit(-1);
}

