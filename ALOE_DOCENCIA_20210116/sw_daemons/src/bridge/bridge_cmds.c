/*
 * bridge_cmds.c
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
#include <string.h>



#include "phal_hw_api.h"

#include "phid.h"
#include "phal_daemons.h"

#include "xitf.h"
#include "pkt.h"
#include "set.h"
#include "daemon.h"

#include "bridge.h"

#include "hwman_cmds.h"
#include "bridge_cmds.h"
#include "frontend.h"

/** enable to print debug messages */
/*#define DEB
#define DEB_PKT
*/
#define DEFAULT_EXT_OUT	1 /* itf number, in case of non-local objects */

#define MAX_IN_ITF 5
#define MAX_OUT_ITF 5

#define MAX_XITF (MAX_IN_ITF+MAX_OUT_ITF)

#define BRIDGE_MAX_ITF 10

#define DEF_PKT_DATA_LEN (16*1024)

#define NOF_SET	1
#define NOF_SETMEM 10
#define NOF_DAEMON 1
#define NOF_PKT 1
#define NOF_XITF 50

int itf_rcv(int fd, int i);

void bridge_alloc()
{
#ifdef MTRACE
    mtrace();
#endif
    hwapi_mem_prealloc(NOF_SET, set_sizeof);
    hwapi_mem_prealloc(NOF_SETMEM, setmember_sizeof);
    hwapi_mem_prealloc(NOF_DAEMON, daemon_sizeof);
    hwapi_mem_prealloc(NOF_PKT, pkt_sizeof);
    hwapi_mem_prealloc(NOF_XITF, xitf_sizeof);
}


/*Buffer for any packet*/
char gbuff[DEF_PKT_DATA_LEN];

struct bridge_itf bridge_itf_table[BRIDGE_MAX_ITF];

int nof_in_itf, nof_out_itf;

struct ext_itf ext_in_itf[MAX_IN_ITF];
struct ext_itf ext_out_itf[MAX_OUT_ITF];

static int pe_id = 0;

static struct hwapi_xitf_i xitf[MAX_XITF];

static daemon_o daemon;
static pkt_o pkt;


/** Adds a new neighgour
 */
int new_neighbour(int fd, void *packet)
{
    struct bridge_ident *bident;
    xitf_o xitf;
    int loc_itf_id;
    int i;

    i=0;
    while(i<nof_in_itf && ext_in_itf[i].fd!=fd) {
    	i++;
    }
    if (i==nof_in_itf) {
    	daemon_error(daemon,"Error in new_neighbour. Can't find fd %d\n",fd);
    	return 0;
    }
    loc_itf_id=ext_in_itf[i].id;

    bident = (struct bridge_ident*) packet;

#ifdef DEB
    daemon_info(daemon, "New neighbour with id 0x%x (litf=0x%x,oitf=0x%x,C=%.2f)",
            bident->pe_id, loc_itf_id, bident->xitf.id, bident->xitf.BW);
#endif

    xitf = xitf_new();
    if (!xitf) {
        return 0;
    }

    memcpy(&xitf->xitf, &bident->xitf, sizeof (struct hwapi_xitf_i));
    xitf->remote_id = loc_itf_id;
    xitf->remote_pe = bident->pe_id;

    /* send to SWMAP */
    pkt_clear(pkt);
    pkt_putvalue(pkt, FIELD_PEID, pe_id);
    pkt_put(pkt, FIELD_XITF, xitf, xitf_topkt);
    xitf_delete(&xitf);

    daemon_sendto(daemon, HWMAN_ADDITF, 0, DAEMON_HWMAN);

    return 1;
}

/** Initialize function.
 *
 * Create interfaces and initialize them
 */
int bridge_init(daemon_o d)
{
    int i, n;
    struct hwapi_cpu_i cpu_i;

    daemon = d;
    pkt = daemon_pkt(daemon);

    memset(bridge_itf_table, 0, sizeof (struct bridge_itf) * BRIDGE_MAX_ITF);
    memset(ext_in_itf, 0, MAX_IN_ITF * sizeof (struct ext_itf));
    memset(ext_out_itf, 0, MAX_IN_ITF * sizeof (struct ext_itf));

    nof_in_itf = 0;
    nof_out_itf = 0;

    /* wait to be registered */
    do {
        hwapi_hwinfo_cpu(&cpu_i);
        pe_id = cpu_i.pe_id;
        if (!pe_id) {
            hwapi_relinquish_daemon();
        }
    } while (!pe_id);

    n = hwapi_hwinfo_xitf(xitf, MAX_XITF);

    for (i = 0; i < n; i++) {
        if (xitf[i].id & EXT_ITF_DATA_MASK) {
            if (xitf[i].mode == FLOW_READ_ONLY) {
                if (!utils_create_attach_ext(xitf[i].id, 0xf000, &ext_in_itf[nof_in_itf].fd, FLOW_READ_ONLY)) {
                    daemon_error(daemon, "Creating itf %d", xitf[i].id);
                } else {
                    ext_in_itf[nof_in_itf].id = xitf[i].id;
                    ext_in_itf[nof_in_itf].xitf_pos = i;
                    daemon_info(daemon, "External input itf 0x%x configured fd %d.", xitf[i].id, ext_in_itf[nof_in_itf].fd);
#ifndef RUN_NONBLOCKING
                    if (!hwapi_itf_addcallback(ext_in_itf[nof_in_itf].fd,itf_rcv,-1)) {
                    	daemon_error(daemon,"Can't add itf callback\n");
                    }
                    nof_in_itf++;
#endif
                }
            } else if (xitf[i].mode == FLOW_WRITE_ONLY) {
                if (!utils_create_attach_ext(xitf[i].id, 0xf000, &ext_out_itf[nof_out_itf].fd, FLOW_WRITE_ONLY)) {
                    daemon_error(daemon, "Creating itf %d", xitf[i].id);
                } else {
                    ext_out_itf[nof_out_itf].id = xitf[i].id;
                    ext_out_itf[nof_out_itf].xitf_pos = i;
                    daemon_info(daemon, "External output itf 0x%x configured fd %d.", xitf[i].id, ext_out_itf[nof_out_itf].fd);
                    nof_out_itf++;
                }
            }
        }
    }
    return 1;
}

/** Bridge a packet to inside
 */
int packet_in(char *pbuff, int pktsize, int keep_header)
{
    int n;
    int itf;
    int i;
    int obj_tstamp;
    data_header_t *head = (data_header_t*) pbuff;

    /* search destination itf */
    for (i=0;i<BRIDGE_MAX_ITF;i++) {
    	if (head->ObjId == bridge_itf_table[i].dest.ObjId
            && head->ItfId == bridge_itf_table[i].dest.ItfId) {
    		break;
    	}
    }
    if (i < BRIDGE_MAX_ITF) {
        itf = bridge_itf_table[i].fd_w;
    } else {
        daemon_error(daemon, "Can't find destination 0x%x:0x%x", head->ObjId, head->ItfId);
        return 0;
    }

    /* if destination itf is internal don't transmit header */
    if (!keep_header) {
        pbuff += PH_HEADER_SZ;
        pktsize -= PH_HEADER_SZ;
    }
/*
    obj_tstamp=hwapi_proc_getObjTstamp(head->ObjId);
    if (head->obj_tstamp>obj_tstamp+4) {
    	printf("BRIDGE: Dropping delayed packet tstamp %d, obj tstamp %d\n", head->obj_tstamp,obj_tstamp);
    } else {
*/
    /* route packet */
        n = hwapi_itf_snd_(itf, pbuff, pktsize,head->obj_tstamp);
        if (!n) {
            printf("BRIDGE: Error bridging packet. Full queue.\n");
        }
    /*}*/


    return (n);
}

/** Bridge a packet to outside
 */
int packet_out(char *pbuff, int pktsize, data_header_t *hd, int fd_w,
        int make_header, int obj_tstamp)
{
    int n;
    data_header_t *hd_pkt = (data_header_t*) pbuff;

    if (make_header) {
        hd_pkt->ItfId=hd->ItfId;
        hd_pkt->ObjId=hd->ObjId;
        hd_pkt->obj_tstamp=obj_tstamp;
        pktsize += PH_HEADER_SZ;
    } else {
        pbuff += PH_HEADER_SZ;
    }

    /* route packet */
    n = hwapi_itf_snd(fd_w, pbuff, pktsize);
    if (!n) {
        printf("BRIDGE: Error bridging packet to outside. Full queue.\n");
    }
    return (n);
}



/** Identify Command
 *
 * Command to start identification of output interfaces.
 *
 * Any field
 */
int bridge_incmd_ident(cmd_t *cmd)
{
    int i, n;
    struct bridge_ident *bident;
    char msg[128];
    time_t *tdata=(time_t*)msg;


    bident = (struct bridge_ident*) gbuff;
    bident->magic = BRIDGE_IDENT_MAGIC;
    bident->pe_id = pe_id;

    #ifdef DEB
    daemon_info(daemon, "Received ident request to %d interfaces", nof_out_itf);
    #endif

    for (i = 0; i < nof_out_itf; i++) {
        if (ext_out_itf[i].fd) {
/*
            get_time(tdata);
            n = hwapi_itf_snd(ext_out_itf[i].fd, msg, 128);
            if (n < 0) {
                daemon_error(daemon, "Error writting to itf 0x%x",
                        ext_in_itf[i].id);
                return -1;
            }
*/
            memcpy((char*) & bident->xitf,
                    (char*) & xitf[ext_out_itf[i].xitf_pos],
                    sizeof (struct hwapi_xitf_i));

            n = hwapi_itf_snd(ext_out_itf[i].fd, gbuff,
                    BIDENT_PKT_SZ);
            if (n < 0) {
                daemon_error(daemon, "Error writting to itf 0x%x",
                        ext_in_itf[i].id);
                return -1;
            }
            #ifdef DEB
            daemon_info(daemon, "Ident packet sended to 0x%x BW=%.2f!!", ext_out_itf[i].id, xitf[ext_out_itf[i].xitf_pos].BW);
            #endif
        }
    }

    return 1;
}


void bridge_rmitf_i(int i, int obj_mode)
{
    if (obj_mode == FLOW_WRITE_ONLY) {
        hwapi_itf_unattach(bridge_itf_table[i].fd_r, FLOW_READ_ONLY);
#ifndef RUN_NONBLOCKING
        hwapi_itf_rmcallback(bridge_itf_table[i].fd_r);
#endif

    } else if (obj_mode == FLOW_READ_ONLY) {
        hwapi_itf_unattach(bridge_itf_table[i].fd_w, FLOW_WRITE_ONLY);
    } else {
        daemon_error(daemon, "Invalid mode %d\n", obj_mode);
    }
    #ifdef DEB
    daemon_info(daemon, "Deleted itf 0x%x:0x%x at %d", bridge_itf_table[i].dest.ObjId, bridge_itf_table[i].dest.ItfId, i);
    #endif
    memset(&bridge_itf_table[i], 0, sizeof (struct bridge_itf));
}


/** Remove itf command
 *
 * Removes an object interface
 *
 * Fields:
 *  FIELD_OBJITFID:  Id of the object interface
 *  FIELD_OBJITFMODE: Mode of the object interface
 *  FIELD_OBJID: Id of the object
 */
int bridge_incmd_rmitf(cmd_t *cmd)
{
    int i;
    int itf_id, obj_id, obj_mode;

    itf_id = pkt_getvalue(pkt, FIELD_OBJITFID);
    obj_mode = pkt_getvalue(pkt, FIELD_OBJITFMODE);
    obj_id = pkt_getvalue(pkt, FIELD_OBJID);

    if (!itf_id || !obj_id) {
    	daemon_error(daemon, "Invalid zero destination\n");
    	return 0;
    }

    for (i=0;i<BRIDGE_MAX_ITF;i++) {
    	if (bridge_itf_table[i].dest.ObjId == obj_id
                && bridge_itf_table[i].dest.ItfId == itf_id) {
    		break;
    	}
    }
    if (i == BRIDGE_MAX_ITF) {
        daemon_error(daemon, "Unknown itf 0x%x:0x%x", obj_id, itf_id);
        return 0;
    }
    bridge_rmitf_i(i, obj_mode);
    return 1;
}


/** Add itf command
 *
 * Adds an object interface and starts bridging packets
 *
 * Fields:
 *  FIELD_OBJITFID:  Id of the object interface
 *  FIELD_OBJXITFID: Id of the external interfac
 *  FIELD_OBJITFMODE: Mode of the object interface
 *  FIELD_DATAHEADER: see data_header_t at bridge.h (objid and itfid)
 */
int bridge_incmd_additf(cmd_t *cmd)
{
    int i, j, n;
    data_header_t *head;
    hwitf_t obj_itf_id;
    xitfid_t ext_itf_id;
    char obj_mode;

    head = (data_header_t*) pkt_getptr(pkt, FIELD_DATAHEADER);
    if (!head) {
        daemon_error(daemon, "Invalid packet.");
        return 0;
    }

    if (!head->ObjId || !head->ItfId) {
    	daemon_error(daemon, "Invalid zero destination\n");
    	return 0;
    }

    obj_itf_id = (hwitf_t) pkt_getvalue(pkt, FIELD_OBJITFID);
    obj_mode = (char) pkt_getvalue(pkt, FIELD_OBJITFMODE);
    ext_itf_id = (xitfid_t) pkt_getvalue(pkt, FIELD_OBJXITFID);

    if (!obj_itf_id || !obj_mode || !ext_itf_id) {
        daemon_error(daemon, "Invalid packet.");
        return 0;
    }
    /* search if entry already in db */
    j=-1;
    for (i=0;i<BRIDGE_MAX_ITF;i++) {
    	if (head->ObjId == bridge_itf_table[i].dest.ObjId
            && head->ItfId == bridge_itf_table[i].dest.ItfId) {
    		j=i;
    		break;
    	}
    }
    /* if not found, find a free space */
    if (j==-1) {
    	i=0;
    	while(i<BRIDGE_MAX_ITF && bridge_itf_table[i].dest.ObjId)
    		i++;
    	if (i==BRIDGE_MAX_ITF) {
    		daemon_error(daemon, "No more space in interface table");
    		return 0;
    	} else {
    		memcpy(&bridge_itf_table[i].dest, head, sizeof (data_header_t));
    	}
    } else {
    	i=j;
    }

    /* attach bridge side of object interface if not yet attached*/
    if (obj_mode == FLOW_WRITE_ONLY) {
        if (bridge_itf_table[i].fd_r) {
            hwapi_itf_unattach(bridge_itf_table[i].fd_r, FLOW_READ_ONLY);
            printf("replacing fd r %d\n",i);
        }
        n = hwapi_itf_attach(obj_itf_id, FLOW_READ_ONLY);
        if (n < 0) {
            daemon_error(daemon, "Can't attach to itf 0x%x.", obj_itf_id);
            memset(&bridge_itf_table[i], 0, sizeof (struct bridge_itf));
            return 0;
        }
        bridge_itf_table[i].fd_r = n;

#ifndef RUN_NONBLOCKING
        if (!hwapi_itf_addcallback(bridge_itf_table[i].fd_r,itf_rcv,i)) {
        	daemon_error(daemon, "Can't add itf callback\n");
        }
#endif

        #ifdef DEB
        daemon_info(daemon, "Attaching %d to 0x%x", bridge_itf_table[i].fd_r, obj_itf_id);
        #endif
        
        j = 0;
        while (j < nof_out_itf && ext_out_itf[j].id != ext_itf_id)
            j++;
        if (j == nof_out_itf) {
            daemon_error(daemon, "Can't find external itf 0x%x",
                    ext_itf_id);
            memset(&bridge_itf_table[i], 0, sizeof (struct bridge_itf));
            return 0;
        }
        if (!ext_out_itf[j].fd) {
            daemon_error(daemon, "External interface must be configurated first (0x%x at %d)",
                    ext_itf_id, j);
            memset(&bridge_itf_table[i], 0, sizeof (struct bridge_itf));
            return 0;
        }
        bridge_itf_table[i].fd_w = ext_out_itf[j].fd;
    } else {
        if (bridge_itf_table[i].fd_w) {
            hwapi_itf_unattach(bridge_itf_table[i].fd_w, FLOW_WRITE_ONLY);
            printf("replacing itf w %d\n", i);
        }
        n = hwapi_itf_attach(obj_itf_id, FLOW_WRITE_ONLY);
        if (n < 0) {
            daemon_error(daemon, "Can't attach to itf 0x%x.", obj_itf_id);
            memset(&bridge_itf_table[i], 0, sizeof (struct bridge_itf));
            return 0;
        }
        bridge_itf_table[i].fd_w = n;
    }

    bridge_itf_table[i].mode = obj_mode;
    #ifdef DEB
    daemon_info(daemon, "Table updated pos %d. Destination=0x%x:0x%x extid=0x%x objid=0x%x", i, head->ObjId, head->ItfId, ext_itf_id, obj_itf_id);
    #endif
    return 1;
}

int p=0,t=0;

int itf_rcv(int fd, int i) {
	int n;
	int c=0;
	int obj_tstamp;
#ifdef DEB_PKT
	time_t tim;
#endif

	if (i==-1) {
		do {
			n = hwapi_itf_rcv(fd, gbuff, DEF_PKT_DATA_LEN);
			if (n < 0) {
				daemon_error(daemon, "Error receiving packet from fd %d.", fd);
				return 0;
			}
			if (!n) {
				if (p > 5 && !c) {
#ifdef DEB
					p = 0;
					printf("bridge received 0 at %d %d\n", get_tstamp(), p);
#endif
				}
				break;
			}
			p++;

			c++;
			if (c > 1) {
#ifdef DEB
				printf("bridge received %d at %d\n", c, get_tstamp());
#endif
			}

			if (gbuff[0] == BRIDGE_IDENT_MAGIC) {
				new_neighbour(fd, gbuff);
				break;
			}
#ifdef DEB_PKT
			get_time(&tim);
			daemon_info(daemon, "Bridged input packet %d words at TS=%d (%d:%d)", n,
					get_tstamp(),tim.tv_sec,tim.tv_usec);
#endif
			n = packet_in(gbuff, n, 0);
		} while (n > 0);
	} else {
		/* receive from object */
		do {
			n = hwapi_itf_rcv__(fd, &gbuff[PH_HEADER_SZ],	DEF_PKT_DATA_LEN - PH_HEADER_SZ, 0, &obj_tstamp);
			if (!n) {
				break;
			}
			if (n < 0) {
				daemon_error(daemon,
						"Bridging output data packet (0x%x:0x%x %d mode %d) ",
						bridge_itf_table[i].dest.ObjId,
						bridge_itf_table[i].dest.ItfId, i,
						bridge_itf_table[i].mode);
				bridge_rmitf_i(i, FLOW_WRITE_ONLY);
				return 0;
			}

			t = 1;
#ifdef DEB_PKT
			get_time(&tim);
			daemon_info(daemon, "Bridged output packet %d words at TS=%d (%d:%d)", n,
					get_tstamp(),tim.tv_sec,tim.tv_usec);
#endif

			c++;
			if (c > 1) {
#ifdef DEB
				printf("bridge sended %d at %d\n", c, get_tstamp());
#endif
			}
			/* ... and bridge */
			n = packet_out(gbuff, n, &bridge_itf_table[i].dest,
					bridge_itf_table[i].fd_w, 1, obj_tstamp);
		} while (n > 0);
	}
	return 1;
}

int bridge_background()
{
    int n, i;
    

    for (i = 0; i < nof_in_itf; i++) {
        if (ext_in_itf[i].fd) {
            if (!itf_rcv(ext_in_itf[i].fd,-1))
            	return 0;
        }
    }

    for (i = 0; i < BRIDGE_MAX_ITF; i++) {
        if (bridge_itf_table[i].mode == FLOW_WRITE_ONLY
                && bridge_itf_table[i].fd_r
                && bridge_itf_table[i].fd_w) {
            if (!itf_rcv(bridge_itf_table[i].fd_r,i))
            	return 0;
        }
    }
    return 1;
}

