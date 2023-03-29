/*
 * net_utils.c
 *
 * Copyright (c) 2009 Xavier Reves, UPC <xavier.reves at tsc.upc.edu>. All rights reserved.
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "phal_hw_api.h"
#include <errno.h>

#define DEFAULT_PROTOCOL 0

/********************
 *   SETUP CLIENT   *
 ********************/	
int setup_client(unsigned long int ip, int port, int type)
{
	int n;
        int trials=0;
	int clientFd, serverLen;
	struct sockaddr_in serverINETAddress;
	struct sockaddr *serverSockAddrPtr;

	serverSockAddrPtr = (struct sockaddr *)&serverINETAddress;
	serverLen = sizeof(serverINETAddress);

	bzero((char *)&serverINETAddress,serverLen);
	serverINETAddress.sin_family = AF_INET;
	serverINETAddress.sin_addr.s_addr = ip;
	serverINETAddress.sin_port = htons(port);
		
	clientFd = socket (AF_INET, type?SOCK_DGRAM:SOCK_STREAM, DEFAULT_PROTOCOL);
	if (clientFd == -1)
	{
		perror("socket");
		return(-1);
	}
	
	/*fprintf(out_file,"TESTBED: Connecting...\n");*/
        do {
            n=connect(clientFd, serverSockAddrPtr, serverLen);
            if (n)
            {
                if (errno != ECONNREFUSED) {
                    printf("Error connecting to %s:%d\n", inet_ntoa(serverINETAddress.sin_addr), port);
                    perror("connect");
                    return(-1);
                }
                printf("trial %d could not connect\n", trials);
                sleep(1);
                trials++;
            }
        } while (n && errno==ECONNREFUSED && trials<10);
	printf("Connected with %s\n",inet_ntoa(serverINETAddress.sin_addr));

	return(clientFd);
}

/********************
 *   SETUP SERVER   *
 ********************/	
int setup_server(unsigned long int ip, int port, int *sck_fd, int type)
{
	int n;
	int serverFd;
        unsigned int serverLen, clientLen;
	int socketFd=0;
	struct sockaddr_in serverINETAddress;
	struct sockaddr_in clientINETAddress;
	struct sockaddr *serverSockAddrPtr;
	struct sockaddr *clientSockAddrPtr;

	serverLen = sizeof(serverINETAddress);
	clientLen = sizeof(clientINETAddress);

	bzero((char *)&serverINETAddress,serverLen);
	serverINETAddress.sin_family = AF_INET;
	serverINETAddress.sin_addr.s_addr = ip;
	serverINETAddress.sin_port = htons(port);
	
	serverSockAddrPtr = (struct sockaddr*)&serverINETAddress;
	clientSockAddrPtr = (struct sockaddr *)&clientINETAddress;
	
	/*get a new socket fd if not yet obtained*/
	if (sck_fd!=NULL)
		socketFd=*sck_fd;

	if (socketFd<=0)
	{
		socketFd = socket(AF_INET, type?SOCK_DGRAM:SOCK_STREAM, 0);
		if (socketFd==-1)
		{
			perror("socket");
			exit(-1);
		}
		n=bind(socketFd, serverSockAddrPtr, serverLen);
		if (n==-1)
		{
			perror("error");
			exit(-1);
		}
		if (sck_fd!=NULL)
			*sck_fd=socketFd;
	}	

        if (!type) {
            /*Get a different file descriptor for each connection*/
            listen (socketFd, 1);
            do {
                    serverFd = accept(socketFd, clientSockAddrPtr, &clientLen);
            } while(serverFd==-1 && errno==EINTR);
            if (serverFd==-1)
            {
                    perror("accept");
                    exit(-1);
            }
        } else {
            serverFd=socketFd;
        }
	return(serverFd);
}
