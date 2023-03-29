/*
 * hwapi_backd_mem.c
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
#include <semaphore.h>

/* include command declarations */
#include <sys/msg.h>

#include "mem.h"

#include "phal_hw_api.h"
#include "hwapi_backd.h"

/** Main HWAPI structure */
extern struct hw_api_db *hw_api_db;

extern int verbose;
extern int trace;


/** semafore union */
typedef union semun {
	int val;
	struct semid_ds *buf;
	ushort * array;
} semun_t;


/** remove any possible msg interface */
void purge_msg() {
	key_t key;
	int i;

	for (i = 0; i < 0x100; i++) {
		key = ftok(LOCK_FILE, i);
		if (msgget(key, 0) != -1) {
			msgctl(msgget(key, 0), IPC_RMID, NULL);
		}
	}
}

int clear_shm() {
	int i, j;
	key_t shm_key,api_msg_key,data_msg_key,semkey;
	int shm_id;
	int semid;

	shm_key = ftok(LOCK_FILE, HW_API_SHM_KEY);
	api_msg_key = ftok(LOCK_FILE, HW_API_MSG_KEY);
	data_msg_key = ftok(LOCK_FILE, DATA_MSG_KEY);


	/* if shared memory is created */
	if ((shm_id = shmget(shm_key, sizeof(struct hw_api_db), 0))!=-1) {
		if ((hw_api_db = (struct hw_api_db *) shmat(shm_id, NULL, 0))
				== (struct hw_api_db *) -1) {
			printf("Shared memory could not be attached\n");
			perror("shmat");
			return 0;
		}
		for (i = 0; i < MAX_INT_ITF; i++) {
			if (hw_api_db->int_itf[i].id) {
				if (msgctl(hw_api_db->int_itf[i].id, IPC_RMID, NULL) == -1) {
					printf("error removing msg itf id %d\n", hw_api_db->int_itf[i].id);
					perror("msgctl");
				}
			}
		}

		if (sem_destroy(&hw_api_db->sem_dac)) {
			perror("sem_init var");
		}

		if (sem_destroy(&hw_api_db->sem_var)) {
			perror("sem_init var");
		}

		if (sem_destroy(&hw_api_db->sem_status)) {
			perror("sem_init status");
		}
		for (i=0;i<MAX_DAEMONS;i++) {
			if (sem_destroy(&hw_api_db->sem_daemons[i])) {
				perror("sem_init timer");
			}
		}

		/** create object scheduling semaphores */
		for (j = 0; j < hw_api_db->cpu_info.nof_cores; j++) {
			for (i=0;i<MAX_PROCS;i++) {
				if (sem_destroy(&hw_api_db->sem_objects[j][i])) {
					perror("sem_init timer");
				}
			}
		}

		/* unattach pointer if exists */
		if (hw_api_db) {
			shmdt(hw_api_db);
		}

		/* and clear shared memory */
		shmctl(shmget(shm_key, sizeof(struct hw_api_db), 0), IPC_RMID, NULL);
	}


	/* same for msg for api */
	if (msgget(api_msg_key, 0) != -1) {
		if (msgctl(msgget(api_msg_key, 0), IPC_RMID, NULL)) {
			printf("error removing msg itf id %d\n", hw_api_db->int_itf[i].id);
			perror("msgctl");
		}
	}
	if (msgget(data_msg_key, 0) != -1) {
		if (msgctl(msgget(data_msg_key, 0), IPC_RMID, NULL)) {
			printf("error removing msg itf id %d\n", hw_api_db->int_itf[i].id);
			perror("msgctl");
		}
	}

	return 1;

}

void create_shm() {
	key_t shm_key,api_msg_key,data_msg_key;
	int shm_id;

	/*Create shared memory control area */
	shm_key = ftok(LOCK_FILE, HW_API_SHM_KEY);
	api_msg_key = ftok(LOCK_FILE, HW_API_MSG_KEY);
	data_msg_key = ftok(LOCK_FILE, DATA_MSG_KEY);

	printv("Creating main shared memory\n");
	if ((shm_id = shmget(shm_key, sizeof(struct hw_api_db), IPC_CREAT
			| IPC_EXCL | 0666)) == -1) {
		printf("HWAPI_BACKD: Shared memory could not be created\n");
		perror("shmget");
		exit(-1);
	}

	if ((hw_api_db = (struct hw_api_db *) shmat(shm_id, NULL, 0))
			== (struct hw_api_db *) -1) {
		printf("Shared memory could not be attached\n");
		perror("shmat");
		exit(-1);
	}

}

/** Init Shared Memory
 *
 * Initializes shared memory regions
 */
int init_shm_db() {
	int i, j,n;
	struct msqid_ds info;

	semun_t arg;
	int data_msg_id,api_msg_id,shm_id,semid;
	key_t shm_key,api_msg_key,data_msg_key,semkey;

	/*Create shared memory control area */
	shm_key = ftok(LOCK_FILE, HW_API_SHM_KEY);
	api_msg_key = ftok(LOCK_FILE, HW_API_MSG_KEY);
	data_msg_key = ftok(LOCK_FILE, DATA_MSG_KEY);


	printv("Creating mailbox\n");
	/* Create daemons' api communications mbox */
	if ((api_msg_id = msgget(api_msg_key, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
		printf("HWAPI_BACKD: Message queue for deamons control could not be created\n");
		perror("msgget");
		exit(-1);
	}

	/* Create daemons data communications mbox */
	if ((data_msg_id = msgget(data_msg_key, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
		printf("HWAPI_BACKD: Message queue for data could not be created\n");
		perror("msgget");
		exit(-1);
	}

	msgctl(data_msg_id, IPC_STAT, &info);
	info.msg_qbytes = MSG_QBYTES;
	if (msgctl(data_msg_id, IPC_SET, &info) == -1) {
		perror("msgctl");
		printf(
				"HWAPI_BACKD: Error setting size of message queue. Configure kernel to allow msg buffers of %d bytes.\n\n"
					"\t1. Set '/proc/sys/kernel/msgmnb' to '%d'. It will be reset on next system boot.\n"
					"\t2. Add line: 'kernel.msgmnb=%d' to '/etc/sysctl.conf' and run 'sysctl -p' as root\n",
				MSG_QBYTES, MSG_QBYTES, MSG_QBYTES);
		exit(-1);
	}
	struct msginfo info2;
	msgctl(data_msg_id, IPC_INFO, (struct msqid_ds*) &info2);
	if (info2.msgmni < MAX_INT_ITF) {
		printf(
				"HWAPI_BACKD: Maximum number of message queues must be bigger than %d.\n\n"
					"\t1. Set '/proc/sys/kernel/msgmni' to '%d'. It will be reset on next system boot.\n"
					"\t2. Add line: 'kernel.msgmni=%d' to '/etc/sysctl.conf' and run 'sysctl -p' as root\n",
				MAX_INT_ITF, MAX_INT_ITF, MAX_INT_ITF);
		exit(-1);
	}
	if (info2.msgmax < MSG_BUFF_SZ) {
		printf(
				"HWAPI_BACKD: Maximum size for messages must be bigger than %d.\n\n"
					"\t1. Set '/proc/sys/kernel/msgmax' to '%d'. It will be reset on next system boot.\n"
					"\t2. Add line: 'kernel.msgmax=%d' to '/etc/sysctl.conf' and run 'sysctl -p' as root\n",
				MSG_BUFF_SZ, MSG_BUFF_SZ, MSG_BUFF_SZ);
		exit(-1);
	}

	msgctl(data_msg_id, IPC_STAT, &info);

#ifdef DEB_MEM
	printf("HWAPI_BACKD: Msg queue for data comms created with id %d, size %d\n",
			data_msg_id, (int) info.msg_qbytes);
#endif


	hw_api_db->int_itf[0].key_id = HW_API_MSG_KEY;
	hw_api_db->int_itf[1].key_id = DATA_MSG_KEY;

	hw_api_db->data_msg_id=data_msg_id;
	hw_api_db->api_msg_id=api_msg_id;

	printv("Creating semaphores...\n");

	if (sem_init(&hw_api_db->sem_dac,1,1)) {
		perror("sem_init dac");
	}
	if (sem_init(&hw_api_db->sem_var,1,1)) {
		perror("sem_init var");
	}
	if (sem_init(&hw_api_db->sem_status,1,1)) {
		perror("sem_init status");
	}
	for (i=0;i<MAX_DAEMONS;i++) {
		if (sem_init(&hw_api_db->sem_daemons[i],1,1)) {
			perror("sem_init timer");
		}
	}

	/** create object scheduling semaphores */
	for (j = 0; j < hw_api_db->cpu_info.nof_cores; j++) {
		for (i=0;i<MAX_PROCS;i++) {
			if (sem_init(&hw_api_db->sem_objects[j][i],1,1)) {
				perror("sem_init timer");
			}
		}
	}

	printv("Initiating dynamic memory pool for stats...\n");
	/* create memory pool object for statistics variables */
	/* default slot size to 32 bytes */
	if (!mem_new(hw_api_db->var_table, VAR_TABLE_SZ, 32)) {
		printf("HWAPI_BACKD: Error creating shared memory!\n");
		exit(-1);
	}

	get_time(&hw_api_db->time.init_time);

#ifdef DISABLE_LOGS
	hw_api_db->cpu_info.disable_logs = 1;
#endif


	return 1;
}
