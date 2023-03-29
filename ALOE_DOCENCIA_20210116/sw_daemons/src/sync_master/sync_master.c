/*
 * sync_master.c
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

#include "phid.h"
#include "itf_utils.h"
#include "phal_hw_api.h"
#include "sync.h"

#define ever (;;)

#define TEST
/*
#define DEB
*/
int sync_run;
int sm_testing = 0;
char line_str[128];

int fd_w, fd_r;
hwitf_t id_w, id_r;
int sm_error;

#define BUFF_SZ 128

#define MAX_CLIENTS 4
struct client {
    int fd_w;
    int fd_r;
    int continuous;
};

struct client clients[MAX_CLIENTS];

#define MAX_EXT_ITF	10
struct hwapi_xitf_i xitf_i[MAX_EXT_ITF];

void sync_master_background();
void continuous_sync(int tstamp);


void continuous_sync(int tstamp) {
	int n;
	int i;
	struct syncdata_cont pkt;
	pkt.tstamp=tstamp;
	pkt.tslot_len=get_tslot();
	for (i=0;i<MAX_CLIENTS;i++) {
		if (clients[i].continuous) {
			n = hwapi_itf_snd(clients[i].fd_w, &pkt, sizeof(struct syncdata_cont));
		}
	}

}

void start_continuous_sync(int arg) {
	clients[arg].continuous=1;
	hwapi_addperiodfunc(continuous_sync,1);
}

int answer_packet_master(int fd, int arg)
{
    int n1, n2;
    int sm_error;
    struct syncdata_tx sd_tx;
    struct syncdata_rx sd_rx;
    
    n1 = hwapi_itf_rcv(fd, &sd_tx, sizeof (struct syncdata_tx));
    if (!n1) {
      printf("received 0\n");
        return 0;
    }

    if (sd_tx.key == SYNC_CONTINUOUS) {
    	start_continuous_sync(arg);
    	printf("SYNCMS: Slave requestes continuous sync\n");
    	return 1;
    } else if (!sd_tx.key) {
    	hwapi_itf_snd(clients[arg].fd_w, &sd_tx, sizeof (struct syncdata_tx));
    	return 1;
    }

    sd_rx.next_ts=get_time_to_ts(&sd_rx.ref_time);
    sd_rx.new_tstamp=get_tstamp()+1;
    n2 = hwapi_itf_snd(clients[arg].fd_w, &sd_rx, sizeof (struct syncdata_rx));
    if (n1<sizeof (struct syncdata_tx)) {
        xprintf("SYNCMS: Size (%d) sm_error reading sync information\n", n1);
        sm_error = 1;
    }
    if (n2<sizeof (struct syncdata_rx)) {
        xprintf("SYNCMS: Size (%d) sm_error writing sync information\n", n2);
        sm_error = 1;
    }
    if (sd_tx.key != SYNC_KEY) {
        xprintf("SYNCMS: Sync Key received (0x%x) is not correct\n", sd_tx.key);
        sm_error = 1;
    }
#ifdef DEB
    xprintf("SYNCMS: Sended sync packet, snd time %d:%d tstamp=%d\n",sd_rx.ref_time.tv_sec,sd_rx.ref_time.tv_usec,sd_rx.new_tstamp);
#endif
    return 1;
}

/***********
 *   MAIN  *
 ***********/

#ifdef RUN_AS_PROC

int main(int argc, char **argv) {
	memset(clients,0,MAX_CLIENTS*sizeof(struct client));
	if (!sync_master_init_p())
		exit(0);
	sync_master_background();
}

#endif

struct sync_register sync_reg;
int n;
hwitf_t remote_id;

int sync_master_init_p(void *ptr, int daemon_idx) {

    int n, i, k;

#ifdef RUN_AS_PROC
    if (!hwapi_init()) {
#else
    if (!hwapi_init_noprocess(ptr, daemon_idx)) {
#endif
        xprintf("SYNCMS: sm_error initiating hwapi\n");
        exit(0);
    }

	n = hwapi_hwinfo_xitf(xitf_i, MAX_EXT_ITF);
	if (n < 0) {
		xprintf("SYNCMS: Error getting External interface information\n");
		return 0;
	} else if (n == MAX_EXT_ITF) {
		xprintf("SYNCMS: Caution some interfaces may have not been configured (%d)\n", n);
	}

	k = 0;
	for (i = 0; i < n && k<MAX_CLIENTS; i++) {
		if ((xitf_i[i].id & EXT_ITF_TYPE_MASK) == EXT_ITF_TYPE_SYNC_CLIENT) {
			if (xitf_i[i].mode != FLOW_READ_WRITE) {
				xprintf("SYNCMS: Error invalid mode for slave itf (%d)\n", xitf_i[i].mode);
				return 0;
			}
			if (!utils_create_attach_ext_bi(xitf_i[i].id, BUFF_SZ,
					&clients[k].fd_w, &clients[k].fd_r)) {
				xprintf("SYNCMS: Error configuring xitf 0x%x\n", xitf_i[i].id);
				return 0;
			}
#ifndef RUN_NONBLOCKING
			if (!hwapi_itf_addcallback(clients[k].fd_r, answer_packet_master,k)) {
				xprintf("SYNCMS: Error adding callback to interface\n");
			}
#endif
#ifdef DEB
			xprintf("SYNCMS: Listening on slave itf 0x%x\n",xitf_i[i].id);
#endif
			k++;
		}
	}
	if (k==MAX_CLIENTS) {
		xprintf("SYNCMS: Caution some sync slave interfaces may be unconfigured. Increase MAX_CLIENTS\n");
	}
	if (!k) {
		xprintf("SYNCMS: Caution any sync slave interface was configured\n");
	}

    printf("SYNCMS:\tInit ok\n");
	return 1;
}
void sync_master_background()
{

    sm_error = 0;
    while (!sm_error) {
#ifdef RUN_NONBLOCKING
        answer_packet_master(0);
        hwapi_relinquish_daemon();
#else
        hwapi_idle();
        #endif
    }
}
