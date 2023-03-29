/*
 * statsman.c
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

#include "statsman_cmds.h"

/* main function declaration */
void statsman_run();
void statsman_alloc();
int statsman_init(daemon_o daemon);

cmd_t statsman_cmds[] = {{
	STATSMAN_STATLS, statsman_incmd_statls, "List statistic", "Error listing statistic"}, {
	STATSMAN_STINIT, statsman_incmd_init, "Initialize statistic", "Error initializing statistic"}, {
	STATSMAN_STINITOBJECT, statsman_incmd_init, "Initialize object statistic", "Error initializing object statistic"}, {
	STATSMAN_PMINIT, statsman_incmd_init, "Initialize parameter", "Error initializing parameter"}, {
	STATSMAN_LOGINIT, statsman_incmd_init, "Initialize log", "Error initializing log"}, {
	STATSMAN_STCLOSE, statsman_incmd_close, "Close statistic", "Error closing statistic"}, {
	STATSMAN_PMCLOSE, statsman_incmd_close, "Close parameter", "Error closing parameter"}, {
	STATSMAN_LOGCLOSE, statsman_incmd_close, "Close log", "Error closing log"}, {
	STATSMAN_STGET, statsman_incmd_stact, "Get statistic value", "Error getting stat value"}, {
	STATSMAN_STSET, statsman_incmd_stact, "Set statistic value", "Error setting stat value"}, {
	STATSMAN_STREPORT, statsman_incmd_stact, "Report statistic value", "Error reporting statistic"}, {
	STATSMAN_STVALOK, statsman_incmd_stactack, "Get statistic ok", "Error getting stat value"}, {
	STATSMAN_STVALERR, statsman_incmd_stactack, "Get statistic error", "Error getting stat value"}, {
	STATSMAN_STREPORTOK, statsman_incmd_stactack, "Report statistic ok", "Error reporting stat value"}, {
	STATSMAN_STREPORTERR, statsman_incmd_stactack, "Report statistic error", "Error reporting stat value"}, {
	STATSMAN_STREPORTVALUE, statsman_incmd_streportvalue, "Report statistic value", "Error reporting stat value"}, {
	STATSMAN_STSETOK, statsman_incmd_stactack, "Set statistic ok", "Error setting stat value"}, {
	STATSMAN_STSETERR, statsman_incmd_stactack, "Set statistic error", "Error setting stat value"}, {
	STATSMAN_PMGET, statsman_incmd_pmget, "Parameter value request", "Error getting parameter"}, {
	STATSMAN_LOGWRITE, statsman_incmd_logwrite, "Write log", "Error writting log"}, { 0, NULL, NULL, NULL}};


#ifdef RUN_AS_PROC
int main()
{
	if (!statsman_init_p())
		return 0;
	statsman_run();
		return 1;
}
#endif

daemon_o statsman_daemon;


int statsman_init_p(void *ptr, int daemon_idx) {
	int n;

#ifdef RUN_AS_PROC
	if (!hwapi_init ()) {
#else
	if (!hwapi_init_noprocess(ptr, daemon_idx)) {
#endif
		printf("STATSMAN: Error initiating api\n");
		return 0;
	}

	statsman_alloc();

	statsman_daemon = daemon_new(DAEMON_STATSMAN, statsman_cmds);
	if (!statsman_daemon) {
		xprintf("STATSMAN: Error creating statsman_daemon\n");
		return 0;
	}

	statsman_init(statsman_daemon);

	daemon_info(statsman_daemon,"Init ok");

	return 1;
}

int statsman_process_cmd(int blocking) {
	int n;
	do {
		n = daemon_rcvpkt(statsman_daemon, blocking);
		if (n > 0) {
			daemon_processcmd(statsman_daemon);
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


void statsman_delete() {
	daemon_delete(&statsman_daemon);
}

/** stats statsman_daemon Main Routine
 */
void statsman_run() {

	while (1) {

#ifndef RUN_NONBLOCKING
		statsman_process_cmd(1);
#else
		statsman_process_cmd(0);
		hwapi_relinquish_daemon();
#endif
	}

	statsman_delete(&statsman_daemon);

	exit(-1);
}

