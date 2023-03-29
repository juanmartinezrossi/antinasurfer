/*
 * daemon.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "phal_hw_api.h"

#include "phal_daemons.h"
#include "phid.h"

#include "set.h"
#include "pkt.h"
#include "daemon.h"

#include "frontend_cmds.h"

#define T daemon_o

#define DEFAULT_ITF_SZ	2048
#define WAIT_FE_SEC	20	/*AGB 20*/

#define PKT_BUFF_SZ	(1024*128)
static char pkt_buff[PKT_BUFF_SZ];


char *daemon_names[] = {"", "FE", "CMDMAN", "STATS", "STMAN", "HWMAN", "SWMAN", "SWLOAD", "EXEC", "BRIDGE"};

#define ERROR_LINE_LEN	1024
#define ERROR_DESC_LEN	700
char errorLine[ERROR_LINE_LEN];
char errorDesc[ERROR_DESC_LEN];

enum printType {
    Error, Info
};


int utils_attach_itf_wait(hwitf_t id, int mode, int timeout_sec);
int utils_create_attach_itf(hwitf_t id, int mode, int size);
void daemon_printf(T daemon, enum printType type, char *format, va_list args);

void cmd_delete(void **x)
{
    assert(x && *x);

    DELETE(*x);
    x = NULL;
}

int cmd_findtype(const void * x, const void *member)
{
    struct cmd *c = (struct cmd *) member;
    assert(x && member);

    return c->cmd != *((cmdid_t *) x);
}

/** Creates a new daemon object
 *
 * Create the daemon object, initializes hwapi and creates input and output
 * interfaces to FRONT-END daemon.
 *
 * @param daemon_code Code for the caller's daemon (see phal_daemons.h)
 *
 * @returns !=0 daemon object
 * @returns NULL if error
 */
T
daemon_new(dmid_t daemon_code, cmd_t *cmds)
{
    T daemon = NULL;
    int fd_r, fd_w;
    hwitf_t input_itf_id;
    struct header *head = (struct header *) pkt_buff;
    int i;

    assert(daemon_code);

    NEW(daemon);
    if (!daemon) {
        printf("Error allocating memory\n");
        return NULL;
    }

    input_itf_id = daemon_code;
    fd_r = utils_create_attach_itf(input_itf_id, FLOW_READ_ONLY, DEFAULT_ITF_SZ);
    if (fd_r < 0) {
        printf("Error creating itf\n");
        daemon_delete(&daemon);
        return NULL;
    }

    fd_w = utils_attach_itf_wait(DAEMON_FRONTEND, FLOW_WRITE_ONLY, WAIT_FE_SEC);
    if (fd_w < 0) {
        xprintf("Timeout while waiting to attach to front-end\n");
        daemon_delete(&daemon);
        return NULL;
    }

    /* send a packet to front-end indicating our input interface and daemon code */
    memset(pkt_buff, 0, HEAD_SZ + 2);
    head->cmd = FRONTEND_CFGITF;
    head->dst_daemon = DAEMON_FRONTEND;
    pkt_buff[HEAD_SZ] = daemon_code;
    pkt_buff[HEAD_SZ + 1] = input_itf_id;
    if (hwapi_itf_snd(fd_w, pkt_buff, 2 + HEAD_SZ) < 0) {
        xprintf("Error sending through interface.\n");
        return NULL;
    }

    daemon->fd_r = fd_r;
    daemon->fd_w = fd_w;
    daemon->my_code = daemon_code;
    daemon->pkt = pkt_new((int*) pkt_buff, PKT_BUFF_SZ);
    daemon->cmd_set = Set_new(0, NULL);
    if (!daemon->pkt || !daemon->cmd_set) {
        printf("Error creating packet\n");
        daemon_delete(&daemon);
        return NULL;
    }

    daemon->name = daemon_names[daemon_code];

    if (cmds) {
        i = 0;
        while (cmds[i].call) {
            daemon_addcmd(daemon, &cmds[i]);
            i++;
        }
    }
    return daemon;
}

int daemon_cmdfd(T daemon) {
	assert(daemon);
	return daemon->fd_r;
}

void daemon_delete(T * daemon)
{
    assert(daemon);
    assert(*daemon);

    if ((*daemon)->cmd_set) {
        Set_destroy(&(*daemon)->cmd_set, cmd_delete);
    }

    if ((*daemon)->pkt) {
        pkt_delete(&(*daemon)->pkt);
    }

    DELETE(*daemon);
    daemon = NULL;
}

pkt_o daemon_pkt(T daemon)
{
    assert(daemon);

    return daemon->pkt;
}

/** Add a command function to the selector
 *
 * This function adds a command to daemon object and associates it with a callback
 * function which will be called when processcmd() function is called and
 * the associated command type has been received.
 *
 * @param daemon Daemon object
 * @param type command type
 * @param call callback function to the processing function for this command
 *		type
 *
 * @returns 1 if ok
 * @returns 0 if error
 */
int daemon_addcmd(T daemon, cmd_t *cmd)
{
    assert(daemon && cmd);

    Set_put(daemon->cmd_set, cmd);

    return 1;
}

/** Receive packet
 *
 * This function is used to receive a packet through the input interface, created
 * during the creation of the daemon object.
 *
 * @param daemon Daemon object
 *
 * @returns >0 length of the packet received, if any
 * @returns 0 if no packet available
 * @returns -1 if error reading packet
 */
int daemon_rcvpkt(T daemon, int blocking)
{
    int n;

    assert(daemon);

    n = hwapi_itf_rcv_(daemon->fd_r, pkt_buff, PKT_BUFF_SZ, blocking);
    if (n < 0) {
        return -1;
    } else if (!n) {
        return 0;
    }

    if (!pkt_readvalues(daemon->pkt, n)) {
        return 0;
    }
    return n;
}

/** Send Packet to a daemon and pe
 *
 * This function sends daemon packet (accessed through daemon_pkt() function)
 * through the output interface towards FRONT-END.
 * Note that input and output buffers are the same, so, pkt_clear() function should
 * be called if we desire to erase its contents before sending it.
 *
 * It overwrittes type, destpe and destdaemon fields with pe & destdaemon params.
 *
 * @param daemon daemon object
 * @param type command type
 * @param pe destination Processor Id
 * @param destdaemon destination daemon code
 *
 * @returns same as sndpkt
 */
int daemon_sendto(T daemon, cmdid_t cmd, peid_t pe, dmid_t destdaemon)
{
    int n;

    assert(daemon && cmd);

    pkt_setcmd(daemon->pkt, cmd);
    pkt_setdestpe(daemon->pkt, pe);
    pkt_setdestdaemon(daemon->pkt, destdaemon);

    do {
        n = hwapi_itf_snd(daemon->fd_w, pkt_buff,pkt_len(daemon->pkt));
        if (!n) {
            hwapi_relinquish_daemon();
        }
    } while (!n);

    return n;
}

/** Process command
 *
 * This function shall be called after daemon_rcvpkt() has returned something >0.
 * In that case, the callback selector will pick the function associated to the
 * received packet command type and call it.
 *
 * @param daemon daemon object
 *
 * @returns caller return code
 * @return -1 if command not found
 */
int daemon_processcmd(T daemon)
{
    struct cmd *pcmd;
    cmdid_t c;

    assert(daemon);

    if (pkt_len(daemon->pkt)) {
        c = pkt_getcmd(daemon->pkt);

        pcmd = (cmd_t*) Set_find(daemon->cmd_set, &c,
                cmd_findtype);
        if (!pcmd) {
            return -1;
        }
        assert(pcmd->call);
        daemon->cur_cmd = pcmd;
        (pcmd->call)(pcmd);
        daemon->cur_cmd = NULL;
    } else {
        return -1;
    }
    return 1;
}

cmd_t * daemon_cmd(T daemon)
{
    return daemon->cur_cmd;
}

void daemon_error(T daemon, char *format, ...)
{
    va_list args;
    va_start(args, format);

    daemon_printf(daemon, Error, format, args);

    va_end(args);
}

void daemon_info(T daemon, char *format, ...)
{
    va_list args;
    va_start(args, format);

    daemon_printf(daemon, Info, format, args);

    va_end(args);
}

void daemon_printf(T daemon, enum printType type, char *format, va_list args)
{
    char *info = NULL;
    assert(daemon);

    switch (type) {
    case Error:
        if (daemon->cur_cmd) {
            info = daemon->cur_cmd->errorInfo;
        }
        break;
    case Info:
        if (daemon->cur_cmd) {
            info = daemon->cur_cmd->cmdInfo;
        }
        break;
    }
    vsnprintf(errorDesc, ERROR_DESC_LEN, format, args);
    if (!info) {
        printf("%s:\t%s\n",
                daemon->name ? daemon->name : "Unknown",
                errorDesc);
    } else {
        printf("%s:\t%s%s %s\n",
                daemon->name ? daemon->name : "Unknown",
                info, ":" , errorDesc);
    }
}

/**  Attaches to an interface.
 * In the case it has not been yet created (returns 0),
 * it waits for "timeout_sec" seconds.
 *
 * @param id id of the interface to attach
 * @param mode read/write
 * @param timeout_sec number of seconds to wait
 *
 * @return >0 if successfully, returns file descriptor
 * @return <0 if failed
 */
int utils_attach_itf_wait(hwitf_t id, int mode, int timeout_sec)
{
    int n;
    time_t tdata[3];

    gettimeofday(&tdata[1],NULL);
    do {
        n = hwapi_itf_attach(id, mode);
        if (n == -1) {
            return -1;
        } else if (n == -2) {
            hwapi_relinquish_daemon();
            gettimeofday(&tdata[2],NULL);
            get_time_interval(tdata);
        }
    } while ((n == -2) && tdata[0].tv_sec < timeout_sec);

    if (n < 1) {
        return -1;
    } else {
        return n;
    }
}

/** Creates an interface and attaches one side (depending on mode)
 *
 * @param id id of the interface to create
 * @param mode mode to attach the file descriptor
 * @param size size of the interface to create
 *
 * @return >0 if successfully, returns file descriptor
 * @return <0 if failed
 */
int utils_create_attach_itf(hwitf_t id, int mode, int size)
{
    id = hwapi_itf_create(id, size);
    if (!id) {
        printf("Error creating itf\n");
        return -1;
    }
    return hwapi_itf_attach(id, mode);
}

int utils_create_attach_ext_id(xitfid_t xitfid, int size, int *fd, char mode, hwitf_t *id)
{
    hwitf_t n;

    n = hwapi_itf_create(0, size);
    if (!n) {
        xprintf("Error creating itf\n");
        return 0;
    }
    if (id)
        *id = n;

    if (hwapi_itf_link_phy(n, xitfid, mode) < 0) {
        xprintf("Error linking with external itf\n");
        return 0;
    }
    if (fd) {
        *fd = hwapi_itf_attach(n, mode);
        if (*fd < 0) {
            xprintf("Error attaching itf\n");
            return 0;
        }
    }
    return 1;
}

int utils_create_attach_ext(xitfid_t xitfid, int size, int *fd, int mode)
{
    return utils_create_attach_ext_id(xitfid, size, fd, mode, NULL);
}

int utils_create_attach_ext_bi(xitfid_t xitfid, int size, int *fd_w, int *fd_r)
{
    assert(xitfid && fd_w && fd_r);

    if (!utils_create_attach_ext(xitfid, size, fd_r, FLOW_READ_ONLY)) {
        return 0;
    }

    if (!utils_create_attach_ext(xitfid, size, fd_w, FLOW_WRITE_ONLY)) {
        return 0;
    }

    return 1;
}

