/*
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
#include "phal_daemons.h"
#include "dac_api.h"

//#include <phal_sw_api.h>
//#include "skeleton.h"


#define SYNC_CHECKRT

timer_t timer_id;
struct itimerspec itval;

/** Main HWAPI structure */
extern struct hw_api_db *hw_api_db;

extern int verbose;
extern int trace;

void post_objects() {
	int i;

	for (i=0;i<hw_api_db->cpu_info.nof_cores;i++) {
		sem_post(&hw_api_db->sem_objects[i][0]);
	}
}

void post_daemons() {
	int i;

	for (i=0;i<hw_api_db->nof_sem_daemons;i++) {
		sem_post(&hw_api_db->sem_daemons[i]);
	}
}

void run_periodic_tasks() {
	int i;
	union sigval val;

	for (i=0;i<MAX_PERIOD_TASKS;i++) {
		if (hw_api_db->period_tasks[i].fnc) {
			hw_api_db->period_tasks[i].cnt++;
			if (hw_api_db->period_tasks[i].cnt==hw_api_db->period_tasks[i].period) {
				val.sival_int=((i<<4)&0xfffffff0);
				val.sival_int|=PROCCMD_PERIODTSK & 0xf;
				sigqueue(hw_api_db->period_tasks[i].pid,SIG_PROC_CMD,val);
				hw_api_db->period_tasks[i].cnt=0;
			}
		}
	}
}

void check_realtime() {
	int i;
	int maxts;
	union sigval sival;

	/** First module, check that finished execution */
/*if(hw_api_db->time.tslot == 150000){
	printf("1500000: hw_api_db->proc_db[i].status.cur_status=%d = 3?\n", hw_api_db->proc_db[i].status.cur_status);	
	printf("hw_api_db->cpu_info.debug_level=%d\n", hw_api_db->cpu_info.debug_level);	
	printf("hw_api_db->proc_db[i].relinquished=%d\n", hw_api_db->proc_db[i].relinquished);
	printf("hw_api_db->time.tslot=%d\n", hw_api_db->time.tslot);
}
*/


	i=hw_api_db->max_proc_id-1;
	if (hw_api_db->proc_db[i].status.cur_status == PHAL_STATUS_RUN
		&& !hw_api_db->cpu_info.debug_level && !hw_api_db->proc_db[i].relinquished) {

/*		modinfo_msg("______________________________Tslot=%d",hw_api_db->time.tslot);
		
		printf("hw_api_db->proc_db[i].status.cur_status=%d = 3?\n", hw_api_db->proc_db[i].status.cur_status);	
		printf("!hw_api_db->cpu_info.debug_level=%d\n", !hw_api_db->cpu_info.debug_level);	
		printf("!hw_api_db->proc_db[i].relinquished=%d\n", !hw_api_db->proc_db[i].relinquished);	
*/
		printf("[%d]:\tRT-Fault first object still executing (obj ts: %d)\n",hw_api_db->time.tslot,hw_api_db->proc_db[i].cur_tstamp);
	}

	/* Rest of the modules, check that run every timeslot */
	if (!hw_api_db->cpu_info.debug_level) {
		for (i=0;i<hw_api_db->max_proc_id;i++) {
			maxts=i?2:1;
			if (		hw_api_db->proc_db[i].status.cur_status == PHAL_STATUS_RUN
					&& 	hw_api_db->proc_db[i].status.next_status== PHAL_STATUS_RUN
					&& (hw_api_db->proc_db[i].cur_tstamp<hw_api_db->time.tslot-maxts)
					&& !hw_api_db->proc_db[i].rt_fault) {
				if (hw_api_db->cpu_info.print_rtfaults) {
					hw_api_db->proc_db[i].rt_fault=1;
					//printf("[%d]:\tRT-Fault object %s pid %d (OBJ TS: %d)\n",hw_api_db->time.tslot,hw_api_db->proc_db[i].obj_name,hw_api_db->proc_db[i].pid,hw_api_db->proc_db[i].cur_tstamp);
					sival.sival_int=((i<<4)&0xfffffff0);
					sival.sival_int|=PROCCMD_RTFAULT & 0xf;
					//set_realtime(hw_api_db->exec_pid, hw_api_db->kernel_prio+1, -1);
					sigqueue(hw_api_db->exec_pid,SIG_PROC_CMD,sival);
					//set_realtime(hw_api_db->exec_pid, hw_api_db->exec_prio, -1);
					break;
				} else {
					printf("[%d]:\tRT-Fault object %s (OBJ TS: %d)\n",hw_api_db->time.tslot,hw_api_db->proc_db[i].obj_name,hw_api_db->proc_db[i].cur_tstamp);
				}
				break;
			}
		}
	}
}

time_t kernel_time;
#define US_IN_S 1000000

int last_offset=0;

void syncTslot ()
{
	int ret;

#ifdef SYNC_CHECKRT
	check_realtime();
#endif

	if (hw_api_db->time.new_tstamp && !hw_api_db->time.next_ts) {
		if (last_offset) {
			hw_api_db->time.tslot=hw_api_db->time.new_tstamp+2;
		} else {
			hw_api_db->time.tslot=hw_api_db->time.new_tstamp+1;
		}

		tstamp_to_time(hw_api_db->time.tslot,&hw_api_db->time.slotinit);
		//printf("now set timeslot to %d\n",hw_api_db->time.tslot);
		hw_api_db->time.new_tstamp=0;
	} else {
		hw_api_db->time.tslot++;
//		if(hw_api_db->time.tslot > 150000)printf("hwapi_backd_exec.c: hw_api_db->time.tslot++=%u\n", hw_api_db->time.tslot++);

		hw_api_db->time.slotinit.tv_usec+=hw_api_db->cpu_info.tslen_usec;
		while(hw_api_db->time.slotinit.tv_usec>=1000000) {
		  hw_api_db->time.slotinit.tv_sec++;
		  hw_api_db->time.slotinit.tv_usec-=1000000;
		}
	}

	set_time(&hw_api_db->time.slotinit);

	if (hw_api_db->time.next_ts) {
		if (hw_api_db->time.next_ts<0) {
			printf("is negative, was %d\n",hw_api_db->time.next_ts);
			hw_api_db->time.next_ts=hw_api_db->cpu_info.tslen_usec+hw_api_db->time.next_ts;
			last_offset=1;
		}
		itval.it_value.tv_sec = 0;
		itval.it_value.tv_nsec = hw_api_db->time.next_ts * 1000;
		ret = timer_settime (timer_id, 0, &itval, NULL);
		if (ret==-1) {
			perror("timer_settime A");
		}
		//printf("now adjusting next ts to %d\n",hw_api_db->time.next_ts);
		hw_api_db->time.next_ts=0;
	}

	if (hw_api_db->dac_itf && !hw_api_db->dac_drives_clock) {
		sem_post(&hw_api_db->sem_dac);
	}

	post_objects();
	post_daemons();

	run_periodic_tasks();
}


int run_main_timer() {
	time_t tdata;
	int sleep_time,actual_time;
	int tstamp;
	static int next_sig;
	int ret;
	unsigned int ns;
	unsigned int sec;
	struct sigevent sigev;
	sigset_t sigset;
	int sig;
	int i;
	int first;
	struct itimerspec timerSpec;
	struct itimerspec oldSpec;
	struct timespec timeout;
	uint64_t exp;
	int err,j;
	int n;

	memset(&timerSpec, 0, sizeof(timerSpec));

	if ((hw_api_db->dac_itf && hw_api_db->dac_drives_clock)
			|| hw_api_db->sync_drives_clock) {
		if (hw_api_db->sync_drives_clock) {
			printf("SYNC drives kernel clock\n");
		} else {
			printf("Dac drives kernel clock\n");
		}
		while(1) {
			sem_wait(&hw_api_db->sem_dac);
			syncTslot();
		}
	} else {

		sig = SIG_KERNEL_SCHED;

		sigemptyset (&sigset);
		sigaddset (&sigset, sig);
		sigprocmask (SIG_BLOCK, &sigset, NULL);

		/* Create a timer that will generate the signal we have chosen */
		sigev.sigev_notify = SIGEV_SIGNAL;
		sigev.sigev_signo = sig;
		sigev.sigev_value.sival_ptr = (void *) &timer_id;
		ret = timer_create (CLOCK_MONOTONIC, &sigev, &timer_id);
		if (ret == -1)  {
			printf("Error creating timer");
			perror("timer_create");
			return -1;
		}

		printva("Setting timer to %d usec\n",hw_api_db->cpu_info.tslen_usec);

		/* Make the timer periodic */
		sec = 0;
		ns = hw_api_db->cpu_info.tslen_usec * 1000;
		itval.it_interval.tv_sec = sec;
		itval.it_interval.tv_nsec = ns;
		itval.it_value.tv_sec = sec;
		itval.it_value.tv_nsec = ns;
		ret = timer_settime (timer_id, 0, &itval, NULL);
		if (ret==-1) {
			perror("timer_settime B");
		}

#if SCHEDULING!=SCHED_SEM
		/* save the signal to children */
		sigchilds=SIG_PROC_SCHED;
#endif

		printv("timer armed.\n");
		memset(&hw_api_db->time,0,sizeof(struct phtime));

		timeout.tv_sec=1;
		timeout.tv_nsec=0;
		while(1) {
			//sigwait (&sigset, &sig);
			if ((n=sigtimedwait(&sigset,NULL,&timeout))==-1) {
				if (errno==EAGAIN) {
					perror("sigtimedout");
					exit(0);
				}
			}
			syncTslot();
			if ((n=timer_getoverrun(timer_id))>1) {
				printva("overrrun %d\n",n);
			}
		}

#ifdef testing
		fork();
		j=0;
		while(j<10) {
			do {
				err=read(hw_api_db->timerfd, &exp, sizeof(uint64_t));
			} while(err=-1 && errno==EINTR);
			if (err==-1 && errno!=EINTR) {
				perror("read");
			}
			gettimeofday(&tdata,NULL);
			printf("%d-%d---read at %d:%d\n",getpid(),(int) exp,tdata.tv_sec,tdata.tv_usec);
		.	j++;
		}
#endif
	}
	return 1;
}
