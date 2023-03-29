/*
 * daemon.h
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>.  All rights reserved.
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

#ifndef DAEMON_INCLUDED
#define DAEMON_INCLUDED

/** @defgroup daemon ALOE Daemon Tools
 * @ingroup common_phal
 *
 * This object is used to manage common daemon functionalities.
 *
 * When created, the object initializes the hwapi library
 * and creates input and output interface towards the front-end.
 *
 * User can add functions associated with packet types and
 * use a processcmd() function to select one of them when a
 * new packet (command) is received.
 *
 * Functions to send/rcv packets from the in/out main interfaces
 * are also provided.
 * @{
 */
#define T daemon_o

#define daemon_sizeof (sizeof (struct daemon_o))

typedef struct cmd {
    cmdid_t cmd;
    int (*call)(struct cmd * cmd);
    char *cmdInfo;
    char *errorInfo;
} cmd_t;

struct T {
    pkt_o pkt;
    int fd_r;
    int fd_w;
    dmid_t my_code;
    Set_o cmd_set;
    cmd_t *cur_cmd;
    char *name;
};

typedef struct T *T;

T daemon_new(dmid_t daemon_code, cmd_t *cmds);
void daemon_delete(T * daemon);
void daemon_relinquish();
int daemon_addcmd(T daemon, cmd_t *cmd);
int daemon_rcvpkt(T daemon, int blocking);
int daemon_sendto(T daemon, cmdid_t cmd,
        peid_t pe, dmid_t destdaemon);
int daemon_processcmd(T daemon);
pkt_o daemon_pkt(T daemon);
void daemon_info(T daemon, char *format, ...);
void daemon_error(T daemon, char *format, ...);
int utils_create_attach_ext(xitfid_t xitfid, int size, int *fd, int mode);

/** @} */
#undef T
#endif /*  */
