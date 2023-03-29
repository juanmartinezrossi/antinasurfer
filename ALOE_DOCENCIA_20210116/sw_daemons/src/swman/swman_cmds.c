/*
 * swman_cmds.c
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
#include <mcheck.h>
#include "phal_hw_api.h"
#include "phal_hw_api_man.h"
#include "phid.h"
#include "phal_daemons.h"

#include "set.h"
#include "str.h"
#include "log.h"
#include "var.h"
#include "app.h"
#include "rtcobj.h"
#include "phitf.h"
#include "phobj.h"
#include "cfg_parser.h"
#include "pkt.h"
#include "daemon.h"

#include "swman.h"

#include "swload_cmds.h"
#include "exec_cmds.h"
#include "cmdman_cmds.h"
#include "swman_cmds.h"
#include "hwman_cmds.h"
#include "bridge_cmds.h"

#include "cfg_parser.h"
#include "swman_parser.h"
#include "exec.h"

/*#define DEB
*/

#define TS_CORRECT

int status_ok(app_o app);
int reports_startstop(str_o app_name, int start);

#define AUTOMATIC_REPORTS 1

#define UPLOAD_PACKETS 10000

#define DEFAULT_STATUS_INC_TS 10

#define BASE_LOG_PATH "execlogs"

#define RELINQUISH

int automatic_reports = AUTOMATIC_REPORTS;

/* This is the main database for objects */
Set_o apps_db = NULL;

/** Set for the executable information */
Set_o execs;

peid_t local_id = 0;

daemon_o daemon;
pkt_o pkt;

#define PRINT_RTFAULTS

#define NOF_OBJ	1000
#define NOF_ITF	1000
#define NOF_STR	5000
#define NOF_VAR	1000
#define NOF_APP 10
#define NOF_SET	800
#define NOF_SETMEM 1000
#define NOF_CFG	100
#define NOF_SECT 1000
#define NOF_KEYS 5000

void swman_alloc() {
#ifdef MTRACE
    mtrace();
#endif
    hwapi_mem_prealloc(NOF_OBJ, phobj_sizeof);
    hwapi_mem_prealloc(NOF_OBJ, rtcobj_sizeof);
    hwapi_mem_prealloc(NOF_OBJ, log_sizeof);
    hwapi_mem_prealloc(NOF_ITF, phitf_sizeof);
    hwapi_mem_prealloc(NOF_STR, str_sizeof);
    hwapi_mem_prealloc(NOF_APP, app_sizeof);
    hwapi_mem_prealloc(NOF_VAR, var_sizeof);
    hwapi_mem_prealloc(NOF_SET, set_sizeof);
    hwapi_mem_prealloc(NOF_SETMEM, setmember_sizeof);
    hwapi_mem_prealloc(NOF_CFG, cfg_sizeof);
    hwapi_mem_prealloc(NOF_SECT, sect_sizeof);
    hwapi_mem_prealloc(NOF_KEYS, key_sizeof);
    hwapi_mem_prealloc(1, daemon_sizeof);
    hwapi_mem_prealloc(1, pkt_sizeof);
}





void writeAPPname(char *filename){		/*AGBAPRIL20*/

	FILE *fp;
	int k=0, z;

	/*printf("swman_cmds.c: writeAPPname()\n");*/


	fp = fopen("APPname.bin", "wb");
	if (fp == NULL){
		printf("ERROR opening write file: fopen() failed for '%s'\n", "APPname.bin");
		exit(0);		
	}
	printf("PARSER %s\n", filename);
	z=strlen(filename);
	k=fwrite(filename, 1, z+1, fp);
	k;
	if(k!=z+1){
		printf("ERROR opening write file: k=%d != z=%d \n", k, z);
		exit(0);		
	}
	printf("NO ERROR opening write file: k=%d = z=%d \n", k, z);
	fclose(fp);
}




/* function to initialize database */
int swman_init(daemon_o d) {
    apps_db = Set_new(0, NULL);
    if (!apps_db) {
        daemon_error(daemon, "Error creating main database");
        return 0;
    }
    daemon = d;
    pkt = daemon_pkt(daemon);

    execs = Set_new(0,NULL);
    if (!execs) {
        daemon_error(daemon, "Error creating main database");
        return 0;
    }
    swman_parser_execs(execs);
    return 1;
}

int next_pending_status = -1;

/** Set app status
 *
 * Tries to set a new waveform status. It only can be changed
 * when all objects did change their status.
 *
 * Even if the objects have not already changed their status,
 * a new status can be requested which will be executed as soon
 * as all objects did finish their status change. This "queue" of
 * status is of length 1.
 *
 * @return 1 if status did changed, 0 delayed, -1 if can't change
 */
int set_app_status(app_o app, status_t new_status) {


    if (status_ok(app) || new_status == PHAL_STATUS_STOP
            || new_status == PHAL_STATUS_ABNSTOP) {
        app->cur_status = new_status;
        return 1;
    } else if (!app->next_pending_status) { /** no status is pending */
        return 0;
    } else {
        return -1;
    }
}

/** Check if app changed ok
 *
 * Checks if all the waveform objects did change their status.
 * That means that any object is in an status different of
 * current waveform status.
 *
 * When all objects are in correct status, next pending status is set, if any
 *
 * @return 1 if all objects ok, 0 if not ok
 */
int status_ok(app_o app)
{
    int i=0;
    phobj_o obj;

    for (i=0;i<Set_length(app->objects);i++) {
        obj = Set_get(app->objects, i);
        if (!obj) {
            return 0;
        }
        if (app->cur_status != obj->status) {
            return 0;
        }
    }
    return 1;
}

void unmap_app(str_o app_name) {
    pkt_clear(pkt);
    pkt_put(pkt, FIELD_APPNAME, app_name, str_topkt);
    daemon_sendto(daemon, HWMAN_UNMAPOBJ, 2, DAEMON_HWMAN);
}

void loadapp_error(app_o app) {
    str_o app_name = NULL;

    if (app) {
        app_name = app->name;
    } else {
        app_name = pkt_get(pkt, FIELD_APPNAME, str_newfrompkt);
        if (!app_name) {
            app_name = str_snew("'unknown'");
        }
    }

    pkt_clear(pkt);
    pkt_put(pkt, FIELD_APPNAME, app_name, str_topkt);
    daemon_sendto(daemon, CMDMAN_SWLOADAPPERR, 0, DAEMON_CMDMAN);
    if (app) {
        unmap_app(app_name);
        Set_remove(apps_db, app);
        app_delete(&app);
    } else {
        str_delete(&app_name);
    }

}

int check_interfaces(app_o app)
{
    int i,j;
    phobj_o robj,wobj;
    phitf_o ritf;
    int error=0;

    for (i=0;i<Set_length(app->objects);i++) {
        robj=Set_get(app->objects,i);
        assert(robj);
        for (j=0;j<Set_length(robj->itfs);j++) {
            ritf=Set_get(robj->itfs,j);
            assert(ritf);
            wobj=Set_find(app->objects,ritf->remoteobjname,phobj_findobjname);
            if (!wobj) {
                daemon_error(daemon, "Can't find remote object for itf %s:%s->%s:%s",
                        str_str(robj->objname),str_str(ritf->name),
                        str_str(ritf->remoteobjname),str_str(ritf->remotename));
               error=1;
            } else {
                if (!Set_find(wobj->itfs,ritf->remotename,phitf_findname)) {
                    daemon_error(daemon, "Can't find remote interface for itf %s:%s->%s:%s",
                            str_str(robj->objname),str_str(ritf->name),
                            str_str(ritf->remoteobjname),str_str(ritf->remotename));
                    error=1;
                }
            }

        }
    }

    return error?0:1;
}

/** SWMAN: Load an waveform function
 *
 * This function reads the waveform config file, parses it
 *	and sends it to SWLOAD the plain list of objects and their
 *	interconnection so the loading and linking is performed.
 *
 *	This packet may be captured by any HWMAN daemon located in the
 *	same or lower processor and perform a re-assignation of sw into
 *	processing resources (see doc)
 *
 *	The configuration file of the waveform must have the following name:
 *		- swman/appname.app

 *
 * 	In these files, objects, waveforms and interfaces are described
 *	using names. After reading the configuration file, an id is generated
 * 	for every object in the waveform. This id must be unique in all ALOE
 *	environment.
 *
 *	This id will be internally associated with the object and, after being
 *	registered by SWLOAD when process is launched, it will be available to the
 *	whole processor, enabling the EXEC sensor to monitor its execution status.
 *
 *	@todo comment this
 *	@todo errors in downloading
 */
int swman_incmd_loadapp(cmd_t *cmd) {
	float g,*x;
    app_o app = NULL;
    phobj_o obj;
    str_o app_name;
    int i;
    struct hwapi_cpu_i cpu_i;

    if (!local_id) {
        hwapi_hwinfo_cpu(&cpu_i);
        if (cpu_i.pe_id) {
            local_id = cpu_i.pe_id;
        } else {
            daemon_error(daemon, "Error loading waveform: Not registered.");
            loadapp_error(NULL);
            return 0;
        }
    }

    app_name = (str_o) pkt_get(pkt, FIELD_APPNAME, str_newfrompkt);
    if (!app_name) {
        daemon_error(daemon, "Invalid packet");
        loadapp_error(NULL);
        return 0;
    }
    app = Set_find(apps_db, app_name, app_findname);
    if (app) {
        daemon_error(daemon, "Already loaded");
        str_delete(&app_name);
        loadapp_error(NULL);
        return 0;
    } else {
        app = app_new();
        if (!app) {
            daemon_error(daemon, "Can't create waveform");
            loadapp_error(app);
            str_delete(&app_name);
            return 0;
        } else {
            str_cpy(app->name, app_name);
            Set_put(apps_db, app);
            str_delete(&app_name);
        }
    }


	 printf("SWMAN_CMDS: %s\n", str_str(app->name));
	 writeAPPname(str_str(app->name));	/*AGBAPRIL20*/




    if (!swman_parser_app(str_str(app->name), app->objects)) {
        daemon_error(daemon, "Error parsing configuration file.");
        loadapp_error(app);
        return 0;
    }

    if (!check_interfaces(app)) {
        daemon_error(daemon, "Error in waveform interfaces\n");
        loadapp_error(app);
        return 0;
    }

    if (!Set_length(app->objects)) {
        daemon_error(daemon, "Any object was parsed.");
        loadapp_error(app);
        return 0;
    }
    if (!set_app_status(app, PHAL_STATUS_STARTED)) {
        daemon_error(daemon, "Error setting initial status.");
        loadapp_error(app);
        return 0;
    }
    x = pkt_getptr(pkt, FIELD_GCOST);
    if (x)
    	g = *x;
    else
    	daemon_error(daemon, "Error in packet. Expected g-cost\n");
    pkt_clear(pkt);
    x = pkt_putptr(pkt, FIELD_GCOST, sizeof(float));
    *x = g;

    for (i = 0; i < Set_length(app->objects); i++) {
        obj = Set_get(app->objects, i);
        str_cpy(obj->appname, app->name);
        obj->app_id = app->app_id;
    }

    if (pkt_put(pkt, FIELD_APP, app, app_topkt)) {
        daemon_sendto(daemon, HWMAN_MAPOBJ, local_id, DAEMON_HWMAN);
    }

    daemon_info(daemon, "Loading app\n");

    return 1;
}

int swman_sendobjstatus(app_o app, status_t status, int tstamp) {
    int i;
    Set_o objset;
    phobj_o obj;
    int xtstamp;

    objset = app->objects;
    if (!objset) {
        return 0;
    }

    if (!tstamp) {
        xtstamp = get_tstamp() + DEFAULT_STATUS_INC_TS;
    } else {
        xtstamp = tstamp;
    }

    /* for every object */
    for (i = 0; i < Set_length(objset); i++) {
        obj = Set_get(objset, i);

        if (!obj->pe_id) {
            daemon_error(daemon, "Some objects aren't registered.");
            return 0;
        }

        /* send only if object is not stopped */
        if (obj->status != PHAL_STATUS_STOP && obj->status
                != PHAL_STATUS_ABNSTOP) {

            pkt_clear(pkt);
            pkt_putvalue(pkt, FIELD_STATUS, (uint) status);

            pkt_putvalue(pkt, FIELD_OBJID,
                    (uint) obj->obj_id);
            pkt_putvalue(pkt, FIELD_TSTAMP, (uint) tstamp);
            daemon_sendto(daemon, EXEC_SETSTATUS,
                    obj->pe_id, DAEMON_EXEC);

        }
    }

    return 1;
}

/** SWMAN: Receives RTFAULT signal
 *
 *
 *
 */
int swman_incmd_rtfault(cmd_t *cmd) {
    int tstamp;
    struct hwapi_proc_i proc;
    time_t tdata[3];
    void *x;
    app_o app;
    phobj_o obj;
    float C;

    x=pkt_getptr(pkt, FIELD_EXEINFO);
    if (!x) {
        daemon_error(daemon,"Error in packet, missing proc info\n");
        return 1;
    }
    memcpy(&proc,x,sizeof(struct hwapi_proc_i));
    x=pkt_getptr(pkt, FIELD_TIME);
    if (!x) {
        daemon_error(daemon,"Error in packet, missing proc info\n");
        return 1;
    }
    memcpy(tdata,x,sizeof(time_t)*3);
    tstamp = pkt_getvalue(pkt, FIELD_TSTAMP);
    C = pkt_getfloat(pkt, FIELD_C);
#ifdef PRINT_RTFAULTS
    if (proc.relinquished) {
        daemon_info
            (daemon, "CAUTION REAL-TIME VIOLATION. Object %s did not execute at ts %d (sync ts %d).\n\tObj TS=%d. Last Exec=%d:%d -> %d:%d Now=%d:%d\n",
            proc.obj_name, tstamp,hwapi_last_sync(),
            proc.cur_tstamp,
            proc.tdata[1].tv_sec,proc.tdata[1].tv_usec,
            proc.tdata[2].tv_sec,proc.tdata[2].tv_usec,
            tdata[2].tv_sec,tdata[2].tv_usec);
    } else {
    	daemon_info(daemon,
			"CAUTION REAL-TIME VIOLATION. Object %s.\n\tTS=%d. Start=%d:%d Now=%d:%d ExecUS=%d ExecKOPTS=%d LimitUS=%d\n",
			proc.obj_name, tstamp,
			proc.tdata[1].tv_sec, proc.tdata[1].tv_usec,
			tdata[2].tv_sec, tdata[2].tv_usec,
			tdata[0].tv_usec, (int) (tdata[0].tv_usec*C));
    }
#endif

    app=Set_find(apps_db,&proc.app_id,app_findid);
    if (!app) {
        daemon_info(daemon, "Error app id 0x%x not found\n",proc.app_id);
        return 1;
    }

    app->nof_rtfaults++;

    obj=Set_find(app->objects,&proc.obj_id,phobj_findid);
    if (!obj) {
        daemon_info(daemon, "Error object 0x%x not found\n",proc.obj_id);
        return 1;
    }

    obj->rtc->nof_faults++;
    return 1;
}


/** SWMAN: Change waveform status command.
 *
 * This function is called when a command to change an waveform
 *	status is received from CMD_MAN. If the waveform has been
 *	loaded from this swman and the objects have been (all of them)
 * 	correctly loaded, the status change will be ordered to the
 *	(under us) array of sensors (EXEC) controlling objects execution.
 */
int swman_incmd_appstatus(cmd_t *cmd) {
    app_o app = NULL;
    phobj_o obj;
    str_o app_name;
    int i, n;
    status_t status;
    int tstamp;

    app_name = pkt_get(pkt, FIELD_APPNAME, str_newfrompkt);
    if (!app_name) {
        daemon_error(daemon, "Error in packet");
        return 0;
    }
    status = (status_t) pkt_getvalue(pkt, FIELD_STATUS);
    tstamp = (int) pkt_getvalue(pkt, FIELD_TSTAMP);

    app = Set_find(apps_db, app_name, app_findname);
    if (!app) {
        daemon_error(daemon, "Unknown waveform %s", str_str(app_name));
        str_delete(&app_name);
        loadapp_error(NULL);
        return 0;
    }
    str_delete(&app_name);

    if (status == PHAL_STATUS_STEP) {
        n = set_app_status(app, PHAL_STATUS_PAUSE);
    } else {
        n = set_app_status(app, status);
    }
    if (!n) {
        app->next_pending_status = status;
        app->next_pending_tstamp = tstamp;
        return 0;
    }
    if (n < 0) {
        daemon_error(daemon, "Can't change waveform status");
        loadapp_error(app);
        return 0;
    }

    if (!swman_sendobjstatus(app, status, tstamp)) {
        daemon_error(daemon, "Can't set waveform status");
        loadapp_error(app);
        return 0;
    }

    if (status == PHAL_STATUS_STEP) {
        for (i = 0; i < Set_length(app->objects); i++) {
            obj = Set_get(app->objects, i);
            assert(obj);
            obj->status = PHAL_STATUS_STEP;
        }
    }

    return 1;
}

/** SWMAN: Object reports status change
 *
 * Here EXEC will tell us a successfull change in its status had been done
 *	or if an error occurred.
 * It can also be received any time if the EXEC sensor detects that some
 *	object is not in correct state. In such case, it will be reported to
 * 	CMD_MAN daemon.
 * In the case that all the objects reach the correct status, it will also
 *	be reported to CMD_MAN indicating a successful change of the waveform status
 *
 * @todo What to do when an waveform status change failed in some object
 *
 */
int swman_incmd_statusrep(cmd_t *cmd) {
    app_o app;
    status_t status;
    phobj_o obj;
    short out_type = 0;

    app = Set_find(apps_db, pkt_getptr(pkt, FIELD_APPID), app_findid);
    if (!app) {
#ifdef DEB
        daemon_error(daemon, "Can't find waveform with id %d (object 0x%x, status %d, ok %d)",
                pkt_getvalue(pkt, FIELD_APPID), pkt_getvalue(pkt, FIELD_OBJID),
                pkt_getvalue(pkt, FIELD_STATUS),
                pkt_getcmd(pkt) == SWMAN_STATUSOK);
#endif
        return 0;
    }
    obj = Set_find(app->objects, pkt_getptr(pkt, FIELD_OBJID),
            phobj_findid);
    if (!obj) {
        daemon_error(daemon, "Object 0x%x not found.", pkt_getvalue(
                pkt, FIELD_OBJID));
        return 0;
    }

    status = (status_t) pkt_getvalue(pkt, FIELD_STATUS);

    if (pkt_getcmd(pkt) == SWMAN_STATUSOK
            && (status == app->cur_status || app->cur_status == PHAL_STATUS_ABNSTOP)) {

        obj->status = status;
        /* if this is the first packet, register pe where it is running */
        if (obj->status == PHAL_STATUS_STARTED) {
            obj->pe_id = pkt_getsrcpe(pkt);
        }
    } else {
        if (status != app->cur_status && status != PHAL_STATUS_ABNSTOP) {
            if (app->cur_status != PHAL_STATUS_ABNSTOP) {
                daemon_error(daemon, "Incorrect status for object 0x%x (obj %d, app %d)",
                    obj->obj_id, pkt_getvalue(pkt, FIELD_STATUS),
                    app->cur_status);
            }
        } else if (status == PHAL_STATUS_ABNSTOP) {
            if (app->cur_status != PHAL_STATUS_ABNSTOP) {
                daemon_info(daemon, "Object %s died unexpectectly.", str_str(obj->objname));
            }
            obj->status = PHAL_STATUS_ABNSTOP;
        } 

        /* stop all waveform */
        if (app->cur_status != PHAL_STATUS_ABNSTOP) {
            daemon_info(daemon, "Aborting execution of %s. Bad status for object %s\n", str_str(app->name), str_str(obj->objname));
            if (!set_app_status(app, PHAL_STATUS_ABNSTOP)) {
                daemon_error(daemon, "Error setting stop status.");
            }
            swman_sendobjstatus(app, PHAL_STATUS_ABNSTOP, 0);
        }
        obj->status = PHAL_STATUS_ABNSTOP;

    }

    /* if all objects changed their status we need to report it to CMDMAN */
    if (status_ok(app)) {
        if (obj->status == PHAL_STATUS_STARTED) {
            out_type = CMDMAN_SWLOADAPPOK;
            if (automatic_reports) {
                sleep_ms(100);
                reports_startstop(obj->appname, 1);
            }
        } else if (app->cur_status == PHAL_STATUS_ABNSTOP) {
            out_type = CMDMAN_SWSTATUSERR;
        } else {
            out_type = CMDMAN_SWSTATUSOK;
        }
        
        /* if a pending status is in queue execute it */
        if (app->next_pending_status) {
            app->cur_status = app->next_pending_status;
            app->next_pending_status = 0;

            if (!swman_sendobjstatus(app, app->cur_status, app->next_pending_tstamp)) {
                daemon_error(daemon, "Error setting waveform status");
                loadapp_error(NULL);
                return 0;
            }
        }

        if (app->cur_status == PHAL_STATUS_STOP || app->cur_status == PHAL_STATUS_ABNSTOP) {
            if (automatic_reports) {
                reports_startstop(obj->appname, 0);
            }
            unmap_app(app->name);
            Set_remove(apps_db, app);
            app_delete(&app);
            daemon_info(daemon, "Removed waveform from db");
        }

    }

    if (out_type) {
        pkt_clear(pkt);
        pkt_put(pkt, FIELD_APPNAME, app->name, str_topkt);
        daemon_sendto(daemon, out_type, 0, DAEMON_CMDMAN);
    }

    return 1;
}

/** SWLOAD reports executable error.
 */
int swman_incmd_apperror(cmd_t *cmd) {
    app_o app;
    str_o app_name;

    app_name = pkt_get(pkt, FIELD_APPNAME, str_newfrompkt);
    if (!app_name) {
        daemon_error(daemon, "Error in packet");
        return 0;
    }
    app = Set_find(apps_db, app_name, app_findname);
    if (!app) {
        daemon_error(daemon, "Waveform %s not found", str_str(app_name));
        str_delete(&app_name);
        return 1;
    }

    str_delete(&app_name);
    loadapp_error(app);

    return 1;
}

int cnt=0;
int pc=0;

int testdownload(void *x, char **start, char *end) {
    int t, n, i;
    unsigned char *p;
    unsigned int *l;

    assert(x && start && end);
    assert(end >= *start && *start);

    if ((end - *start) < DEFAULT_PKT_SZ) {
        return 0;
    }
    
    n=DEFAULT_PKT_SZ>>2;

    l = (unsigned int*) (char*) * start;
    
    for (i=0;i<n;i++) {
      l[i] = pc*10000+i;
    }
    
    printf("%d,%d,%d,%d\n",l[0],l[1],l[2],l[3]);
    
    pc++;

    *start += n<<2;
/*    printf("packet %d checksum 0x%x\n",cnt,exeobj->checksum);
    cnt++;
*/    
    return 1;
}

void loadexec_error(str_o name, peid_t src_pe) {
    if (name) {
        pkt_clear(pkt);
        pkt_put(pkt, FIELD_EXENAME, name, str_topkt);
    }
    daemon_sendto(daemon, SWLOAD_EXECERR, src_pe, DAEMON_SWLOAD);
}

int getexecfrompkt(execinfo_o *exec, execimp_o *imp)
{
    str_o exename;

    exename = pkt_get(pkt, FIELD_EXENAME, str_newfrompkt);
    if (!exename) {
        daemon_error(daemon, "Error in packet. Missing executable name");
        return 0;
    }

    /**workaround for backwards compatibility, linux executables can
     * be read directly from the path without defining it in the
     * execinfo config file
     */
    if (pkt_getvalue(pkt, FIELD_PLATID) & PLATFORM_LIN) {
        *exec = execinfo_new();
        str_cpy((*exec)->name,exename);
        *imp = execimp_new();
        Set_put((*exec)->versions,*imp);
        (*imp)->name = (*exec)->name;
        (*imp)->platform = pkt_getvalue(pkt, FIELD_PLATID);
        (*imp)->mode = BINARY;
        return 1;
    }


    /** find executable */
    *exec = Set_find(execs,exename,execinfo_findname);
    if (!*exec) {
        daemon_error(daemon, "Can't find executable name %s\n",str_str(exename));
        loadexec_error(exename,pkt_getsrcpe(pkt));
        str_delete(&exename);
        return 0;
    }

    str_delete(&exename);

    /** find implementation version for the platform */
    *imp = Set_find((*exec)->versions,pkt_getptr(pkt, FIELD_PLATID),execimp_findplatformbinary);
    if (!*imp) {

        /* if binary not available, find if source is */
        *imp = Set_find((*exec)->versions,pkt_getptr(pkt, FIELD_PLATID),execimp_findplatformsource);
        if (!*imp) {
            daemon_error(daemon, "Can't find suitable platform version for executable %s\n", str_str(exename));
            loadexec_error(exename,pkt_getsrcpe(pkt));
            return 0;
        }
        return 1;
        /** @todo Compile source code */
    }

}


/** SWMAN: Request executable info
 *
 * SWLOAD requests program memory size so it can allocate it.
 */
int swman_incmd_execinfo(cmd_t *cmd)
{
    execimp_o imp;
    execinfo_o exec;
    peid_t src_pe;
    objid_t objid;
    int platid;

    src_pe = pkt_getsrcpe(pkt);
    objid = pkt_getvalue(pkt, FIELD_OBJID);
    platid = pkt_getvalue(pkt, FIELD_PLATID);

    if (!getexecfrompkt(&exec, &imp)) {
        return 0;
    }

    pkt_clear(pkt);
    pkt_put(pkt, FIELD_PINFO, imp, execimp_pinfotopkt);
    pkt_putvalue(pkt,FIELD_OBJID,objid);
    pkt_putvalue(pkt, FIELD_PLATID, platid);
    pkt_put(pkt,FIELD_EXENAME,exec->name,str_topkt);
    daemon_sendto(daemon, SWLOAD_EXECINFO, src_pe, DAEMON_SWLOAD);

    return 1;
}


/** SWMAN: Processor downloading function
 *
 * After the SWLOAD sensor obtains information about platform characteristics
 * and loading procedures, and in the case absolute addressing is required, memory
 * has been allocated, a command to download the executable for certain platform is
 * issued.
 *
 * This function processes such command. The downloading method is very simple:
 *	1) Check in which form is presented the executable and:
 *		a) source code: Compile for the platform.
 *		b) compiled: Link with ALOE specific platform libraries.
 *		c) linked: Do nothing.
 *	2) If absolute addressing is required and relocator for the platform is available
 *		do the relocation procedure to the appropiate addresses.
 *	3) Send the executable file in packets of pre-defined size (TODO: platform
 *		can choose appropiate size, or negotiate it). This downloading is divided in
 *		three stages:
 *			a) LOADSTART packet: Send a packet indicating the loading starting
 *				and the total number of bytes that will be transmitted.
 *			b) LOADING packets: Send the executable itself, splitted in packets
 *			c) LOADEND packet: Indicate the end of the transmision and the checksum
 */
int swman_incmd_execreq(cmd_t *cmd) {
    peid_t src_pe;
    int n;
    int c = 0;
    int fd = 0;
    int exeid;
    execimp_o imp;
    execinfo_o exec;
    char *pinfoptr;

    src_pe = pkt_getsrcpe(pkt);
    exeid = pkt_getvalue(pkt,FIELD_OBJID);

    if (!getexecfrompkt(&exec, &imp)) {
        return 0;
    }

    /* update local process info with destination platform's */
    pinfoptr=pkt_getptr(pkt,FIELD_PINFO);
    if (pinfoptr) {
        memcpy(&imp->pinfo,pinfoptr,sizeof (struct pinfo));
    }

    if (!execimp_open(imp)) {
        daemon_error(daemon, "Error opening executable");
        loadexec_error(exec->name, src_pe);
        return 0;
    }

    imp->checksum = 0;

    pkt_clear(pkt);
    pkt_putvalue(pkt, FIELD_OBJID, exeid);
    pkt_put(pkt, FIELD_PINFO, imp, execimp_pinfotopkt);
    daemon_sendto(daemon, SWLOAD_LOADSTART, src_pe, DAEMON_SWLOAD);

    daemon_info(daemon, "Sending executable %s...", str_str(exec->name));

    do {
        pkt_clear(pkt);
        pkt_putvalue(pkt, FIELD_OBJID, exeid);
        n = pkt_put(pkt, FIELD_BINDATA, imp, execimp_popbindata);
        if (n > 0) {
            daemon_sendto(daemon, SWLOAD_LOADING, src_pe,
                    DAEMON_SWLOAD);
            c++;
            if (c == UPLOAD_PACKETS) {
                c = 0;
                #ifdef RELINQUISH
                hwapi_relinquish_daemon();
                #else
                sleep_ms(1);
                #endif
            }
        } else if (n < 0) {
            daemon_error(daemon, "Error sending executable %s\n", str_str(exec->name));
            loadexec_error(exec->name, src_pe);
            return 0;
        }
    } while (n);

    pkt_clear(pkt);
    pkt_putvalue(pkt, FIELD_OBJID, exeid);
    pkt_putvalue(pkt, FIELD_CHKSUM, imp->checksum);
    daemon_sendto(daemon, SWLOAD_LOADEND, src_pe, DAEMON_SWLOAD);

    execimp_close(imp);
    
    return 1;
}


/** SWMAN: Request waveform information
 */
int swman_incmd_appinfo(cmd_t *cmd) {
    str_o app_name;
    app_o app;
    str_o obj_name;
    int i;

    app_name = pkt_get(pkt, FIELD_APPNAME, str_newfrompkt);
    if (!app_name) {
        daemon_error(daemon, "Invalid packet");
        return 0;
    }

    app = Set_find(apps_db, app_name, app_findname);
    if (!app) {
        daemon_error(daemon, "Can't find waveform %s", str_str(app_name));
        daemon_sendto(daemon, CMDMAN_APPINFOERR, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
        str_delete(&app_name);
        return 0;
    }

    obj_name = pkt_get(pkt, FIELD_OBJNAME, str_newfrompkt);
#ifdef DEB	
    daemon_info(daemon, "Sending waveform information for %s", str_str(app_name));
#endif
    str_delete(&app_name);

    pkt_clear(pkt);
    app_o tmpapp;
    if (obj_name) {
        phobj_o obj = Set_find(app->objects, obj_name, phobj_findobjname);
        if (!obj) {
            daemon_error(daemon, "Can't find object %s", str_str(obj_name));
            daemon_sendto(daemon, CMDMAN_APPINFOERR, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
            str_delete(&obj_name);
            return 0;
        }
        tmpapp = app_new();
        Set_put(tmpapp->objects, obj);
    } else {
        tmpapp = app;
    }
    for (i=0;i<Set_length(app->objects);i++) {
        phobj_o k = Set_get(app->objects,i);        
    }
    pkt_put(pkt, FIELD_APP, tmpapp, app_topkt);
    daemon_sendto(daemon, CMDMAN_APPINFOOK, pkt_getsrcpe(pkt), DAEMON_CMDMAN);

    return 1;
}

int applist_topkt(void *x, char **start, char *end) {
    return Set_topkt((Set_o) x, start, end, app_topkt);
}

/** SWMAN: Request waveform list
 */
int swman_incmd_applist(cmd_t *cmd) {
    Set_o apps_list;

    #ifdef DEB
    daemon_info(daemon, "Sending waveform's list for %d waveforms", Set_length(apps_db));
    #endif
    apps_list = Set_dup(apps_db, app_xdupnoobj);
    if (!apps_list) {
        daemon_error(daemon, "Error creating apps list");
        daemon_sendto(daemon, CMDMAN_APPLSERR, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
        return 0;
    }

    pkt_clear(pkt);
    pkt_put(pkt, FIELD_APP, apps_list, applist_topkt);
    daemon_sendto(daemon, CMDMAN_APPLSOK, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
    Set_destroy(&apps_list, app_xdelete);
    return 1;
}

char logpath[128];

int reports_startstop(str_o app_name, int start)
{
    app_o app;
    phobj_o obj;
    int i;
    char t[4];

#ifdef DEB
    daemon_info(daemon, "%s report for waveform %s", start ? "Starting" : "Stopping", str_str(app_name));
#endif
    app = Set_find(apps_db, app_name, app_findname);
    if (!app) {
        return 0;
    }
    app_o app_pkt;
    i = 0;
    int pe = -1;

    do {
        app_pkt = app_new();
        assert(app_pkt);
        str_cpy(app_pkt->name, app->name);
        pe = -1;
        do {
            obj = phobj_dup(Set_get(app->objects, i));
            assert(obj);
            Set_destroy(&obj->params, var_xdelete);
            obj->params = Set_new(0, NULL);
            Set_destroy(&obj->stats, var_xdelete);
            obj->stats = Set_new(0, NULL);
            if (pe == -1) {
                pe = obj->pe_id;
            }
            if (pe == obj->pe_id) {
                Set_put(app_pkt->objects, obj);
                i++;
            }
        } while (pe == obj->pe_id && i < Set_length(app->objects));
        assert(pe > 0);
        sprintf(t,"%d",i);
        str_set(app_pkt->name,strcat(str_str(app_pkt->name),t));
        pkt_clear(pkt);
        pkt_put(pkt, FIELD_APP, app_pkt, app_topkt);
        if (pe!=obj->pe_id) {
            phobj_delete(&obj);
        }
        app_delete(&app_pkt);
        int cmd = start ? EXEC_REPORTSTART : EXEC_REPORTSTOP;
        daemon_sendto(daemon, cmd, pe, DAEMON_EXEC);
    } while (i < Set_length(app->objects));

    return 1;
}

/** SWMAN: Answer waveform information
 */
int swman_incmd_appinfocmd(cmd_t *cmd) {
    app_o app;
    str_o app_name;
    phobj_o obj;
    int i;

    app_name = pkt_get(pkt, FIELD_APPNAME, str_newfrompkt);
    if (!app_name) {
        daemon_error(daemon, "Invalid packet");
        return 0;
    }
    switch (pkt_getcmd(pkt)) {
        case SWMAN_REPORTSTART:
        case SWMAN_REPORTSTOP:
            if (reports_startstop(app_name, pkt_getcmd(pkt) == SWMAN_REPORTSTART)) {
                daemon_sendto(daemon, CMDMAN_REPORTOK, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
            } else {
                daemon_sendto(daemon, CMDMAN_REPORTERR, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
            }
            break;
        case SWMAN_LOGSSTART:
            app = Set_find(apps_db, app_name, app_findname);
            if (!app) {
                daemon_error(daemon, "Can't find waveform");
                daemon_sendto(daemon, CMDMAN_LOGSERR, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
                str_delete(&app_name);
                return 1;
            }
            log_o log = NULL;
            for (i = 0; i < Set_length(app->objects); i++) {
                obj = Set_get(app->objects, i);
                assert(obj);

                sprintf(logpath, "%s/%s/%s", BASE_LOG_PATH, str_str(obj->appname), str_str(obj->objname));

                log = log_new(-1, logpath);
                if (!log) {
                    daemon_error(daemon, "Could not create log for object %s", str_str(obj->objname));
                    break;
                }

                Set_put(obj->logs, log);
            }
            daemon_sendto(daemon, CMDMAN_LOGSOK, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
            break;
        case SWMAN_LOGSSTOP:
            app = Set_find(apps_db, app_name, app_findname);
            if (!app) {
                daemon_error(daemon, "Can't find waveform");
                daemon_sendto(daemon, CMDMAN_LOGSERR, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
                str_delete(&app_name);
                return 1;
            }
            int n;
            for (i = 0; i < Set_length(app->objects); i++) {
                obj = Set_get(app->objects, i);
                assert(obj);
                n = -1;
                log = Set_find(obj->logs, &n, log_findid);
                if (log) {
                    Set_remove(obj->logs, log);
                    log_delete(&log);
                }
            }
            daemon_sendto(daemon, CMDMAN_LOGSOK, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
            break;
    }

    str_delete(&app_name);

    return 1;
}

char logstr[128];

/** SWMAN: waveform information report
 */
int swman_incmd_appinforeport(cmd_t *cmd) {
    phobj_o objpkt, objlocal;
    int i, n;
    app_o app_pkt, app_local;
    log_o log;
    int tmprtfaults;
    int ts_diff;

    app_pkt = (app_o) pkt_get(pkt, FIELD_APP, app_newfrompkt);
    if (!app_pkt) {
        daemon_error(daemon, "Invalid packet");
        return 0;
    }

    char t = 0;
    app_local = NULL;
    for (i = 0; i < Set_length(app_pkt->objects); i++) {
        objpkt = Set_get(app_pkt->objects, i);
        assert(objpkt);

        if (app_local) {
            t = str_cmp(app_local->name, objpkt->appname);
        }
        if (!app_local | t) {
            app_local = Set_find(apps_db, objpkt->appname, app_findname);
            if (!app_local) {
                daemon_error(daemon, "Unknown waveform %s", str_str(objpkt->appname));
                break;
            }
        }

#ifdef TS_CORRECT
        if (objpkt->rtc->rep_tstamp>app_local->report_tstamp+REPORT_INTERVAL/2) {
        	app_local->report_tstamp=objpkt->rtc->rep_tstamp;
        	ts_diff=0;
        } else {
        	ts_diff=objpkt->rtc->rep_tstamp-app_local->report_tstamp;
        }
#else
        ts_diff=0;
#endif
        objlocal = Set_find(app_local->objects, objpkt, phobj_cmpid);
        if (!objlocal) {
            daemon_error(daemon, "Unknown object %s at app %s. Skipping report.\n", str_str(objpkt->objname), str_str(objpkt->appname));
            app_delete(&app_pkt);
            return 0;
        }

        /* keep track of rtfaults */
        tmprtfaults = objlocal->rtc->nof_faults;

        rtcobj_delete(&objlocal->rtc);
        objlocal->rtc = rtcobj_dup(objpkt->rtc);
        objlocal->rtc->tstamp-=ts_diff;
        objlocal->rtc->nof_faults = tmprtfaults;
        objlocal->core_idx = objpkt->core_idx;

        Set_destroy(&objlocal->itfs, phitf_xdelete);
        objlocal->itfs = Set_dup(objpkt->itfs, phitf_xdup);

        if (objlocal->logs) {
            n = -1;
            log = Set_find(objlocal->logs, &n, log_findid);
            if (log) {
                sprintf(logstr, "%d,%d", objlocal->rtc->cpu_usec, objlocal->rtc->tstamp);
                log_Write(log, logstr);
            }
        }
    }

    app_delete(&app_pkt);

    return 1;

}





