/* hwapi_linux_dac.c
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

#include "phal_hw_api.h"
#include "hwapi_backd.h"

/** hw_api general database */
extern struct hw_api_db *hw_api_db;

int hwapi_dac_getSampleType() {
	return hw_api_db->dac.sampleType;
}

double hwapi_dac_getInputFreq() {
	return hw_api_db->dac.inputFreq;
}

double hwapi_dac_getOutputFreq() {
	return hw_api_db->dac.outputFreq;
}

int hwapi_dac_setInputFreq(double freq) {
	hw_api_db->dac.inputFreq=freq;
	return 1;
}

int hwapi_dac_setOutputFreq(double freq) {
	hw_api_db->dac.outputFreq=freq;
	return 1;
}

int hwapi_dac_setInputLen(int nSamples)
{
	if (nSamples<10) {
		printf("Too few samples per time-slot!!\n");
		return 0;
	}
    hw_api_db->dac.NsamplesIn=nSamples;
    return 1;
}

int hwapi_dac_setOutputLen(int nSamples)
{
	if (nSamples<10) {
		printf("Too few samples per time-slot!!\n");
		return 0;
	}
    hw_api_db->dac.NsamplesOut=nSamples;
    return 1;
}
int hwapi_dac_getInputLen()
{

    return hw_api_db->dac.NsamplesIn;
}

int hwapi_dac_getOutputLen()
{

    return hw_api_db->dac.NsamplesOut;
}

int hwapi_dac_getMaxInputLen() {
	return sizeof(_Complex float) * DAC_BUFFER_SZ;
}
int hwapi_dac_getMaxOutputLen() {
	return sizeof(_Complex float) * DAC_BUFFER_SZ;
}

void *hwapi_dac_getOutputBuffer(int channel)
{
	if (channel<0 || channel>hw_api_db->dac.nof_channels) {
		printf("Error invalid channel number %d\n",channel);
		return 0;
	}
    return hw_api_db->dac.dacoutbuff[channel];
}

void *hwapi_dac_getInputBuffer(int channel)
{
	if (channel<0 || channel>hw_api_db->dac.nof_channels) {
		printf("Error invalid channel number %d\n",channel);
		return 0;
	}
    return hw_api_db->dac.dacinbuff[channel];
}


