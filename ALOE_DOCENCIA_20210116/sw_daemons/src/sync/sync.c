/*
 * sync.c
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "phal_hw_api.h"
#include "phid.h"
#include "sync.h"

#include "itf_utils.h"

#define ever (;;)

#define DEB

int answer_packet(int fd, int arg);

int sync_isrunning;
int sync_testing = 0;
char line_str[128];

#define BUFF_SZ 128

void sync_background();

unsigned int itot;
int fd_w, fd_r;
hwitf_t id_w, id_r;
int period;
int round_trip_limit, interval;
time_t xtime, difft;
struct syncdata_tx sd_tx;
struct syncdata_rx sd_rx;
time_t ttot;
int fbytes;
int ack;
int syncd=0;

time_t time_tx[3];

int send_sync_req()
{
    int n;
    get_time(&time_tx[1]);
    n = hwapi_itf_snd(fd_w, &sd_tx, sizeof (struct syncdata_tx));
    return n;
}

#ifdef TEST_CONT
time_t test[3];
int save[10000];
int saveidx=0;
#endif

int sync_tslot(int fd) {

	int n;
	int tstamp;
	int i,mean,max;
	time_t t;
	struct syncdata_cont pkt;

    n = hwapi_itf_rcv_(fd, &pkt, sizeof(struct syncdata_cont),1);
    if (!n)
        return 0;
    get_time(&t);

#ifdef TEST_CONT
    get_time(&test[2]);
    get_time_interval(test);
    get_time(&test[1]);
    save[saveidx++]=abs(test[0].tv_usec-1000);
    if (saveidx==10000) {
    	mean=0;
    	max=0;
    	for (i=0;i<10000;i++) {
    		mean+=save[i];
    		if (save[i]>max)
    			max=save[i];
    	}
    	mean/=10000;
        printf("Mean error is %d. Max is %d\n",mean,max);
        saveidx=0;
    }
#endif
    set_tslot(pkt.tslot_len);
    time_sync_tslot(pkt.tstamp);
    return 1;
}

int answer_packet(int fd, int arg)
{
    int n;
    int tstamp;
    int offset;

    get_time(&time_tx[2]);

    n = hwapi_itf_rcv(fd, &sd_rx, sizeof (struct syncdata_rx));
    if (!n)
        return 0;

    /*Compute loop time interval and adjust time*/
    get_time_interval(time_tx);
    tstamp=get_tstamp();
    interval=time_tx[0].tv_usec;
    if (sd_rx.next_ts>interval/2 && !time_tx[0].tv_sec && interval < round_trip_limit) {
    	sd_rx.next_ts-=interval/2;
    	printf("Reference is %d:%d, here was %d:%d\n",sd_rx.ref_time.tv_sec,sd_rx.ref_time.tv_usec,time_tx[1].tv_sec,time_tx[1].tv_usec);
    	printf("Setting tstamp %d, next-to-tstamp %d\n", sd_rx.new_tstamp,sd_rx.next_ts);
    	time_sync(sd_rx.new_tstamp,sd_rx.next_ts);

		difft.tv_sec = sd_rx.ref_time.tv_sec - time_tx[1].tv_sec;
		difft.tv_usec = sd_rx.ref_time.tv_usec - time_tx[1].tv_usec - interval/2;

#ifdef DEB
        if (!(itot%16) || difft.tv_usec > interval) {
		  printf("==- %d -==\n", itot);
		  printf("SYNC: Approx. time misalignment: %d sec, %d usec (Now is %d:%d)\n", (int) difft.tv_sec, (int) difft.tv_usec,time_tx[2].tv_sec,time_tx[2].tv_usec);
		  printf("SYNC: Max. estimated error: %d usec\n", interval / 2);
        }
#endif
    } else {
        printf("%d >>>> round trip error too large (%d.%06d): %d.%d\n", itot, time_tx[2].tv_sec,time_tx[2].tv_usec, time_tx[0].tv_sec,time_tx[0].tv_usec);
    }
    ack = 1;
    itot++;

    return 1;
}

/***********
 *   MAIN  *
 ***********/


#ifdef RUN_AS_PROC

int main(int argc, char **argv) {
	if (!sync_init_p())
		exit(0);
	sync_background();
}

#endif

int last_tstamp, n;
int period_ts;
struct sync_register sync_reg;

struct hwapi_cpu_i cpu;

int sync_init_p(void *ptr, int daemon_idx) {

	time_t t;

#ifdef RUN_AS_PROC
    if (!hwapi_init()) {
#else
    if (!hwapi_init_noprocess(ptr, daemon_idx)) {
#endif
        xprintf("SYNC: Error initiating hwapi\n");
        return 0;
    }

    /*Set default values*/
    period = DEFAULT_PERIOD;
    round_trip_limit = DEFAULT_ROUND_TRIP;

    t.tv_sec = 0;
    t.tv_usec = period * 1000;
    period_ts = time_to_tstamp(&t);

    if (!utils_create_attach_ext_id(EXT_ITF_TYPE_SYNC_MASTER, BUFF_SZ,
            &fd_w, FLOW_WRITE_ONLY, &id_w)) {
        xprintf("SYNC: Error configuring slave sync output itf\n");
        return 0;
    }
    if (!utils_create_attach_ext_id(EXT_ITF_TYPE_SYNC_MASTER, BUFF_SZ,
            &fd_r, FLOW_READ_ONLY, &id_r)) {
        xprintf("SYNC: Error configuring slave sync output itf\n");
        return 0;
    }

#ifndef RUN_NONBLOCKING
    if (time_sync_drives_clock()) {
        printf("SYNC: Time slot driven by sync_master packets\n", period_ts);
        /*if (!hwapi_itf_addcallback(fd_r,sync_tslot,0)) {
        	printf("SYNC: Error registering callback function\n");
        }*/
        sd_tx.key = SYNC_CONTINUOUS;
        if (hwapi_itf_snd(fd_w, &sd_tx, sizeof (struct syncdata_tx))<0) {
        	printf("SYNC: Error sending sync request packet\n");
        }
    } else {
        printf("SYNC: Rounded period is %d time-slots\n", period_ts);
        if (!hwapi_itf_addcallback(fd_r,answer_packet,0)) {
        	printf("SYNC: Error registering callback function\n");
        }
        sd_tx.key = SYNC_KEY;
    }

#endif

    xprintf("SYNC:\tInit Ok\n");

    sleep_ms(500);

    return 1;
}

void period_sync(int tstamp) {
    if (ack) {
        ack = 0;
        if (!syncd) {
            last_tstamp = get_tstamp();
            n = send_sync_req();
            if (n<sizeof (struct syncdata_tx)) {
                printf("SYNC: Error writing sync info to shell\n");
                exit(-1);
            }
        }
    }
}

void sync_background()
{
	int tstamp;

    /*For ever synchronise*/
    itot = 0;
    ack = 1;
    last_tstamp = get_tstamp();
    sync_isrunning = 1;

#ifndef RUN_NONBLOCKING
    if (!time_sync_drives_clock()) {
    	hwapi_addperiodfunc(period_sync,period_ts);
    }
#endif

    while (sync_isrunning)
    {
#ifdef RUN_NONBLOCKING
    	tstamp = get_tstamp();
        if ((tstamp - last_tstamp) >= period_ts) {
        	period_sync(tstamp);
        }

        /*Sleep for a period*/
        hwapi_relinquish_daemon();
#else
        if (time_sync_drives_clock()) {
        	sync_tslot(fd_r);
        } else {
            hwapi_idle();
        }
#endif
    }

}
