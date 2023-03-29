/* hwapi_linux_time.c
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


#include "mem.h"
#include "phal_hw_api.h"
#include "hwapi_backd.h"

/** hw_api general database */
extern struct hw_api_db *hw_api_db;

/** time slot duration pointer*/
extern int *ts_len;



void sleep_ms(int msec)
{
    struct timespec treq, trem;
    if (msec <= 0)
        return;
    treq.tv_sec = msec / 1000;
    treq.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&treq, &trem);
}

void sleep_us(int usec)
{
    struct timespec treq, trem;
    if (usec <= 0)
        return;
    treq.tv_sec = usec / 1000000;
    treq.tv_nsec = (usec % 1000000) * 1000;
    clock_nanosleep(CLOCK_MONOTONIC, 0, &treq, &trem);
}

void get_time(time_t * t)
{
    time_t tact;
    struct timespec x;
    clock_gettime(CLOCK_MONOTONIC,&x);
    tact.tv_sec = x.tv_sec;
    tact.tv_usec = x.tv_nsec/1000;
    timersub(&tact, &hw_api_db->time.init_time, t);
}

int get_time_to_ts(time_t *t) {
	time_t *x;
	time_t tact;
	if (t)
		x=t;
	else
		x=&tact;
	get_time(x);
	return hw_api_db->cpu_info.tslen_usec-x->tv_usec+hw_api_db->time.slotinit.tv_usec;
}

void set_time(time_t * t)
{
    time_t tact;
    time_t tt;
    struct timespec x;

    clock_gettime(CLOCK_MONOTONIC,&x);
    tact.tv_sec=x.tv_sec;
    tact.tv_usec=x.tv_nsec/1000;
    timersub(&tact, t, &hw_api_db->time.init_time);
}

int time_sync_drives_clock() {
	return hw_api_db->sync_drives_clock;
}

void time_sync(int tstamp, int usec) {
	hw_api_db->time.new_tstamp=tstamp;
	hw_api_db->time.next_ts=usec;
}

void time_sync_tslot(int tstamp) {
	hw_api_db->time.new_tstamp=tstamp;
	hw_api_db->time.next_ts=0;
	sem_post(&hw_api_db->sem_dac);
}

int hwapi_last_sync() {
    return hw_api_db->sync_ts;
}

void set_tslot(int len) {
  *ts_len = len;
}

int get_tslot() {
	return *ts_len;
}

void init_time(void)
{
    struct timespec x;
    clock_gettime(CLOCK_MONOTONIC,&x);
    hw_api_db->time.init_time.tv_sec = x.tv_sec;
    hw_api_db->time.init_time.tv_usec = x.tv_nsec/1000;
}


void add_time(time_t * t1, time_t * t2, time_t * r)
{

    timeradd(t1, t2, r);
}

void sub_time(time_t * t1, time_t * t2, time_t * r)
{

    timersub(t1, t2, r);
}

void get_time_interval(time_t * tdata)
{

    /*First element stores the result,
     *second element is the start time and
     *third element is the end time.
     */
    tdata[0].tv_sec = tdata[2].tv_sec - tdata[1].tv_sec;
    tdata[0].tv_usec = tdata[2].tv_usec - tdata[1].tv_usec;
    if (tdata[0].tv_usec < 0) {

        tdata[0].tv_sec--;
        tdata[0].tv_usec += 1000000;
    }
}

int time_to_tstamp(time_t *tdata)
{
	unsigned long long int x;
    if (!ts_len) {
        ts_len = &hw_api_db->cpu_info.tslen_usec;
    }
    x = ((unsigned long long int) tdata->tv_sec)*1000000 + tdata->tv_usec;

    return (int) (x/(*ts_len));

//    return (int) ((tdata->tv_sec*1000000+tdata->tv_usec)/(*ts_len));
}

void tstamp_to_time(int tstamp, time_t *tdata)
{
	unsigned long long int x;
    if (!ts_len) {
        ts_len = &hw_api_db->cpu_info.tslen_usec;
    }
    x = ((unsigned long long int) tstamp * (*ts_len));
    tdata[0].tv_sec = (int) (x/ 1000000);
    tdata[0].tv_usec = (int) (x - tdata[0].tv_sec * 1000000);
}

int get_tstamp(void)
{
    return (int) hw_api_db->time.tslot;
}


