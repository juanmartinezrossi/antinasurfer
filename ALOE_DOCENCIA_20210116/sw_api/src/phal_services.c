/*
 * phal_services.c
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <complex.h>

#include "phal_hw_api.h"
#include "phal_sw_api.h"

#include "phal_daemons.h"
#include "stats.h"
#include "str.h"
#include "log.h"

#define EXIT_ON_FULLQUEUE

#define PHAL_STATUS_ABNSTOP 7

/** Uncomment to print params and itfs initialization errors
/*
#define DEBPARAMS
#define DEBITF
*/

static int obj_id;
static int params_initiated;
static int init_done, stop_done;
static int relinquish_slots;
static time_t run_time[3];

static char aux_string[256];


struct my_fd {
    int fd;
    int mode;
    int id;
};

#define MAX_OBJ_FD 50
static struct my_fd my_fd[MAX_OBJ_FD];

#define MAX_STATS 50
struct mystats {
    int id;
    int elemsz;
};
static struct mystats my_stats[MAX_STATS];
static struct mystats my_objstats[MAX_STATS];

struct time_counter {
    int statId;
    time_t tdata[3];
};

#define MAX_TIME_COUNTERS 50
static struct time_counter time_counters[MAX_TIME_COUNTERS];

/** My process information
 */
static struct hwapi_proc_i my_obj;
static int status;

static struct hwapi_cpu_i cpu;

/* Buffers for logs */
static char logbuffer1[LOG_LINE_SZ], logbuffer2[LOG_LINE_SZ];
static char *logbuffer = logbuffer1;
static int logoffset;
static int log_created;
static char timestr[64];

/* internal functions */
void flush_logs(void);
int sizeoftype(int type);
int phalinitiated;

/**@defgroup base Basic Functions
 * This functions include some common and basic
 * functions required to use the SW API Library
 *
 * @{
 */

/** Initalize ALOE SW-API Library.
 *
 * This function must be called before calling
 * any other ALOE function.
 *
 * @returns 1 ok 0 error
 */
int InitPHAL()
{
    init_done = 0;
    params_initiated = 0;
    logoffset=0;
    log_created=0;
    phalinitiated=0;

    /* Init hwapi */
    if (!hwapi_init()) {
        xprintf("SWAPI: Error initiating HWAPI.\n");
        return 0;
    }

    hwapi_atterm(ClosePHAL);

    /* get my process information */
    hwapi_proc_myinfo(&my_obj);

    obj_id = my_obj.obj_id;

    /* get cpu info */
    hwapi_hwinfo_cpu(&cpu);

    /* update status */
    hwapi_proc_status_get();

    memset(my_fd, 0, sizeof (struct my_fd) * MAX_OBJ_FD);
    memset(my_stats, 0, sizeof (struct mystats) * MAX_STATS);
    memset(time_counters,0,sizeof(struct time_counter)*MAX_TIME_COUNTERS);
    memset(logbuffer1, 0, LOG_LINE_SZ);
    memset(logbuffer2, 0, LOG_LINE_SZ);

    phalinitiated=1;

    return (obj_id);
}

/** Close SW-API
 *
 * Liberates object resources. No more calls to any API function can be performed
 * after this function call.
 * This function must be called 'allways' when the object finishes its execution.
 * Some resources might be unfreed otherwise, with the possibility of overall system
 * unstability
 */
void ClosePHAL()
{
    int i, n;

    if (!phalinitiated)
        return;

    if (log_created) {
        CloseLog(log_created);
    }

    /* close all opened stats not closed */
    for (i = 0; i < MAX_STATS; i++) {
        if (my_stats[i].id) {
            CloseStat(i);
        }
    }

    /* close parameters if not closed */
    if (params_initiated) {
        CloseParamFile();
    }


    n = hwapi_proc_status_get();
    if (n!=PHAL_STATUS_RUN) {
        /* Close attached Flows */
        for (i = 0; i < MAX_OBJ_FD; i++) {
            if (my_fd[i].fd) {
                CloseItf(my_fd[i].fd);
            }
        }

        /* Make sure all interfaces are removed. Is it necessary? */
        hwapi_itf_delete_own(my_obj.obj_name);
    }
    if (n != PHAL_STATUS_STOP && n != PHAL_STATUS_ABNSTOP) {
        hwapi_proc_status_new(obj_id, PHAL_STATUS_ABNSTOP, -1);
        hwapi_proc_status_get();
        hwapi_proc_status_get();
    }

    /* wait to exit until EXEC daemon confirms our new status (stop) */
    while (!hwapi_proc_status_new(obj_id, PHAL_STATUS_STOP, 0)) {
    	sleep_ms(1);
    }

    phalinitiated=0;

    hwapi_exit();

}


#ifdef PRINT_OBJ_REL_TIME
int last_printed = 0;
#endif

/**@} */

extern int signaled;

/**@ingroup exec_status
 *@{
 */
/** Relinquish Function.
 *
 * Realeases the CPU ownership of the object.
 * Object goes sleep until the beggining of the next time slot.
 * This function must be called on every object's execution cycle (see ALOE Manual)
 *
 * @param nslots Number of slots to sleep (default 1)
 *
 */
void Relinquish(int nslots)
{
#ifdef PRINT_OBJ_REL_TIME
    time_t t;
    char *s;
#endif
    if (status == PHAL_STATUS_INIT) {
        init_done = 1;
    } else if (status == PHAL_STATUS_STOP || status == PHAL_STATUS_ABNSTOP) {
        xprintf("SWAPI: Error can't call Relinquish in stop, use ClosePHAL and exit\n");
        ClosePHAL();
        exit(-1);
    }


#ifdef PRINT_OBJ_REL_TIME
    hwapi_proc_myinfo(&my_obj);
    get_time(&t);

    if (strlen(my_obj.obj_name) < 8)
        s = "\t\t";
    else
        s = "\t";

    if (get_tstamp() - last_printed > PRINT_OBJ_REL_TIME) {
        xprintf("%s%s@ %d - %d - %d\t(%d usec cpu)\n", my_obj.obj_name, s, get_tstamp(), my_obj.obj_tstamp, my_obj.obj_tstamp,my_obj.cpu_time[0].tv_usec);
        last_printed = get_tstamp();
    }

#endif
    flush_logs();

    hwapi_relinquish(nslots);

}

char *GetObjectName()
{
    return my_obj.obj_name;
}

char *GetAppName()
{
    return my_obj.app_name;
}


/** Get Status.
 *
 * Get current object status. This function does not return until object has something to do, that is,
 * status is RUN or the first INIT or CLOSE.
 *
 * @returns PHAL_STATUS_INIT, PHAL_STATUS_RUN or PHAL_STATUS_CLOSE
 */
int Status(void)
{
    while (1)
 {

        status = hwapi_proc_status_get();

        /* return status */
        if (status == PHAL_STATUS_RUN) {
            hwapi_proc_mytstampinc();
            return (PHAL_STATUS_RUN);
        } else if (status == PHAL_STATUS_INIT && !init_done) {
            return (PHAL_STATUS_INIT);
        } else if (status == PHAL_STATUS_STOP || status
                == PHAL_STATUS_ABNSTOP) {
            return (PHAL_STATUS_STOP);
        } else if (status == PHAL_STATUS_STEP) {
            hwapi_proc_mytstampinc();
            hwapi_proc_status_get();
            hwapi_proc_status_ack(obj_id);
            hwapi_proc_status_new(obj_id, PHAL_STATUS_PAUSE, 0);
            hwapi_proc_status_get();
            return PHAL_STATUS_RUN;
        } else {
            hwapi_relinquish(0);
        }
    }
}

/**@} */

/**@defgroup time Timing Functions
 *
 * This section contains the functions used for time management
 * @{
 */


/** Initialize Counter
 * Initializes a time counter. It uses stats services.
 * It counts time in microseconds, until 1000000 (1s)
 * @param name Name for the counter (stat)
 * @return Id for the counter. <0 if error
 */
int InitCounter(char *name)
{
    int i;
    int statId;
    if (!cpu.enable_stats)
    	return 1;
    i=0;
    while(i<MAX_TIME_COUNTERS && time_counters[i].statId)
        i++;
    if (i==MAX_TIME_COUNTERS) {
        printf("SWAPI: No more time counters allowed\n");
        return -1;
    }
    statId = InitStat(name,STAT_TYPE_INT,1);
    if (statId<0) {
        printf("SWAPI: Error initiating stat for time counter\n");
        return -1;
    }
    time_counters[i].statId = statId;
    return i;
}
/** Start time counter
 * @param CounterId Id returned from InitCounter() function
 */
void StartCounter(int CounterId)
{
    if (!cpu.enable_stats)
    	return;

   assert(CounterId>=0 && CounterId<MAX_TIME_COUNTERS);
   get_time(&time_counters[CounterId].tdata[1]);
}

/** Stop time Counter
 * Stops the timer and saves statistics value with the resulting inteval
 * @param CounterId Id returned from InitCounter() function
 */
void StopCounter(int CounterId)
{
    if (!cpu.enable_stats)
    	return;

   assert(CounterId>=0 && CounterId<MAX_TIME_COUNTERS);
   get_time(&time_counters[CounterId].tdata[2]);
   get_time_interval(time_counters[CounterId].tdata);
   SetStatsValue(time_counters[CounterId].statId,&time_counters[CounterId].tdata[0].tv_usec,1);
}

/** Get Tempo
 *
 * Returns the amount of samples to generate given a sampling frequency.
 * It internally does a trivial operation, however, the utility of this function
 * comes when the object implementation does not have any information on the
 * underlying time slicing (in other words, the object execution period).
 * Thus, calling this function before every execution cycle gives the amount of
 * samples to be generated given a desired sampling frequency and the platform-hidden
 * time slot duration
 *
 * @param freq Desired sampling frequency, in Hz
 * @returns Number of samples to be generated (freq*ts)
 */
unsigned int GetTempo(float freq)
{
    unsigned int nof_words;
    struct hwapi_cpu_i cpu;

    hwapi_hwinfo_cpu(&cpu);
    nof_words = (unsigned int) (freq * ((float) cpu.tslen_usec / 1000000));
    return nof_words;
}

/** Get current time stamp
 *
 * Returns the current object time stamp, this is, the amount of time it has executed
 * in the RUN state since the begining.
 *
 * @returns Time stamp
 */
int GetTstamp(void)
{
    struct hwapi_proc_i obj;
    hwapi_proc_myinfo(&obj);
    return obj.obj_tstamp;
}

/**@} */

/**@defgroup interfaces Logical Interfaces
 *
 * This section contains all functions related to the utilization of logical interfaces
 *
 * An logical interface is a flow of data between one object an another. Before using it,
 * it must be created (initialized) by the object, as well as described in the waveform definition
 * file (*.app).
 * @{
 */

/** Create Interface
 *
 * Initializes an interface and allocated necessary resources. After this function has been called
 * you can begin sending and receiving data through the interface.
 * The function returns a file descriptor than will be used later to use the interface.
 *
 * @param itf_name Name of the interface. For maximum length, see HW API SYSTEM_STR_MAX_LEN
 * @param mode FLOW_READ_ONLY, FLOW_WRITE_ONLY or FLOW_READ_WRITE
 *
 * @returns >0 file descriptor to use the interface
 * @returns -1 if error
 */
int CreateItf(char *itf_name, int mode)
{
    int i, fd;
    unsigned char itf_id;

    /* search a free fd in local db */
    for (i = 0; i < MAX_OBJ_FD; i++) {
        if (!my_fd[i].fd) {
            break;
        }
    }

    if (i == MAX_OBJ_FD) {
        xprintf("SWAPI: Error in %s: no more space for itfs in local db.\n",my_obj.obj_name);
        return -1;
    }

    /* get itf id */
    itf_id = hwapi_itf_find(my_obj.obj_name, itf_name, mode);
    if (!itf_id) {
        #ifdef DEBITF
        xprintf("SWAPI: Error itf %s:%s not found.\n", my_obj.obj_name, itf_name);
        #endif
        return -1;
    }

    /* attach to itf */
    fd = hwapi_itf_attach(itf_id, mode);
    if (fd < 0) {
        xprintf("SWAPI: Error attaching to itf 0x%x.\n", itf_id);
        return -1;
    }

    /* save info */
    my_fd[i].id = itf_id;
    my_fd[i].fd = fd;
    my_fd[i].mode = mode;

    return fd;
}

/** Close Interface
 *
 * Deallocates interface resources and prevents it for using again. This action is performed automatically by
 * ClosePHAL() function (at exit), however it is a good policy to manually call it for each interface for debuggin
 * purposes.
 *
 * @param fd File descriptor to close
 * @returns 1 closed ok
 * @returns 0 not closed
 */
int CloseItf(int fd)
{
    int i;

    /* search itf */
    for (i = 0; i < MAX_OBJ_FD; i++) {
        if (my_fd[i].fd == fd) {
            break;
        }
    }

    if (i == MAX_OBJ_FD) {
        xprintf("SWAPI: Error can't find fd %d\n", fd);
        return 0;
    }

    hwapi_itf_unattach(my_fd[i].fd, my_fd[i].mode);

    if (my_fd[i].id && (my_fd[i].mode == FLOW_WRITE_ONLY || hwapi_itf_isexternal(my_fd[i].id))) {
        hwapi_itf_delete(my_fd[i].id);
    }

    memset(&my_fd[i], 0, sizeof (struct my_fd));

    return 1;
}

/** Write to an interface
 *
 * This function writes certain amount of data into a logical interface.
 *  *
 * @warning: The function is non-blocking (if the queue is full, no data will be written)
 *
 * @param fd File descriptor of the interface
 * @param buffer Pointer for the user data
 * @param size Amount of bytes to be written
 *
 * @returns >0 The amount of bytes written
 * @returns =0 Data could not be written
 * @returns -1 Error sending through interface
 */
int WriteItf(int fd, void *buffer, int size)
{
    return hwapi_itf_snd(fd, buffer, size);
}

/** Read from an interface
 *
 * This function reads certain amount of data from a logical interface.
 * The packetization problem is managed internally by ALOE. This means that although the remote object
 * send its information in discrete packets, the receiver can obtain any desired amount of information.
 * If packet has to be splitted or joined, it'll be done by ALOE.
 *
 * @warning: The function is non-blocking.
 *
 * @param fd File descriptor of the interface
 * @param buffer Pointer where to save the data
 * @param size Amount of bytes to be received
 *
 * @returns >0 The amount of bytes received
 * @returns =0 Any data available
 * @returns -1 Error receiveing from interface
 */
int ReadItf(int fd, void *buffer, int size)
{
    return hwapi_itf_rcv(fd, buffer, size);
}

/** Obtain Interface Status
 *
 * Obtain actual interface utilization, i.e. the amount of bytes in the queue
 *
 * @param fd File descriptor of the interface
 * @return >=0 The amount of bytes in the queue
 * @return -1 Error
 */
int GetItfStatus(int fd)
{
    return hwapi_itf_status(fd);
}

/**@} */


/**@defgroup params Initialization Parameters
 *
 * This collection of functions are used to obtain object's initialization parameter values.
 * Parameters are those variables that do change (sustantially) the behaviour of the object and
 * require long execution times to be performed (at least bigger than one time slot).
 *
 * As this parametrization can't be performed at real-time, a special initialization phase is
 * defined (INIT) so the object can take its time to perform such parametrization.
 *
 * An example of this can be computing filter coefficients given a band of pass. It is obvious that
 * this operation can't be performed at real-time.
 *
 * Before obtaining this values, STATSMAN daemons must read the file (system file) where this parameters
 * are kept. This operation has to explicity be done by the object calling the InitParamFile() function.
 *
 *@{
 */

/** Initiates Parameter Configuration File.
 *
 * Initiates the procedure to read the configuration file and allocate resources.
 * After the object has called this function, it can obtain parameter values using the GetParam function.
 *
 * @return 1 if ok 0 if error
 */

int InitParamFile(void)
{
    int stat_id, opt;
    int count = 0;

 /*   printf("\n phal_services.c: InitParamFile\n");*/

    if (params_initiated) {
        xprintf("SWAPI: Error params have been already initiated.\n");
        return 0;
    }

    sprintf(aux_string, "%s:%s", my_obj.app_name, my_obj.obj_name);

/*	printf("InitParamFile():aux_string=%s\n", aux_string);*/

    stat_id = hwapi_var_create(aux_string, 1, VAR_IS_PMINIT);
    if (stat_id < 0) {
        xprintf("SWAPI: Error initiating params.\n");
        return 0;
    }

    /* wait to be initiated */
    do {
        opt = hwapi_var_getopt(stat_id);

        if (opt != VAR_IS_PMINITOK && opt != VAR_IS_PMINITERR) {
            hwapi_relinquish(1);
            count++;
        }
    } while ((opt != VAR_IS_PMINITOK && opt != VAR_IS_PMINITERR));

    if (opt != VAR_IS_PMINITOK) {
        #ifdef DEBPARAMS
        xprintf("SWAPI: Error initiating parameters file.\n");
        #endif
 /*       xprintf("SWAPI: Error initiating parameters file.\n");*/
        hwapi_var_delete(stat_id);
        return 0;
    }

    /* remove variable */
    hwapi_var_delete(stat_id);

    params_initiated = 1;
 /*   printf("\n phal_services.c: InitParamFile  END\n\n");*/
    return 1;

}

/** Close Parameter Configuration File.
 *
 * Deallocates resources regarding parameter usage. Any more parameter can be obtained
 * after this function has been called.
 *
 * @return 1 if ok 0 if error
 */

int CloseParamFile(void)
{
    int stat_id;

    if (!params_initiated) {
        xprintf("SWAPI: Error must initiate params file first.\n");
        return 0;
    }

    sprintf(aux_string, "%s:%s", my_obj.app_name, my_obj.obj_name);

    stat_id = hwapi_var_create(aux_string, 1, VAR_IS_PMCLOSE);
    if (stat_id < 0) {
        xprintf("SWAPI: Error closing params.\n");
        return 0;
    }

    params_initiated = 0;

    return 1;

}

/** Obtain a parameter value.
 *
 * Get an initialization parameter value.
 *
 * @param ParamName Name of the parameter.
 * @param ParamValue Pointer where to save data.
 * @param ParamType One of the following (STAT_TYPE_INT, _FLOAT, _CHAR)
 * @param ParamLen Length of the expected parameter, in bytes.
 *
 * @return 1 if ok, 0 if error
 */
int GetParam(char *ParamName, void *ParamValue, int ParamType, int ParamLen)
{
    int stat_id, opt;
    int count = 0;
	int type;

    if (!params_initiated) {
        /*xprintf("SWAPI: Error must initiate params file first.\n");*/
		    xprintf("SWAPI: WARNING \x1b[22;34m\033[1m%s\x1b[0m not initialized in statman/APP_folder/%s.params file.\n", ParamName, GetObjectName());
        return 0;
    }
    stat_id = hwapi_var_create2(ParamName, ParamLen * sizeoftype(ParamType),
            VAR_IS_PMGET,ParamType);
    if (stat_id < 0) {
        xprintf("SWAPI: Error initiating params.\n");
        return 0;
    }

    /* wait to be initiated */
    do {
        opt = hwapi_var_getopt(stat_id);
        if (opt != VAR_IS_PMVALOK && opt != VAR_IS_PMVALERR) {
            hwapi_relinquish(1);
            count++;
        }
    } while ((opt != VAR_IS_PMVALOK && opt != VAR_IS_PMVALERR));

    if (opt != VAR_IS_PMVALOK) {
        #ifdef DEBPARAMS
        xprintf("SWAPI: Error initiating parameter %s (%d).\n",ParamName,opt);
        #endif
        hwapi_var_delete(stat_id);
        return 0;
    }
    /* copy data */
    if (hwapi_var_read(stat_id, ParamValue, ParamLen
            * sizeoftype(ParamType)) < 0) {
        xprintf("SWAPI: Error reading variable\n");
        hwapi_var_delete(stat_id);
        return 0;
    }

    /* and delete variable */
    hwapi_var_delete(stat_id);

    return 1;
}

/**@}*/


int sizeoftype(int type)
{
    switch (type) {
    case STAT_TYPE_CHAR:
    case STAT_TYPE_UCHAR:
        return sizeof (char);
    case STAT_TYPE_FLOAT:
        return sizeof (float);
    case STAT_TYPE_INT:
        return sizeof (int);
    case STAT_TYPE_SHORT:
        return sizeof (short);
	case STAT_TYPE_COMPLEX:
        return sizeof (_Complex float);
	case STAT_TYPE_STRING:
        return 128;
    default:
        xprintf("SWAPI: Error invalid stat type %d\n", type);
        return 0;
    }
}

/** @defgroup stats Statistics
 *
 *  Statistics are variables that evolve in time. They can be viewed in real-time
 * by ALOE or another higher-level application (CMDMAN), captured and dumped to a file
 * or modified.
 *
 * In order to be viewed/modified, the object must announce its presence (initialize) and give
 * it a name. Then, it must update its contents at every execution cycle (or whenever it desires)
 * using the SetStatsValue function.
 *
 * Conversely, using the GetStatsValue function it can obtain it last value.
 *
 *@{
 */



/** Initializes an statistics variable.
 *
 * Initializes a variable given a name, type and size.
 * Once the variable has been initialized, it can be
 * accessed (to view or modify) by higher layers (CMDMAN, user, etc.).
 *
 * The function returns an statistic identifier which is the one to be used later
 * when viewing/modifying its value.
 *
 * @param StatName Name of the variable
 * @param StatType Variable Type
 * @param StatSize Size in elements (depends on type) of the variable
 *
 * @returns > 0 Stat_id
 * @returns -1 if error
 */
int InitStat(char *StatName, int StatType, int StatSize)
{
    char *s;
    int stat_id, opt, stat_id2;
    int count = 0;
    int i, j;
    int type;
    int sizetype;

/*    printf("InitStat\n");*/

    if (!cpu.enable_stats)
    	return 1;

    sizetype=sizeoftype(StatType);
    if (!sizetype) {
    	xprintf("SWAPI: Error initiating stat %s:%s\n",my_obj.obj_name, StatName);
    	return -1;
    }

    /* check if available local stats */
    i = 1;
    while (i < MAX_STATS && my_stats[i].id) {
        i++;
    }
    if (i == MAX_STATS) {
        xprintf("SWAPI: No more space at local db for stats\n");
        return -1;
    }

    if (StatSize <= 0) {
        xprintf("SWAPI: Error invalid size %d for stat %s:%s\n", StatSize, my_obj.obj_name, StatName);
    }

    /* check name */
    j = 0;
    s = StatName;
    while (s[j] != '\0') {
        if (isspace(s[j])) {
            xprintf("SWAPI: Invalid stat name '%s'. Spaces are not allowed\n", StatName);
            return -1;
        }
        j++;
    }
    
    sprintf(aux_string, "%s:%s:%s", my_obj.app_name, my_obj.obj_name, StatName);
    stat_id = hwapi_var_create2(aux_string, StatSize * sizetype, VAR_IS_STINIT, StatType);
    if (stat_id < 0) {
        xprintf("SWAPI: Error initiating stat.\n");
        return -1;
    }

    /* wait to be initiated */
    do {
        opt = hwapi_var_getopt(stat_id);
        if (opt != VAR_IS_STOK && opt != VAR_IS_STERR) {
            hwapi_relinquish(1);
            count++;
        }
    } while (opt != VAR_IS_STOK && opt != VAR_IS_STERR && count<1000);

/*    printf("A opt=%d\n", opt);*/

    if (opt != VAR_IS_STOK) {
        xprintf("SWAPI: Error initiating stat %s:%s (%d).\n",my_obj.obj_name,StatName,opt);
        hwapi_var_delete(stat_id);
        return -1;
    }
 /*   printf("B opt=%d\n", opt);*/

 /*   printf("OUT IniStat: stat_id=%d, i=%d, sizeoftype(StatType)=%d\n", stat_id, i, sizeoftype(StatType));*/
    my_stats[i].id = stat_id;
    my_stats[i].elemsz = sizeoftype(StatType);
    return i;
}


/**
 */
int InitObjectStat(char *ObjectName, char *StatName, int StatType, int StatSize)
{
    char *s;
    int stat_id, opt;
    int count = 0;
    int i, j;
    int type;

    /* check if available local stats */
    i = 1;
    while (i < MAX_STATS && my_objstats[i].id) {
        i++;
    }
    if (i == MAX_STATS) {
        xprintf("SWAPI: No more space at local db for stats\n");
        return -1;
    }

    if (StatSize <= 0) {
        xprintf("SWAPI: Error invalid size %d\n", StatSize);
    }

    /* check name */
    j = 0;
    s = StatName;
    while (s[j] != '\0') {
        if (isspace(s[j])) {
            xprintf("SWAPI: Invalid stat name '%s'. Spaces are not allowed\n", StatName);
            return -1;
        }
        j++;
    }

    sprintf(aux_string, "%s:%s:%s", my_obj.app_name, ObjectName, StatName);

    stat_id = hwapi_var_create(aux_string, StatSize * sizeoftype(StatType),
            0);
    if (stat_id < 0) {
        xprintf("SWAPI: Error initiating stat.\n");
        return -1;
    }

     /** todo: during initialization phase, check if a remote object statistics
             * is available. the problem is that it might not be initialized yet.
             */
   #ifdef todo
    do {
        opt = hwapi_var_getopt(stat_id);

        if (opt != VAR_IS_STOK && opt != VAR_IS_STERR) {
            hwapi_relinquish(1);
            count++;
        }
    } while (opt != VAR_IS_STOK && opt != VAR_IS_STERR && count<100);

    if (opt != VAR_IS_STOK) {
        xprintf("SWAPI: Error initiating stat %s:%s (%d).\n",ObjectName,StatName,opt);
        hwapi_var_delete(stat_id);
        return -1;
    }
    #endif

    my_objstats[i].id = stat_id;
    my_objstats[i].elemsz = sizeoftype(StatType);
    return i;
}


/** Close an statistic.
 *
 * Closes an stat variable so it can not be used
 * neither anymore by the object or other entities.
 * Resources are also de-allocated.
 *
 * @returns 1 ok
 * @returns 0 error
 */
int CloseStat(int StatId)
{

    if (!cpu.enable_stats)
    	return 1;

    if (!StatId) {
        xprintf("SWAPI: Invalid statId %d\n", StatId);
        return 0;
    }

    /* check it has been created */
    if (!my_stats[StatId].id) {
        xprintf("SWAPI: Stat %d not created\n", StatId);
        return 0;
    }
    if (!hwapi_var_setopt(my_stats[StatId].id, VAR_IS_STCLOSE)) {
        xprintf("SWAPI: Error closing stat.\n");
        return 0;
    }

    my_stats[StatId].id = 0;

    /* stat will deleted by STATS when closing request is ack */

    return 1;

}



/**
 */
int CloseObjectStat(int StatId)
{
    if (!StatId) {
        xprintf("SWAPI: Invalid statId %d\n", StatId);
        return 0;
    }

    /* check it has been created */
    if (!my_objstats[StatId].id) {
        xprintf("SWAPI: Stat %d not created\n", StatId);
        return 0;
    }
    if (!hwapi_var_setopt(my_objstats[StatId].id, VAR_IS_STCLOSE)) {
        xprintf("SWAPI: Error closing stat.\n");
        return 0;
    }

    my_objstats[StatId].id = 0;

    /* stat will deleted by STATS when closing request is ack */

    return 1;
}


/** Set stat value.
 *
 * Modifies a variable value.
 *
 * @param StatId Id of the variable
 * @param Value Pointer to user data
 * @param Size In elements, depending on type.
 *
 * @returns 1 ok
 * @returns 0 error
 */
int SetStatsValue(int StatId, void * Value, int Size)
{
    if (!cpu.enable_stats)
    	return 1;

/*    printf("SetStatsValue():my_stats[%d].id=%d\n", StatId, my_stats[StatId].id);*/

    return hwapi_var_writets(my_stats[StatId].id, Value, Size * my_stats[StatId].elemsz);
}

/** Get stat value.
 *
 * Obtains a variable value. Value is saved in
 * User buffer.
 *
 * @param StatId Id of the variable
 * @param Buffer Pointer to buffer where data must be saved.
 * @param BuffElemSize Size (in elems) of the user-buffer
 *
 * @returns 1 ok
 * @returns 0 error
 */
int GetStatsValue(int StatId, void * Buffer, int BuffElemSize)
{
    if (!cpu.enable_stats)
    	return 1;

    return hwapi_var_read(my_stats[StatId].id, Buffer, BuffElemSize * my_stats[StatId].elemsz);
}



/**
 */
int SetObjectStatsValue(int StatId, void * Value, int Size)
{
    struct hwapi_cpu_i cpu;
    hwapi_hwinfo_cpu(&cpu);

    if (cpu.enable_stats) {
        return 1;
    }

    if (hwapi_var_getopt(my_objstats[StatId].id)) {
        printf("SWAPI: Error setting object stat. A value is still in queue\n");
        return 0;
    }

    hwapi_var_writets(my_objstats[StatId].id, Value, Size * my_stats[StatId].elemsz);

    hwapi_var_setopt(my_objstats[StatId].id, VAR_IS_SETOBJECT);

    return 1;
}


/**@} */





/**@defgroup logs Logs
 *
 * Logs are text files where the object can print strings and are kept during the
 * whole execution of the waveform. They are saved in logs directory under phal-repositories
 *
 * They must be explicity created by the object before using it (CreateLog)
 *
 * @warning Currently only one log is supported
 *
 * @{
 */

/** Creates a log
 *
 * Creates and allocates necessary resources for using a log file.
 *
 * @returns > 0 Log identifier
 * @returns -1 if error
 */
int CreateLog()
{
    int stat_id, opt;
    int count = 0;

    /*printf("\nCreateLog\n");*/

    if (!cpu.enable_logs)
    	return 1;

    if (log_created) {
        xprintf("SWAPI: Caution in %s: only one log per object is allowed\n",my_obj.obj_name);
        return log_created;
    }

    sprintf(aux_string, "%s:%s", my_obj.app_name, my_obj.obj_name);
    stat_id = hwapi_var_create(aux_string, LOG_LINE_SZ,
            VAR_IS_LOGINIT);
    if (stat_id < 0) {
        xprintf("SWAPI: Error creating log: Error creating stat.\n");
        return 0;
    }

    /* wait to be initiated */
    do {
        opt = hwapi_var_getopt(stat_id);
        if (opt != VAR_IS_LOGINITOK && opt != VAR_IS_LOGINITERR) {
            hwapi_relinquish(1);
            count++;
        }
    } while (opt != VAR_IS_LOGINITOK && opt != VAR_IS_LOGINITERR);
    if (opt != VAR_IS_LOGINITOK) {
        xprintf("SWAPI: Error creating log: Invalid response.\n");
        hwapi_var_delete(stat_id);
        return 0;
    }

    log_created = stat_id;

    return stat_id;
}

/** Close a Log.
 *
 * Closes and deallocates log resources for a given log.
 *
 * @param LogId Log Identifier to close
 * @returns 1 ok
 * @returns 0 error
 */
int CloseLog(int LogId)
{
    if (!cpu.enable_logs)
    	return 1;

    if (!LogId) {
        xprintf("SWAPI: Invalid LogId %d\n", LogId);
        return 0;
    }

    if (!hwapi_var_setopt(LogId, VAR_IS_LOGCLOSE)) {
        xprintf("SWAPI: Error closing log.\n");
        return 0;
    }

    log_created = 0;

    return 1;
}

/** Log
 *
 * Write formated string to default log (1)
 */
void Log(const char *format, ...)
{
    if (!cpu.enable_logs)
    	return;

    va_list args;
    va_start(args,format);
    vsnprintf(aux_string,256,format,args);
    if (log_created)
        WriteLog(1,aux_string);
    else
        printf(format,args);
}


/** LogWrite
 *
 * Writes a character string to a log
 *
 * @warning String must ends with a '\0'
 *
 * @param LogId Log identifier
 * @param str String to write
 *
 * @returns 1 ok
 * @returns 0 error
 */
int WriteLog(int LogId, char *str)
{
    struct hwapi_proc_i obj;

    if (!cpu.enable_logs)
    	return 1;

    assert(str);

    hwapi_proc_myinfo(&obj);

    if (!*str) {
        return 0;
    }

    /* check line is shorter than maximum allowed line */
    if (strlen(str) > LOG_LINE_SZ - 50) {
        xprintf("SWAPI: Error logging, maximum line length exceed (%d>%d)\n", strlen(str), LOG_LINE_SZ);
        return 0;
    }

    sprintf(timestr, "\n[%03d]:\t", obj.obj_tstamp);

    /* if space not avaiable in current buffer
     * send and save in the other */
    if (strlen(str) + strlen(timestr) + logoffset >= LOG_LINE_SZ) {
        if (hwapi_var_getopt(LogId) == VAR_IS_LOGWRITE) {
            xprintf("SWAPI: [%s]: CAUTION! Missed log due to full buffer at %d\n", my_obj.obj_name, get_tstamp());
        }
        flush_logs();

    }
    
    memcpy(&logbuffer[logoffset], timestr, strlen(timestr));
    logoffset += strlen(timestr);

    memcpy(&logbuffer[logoffset], str, strlen(str));

    logoffset += strlen(str);

    return 1;
}

/**@} */

void flush_logs(void)
{
    if (!cpu.enable_logs)
    	return;

    if (logoffset > 0) {
        logbuffer[logoffset] = '\0';
        logoffset++;
        if (hwapi_var_write(log_created, logbuffer, logoffset) < 0) {
            xprintf("SWAPI: Error writting to log: Invalid id (%d)\n", log_created);
            return;
        }

        hwapi_var_setopt(log_created, VAR_IS_LOGWRITE);
        logbuffer = (logbuffer == logbuffer1) ? logbuffer2 : logbuffer1;
        logoffset = 0;
    }
}

/*////////////////////GLOBAL VARIABLES////////////////////////*/
void ListPublicStats(int nof_listedstats){

	printf("ListPublicStats(): List of available Stats and their current parameters\n");
	hwapi_var_printlist(nof_listedstats);

}



int GetPublicStatID(char *app_name, char *obj_name, char *StatName){

	int ID=0;

	sprintf(aux_string, "%s:%s:%s", app_name, obj_name, StatName);
/*	sprintf(aux_string, "%s:%s:%s", my_obj.app_name, my_obj.obj_name, StatName);*/
/*	printf("GetPublicStatID: %s\n", aux_string);*/
	ID=hwapi_var_getID(aux_string);

	return(ID);
}

/*Vars name show the app and module that created them*/
/* To create Stats in a different object than the current one*/
int CreateRemotePublicStat(char *app_name, char *ObjectName,char *StatName, int StatType, int datalength){

    int type;
    int sizetype;
    int stat_id, opt, count=0;

    printf("CreateGlobalStat: %s\n", StatName);

	if (!cpu.enable_stats)return 1;
    sizetype=sizeoftype(StatType);
    if (!sizetype) {
    	xprintf("SWAPI: Error initiating stat %s:%s\n",ObjectName, StatName);
    	return -1;
    }

	sprintf(aux_string, "%s:%s:%s", app_name, ObjectName, StatName);
	printf("CreatePublicStat(): aux_string=%s\n", aux_string);

/*	stat_id = hwapi_var_create(aux_string, datalength*sizetype, 0);*/
	stat_id = hwapi_var_create2(aux_string, datalength*sizetype, 0, StatType);
	if (stat_id < 0) {
	     xprintf("SWAPI: Error initiating stat.\n");
	     return -1;
	}
	hwapi_var_setopt(stat_id, 99);
	printf("CreatePublicStat(): StatName=%s, StatIdx=%d\n", aux_string, stat_id);
	return(stat_id);

}
/* To create a public stat from the current object*/
/* This should be the normal behaviour. Each object publish the Stats he want to make available*/
int CreateLocalPublicStat(char *StatName, int StatType, int datalength){

    int type;
    int sizetype;
    int stat_id, opt, count=0;

/*    printf("CreatePublicStat: %s\n", StatName);*/

	if (!cpu.enable_stats)return 1;
    sizetype=sizeoftype(StatType);
    if (!sizetype) {
    	xprintf("SWAPI: Error initiating stat %s:%s\n",GetObjectName(), StatName);
    	return -1;
    }

	sprintf(aux_string, "%s:%s:%s", GetAppName(), GetObjectName(), StatName);
/*	printf("CreatePublicStat(): aux_string=%s\n", aux_string);*/

/*	stat_id = hwapi_var_create(aux_string, datalength*sizetype, 0);*/
	stat_id = hwapi_var_create2(aux_string, datalength*sizetype, 0, StatType);
	if (stat_id < 0) {
	     xprintf("SWAPI: Error initiating stat.\n");
	     return -1;
	}
	hwapi_var_setopt(stat_id, 99);
	printf("CreatePublicStat(): StatName=%s, StatIdx=%d\n", aux_string, stat_id);
	return(stat_id);

}


/* dataLength in number of typed data*/
int SetPublicStatsValue(int StatId, void * Value, int datalength){
	int size;
	int writelength=0;
	int type;
	int typesz=0;

    if (!cpu.enable_stats)
    	return 1;

    type=hwapi_var_gettype(StatId);
    typesz=sizeoftype(type);

/*    size=hwapi_var_getsz(StatId)*typesz;*/
    size=datalength*typesz;
/*    printf("SetPublicStatsValue(): size=%d\n", size);*/
    writelength=hwapi_var_write(StatId, Value, size)/typesz;

/*    printf("SetPublicStatsValue():writelength(in bytes)=%d\n", writelength);*/
/*    hwapi_var_setopt(StatId, VAR_IS_SETOBJECT);*/
    return(writelength);
}

int GetPublicStatsValue(int StatId, void * Buffer, int datalength){
	int type;
	int typesz;
	int bytesreadedlength;

    if (!cpu.enable_stats)
    	return 1;

    type=hwapi_var_gettype(StatId);
    typesz=sizeoftype(type);

    bytesreadedlength=hwapi_var_read(StatId, Buffer, datalength*typesz);
/*    printf("GetPublicStatsValue():bytesreadedlength(in bytes)=%d\n",bytesreadedlength);*/
    return(bytesreadedlength/typesz);
}



