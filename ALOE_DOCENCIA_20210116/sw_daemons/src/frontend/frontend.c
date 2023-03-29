/*
 * frontend.c
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>. All rights reserved.
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


#include "phal_hw_api.h"

#include "phid.h"
#include "phal_daemons.h"

#include "frontend.h"
#include "frontend_cmds.h"

#include "itf_utils.h"

#include "set.h"
#include "xitf.h"
#include "str.h"
#include "pe.h"
#include "pkt.h"
#include "daemon.h"

#include "swload_cmds.h"
#include "hwman_cmds.h"

#define BUFF_SZ (20*1024)

int bridge(int pkt_len, int fd);
int register_ack();
int register_son(int slave_itf_idx);
int search_slave_itf(int pe_id);
int report_hwstatus(int tstamp);
void ident_neighbours(void);
int route(int pkt_wlen, int slave_itf_idx);
int init_daemon_cfg();
void frontend_run();
int register_to_parent(void);
int config_itfs();
int process_frontend_pkt(int pkt_len, int slave_itf_idx);
int itf_rcv(int fd, int routeto);

#define HWSTATUS_PERIOD 100
int first_monitor = 0;
int last_hwstatus_tstamp = 0;


/* buffer for packets */
char fepkt_buff[BUFF_SZ];
struct header *head = (struct header *) fepkt_buff;
char *body = &fepkt_buff[HEAD_SZ];

/** external master r/w interface */
int master_r, master_w;

/** input internal interface */
int int_input;

/** local processor id
 */
int fe_local_id;

/** Parent processor id
 */
int parent_id;


/** database for keeping ids for processors under us
 */
struct pe_under {
    peid_t id; /**< Assigned PE Id */
    int reqid; /**< Requested PE Id */
    int slave_itf_idx; /**< Index in the slave interface database */
};

typedef struct pe_under *pe_under_o;


#define NOF_SET	10
#define NOF_PE 50
#define NOF_STR 50


void frontend_alloc()
{
    hwapi_mem_prealloc(NOF_STR, str_sizeof);
    hwapi_mem_prealloc(NOF_PE, pe_sizeof);
    hwapi_mem_prealloc(NOF_PE, sizeof(struct pe_under));
    hwapi_mem_prealloc(NOF_SET, set_sizeof);
    hwapi_mem_prealloc(NOF_PE, setmember_sizeof);
    hwapi_mem_prealloc(1, daemon_sizeof);
    hwapi_mem_prealloc(1, pkt_sizeof);
}

/** database for slave interfaces
 */
struct slave_itf {
    int phy_itf;
    int fd_r;
    int fd_w;
};

#define MAX_SLAVE_ITF 	10
#define MAX_EXT_ITF 	10
struct slave_itf slave_itf[MAX_SLAVE_ITF];
struct hwapi_xitf_i xitf_i[MAX_EXT_ITF];

/** database for daemons 
 */
struct daemon_cfg {
    unsigned char itf_id;
    int fd_w; /**< File descriptor for writing itf. 0 if not present */
    int ismanager;
};

struct daemon_cfg daemon_cfg[DAEMON_MAX];


Set_o f_pedb = NULL;

int pe_under_findid(const void *x, const void *member)
{
    pe_under_o pe = (pe_under_o) member;

    return *((int*) x) != pe->id;
}

int pe_under_findreqid(const void *x, const void *member)
{
    pe_under_o pe = (pe_under_o) member;

    return *((int*) x) != pe->reqid;
}



pkt_o pkt;

int fpc=0;

void frontend_process_cmd(int blocking) {
	frontend_run();
}

int frontend_init_p(void *ptr, int daemon_idx) {

#ifdef RUN_AS_PROC
    if (!hwapi_init()) {
#else
    if (!hwapi_init_noprocess(ptr, daemon_idx)) {
#endif
        xprintf("F-END:\tError initiating HWAPI. Exiting...\n");
        exit(-1);
    }


    frontend_alloc();

    pkt = pkt_new((int*) fepkt_buff, BUFF_SZ);
    f_pedb = Set_new(0, NULL);
    if (!pkt || !f_pedb) {
        xprintf("F-END:\tError initiating\n");
        return 0;
    }

    memset(slave_itf, 0, MAX_SLAVE_ITF * sizeof (struct slave_itf));

    init_daemon_cfg();

    if (!config_itfs()) {
        xprintf("F-END:\tError initializing interfaces\n");
        return 0;
    }

    /* register to parent front-end or become a local master */
    if (master_r && master_w) {
        /* we have a master control interface, so we have to register
         * to our parent
         */
        fe_local_id = 0;
        parent_id = 0;
        register_to_parent();

    } else {
        /* we do not have a master interface, so we do not have a parent
         * who register to
         */
        fe_local_id = 2;
        parent_id = 2;
        hwapi_hwinfo_setid(fe_local_id);
        xprintf("F-END:\tI'm a local master.\n");
    }


    xprintf("F-END:\tInit ok\n");

    return 1;
}

int max_n;

int itf_rcv(int fd, int routeto) {
	int n;

	do {
		n = hwapi_itf_rcv(fd, fepkt_buff, BUFF_SZ);
		if (n < 0) {
			xprintf("F-END:\tError receiving from interface.\n");
			return 0;
		}
		if (n > 0) {
			route(n, routeto);
			max_n = n > max_n ? n : max_n;
		}
	} while (n>0);
	return 1;
}


void frontend_run()
{
    int i, n;
	unsigned int *p;
	int tstamp;

    #ifndef RUN_NONBLOCKING
	if (master_r) {
		if (!hwapi_itf_addcallback(master_r,itf_rcv,-1)) {
			xprintf("FRONTEND: Can't add itf callback\n");
		}
	}

	if (!hwapi_itf_addcallback(int_input,itf_rcv,-1)) {
		xprintf("FRONTEND: Can't add itf callback\n");
	}


	/** Here I have the id, now register the slave itf callback functions, if any */
	for (i = 0; i < MAX_SLAVE_ITF; i++) {
		 if (slave_itf[i].fd_r > 0) {
			 if (!hwapi_itf_addcallback(slave_itf[i].fd_r,itf_rcv,i)) {
				 xprintf("FRONTEND: Can't add itf callback\n");
			 }
		 }
	 }


	hwapi_addperiodfunc(report_hwstatus, 1);

	/** register slave itf after registered */
#endif

    /* main loop */
    while (1) {

#ifndef RUN_NONBLOCKING


    	hwapi_idle();

#else
    	tstamp=get_tstamp();

        max_n = 0;

        if (last_hwstatus_tstamp < tstamp - HWSTATUS_PERIOD) {
        	report_hwstatus(tstamp);
        	last_hwstatus_tstamp=tstamp;
        }

        if (master_r)
        master_rcv(master_r,-1);

        input_rcv(int_input,-1);

        /* rcv from all slave control interfaces if we already registered */
        if (fe_local_id) {
            for (i = 0; i < MAX_SLAVE_ITF; i++) {
                if (slave_itf[i].fd_r > 0) {
                	slave_process(slave_itf[i].fd_r,i);
                }
            }
        }

        if (!max_n) {
            hwapi_relinquish_daemon();
        }
#endif
    }
}


#ifdef RUN_AS_PROC

int main()
{

	if (!frontend_init_p(NULL,0))
		exit(0);


    frontend_run();
    return 1;

}
#endif

int init_daemon_cfg(void)
{
    int i;

    memset(daemon_cfg, 0, DAEMON_MAX * sizeof (struct daemon_cfg));

    for (i = 0; i < DAEMON_MAX; i++) {
        daemon_cfg[i].itf_id = i;

        switch (i) {
        case DAEMON_CMDMAN:
        case DAEMON_STATSMAN:
        case DAEMON_SWMAN:
        case DAEMON_HWMAN:
            daemon_cfg[i].ismanager = 1;
            break;
        default:
            daemon_cfg[i].ismanager = 0;
            break;
        }
    }
    return 1;
}

int config_itfs(void)
{
    int n, i, k;

    n = hwapi_hwinfo_xitf(xitf_i, MAX_EXT_ITF);
    if (n < 0) {
        xprintf("F-END:\tError getting External interface information\n");
        return 0;
    } else if (n == MAX_EXT_ITF) {
        xprintf("F-END:\tCaution some interfaces may have not been configured (%d)\n", n);
    }

    k = 0;
    for (i = 0; i < n; i++) {
        if ((xitf_i[i].id & EXT_ITF_TYPE_MASK) == EXT_ITF_TYPE_SLAVE) {
            if (xitf_i[i].mode != FLOW_READ_WRITE) {
                xprintf("F-END:\tError invalid mode for slave itf (%d)\n", xitf_i[i].mode);
                return 0;
            }
            if (!utils_create_attach_ext_bi(xitf_i[i].id, BUFF_SZ,
                    &slave_itf[k].fd_w, &slave_itf[k].fd_r)) {
                xprintf("F-END:\tError configuring xitf 0x%x\n", xitf_i[i].id);
                return 0;
            }
            slave_itf[k].phy_itf = xitf_i[i].id;
            k++;
        } else if (xitf_i[i].id == EXT_ITF_TYPE_MASTER) {
            if (xitf_i[i].mode != FLOW_WRITE_READ) {
                xprintf("F-END:\tError invalid mode for master itf (%d)\n", xitf_i[i].mode);
                return 0;
            }
            if (!utils_create_attach_ext_bi(EXT_ITF_TYPE_MASTER, BUFF_SZ, &master_w, &master_r)) {
                xprintf("F-END:\tError configuring master external interface.\n");
                return 0;
            }
        }
    }

    /* configure internal input interface */
    int_input = utils_create_attach_itf(DAEMON_FRONTEND,
            FLOW_READ_ONLY, 4096);
    if (int_input <= 0) {
        if (int_input == -2) {
            xprintf("F-END:\tInterface does not exist\n");
        }
        xprintf("F-END:\tError creating input internal interface\n");
        return 0;
    }

    return 1;
}

int cfg_daemon_itf(dmid_t daemon_code, hwitf_t itf_id)
{
    int n;

    assert((daemon_code > 0 && daemon_code < DAEMON_MAX) && itf_id);

    n = hwapi_itf_attach(itf_id, FLOW_WRITE_ONLY);
    if (!n) {
        /* daemon is not present */
        daemon_cfg[daemon_code].fd_w = 0;
    } else {
        daemon_cfg[daemon_code].fd_w = n;
    }

    return n;
}

int route(int pkt_len, int slave_itf_idx)
{
    int n;
    struct daemon_cfg *daemon;
    int dest_daemon;
    int out_fd;


    dest_daemon = head->dst_daemon;

    if (dest_daemon < 0 || dest_daemon > DAEMON_MAX) {
        xprintf("F-END:\tError with destination daemon (%d from %d,len %d, itf %d)\n", dest_daemon, head->src, pkt_len, slave_itf_idx);
        return -1;
    }

    /** every packet entering my input interface has 0 src id.
     * Replace it with correct local id
     */
    if (!head->src) {
        head->src = fe_local_id;
    }

    /** if destination daemon is F-END, process it */
    if (dest_daemon == DAEMON_FRONTEND) {
        return process_frontend_pkt(pkt_len, slave_itf_idx);
    }

    out_fd = 0;
    daemon = &daemon_cfg[dest_daemon];

    /** Here begins routing algorithm
     */

    /** If destination is local */
    if (head->dst_pe == fe_local_id) {
        if (daemon->fd_w) {
            out_fd = daemon->fd_w;
        } else {
            xprintf("F-END:\tReceived a packet for me but can't find daemon (%d)\n", dest_daemon);
        }
    }/** if destination is 0, it is a packet going to myself,
	 * in the case the daemon is in my processor, or upwards otherwise
	 */
    else if (head->dst_pe == 0) {
        if (daemon->fd_w) {
            out_fd = daemon->fd_w;
        } else {
            if (master_w) {
                out_fd = master_w;
            } else {
                xprintf("F-END:\tCan't bridge packet upwards. (Dest daemon %d, fd %d)\n", dest_daemon, daemon->fd_w);
            }
        }
    }/** if destination is 1, packet goes upwards directly or lost*/
    else if (head->dst_pe == 1 && master_w) {
        head->dst_pe = 0;
        out_fd = master_w;
    }/** Rest of the packets must find they route to the destination
	 * or in other words, must be bridged to the slave interface where
	 * the processor can be found
	 */
    else if (head->dst_pe > 1) {
        /* bridge downwards */
        n = search_slave_itf(head->dst_pe);
        if (n >= 0) {
            out_fd = slave_itf[n].fd_w;
        } else {
            xprintf("F-END:\tError destination PE not found (%d)\n", head->dst_pe);
        }
    }

    if (!out_fd) {
        /* if fall here, packet is discarted, don't show error if don't have master */
        if (master_w) {
            xprintf("F-END:\tError routing. Packet discarted.\n");
        }
    } else {
       bridge(pkt_len, out_fd);
    }
    return 1;
}

int bridge(int pkt_len, int fd)
{
    return hwapi_itf_snd(fd, fepkt_buff, pkt_len);
}

int search_slave_itf(int pe_id)
{
    pe_under_o pe;

    pe = Set_find(f_pedb, &pe_id, pe_under_findid);
    if (!pe) {
        return -1;
    } else {
        return pe->slave_itf_idx;
    }
}

int process_frontend_pkt(int pkt_len, int slave_itf_idx)
{

    switch (head->cmd) {
    case FRONTEND_CFGITF:
        cfg_daemon_itf((dmid_t) body[0], (hwitf_t) body[1]);
        break;
    case FRONTEND_REGISTERSON:
        pkt_readvalues(pkt, pkt_len);
        register_son(slave_itf_idx);
        break;
    case FRONTEND_REGISTERACK:
        pkt_readvalues(pkt, pkt_len);
        register_ack();
        break;
    default:
        xprintf("F-END:\tError invalid command.\n");
        break;
    }
    return 1;
}

int register_to_parent(void)
{
    if (!master_w) {
        return 0;
    }

    pkt_clear(pkt);
    if (!pkt_putvalue(pkt, FIELD_PEID, 4) ||
            !pkt_putvalue(pkt, FIELD_REQPEID, 4)) {
        xprintf("FRON-END: Could not send register packet\n");
        return 0;
    }
    pkt_setdestdaemon(pkt, DAEMON_FRONTEND);
    pkt_setcmd(pkt, FRONTEND_REGISTERSON);

    if (hwapi_itf_snd(master_w, fepkt_buff, pkt_len(pkt)) < 0) {
        xprintf("F-END:\tError registering to parent\n");
        return 0;
    }

    xprintf("F-END:\tRegistering to parent...\n");

    return 1;
}

int register_son(int slave_itf_idx)
{
    pe_under_o pe;
    int found;
    int proposed_id, id;

    id = pkt_getvalue(pkt, FIELD_PEID);
    proposed_id = pkt_getvalue(pkt, FIELD_REQPEID);

    if (!id || !proposed_id) {
        xprintf("F-END:\tReceived an invalid register packet from son. Registration uncomplete. (%d,%d)\n",id,proposed_id);
        return 0;
    }
    do {
        found = 0;
        if (id != fe_local_id) {
            pe = Set_find(f_pedb, &id, pe_under_findid);
            if (pe) {
                found = 1;
            }
        } else {
            found = 1;
        }
        if (found) {
            id++;
        }
    } while (found);

    NEW(pe);
    assert(pe);

#ifdef DEB
    xprintf("F-END:\tRegistering son with id 0x%x reqid 0x%x\n", id, proposed_id);
#endif
    pe->id = id;
    pe->reqid = proposed_id;
    pe->slave_itf_idx = slave_itf_idx;

    Set_put(f_pedb, pe);

    pkt_clear(pkt);
    pkt_putvalue(pkt, FIELD_PEID, id);
    pkt_putvalue(pkt, FIELD_REQPEID, proposed_id);

    if (master_w) {
        hwapi_itf_snd(master_w, fepkt_buff, pkt_len(pkt));
    } else {
        pkt_setcmd(pkt, FRONTEND_REGISTERACK);
        hwapi_itf_snd(slave_itf[slave_itf_idx].fd_w, fepkt_buff, pkt_len(pkt));
    }

    return 1;
}

int register_ack()
{
    pe_under_o pe;
    int proposed_id, id;
    int i;

    id = pkt_getvalue(pkt, FIELD_PEID);
    if (!id) {
        xprintf("F-END:\tReceived an invalid register packet anser. Registration uncomplete.\n");
        return 0;
    }

    /* first packet, not yet registered */
    if (!fe_local_id) {
        fe_local_id = id;
        hwapi_hwinfo_setid(fe_local_id);
        xprintf("F-END:\tReceived connection. PEId=0x%x.\n", fe_local_id);

    } else {
        proposed_id = pkt_getvalue(pkt, FIELD_REQPEID);
        pe = Set_find(f_pedb, &proposed_id, pe_under_findreqid);
        if (!pe) {
            xprintf("F-END:\tError can't find pe with id 0x%x\n", proposed_id);
            return 0;
        }

        pkt_clear(pkt);
        pkt_putvalue(pkt, FIELD_PEID, id);
        pkt_putvalue(pkt, FIELD_REQPEID, pe->reqid);

        pe->id = id;

        if (hwapi_itf_snd(slave_itf[pe->slave_itf_idx].fd_w, fepkt_buff, pkt_len(pkt)) < 0) {
            xprintf("F-END:\tError sending packet\n");
            return 0;
        }

        xprintf("F-END:\tAdded son PEId=0x%x REQ=0x%x\n", id, proposed_id);
    }
    return 1;
}



/** @todo: This function should gather other hardware parameters and report them to HWMAN
 */
int report_hwstatus(int tstamp)
{
    int fd;
    pe_o pe;
    struct hwapi_cpu_i cpu_i;
    int cmd;

    if (!master_w) {
        if (!daemon_cfg[DAEMON_HWMAN].fd_w) {
            return 0;
        }
    }
    cmd = 0;
	if (fe_local_id>1) {
	    if (!first_monitor) {
	        first_monitor = 1;
	        cmd = HWMAN_ADDCPU;
	    	hwapi_setperiod(report_hwstatus, HWSTATUS_PERIOD);

	    } else if (!daemon_cfg[DAEMON_HWMAN].fd_w && master_w) {
			cmd = HWMAN_UPDCPU;
	    }
	}
    if (!cmd)
        return 0;

    last_hwstatus_tstamp = tstamp;

    pe = pe_new();
    if (!pe) {
        return -1;
    }

    hwapi_hwinfo_cpu(&cpu_i);
    pe->id = fe_local_id;
    pe->C = cpu_i.C;
    pe->tslen_us = cpu_i.tslen_usec;
    pe->intBW = cpu_i.intBW;
    pe->plat_family = cpu_i.plat_family;
    pe->tstamp = tstamp;
    pe->nof_cores = cpu_i.nof_cores;
    pe->exec_order = cpu_i.exec_order;
    str_set(pe->name, cpu_i.name);

    pkt_clear(pkt);
    if (!pkt_put(pkt, FIELD_PE, pe, pe_topkt)) {
        xprintf("F-END:\tError building packet\n");
    } else {
        if (!daemon_cfg[DAEMON_HWMAN].fd_w && master_w) {
            fd = master_w;
        } else if (daemon_cfg[DAEMON_HWMAN].fd_w) {
            fd = daemon_cfg[DAEMON_HWMAN].fd_w;
        } else {
            xprintf("F-END:\tError can't send hwreport\n");
            return -1;
        }

        pkt_setdestpe(pkt, 0);
        pkt_setdestdaemon(pkt, DAEMON_HWMAN);
        pkt_setcmd(pkt, cmd);

        if (!hwapi_itf_snd(fd, fepkt_buff, pkt_len(pkt))) {
            xprintf("F-END:\tError sending packet\n");
        }
    }
    pe_delete(&pe);
    return 1;
}

