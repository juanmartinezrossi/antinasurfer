/*
 * cmdman_backend.h
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

#define PNAME_LEN 128

#define MAX_ITFS    50		/*AGBJAN19*/

struct itf_info {
    int mode;
    char name[PNAME_LEN];
    int remote_name[PNAME_LEN];
    int remote_obj_name[PNAME_LEN];
    int xitf_id;
    int fifo_usage;
    int bpts;
};

struct proc_info {
    char name[PNAME_LEN];
    int obj_id;
    int pe_id;
    int core_id;
    int status;
    int tstamp;
    int period;
    int nvcs;
    int cpu_usec;
    int max_usec;
    float mean_mops;
    float max_mops;
    int max_end_usec;
    int start_usec;
    int end_usec;
    int nof_itf;
    int nof_faults;
    float C;
    struct itf_info itf[MAX_ITFS];
};

struct app_info {
    char name[PNAME_LEN];
    int app_id;
    int status;
    int nof_rtfaults;
};

struct stat_info {
    char objname[PNAME_LEN];
    char statname[PNAME_LEN];
    int type;
    int size;
};

struct xitf_info {
    struct hwapi_xitf_i xitf;
    float totalBW;
    xitfid_t remote_id;
    peid_t remote_pe;
};

struct pe_info {
    char name[CPU_NAME_LEN];
    int id;
    int plat_family;
    int nof_cores;
    float C;
    float intBW;
    float totalC;
    float totalintBW;
    int nof_xitf;
    int core_id;
    struct xitf_info xitf[MAX_ITFS];
};

int cmdman_init(void);

int cmdman_loadapp(char *app_name, float g, int max_sec);

int cmdman_appstatus(char *app_name, int new_status, int ts_ini, int ts_len, int max_sec);

int cmdman_statget(char *app_name, char *obj_name, char *stat_name, int window,
        void *value, int *type, int *tstamp, int max_size, int max_sec);

int cmdman_statlist(char *app_name, char *obj_name, struct stat_info *stats, int max_stats, int max_sec);
int cmdman_statset(char *app_name, char *obj_name, char *stat_name, void *value, int size, int type, int max_sec);
int cmdman_statreport(char *app_name, char *obj_name, char *stat_name,
        int action, int window, int period, int max_sec);

int cmdman_execreports(char *app_name, int start, int max_sec);
int cmdman_execlogs(char *app_name, int start, int max_sec);
int cmdman_appinfo(char *app_name, char *obj_name, struct proc_info *proc, int max_procs, int max_sec);
int cmdman_applist(struct app_info *apps, int max_apps, int max_sec);
int cmdman_pelist(struct pe_info *peinfo, int max_pe, int max_sec);

