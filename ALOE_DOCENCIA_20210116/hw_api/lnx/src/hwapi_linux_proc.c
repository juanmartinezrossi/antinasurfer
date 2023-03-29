/* hwapi_linux_proc.c
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

#define __USE_GNU
#include <ucontext.h>
#include <execinfo.h>

#include "mem.h"
#include "phal_hw_api.h"
#include "hwapi_backd.h"

/** hw_api general database */
extern struct hw_api_db *hw_api_db;

/** pointer to proc db */
extern struct hwapi_proc_i *proc_db;

/** save my proc pointer */
extern struct hwapi_proc_i *my_proc;

extern struct launch_daemons *daemon_i;

/** time slot duration pointer*/
extern int *ts_len;

#define STRING_BUFF             (SYSTEM_STR_MAX_LEN*2)

/* general string buffers */
char aux_string[STRING_BUFF];

extern int isterming;


/** @defgroup proc Process Management Functions
 *
 * This group of functions provide tools for creating, monitoring and
 * changing the status of processes (objects) running in the local processor.
 *
 * @{
 */

int hwapi_proc_needsrelocate()
{
    return 0;
}

/** Request space for an executable
 *
 * Requests space to allocate executable program space.
 * Space is splitted in a set of slots. Objects reserve one
 * or more slots so that: numslots*slotsz>program_len
 * see hwapi_dsp.h
 *
 * When this function is called, space is reserved. It won't be
 * released until a call to hwapi_proc_remove() function.
 * So if the loading processs is canceled user should call the that
 * function so the space is realeased for next calls.
 * @param request_length Requested length
 * @param addr allocated address
 * @param space allocated space
 */
int hwapi_proc_request(struct hwapi_proc_launch *pinfo)
{
	return 1;
}

/** Create a New Process.
 *
 * This function creates a process and launches it. As the process resides in user
 * memory (pdata buffer), it must be writed to the disk before launching it. This file
 * will be deleted when the process does.
 *
 * @todo (in Linux) path to downloaded software should be either relative or guessed but not a constant
 *
 * @param pdata Pointer to program data
 * @param pinfo Pointer to process information structure
 * @param core_idx Index of the core where the process must be executed
 *
 * @return >0 Pid of the process
 * @return <0 error
 */
int hwapi_proc_create(char *pdata, struct hwapi_proc_launch *pinfo, int exec_position, int core_idx)
{
    int n;
    int i;
    int f;
    int pid;
    int blen, wlen;
    int c;
    char ib;
    cpu_set_t set;

    assert(pdata && pinfo);

    if (!pinfo->app_name || !pinfo->exe_name || !pinfo->obj_name) {
        printf("HWAPI: Error launching executable. Some parameters are empty\n");
        return -1;
    }

    blen = pinfo->blen;
    wlen = pinfo->wlen;

    i = 0;
    while (i < MAX_PROCS && proc_db[i].pid && proc_db[i].obj_id) {
        i++;
    }
    if (i == MAX_PROCS) {
        printf("HWAPI: No more space in process db.\n");
        return -1;
    }


    /* check if file is already created */
    sprintf(aux_string, "%s/%s", RCV_PROC_PATH, pinfo->exe_name);
    f = open(aux_string, O_CREAT | O_WRONLY | O_EXCL, S_IRWXU);
    /* check error */
    if (f < 0 && errno != EEXIST) {
        perror("fopen");
        return -1;
    }

    /* write buffer to temporal file unless file exists */
    if (f > 0) {
        n = write(f, pdata, blen);
        if (n != blen) {
            perror("write");
            printf("removing file...\n");
            if (remove(aux_string)) {
                perror("remove");
            }
        }
    }

    /* close file */
    close(f);

    memset(&proc_db[i], 0, sizeof (struct hwapi_proc_i));

    proc_db[i].app_id = pinfo->app_id;
    proc_db[i].obj_id = pinfo->obj_id;
    proc_db[i].change_progress = 1;
    proc_db[i].status.cur_status = 0;
    proc_db[i].status.next_status = 1;
    proc_db[i].status.next_tstamp = get_tstamp();
    proc_db[i].obj_tstamp = 0;
    proc_db[i].rt_fault = 0;
    proc_db[i].core_idx = core_idx;
    proc_db[i].exec_position = exec_position;
    strncpy(proc_db[i].path, aux_string, 128);
    strncpy(proc_db[i].obj_name, pinfo->obj_name, 24);
    strncpy(proc_db[i].app_name, pinfo->app_name, 24);

    /* tell background daemon to launch the process */
    ib = i;
    if (send_parent_cmd(HWD_CREATE_PROCESS, &ib, 1, (char*) & pid, 4) != 1) {
        printf("HWAPI: Error creating child\n");
        memset(&proc_db[i], 0, sizeof (struct hwapi_proc_i));
        return -1;
    }

    /* wait creation of the process */
    c = 0;
    while (pid != proc_db[i].pid && c < 100 && pid > 0) {
        sleep_ms(5);
        c++;
    }

    /* if error, remove file and reset database structure */
    if (pid != proc_db[i].pid) {
        printf("HWAPI: Child did not started, removing it...\n");
        memset(&proc_db[i], 0, sizeof (struct hwapi_proc_i));
        return -1;
    }

    if (i+1>hw_api_db->max_proc_id)
    	hw_api_db->max_proc_id=i+1;

    return pid;
}

/** Remove Process Function:
 *
 * Removes a process from the system and de-allocates all resources
 * it was using.
 *
 * @param obj_id Id of the object to remove
 *
 * @return 1 success
 * @return 0 error
 */
int hwapi_proc_remove(int obj_id)
{
    int i;

    /* search it in the database */
    i = 0;
    while (i < hw_api_db->max_proc_id && proc_db[i].obj_id != obj_id) {
        i++;
    }
    if (i == hw_api_db->max_proc_id) {
        printf("HWAPI: Error removing object from db. Id 0x%x not found.\n", obj_id);
        return 0;
    }

    /* clear db */
    proc_db[i].obj_id = 0;

    if (i+1==hw_api_db->max_proc_id) {
    	for (i=hw_api_db->max_proc_id;i>=0;i--) {
    		if (proc_db[i].obj_id) {
    			hw_api_db->max_proc_id=i+1;
    			break;
    		}
    	}
    }

    return 1;
}

int hwapi_proc_kill(int obj_id)
{

    int c;
    int i,j;

    /* search it in the database */
    i = 0;
    while (i < hw_api_db->max_proc_id && proc_db[i].obj_id != obj_id) {
        i++;
    }
    if (i == hw_api_db->max_proc_id) {
        printf("HWAPI: Error removing object from db. Id 0x%x not found.\n", obj_id);
        return 0;
    }

    if (!proc_db[i].pid) {
    	printf("Object Id 0x%x position %d has pid=0!!\n",proc_db[i].obj_id,i);
        return 1;
    }

    /* send sigterm */
    kill(proc_db[i].pid, SIGTERM);
    c = 0;
    while (proc_db[i].pid) {
        sleep_ms(100);
        c++;
        if (c == 10) {
            printf("HWAPI: Error terminating process %d, sending kill signal...\n",
                    proc_db[i].pid);
            kill(proc_db[i].pid, SIGKILL);

            return 0;
        }
    }

    return 1;
}
/** Force a relinquish to an object
 */
int hwapi_proc_relinquish(int obj_id)
{

    int c;
    int i,j;

    /* search it in the database */
    i = 0;
    while (i < hw_api_db->max_proc_id && proc_db[i].obj_id != obj_id) {
        i++;
    }
    if (i == hw_api_db->max_proc_id) {
        printf("HWAPI: Error removing object from db. Id 0x%x not found.\n", obj_id);
        return 0;
    }

    if (!proc_db[i].pid) {
        return 1;
    }

    union sigval x;
    x.sival_int=PROCCMD_FORCEREL;
    if (sigqueue(proc_db[i].pid, SIG_PROC_CMD, x)) {
        perror("sigqueue");
    }

    return 1;
}

/** Get object new status
 *
 * Obtains the status the object must run in the current timestamp.
 *
 * This function is called by Services API during the Status()
 * call, in every object execution's loop.
 *
 * During an status change procedure, the new status won't be
 * returned after the timestamp is same or greater than the one
 * passed as the third parameter in the hwapi_proc_status_new() function
 *
 */
int hwapi_proc_status_get(void)
{
    int r;

    sem_wait(&hw_api_db->sem_status);
    r = my_proc->status.cur_status;
    if (my_proc->status.next_status != my_proc->status.cur_status) {
        if (my_proc->change_progress < 2) {
            if (get_tstamp() >= my_proc->status.next_tstamp) {
                my_proc->change_progress = 2;
                r = my_proc->status.next_status;
            }
        } else {
            my_proc->status.cur_status = my_proc->status.next_status;
            r = my_proc->status.cur_status;
        }
    }

    sem_post(&hw_api_db->sem_status);

    return r;
}

/** Set a new status for an object
 * @returns 1 if changed ok, 0 if can't change, -1 if error
 */
int hwapi_proc_status_new(int obj_id, int next_status, int next_tstamp)
{
    int i, r;
    i = 0;

    /* caller is the object and is terminating, return inmediatly */
    if (my_proc && isterming)
        return 1;

    if (!next_tstamp) {
        next_tstamp = get_tstamp();
    }

    while (i < hw_api_db->max_proc_id && proc_db[i].obj_id != obj_id) {
        i++;
    }
    if (i == hw_api_db->max_proc_id) {
        #ifdef DEB
        printf("HWAPI: Error object 0x%x not found\n", obj_id);
        #endif
        return -1;
    }

    sem_wait(&hw_api_db->sem_status);

    if ((!proc_db[i].change_progress
            && proc_db[i].status.next_status == proc_db[i].status.cur_status)
            || next_tstamp == -1) {

        proc_db[i].status.next_status = next_status;
        proc_db[i].status.next_tstamp = next_tstamp;
        proc_db[i].change_progress = 1;

        r = next_status;
    } else {
        r = 0;
    }

    sem_post(&hw_api_db->sem_status);

    return r;
}

/** Confirm a successfull status change.
 * @returns 1 if changed ok, 0 if still not changed.
 */
int hwapi_proc_status_ack(int obj_id)
{
    int i, r;
    i = 0;
    while (i < hw_api_db->max_proc_id && proc_db[i].obj_id != obj_id) {
        i++;
    }
    if (i == hw_api_db->max_proc_id) {
        printf("HWAPI: Error obj id not found 0x%x\n", obj_id);
        return -1;
    }

    sem_wait(&hw_api_db->sem_status);

    if (proc_db[i].status.cur_status == proc_db[i].status.next_status
            && proc_db[i].change_progress == 2) {
        proc_db[i].change_progress = 0;
        r = 1;
    } else {
        r = 0;
    }

    sem_post(&hw_api_db->sem_status);

    return r;
}

/** Proces Monitoring.
 *
 * Function used to gather information about current process status,
 * future status, cpu timing and other related information
 *
 * @param proc Pointer to hwapi_proc_i structure
 * @param obj_id id of the object assigned to the process
 *
 * @return 1 ok
 * @return 0 error (not found)
 */
int hwapi_proc_info(int obj_id, struct hwapi_proc_i *pinfo)
{
    int i;
    i = 0;

    assert(pinfo);

    while (i < hw_api_db->max_proc_id && proc_db[i].obj_id != obj_id) {
        i++;
    }
    if (i == hw_api_db->max_proc_id) {
        return 0;
    }

    memcpy(pinfo, &proc_db[i], sizeof (struct hwapi_proc_i));

    return 1;
}

int hwapi_proc_info_idx(int idx, struct hwapi_proc_i *pinfo)
{
    int i;
    i = 0;

    assert(pinfo);

    memcpy(pinfo, &proc_db[idx], sizeof (struct hwapi_proc_i));

    return 1;
}

/** My process information.
 *
 * Fills hwapi_proc_i structure with the caller's info.
 *
 * @param proc Pointer to hwapi_proc_i structure
 *
 */
int hwapi_proc_myinfo(struct hwapi_proc_i *pinfo)
{
    assert(pinfo && my_proc);
    memcpy(pinfo, my_proc, sizeof (struct hwapi_proc_i));

    return 1;
}

void hwapi_proc_mytstampinc()
{
    assert(my_proc);

    my_proc->obj_tstamp++;
}

/** Get Processes List.
 * Use this function with caution because gives access to kernel memory.
 * The function returns the hw_api_db->proc_db buffer size
 *
 */
int hwapi_proc_list(struct hwapi_proc_i **pinfo)
{
    int i, k;

    assert(pinfo);
    *pinfo=proc_db;
    return hw_api_db->max_proc_id;
}

int hwapi_proc_get(struct hwapi_proc_i *pinfo, int max_procs) {
	int n;

	if (hw_api_db->max_proc_id>max_procs)
		n=max_procs;
	else
		n=hw_api_db->max_proc_id;
	memcpy(pinfo,hw_api_db->proc_db,n*sizeof(struct hwapi_proc_i));
	return n;
}

/** @} */

