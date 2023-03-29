/*
 * bridge.c
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

#include "pkt.h"
#include "set.h"
#include "daemon.h"

#include "bridge.h"

#include "bridge_cmds.h"

void bridge_run();
void bridge_alloc();
int bridge_init(daemon_o daemon);
int bridge_background();
int _bridge_process_cmd(int fd, int arg);

#ifdef RUN_AS_PROC
int main()
{
	if (!bridge_init_p())
		return 0;

	bridge_run();
	return 1;
}
#endif

static cmd_t bridge_cmds[] = { { BRIDGE_IDENT, bridge_incmd_ident,
		"Ident neighbours", "Error identifying neighbours" }, { BRIDGE_RMITF,
		bridge_incmd_rmitf, "Remove interface", "Error removing interface" }, {
		BRIDGE_ADDITF, bridge_incmd_additf, "Add interface",
		"Error adding interface" }, { 0, NULL, NULL, NULL } };

daemon_o bridge_daemon;

int bridge_init_p(void *ptr, int daemon_idx) {

	int n;

	if (!hwapi_init ()) {
		printf("BRIDGE: Error initiating api\n");
		return 0;
	}

	bridge_alloc();

	bridge_daemon = daemon_new(DAEMON_BRIDGE, bridge_cmds);
	if (!bridge_daemon) {
		xprintf("BRIDGE: Error creating daemon\n");
		return 0;
	}

	n = bridge_init(bridge_daemon);
	if (n < 0) {
		xprintf("BRIDGE: Error initiating external itf\n");
		return 0;
	}

	daemon_info(bridge_daemon, "Init ok");

	return 1;

}

int _bridge_process_cmd(int fd, int arg) {
	bridge_process_cmd(0);
}

int bridge_process_cmd(int blocking) {
	int n;
	do {
		n = daemon_rcvpkt(bridge_daemon,blocking);
		if (n > 0) {
			daemon_processcmd(bridge_daemon);
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

void bridge_delete() {
	daemon_delete(&bridge_daemon);
}

void bridge_run() {

#ifndef RUN_NONBLOCKING
	hwapi_itf_addcallback(daemon_cmdfd(bridge_daemon),_bridge_process_cmd,0);
#endif

	while (1) {

#ifndef RUN_NONBLOCKING
		hwapi_idle();
#else
		bridge_process_cmd(0);
		bridge_background();
		hwapi_relinquish_daemon();
#endif

	}

	bridge_delete();
}
