/*
 * phal_hw_api.h
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
 
#ifdef __cplusplus 
extern "C" {
#endif
 
/* platform specific definitions */
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "phid.h"
#include <setjmp.h>



#define RUN_AS_PROC	/**< Define to load daemons as stand-alone processes */

/*#define RUN_NONBLOCKING */ /**< Define to run some daemons in non-blocking mode */


/** Platform Id's */
#define PLATFORM_LIN          0x10
#define PLATFORM_LIN_X86      0x11
#define PLATFORM_LIN_X86_64   0x12
#define PLATFORM_LIN_ARM      0x13
#define PLATFORM_LIN_PPC      0x14

#define PLATFORM_TI           0x20
#define PLATFORM_TI_6455      0x21
#define PLATFORM_TI_6416      0x22

#define MTRACE

#define xprintf printf	
#define time_t struct timeval
#define getpid getpid
/* end */

#define SYSTEM_STR_MAX_LEN  129


/* memory */
#define  NEW(p) ((p) = hwapi_mem_alloc((long)sizeof *(p)))
#define  DELETE(p) (hwapi_mem_free(p))

/** Header for sending data through external interfaces */
typedef struct packet_header_t {
    int id;
    int opts;
    int size;
    int tstamp;
} packet_header_t;
#define PACKET_MAGIC            0x1234F1F6
#define PACKET_HEADER_SZ	(sizeof (packet_header_t))


/* interface constants */
#define FLOW_READ_ONLY	1
#define FLOW_WRITE_ONLY	2
#define FLOW_READ_WRITE	3
#define FLOW_WRITE_READ	4

#define SCHED_ORDERED 1
#define SCHED_REVERSE 0


/** Structure For CPU Information
 * @todo Define other RT or monitoring info
 */
#define CPU_NAME_LEN    32

struct hwapi_cpu_i {
    char name[CPU_NAME_LEN];
    int pe_id;
    float C; /**< Computing Capacity (in MOPs) */
    float intBW; /**< Internal memory BW (in Mbps) */
    int status; /**< status flag */
    int tslen_usec; /**< Duration of the time slot (in usec) */
    int plat_family; /**< Platform family identification */
    int debug_level; /**< Platform debug level */
    int enable_stats;
    int enable_logs;
    int cpu_usage;
    int total_fifo;
    int used_fifo;
    int nof_cores;
    int print_rtfaults;
    int kill_rtfaults;
    int relinq_rtfaults;
    int trace;
    int verbose;
    int exec_order;
    int print_itf_pkt;
};

/** Structure for External interfaces Information
 */
struct hwapi_xitf_i {
	xitfid_t id; /**< Id of the interface */
    char mode; /**< r/w mode */
    short delay; /**< delay (in usec) */
    float BW; /**< BW in Mbps */
};

/** Object status structure
 */
struct hwapi_proc_status {
    int cur_status;
    int next_status;
    int next_tstamp;
};

/** Process Information/Monitoring Structure
 */
struct hwapi_proc_i {
    char obj_name[SYSTEM_STR_MAX_LEN];
    char app_name[SYSTEM_STR_MAX_LEN];
    char path[SYSTEM_STR_MAX_LEN];
    int app_id;
    int obj_id;
    int pid;
    int cur_tstamp;
    int rel_tstamp;
    int obj_tstamp;
    int change_progress;
    int exec_position;
    int relinquished;
    int sem_idx;
    int prio;
    int core_idx;
    time_t tdata[3];
    int sys_period;
    int sys_cpu;
    int sys_start;
    int sys_end;
    int sys_mem;
    int sys_nvcs;
    char rt_fault;
    struct hwapi_proc_status status;
};

/** Process loading properties
 */
struct hwapi_proc_launch {
    int app_id;
    int obj_id;
    char *obj_name;
    char *app_name;
    char *exe_name;
    int blen;
    int wlen;
    /* only required by absolute-addressing computers */
    unsigned int mem_st;
    unsigned int mem_sz;
    unsigned int stack_st;
    unsigned int stack_sz;
    int usec_limit;
};


/** Variable structure information
 */
#define VAR_NAME_LEN	SYSTEM_STR_MAX_LEN

struct hwapi_var_i {
    char name[VAR_NAME_LEN];
    objid_t obj_id;
    varid_t stat_id;
    varsz_t size;
    int cur_size;
    int opts;
    int type;
    int table_offset;
    int write_tstamp;
};




/* function prototypes */

int hwapi_init(void);
void hwapi_exit();
void hwapi_atterm(void (*fnc)(void));
void hwapi_relinquish(int slots);
int hwapi_addperiodfunc(void (*fnc) (int),int period);
int hwapi_rmperiodfunc(void (*fnc) (int));
int hwapi_setperiod(void (*fnc) (int),int period);
void hwapi_relinquish_daemon();


hwitf_t hwapi_itf_create(hwitf_t id, int size);
int hwapi_itf_avail();
int hwapi_itf_delete(hwitf_t id);
int hwapi_itf_delete_own(char *obj_name);
int hwapi_itf_attach(hwitf_t id, int mode);
int hwapi_itf_unattach(int fd, int mode);
int hwapi_itf_match(hwitf_t itf_id, char *w_obj, char *w_itf, char *r_obj, char *r_itf);
int hwapi_itf_snd(int fd, void *buffer, int size);
int hwapi_itf_rcv(int fd, void *buffer, int size);
int hwapi_itf_isexternal(hwitf_t id);
int hwapi_itf_link_phy(hwitf_t itf_id, xitfid_t phy_itf_id, char mode);
int hwapi_itf_addcallback(int fd, int (*callback) (int,int), int arg);
int hwapi_itf_rmcallback(int fd);
int hwapi_itf_setblocking(hwitf_t itf_id);
int hwapi_itf_list(char *w_obj, hwitf_t *ids, int max_itf);
int hwapi_itf_status_id(hwitf_t id);
int hwapi_itf_status(int fd);
hwitf_t hwapi_itf_find(char *obj_name, char *itf_name, char mode);


int hwapi_hwinfo_isDebugging();
int hwapi_hwinfo_xitf(struct hwapi_xitf_i *xitf, int max_itf);
void hwapi_hwinfo_cpu(struct hwapi_cpu_i *info);
void hwapi_hwinfo_setid(int pe_id);




int hwapi_proc_request(struct hwapi_proc_launch *pinfo);
int hwapi_proc_create(char *pdata, struct hwapi_proc_launch *pinfo, int exec_position, int core_idx);
int hwapi_proc_remove(int obj_id);
int hwapi_proc_kill(int obj_id);
int hwapi_proc_mystatus(void);
int hwapi_proc_status_new(int obj_id, int next_status, int next_tstamp);
int hwapi_proc_status_ack(int obj_id);
int hwapi_proc_status_get(void);
int hwapi_proc_info(int obj_id, struct hwapi_proc_i *pinfo);
int hwapi_proc_myinfo(struct hwapi_proc_i *pinfo);
int hwapi_proc_list(struct hwapi_proc_i **pinfo);
void hwapi_proc_mytstampinc();
void hwapi_proc_tstamp_set(objid_t obj_id, int tstamp);

void hwapi_var_printlist(int nof_vars);				/*ADDED DEC15*/
varid_t hwapi_var_getID(char *name);			/*ADDED DEC15*/
varid_t hwapi_var_create(char *name, int size, int opts);
varid_t hwapi_var_create2(char *name, int size, int opts, int type);
int hwapi_var_delete(varid_t stat_id);
int hwapi_var_list(int opts, struct hwapi_var_i *var_list, int nof_elems);
int hwapi_var_getptr(struct hwapi_var_i **var_list);
int hwapi_var_setopt(varid_t stat_id, int new_opt);
int hwapi_var_getopt(varid_t stat_id);
int hwapi_var_gettype(varid_t stat_id);
int hwapi_var_get_tstamp(varid_t stat_id);
int hwapi_var_getsz(varid_t stat_id);
int hwapi_var_readts(varid_t stat_id, void *value, int buff_sz, int *tstamp);
int hwapi_var_read(varid_t stat_id, void *value, int buff_sz);
int hwapi_var_write(varid_t stat_id, void *value, int value_sz);
int hwapi_var_writets(varid_t stat_id, void *value, int value_sz);


void hwapi_mem_init(void);
int hwapi_mem_disalloc(int block_sz);
int hwapi_mem_prealloc(int nof_block, int block_sz);
int hwapi_mem_used_chunks();
void * hwapi_mem_alloc(int size);
void hwapi_mem_free(void *ptr);
void hwapi_mem_silent(int silent);

int hwapi_dac_getSampleType();
double hwapi_dac_getInputFreq();
double hwapi_dac_getOutputFreq();
int hwapi_dac_setInputFreq(double freq);
int hwapi_dac_setOutputFreq(double freq);
int hwapi_dac_setInputLen(int nSamples);
int hwapi_dac_setOutputLen(int nSamples);
int hwapi_dac_getInputLen();
int hwapi_dac_getOutputLen();
int hwapi_dac_getMaxInputLen();
int hwapi_dac_getMaxOutputLen();
void *hwapi_dac_getOutputBuffer(int channel);
void *hwapi_dac_getInputBuffer(int channel);



void get_time_interval(time_t * tdata);
int get_time_to_ts();
int get_tstamp(void);
void set_tstamp(int tstamp);
void get_time(time_t * t);
void wait_sem(int sem_id);
void post_sem(int sem_id);
int wait_sem_n(int sem_id, int n);
int post_sem_n(int sem_id,int n);
void sleep_us(int usec);
void sleep_ms(int msec);
int time_to_tstamp(time_t *tdata);
void time_sync(int tstamp, int usec);
int get_time_to_ts(time_t *t);
void set_time(time_t * t);
void tstamp_to_time(int tstamp, time_t *tdata);

#ifdef __cplusplus 
}
#endif


