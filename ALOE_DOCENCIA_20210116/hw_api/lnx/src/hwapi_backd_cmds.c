/*
 * hwapi_backd_cmds.c
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


/** Main HWAPI structure */
extern struct hw_api_db *hw_api_db;

/** Buffer for input messages */
struct msgbuff msg;

extern int verbose;
extern int trace;

extern pid_t command_pid;
void check_commands_loop();


int kernel_commands_thread() {
	cpu_set_t set;
	sigset_t sigset;
	struct sched_param param;

	command_pid=fork();
	switch(command_pid) {
	case -1:
		printf("Error creating command processing process\n");
		return -1;
	case 0:

		sigemptyset(&sigset);
		sigprocmask(SIG_SETMASK,&sigset,NULL);
		sigaddset(&sigset,SIG_PROC_CMD);
		sigprocmask(SIG_BLOCK,&sigset,NULL);

		set_realtime(getpid(),hw_api_db->kernel_cmd_prio,hw_api_db->kernel_cmd_cpuid);

		check_commands_loop();
		exit(0);
	}

	return 1;
}

void check_commands_loop() {

	while(check_commands()!=-1);
		printf("HWAPI_BACKD: Check commands loop failed\n");

	exit(0);
}
/* This function is called by the sigusr1 interruption handler.
 * It receives the message through the api_msg_id msgbox, processes it and sends answer back.
 */
int check_commands() {
	int n;
	int ret;
	int api_msg_id;
	int errno;

	api_msg_id=hw_api_db->api_msg_id;

	n = msgrcv(api_msg_id, &msg, MSG_SZ, MSG_MAGIC, 0);
	if (n > 0) {
		ret = Answer_command(msg.body[0], &msg.body[1], n - MSG_HEAD_SZ - 2);
		/* answer with return value */
		msg.head.dst = msg.head.src;
		msg.body[0] = ret;
		if (-1==msgsnd(api_msg_id, &msg, n, 0)) {
			return -1;
		}
	} else if (n == -1) {
		if (errno != EINTR) {
			perror("Receiving message from child");
			exit(-1);
		} else {

			return 1;
		}
	}
	return n;
}

/** Command Processing.
 *
 * Accepts the following commands with the following contents. First word is the
 * command type, following is data needed by this command:
 *	- HWD_SETUP_XITF: Starts an external interface (configured at interface configuration file).
 *		- word 1: Id of the external interface.
 *		- word 2: Mailbox of the internal interface to be bridged.
 *		- word 3: Mode (READ_ONLY/WRITE_ONLY).
 *	- CREATE_PROCESS: Creates a process.
 *		- word 0: Position in the process database of the process to be
 *			created.
 */
char Answer_command(int cmd, char *packet, int len) {
	int i, id;
	int pid;
	key_t itf_key;
	struct sched_param param;
	struct setup_dac_cmd *dac_cmd;
	struct dac_interface *dac_itf;
	struct net_db *net_db=&hw_api_db->net_db;

	switch (cmd) {
	case HWD_SETUP_XITF:

		i = 0;
		while (i < net_db->nof_ext_itf && net_db->ext_itf[i].id
				!= (hwitf_t) packet[0]) {
			i++;
		}

		if (i == net_db->nof_ext_itf) {
			printf("HWAPI_BACKD: Itf id %d does not exist.\n", packet[0]);
			return -1;
		}

		itf_key = ftok(LOCK_FILE, (int) packet[1]);
		if ((id = msgget(itf_key, 0)) == -1) {
			xprintf("HWAPI_BACKD: Error attaching to itf %d\n", packet[1]);
			perror("msgget");
			return -1;
		}

		if (packet[2] == FLOW_READ_ONLY) {
			net_db->ext_itf[i].in_mbox_id = id;
			net_db->ext_itf[i].in_key_id = (hwitf_t) packet[1];
			net_db->ext_itf[i].int_itf_idx=(int) packet[3];
		} else if (packet[2] == FLOW_WRITE_ONLY) {
			net_db->ext_itf[i].out_mbox_id = id;
			net_db->ext_itf[i].out_key_id = (hwitf_t) packet[1];
		} else {
			printf("HWAPI_BACKD: Invalid mode 0x%x while setting itf\n",
					packet[2]);
			return -1;
		}

		int j = 0;
		while (j < MAX_INT_ITF && hw_api_db->int_itf[j].key_id
				!= (hwitf_t) packet[1])
			j++;
		if (j < MAX_INT_ITF) {
			hw_api_db->int_itf[j].network_pid = net_db->ext_itf[i].pid;
			hw_api_db->int_itf[j].network_bpid = net_db->ext_itf[i].bpid;
		}
		return 1;
		break;

	case HWD_CREATE_PROCESS:

		/* process index is received in the packet */
		i = packet[0];

		if ((pid=create_process(i))==-1) {
			printf("HWAPI_BACKD: Error creating process\n");
			return -1;
		}
		memcpy(packet, &pid, sizeof(int));
		return 1;
		break;
	default:
		printf("HWAPI_BACKD: Error unknown command 0x%x\n", cmd);
		break;
	}

	printf(	"HWAPI_BACKD: Error answering command... Not suposed to arrive here.\n");

	return -1;
}
