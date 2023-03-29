/*
 * exec_cmds.c
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

/* standard libraries */
#include <stdlib.h>
#include <string.h>
#include "phal_hw_api.h"
#include "phid.h"
#include "phal_daemons.h"

#include "pkt.h"
#include "set.h"
#include "daemon.h"

#include "set.h"
#include "str.h"
#include "rtcobj.h"
#include "phobj.h"
#include "app.h"

/** Enable this constant to print debugging messages
 */

#include "execx.h"

#include "exec_cmds.h"
#include "swman_cmds.h"
#include "phitf.h"

#define ENABLE_REPORTS

/** wait timeslots before printing rtfault again
 */
#define RTPRINT_GUARD_TS    20

/*#define DEB
*/
#define RTFAULT_PRINTEXEC

/*#define RTFAULT_PRINTEXTENDED
*/
#define SYNC_MARGIN 4

#define REL_RTFAULT
#define PRINT_RTCFAULT

#define NEW_STATUS_TOUT	20

#define MAX_PROCS 100
struct hwapi_proc_i hwapi_obj[MAX_PROCS];


struct st_db {
	int max_ts;
	char name[16];
};
struct st_db st_db[PHAL_STATUS_MAX];

void init_state_str();
int update_report_table();

#define REPORT_WINDOW	REPORT_INTERVAL
#define REPORT_PROCS	60

#define MAX_REPORT_ITF  5

struct report_data_itf {
	int usage;
};

struct report_data {
	int cpu_usec;
	int period;
	int sys_end;
	int sys_start;
	int tstamp;
	char executed;
};

#define REPORT_DATA_SZ sizeof(struct report_data)


struct report_obj {
	phobj_o obj;
	int hwapi_idx;
	int report_idx;
	int last_tstamp;
	int nof_itfs;
	hwitf_t itf_id[MAX_REPORT_ITF];
	phitf_o itf[MAX_REPORT_ITF];
	struct report_data report[REPORT_WINDOW];
};

int last_report_idx=0;
static struct report_obj reports[REPORT_PROCS];

Set_o report_apps = NULL;

struct hwapi_cpu_i cpu_info;

static daemon_o daemon;
static pkt_o pkt;

int lastprint;

#define NOF_OBJ	200			/*100 AGBABRIL20*/
#define NOF_ITF	1000		/*100 AGBABRIL20*/
#define NOF_STR	5000		/*1000 AGBABRIL20*/
#define NOF_VAR	500
#define NOF_APP 20			/*10 AGBABRIL20*/
#define NOF_SET	1000
#define NOF_SETMEM 2000		/*1000 AGBABRIL20*/

int last_msg_ts = 0;

void send_rtfault(int tstamp, time_t *tdata, struct hwapi_proc_i *proc, float C);
void exec_rtfault(int p_id);

void exec_alloc() {
#ifdef MTRACE
	mtrace();
#endif
	hwapi_mem_prealloc(NOF_OBJ, phobj_sizeof);
	hwapi_mem_prealloc(NOF_OBJ, rtcobj_sizeof);
	hwapi_mem_prealloc(NOF_ITF, phitf_sizeof);
	hwapi_mem_prealloc(NOF_STR, str_sizeof);
	hwapi_mem_prealloc(NOF_APP, app_sizeof);
	hwapi_mem_prealloc(NOF_SET, set_sizeof);
	hwapi_mem_prealloc(NOF_SETMEM, setmember_sizeof);
	hwapi_mem_prealloc(1, daemon_sizeof);
	hwapi_mem_prealloc(1, pkt_sizeof);
}

void exec_init(daemon_o d) {
	report_apps = Set_new(0, NULL);
	assert(report_apps);
	daemon = d;
	pkt = daemon_pkt(daemon);
	init_state_str();
	lastprint = 0;

	hwapi_rtfault_callback(exec_rtfault);

	hwapi_hwinfo_cpu(&cpu_info);
}

void exec_rtfault(int i) {
	struct hwapi_proc_i info;
	time_t tdata[3];

	hwapi_proc_info_idx(i,&info);

#ifdef RTFAULT_PRINTEXEC
	#ifdef RTFAULT_PRINTEXTENDED
	daemon_info(daemon,
			"REAL-TIME VIOLATION: Object %s did not execute at ts %d.\n\tObj TS=%d. Last Exec=%d:%d -> %d:%d Now=%d:%d\n",
			info.obj_name, tstamp, info.obj_tstamp,
			info.tdata[1].tv_sec, info.tdata[1].tv_usec,
			info.tdata[2].tv_sec, info.tdata[2].tv_usec,
			tdata[2].tv_sec, tdata[2].tv_usec);
	#else
	daemon_info(daemon,
			"REAL-TIME VIOLATION: Object %s did not execute at timeslot %d\n",
			info.obj_name, info.obj_tstamp);

	#endif
#else
	get_time(&tdata[2]);
	memcpy(&tdata[1],&info.tdata[1],sizeof(time_t));
	get_time_interval(tdata);
	send_rtfault(get_tstamp(), tdata, &info,cpu_info.C);
#endif

	if (cpu_info.kill_rtfaults) {
#ifdef DEB
		daemon_info(daemon, "Killing Object %s\n",info.obj_name);
#endif
		hwapi_proc_kill(info.obj_id);
	} else if (cpu_info.relinq_rtfaults) {
		hwapi_proc_relinquish(info.obj_id);
	}
}

int rmean(int *x) {
	int i;
	int res = 0;
	char *ptr = (char*) x;
	for (i = 0; i < REPORT_WINDOW; i++) {
		res += *((int*) ptr);
		ptr += REPORT_DATA_SZ;
	}

	return (int) res / REPORT_WINDOW;
}

int rmax(int *x) {
	int i;
	int max = -99999999;
	char *ptr = (char*) x;

	for (i = 0; i < REPORT_WINDOW; i++) {
		if (*((int*) ptr) > max) {
			max = *((int*) ptr);
		}
		ptr += REPORT_DATA_SZ;
	}

	return max;
}


int rmin(int *x) {
	int i;
	int min = 99999999;
	char *ptr = (char*) x;

	for (i = 0; i < REPORT_WINDOW; i++) {
		if (*((int*) ptr) < min) {
			min = *((int*) ptr);
		}
		ptr += REPORT_DATA_SZ;
	}

	return min;
}

void save_exec_stats(struct hwapi_proc_i *procs, int max_procs, int tstamp) {
	int i, j, k;
	app_o app;

	for (i = 0; i < last_report_idx; i++) {
		if (reports[i].obj) {
			k = reports[i].hwapi_idx;
			if (procs[k].obj_id != reports[i].obj->obj_id) {
#ifdef DEB
				daemon_info(daemon, "Caution, object 0x%x not reporting", reports[i].obj->obj_id);
#endif
				update_report_table();
				break;
			} else {
#ifdef ENABLE_REPORTS
				if (tstamp >= reports[i].last_tstamp + 1) {
					reports[i].report[reports[i].report_idx].cpu_usec = procs[k].sys_cpu;
					reports[i].report[reports[i].report_idx].period = procs[k].sys_period;
					reports[i].report[reports[i].report_idx].sys_end = procs[k].sys_end;
					reports[i].report[reports[i].report_idx].sys_start = procs[k].sys_start;
					reports[i].report[reports[i].report_idx].executed = 1;
					/*reports[i].report[reports[i].report_idx].tstamp = procs[k].cur_tstamp==tstamp?procs[k].obj_tstamp-1:procs[k].obj_tstamp;
					*/reports[i].last_tstamp = tstamp;
				}
#endif
				reports[i].report_idx++;
				if (reports[i].report_idx == REPORT_WINDOW) {
					reports[i].report_idx = 0;
#ifdef REP_ITF
					for (j = 0; j < reports[i].nof_itfs; j++) {
						reports[i].itf[j]->fifo_usage = hwapi_itf_status_id(
								reports[i].itf_id[j]);
						if (reports[i].itf[j]->fifo_usage < 0) {
#ifdef DEB
							daemon_info(daemon, "Caution, object 0x%x not reporting (itf failed)", reports[i].obj->obj_id);
#endif
							update_report_table();
							break;
						}
					}
#endif
#ifdef ENABLE_REPORTS
/** if reports are enabled, send averaged values to execinfo command */

					if (procs[k].cur_tstamp==tstamp)
						reports[i].obj->rtc->tstamp = procs[k].obj_tstamp-1;
					else
						reports[i].obj->rtc->tstamp = procs[k].obj_tstamp;
					reports[i].obj->rtc->rep_tstamp = tstamp;
					reports[i].obj->rtc->nvcs = procs[k].sys_nvcs;
					reports[i].obj->rtc->cpu_usec = rmean(&reports[i].report[0].cpu_usec);
					reports[i].obj->rtc->mean_period = rmean(&reports[i].report[0].period);
					reports[i].obj->rtc->max_usec = rmax(&reports[i].report[0].cpu_usec);
					reports[i].obj->rtc->mean_mops = (reports[i].obj->rtc->cpu_usec * cpu_info.C/1000);
					reports[i].obj->rtc->max_mops = (reports[i].obj->rtc->max_usec * cpu_info.C/1000);
					reports[i].obj->rtc->start_usec = rmean(&reports[i].report[0].sys_start);
					reports[i].obj->rtc->max_end_usec = rmax(&reports[i].report[0].sys_end);
					reports[i].obj->rtc->end_usec = rmean(&reports[i].report[0].sys_end);
					reports[i].obj->core_idx = procs[k].core_idx;

/** send instantanteus values if reports are disabled */
#else
					reports[i].obj->rtc->nvcs = procs[k].sys_nvcs;
					reports[i].obj->rtc->cpu_usec = procs[k].sys_cpu;
					reports[i].obj->rtc->mean_period = procs[k].sys_period;
					reports[i].obj->rtc->max_usec = procs[k].sys_cpu;
					reports[i].obj->rtc->var_cpu = procs[k].sys_start;
					reports[i].obj->rtc->mean_mac
							= (int) reports[i].obj->rtc->cpu_usec * cpu_info.C;
					reports[i].obj->rtc->max_mac
							= (int) reports[i].obj->rtc->max_usec * cpu_info.C;
					reports[i].obj->rtc->start_usec = procs[k].sys_end;
					reports[i].obj->rtc->end_usec = procs[k].sys_end;
					reports[i].obj->core_idx = procs[k].core_idx;
#endif
					}
			}
		}
	}

	if (!report_apps)
		return;

	pkt_clear(pkt);
	for (i = 0; i < Set_length(report_apps); i++) {
		app = Set_get(report_apps, i);
		assert(app);
		if (tstamp - app->next_pending_tstamp > REPORT_INTERVAL) {
			app->report_tstamp = tstamp;
			pkt_put(pkt, FIELD_APP, app, app_topkt);
			daemon_sendto(daemon, SWMAN_APPINFOREPORT, 0, DAEMON_SWMAN);
			app->next_pending_tstamp = tstamp;
			return;
		}
	}
}

int update_report_table() {
	int i, j, k, l, n, m, p;
	phobj_o obj;
	phitf_o itf;
	app_o app;
	int max_procs;
	struct hwapi_proc_i *hwapi_obj;

	memset(reports, 0, sizeof(struct report_obj) * REPORT_PROCS);
	last_report_idx=0;
	max_procs = hwapi_proc_list(&hwapi_obj);
	l = 0;
	for (k = 0; k < Set_length(report_apps); k++) {
		app = Set_get(report_apps, k);
		assert(app);

		for (i = 0; i < Set_length(app->objects); i++) {
			obj = Set_get(app->objects, i);
			assert(obj);
			j = 0;
			while (j < max_procs && hwapi_obj[j].obj_id != obj->obj_id)
				j++;
			if (j < max_procs) {
				reports[l].obj = obj;
				reports[l].hwapi_idx = j;
				p = 0;
				for (m = 0; m < Set_length(obj->itfs) && m < MAX_REPORT_ITF; m++) {
					itf = Set_get(obj->itfs, m);
					assert(itf);
					if (itf->mode & FLOW_WRITE_ONLY || itf->xitf_id != 0) {
						reports[l].itf[p] = itf;
						reports[l].itf_id[p] = hwapi_itf_find(str_str(
								obj->objname), str_str(itf->name), itf->mode);
						p++;
					}
				}
				reports[l].nof_itfs = p;
				l++;
				if (l>last_report_idx) {
					last_report_idx=l;
				}
			}
		}
	}
	return 1;
}

int remove_report_idx(int idx) {
	int j;

	memset(&reports[idx], 0, sizeof(struct report_obj));
	return 1;
}

/** Starts or stops reporting of objects execution statistics. 
 * 'app' here does not refer to a waveform but to a group of objects. This group will be used
 * for reporting and for starting/stoppping. So the field app_name in the entity app does not need 
 * to refer to a real waveform name, can be anyone, but unique.
 */
int exec_incmd_reportcmd(cmd_t *cmd) {
	int start;
	app_o app_pkt, app_local;

	pkt = daemon_pkt(daemon);

	start = pkt_getcmd(pkt) == EXEC_REPORTSTART;

	app_pkt = pkt_get(pkt, FIELD_APP, app_newfrompkt);
	assert(app_pkt);

	if (start) {
		app_local = Set_find(report_apps, app_pkt->name, app_findname);
		if (app_local) {
			daemon_error(daemon, "Group %s already reporting", str_str(
					app_pkt->name));
			app_delete(&app_pkt);
			return 0;
		}
		Set_put(report_apps, app_pkt);
#ifdef DEB
		daemon_info(daemon, "Added group %s to reports", str_str(app_pkt->name));
#endif
		if (!update_report_table()) {
			daemon_info(daemon, "Error in table.");
			return 0;
		}
	} else {
#ifdef DEB
		daemon_info(daemon, "Stopping report for app %s", str_str(app_pkt->name));
#endif
		app_local = Set_find(report_apps, app_pkt->name, app_findname);
		if (!app_local) {
			daemon_error(daemon, "Unknown object group %s", str_str(
					app_pkt->name));
			app_delete(&app_pkt);
			return 0;
		}
		Set_remove(report_apps, app_local);
		app_delete(&app_local);
		app_delete(&app_pkt);
		update_report_table();
	}

	return 1;
}

/** Processing Function
 *
 * REQUIRED PACKET FIELDS:
 *	- FIELD_OBJID: Id of the object to change the status.
 *	- FIELD_STATUS: New status for the object.
 *
 * ACTIONS:
 *	- Uses hw_api functions to set new status.
 *	- The previous function will return -1 if can't change stauts of the object because
 * either it does not exists or it is already in a change procedure.
 *
 * OUTPUT:
 *	- SWMAN_STATUSOK command if successfull
 *	- SWMAN_STATUSERR command if error
 */
int exec_incmd_setstatus(cmd_t *cmd) {
	status_t status;
	objid_t obj_id;
	int tstamp;
	struct hwapi_proc_i proc;

	status = (status_t) pkt_getvalue(pkt, FIELD_STATUS);
	tstamp = (int) pkt_getvalue(pkt, FIELD_TSTAMP);
	obj_id = (objid_t) pkt_getvalue(pkt, FIELD_OBJID);

	/* if received abnormal stop */
	if (status == PHAL_STATUS_ABNSTOP) {
		tstamp = get_tstamp() - 1;
#ifdef DEB
		daemon_info(daemon, "Abnormal STOP for object 0x%x", obj_id);
#endif
	}

	if (hwapi_proc_status_new(obj_id, status, tstamp) <= 0) {
		if (status != PHAL_STATUS_ABNSTOP) {
			daemon_error(daemon,
					"Can't change status for object 0x%x. Killing process...",
					obj_id);
		}
		if (!hwapi_proc_info(obj_id, &proc)) {
#ifdef DEB
			daemon_error(daemon, "Unknown object 0x%x", obj_id);
#endif
			return 0;
		}
		exec_statusrep(&proc, 0);
		hwapi_proc_kill(obj_id);
	} else {

#ifdef DEB
		daemon_info(daemon, "Set new status %d for object %d at tstamp %d", status, obj_id, tstamp);
#endif
	}

	/* *a successful change will be reported by exec_run() function when it detects it has been correctly
	 realized */
	return 1;
}

/** @defgroup sensors_exec EXEC Sensor Monitor Function
 *
 * EXEC Sensor, after processing all possible input commands, has the tasks to monitor
 * the object execution status and report changes or fault to manger (SW MANAGER).
 *
 * Status succesfull or error change report is executed by exec_statusrep function.
 *
 * @ingroup sensors
 *
 * @{
 */

/** Exec Status Report Function
 *
 * This function builds SWMAN_STATUSOK or SWMAN_STATUSERR commands and sends them. It indicates the succesfull or
 * error in an status change procedure for an object
 * @todo appid
 *
 * @param obj Pointer to hwapi_proc_i structure of the object
 * @param ok 1 for STATUSOK 0 for STATUSERR
 *
 * @return 1 if send ok
 * @return -1 if send error
 */
int exec_statusrep(struct hwapi_proc_i *obj, int ok) {
	assert(daemon && obj);
	pkt_clear(pkt);

	pkt_putvalue(pkt, FIELD_APPID, (uint) obj->app_id);
	pkt_putvalue(pkt, FIELD_OBJID, (uint) obj->obj_id);
	pkt_putvalue(pkt, FIELD_STATUS, (uint) obj->status.cur_status);

	daemon_sendto(daemon, ok ? SWMAN_STATUSOK : SWMAN_STATUSERR, 0,
			DAEMON_SWMAN);

#ifdef DEB
	daemon_info(daemon, "Sent status %s %d for object %s", ok ? "ok" : "error", obj->status.cur_status,
			obj->obj_name);
#endif
	return 1;
}

int last_tstamp=0;
/** Exec Background Function
 *
 * This function implements the monitoring of the execution status of the objects.
 *
 * @todo Document this funciton
 * @todo Rewrite this function
 */
void exec_background(cmd_t *cmd) {
	int i, n,max_procs;
	int tstamp;
	time_t tdata[3];

	/*get_time(&tdata[2]);*/
	tstamp = get_tstamp();

	last_tstamp=tstamp;

	max_procs = hwapi_proc_get(hwapi_obj,MAX_PROCS);

	/** save exec stats */
	save_exec_stats(hwapi_obj, max_procs, tstamp);

	/** now check every object is in correct state */
	for (i = 0; i < max_procs; i++) {
		if (!hwapi_obj[i].obj_id) {
#ifdef DEB
/*			printf("Caution position %d has 0 id\n",i);
 *
 */
#endif
		} else {

			/** if object died, report and remove it
			 */
			if (!cpu_info.debug_level && !hwapi_obj[i].pid
					&& hwapi_obj[i].status.cur_status) {
#ifdef DEB
				daemon_info(daemon, "Caution object %s died!", hwapi_obj[i].obj_name);
#endif
				if (hwapi_obj[i].status.cur_status != PHAL_STATUS_ABNSTOP
						&& hwapi_obj[i].status.cur_status != PHAL_STATUS_STOP) {
					exec_statusrep(&hwapi_obj[i], 0);
				}
				if (!hwapi_proc_remove(hwapi_obj[i].obj_id)) {
					daemon_error(daemon, "Error removing process 0x%x",	hwapi_obj[i].obj_id);
				}
			} else if (hwapi_obj[i].change_progress
					&& hwapi_obj[i].status.cur_status != PHAL_STATUS_STEP) {

				/** ack new status */
				if (hwapi_obj[i].status.cur_status
						== hwapi_obj[i].status.next_status
						&& hwapi_obj[i].change_progress == 2) {
					if (hwapi_proc_status_ack(hwapi_obj[i].obj_id) == 1) {
						exec_statusrep(&hwapi_obj[i], 1);
					} else {
						/* old data */
						break;
					}
				} else {
					if (tstamp > hwapi_obj[i].status.next_tstamp
							+ st_db[hwapi_obj[i].status.next_status].max_ts
							&& hwapi_obj[i].status.next_tstamp
							&& !cpu_info.debug_level) {


						daemon_info(
								daemon,
								"Caution object %s did not changed to %s after %d-%d=%d tslots (%d!=%d)",
								hwapi_obj[i].obj_name, st_db[hwapi_obj[i].status.next_status].name, tstamp,
								hwapi_obj[i].status.next_tstamp, tstamp
										- hwapi_obj[i].status.next_tstamp,
								hwapi_obj[i].status.cur_status,
								hwapi_obj[i].status.next_status);

						/* remove process */
						hwapi_proc_kill(hwapi_obj[i].obj_id);

						/** notify to manager object did not changed its status
						 */
						hwapi_obj[i].status.cur_status
								= PHAL_STATUS_ABNSTOP;
						exec_statusrep(&hwapi_obj[i], 0);
					}
				}
			}
		}
	}
}

void send_rtfault(int tstamp, time_t *tdata, struct hwapi_proc_i *proc, float C) {
	void *x;

	pkt_clear(pkt);
	pkt_putvalue(pkt, FIELD_TSTAMP, tstamp);
	x = pkt_putptr(pkt, FIELD_TIME, sizeof(time_t) * 3);
	if (x) {
		memcpy(x, tdata, sizeof(time_t) * 3);
	}
	x = pkt_putptr(pkt, FIELD_EXEINFO, sizeof(struct hwapi_proc_i));
	if (x) {
		memcpy(x, proc, sizeof(struct hwapi_proc_i));
	}

	pkt_putfloat(pkt, FIELD_C, C);
	daemon_sendto(daemon, SWMAN_RTFAULT, 0, DAEMON_SWMAN);
}

/** @} */

void init_state_str() {
	int i;

	/* configure status names */
	memset(st_db, 0, sizeof(struct st_db));

	for (i = 0; i < PHAL_STATUS_MAX; i++) {
		switch (i) {
		case PHAL_STATUS_STARTED:
			st_db[i].max_ts = LOAD_LIMIT_TS;
			sprintf(st_db[i].name, "LOAD");
			break;
		case PHAL_STATUS_INIT:
			st_db[i].max_ts = INIT_LIMIT_TS;
			sprintf(st_db[i].name, "INIT");
			break;
		case PHAL_STATUS_RUN:
			st_db[i].max_ts = RUN_LIMIT_TS;
			sprintf(st_db[i].name, "RUN");
			break;
		case PHAL_STATUS_PAUSE:
			st_db[i].max_ts = PAUSE_LIMIT_TS;
			sprintf(st_db[i].name, "PAUSE");
			break;
		case PHAL_STATUS_ABNSTOP:
		case PHAL_STATUS_STOP:
			st_db[i].max_ts = STOP_LIMIT_TS;
			sprintf(st_db[i].name, "STOP");
			break;
		case PHAL_STATUS_STEP:
			st_db[i].max_ts = STEP_LIMIT_TS;
			sprintf(st_db[i].name, "STEP");
			break;
		}
	}

}
