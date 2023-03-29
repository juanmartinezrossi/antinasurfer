/* hwapi_linux_hwinfo.c
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
#include <semaphore.h>
#define __USE_GNU
#include <ucontext.h>
#include <execinfo.h>

#include "mem.h"
#include "phal_hw_api.h"
#include "hwapi_backd.h"

/** hw_api general database */
extern struct hw_api_db *hw_api_db;

/** pointer to proc db */
extern struct hwapi_proc_i *proc_db;

/** save my proc pointer */
extern struct hwapi_proc_i *my_proc;


/** @defgroup hw Hardware Monitoring Functions
 *
 * A set of functions are available to gather information about
 * underlying hardware characteristics and current status.
 *
 * @{
 */

/** Physical Interface Information.
 *
 * Obtains information of the Physical External interfaces. It is read from
 * the physical interfaces configuration file (RUNPH Launcher).
 *
 * User must provide the maximum amount of external interfaces he wants to read.
 * It depends on the length of the xitf buffer (number of elements)
 *
 * @param xitf Pointer to information structure (check phal_hw_api.h)
 * @param max_itf Maximum number of interfaces (buffer length, in elements)
 *
 * @return >=0 Number of interfaces read
 * @return <0 error
 */
int hwapi_hwinfo_xitf(struct hwapi_xitf_i *xitf, int max_itf)
{
    int i;
    assert(xitf && max_itf >= 0);

    if (max_itf > hw_api_db->net_db.nof_ext_itf) {
        max_itf = hw_api_db->net_db.nof_ext_itf;
    }

    /* copy all interfaces */
    for (i = 0; i < max_itf; i++) {
        memcpy(&xitf[i], &hw_api_db->net_db.ext_itf[i].info,
                sizeof (struct hwapi_xitf_i));
    }
    return max_itf;
}

int hwapi_hwinfo_isDebugging()
{
	return hw_api_db->cpu_info.debug_level?1:0;
}

/** Get Processor Information.
 *
 *
 * @todo Definition of structure, dynamic change? battery? etc.
 *
 *
 * Function used to gather processing resource (CPU) information.
 * Data is writted to the struct pointed by info:
 *
 *
 * @see hwapi_cpu_i
 * @param info CPU information struct
 *
 */
void hwapi_hwinfo_cpu(struct hwapi_cpu_i * info)
{
    assert(info);
    struct msqid_ds msgstatus;

    /* fill info, copy it from hw_api_db struct */
    memcpy(info, &hw_api_db->cpu_info, sizeof (struct hwapi_cpu_i));
    /*msgctl(hw_api_db->data_msg_id, IPC_STAT, &msgstatus);
    hw_api_db->cpu_info.used_fifo = msgstatus.__msg_cbytes;
    hw_api_db->cpu_info.total_fifo = msgstatus.msg_qbytes;
*/
}

void hwapi_hwinfo_setid(int pe_id)
{
    hw_api_db->cpu_info.pe_id = pe_id;
}


/** @} */
