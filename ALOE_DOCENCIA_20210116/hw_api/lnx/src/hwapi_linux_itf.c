/* hwapi_linux_itf.c
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

/** pointer to internal interfaces db */
extern struct int_itf *int_itf;

extern struct file_des file_des[MAX_FILE_DES];

/** save my proc pointer */
extern struct hwapi_proc_i *my_proc;

extern struct launch_daemons *daemon_i;

extern struct itf_local_callback itf_local_callback[MAX_ITF_LOCAL_CALLBACK];


/** structs for messages */
struct msgbuff msg;


int sigusr_set = 0;



/** local declarations */
char send_parent_cmd(char cmd, char *data, int sz_data, char *result, int sz_res);




/** @defgroup itf Packet Interface Management Functions
 *
 * HW API provides the user to a set of functions to create FIFO-like packet
 * oriented data interface.
 *
 * This interfaces have a read and a write side. Once it is created, the user
 * must attach a file descriptor to the read or write side, thus, communication is possible
 * and packets written in the write side will be available in the read side.
 *
 * A name and an owner Id (object Id) can be assigned to any of the sides so separate
 * processes can share an interface easier.
 *
 * @{
 */

int hwapi_itf_addcallback(int fd, int (*callback) (int,int), int arg) {
	int i=0,j;
	int int_pos;

	if (fd<0 || fd>MAX_FILE_DES) {
		printf("Invalid fd %d\n",fd);
		return 0;
	}

	int_pos = file_des[fd].int_itf_idx;

	while(i<MAX_ITF_CALLBACK && int_itf[int_pos].itf_callback[i].pid) {
		i++;
	}
	if (i==MAX_ITF_CALLBACK) {
		printf("Error too many callbacks for interface %d\n",int_pos);
		return 0;
	}
	j=0;
	while(j<MAX_ITF_LOCAL_CALLBACK && itf_local_callback[j].fd) {
		j++;
	}
	if (j==MAX_ITF_LOCAL_CALLBACK) {
		printf("Error too many local callbacks installed\n");
		return 0;
	}
	int_itf[int_pos].itf_callback[i].pid=getpid();
	int_itf[int_pos].itf_callback[i].local_idx=j;

	itf_local_callback[j].fd=fd;
	itf_local_callback[j].callback=callback;
	itf_local_callback[j].arg=arg;
	itf_local_callback[j].itf_callback_idx=i;

	if (!int_itf[int_pos].hascallback) {
		int_itf[int_pos].hascallback=1;
	}

	return 1;
}

void itf_callback(int i) {

	if (i<0 || i>MAX_ITF_LOCAL_CALLBACK) {
		printf("Error received a signal from an invalid itf %d\n",i);
		return;
	}
	if (itf_local_callback[i].callback && itf_local_callback[i].fd) {
		itf_local_callback[i].callback(itf_local_callback[i].fd,itf_local_callback[i].arg);
	} else {
		printf("Error received a signal from an unregisterd itf %d\n",i);
		return;
	}

}

int hwapi_itf_rmcallback(int fd) {
	int j,i;
	int int_pos;

	if (fd<0 || fd>MAX_FILE_DES) {
		printf("Invalid fd %d\n",fd);
		return 0;
	}

	j=0;
	while(j<MAX_ITF_LOCAL_CALLBACK && itf_local_callback[j].fd!=fd) {
		j++;
	}
	if (j==MAX_ITF_LOCAL_CALLBACK) {
		printf("Error can't find local callback %d installed\n",fd);
		return 0;
	}

	int_pos = file_des[fd].int_itf_idx;
	memset(&int_itf[int_pos].itf_callback[itf_local_callback[j].itf_callback_idx],0,sizeof(struct itf_callback));
	memset(&itf_local_callback[j],0,sizeof(struct itf_local_callback));

	int_itf[int_pos].hascallback=0;
	for (i=0;i<MAX_ITF_CALLBACK;i++) {
		if (int_itf[int_pos].itf_callback[i].pid) {
			int_itf[int_pos].hascallback=1;
			break;
		}
	}
	return 1;
}

/** Get number of available interfaces
 *
 */
int hwapi_itf_avail()
{
    int i, n;
    n = 0;
    for (i = 0; i < MAX_INT_ITF; i++) {
        if (!int_itf[i].key_id) {
            n++;
        }
    }
    return n;
}

/** Create Interface.
 *
 * Creates a packet-oriented internal data interface.
 * If id parameter is different of 0 it will be the
 * assigned id for the interface. Otherwise it will
 * be generated.
 *
 * @param id 0 for automatic, else for fixed
 * @param size maximum packet size allowed in bytes
 *
 * @return !=0 Id of the interface
 * @return =0 error creating interface
 */
hwitf_t hwapi_itf_create(hwitf_t id, int size)
{
    int i;
    int iid;
    key_t itf_key;
    struct msqid_ds info;

    /* check if id already used */
    if (id) {
        i = 0;
        while (i < MAX_INT_ITF && int_itf[i].key_id != id) {
            i++;
        }
        if (i < MAX_INT_ITF) {
            printf("HWAPI: Error Id=0x%x already used\n", id);
            return 0;
        }
    } else {
        /* search an used one */
        do {
            id = (rand() & 0xff);
            i = 0; while (i < MAX_INT_ITF && int_itf[i].key_id != id) {
            i++;
        }
        } while (i < MAX_INT_ITF);
    }

    /* search for a free position in db */
    i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id) {
        i++;
    }
    if (i == MAX_INT_ITF) {
        printf("HWAPI: Error no more itf allowed (%d)\n", i);
        return 0;
    }

    iid = (int) id;
    itf_key = ftok(LOCK_FILE, iid);
    int_itf[i].key_id = id;

    if ((int_itf[i].id = msgget(itf_key, IPC_CREAT | IPC_EXCL | 0666))
            == -1) {
        printf("HWAPI: Message queue with key_id 0x%x could not be created\n", id);
        perror("msgget");
        memset(&int_itf[i], 0, sizeof (struct int_itf));
        return 0;
    }

    msgctl(int_itf[i].id, IPC_STAT, &info);
    info.msg_qbytes = MSG_QBYTES;
    if (msgctl(int_itf[i].id, IPC_SET, &info) == -1) {
        perror("msgctl");
        return 0;
    }

    /* return id */
    return id;
}

/** Set interface delay
 *
 * Set a delay of N time-slots
 * @param id Id of the interface
 * @param nof_tslots Number of time-slots to delay
 */
int hwapi_itf_setdelay(hwitf_t id, int nof_tslots)
{

    int i;

    i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id != id) {
        i++;
    }
    if (i == MAX_INT_ITF) {
        printf("HWAPI: Error can't find itf id=0x%x.\n", id);
        return 0;
    }
    int_itf[i].delay = nof_tslots;
}


int hwapi_itf_delete_own(char *obj_name)
{
    int i;

    for (i = 0; i < MAX_INT_ITF; i++) {

        if (!strncmp(int_itf[i].w_obj, obj_name, SYSTEM_STR_MAX_LEN)
                || !strncmp(int_itf[i].r_obj, obj_name, SYSTEM_STR_MAX_LEN)) {
            hwapi_itf_delete(int_itf[i].key_id);
        }
    }

    return 1;
}

void hwapi_itf_setexternal(hwitf_t id)
{
    int i;

    i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id != id) {
        i++;
    }
    if (i == MAX_INT_ITF) {
        #ifdef DEBUG_ITF
        printf("HWAPI: Error can't find itf id=0x%x.\n", id);
        #endif
    }
    int_itf[i].opts |= ITF_OPT_EXTERNAL;
}

/** Get if an interface is external or not
 * @param id for the interface
 */
int hwapi_itf_isexternal(hwitf_t id)
{
    int i;

    i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id != id) {
        i++;
    }
    if (i == MAX_INT_ITF) {
        #ifdef DEBUG_ITF
        printf("HWAPI: Error can't find itf id=0x%x.\n", id);
        #endif
        return 0;
    }
    return int_itf[i].opts & ITF_OPT_EXTERNAL;
}

/** Delete Interface.
 *
 * Deletes internal data interface with id='id' (and clears resources)
 *
 * @param id Id of the interface to delete
 *
 * @return 1  Deleted successfully
 * @return 0 Error deleting interface
 */
int hwapi_itf_delete(hwitf_t id)
{
    int i;

    i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id != id) {
        i++;
    }
    if (i == MAX_INT_ITF) {
        #ifdef DEBUG_ITF
        printf("HWAPI: Error can't find itf id=0x%x.\n", id);
        #endif
        return 0;
    }

    if (msgctl(int_itf[i].id, IPC_RMID, NULL) == -1) {
#ifdef DEBUG_ITF
        printf("HWAPI: Error removing interface 0x%x\n", int_itf[i].id);
        perror("msgctl");
#endif
        return -1;
    }

    /* clear interface resources */
    memset(&int_itf[i], 0, sizeof (struct int_itf));

    return 1;
}

/** Attach to an Interface.
 *
 * Creates a file descriptor to use the interface. This function
 * should be called 'after' creating the interface and 'before' using it.
 *
 * It must be attached in FLOW_READ_ONLY (for reading) or FLOW_WRITE_ONLY
 * mode (for writting). A file descriptor will be returned which can be used
 * to read/write packets
 *
 * @param id Id of the interface created with hwapi_create_itf()
 * @param mode FLOW_READ_ONLY or FLOW_WRITE_ONLY constants
 *
 * @return >0 File descriptor
 * @return =-2 Interface not available
 * @return =-1 Error attaching interface
 */
int hwapi_itf_attach(hwitf_t id, int mode)
{
    int i, j;
    int iid;
    key_t itf_key;

    /* first of all, search itf in interfaces database */
    i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id != id) {
        i++;
    }
    if (i == MAX_INT_ITF) {
        return -2;
    }

    /* then search a free position in file descriptors database */
    j = 3; /* first 2 are reserved */
    while (j < MAX_FILE_DES && file_des[j].mode) {
        j++;
    }
    if (j == MAX_FILE_DES) {
        printf("HWAPI: Error no more local itf allowed (%d).\n", j);
        return -1;
    }

    /* now fill local db */
    iid = (int) id;
    itf_key = ftok(LOCK_FILE, iid);
    if ((file_des[j].id = msgget(itf_key, 0)) == -1) {
        printf("HWAPI: Error attaching to itf 0x%x\n", id);
        perror("msgget");
        return -1;
    }
    file_des[j].mode = mode;
    file_des[j].int_itf_idx = i;

    return j;
}

/** Unattach from an Interface.
 *
 * Un-attaches a file descriptor from the (previously attached)
 * interface in mode 'mode'
 *
 * @param fd file descriptor of the interface
 * @param mode FLOW_READ_ONLY or FLOW_WRITE_ONLY constants
 *
 * @return >0 Successfully unattached
 * @return <0 Error unattaching
 */
int hwapi_itf_unattach(int fd, int mode)
{

    /* check correct fd */
    if (fd < 0 || fd > MAX_FILE_DES) {
        printf("HWAPI: Error invalid local fd %d.\n", fd);
        return -1;
    }

    /* clear resources */
    memset(&file_des[fd], 0, sizeof (struct file_des));

    return 1;
}

/** Set interface to blocking
 * @param itf_id
 * @return 1 if ok, 0 if error
 */
int hwapi_itf_setblocking(hwitf_t itf_id)
{
    int i;

    i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id != itf_id) {
        i++;
    }
    if (i == MAX_INT_ITF) {
        return 0;
    }

    int_itf[i].opts |= ITF_OPT_BLOCKING;

    return 1;

}
int msgsndcallback(int msgid, void *buffer, int size , int flags, int int_pos) {
	int n;
	int i;
	union sigval sival;

	n = msgsnd(msgid, buffer, size, flags);
	if (n==-1)
		return -1;


	if (hw_api_db->int_itf[int_pos].hascallback) {
		//printf("---- %d is sending to pos %d %d\n",getpid(),int_pos);
		for (i=0;i<MAX_ITF_CALLBACK;i++) {
			if (hw_api_db->int_itf[int_pos].itf_callback[i].pid) {
				sival.sival_int=((hw_api_db->int_itf[int_pos].itf_callback[i].local_idx<<4)&0xfffffff0);
				sival.sival_int|=PROCCMD_ITFCALLBACK & 0xf;
				sigqueue(hw_api_db->int_itf[int_pos].itf_callback[i].pid,SIG_PROC_CMD,sival);
			}
		}
	} else {
		//printf("++++ %d has not callback at pos %d\n",getpid(),int_pos);
	}
	return n;
}
/** Send a packet.
 *
 * Sends a packet of data through a data interface. The user needs
 * to provide a file descriptor, get after calling the attaching function.
 *
 * @param fd file descriptor
 * @param buffer pointer to data to send
 * @param size size of the packet in bytes (8-bit)
 *
 * @return >0 Successfully sended. Returns amount of bytes sended.
 * @return =0 Packet could not be send (full queue)
 * @return <0 Error sending packet
 */
int hwapi_itf_snd_(int fd, void *buffer, int size, int obj_tstamp)
{
    int n;
    int tstamp;
    int id;
    int delay = 0;
    int flags;
    int int_pos;
    struct msqid_ds msgstatus;

    assert(buffer);
    assert(size >= 0);

    /* check for valid fd */
    if (fd < 0 || fd > MAX_FILE_DES) {
        printf("HWAPI: Error invalid fd=%d. 0<=fd<=%d.\n", fd,
                MAX_FILE_DES);
        return -1;
    }

    /* check if fd is write enabled */
    if (!(file_des[fd].mode & FLOW_WRITE_ONLY)) {
        printf
                ("HWAPI: Error flow is not write enable (fd=%d id=0x%x mode=%d pid=%d)\n",
                fd, file_des[fd].id, file_des[fd].mode, getpid());
        return -1;
    }

    /* get interface id */
    id = file_des[fd].id;

    /* get interface position */
    int_pos = file_des[fd].int_itf_idx;

    /* check if size exceeds internal limit */
    if (size > MSG_BUFF_SZ) {
        printf("HWAPI: Error packet too long (%d/%d)\n", size, MSG_BUFF_SZ);
        return -1;
    }

    if (my_proc) {
        delay = int_itf[int_pos].delay;
        if (!delay) {
            delay=hw_api_db->default_delay;
        }
    } else {
        delay=0;
    }

    /* calculate packet time stamp and destination */
    if (delay) {
        /* write future tstamp. Packet is only available on next tstamp */
        tstamp = my_proc->obj_tstamp;
        msg.head.dst = (long) (tstamp + delay);
        if (int_itf[int_pos].opts&ITF_OPT_EXTERNAL) {
        	msg.head.dst += hw_api_db->external_delay;
        }
    } else {
        msg.head.dst = 1;
    }

    /* destination timestamp can be forced by obj_tstamp parameter if !=-1 */
    if (obj_tstamp>0) {
    	msg.head.dst=(long) obj_tstamp;
    }

    if (int_itf[int_pos].opts&ITF_OPT_BLOCKING) {
        flags = 0;
    } else {
        flags = IPC_NOWAIT;
    }

    /* fill message header */
    msg.head.src = 0;
    msg.head.opts = int_itf[int_pos].opts;
    msg.head.tstamp = tstamp + delay;
    memcpy(msg.body, buffer, size);

    flags = IPC_NOWAIT;
    n = msgsndcallback(id, &msg, size + MSG_HEAD_SZ, flags,int_pos);

    if (n == -1 && errno == EAGAIN) {
        /* full queue, print only for objects interfaces */
        if (my_proc) {
            msgctl(id, IPC_STAT, &msgstatus);
            time_t t;
            get_time(&t);
            printf("\nCAUTION: Full msg queue at %d:%d (ts=%d,objts=%d), caller object %s. Current status: %d/%d bytes, %d msg\n\n",
                    (int) t.tv_sec, (int) t.tv_usec, get_tstamp(), (int) my_proc->obj_tstamp,
                    my_proc->obj_name, (int) msgstatus.msg_cbytes, (int) msgstatus.msg_qbytes,
                    (int) msgstatus.msg_qnum);
        }
        return 0;
    } else if (n == -1) {
        printf("HWAPI: Error while sending through itf id %d key 0x%x at pos %d type %d size %d pid %d errno %d\n", id, int_itf[int_pos].key_id, int_pos, (long) msg.head.dst, size, getpid(),errno);
        perror("msgsnd");
        return -1;
    }
    /* success */

    if (my_proc && hw_api_db->cpu_info.print_itf_pkt) {
        time_t t;
        get_time(&t);
        printf("------- %s:\tsnd packet size %d at ts=%d dst_ts=%d (real ts=%d) time %d:%d dst %d\n", my_proc->obj_name,
                size, my_proc->obj_tstamp,msg.head.tstamp,get_tstamp(),t.tv_sec,t.tv_usec,msg.head.dst);
    }

    return size;
}
int hwapi_itf_snd(int fd, void *buffer, int size) {
	return hwapi_itf_snd_(fd,buffer,size,-1);
}
int hwapi_itf_status_id(hwitf_t id)
{
    struct msqid_ds msgstatus;
    int i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id != id)
        i++;
    if (i == MAX_INT_ITF) {
        #ifdef DEBUG_ITF
        printf("HWAPI: Itf %d not found\n", id);
        #endif
        return -1;
    }
    if (!msgctl(int_itf[i].id, IPC_STAT, &msgstatus)) {
        return (int) msgstatus.msg_cbytes-msgstatus.msg_qbytes*MSG_HEAD_SZ;
    } else {
        return -1;
    }
}

int hwapi_itf_status(int fd)
{
    struct msqid_ds info;

    /* check valid file descriptor */
    if (fd < 0 || fd > MAX_FILE_DES) {
        printf("HWAPI: Error invalid fd=%d. 0<=fd<=%d.\n", fd,
                MAX_FILE_DES);
        return -1;
    }

    if (!msgctl(file_des[fd].id, IPC_STAT, &info)) {
        return (int) info.msg_qnum;
    } else {
        return 0;
    }
}

/** Receive a packet.
 *
 * Recevies a packet through a file descriptor. User must
 * indicate provided buffer's length (in bytes).
 *
 * @param fd file descriptor
 * @param buffer pointer to buffer where receive data
 * @param size size of the buffer in bytes (8-bit)
 *
 * @return >0 Successfully received. Returns amount of bytes.
 * @return =0 No packets available (empty queue)
 * @return <0 Error receiving packet
 */

int hwapi_itf_rcv__(int fd, void *buffer, int size, int blocking, int *obj_tstamp)
{
    int n;
    int type;
    int tstamp;
    int c;
    int id;
    int delay;
    int int_pos;
    int flags;

    assert(buffer);
    assert(size >= 0);

    /* check valid file descriptor */
    if (fd < 0 || fd > MAX_FILE_DES) {
        printf("HWAPI: Error invalid fd=%d. 0<=fd<=%d.\n", fd,
                MAX_FILE_DES);
        return -1;
    }

    /* check if reading enabled */
    if (!(file_des[fd].mode & FLOW_READ_ONLY)) {
        printf
                ("HWAPI: Error flow is not read enable (fd=%d id=0x%x mode=%d pid=%d)\n",
                fd, file_des[fd].id, file_des[fd].mode, getpid());

        return -1;
    }

    /* get interface id */
    id = file_des[fd].id;

    /* get interface db position */
    int_pos = file_des[fd].int_itf_idx;

    /* obtain current timestamp */
    if (my_proc)
    	tstamp = my_proc->obj_tstamp;
    else
    	tstamp = get_tstamp();

    if ((int_itf[int_pos].opts&ITF_OPT_BLOCKING) || blocking) {
        flags = 0;
    } else {
        flags = IPC_NOWAIT;
    }


    if (!my_proc)
      type=0;
    else
      type = -tstamp;

    n = msgrcv(id, &msg, MSG_SZ, type, flags);

    /* any message available */
    if (n == -1 && errno == ENOMSG) {
        return 0;
    }

    if (n==-1 && errno==EINTR && blocking) {
    	return 0;
    }

    /* check error */
    if (n < 0) {
        printf("HWAPI: Error reading itf id 0x%x key 0x%x at pos %d (%s) size %d\n", id, int_itf[int_pos].key_id,
                                int_pos, daemon_i ? daemon_i->path : my_proc->obj_name,MSG_SZ);
        perror("msgrcv");
        return n;
    }

    if (n<MSG_HEAD_SZ) {
        return 0;
    }

    n-=(MSG_HEAD_SZ);

    /* check packet lenght does not exceed user buffer length */
    if (n > size) {
        n = size;
    }

    /* copy packet to user buffer */
    memcpy(buffer, msg.body, n);

    /* and save original object time-stamp */
    if (obj_tstamp) {
    	*obj_tstamp=msg.head.tstamp;
    }

    if (my_proc && hw_api_db->cpu_info.print_itf_pkt) {
        time_t t;
        get_time(&t);
        printf("+++++++ %s:\treceived packet size %d at ts=%d (real ts=%d) time %d:%d dst %d\n", my_proc->obj_name,
                n, my_proc->obj_tstamp,get_tstamp(), t.tv_sec,t.tv_usec,msg.head.dst);
    }

    return n;
}
int hwapi_itf_rcv(int fd, void *buffer, int size)
{
	return hwapi_itf_rcv__(fd, buffer, size, 0, NULL);
}
int hwapi_itf_rcv_(int fd, void *buffer, int size, int blocking)
{
	return hwapi_itf_rcv__(fd, buffer, size, blocking, NULL);
}

/** Link Physical Interface to a logical Interface
 *
 *
 *
 * Links a physical interface (external) to an internal packet
 * oriented interface. After the link is successfully realized,
 * all packets written to the internal interface will be bridged
 * (by RUNPH_NET) towards the physical interface. Conversely, all
 * packets received to the phy. itf. will be bridged to the internal
 * interface.
 *
 * Interface must be created before calling this function. Then, in
 * order to use it, it must be attached.
 *
 * @param itf_id Local interface Id
 * @param phy_itf_id External Physical Interface Id
 * @param mode FLOW_READ_ONLY or FLOW_WRITE_ONLY constant
 *
 * @return >0 Successfully linked
 * @return <0 Error linking
 */
int hwapi_itf_link_phy(hwitf_t itf_id, xitfid_t phy_itf_id, char mode)
{
    int n, i;
    char cmd[3];

    /* check correct params */
    if (!itf_id) {
        printf("HWAPI: Error invalid parameter itf_id must be != 0\n");
        return -1;
    }
    if (!phy_itf_id) {
        printf("HWAPI: Error invalid parameter xitf_id must be !=0\n");
        return -1;
    }

    /* search internal interface */
    i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id != itf_id) {
        i++;
    }
    if (i == MAX_INT_ITF) {
        printf("HWAPI: Can't find itf id 0x%x.\n", itf_id);
        return -1;
    }
    if (i>127) {
    	printf("FIXME! phy interfaces must be the first to attach");
    }

    /* send message to RUNPH process */
    cmd[0] = phy_itf_id;
    cmd[1] = itf_id;
    cmd[2] = mode;
    cmd[3] = i;
    n = send_parent_cmd(HWD_SETUP_XITF, cmd, 4, NULL, 0);
    if (n <= 0) {
        /* error */
        printf("HWAPI: Error while setting xitf (%d)\n", n);
    }

    int_itf[i].opts |= ITF_OPT_EXTERNAL;

    return n;
}

/** Match Interface.
 *
 * Assign an object id and interface name for the read and write side
 *
 * @return 1 if ok 0 if error
 */
int hwapi_itf_match(hwitf_t itf_id, char *w_obj, char *w_itf, char *r_obj,
        char *r_itf)
{
    assert(w_itf && w_obj && r_itf && r_obj);
    int i;

    i = 0;
    while (i < MAX_INT_ITF && int_itf[i].key_id != itf_id) {
        i++;
    }
    if (i == MAX_INT_ITF) {
        printf("HWAPI: Interface 0x%x not found\n", itf_id);
        return 0;
    }
    strncpy(int_itf[i].w_obj, w_obj, SYSTEM_STR_MAX_LEN);
    strncpy(int_itf[i].w_itf, w_itf, SYSTEM_STR_MAX_LEN);
    strncpy(int_itf[i].r_obj, r_obj, SYSTEM_STR_MAX_LEN);
    strncpy(int_itf[i].r_itf, r_itf, SYSTEM_STR_MAX_LEN);

    return 1;
}

int hwapi_itf_list(char *w_obj, hwitf_t *ids, int max_itf)
{
    int i, k;

    k = 0;
    for (i = 0; i < MAX_INT_ITF && k < max_itf; i++) {
        if (!strncmp(int_itf[i].w_obj, w_obj, SYSTEM_STR_MAX_LEN)) {

            ids[k++] = int_itf[i].key_id;
        }
    }
    return k;
}

/** Get itf id given an itf name and object name.
 */
hwitf_t hwapi_itf_find(char *obj_name, char *itf_name, char mode)
{
    int i;

    /* first of all, search itf in interfaces database */
    i = MAX_INT_ITF;
    if (mode & FLOW_WRITE_ONLY) {
        i = 0;
        while (i < MAX_INT_ITF
                && (strncmp(itf_name, int_itf[i].w_itf, SYSTEM_STR_MAX_LEN)
                || strncmp(obj_name, int_itf[i].w_obj, SYSTEM_STR_MAX_LEN))) {
            i++;
        }
    }
    if ((mode & FLOW_READ_ONLY) && (i == MAX_INT_ITF)) {
        i = 0;
        while (i < MAX_INT_ITF
                && (strncmp(itf_name, int_itf[i].r_itf, SYSTEM_STR_MAX_LEN)
                || strncmp(obj_name, int_itf[i].r_obj, SYSTEM_STR_MAX_LEN))) {
            i++;
        }
    }
    if (i == MAX_INT_ITF) {
        #ifdef DEBUG_ITF
        printf("HWAPI: notfound itf %d %s:%s r/w\n", mode, itf_name, obj_name);
        #endif
        return 0;
    }
    return int_itf[i].key_id;
}


/**********************************************************

 int send_parent_cmd(int cmd, int *data, int size);

 Internal function, send a command to parent process (runph)

 **********************************************************/
char send_parent_cmd(char cmd, char *data, int sz_data, char *result, int sz_res)
{
    int n;

    /* clear msb bit */
    msg.head.dst = MSG_MAGIC;
    msg.head.src = getpid();

    /* check size */
    if (sz_data < 0 || sz_data > MSG_SZ) {
        printf("HWAPI: Error with size %d\n", sz_data);
        return -1;
    }

    msg.body[0] = cmd;

    /* copy body */
    memcpy(&msg.body[1], data, sz_data);

    /* send message */
    n = msgsnd(hw_api_db->api_msg_id, &msg, sz_data + MSG_HEAD_SZ + 1, 0);
    if (n == -1) {
        perror("msgsnd");
        return -1;
    }

    /* return message type is my pid */
    n = msgrcv(hw_api_db->api_msg_id, &msg, MSG_SZ, getpid(), 0);
    if (n == -1) {
        perror("Receiving message from parent");
        return -1;
    }

    /* copy result */
    memcpy(result, &msg.body[1], sz_res);

    /* return value is first word */
    return msg.body[0];
}

/** @} */
