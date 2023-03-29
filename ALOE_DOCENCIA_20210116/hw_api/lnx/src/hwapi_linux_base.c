/*
 * hwapi_linux.c
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>,
 *                    Xavier Reves, UPC <xavier.reves at tsc.upc.edu>
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
/**
 * @file phal_hw_api.c
 * @brief ALOE Hardware Library LINUX Implementation
 *
 * This file contains the implementation of the HW Library for
 * linux platforms.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <setjmp.h>
#include <sys/select.h>
#include <semaphore.h>

#define __USE_GNU
#include <ucontext.h>
#include <execinfo.h>

#include "mem.h"
#include "phal_hw_api.h"
#include "hwapi_backd.h"

/** hw_api general database */
struct hw_api_db *hw_api_db;

/** pointer to proc db */
struct hwapi_proc_i *proc_db;

/** save my proc pointer */
struct hwapi_proc_i *my_proc;

struct launch_daemons *daemon_i;

/** shared vars db */
struct hwapi_var_i *var_db;

/** time slot duration pointer*/
int *ts_len;

/** pointer to internal interfaces db */
struct int_itf *int_itf;

int mem_silent = 0;

int hwapi_initated = 0;

/** shared memory ids */
int api_shm_id, api_msg_id, data_msg_id;

/** database for local itf */
struct file_des file_des[MAX_FILE_DES];

/** Callback function process termination */
void (*term_fnc)(void) = NULL;

/** Callback function for rtfault events */
void (*function_rt) (int);

/** Indicate if the process scheduling is governed by the kernel or by myself */
int do_sleep=0;

/** global variables for printing trace */
int bt_printed=0;

/** indicates the object is terminating */
int isterming=0;

/* I am processing an rtfault signal */
int sigrtfault_recv=0;

sigset_t waitsignals;

/** Local declarations */
void force_relinquish();
void print_bt(ucontext_t *uc);
void clear_obj_shm();

struct itf_local_callback itf_local_callback[MAX_ITF_LOCAL_CALLBACK];

/** Handler for SIGSEGV, SIGILL, etc. signals. Prints backtrace  */
void bt_sighandler(int sig, siginfo_t *info, void *secret)
{
    ucontext_t *uc = (ucontext_t *) secret;
    /* Do something useful with siginfo_t */
    if (sig == SIGSEGV)
        printf("Got signal %d, faulty address is %p\n", sig, info->si_addr);
    else
        printf("Got signal %d\n", sig);

    isterming=1;
    print_bt(uc);
    fflush(NULL);
    exit(0);
}

/** Handler for asynchronous command processing.
 * Command is indicated by the info->si_value.sival_ptr
 */
void cmd_sighandler(int sig, siginfo_t *info, void *secret) {
	int cmd,value;
	sigset_t set;
	int n;
	struct timespec timeout;

	value= (info->si_value.sival_int&0xfffffff0)>>4;
	cmd = 0xf & info->si_value.sival_int;

	switch(cmd) {
	case PROCCMD_FORCEREL:
		force_relinquish();
		break;
	case PROCCMD_RTFAULT:
		sigrtfault_recv=1;
		function_rt(value);
		sigrtfault_recv=0;
		break;
	case PROCCMD_PERIODTSK:
		hw_api_db->period_tasks[value].fnc(get_tstamp());
		break;
	case PROCCMD_ITFCALLBACK:
		itf_callback(value);
		break;
	}
}

/** Function registered atexit(). Prints backtrace if selected at platform.conf and softly terminates  */
void exit_fnc(void)
{
	isterming=1;
    if (hwapi_initated) {
        hwapi_initated=0;
        /** the object may define a callback function to call at term */
        if (term_fnc!=NULL) {
            term_fnc();
        }

        /* clear shared memory resources */
    	clear_obj_shm();

        if (hw_api_db->printbt_atexit && !bt_printed) {
            print_bt(NULL);
        }
    }
    exit(0);
}

/** Install three signal handlers:
 * - one to capture SEGV, SIGBUS, SIGILL, SIGFPE signals and print trace if enabled
 * - one for receiving asynchronous commands and
 */
void install_signals() {
    struct sigaction sa;

    /** backtrace handler for these signals */
    memset(&sa,0,sizeof(struct sigaction));
    sa.sa_sigaction = bt_sighandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);

    if (my_proc) {
    	sa.sa_sigaction = exit_fnc;
    	sigaction(SIGTERM,&sa,NULL);
    	sigaction(SIGINT,&sa,NULL);
    }

    /* block asynchronous command signal. By default received synchronously at hwapi_idle*/
    sigemptyset(&waitsignals);
	sigaddset(&waitsignals,SIG_PROC_CMD);
	sigprocmask(SIG_BLOCK,&waitsignals,NULL);
}

/** Setup shared memory, semaphores and mailboxes */
int setup_obj_shm() {
    int *ptr;
    key_t key;
    int flags=0;

    key = ftok(LOCK_FILE, HW_API_SHM_KEY);
    api_shm_id = shmget(key, sizeof (struct hw_api_db), flags);
    if (api_shm_id < 0) {
        printf("Error shared memory for api doesn't exist. Run console first\n");
        perror("shmget");
        return 0;
    }

    ptr = (int *) shmat(api_shm_id, NULL, 0);
    if (ptr == (int *) - 1) {
        perror("shmat");
        return 0;
    }

    hw_api_db = (struct hw_api_db *) ptr;
    proc_db = hw_api_db->proc_db;
    int_itf = hw_api_db->int_itf;
    var_db = hw_api_db->var_db;
    ts_len = &hw_api_db->cpu_info.tslen_usec;

    /* get lock key for control messages */
    key = ftok(LOCK_FILE, HW_API_MSG_KEY);

    /* get id for control messages */
    api_msg_id = msgget(key, flags);
    if (api_msg_id < 0) {
        printf("Error initiating message queue. Console not initiated\n");
        perror("msgget");
        return 0;
    }

    memset(itf_local_callback,0,MAX_ITF_LOCAL_CALLBACK*sizeof(struct itf_local_callback));

    return 1;
}

/** We only need to detach the shared memory */
void clear_obj_shm() {
	shmdt(hw_api_db);
}

/** @defgroup base Base API Functions
 *
 * This functions include some common and basic
 * functions required to use the HW API Library
 *
 * @{
 */

/** Initialize Hardware API.
 *
 * Initializes hwapi library. This function must be called before
 * calling any other hwapi function.
 *
 *
 * @return 1 if successfully registered
 * @return 0 if failed
 *
 */
int hwapi_init(void)
{
    int i, pid;
    struct sigaction sa;


    hwapi_mem_init();

    if (!setup_obj_shm()) {
    	exit(0);
    }

    /* initialize internal interface database */
    memset(file_des, 0, MAX_FILE_DES * sizeof (struct file_des));

	if (hw_api_db->flushout) {
		setbuf(stdout,NULL);
	}

    /* Now I have to discover whether I am an object or a daemon */
    i = 0;
    pid = getpid();
    while (i < MAX_PROCS && proc_db[i].pid != pid) {
        i++;
    }
    /* I am a process!*/
    if (i < MAX_PROCS) {
    	/** only objects register exit function */
        atexit(exit_fnc);

        if (hw_api_db->printbt_atterm) {
            sigaction(SIGTERM, &sa, NULL);
        }

        /* set to null to indicate I am not a daemon */
        daemon_i = NULL;

        /* find myself in the database */
        my_proc = &proc_db[i];

        my_proc->relinquished = 1;
        my_proc->status.next_tstamp = get_tstamp();

    /** I am a daemon! */
    } else {

		/* Now find which daemon I am */
		i = 0;
		while (i < MAX_DAEMONS && hw_api_db->daemons[i].pid != pid) {
			i++;
		}
		if (i == MAX_DAEMONS) {
			printf("CAUTION! did not find daemon in daemon's db\n");
			exit(0);
		}

		/* save pointer to daemon */
		daemon_i = &hw_api_db->daemons[i];
		daemon_i->sem_idx=-1;

		/* indicate I am not an object */
		my_proc = NULL;

		/* cmdman schedules itself using sleeps */
		do_sleep=1;
		if (strstr(daemon_i->path,"cmdman")) {
			do_sleep=0;
		}
    }

    install_signals();

	hwapi_initated=1;
    return 1;
}

/** daemons can call this function to execute based on the main kernel timer
 */
void hwapi_schedtimer() {
	sigset_t set;
	struct sigaction sa;
	int i;

	assert(daemon_i);

	/** unblock commands signal and install handler. Commands are now received asynchronously */
	memset(&sa,0,sizeof(struct sigaction));
	sa.sa_sigaction = cmd_sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_SIGINFO;
	sigaction(SIG_PROC_CMD, &sa, NULL);

	sigemptyset(&set);
	sigaddset(&set,SIG_PROC_CMD);
	sigprocmask(SIG_UNBLOCK,&set,NULL);

	/** add myself to sem_daemons (note: this list only accepts additions */
	daemon_i->sem_idx=hw_api_db->nof_sem_daemons;
	hw_api_db->nof_sem_daemons++;
}

void hwapi_idle() {
	siginfo_t info;
	int n;

	assert(daemon_i);

	n=sigwaitinfo(&waitsignals,&info);
	if (n==-1) {
		perror("sigwait");
	}

	cmd_sighandler(SIG_PROC_CMD, &info, NULL);
}

/** Exit function.
 *
 * Clears hwapi resources, no more hwapi functions can be called after
 * this functions is called.
 *
 * The function does NOT kill process thread, just clears shared memory resources
 *
 */
void hwapi_exit()
{
    /* clear shared memory resources */
    hwapi_initated=0;
    clear_obj_shm();

    exit(0);
}


void force_relinquish()
{
	printf("Not implemented\n");
}

/** Process Relinquish.
 *
 * Function used by processes to relinquish their cpu ownership.
 * This function must be called by SERVICES API before the end of the
 * time slot, otherwise RTCFault will be flagged.
 *
 * @param slots number of slots to relinquish
 * @return >=0 number of processes
 */
void hwapi_relinquish(int slots)
{
    unsigned int actual_time, tstamp;
    time_t tdata;
    int i;
    siginfo_t info;

    /** make sure we are an object */
    assert(my_proc);

    /* check params */
    if (!slots || slots < 0) {
        slots = 1;
    }

    /* get actual time */
    get_time(&tdata);
    /* and tstamp */
    tstamp = get_tstamp();

    my_proc->rel_tstamp = tstamp;

    /** compute execution time */
    memcpy(&my_proc->tdata[2],&tdata,sizeof(time_t));
    get_time_interval(my_proc->tdata);

    /* save execution stats */
    my_proc->sys_start=my_proc->tdata[1].tv_usec-hw_api_db->time.slotinit.tv_usec;
    if (my_proc->tdata[2].tv_usec>hw_api_db->time.slotinit.tv_usec)
    	my_proc->sys_end=my_proc->tdata[2].tv_usec-hw_api_db->time.slotinit.tv_usec;
    my_proc->sys_cpu = my_proc->tdata[0].tv_usec;

    my_proc->relinquished = 1;

    /** go to sleep and wait for next timeslot */
    sem_post(&hw_api_db->sem_objects[my_proc->core_idx][my_proc->sem_idx+1]);
    sem_wait(&hw_api_db->sem_objects[my_proc->core_idx][my_proc->sem_idx]);

    /* here begin the execution of the next timeslot*/
    my_proc->relinquished = 0;

	/* this is the next timeslot */
    my_proc->cur_tstamp = my_proc->rel_tstamp+1;

    /* check periodicity */
    get_time(&tdata);
    memcpy(&my_proc->tdata[2],&tdata,sizeof(time_t));
    get_time_interval(my_proc->tdata);
    my_proc->sys_period = my_proc->tdata[0].tv_usec;

    /* save to tdata[1] for computing execution time when finished */
    memcpy(&my_proc->tdata[1],&tdata,sizeof(time_t));
    

#ifdef PRINT_OBJ_TIME
    printf
            ("%d\tOBJECT %s:\tTS %d. CPU %d usec. TTI %d usec. Start %d:%d\n",
            my_proc->exec_position,my_proc->obj_name,
            my_proc->obj_tstamp, my_proc->sys_cpu, my_proc->sys_period, my_proc->tdata[1].tv_sec, my_proc->tdata[1].tv_usec);
#endif

}


/** Daemon Relinquish.
 *
 * Relinquish for daemons.
 *
 */
void hwapi_relinquish_daemon()
{
    /** make sure I am a daemon */
    assert(!my_proc);

    /* If my scheduling is governed */
    if (!do_sleep && daemon_i->sem_idx>-1) {
    	/** wait for scheduling */
		sem_wait(&hw_api_db->sem_daemons[daemon_i->sem_idx]);
    } else {
    	/* If my scheduling is governed by myself, go sleep */
    	sleep_us(hw_api_db->cpu_info.tslen_usec);
    }
}

int hwapi_addperiodfunc(void (*fnc) (int),int period) {
	int i;

	i=0;
	while(i<MAX_PERIOD_TASKS && hw_api_db->period_tasks[i].fnc) {
		i++;
	}
	if (i==MAX_PERIOD_TASKS) {
		printf("No more period tasks allowed.\n");
		return -1;
	}
	hw_api_db->period_tasks[i].period=period;
	hw_api_db->period_tasks[i].cnt=0;
	hw_api_db->period_tasks[i].pid=getpid();
	hw_api_db->period_tasks[i].fnc=fnc;

	return 1;
}


int hwapi_rmperiodfunc(void (*fnc) (int)) {
	int i;

	i=0;
	while(i<MAX_PERIOD_TASKS && hw_api_db->period_tasks[i].fnc!=fnc) {
		i++;
	}
	if (i==MAX_PERIOD_TASKS) {
		printf("Error can't find periodic function 0x%x\n",fnc);
		return -1;
	}
	memset(&hw_api_db->period_tasks[i],0,sizeof(struct period_task));
	return 1;
}


int hwapi_setperiod(void (*fnc) (int),int period) {
	int i;

	i=0;
	while(i<MAX_PERIOD_TASKS && hw_api_db->period_tasks[i].fnc!=fnc) {
		i++;
	}
	if (i==MAX_PERIOD_TASKS) {
		printf("Error can't find periodic function 0x%x\n",fnc);
		return -1;
	}
	hw_api_db->period_tasks[i].period=period;
	hw_api_db->period_tasks[i].cnt=0;
	return 1;
}


/** Install rt-fault alert callback function */
void hwapi_rtfault_callback(void (*function) (int)) {
	function_rt=function;
    sigrtfault_recv=0;
}

/** Install at_term callback function */
void hwapi_atterm(void (*fnc)(void))
{
    term_fnc = fnc;
}

/** Print backtrace */
void print_bt(ucontext_t *uc)
{
    int i, trace_size;
    void *trace[16];
    char **messages = (char **) NULL;

    if ((hw_api_db->printbt_atterm && my_proc && my_proc->status.cur_status<5)
            || hw_api_db->printbt_atexit) {
        trace_size = backtrace(trace, 16);
/*        trace[1]=uc->uc_mcontext.gregs[REG_EBP];
*/        messages = backtrace_symbols(trace, trace_size);
        printf("[bt] Execution path size %d:\n", trace_size);
        bt_printed=1;
        for (i = 1; i < trace_size; ++i)
            printf("[bt] %s\n", messages[i]);
    }
}




/** @} */


