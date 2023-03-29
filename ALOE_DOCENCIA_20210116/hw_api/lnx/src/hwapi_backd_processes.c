/*
 * hwapi_backd_daemons.c
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
#define _POSIX_C_SOURCE  200809L
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
#include <sys/msg.h>


/** HWAPI includes */
#include "phal_hw_api.h"
#include "hwapi_backd.h"
#include "hwapi_netd.h"
#include "params.h"

#include "set.h"
#include "pkt.h"
#include "daemon.h"

extern int valgrind;

/** Main HWAPI structure */
extern struct hw_api_db *hw_api_db;

extern int verbose;
extern int trace;

extern int old_pgid;

extern char valgrind_tool[SYSTEM_STR_MAX_LEN];

extern struct sembuf op1[MAX_DAEMONS];


/** Sets Real-time scheduling for proces id pid */
int set_realtime(int pid, int prio, int p_id) {
	int n;
	struct sched_param param;
	cpu_set_t set;

	if (pid==getpid() || !pid)
		mlockall(MCL_CURRENT | MCL_FUTURE);

	if (p_id>-1) {
		/* set processor affinity */
		CPU_ZERO(&set);
		CPU_SET(p_id, &set);
		if (sched_setaffinity(pid, sizeof(cpu_set_t), &set)) {
			perror("sched_setaffinity");
			return 0;
		}
	}
	if (pid) {
		if (prio>hw_api_db->kernel_prio && pid!=hw_api_db->exec_pid) {
			printf("HWAPI_BACKD: WARNING! Trying to set a priority higher than kernel priority for process %d. Increase kernel priority (%d)\n",pid,hw_api_db->kernel_prio);
			prio=hw_api_db->kernel_prio-1;
			printf("HWAPI_BACKD: Decreasing priority to %d\n",prio);
		}
	}
	param.__sched_priority = prio;
	if (sched_setscheduler(pid, SCHED_FIFO, &param)) {
		printf("HWAPI_BACKD: Error setting scheduler\n");
		perror("setscheduler");
		return 0;
	}
	return 1;
}

int create_process(int i) {
	int pid;
	int j;
	int do_valgrind=0;
	char *exe_name;
	char *x;
	int core_id;
	char *valgrind_arg;

	if (!strstr(valgrind_tool,"=")) {
		valgrind_arg=strdup(valgrind_tool);
		sprintf(valgrind_tool,"%s=%s",VALGRIND_ARG,valgrind_arg);
		free(valgrind_arg);
	}
	valgrind_arg=valgrind_tool;
	exe_name=rindex(hw_api_db->proc_db[i].path,'/');
	if (!exe_name) {
		exe_name = hw_api_db->proc_db[i].path;
	} else {
		exe_name++;
	}
	x=strdup(hw_api_db->valgrind_objects);
	if (search_in_csv(x,exe_name)>-1) {
		do_valgrind=1;
	}
	free(x);
	/* create child */
	switch (pid = fork()) {
	case -1:
		perror("fork");
		return -1;
	case 0:
		/* set pid, this indicates a successfull creation of the child */
		hw_api_db->proc_db[i].pid = getpid();

		printva("Object with pid %d created successfully.\n",getpid());
		/** Now launch executable */
		if (do_valgrind && valgrind) {
			execlp(VALGRIND_CMD,VALGRIND_CMD,valgrind_tool,hw_api_db->proc_db[i].path, NULL);
		} else {
			execl(hw_api_db->proc_db[i].path, hw_api_db->proc_db[i].path, NULL);
		}
		perror("execl");
		printf("HWAPI_BACKD: Error while launching process %s\n", hw_api_db->proc_db[i].path);
		exit(-1);
	default:

		hw_api_db->proc_db[i].prio = hw_api_db->obj_prio;
		hw_api_db->proc_db[i].sem_idx = hw_api_db->proc_db[i].exec_position;
		core_id = hw_api_db->core_mapping[hw_api_db->proc_db[i].core_idx];

		if (hw_api_db->proc_db[i].sem_idx>hw_api_db->last_semidx[hw_api_db->proc_db[i].core_idx]) {
			hw_api_db->last_semidx[hw_api_db->proc_db[i].core_idx]=hw_api_db->proc_db[i].sem_idx;
		}

		set_realtime(pid, hw_api_db->proc_db[i].prio, core_id);

		/* parent returns child pid */
		return pid;
	}
}

void remove_process(int i) {
	int j;
	int sem_i;
	/* move all schedule */
	sem_i=hw_api_db->proc_db[i].sem_idx+1;
	while(sem_i<=hw_api_db->last_semidx[hw_api_db->proc_db[i].core_idx]) {
		for (j = 0; j < MAX_PROCS; j++) {
			if (hw_api_db->proc_db[j].core_idx == hw_api_db->proc_db[i].core_idx
				&& hw_api_db->proc_db[j].sem_idx == sem_i) {

				hw_api_db->proc_db[j].sem_idx--;
				sem_post(&hw_api_db->sem_objects[hw_api_db->proc_db[i].core_idx][sem_i]);
			}
		}
		sem_i++;
	}
	hw_api_db->last_semidx[hw_api_db->proc_db[i].core_idx]--;
}

/** Start Daemons
 *
 * Starts all daemons (parsed from the daemon configuration file and saved in daemons structure)
 * by forking and exec()
 */
void start_daemons() {
	int i,j;
	int pid;
	struct sched_param param;
	cpu_set_t set;
	sigset_t sigset;
	char *x;
	time_t td;
	uint64_t r;
	struct itimerspec timerSpec;
	struct itimerspec oldSpec;

	char *valgrind_arg;

	/** prepare valgrind arguments */
	valgrind_arg=strdup(valgrind_tool);
	sprintf(valgrind_tool,"%s=%s",VALGRIND_ARG,valgrind_arg);
	free(valgrind_arg);
	valgrind_arg=valgrind_tool;

	j=0;
	for (i = 0; i < MAX_DAEMONS; i++) {

		if (hw_api_db->daemons[i].runnable) {
			j++;

			hw_api_db->nof_daemons=j;
			if (hwapi_hwinfo_isDebugging()) {
				x = strdup(hw_api_db->daemons[i].path);
				if (strstr(hw_api_db->daemons[i].path, "cmdman")) {
					sprintf(hw_api_db->daemons[i].path,
							"cmdman_console/bin/%s", x);
				} else {
					sprintf(hw_api_db->daemons[i].path,
							"sw_daemons/lnx_make/bin/%s", x);
				}
				free(x);
			}

			/* create child */
			switch (pid = fork()) {
			case -1:
				perror("fork");
				exit(-1);
			case 0:

				set_realtime(getpid(), hw_api_db->daemons[i].prio, hw_api_db->daemons[i].cpuid);
				if (strstr(hw_api_db->daemons[i].path, "exec")) {
					hw_api_db->exec_pid = getpid();
					hw_api_db->exec_prio = hw_api_db->daemons[i].prio;
				}

				/* save pid */
				hw_api_db->daemons[i].pid = getpid();

				printva("Created daemon pid: %d\n",getpid());

				/* launch daemon executable */
				if (hw_api_db->daemons[i].valgrind && valgrind) {
					execlp(VALGRIND_CMD,VALGRIND_CMD,valgrind_arg,hw_api_db->daemons[i].path, NULL);
				} else {
					if (strstr(hw_api_db->daemons[i].path,"cmdman") && hw_api_db->run_as_daemon) {
						execlp(hw_api_db->daemons[i].path, hw_api_db->daemons[i].path, "-l","-d", NULL);
					} else {
						execlp(hw_api_db->daemons[i].path, hw_api_db->daemons[i].path, NULL);
					}
				}

				printf("\nHWAPI_BACKD: Error launching %s\n\n", hw_api_db->daemons[i].path);
				perror("execlp");
				hw_api_db->daemons[i].pid = 0;
				exit(-1);
			default:
				break;
			}
		}
	}


	printv("Done\n");
}


int kill_child(int *pid, int sleep_time, char *name) {
	int c;

	if (kill(*pid, SIGTERM))
		perror("kill");
	c = 0;
	if (!sleep_time)
		return 0;
	while (*pid > 0 && !waitpid(*pid, NULL, WNOHANG)) {
		sleep_ms(sleep_time);
		c++;
		if (c == 10) {
			printf("HWAPI_BACKD: Process '%s' with pid %d not responding. Sending kill...\n", name, *pid);
			kill(*pid, SIGKILL);
			return 1;
		}
	}
	return 0;
}

/** Close Daemons
 *
 * Terminates all daemons and proccesses by sending a SIGTERM
 */
void kill_chlds(int wait_time) {
	int i;
	int sleep_time;
	int error = 0;

	sleep_time = wait_time / 10;

	/* kill all application modules... */
	for (i = 0; i < MAX_PROCS; i++) {
		if (hw_api_db->proc_db[i].pid > 0) {
			error += kill_child(&hw_api_db->proc_db[i].pid, sleep_time, hw_api_db->proc_db[i].obj_name);
		}
	}
	/* ... and daemons ... */
	for (i = 0; i < MAX_DAEMONS; i++) {
		if (hw_api_db->daemons[i].pid > 0) {
			error += kill_child(&hw_api_db->daemons[i].pid, sleep_time, hw_api_db->daemons[i].path);
		}
	}

	/* ... and network childs */
	if (hw_api_db->net_db.net_chld > 0) {
		error += kill_child(&hw_api_db->net_db.net_chld, sleep_time, "Network Main");
		for (i = 0; i < hw_api_db->net_db.nof_ext_itf; i++) {
			if (hw_api_db->net_db.ext_itf[i].pid > 0) {
				error += kill_child(&hw_api_db->net_db.ext_itf[i].pid, sleep_time, "Interface");
			}
			if (hw_api_db->net_db.ext_itf[i].bpid > 0) {
				error += kill_child(&hw_api_db->net_db.ext_itf[i].bpid, sleep_time, "Interface");
			}
		}
	}

	if (error) {
		printf("HWAPI_BACKD: Some processes were not closed properly (%d).\n", error);
	}

}


void remove_file(int i) {
	int j;
	/* check that any other process is using the same executable */
	for (j = 0; j < MAX_PROCS; j++) {
		if (j != i) {
			if (!strncmp(hw_api_db->proc_db[i].path,
					hw_api_db->proc_db[j].path, SYSTEM_STR_MAX_LEN)) {
				break;
			}
		}
	}
	if (j == MAX_PROCS) {
		if (remove(hw_api_db->proc_db[i].path)) {
			perror("remove");
		}

	}
	hw_api_db->proc_db[i].path[0] = '\0';
}

void clear_files() {
	int i;
	for (i = 0; i < MAX_PROCS; i++) {
		if (hw_api_db->proc_db[i].path[0] != '\0') {
			remove_file(i);
		}
	}
}

/** Clear all zombies (my childs)
 */
void clear_zombies() {
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
	}

void run_daemon(char *output) {
	int null_fd, out_fd,err_fd;
	int status;
	int sid;


	if (fork()) {
		exit(0);
	}

	 /* Change the file mode mask */
	umask(0);

	/* Open any logs here */

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		/* Log the failure */
		exit(EXIT_FAILURE);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	open("/dev/null",0);
	if (output) {
		out_fd=open(output, O_CREAT | O_TRUNC | O_WRONLY, 0666);
		if (out_fd==-1) {
			perror("open");
			exit(0);
		}
	} else {
		open("/dev/null",0);
	}

}



