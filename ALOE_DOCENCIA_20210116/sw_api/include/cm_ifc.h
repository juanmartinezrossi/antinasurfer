/*
 * cm_ifc.h
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

/* Declarations to interface with other	*/
/* modules using CM API					*/
/****************************************/

#include "phal_sw_api.h"

#ifndef CM_IFC
#define CM_IFC

/*Different status*/
#ifndef TBSTATUS
#define TB_INIT  PHAL_STATUS_INIT
#define TB_RUN   PHAL_STATUS_RUN
#define TB_STOP  PHAL_STATUS_STOP
#endif


/*FLow states*/
#define FLOW_CORRECT 1
#define FLOW_ERROR  -1
#define FLOW_EINVAL -2

/*Functions requesting info of other modules may receive these errors*/
#define QUERY_SYSERROR   -1
#define QUERY_TIMEOUT    -2
#define QUERY_NO_INIT    -3
#define QUERY_NOREG      -4
#define QUERY_DATAERR    -6
#define QUERY_BADPARAM   -7
#define QUERY_AGAIN      -8

/*CM API has NewFlow, no CreateFlow*/
#define CreateFlow NewFlow

/*Information of flows*/
struct flow_config {
    char rw;
    char type;
    char options;
    const char *name;
    unsigned int max_size;
    unsigned int magic;
};

/*Global control functions*/
int TestBedStatus();
int InitCommManager(int module_id);
int InitCommManagerN(int module_id, const char *name);
int CloseCommManager(int module_id);

/*Message/Packet passing functions*/
int CreateFlow(int module_id, int remote_module, struct flow_config fconf);
int WriteFlow(int flow_id, unsigned int nof_bytes, void *buffer);
int ReadFlow(int flow_id, unsigned int nof_bytes, void *buffer);
int GetFlowStatus(int flow_id);
int CheckFlow(int flow_id);
int DeleteFlow(int flow_id);

/*Initialisation files reading*/
int ReadInitFile();
int GetParameter(int file_id, char *param_name, void *param);
int GetParameterMid(int file_id, char *pname, void *param, int mid);

/*Log files generation*/
int WriteMsg(int log_id, int dest, char *short_msg_desc, char *long_msg_desc);
int WriteVar(int log_id, char *var_name, char *var_value);
int LogStat(int log_id, int stat_id, int idx);
int GetNameMid(int mid, char *name, int buffer_size);

/*Statistics Control*/
int InitStatistics(int module_id);
int CreateStat(char *name);
int SetStatValue(int id, void *value, int idx);
int GetStatValue(int id, void *value, int idx);
int SetStatLabel(int vect_id, char *label, int idx);
int GetStatIndex(int trow_id, char *element_name);
int GetStatId(int trow_id, char *element_name);
int DeleteStat(int id);
int CloseStatistics(int module_id);

/*Other*/
int GetTimeStamp();
void UnrecoveryError(char *string);

#endif
