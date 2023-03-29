/*
 * stats_cmds.c
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

#include <stdlib.h>
#include <string.h>

#include "phal_hw_api.h"

#include "phal_daemons.h"
#include "phid.h"

#include "set.h"
#include "str.h"
#include "var.h"
#include "rtcobj.h"
#include "phobj.h"
#include "pkt.h"
#include "log.h"
#include "daemon.h"

#include "stats.h"

#include "stats_cmds.h"
#include "statsman_cmds.h"


/*#define DEB
*/
static daemon_o daemon;
static pkt_o pkt;

#define NOF_OBJ	1000
#define NOF_ITF	1000
#define NOF_STR	1000
#define NOF_VAR	1000
#define NOF_APP 1
#define NOF_SET	50
#define NOF_SETMEM 100

/*#define NOF_SET	100
#define NOF_SETMEM 1000
 */
void stats_alloc()
{
#ifdef MTRACE
    mtrace();
#endif
    hwapi_mem_prealloc(NOF_OBJ, phobj_sizeof);
    hwapi_mem_prealloc(NOF_STR, str_sizeof);
    hwapi_mem_prealloc(NOF_VAR, var_sizeof);
    hwapi_mem_prealloc(NOF_SET, set_sizeof);
    hwapi_mem_prealloc(NOF_SETMEM, setmember_sizeof);
    hwapi_mem_prealloc(1, daemon_sizeof);
    hwapi_mem_prealloc(1, pkt_sizeof);
	hwapi_mem_prealloc(100,4);
}

int report_rmi(int i);


#define SYS_CPU_STAT_NAME "CPU_usec"
#define SYS_REL_STAT_NAME "SYS_REL"

#define MAX_VAR_REPORTS	32

struct report_var {
    var_o var;
    objid_t obj_id;
    char issys_cpu;
    char issys_rel;
    char synched;
    int last_tstamp;
    int report_idx;
    int ts_window;
    int ts_period;
    int *ts_vec;
    char *data;
};

static struct report_var reports[MAX_VAR_REPORTS];
int max_report_idx = 0;

void stats_init(daemon_o d)
{
    daemon = d;
    pkt = daemon_pkt(daemon);

    memset(reports, 0, MAX_VAR_REPORTS * sizeof (struct report_var));
}

int report_add(var_o var, objid_t obj_id, int ts_window, int ts_period)
{
    int i, j, n;

    if (ts_window > ts_period && ts_period != -1) {
        daemon_error(daemon, "Error in report params. Window must be smaller than period\n");
        return 0;
    }

    j = MAX_VAR_REPORTS;
    for (i = 0; i < MAX_VAR_REPORTS; i++) {
        if (reports[i].var) {
               if (reports[i].var->id == var->id) {
                    break;
                }
        } else if (j == MAX_VAR_REPORTS) {
            j = i;
        }
    }
    if (i == MAX_VAR_REPORTS) {
        if (j == MAX_VAR_REPORTS) {
            daemon_error(daemon, "Not enought space in db\n");
            return 0;
        }
        i = j;
    } else {
        report_rmi(i);
    }
    memset(&reports[i], 0, sizeof (struct report_var));
    reports[i].data = malloc(var_maxbsize(var) * ts_window);
    if (!reports[i].data) {
        daemon_error(daemon, "Error allocating memory for variable report\n");
        return 0;
    }
    reports[i].ts_vec = malloc(sizeof (int) * ts_window);
    if (!reports[i].ts_vec) {
        daemon_error(daemon, "Error allocating memory");
        return 0;
    }
    reports[i].obj_id = obj_id;
    reports[i].var = var_dup(var);
    reports[i].ts_window = ts_window;
    reports[i].ts_period = ts_period;
    reports[i].last_tstamp = 0;
    reports[i].issys_cpu = !str_scmp(var->name, SYS_CPU_STAT_NAME);
    reports[i].issys_rel = !str_scmp(var->name, SYS_REL_STAT_NAME);
    if (reports[i].issys_cpu || reports[i].issys_rel) {
        n=0;
        var_setvalue(reports[i].var, &n, 1);
    }
    reports[i].synched = 0;
    if (i + 1 > max_report_idx) {
        max_report_idx = i + 1;
    }

    daemon_info(daemon, "Starting report for stat %s object 0x%x w=%d,p=%d\n", str_str(var->name), obj_id, ts_window, ts_period);

    return 1;
}

int report_rmid(varid_t id)
{
    int i;

    for (i = 0; i < MAX_VAR_REPORTS; i++) {
        if (reports[i].var) {
            if (reports[i].var->id == id)
                break;
        }
    }
    if (i == MAX_VAR_REPORTS) {
        return 0;
    }

    return report_rmi(i);
}

int report_rm(var_o var)
{
    return report_rmid(var->id);
}

int report_rmi(int i)
{
    var_delete(&reports[i].var);
    free(reports[i].data);
    free(reports[i].ts_vec);

    memset(&reports[i], 0, sizeof (struct report_var));
    i++;
    while (i < MAX_VAR_REPORTS && reports[i].var)
        i++;
    max_report_idx = i + 1;
    return 1;
}

void do_reports()
{
    int i,*x,*y;
    struct hwapi_proc_i hwapi_obj;
    int rlen;
    int tstamp;

    for (i = 0; i < max_report_idx; i++) {
        if (reports[i].var) {
            if (!hwapi_proc_info(reports[i].obj_id, &hwapi_obj)) {
                pkt_clear(pkt);
                pkt_putvalue(pkt, FIELD_OBJID, reports[i].obj_id);
                pkt_putvalue(pkt, FIELD_STID, reports[i].var->id);
                daemon_sendto(daemon, STATSMAN_STCLOSE, 0, DAEMON_STATSMAN);
                report_rmi(i);
                break;
            }
            if (reports[i].issys_cpu || reports[i].issys_rel) {
                tstamp = hwapi_obj.obj_tstamp;
            } else {
                tstamp = hwapi_var_get_tstamp(reports[i].var->id);
            }
            if (tstamp > reports[i].last_tstamp) {
                if (reports[i].report_idx < reports[i].ts_window) {
                    if (reports[i].issys_cpu) {
                        *((int*) & reports[i].data[4 * reports[i].report_idx]) = (int) hwapi_obj.sys_cpu;
                        reports[i].ts_vec[reports[i].report_idx] = tstamp;
                        reports[i].last_tstamp = hwapi_obj.obj_tstamp;                        
                    } else if (reports[i].issys_rel) {
                        *((int*) & reports[i].data[4 * reports[i].report_idx]) = (int) hwapi_obj.sys_end;
                        reports[i].ts_vec[reports[i].report_idx] = tstamp;
                        reports[i].last_tstamp = hwapi_obj.obj_tstamp;

                    } else {
                        var_readreport(reports[i].var, reports[i].data, reports[i].ts_vec, reports[i].report_idx, reports[i].ts_window);
                        reports[i].last_tstamp = reports[i].ts_vec[reports[i].report_idx];
                    }
                } else {
                    reports[i].last_tstamp = hwapi_obj.obj_tstamp;
                }
                reports[i].report_idx++;
            }
            if (reports[i].report_idx == reports[i].ts_period
                    || (reports[i].ts_period == -1 && reports[i].report_idx == reports[i].ts_window)
                    || (hwapi_obj.status.cur_status != PHAL_STATUS_RUN && reports[i].report_idx > 0 && reports[i].ts_period >= 0)
                    || (hwapi_obj.status.cur_status == PHAL_STATUS_RUN && !reports[i].synched && reports[i].ts_period >= 0
                    && (get_tstamp() % reports[i].ts_period) == 0)) {
                reports[i].synched = 1;
                rlen = reports[i].report_idx;
                if (rlen > reports[i].ts_window) {
                    rlen = reports[i].ts_window;
                }
                pkt_clear(pkt);
                pkt_putvalue(pkt, FIELD_OBJID, reports[i].obj_id);
                pkt_putvalue(pkt, FIELD_STID, reports[i].var->id);
                pkt_putvalue(pkt, FIELD_STTYPE, var_type(reports[i].var));
                pkt_putvalue(pkt, FIELD_STSIZE, var_length(reports[i].var));
                pkt_putvalue(pkt, FIELD_STREPORTSZ, rlen);
                x = pkt_putptr(pkt, FIELD_STREPORTVALUE, var_bsize(reports[i].var) * rlen);
                memcpy(x,
                        reports[i].data, var_bsize(reports[i].var) * rlen);
                y = pkt_putptr(pkt, FIELD_STREPORTTSVEC, sizeof (int) * rlen);
                memcpy(y,
                        reports[i].ts_vec, sizeof (int) * rlen);
                daemon_sendto(daemon, (reports[i].ts_period == -1) ? STATSMAN_STVALOK : STATSMAN_STREPORTVALUE, 0, DAEMON_STATSMAN);
                reports[i].report_idx = 0;
                if (reports[i].ts_period == -1) {
                    report_rmi(i);
                }
            }
        }
    }
}

/** Processing Function
 *
 * REQUIRED PACKET FIELDS:
 *	- FIELD_STID: (for STAT INIT) Id of the stat for which the init request was done
 *	- FIELD_PMID: (for PARAM INIT) Id of the global variable used by SERVICES to request
 * the initialization procedure (see function implementation stats_incmd_initack at stats_cmds.c)
 *
 * ACTIONS:
 *	- Uses hw_api to set the global variable option to the appropiate constant. This way, the 
 * services API knows the stat/parameters have been successfully initiated and are ready to be used.
 *
 * OUTPUT: (no output)
 */
int stats_incmd_initack(cmd_t *cmd)
{

#ifdef DEB
    printf("STATS: Init ACK: cmd %d stat_id %d\n", pkt_getcmd(pkt),pkt_getvalue(pkt, FIELD_STID));
#endif

    switch (pkt_getcmd(pkt)) {
    case STATS_STINITOK:
        if (hwapi_var_setopt((varid_t) pkt_getvalue(pkt, FIELD_STID),
                VAR_IS_STOK) == 0) {
            daemon_error(daemon, "Error setting stat option");
            return -1;
        }
        break;
    case STATS_STINITERR:
        if (hwapi_var_setopt((varid_t) pkt_getvalue(pkt, FIELD_STID),
                VAR_IS_STERR) == 0) {
            daemon_error(daemon, "Error setting stat option");
            return -1;
        }
        break;
    case STATS_PMINITOK:
        if (hwapi_var_setopt((varid_t) pkt_getvalue(pkt, FIELD_PMID),
                VAR_IS_PMINITOK) == 0) {
            daemon_error(daemon, "Error setting stat option");
            return -1;
        }
        break;
    case STATS_PMINITERR:
        if (hwapi_var_setopt((varid_t) pkt_getvalue(pkt, FIELD_PMID),
                VAR_IS_PMINITERR) == 0) {
            daemon_error(daemon, "Error setting stat option");
            return -1;
        }
        break;
    case STATS_LOGINITOK:
        if (hwapi_var_setopt((varid_t) pkt_getvalue(pkt, FIELD_LOGID),
                VAR_IS_LOGINITOK) == 0) {
            daemon_error(daemon, "Error setting stat option");
            return -1;
        }
        break;
    case STATS_LOGINITERR:
        if (hwapi_var_setopt((varid_t) pkt_getvalue(pkt, FIELD_LOGID),
                VAR_IS_LOGINITERR) == 0) {
            daemon_error(daemon, "Error setting stat option");
            return -1;
        }
        break;
    default:
        daemon_error(daemon, "Error in command");
    }

    return 1;
}

/** Processing Function
 *
 * REQUIRED PACKET FIELDS:
 *	- FIELD_STID: Id of the stat to view/modify
 *	- FIELD_STTYPE: (for SET) Type of the stat
 *	- FIELD_STVALUE: (for SET) Data Value
 *
 * ACTIONS:
 *	- Uses hw_api functions to set or get the variable value
 *
 * OUTPUT: 
 *	- STATSMAN_STSETOK command if value has been changed successfully
 *	- STATSMAN_STSETERR command if impossible to change value
 *
 *	- STATSMAN_STVALOK command if value was obtained successfully
 *	- STATSMAN_STVALERR command if impossible to get value
 */
int stats_incmd_stact(cmd_t *cmd)
{
    var_o var;
    objid_t obj_id;
    int action, window, period;
    struct hwapi_proc_i proc;

    var = pkt_get(pkt, FIELD_STAT, var_newfrompkt);
    if (!var) {
        daemon_error(daemon, "Error reading stat from packet");
        return 0;
    }

    if (pkt_getcmd(pkt) == STATS_STREPORT) {
        action = pkt_getvalue(pkt, FIELD_STREPORTACT);
        if (action) {
            window = pkt_getvalue(pkt, FIELD_STREPORTWINDOW);
            period = pkt_getvalue(pkt, FIELD_STREPORTPERIOD);
        }
    }

    window = pkt_getvalue(pkt, FIELD_STREPORTWINDOW);
    obj_id = (objid_t) pkt_getvalue(pkt, FIELD_OBJID);
    pkt_clear(pkt);

    pkt_putvalue(pkt, FIELD_OBJID, obj_id);
    pkt_putvalue(pkt, FIELD_STID, (uint) var->id);

    switch (pkt_getcmd(pkt)) {
    case STATS_STGET:
        if (!window) {
            if (!str_scmp(var->name, SYS_CPU_STAT_NAME)) {
                hwapi_proc_info(obj_id, &proc);
                var_setvalue(var, &proc.sys_cpu, 1);
                var->tstamp = proc.obj_tstamp;
            } else if (!str_scmp(var->name, SYS_REL_STAT_NAME)) {
                hwapi_proc_info(obj_id, &proc);
                var_setvalue(var, &proc.sys_end, 1);
                var->tstamp = proc.obj_tstamp;
            } else {
                if (!var_readfromhwapi(var)) {
                    daemon_error(daemon, "Error getting stats value from hwapi");
                    daemon_sendto(daemon, STATSMAN_STVALERR, 0,
                            DAEMON_STATSMAN);
                    break;
                }
            }
            pkt_put(pkt, FIELD_STAT, var, var_topkt);
            pkt_putvalue(pkt, FIELD_STREPORTSZ,0);
            daemon_sendto(daemon, STATSMAN_STVALOK, 0, DAEMON_STATSMAN);
        } else {
            if (!report_add(var, obj_id, window, -1)) {
                daemon_error(daemon, "Error getting multiple stat value");
            }
        }
        break;
    case STATS_STSET:
        if (!var_writetohwapi(var)) {
            daemon_error(daemon, "Error setting stats value");
            daemon_sendto(daemon, STATSMAN_STSETERR, 0,
                    DAEMON_STATSMAN);
        } else {
            daemon_sendto(daemon, STATSMAN_STSETOK, 0,
                    DAEMON_STATSMAN);
        }
        break;
    case STATS_STREPORT:
        pkt_putvalue(pkt, FIELD_STREPORTACT, action);
        if (action) {
            if (!report_add(var, obj_id, window, period)) {
                daemon_error(daemon, "Error setting reports for variable %s\n", str_str(var->name));
                daemon_sendto(daemon, STATSMAN_STREPORTERR, 0, DAEMON_STATSMAN);
            } else {
                daemon_sendto(daemon, STATSMAN_STREPORTOK, 0, DAEMON_STATSMAN);
            }
        } else {
            if (!report_rm(var)) {
                daemon_error(daemon, "Error stopping reports for variable %s\n", str_str(var->name));
                daemon_sendto(daemon, STATSMAN_STREPORTERR, 0, DAEMON_STATSMAN);
            } else {
                daemon_info(daemon, "Stopping report for variable %s object 0x%x\n", str_str(var->name), obj_id);
                daemon_sendto(daemon, STATSMAN_STREPORTOK, 0, DAEMON_STATSMAN);
            }
        }
        break;
    default:
        daemon_error(daemon, "Unknown command");
    }

    var_delete(&var);

    return 1;
}

/** Processing Function
 *
 * REQUIRED PACKET FIELDS:
 *	- FIELD_PMID: Id of the global variable used by SERVICES to store the value of the parameter.
 *	- FIELD_PMTYPE: Type of the parameter
 *	- FIELD_PMVALUE: Value
 *
 * ACTIONS:
 *	- Uses hw_api to set the global variable value to the received one .
 *
 * OUTPUT: (no output)
 */
int stats_incmd_pmval(cmd_t *cmd)
{
    var_o var;

    if (pkt_getcmd(pkt) == STATS_PMVALOK) {
        var = pkt_get(pkt, FIELD_PARAM, var_newfrompkt);
        if (!var) {
            daemon_error(daemon, "Error reading variable");
            return -1;
        }

        if (!var_writetohwapi(var)) {
            daemon_error(daemon, "Error setting stat");
        }
        if (hwapi_var_setopt(var->id, VAR_IS_PMVALOK) == 0) {
            daemon_error(daemon, "Error setting stat option");
        }
        var_delete(&var);
    } else if (pkt_getcmd(pkt) == STATS_PMVALERR) {
        if (hwapi_var_setopt((varid_t) pkt_getvalue(pkt, FIELD_PMID), VAR_IS_PMVALERR) == 0) {
            daemon_error(daemon, "Error setting stat option");
        }
    } else {
        daemon_error(daemon, "Error in command");
    }

    return 1;
}

/** @defgroup sensors_stats STATS Sensor Background Functions
 *
 * The STATS Sensor monitors object's requests for registering an statistics
 * variable or requesting a parameter value.
 *
 * @ingroup sensors
 *
 * @{
 */



int sizeoftype(int type)
{
    switch (type) {
    case STAT_TYPE_TEXT:
    case STAT_TYPE_CHAR:
    case STAT_TYPE_UCHAR:
        return sizeof (char);
    case STAT_TYPE_FLOAT:
        return sizeof (float);
    case STAT_TYPE_INT:
        return sizeof (int);
    case STAT_TYPE_SHORT:
        return sizeof (short);
    default:
        daemon_info(daemon, "Error invalid stat type %d", type);
        return 0;
    }
}

char test[129];
/** stats_stinit
 *
 * Issues a command to STATSMAN announcing that a new variable
 * has been created.
 * In order the higher layers (CMD_MAN, STATSMAN) to be able to change
 * this value, SERVICES daemon should wait the confirmation of this
 * initialization packet by checking the value of the options field
 * of the variable. In any case, object can still making use of this 
 * variable although anyone will be able to read or write it.
 * 
 */
int stats_stinit(struct hwapi_var_i *var)
{
    char *objname, *appname, *statname;
	str_o obj_name,app_name;
    var_o ovar;
    vartype_t type;
	int t;
	int sizestat;

	pkt_clear(pkt);

    /* variable name contains the application, object & stat name
     sepparated by ':' e.g.: appname:objname:statname */
    appname = var->name;
    strncpy(test,var->name,128);
    objname = strstr(appname, ":");
    if(!objname) {
    	hwapi_var_setopt(var->stat_id, VAR_IS_STERR);
    	daemon_error(daemon, "Invalid app name %s\n",test);
    	return 0;
    }
    *objname = '\0';
    objname++;
    statname = strstr(objname, ":");
    if(!statname) {
    	hwapi_var_setopt(var->stat_id, VAR_IS_STERR);
    	daemon_error(daemon, "Invalid obj name %s\n",test);
    	return 0;
    }
    *statname = '\0';
    statname++;

	type = (vartype_t) var->type;

	sizestat=sizeoftype(type);
	if (!sizestat) {
		daemon_error(daemon, "Error creating stat %s\n",statname);
		hwapi_var_setopt(var->stat_id, VAR_IS_STERR);
		return 0;
	}

    /* create temporal variable using hwapi structure information */
    /**@todo watch here, size/type must be multiple, actually they are, because
     * services created them.
     */
    ovar = var_new(var->size / sizestat, type);
    if (!ovar) {
        daemon_error(daemon, " Error creating stat %s.", statname);
        hwapi_var_setopt(var->stat_id, VAR_IS_STERR);
        return 0;
    }

#ifdef DEB
    printf("STATS: Initiating stat %s:%s Id %d\n",objname,statname,var->stat_id);
#endif

    ovar->id = var->stat_id;
    str_set(ovar->name, statname);

    pkt_put(pkt, FIELD_STNAME, ovar->name, str_topkt);

    app_name = str_snew(appname);
    obj_name = str_snew(objname);
    pkt_put(pkt, FIELD_APPNAME, app_name, str_topkt);
    pkt_put(pkt, FIELD_OBJNAME, obj_name, str_topkt);
    str_delete(&app_name);
    str_delete(&obj_name);

    pkt_putvalue(pkt, FIELD_OBJID, var->obj_id);
    pkt_put(pkt, FIELD_STAT, ovar, var_topkt);

    daemon_sendto(daemon, STATSMAN_STINIT, 0, DAEMON_STATSMAN);

    hwapi_var_setopt(var->stat_id, 0);

    var_delete(&ovar);

    return 1;
}

/** stats_stclose
 */
int stats_stclose(struct hwapi_var_i *var)
{
    pkt_clear(pkt);

    pkt_putvalue(pkt, FIELD_OBJID, var->obj_id);
    pkt_putvalue(pkt, FIELD_STID, var->stat_id);

    daemon_sendto(daemon, STATSMAN_STCLOSE, 0, DAEMON_STATSMAN);

    report_rmid(var->stat_id);

    hwapi_var_delete(var->stat_id);

    return 1;

}

/** stats_pminit
 *
 * Issues a command to STATSMANAGER requesting to initialize
 * or read the parameter configuration file
 */
int stats_pminit(struct hwapi_var_i *var)
{
    char *objname, *appname;
	str_o obj_name, app_name;

    pkt_clear(pkt);

    /* variable name contains the application & object
     sepparated by : e.g.: appname:objname:statname */
    appname = var->name;
    objname = strstr(appname, ":");
    *objname = '\0';
    objname++;

    app_name = str_snew(appname);
    obj_name = str_snew(objname);
    pkt_put(pkt, FIELD_APPNAME, app_name, str_topkt);
    pkt_put(pkt, FIELD_OBJNAME, obj_name, str_topkt);
    str_delete(&app_name);
    str_delete(&obj_name);

    pkt_putvalue(pkt, FIELD_OBJID, var->obj_id);
    pkt_putvalue(pkt, FIELD_PMID, var->stat_id);

    daemon_sendto(daemon, STATSMAN_PMINIT, 0, DAEMON_STATSMAN);

    hwapi_var_setopt(var->stat_id, 0);

    return 1;
}

/** stats_pmclose
 */
int stats_pmclose(struct hwapi_var_i *var)
{

    pkt_clear(pkt);

    pkt_putvalue(pkt, FIELD_OBJID, var->obj_id);

    daemon_sendto(daemon, STATSMAN_PMCLOSE, 0, DAEMON_STATSMAN);

    hwapi_var_delete(var->stat_id);

    return 1;

}

/** stats_pmget
 *
 * Issues a command to STATSMAN requesting to obtain a parameter
 * value 
 */
int stats_pmget(struct hwapi_var_i *var)
{
	str_o pmname;
    vartype_t type;
	int t;

    pkt_clear(pkt);

	type=var->type;

    pmname = str_snew(var->name);
    pkt_put(pkt, FIELD_PMNAME, pmname, str_topkt);
    str_delete(&pmname);

    pkt_putvalue(pkt, FIELD_OBJID, var->obj_id);
    pkt_putvalue(pkt, FIELD_PMID, var->stat_id);
    pkt_putvalue(pkt, FIELD_PMTYPE, type);

    daemon_sendto(daemon, STATSMAN_PMGET, 0, DAEMON_STATSMAN);

    hwapi_var_setopt(var->stat_id, 0);

    return 1;

}

/** stats_loginit
 *
 * Issues a command to STATSMANAGER requesting to initialize
 * a log file
 */
int stats_loginit(struct hwapi_var_i *var)
{
    char *objname, *appname;
	str_o app_name, obj_name;
    pkt_clear(pkt);

    /* variable name contains the application, object & log name
     sepparated by : e.g.: appname:objname:logname */
    appname = var->name;
    objname = strstr(appname, ":");
    *objname = '\0';
    objname++;

    app_name = str_snew(appname);
    obj_name = str_snew(objname);
    pkt_put(pkt, FIELD_APPNAME, app_name, str_topkt);
    pkt_put(pkt, FIELD_OBJNAME, obj_name, str_topkt);
    str_delete(&app_name);
    str_delete(&obj_name);

    pkt_putvalue(pkt, FIELD_OBJID, var->obj_id);
    pkt_putvalue(pkt, FIELD_LOGID, var->stat_id);

    daemon_sendto(daemon, STATSMAN_LOGINIT, 0, DAEMON_STATSMAN);

    hwapi_var_setopt(var->stat_id, 0);

    return 1;
}

/** stats_logclose
 */
int stats_logclose(struct hwapi_var_i *var)
{
    pkt_clear(pkt);
    pkt_putvalue(pkt, FIELD_OBJID, var->obj_id);
    pkt_putvalue(pkt, FIELD_LOGID, var->stat_id);
    daemon_sendto(daemon, STATSMAN_LOGCLOSE, 0, DAEMON_STATSMAN);

    hwapi_var_delete(var->stat_id);

    return 1;
}

char logbuffer[LOG_LINE_SZ];

/** stats_logwrite
 */
int stats_logwrite(struct hwapi_var_i *var)
{
    pkt_clear(pkt);
    pkt_putvalue(pkt, FIELD_OBJID, var->obj_id);
    pkt_putvalue(pkt, FIELD_LOGID, var->stat_id);

    /* contents are read from stat */
    hwapi_var_read(var->stat_id, logbuffer, LOG_LINE_SZ);
    if (strlen(logbuffer) > 0) {
        memcpy(pkt_putptr(pkt, FIELD_LOGTXT, strlen(logbuffer) + 1), logbuffer, strlen(logbuffer) + 1);
        daemon_sendto(daemon, STATSMAN_LOGWRITE, 0, DAEMON_STATSMAN);
    }
    hwapi_var_setopt(var->stat_id, VAR_IS_LOGFLUSHED);

    return 1;
}

int stats_setobjectstat(struct hwapi_var_i *var)
{
    char *objname, *appname, *statname;
    str_o stat_name, obj_name, app_name;
    void *ptr;

    pkt_clear(pkt);

    /* variable name contains the application, object & stat name
     sepparated by ':' e.g.: appname:objname:statname */
    appname = var->name;
    objname = strstr(appname, ":");
    *objname = '\0';
    objname++;
    statname = strstr(objname, ":");
    *statname = '\0';
    statname++;

    app_name = str_snew(appname);
    obj_name = str_snew(objname);
    stat_name = str_snew(statname);
    if (!app_name || !obj_name || !stat_name) {
        printf("STATS: Error creating strings\n");
        return 0;
    }

    printf("STATS: Setting Object stat %s:%s Id %d value sz %d\n",objname,statname,var->stat_id,hwapi_var_getsz(var->stat_id));

    pkt_put(pkt, FIELD_APPNAME, app_name, str_topkt);
    pkt_put(pkt, FIELD_OBJNAME, obj_name, str_topkt);
    pkt_put(pkt, FIELD_STNAME, stat_name, str_topkt);

    str_delete(&app_name);
    str_delete(&obj_name);
    str_delete(&stat_name);

    pkt_putvalue(pkt, FIELD_STSIZE, hwapi_var_getsz(var->stat_id));

    ptr = pkt_putptr(pkt, FIELD_STVALUE, hwapi_var_getsz(var->stat_id));
    if (!ptr) {
        printf("STATS: Error allocating data to pkt\n");
        return 0;
    }

    if (!hwapi_var_read(var->stat_id, ptr, hwapi_var_getsz(var->stat_id))<0) {
        printf("STATS: Error reading stat from hwapi\n");
        return 0;
    }

    daemon_sendto(daemon, STATSMAN_STSET, 0, DAEMON_STATSMAN);

    hwapi_var_setopt(var->stat_id, 0);

    return 1;
}


int stats_stobjectinit(struct hwapi_var_i *var)
{
    char *objname, *appname, *statname;
    str_o stat_name, obj_name, app_name;
    void *ptr;

    pkt_clear(pkt);

    /* variable name contains the application, object & stat name
     sepparated by ':' e.g.: appname:objname:statname */
    appname = var->name;
    objname = strstr(appname, ":");
    *objname = '\0';
    objname++;
    statname = strstr(objname, ":");
    *statname = '\0';
    statname++;

    app_name = str_snew(appname);
    obj_name = str_snew(objname);
    stat_name = str_snew(statname);
    if (!app_name || !obj_name || !stat_name) {
        printf("STATS: Error creating strings\n");
        return 0;
    }

    printf("STATS: Initiating Object stat %s:%s Id %d\n",objname,statname,var->stat_id);

    pkt_put(pkt, FIELD_APPNAME, app_name, str_topkt);
    pkt_put(pkt, FIELD_OBJNAME, obj_name, str_topkt);
    pkt_put(pkt, FIELD_STNAME, stat_name, str_topkt);

    str_delete(&app_name);
    str_delete(&obj_name);
    str_delete(&stat_name);

    daemon_sendto(daemon, STATSMAN_STINITOBJECT, 0, DAEMON_STATSMAN);

    hwapi_var_setopt(var->stat_id, 0);

    return 1;
}

/** STATS Background Processign Function
 */
void stats_background()
{
    int i;
    int max_vars;
    struct hwapi_var_i *var_list;

    do_reports();

    max_vars = hwapi_var_getptr(&var_list);
	for (i = 0; i < max_vars; i++) {
		if (var_list[i].stat_id) {
			switch (var_list[i].opts) {
			case VAR_IS_STINIT:
				var_wait();
				stats_stinit(&var_list[i]);
				var_post();
				break;
			case VAR_IS_STCLOSE:
				stats_stclose(&var_list[i]);
				break;
			case VAR_IS_PMINIT:
				var_wait();
				stats_pminit(&var_list[i]);
				var_post();
				break;
			case VAR_IS_PMCLOSE:
				stats_pmclose(&var_list[i]);
				break;
			case VAR_IS_PMGET:
				stats_pmget(&var_list[i]);
				break;
			case VAR_IS_LOGINIT:
				stats_loginit(&var_list[i]);
				break;
			case VAR_IS_LOGCLOSE:
				stats_logclose(&var_list[i]);
				break;
			case VAR_IS_LOGWRITE:
				stats_logwrite(&var_list[i]);
				break;
			case VAR_IS_STOBJECTINIT:
				stats_stobjectinit(&var_list[i]);
				break;
			case VAR_IS_SETOBJECT:
				stats_setobjectstat(&var_list[i]);
				break;
			}
		}
    }
}


/** @} */
