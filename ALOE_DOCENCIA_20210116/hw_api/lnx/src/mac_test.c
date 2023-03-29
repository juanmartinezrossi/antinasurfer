/*
 * mac_test.c
  *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>
 *
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

/* include command declarations */
#include <sys/msg.h>

#include "mem.h"

#include "phal_hw_api.h"
#include "hwapi_backd.h"
int sems[100];

typedef union semun {
	int val;
	struct semid_ds *buf;
	ushort * array;
} semun_t;

void run_mactest(int procs, int nops) {

	int i, j, x, n;
	struct sched_param param;
	time_t tdata[3], tdata2[3], t;
	semun_t arg;

	for (i = 1; i < procs; i++) {
		sems[i] = semget(IPC_PRIVATE, 1, O_CREAT | 0666);
		if (sems[i] == -1) {
			perror("Error creating semaphore");
			exit(-1);
		}

		arg.val = 0;
		if (semctl(sems[i], 0, SETVAL, arg) < 0) {
			perror("semctl failure");
			return;
		}
	}

	if (getuid()) {
		printf("This test needs root privileges\n");
		exit(0);
	}
	mlockall(MCL_CURRENT | MCL_FUTURE);
	param.__sched_priority = SCHED_PRIO;
	if (sched_setscheduler(0, SCHED_FIFO, &param)) {
		printf("HWAPI_BACKD: Error setting scheduler\n");
		perror("setscheduler");
	}

	printf("Running Integer MAC test for %d procs %d ops\n", procs, nops);

	gettimeofday(&tdata2[1], NULL);
	for (i = 0; i < procs; i++) {
		n = fork();
		if (n == -1) {
			printf("Error forking.\n");
			perror("fork");
			return;
		}
		if (!n) {
			gettimeofday(&t, NULL);
			sleep_us(1000000 - t.tv_usec);
			x = 1;
			gettimeofday(&tdata[1], NULL);
			for (j = 0; j < nops; j++) {
				x += 33 * x;
			}
			gettimeofday(&tdata[2], NULL);
			get_time_interval(tdata);
			printf("child %d: %d:%d\t-> %d:%d\tdiff: %d:%d\t%d MMACS\n", i,
					tdata[1].tv_sec, tdata[1].tv_usec, tdata[2].tv_sec,
					tdata[2].tv_usec, tdata[0].tv_sec, tdata[0].tv_usec, nops
							/ tdata[0].tv_usec);
			exit(0);
		}
	}

	while (wait(&x) > 0)
		;
	gettimeofday(&tdata2[2], NULL);
	get_time_interval(tdata2);
	printf("parent: %d:%d->%d:%d diff: %d:%d\n", tdata2[1].tv_sec,
			tdata2[1].tv_usec, tdata2[2].tv_sec, tdata2[2].tv_usec,
			tdata2[0].tv_sec, tdata2[0].tv_usec);

}
