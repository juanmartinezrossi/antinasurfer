/*
 * hwapi_netd.c
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
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <time.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sched.h>
  #include <netinet/ip.h>

#include "phal_hw_api.h"
#include "hwapi_backd.h"
#include "hwapi_netd.h"

#include "net_utils.h"



/*#define CONN_DEBUG
#define PKT_DEBUG
*/
struct msgbuff msg_rcv, msg_snd;

int data_msg_id;

struct hw_api_db *hwapi_db;
struct net_db *net_db;


/** @defgroup launcher_net ALOE Launcher for Linux Network Bridge Functions
 *
 * This group of functions are called by runph.c main function at boot and act like
 * bridge between logical and external physical interfaces.
 *
 * @{
 */

/** Network Send Packet
 *
 * Checks if a packet is available in logical interface and sends it throught an 
 * output interface. The external ALOE header is attached at the beggining so other
 * platforms understand it.
 * 
 * @param fd File descriptor to the external socket
 * @param xitf external interface structure
 *
 * @return >0 number of bytes sent
 * @return 0 no packets available
 * @return <0 error 
 */
int net_packet_snd(int fd, struct ext_itf *xitf)
{
    int n, l;
    int tstamp;
    long type;
    packet_header_t *pkth;
    char *buffer;
    int x;
    int mbox_id;
    int clientLen=sizeof(struct sockaddr);
    mbox_id=xitf->out_mbox_id;

#ifdef PKT_DEBUG
    time_t tdata;
#endif

    tstamp = get_tstamp();
    type = 0;

    if (xitf->type==NET_UDP && xitf->mode==FLOW_READ_WRITE
    		&& !xitf->remote_udp_connected) {
    	while(!xitf->remote_udp_connected) {
    		sleep_ms(10);
    	}
    }


#ifdef PKT_DEBUG2
    printf("HWAPI_NETD: --SND-- waiting fd=%d mbox=%d...\n", fd, mbox_id);
#endif

    n = msgrcv(mbox_id, &msg_snd, MSG_SZ, type, 0);
    if (n == -1 && errno == ENOMSG) {
        return 0;
    }

    if (n == -1) {
        perror("Receiving from network interface");
        return n;
    }
    if (n == 0) {
        printf("RUNPH: CAUTION! Void packet received\n");
        return 0;
    }
    n = n - MSG_HEAD_SZ;

    buffer = malloc(n + sizeof (packet_header_t));
    assert(buffer);
    
    pkth = (packet_header_t *) buffer;
    if (!net_db->full_bridge) {
        pkth->id = PACKET_MAGIC;
        pkth->opts = msg_snd.head.opts;
        pkth->tstamp = msg_snd.head.dst;
        pkth->size = n;
        x = sizeof (packet_header_t);
    } else {
        x = 0;
    }

    memcpy(&buffer[x], msg_snd.body, n);
   
#ifdef PKT_DEBHEAD
    printf("RUNPH: SNDHEAD: id=0x%x,opts=0x%x,ts=0x%x,sz=0x%x,n=%d\n", pkth->id,
            pkth->opts, pkth->tstamp, pkth->size, n);
#endif
#ifdef PKT_DEBCONT
    int i;
    unsigned int *data = (unsigned int*) &buffer[x];
    for (i=0;i<(pkth->size+sizeof(packet_header_t))>>2;i++)
        xprintf("0x%x-", data[i]);
    printf("\n");
#endif

#ifdef PKT_DEBUG
    get_time(&tdata);
    if (pkth->size>10)
    printf
            ("HWAPI_NETD: -- SND -- TS=%d (t=%d:%d) type=0x%x size=%d opts=0x%x fd %d\n",
            get_tstamp(), tdata.tv_sec, tdata.tv_usec, type, pkth->size,
            pkth->opts,fd);
#endif
    l = n + x;
    n=0;

    if (xitf->type==NET_UDP && xitf->mode==FLOW_READ_WRITE) {
        do {
            n=sendto(fd,buffer,l,0,&xitf->remote_udp_addr,clientLen);
        } while (n == -1 && errno == EINTR);
        if (n == -1) {
            perror("sndto");
            return -1;
        }
    } else {
        do {
            n=write(fd,buffer,l);
        } while (n == -1 && errno == EINTR);
        if (n == -1) {
            perror("write");
            return -1;
        }
    }

    if (n != l) {
        printf("SND ALARM: (%d!=%d)\n", n, l);
    }

    free(buffer);


    return n;
}

/** Network Receive Packet
 *
 * Checks if a packet is available in external physical interface and bridges it to the
 * internal interfcae.  
 * 
 * @param fd File descriptor of the external socket
 * @param mbox Mailbox of the internal interface
 *
 * @return >0 number of bytes received
 * @return 0 no packets available
 * @return <0 error 
 */
int net_packet_rcv_tcp(int fd, struct ext_itf *xitf)
{
    packet_header_t pkth;
    int n, rpm;
    int size;
    char *bptr;

    int mbox_id=xitf->in_mbox_id;
    hwitf_t key_id=xitf->in_key_id;
    int int_itf_idx=xitf->int_itf_idx;

#ifdef PKT_DEBUG
    time_t tdata;
#endif

#ifdef PKT_DEBUG2
    printf("HWAPI_NETD: --RCV-- Waiting fd=%d,mbox=%d\n", fd, mbox_id);
#endif

    bptr = (char *) & pkth;
    rpm = 0;
    do {
        n = read(fd, &bptr[rpm], sizeof (packet_header_t) - rpm);
        if (n >= 0)
            rpm += n;
    } while (n > 0 && rpm < (sizeof (packet_header_t)));

    if (n == 0)
        return -1;

    if (n < 0) {
        perror("read");
        return -1;
    }

    if (rpm != sizeof (packet_header_t)) {
        xprintf("HEADER ERROR (%d!=%d)!!\n", n, sizeof (packet_header_t));
    }

#ifdef PKT_DEBHEAD
    printf("RUNPH: RCVHEAD: id=0x%x,opts=0x%x,ts=0x%x,sz=0x%x fd=%d\n", pkth.id,
            pkth.opts, pkth.tstamp, pkth.size,fd);
#endif

    if (pkth.id != PACKET_MAGIC) {
        printf
                ("RUNPHD: Received an invalid packet 0x%x. Closing connection.\n",
                pkth.id);
        return -1;
    }
    size = pkth.size;
    if (size > (MSG_BUFF_SZ)) {
        printf
                ("RUNPHD: Caution: packet size exceed msg buffer size (%d>%d)... trunkating\n",
                size, MSG_BUFF_SZ);
        size = MSG_BUFF_SZ;
    }
    if (size < 0) {
        printf("RUNPH: Error message size is negative (%d)\n", size);
        return -1;
    }
    if (net_db->full_bridge) {
        memcpy(msg_rcv.body, &pkth, sizeof(packet_header_t));
        bptr = (char *) &msg_rcv.body[sizeof(packet_header_t)];
    } else {
        bptr = (char *) msg_rcv.body;
    }
    rpm = 0;
    do {
        n = read(fd, &bptr[rpm], size-rpm);
        if (n >= 0)
            rpm += n;
    } while ((n == -1 && errno == EINTR) || (n > 0 && rpm < size));


#ifdef PKT_DEBUG
    get_time(&tdata);
   	printf
            ("HWAPI_NETD: -- RCV -- TS=%d (t=%d:%d) type=0x%x size=%d opts=0x%x fd=%d\n",
            get_tstamp(), tdata.tv_sec, tdata.tv_usec, msg_rcv.head.dst, size,
            pkth.opts,fd);
#endif

    if (n == 0) {
    	printf("body not received size %d\n",size);
        return 0;
    }

    if (n < 0) {
        perror("read");
        return n;
    }

    if (rpm != size) {
        xprintf("CONTENTS ERROR (%d!=%d)!!\n", rpm, size);
    }
    unsigned int *data = (unsigned int*) bptr;
    int i;
    if (net_db->litendian) {
        for (i=0;i<pkth.size>>2;i++) {
            data[i] = ntohl(data[i]);
        }
    }

#ifdef PKT_DEBCONT
    for (i=0;i<pkth.size>>2;i++)
        xprintf("0x%x-", data[i]);
    printf("\n");
#endif

    /* read all packet although buffer exceed */
    if (pkth.size > size) {
        n = lseek(fd, (pkth.size - size), SEEK_CUR);
        if (n < 0)
            return n;
    }

    if (net_db->full_bridge) {
        size += sizeof(packet_header_t);
    }
    /* bridge packet to msg mbox */
    if (!pkth.tstamp) {
        pkth.tstamp=1;
    }
    msg_rcv.head.dst = pkth.tstamp;
    msg_rcv.head.opts = pkth.opts;
    

    do {
        n = msgsndcallback(mbox_id, &msg_rcv, MSG_HEAD_SZ + size, 0,int_itf_idx);
    } while (n == -1 && errno == EINTR);

    if (n < 0) {
        printf("error sending message id 0x%x type %d size %d\n",mbox_id,msg_rcv.head.dst,size);
        perror("msgsnd");
        return n;
    }


    return 1;
}

#define UDP_BUFFSZ 100*1024
char udpbuff[UDP_BUFFSZ];

int net_packet_rcv_udp(int fd, struct ext_itf *xitf)
{
    packet_header_t *pkth;
    int n, rpm;
    int size;
    char *bptr;
    int clientLen=sizeof(struct sockaddr);

    int mbox_id=xitf->in_mbox_id;
    hwitf_t key_id=xitf->in_key_id;
    int int_itf_idx=xitf->int_itf_idx;

#ifdef PKT_DEBUG
    time_t tdata;
#endif

#ifdef PKT_DEBUG2
    printf("HWAPI_NETD: --RCV-- Waiting fd=%d,mbox=%d\n", fd, mbox_id);
#endif

    n=recvfrom(fd,udpbuff,UDP_BUFFSZ,0,&xitf->remote_udp_addr,&clientLen);
    if (n < 0) {
        perror("rcvfrom");
        exit(-1);
    }
    xitf->remote_udp_connected=1;
    pkth = (packet_header_t*) udpbuff;

#ifdef PKT_DEBHEAD
    printf("RUNPH: RCVHEADUDP: id=0x%x,opts=0x%x,ts=0x%x,sz=0x%x fd=%d\n", pkth->id,
            pkth->opts, pkth->tstamp, pkth->size,fd);
#endif

    if (pkth->id != PACKET_MAGIC) {
        printf
                ("RUNPHD: Received an invalid packet 0x%x. Closing connection.\n",
                pkth->id);
        return -1;
    }
    size = pkth->size;
    if (size > (MSG_BUFF_SZ)) {
        printf
                ("RUNPHD: Caution: packet size exceed msg buffer size (%d>%d)... trunkating\n",
                size, MSG_BUFF_SZ);
        size = MSG_BUFF_SZ;
    }
    if (size < 0) {
        printf("RUNPH: Error message size is negative (%d)\n", size);
        return -1;
    }
    if (net_db->full_bridge) {
        memcpy(msg_rcv.body, &pkth, sizeof(packet_header_t));
        bptr = (char *) &msg_rcv.body[sizeof(packet_header_t)];
    } else {
        bptr = (char *) msg_rcv.body;
    }
    memcpy(msg_rcv.body,&udpbuff[sizeof(packet_header_t)],size);
    
    /* bridge packet to msg mbox */
    if (!pkth->tstamp) {
        pkth->tstamp=1;
    }
    msg_rcv.head.dst = pkth->tstamp;
    msg_rcv.head.opts = pkth->opts;

    do {
        n = msgsndcallback(mbox_id, &msg_rcv, MSG_HEAD_SZ + size, 0,int_itf_idx);
    } while (n == -1 && errno == EINTR);

    if (n < 0) {
        printf("error sending message id 0x%x type %d size %d\n",mbox_id,msg_rcv.head.dst,size);
        perror("msgsnd");
        return n;
    }

#ifdef PKT_DEBUG
    get_time(&tdata);
    printf
            ("HWAPI_NETD: -- RCV -- TS=%d (t=%d:%d) type=0x%x size=%d opts=0x%x fd=%d\n",
            get_tstamp(), tdata.tv_sec, tdata.tv_usec, msg_rcv.head.dst, size,
            pkth->opts,fd);
#endif

#ifdef DEB_TIME
      time_t t;
      get_time(&t);
      printf("received packet at %d:%d\n",t.tv_sec,t.tv_usec);
#endif

    return 1;
}


/** Close Networds child
 *
 * Closes a network process created for bridging
 *
 * @sa Net_thread()
 */
int Net_close_chld(struct ext_itf *ext_itf, int wait_time)
{
    int c;
    int sleep_time;

    if (!wait_time) {
        wait_time = 500;
    }

    sleep_time = wait_time / 10;

    if (ext_itf->bpid > 0) {
        kill(ext_itf->bpid, SIGTERM);
        c = 0;
        while (ext_itf->bpid > 0) {
            sleep_ms(sleep_time);
            c++;
            if (c == 10) {
                printf("RUNPHD: Trouble closing network b-child. Kill %d.\n",
                        ext_itf->bpid);
                kill(ext_itf->bpid, SIGKILL);
                return -1;
            }
        }
    }
    if (ext_itf->pid > 0) {
        kill(ext_itf->pid, SIGTERM);
        c = 0;
        while (ext_itf->pid > 0) {
            sleep_ms(sleep_time);
            c++;
            if (c == 10) {
                printf("RUNPHD: Trouble closing network child. Kill %d.\n",
                        ext_itf->pid);
                kill(ext_itf->pid, SIGKILL);
                return -1;
            }
        }
    }
    return 1;
}

/** Net Close
 *
 * Closes main network thread and calls Net_close_chld for each
 * process created (one for each interface)
 */
void Net_close(int wait_time)
{
    int i, c;
    int sleep_time;
    int error = 0;

    if (!wait_time) {
        wait_time = 500;
    }

    sleep_time = wait_time / 10;


    /* kill all network childs */
    for (i = 0; i < net_db->nof_ext_itf; i++) {
        if (Net_close_chld(&net_db->ext_itf[i], wait_time) < 0)
            error++;
    }

    /* now kill main net thread */
    if (net_db->net_chld > 0) {
        kill(net_db->net_chld, SIGTERM);
        c = 0;
        while (net_db->net_chld > 0) {
            sleep_ms(sleep_time);
            c++;
            if (c == 10) {
                printf("RUNPHD: Trouble closing main network child. Kill %d\n",
                        net_db->net_chld);
                kill(net_db->net_chld, SIGKILL);
                error++;
                break;
            }
        }
    }

    if (error) {
        printf
                ("RUNPHD: Some network childs could not be closed properly (%d).\n",
                error);
    }
}

/** Network Interface thread
 *
 * This is the main thread for every network inteface. Depending on its type, it listens to 
 * either external or internal interface for new packets and bridge to the opposite site. 
 * In case of bidirectional interfaces, another thread is launched from this function.
 *
 * @todo Check this function
 */
void do_net_chld(struct ext_itf *xitf)
{
    struct sockaddr addr;
    struct sockaddr udpaddr;

    struct sched_param param;
    int n, pid, fd;
    int on=1;
    int i_listen, do_snd, do_rcv;

    switch (xitf->mode) {
    case FLOW_READ_ONLY:
        i_listen = 1;
        do_snd = 0;
        do_rcv = 1;
        break;
    case FLOW_WRITE_ONLY:
        i_listen = 0;
        do_snd = 1;
        do_rcv = 0;
        break;
    case FLOW_READ_WRITE:
        i_listen = 1;
        do_snd = 1;
        do_rcv = 1;
        break;
    case FLOW_WRITE_READ:
        i_listen = 0;
        do_snd = 1;
        do_rcv = 1;
        break;
    }
#ifdef CONN_DEBUG
    printf("RUNPHD: Pid %d starting on itf 0x%x mode 0x%x...\n", getpid(),
            xitf->id, xitf->mode);
#endif
    while (1) {
        /* start the connection or setup server */
        if (i_listen) {
#ifdef CONN_DEBUG
            printf("RUNPH_NET: Pid %d waiting for connection...\n", getpid());
#endif
            fd = setup_server(htonl(INADDR_ANY), xitf->port, &xitf->fsock,xitf->type);
            if (fd < 0) {
                printf("RUNPH_NET: Error setting server\n");
                exit(-1);
            }

           if (xitf->type==NET_TCP) {
/*        	   unsigned char on=1;
               if (setsockopt (xitf->fsock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof (on)))
                 perror("setsockopt1");
*/           } else {
               unsigned char opt = 1;
               setsockopt(xitf->fsock, SOL_SOCKET, SO_PRIORITY, &opt, sizeof(opt));
               int tos=IPTOS_LOWDELAY;
               setsockopt(xitf->fsock, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
    			int bz=0;
    			if (setsockopt(xitf->fsock, SOL_SOCKET, SO_RCVBUF, &bz, sizeof(bz)) == -1) {
    				perror("setsock3");
    			}
           }

            xitf->fd = fd;
#ifdef CONN_DEBUG
            printf("RUNPH_NET: Input connection. id=0x%x fd=%d\n", xitf->id,
                    fd);
#endif
        } else {
#ifdef CONN_DEBUG
            printf("RUNPH_NET: Pid %d waiting to configure output itf to %s:%d...\n",
                    getpid(),xitf->address,xitf->port);
#endif
            while (!xitf->out_mbox_id)
                sleep_ms(10);

#ifdef CONN_DEBUG
            printf("RUNPH_NET: Pid %d configuring output itf to %s:%d...\n",
                    getpid(),xitf->address,xitf->port);
#endif
            fd = setup_client(inet_addr(xitf->address), xitf->port,xitf->type);
            if (fd < 0) {
                printf("RUNPH_NET: Error connecting to server\n");
                exit(-1);
            }
            if (!xitf->fragment) {
				int val = IP_PMTUDISC_DO;
				if (setsockopt(fd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val)))
					perror("setsockoptmtu");
            }
            if (setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof (on)))
              perror("setsockopt3");

           unsigned char opt = 7;
           setsockopt(fd, SOL_SOCKET, SO_PRIORITY, &opt, sizeof(opt));

            xitf->fd = fd;
#ifdef CONN_DEBUG
            printf("RUNPH_NET: Output connection. id=0x%x fd=%d\n", xitf->id,
                    fd);
#endif
        }
        if (do_snd && do_rcv) {
            pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(-1);
            }
            if (pid > 0) {
#ifdef CONN_DEBUG
                printf("RUNPH_NET: Child %d created\n", pid);
#endif
                xitf->bpid = pid;

            }
        } else
            pid = 0;

        if (do_snd || !pid) {
            if (do_snd && !pid) {
                if (i_listen)
                    if (!xitf->type) {
                        close(xitf->fsock);
                    }

#ifdef CONN_DEBUG
                printf("RUNPH_NET: Waiting to configure output itf...\n");
#endif
                while (!xitf->out_mbox_id)
                    sleep_ms(10);

                do {
                    n = net_packet_snd(fd, xitf);
                } while (n > 0);

#ifdef CONN_DEBUG
                printf("RUNPH_NET: Out Connection lost. id 0x%x pid %d.\n",
                        xitf->id, getpid());
#endif
                close(fd);
                xitf->out_mbox_id = 0;
                xitf->fd = 0;
                if (do_rcv)
                    exit(0);
            } else {
#ifdef CONN_DEBUG
                printf("RUNPH_NET: Waiting in mbox to configure...\n");
#endif
                while (!xitf->in_mbox_id)
                    sleep_ms(10);

#ifdef CONN_DEBUG
                printf("RUNPH_NET: starting rcv pid %d mbox 0x%x\n", getpid(),
                        xitf->in_mbox_id);
#endif
                do {
                    if (xitf->type==NET_TCP) {
                        n = net_packet_rcv_tcp(fd, xitf);
                    } else if (xitf->type==NET_UDP) {
                        n = net_packet_rcv_udp(fd, xitf);
                    } else {
                    	printf("RUNPH_NET: Error, unknown itf type %d\n",xitf->type);
                    }
                } while (n > 0);
                close(fd);
#ifdef CONN_DEBUG
                printf("RUNPH_NET: In Connection lost. id 0x%x pid %d\n",
                        xitf->id, getpid());
#endif
                xitf->fd = 0;
                if (do_snd) {
#ifdef CONN_DEBUG
                    printf("killing snd chld proc %d\n", pid);
#endif
                    kill(pid, SIGKILL);
                }
            }
        }
    }
}

/** Main Network Thread
 *
 * This is the main network thread. It launches a new thread for every external interface
 * depending on its characteristics.
 *
 * @todo Check this function.
 */
void Net_thread(struct hw_api_db *hwapi, int msg_id)
{
    int i, n;
    int netpid;
    
    net_db = &hwapi->net_db;
    hwapi_db = hwapi;

    netpid = fork();
    if (netpid < 0) {
        perror("fork");
        exit(-1);
    }
    if (netpid > 0) {
        return;
    }


    set_realtime(getpid(),net_db->base_prio,net_db->base_cpuid);
        
    data_msg_id = msg_id;
    net_db->net_chld = getpid();

    /* Network child continues here */
    i = 0;

    for (i = 0; i < net_db->nof_ext_itf; i++) {
        net_db->ext_itf[i].pid = 0;
        net_db->ext_itf[i].fsock = 0;
    }

    while (1) {
        for (i = 0; i < net_db->nof_ext_itf; i++) {
            if (!net_db->ext_itf[i].pid) {
                n = fork();
                if (n < 0) {
                    perror("fork");
                    exit(-1);
                }
                if (!n) {
                    set_realtime(getpid(),net_db->itf_prio,net_db->itf_cpuid);
                    do_net_chld(&net_db->ext_itf[i]);
                } else {
                    net_db->ext_itf[i].pid = n;
                }
            }
        }
        sleep_ms(10);
    }
    printf("net child is exiting\n");
    exit(0);
}


/** @} */


