/*
 * exec.c
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

#include "phal_daemons.h"
#include "phid.h"

#include "execx.h"

#include "pkt.h"
#include "set.h"
#include "daemon.h"
#include "swman.h"

/* include command declarations */
#include "exec_cmds.h"

/* main function declaration */
void exec_run();
void exec_alloc();
int exec_init(daemon_o exec_daemon);
int exec_background();

#ifdef RUN_AS_PROC
int main()
{
	if (!exec_init_p())
	return 0;
	exec_run();
	return 1;
}
#endif

static cmd_t exec_cmds[] = { { EXEC_SETSTATUS, exec_incmd_setstatus,
		"Set new object status", "Error setting object status" }, {
		EXEC_REPORTSTART, exec_incmd_reportcmd, "Start reports",
		"Error starting reports" }, { EXEC_REPORTSTOP, exec_incmd_reportcmd,
		"Stop reports", "Error stopping reports" }, { 0, NULL, NULL, NULL } };

daemon_o exec_daemon;

int exec_init_p(void *ptr, int daemon_idx) {
	int n;

#ifdef RUN_AS_PROC
	if (!hwapi_init ()) {
#else
	if (!hwapi_init_noprocess(ptr,daemon_idx)) {
#endif
		printf("EXEC: Error initiating api\n");
		return 0;
	}

	hwapi_schedtimer();

	exec_alloc();

	exec_daemon = daemon_new(DAEMON_EXEC, exec_cmds);
	if (!exec_daemon) {
		xprintf("EXEC: Error creating exec_daemon\n");
		return 0;
	}

	exec_init(exec_daemon);

	daemon_info(exec_daemon,"Init ok");

	return 1;
}

int exec_process_cmd(int blocking) {
	int n;
	do {
		n = daemon_rcvpkt(exec_daemon, blocking);
		if (n > 0) {
			daemon_processcmd(exec_daemon);
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

void exec_delete() {
	daemon_delete(&exec_daemon);
}
/** EXEC exec_daemon Main Routine
 */
void exec_run() {

	while (1) {

		exec_process_cmd(0);

		exec_background();

		hwapi_relinquish_daemon();
	}

	exec_delete(&exec_daemon);

	exit(-1);
}

