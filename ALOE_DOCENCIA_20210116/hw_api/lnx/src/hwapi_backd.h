#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "dac_cfg.h"

/*
#define DISABLE_LOGS
*/
#define MSG_BUFF_SZ             (10*1024*1024)
#define MAX_INT_ITF             1024
#define MSG_QBYTES		20*MSG_BUFF_SZ


/*#define DISABLE_STATUS_SEMID
*/
#define DISABLE_MEM_SEMID

/* kernel run on this signal */
#define SIG_KERNEL_SCHED	SIGRTMIN

/** sends command to hwapi process */
#define SIG_PROC_CMD		(SIGRTMIN+2)

#define PROCCMD_FORCEREL	1
#define PROCCMD_RTFAULT		2
#define PROCCMD_PERIODTSK	3
#define PROCCMD_ITFCALLBACK 4

#define ENABLE_DAC

#define SCHED_PRIO              50
#define DAEMONS_PRIO			10
#define RUNPH_PRIO				90
#define RUNPH_CMD_PRIO			10
#define RUNPH_DAC_PRIO			60

#define BASE_PRIO				55
#define ITF_PRIO				55

#define DEFAULT_INTERNAL_BW	10000000

#define TS_ERROR_FRAC           10

#define FILE_SEPARATOR_CH       '/'

/* default path for daemons config file */
#define DEFAULT_DAEMONS_FILE    "/usr/local/etc/platform.conf"

/* default path for xitf config file */
#define DEFAULT_XITF_FILE       "/usr/local/etc/xitf.conf"

/* default path for sched config file */
#define DEFAULT_SCHED_FILE       "/usr/local/etc/scheduling.conf"


/* base phal path */
#define DEFAULT_RES_PATH        "/etc/phal/"

/* path for lock files */
#define LOCK_FILE               "/tmp/phdefault.lock"

/* path to downloaded new processes */
#define RCV_PROC_PATH           "/tmp/"

#define VALGRIND_CMD			"valgrind"
#define VALGRIND_ARG			"--tool"
#define VALGRIND_DEFAULT_TOOL	"callgrind"

#define DEFAULT_CPU_MIPS	100

#define DEF_FILE_LEN            SYSTEM_STR_MAX_LEN


/* Shared memory configuration */
#define HW_API_SHM_KEY          0xAA
#define HW_API_MSG_KEY          0x11
#define DATA_MSG_KEY            0x22



/** structure for keeping path of daemons to launch
 */
struct launch_daemons
{
  char path[SYSTEM_STR_MAX_LEN];
  char output[SYSTEM_STR_MAX_LEN];
  int pid;
  time_t cpu[3];
  int prio;
  int cpuid;
  int valgrind;
  int sem_idx;
  int runnable;
};
#define MAX_DAEMONS             11

#define MAX_EXT_ITF 10

/* external network interfaces config db */
struct ext_itf {
	xitfid_t id;	
	int mode;	
	int port;
	char address[15];
	int remote_udp_connected;
	struct sockaddr remote_udp_addr;
	int pid;
	int bpid;
	int fsock;
	int fd;
	int in_mbox_id;
	int out_mbox_id;
	hwitf_t in_key_id;
	hwitf_t out_key_id;
	int type;
	int int_itf_idx;
	int fragment;
	struct hwapi_xitf_i info;
};



/** Network Database
*/
struct net_db {
	int nof_ext_itf;
	int base_prio;
	int itf_prio;
	int base_cpuid;
	int itf_cpuid;
	pid_t net_chld;
	char full_bridge;
	char litendian;
	struct ext_itf ext_itf[MAX_EXT_ITF];
};


/* database sizes */
#define MAX_CORES               32		//CHANGE 270515: 8
#define MAX_PROCS               100
#define MAX_VARS                2000


#define VAR_TABLE_SZ		(500*1024)

#define ITF_OPT_EXTERNAL	0xF00
#define ITF_OPT_BLOCKING      0xF000


#define DEFAULT_OUTPUTLOG "output.log"
#define RESOURCE_DIR	"example-repository/"

/** internal itf database definition */
struct file_des {
    int int_itf_idx;
    int id;
    int mode;
    int pending_data;
    int pending_rpm;
    int *pending_pkt;
};

/** number of internal interfaces (for a single process) */
#define MAX_FILE_DES	100

struct itf_local_callback {
	int fd;
	int arg;
	int (*callback) (int,int);
	int itf_callback_idx;
};

#define MAX_ITF_LOCAL_CALLBACK 8

struct itf_callback {
	int pid;
	int local_idx;
};

#define MAX_ITF_CALLBACK	4

/** Internal interface database
*/
struct int_itf {
	int id;		
	hwitf_t key_id;
	char r_obj[SYSTEM_STR_MAX_LEN];	
	char w_obj[SYSTEM_STR_MAX_LEN];	
	char r_itf[SYSTEM_STR_MAX_LEN];
	char w_itf[SYSTEM_STR_MAX_LEN];
	int opts;	
	hwitf_t remote_id;
	int delay;
	int network_pid; 
	int network_bpid; 
	int callback_pid; 
	void (*callback) (hwitf_t);
	int hascallback;
	struct itf_callback itf_callback[MAX_ITF_CALLBACK];
};

struct proc_time {
    time_t sys_syscpu;
    time_t sys_usrcpu;
    time_t lst_syscpu;
    time_t lst_usrcpu;
    int scnt;
    int ucnt;
    int last_cntxt;
};


struct setup_dac_cmd {
	int is_tx;
	int freq;
};


struct period_task {
	void (*fnc) (int);
	int period;
	int cnt;
	int pid;
};
#define MAX_PERIOD_TASKS	5


struct phtime {
	unsigned int tslot;
	int new_tstamp;
	int next_ts;
	time_t init_time;
	time_t slotinit;
};

/** Common HW-API database */
struct hw_api_db
{
	struct phtime time;

  /** network db (see below */
  struct net_db net_db;

  /** daemons structure */
  struct launch_daemons daemons[MAX_DAEMONS];

  /** internal interfaces  */
  struct int_itf int_itf[MAX_INT_ITF];

  /** process database */
  struct hwapi_proc_i proc_db[MAX_PROCS];

  int last_prio[MAX_CORES];

  struct proc_time proc_time[MAX_PROCS];
  int max_proc_id;

  /** variable database */
  struct hwapi_var_i var_db[MAX_VARS];

  int max_var_id;

  struct period_task period_tasks[MAX_PERIOD_TASKS];

  /* variable data contents */
  char var_table[VAR_TABLE_SZ];

  /** cpu information */
  struct hwapi_cpu_i cpu_info;

  int data_msg_id;
  int api_msg_id;
  int shm_id;

  int sync_ts;

  int exec_pid;
  int runph_pid;

  int exec_slot_div;
  int memCnt;
  int flushout;
  int printbt_atexit;
  int printbt_atterm;
  int default_delay;
  int external_delay;
  int numsamples;
  struct dac_cfg dac;
  struct dac_interface *dac_itf;
  int dac_drives_clock;
  int sync_drives_clock;

  sem_t sem_dac;
  sem_t sem_var;
  sem_t sem_status;
  sem_t sem_daemons[MAX_DAEMONS];
  sem_t sem_objects[MAX_CORES][MAX_PROCS];
  int last_semidx[MAX_CORES];

  int nof_sem_daemons;

  char lock_file[DEF_FILE_LEN];

  int run_as_daemon;
  char output_file[DEF_FILE_LEN];
  char report_file[DEF_FILE_LEN]; 
  char res_path[DEF_FILE_LEN]; 

  char sched_cfg[DEF_FILE_LEN];
  char xitf_cfg[DEF_FILE_LEN];

  char valgrind_objects[SYSTEM_STR_MAX_LEN];
  char valgrind_tool[SYSTEM_STR_MAX_LEN];

  int nof_daemons;

  int obj_prio;
  int kernel_prio;
  int kernel_cpuid;
  int kernel_cmd_prio;
  int kernel_cmd_cpuid;
  int kernel_dac_prio;
  int kernel_dac_cpuid;
  int exec_prio;
  int runph_grp;
  char core_mapping[MAX_CORES];

  float reserve_cores[MAX_CORES];

};

#define MSG_MAGIC	0x1321



/** Runph Daemon message commands
 */
#define HWD_SETUP_XITF			0x0
#define HWD_CREATE_XITF			0x1
#define HWD_DAEMON_INIT			0x2
#define HWD_CREATE_PROCESS		0x4
#define HWD_READ_XITF_CFG		0x6
#define HWD_RUN_PROCESS			0x7
#define HWD_SETUP_DAC			0x3

/* some constants for internal messaging
*/
#define TSTAMP_PKT_BITS                 4
#define TSTAMP_PKT_MASK                 0xF
#define TSTAMP_MAX_RED                  15


/* packet message header */
struct msghead {
    long dst;   /* message type, must be > 0 */
    long src;
    int opts;
    int tstamp;
};


/** message buffer structure */
struct msgbuff {
   struct msghead head;
   char body[MSG_BUFF_SZ];
};

#define MSG_SZ                  (sizeof(struct msgbuff))
#define MSG_HEAD_SZ		(sizeof(struct msghead))

#ifdef ENABLE_DAC
void close_dac();
int configure_dac (struct dac_interface *dac_itf, struct dac_cfg *dac, int *tslot_len_ptr);
#endif



#define printv(a) \
	if (verbose) printf(a)

#define printva(a,b) \
	if (verbose) printf(a,b)

#define trace(a) \
	if (trace) trace_printk(a)



void print_frame(char *title);
void clear_files();
void clear_zombies();
int clear_shm();
void remove_file(int i);
char Answer_command(int cmd, char *packet, int len);
void launch_daemons();
void kill_chlds();
void clear_zombies();
void remove_all_msg();
void syncTslot ();
int kernel_commands_thread();
void run_daemon(char *output);

